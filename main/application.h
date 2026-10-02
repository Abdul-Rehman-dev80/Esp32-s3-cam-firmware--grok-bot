#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "boards/common/board.h"

class Application {
public:
    static Application& GetInstance();

    void Initialize(Board* board);
    void Run();

    DeviceState GetDeviceState() const { return state_; }
    void SetDeviceState(DeviceState state) { state_ = state; }

    void ToggleChatState();
    void StartListening();
    void StopListening();

private:
    Application() = default;

    Board* board_ = nullptr;
    DeviceState state_ = kDeviceStateStarting;
};

#endif // APPLICATION_H_
