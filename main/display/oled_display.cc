#include "oled_display.h"
#include <esp_log.h>
#include <esp_lvgl_port.h>

static const char* TAG = "OledDisplay";

OledDisplay::OledDisplay(esp_lcd_panel_io_handle_t panel_io,
                         esp_lcd_panel_handle_t panel,
                         int width, int height,
                         bool mirror_x, bool mirror_y)
    : panel_io_(panel_io)
    , panel_(panel)
    , width_(width)
    , height_(height)
    , mirror_x_(mirror_x)
    , mirror_y_(mirror_y) {
    ESP_LOGI(TAG, "Creating OLED display %dx%d", width, height);

    // Initialize LVGL port
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    // Add display to LVGL
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = panel_io,
        .panel_handle = panel,
        .buffer_size = width * height / 8,
        .double_buffer = false,
        .hres = static_cast<uint32_t>(width),
        .vres = static_cast<uint32_t>(height),
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
    };
    disp_ = lvgl_port_add_disp(&disp_cfg);

    SetupUI();
}

OledDisplay::~OledDisplay() {
    // LVGL cleanup
}

void OledDisplay::SetupUI() {
    ESP_LOGI(TAG, "Setting up OLED UI");

    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Digilog");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void OledDisplay::SetEmotion(const std::string& emotion) {
    ESP_LOGD(TAG, "Set emotion: %s", emotion.c_str());
}

void OledDisplay::ShowNotification(const std::string& message, int duration_ms) {
    ESP_LOGI(TAG, "Notification: %s", message.c_str());
}
