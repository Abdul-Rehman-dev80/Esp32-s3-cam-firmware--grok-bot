#include "digilog_s3cam_oled_board.h"
#include "digilog_face_display.h"
#include "boards/common/button.h"
#include "boards/common/esp32_camera.h"
#include "audio/codecs/no_audio_codec.h"
#include "led/single_led.h"
#include "application.h"
#include "mcp_server.h"

#include <esp_log.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <driver/ledc.h>

#ifdef SH1106
#include <esp_lcd_panel_sh1106.h>
#endif

static const char* TAG = "DigilogS3CamOledBoard";

// LEDC resolution for servo PWM (allows fine-grained pulse width control)
static constexpr ledc_timer_bit_t kServoLedcResolution = LEDC_TIMER_14_BIT;
static constexpr uint32_t kServoLedcMaxDuty = (1 << 14) - 1;  // 16383

DigilogS3CamOledBoard::DigilogS3CamOledBoard() {
    ESP_LOGI(TAG, "Initializing Digilog S3CAM OLED board");

    InitializeDisplayI2c();
    InitializeSsd1306Display();
    InitializeServos();
    InitializeCamera();
    InitializeButtons();
    RegisterMcpTools();

    ESP_LOGI(TAG, "Board initialization complete");
}

void DigilogS3CamOledBoard::InitializeDisplayI2c() {
    ESP_LOGI(TAG, "Initializing display I2C bus");

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = DISPLAY_SDA_PIN,
        .scl_io_num = DISPLAY_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus_));
}

void DigilogS3CamOledBoard::InitializeSsd1306Display() {
    ESP_LOGI(TAG, "Initializing SSD1306 OLED display");

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = DISPLAY_I2C_ADDR,
        .scl_speed_hz = 400 * 1000,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .on_color_trans_done = nullptr,
        .user_ctx = nullptr,
        .flags = {
            .dc_low_on_data = 0,
            .disable_control_phase = 0,
        },
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(display_i2c_bus_, &io_config, &panel_io_));

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = GPIO_NUM_NC;
    panel_config.bits_per_pixel = 1;

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = static_cast<uint8_t>(DISPLAY_HEIGHT),
    };
    panel_config.vendor_config = &ssd1306_config;

#ifdef SH1106
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(panel_io_, &panel_config, &panel_));
#else
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io_, &panel_config, &panel_));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));

    if (esp_lcd_panel_init(panel_) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display panel");
        display_ = new NoDisplay();
        return;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

#ifdef CONFIG_DIGILOG_FACE_ENABLE
    display_ = new DigilogFaceDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                       DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
#else
    display_ = new OledDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                               DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
#endif

    ESP_LOGI(TAG, "SSD1306 display initialized (%dx%d)", DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

void DigilogS3CamOledBoard::InitializeServos() {
    ESP_LOGI(TAG, "Initializing dual SG90 servos on GPIO%d (yaw) + GPIO%d (pitch) @ 50Hz",
             SERVO1_GPIO, SERVO2_GPIO);

    // Configure LEDC timer for 50Hz servo PWM
    ledc_timer_config_t timer_config = {
        .speed_mode = SERVO_LEDC_MODE,
        .duty_resolution = kServoLedcResolution,
        .timer_num = SERVO_LEDC_TIMER,
        .freq_hz = SERVO_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    // Configure channel for Servo 1 (Yaw/Base)
    ledc_channel_config_t ch1_config = {
        .gpio_num = SERVO1_GPIO,
        .speed_mode = SERVO_LEDC_MODE,
        .channel = SERVO_LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_LEDC_TIMER,
        .duty = AngleToDuty(SERVO_CENTER_ANGLE),
        .hpoint = 0,
        .flags = {
            .output_invert = 0,
        },
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch1_config));

    // Configure channel for Servo 2 (Pitch/Head)
    ledc_channel_config_t ch2_config = {
        .gpio_num = SERVO2_GPIO,
        .speed_mode = SERVO_LEDC_MODE,
        .channel = SERVO_LEDC_CHANNEL_2,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_LEDC_TIMER,
        .duty = AngleToDuty(SERVO_CENTER_ANGLE),
        .hpoint = 0,
        .flags = {
            .output_invert = 0,
        },
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch2_config));

    servo1_angle_ = SERVO_CENTER_ANGLE;
    servo2_angle_ = SERVO_CENTER_ANGLE;

    ESP_LOGI(TAG, "Servos initialized at center position (90 degrees)");
}

uint32_t DigilogS3CamOledBoard::AngleToDuty(int angle) {
    // Clamp angle to valid range
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    // Map angle (0-180) to pulse width (500-2500us)
    uint32_t pulse_us = SERVO_MIN_PULSE_US +
        (angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) / SERVO_MAX_ANGLE;

    // Convert pulse width to LEDC duty
    // At 50Hz, period = 20ms = 20000us
    // duty = (pulse_us / 20000) * max_duty
    uint32_t duty = (pulse_us * kServoLedcMaxDuty) / 20000;

    return duty;
}

void DigilogS3CamOledBoard::SetServo1Angle(int angle) {
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    uint32_t duty = AngleToDuty(angle);
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL_1, duty));
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL_1));

    servo1_angle_ = angle;
    ESP_LOGD(TAG, "Servo1 (yaw/base) set to %d degrees", angle);
}

void DigilogS3CamOledBoard::SetServo2Angle(int angle) {
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    uint32_t duty = AngleToDuty(angle);
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL_2, duty));
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL_2));

    servo2_angle_ = angle;
    ESP_LOGD(TAG, "Servo2 (pitch/head) set to %d degrees", angle);
}

void DigilogS3CamOledBoard::InitializeCamera() {
    ESP_LOGI(TAG, "Initializing OV2640 camera");

    camera_config_t config = {};
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sccb_sda = CAMERA_PIN_SIOD;
    config.pin_sccb_scl = CAMERA_PIN_SIOC;
    config.sccb_i2c_port = 1;  // Use I2C port 1 (port 0 is display)
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    camera_ = new Esp32Camera(config);
    ESP_LOGI(TAG, "Camera initialized");
}

void DigilogS3CamOledBoard::InitializeButtons() {
    ESP_LOGI(TAG, "Initializing buttons");

    boot_button_ = new Button(BOOT_BUTTON_GPIO);

    boot_button_->OnClick([this]() {
        auto& app = Application::GetInstance();
        if (app.GetDeviceState() == kDeviceStateStarting) {
            EnterWifiConfigMode();
            return;
        }
        app.ToggleChatState();
    });
}

void DigilogS3CamOledBoard::RegisterMcpTools() {
    ESP_LOGI(TAG, "Registering MCP servo and camera tools");

    auto& mcp = McpServer::GetInstance();

    // =========================================================================
    // Base/Yaw Servo Tools (self.base.*)
    // =========================================================================

    mcp.RegisterTool("self.base.set_angle",
        "Rotate the desk robot camera/base (yaw SG90 servo) to an absolute angle. "
        "Use for look/turn commands: 90 is center/forward, smaller angles turn left, "
        "larger turn right. Angle range 0-180.",
        {{"angle", "integer", "Target angle (0-180, 90=center)", true}},
        [this](const auto& args) -> std::string {
            int angle = args.at("angle").get<int>();
            SetServo1Angle(angle);
            return "Base angle set to " + std::to_string(servo1_angle_);
        });

    mcp.RegisterTool("self.base.turn_left",
        "Turn the desk robot camera/base left (decrease yaw angle). "
        "Call this when the user asks to look left or turn left. Clamped to 0-180.",
        {{"degrees", "integer", "Degrees to turn left (default 15)", false}},
        [this](const auto& args) -> std::string {
            int deg = args.contains("degrees") ? args.at("degrees").get<int>() : 15;
            int new_angle = servo1_angle_ - deg;
            SetServo1Angle(new_angle);
            return "Turned left to " + std::to_string(servo1_angle_);
        });

    mcp.RegisterTool("self.base.turn_right",
        "Turn the desk robot camera/base right (increase yaw angle). "
        "Call this when the user asks to look right or turn right. Clamped to 0-180.",
        {{"degrees", "integer", "Degrees to turn right (default 15)", false}},
        [this](const auto& args) -> std::string {
            int deg = args.contains("degrees") ? args.at("degrees").get<int>() : 15;
            int new_angle = servo1_angle_ + deg;
            SetServo1Angle(new_angle);
            return "Turned right to " + std::to_string(servo1_angle_);
        });

    mcp.RegisterTool("self.base.center",
        "Center the desk robot camera/base (yaw) to forward-facing 90 degrees. "
        "Use when the user asks to look forward, center, or reset pan. "
        "Does not change head pitch/tilt.",
        {},
        [this](const auto&) -> std::string {
            SetServo1Angle(SERVO_CENTER_ANGLE);
            return "Base centered to 90 degrees";
        });

    // =========================================================================
    // Head/Pitch Servo Tools (self.head.*)
    // =========================================================================

    mcp.RegisterTool("self.head.set_angle",
        "Set the desk robot head pitch/tilt (second SG90) to an absolute angle. "
        "90 is level/center; smaller angles look down, larger look up "
        "(or reverse depending on mount). Angle range 0-180.",
        {{"angle", "integer", "Target angle (0-180, 90=level)", true}},
        [this](const auto& args) -> std::string {
            int angle = args.at("angle").get<int>();
            SetServo2Angle(angle);
            return "Head angle set to " + std::to_string(servo2_angle_);
        });

    mcp.RegisterTool("self.head.look_up",
        "Tilt the desk robot head up (increase pitch angle). "
        "Call when the user asks to look up. Clamped to 0-180.",
        {{"degrees", "integer", "Degrees to tilt up (default 15)", false}},
        [this](const auto& args) -> std::string {
            int deg = args.contains("degrees") ? args.at("degrees").get<int>() : 15;
            int new_angle = servo2_angle_ + deg;
            SetServo2Angle(new_angle);
            return "Looked up to " + std::to_string(servo2_angle_);
        });

    mcp.RegisterTool("self.head.look_down",
        "Tilt the desk robot head down (decrease pitch angle). "
        "Call when the user asks to look down. Clamped to 0-180.",
        {{"degrees", "integer", "Degrees to tilt down (default 15)", false}},
        [this](const auto& args) -> std::string {
            int deg = args.contains("degrees") ? args.at("degrees").get<int>() : 15;
            int new_angle = servo2_angle_ - deg;
            SetServo2Angle(new_angle);
            return "Looked down to " + std::to_string(servo2_angle_);
        });

    mcp.RegisterTool("self.head.center",
        "Center the desk robot head pitch/tilt to level 90 degrees. "
        "Use when the user asks to level the head, look straight, or reset tilt.",
        {},
        [this](const auto&) -> std::string {
            SetServo2Angle(SERVO_CENTER_ANGLE);
            return "Head centered to 90 degrees";
        });

    ESP_LOGI(TAG, "MCP tools registered: base.set_angle, turn_left, turn_right, center; "
                  "head.set_angle, look_up, look_down, center");
}

Led* DigilogS3CamOledBoard::GetLed() {
    static SingleLed led(BUILTIN_LED_GPIO);
    return &led;
}

AudioCodec* DigilogS3CamOledBoard::GetAudioCodec() {
    static NoAudioCodecSimplex audio_codec(
        AUDIO_INPUT_SAMPLE_RATE,
        AUDIO_OUTPUT_SAMPLE_RATE,
        AUDIO_I2S_SPK_GPIO_BCLK,
        AUDIO_I2S_SPK_GPIO_LRCK,
        AUDIO_I2S_SPK_GPIO_DOUT,
        AUDIO_I2S_MIC_GPIO_SCK,
        AUDIO_I2S_MIC_GPIO_WS,
        AUDIO_I2S_MIC_GPIO_DIN
    );
    return &audio_codec;
}

Display* DigilogS3CamOledBoard::GetDisplay() {
    return display_;
}

Camera* DigilogS3CamOledBoard::GetCamera() {
    return camera_;
}

// Board factory registration
DECLARE_BOARD(DigilogS3CamOledBoard);
