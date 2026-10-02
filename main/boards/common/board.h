#ifndef BOARD_H_
#define BOARD_H_

#include <string>
#include <functional>

// Forward declarations
class Led;
class AudioCodec;
class Display;
class Camera;
class Backlight;
class NetworkInterface;

// Power save levels
enum class PowerSaveLevel {
    kNone,
    kLight,
    kModem,
    kDeep,
};

// Device states
enum DeviceState {
    kDeviceStateStarting,
    kDeviceStateIdle,
    kDeviceStateListening,
    kDeviceStateSpeaking,
    kDeviceStateThinking,
};

// Network events
enum class NetworkEvent {
    kConnecting,
    kConnected,
    kDisconnected,
    kError,
};

using NetworkEventCallback = std::function<void(NetworkEvent, const std::string&)>;

class Board {
public:
    virtual ~Board() = default;

    // Get board information
    virtual std::string GetBoardType() { return "unknown"; }

    // Hardware interfaces
    virtual Led* GetLed() { return nullptr; }
    virtual AudioCodec* GetAudioCodec() { return nullptr; }
    virtual Display* GetDisplay() { return nullptr; }
    virtual Camera* GetCamera() { return nullptr; }
    virtual Backlight* GetBacklight() { return nullptr; }
    virtual NetworkInterface* GetNetwork() { return nullptr; }

    // Network management
    virtual void StartNetwork() {}
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) {}
    virtual const char* GetNetworkStateIcon() { return ""; }

    // Power management
    virtual void SetPowerSaveLevel(PowerSaveLevel level) {}

    // Status
    virtual std::string GetDeviceStatusJson() { return "{}"; }

protected:
    virtual std::string GetBoardJson() { return "{}"; }
};

// Board factory macro
#define DECLARE_BOARD(BoardClass) \
    static BoardClass* CreateBoard() { \
        static BoardClass instance; \
        return &instance; \
    } \
    __attribute__((constructor)) static void RegisterBoard() { \
        /* Board auto-registration */ \
    }

// No display fallback
class NoDisplay : public Display {
public:
    virtual ~NoDisplay() = default;
};

#endif // BOARD_H_
