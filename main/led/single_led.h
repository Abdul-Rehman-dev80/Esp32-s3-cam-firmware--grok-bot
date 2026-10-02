#ifndef SINGLE_LED_H_
#define SINGLE_LED_H_

#include <driver/gpio.h>

class Led {
public:
    virtual ~Led() = default;
    virtual void SetState(bool on) = 0;
    virtual void SetBrightness(int brightness) = 0;
    virtual void SetColor(uint32_t color) = 0;
    virtual void Blink(int on_ms, int off_ms, int count = -1) = 0;
};

class SingleLed : public Led {
public:
    SingleLed(gpio_num_t gpio);
    virtual ~SingleLed() = default;

    virtual void SetState(bool on) override;
    virtual void SetBrightness(int brightness) override;
    virtual void SetColor(uint32_t color) override;
    virtual void Blink(int on_ms, int off_ms, int count = -1) override;

private:
    gpio_num_t gpio_;
    bool state_ = false;
};

#endif // SINGLE_LED_H_
