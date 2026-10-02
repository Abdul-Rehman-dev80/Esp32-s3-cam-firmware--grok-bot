#ifndef DIGILOG_S3CAM_OLED_BOARD_H_
#define DIGILOG_S3CAM_OLED_BOARD_H_

#include "boards/common/wifi_board.h"
#include "config.h"
#include <driver/ledc.h>
#include <esp_lcd_panel_io.h>

class DigilogS3CamOledBoard : public WifiBoard {
public:
    DigilogS3CamOledBoard();
    virtual ~DigilogS3CamOledBoard() = default;

    virtual Led* GetLed() override;
    virtual AudioCodec* GetAudioCodec() override;
    virtual Display* GetDisplay() override;
    virtual Camera* GetCamera() override;

    // Servo control methods (exposed for MCP tools)
    void SetServo1Angle(int angle);  // Yaw/Base servo
    void SetServo2Angle(int angle);  // Pitch/Head servo
    int GetServo1Angle() const { return servo1_angle_; }
    int GetServo2Angle() const { return servo2_angle_; }

private:
    // I2C bus for display
    i2c_master_bus_handle_t display_i2c_bus_ = nullptr;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;

    // Display, camera, button
    Display* display_ = nullptr;
    class Esp32Camera* camera_ = nullptr;
    class Button* boot_button_ = nullptr;

    // Servo state
    int servo1_angle_ = SERVO_CENTER_ANGLE;
    int servo2_angle_ = SERVO_CENTER_ANGLE;

    // Initialization methods
    void InitializeDisplayI2c();
    void InitializeSsd1306Display();
    void InitializeServos();
    void InitializeCamera();
    void InitializeButtons();
    void RegisterMcpTools();

    // Helper to convert angle to LEDC duty
    static uint32_t AngleToDuty(int angle);
};

#endif // DIGILOG_S3CAM_OLED_BOARD_H_
