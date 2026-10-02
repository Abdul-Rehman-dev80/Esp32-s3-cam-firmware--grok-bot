#include "esp32_camera.h"
#include <esp_log.h>

static const char* TAG = "Esp32Camera";

Esp32Camera::Esp32Camera(const camera_config_t& config) {
    ESP_LOGI(TAG, "Initializing ESP32 camera");

    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", ret);
        return;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "Camera initialized successfully");
}

Esp32Camera::~Esp32Camera() {
    if (initialized_) {
        esp_camera_deinit();
    }
}

bool Esp32Camera::TakePhoto(std::vector<uint8_t>& jpeg_data) {
    if (!initialized_) {
        ESP_LOGE(TAG, "Camera not initialized");
        return false;
    }

    camera_fb_t* fb = CaptureFrame();
    if (!fb) {
        ESP_LOGE(TAG, "Failed to capture frame");
        return false;
    }

    // Copy JPEG data
    jpeg_data.assign(fb->buf, fb->buf + fb->len);

    ReleaseFrame(fb);

    ESP_LOGI(TAG, "Photo captured: %zu bytes", jpeg_data.size());
    return true;
}

camera_fb_t* Esp32Camera::CaptureFrame() {
    return esp_camera_fb_get();
}

void Esp32Camera::ReleaseFrame(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}
