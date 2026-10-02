#ifndef DIGILOG_S3CAM_OLED_CONFIG_H_
#define DIGILOG_S3CAM_OLED_CONFIG_H_

#include <driver/gpio.h>

// =============================================================================
// Audio Configuration
// =============================================================================
#define AUDIO_INPUT_SAMPLE_RATE 16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// NoAudioCodecSimplex: separate mic and amp I2S lines
#define AUDIO_I2S_METHOD_SIMPLEX

// INMP441 I2S Microphone (from PINOUT.md)
#define AUDIO_I2S_MIC_GPIO_WS    GPIO_NUM_1     // WS (Word Select / LRCK)
#define AUDIO_I2S_MIC_GPIO_SCK   GPIO_NUM_2     // SCK (Bit Clock)
#define AUDIO_I2S_MIC_GPIO_DIN   GPIO_NUM_14    // SD (Serial Data)

// MAX98357A I2S Amplifier (from PINOUT.md)
#define AUDIO_I2S_SPK_GPIO_DOUT  GPIO_NUM_39    // DIN
#define AUDIO_I2S_SPK_GPIO_BCLK  GPIO_NUM_40    // BCLK
#define AUDIO_I2S_SPK_GPIO_LRCK  GPIO_NUM_21    // LRC

// =============================================================================
// Display Configuration - SSD1306 OLED (from PINOUT.md)
// =============================================================================
#define DISPLAY_SDA_PIN          GPIO_NUM_41
#define DISPLAY_SCL_PIN          GPIO_NUM_42
#define DISPLAY_I2C_ADDR         0x3C           // Standard SSD1306 address
#define DISPLAY_WIDTH            128

#if defined(CONFIG_OLED_SSD1306_128X32)
#define DISPLAY_HEIGHT           32
#elif defined(CONFIG_OLED_SSD1306_128X64)
#define DISPLAY_HEIGHT           64
#elif defined(CONFIG_OLED_SH1106_128X64)
#define DISPLAY_HEIGHT           64
#define SH1106
#else
#define DISPLAY_HEIGHT           64             // Default to 128x64
#endif

#define DISPLAY_MIRROR_X         true
#define DISPLAY_MIRROR_Y         true

// =============================================================================
// Servo Configuration - Dual SG90 @ 50Hz (from PINOUT.md)
// =============================================================================
#define SERVO1_GPIO              GPIO_NUM_47    // Yaw/Base servo
#define SERVO2_GPIO              GPIO_NUM_45    // Pitch/Head servo
#define SERVO_LEDC_TIMER         LEDC_TIMER_0
#define SERVO_LEDC_MODE          LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_CHANNEL_1     LEDC_CHANNEL_0
#define SERVO_LEDC_CHANNEL_2     LEDC_CHANNEL_1
#define SERVO_FREQUENCY_HZ       50             // 50Hz = 20ms period

// SG90 servo timing (in microseconds)
#define SERVO_MIN_PULSE_US       500            // 0 degrees
#define SERVO_MAX_PULSE_US       2500           // 180 degrees
#define SERVO_CENTER_PULSE_US    1500           // 90 degrees (center)

// Angle limits
#define SERVO_MIN_ANGLE          0
#define SERVO_MAX_ANGLE          180
#define SERVO_CENTER_ANGLE       90

// =============================================================================
// Camera Configuration - OV2640 onboard ESP32-S3-CAM
// Camera pins based on bread-compact-wifi-s3cam upstream reference
// (standard ESP32-S3-CAM module pinout)
// =============================================================================
#define CAMERA_PIN_D0            GPIO_NUM_11
#define CAMERA_PIN_D1            GPIO_NUM_9
#define CAMERA_PIN_D2            GPIO_NUM_8
#define CAMERA_PIN_D3            GPIO_NUM_10
#define CAMERA_PIN_D4            GPIO_NUM_12
#define CAMERA_PIN_D5            GPIO_NUM_18
#define CAMERA_PIN_D6            GPIO_NUM_17
#define CAMERA_PIN_D7            GPIO_NUM_16
#define CAMERA_PIN_XCLK          GPIO_NUM_15
#define CAMERA_PIN_PCLK          GPIO_NUM_13
#define CAMERA_PIN_VSYNC         GPIO_NUM_6
#define CAMERA_PIN_HREF          GPIO_NUM_7
#define CAMERA_PIN_SIOC          GPIO_NUM_5     // SCCB Clock (I2C SCL)
#define CAMERA_PIN_SIOD          GPIO_NUM_4     // SCCB Data (I2C SDA)
#define CAMERA_PIN_PWDN          GPIO_NUM_NC    // Power down (not used)
#define CAMERA_PIN_RESET         GPIO_NUM_NC    // Reset (not used)
#define XCLK_FREQ_HZ             20000000       // 20MHz XCLK

// =============================================================================
// LED and Buttons
// =============================================================================
#define BUILTIN_LED_GPIO         GPIO_NUM_48    // Onboard RGB LED (if present)
#define BOOT_BUTTON_GPIO         GPIO_NUM_0     // Standard BOOT button

// =============================================================================
// Face UI Configuration
// =============================================================================
#define DIGILOG_FACE_ANIM_PERIOD_MS  100        // Face animation update period

#endif // DIGILOG_S3CAM_OLED_CONFIG_H_
