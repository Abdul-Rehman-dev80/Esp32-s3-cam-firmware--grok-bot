#include "button.h"
#include <esp_log.h>
#include <iot_button.h>

static const char* TAG = "Button";

Button::Button(gpio_num_t gpio, bool active_low)
    : gpio_(gpio), active_low_(active_low) {
    if (gpio == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "Button GPIO not configured");
        return;
    }

    button_config_t config = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 1500,
        .short_press_time = 180,
        .gpio_button_config = {
            .gpio_num = gpio,
            .active_level = active_low ? 0 : 1,
        },
    };

    esp_err_t ret = iot_button_create(&config, &button_handle_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button on GPIO%d", gpio);
        return;
    }

    ESP_LOGI(TAG, "Button created on GPIO%d", gpio);
}

Button::~Button() {
    if (button_handle_) {
        iot_button_delete(button_handle_);
    }
}

void Button::OnClick(Callback callback) {
    on_click_ = callback;
    if (button_handle_ && callback) {
        iot_button_register_cb(button_handle_, BUTTON_SINGLE_CLICK, ButtonEventCallback, this);
    }
}

void Button::OnLongPress(Callback callback) {
    on_long_press_ = callback;
    if (button_handle_ && callback) {
        iot_button_register_cb(button_handle_, BUTTON_LONG_PRESS_START, ButtonEventCallback, this);
    }
}

void Button::OnPressDown(Callback callback) {
    on_press_down_ = callback;
    if (button_handle_ && callback) {
        iot_button_register_cb(button_handle_, BUTTON_PRESS_DOWN, ButtonEventCallback, this);
    }
}

void Button::OnPressUp(Callback callback) {
    on_press_up_ = callback;
    if (button_handle_ && callback) {
        iot_button_register_cb(button_handle_, BUTTON_PRESS_UP, ButtonEventCallback, this);
    }
}

void Button::ButtonEventCallback(void* arg, void* data) {
    auto* self = static_cast<Button*>(arg);
    auto event = iot_button_get_event(self->button_handle_);

    switch (event) {
        case BUTTON_SINGLE_CLICK:
            if (self->on_click_) self->on_click_();
            break;
        case BUTTON_LONG_PRESS_START:
            if (self->on_long_press_) self->on_long_press_();
            break;
        case BUTTON_PRESS_DOWN:
            if (self->on_press_down_) self->on_press_down_();
            break;
        case BUTTON_PRESS_UP:
            if (self->on_press_up_) self->on_press_up_();
            break;
        default:
            break;
    }
}
