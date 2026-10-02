#ifndef ESP32_CAMERA_H_
#define ESP32_CAMERA_H_

#include <esp_camera.h>
#include <string>
#include <vector>

class Camera {
public:
    virtual ~Camera() = default;
    virtual bool TakePhoto(std::vector<uint8_t>& jpeg_data) = 0;
    virtual bool IsAvailable() const = 0;
};

class Esp32Camera : public Camera {
public:
    Esp32Camera(const camera_config_t& config);
    virtual ~Esp32Camera();

    virtual bool TakePhoto(std::vector<uint8_t>& jpeg_data) override;
    virtual bool IsAvailable() const override { return initialized_; }

    // Get raw frame buffer
    camera_fb_t* CaptureFrame();
    void ReleaseFrame(camera_fb_t* fb);

private:
    bool initialized_ = false;
};

#endif // ESP32_CAMERA_H_
