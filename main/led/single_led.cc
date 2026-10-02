#include "single_led.h"
#include <esp_log.h>

static const char* TAG = "SingleLed";

SingleLed::SingleLed(gpio_num_t gpio) : gpio_(gpio) {
    if (gpio == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "LED GPIO not configured");
        return;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    SetState(false);
    ESP_LOGI(TAG, "LED initialized on GPIO%d", gpio);
}

void SingleLed::SetState(bool on) {
    if (gpio_ == GPIO_NUM_NC) return;

    state_ = on;
    gpio_set_level(gpio_, on ? 1 : 0);
}

void SingleLed::SetBrightness(int brightness) {
    // Simple LED - on/off only
    SetState(brightness > 50);
}

void SingleLed::SetColor(uint32_t color) {
    // Simple LED - ignore color
    SetState(color != 0);
}

void SingleLed::Blink(int on_ms, int off_ms, int count) {
    // Simplified - just turn on
    SetState(true);
}
