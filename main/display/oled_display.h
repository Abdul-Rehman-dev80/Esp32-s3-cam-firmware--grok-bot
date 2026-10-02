#ifndef OLED_DISPLAY_H_
#define OLED_DISPLAY_H_

#include <esp_lcd_panel_io.h>
#include <lvgl.h>
#include <string>

class Display {
public:
    virtual ~Display() = default;
    virtual void SetupUI() {}
    virtual void SetEmotion(const std::string& emotion) {}
    virtual void ShowNotification(const std::string& message, int duration_ms = 3000) {}
    virtual void SetBrightness(int brightness) {}
    virtual int GetWidth() const { return 0; }
    virtual int GetHeight() const { return 0; }
};

class OledDisplay : public Display {
public:
    OledDisplay(esp_lcd_panel_io_handle_t panel_io,
                esp_lcd_panel_handle_t panel,
                int width, int height,
                bool mirror_x, bool mirror_y);
    virtual ~OledDisplay();

    virtual void SetupUI() override;
    virtual void SetEmotion(const std::string& emotion) override;
    virtual void ShowNotification(const std::string& message, int duration_ms = 3000) override;
    virtual int GetWidth() const override { return width_; }
    virtual int GetHeight() const override { return height_; }

protected:
    esp_lcd_panel_io_handle_t panel_io_;
    esp_lcd_panel_handle_t panel_;
    int width_;
    int height_;
    bool mirror_x_;
    bool mirror_y_;
    lv_disp_t* disp_ = nullptr;
};

#endif // OLED_DISPLAY_H_
