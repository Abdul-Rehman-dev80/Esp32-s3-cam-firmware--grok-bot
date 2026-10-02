#ifndef WIFI_BOARD_H_
#define WIFI_BOARD_H_

#include "board.h"
#include <string>
#include <esp_timer.h>

class WifiBoard : public Board {
public:
    WifiBoard();
    virtual ~WifiBoard();

    virtual std::string GetBoardType() override;
    virtual void StartNetwork() override;
    virtual NetworkInterface* GetNetwork() override;
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) override;
    virtual const char* GetNetworkStateIcon() override;
    virtual void SetPowerSaveLevel(PowerSaveLevel level) override;
    virtual std::string GetDeviceStatusJson() override;

    void EnterWifiConfigMode();
    bool IsInWifiConfigMode() const;

protected:
    virtual std::string GetBoardJson() override;
    void OnNetworkEvent(NetworkEvent event, const std::string& data = "");
    void TryWifiConnect();
    void StartWifiConfigMode();

    static void OnWifiConnectTimeout(void* arg);

    esp_timer_handle_t connect_timer_ = nullptr;
    bool in_config_mode_ = false;
    NetworkEventCallback network_event_callback_ = nullptr;
};

#endif // WIFI_BOARD_H_
