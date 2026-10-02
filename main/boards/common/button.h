#ifndef BUTTON_H_
#define BUTTON_H_

#include <driver/gpio.h>
#include <functional>

class Button {
public:
    using Callback = std::function<void()>;

    Button(gpio_num_t gpio, bool active_low = true);
    ~Button();

    void OnClick(Callback callback);
    void OnLongPress(Callback callback);
    void OnPressDown(Callback callback);
    void OnPressUp(Callback callback);

private:
    gpio_num_t gpio_;
    bool active_low_;
    Callback on_click_;
    Callback on_long_press_;
    Callback on_press_down_;
    Callback on_press_up_;

    void* button_handle_ = nullptr;

    static void ButtonEventCallback(void* arg, void* data);
};

#endif // BUTTON_H_
