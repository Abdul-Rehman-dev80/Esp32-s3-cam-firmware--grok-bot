#include "wifi_board.h"
#include <esp_log.h>
#include <esp_wifi.h>

static const char* TAG = "WifiBoard";

WifiBoard::WifiBoard() {
    ESP_LOGI(TAG, "WifiBoard created");
}

WifiBoard::~WifiBoard() {
    if (connect_timer_) {
        esp_timer_stop(connect_timer_);
        esp_timer_delete(connect_timer_);
    }
}

std::string WifiBoard::GetBoardType() {
    return "wifi";
}

void WifiBoard::StartNetwork() {
    ESP_LOGI(TAG, "Starting WiFi network");
    TryWifiConnect();
}

NetworkInterface* WifiBoard::GetNetwork() {
    return nullptr;  // Implemented by wifi-connect component
}

void WifiBoard::SetNetworkEventCallback(NetworkEventCallback callback) {
    network_event_callback_ = callback;
}

const char* WifiBoard::GetNetworkStateIcon() {
    return "wifi";
}

void WifiBoard::SetPowerSaveLevel(PowerSaveLevel level) {
    wifi_ps_type_t ps_type = WIFI_PS_NONE;
    switch (level) {
        case PowerSaveLevel::kLight:
            ps_type = WIFI_PS_MIN_MODEM;
            break;
        case PowerSaveLevel::kModem:
            ps_type = WIFI_PS_MAX_MODEM;
            break;
        default:
            ps_type = WIFI_PS_NONE;
            break;
    }
    esp_wifi_set_ps(ps_type);
}

std::string WifiBoard::GetDeviceStatusJson() {
    return "{}";
}

void WifiBoard::EnterWifiConfigMode() {
    ESP_LOGI(TAG, "Entering WiFi config mode");
    in_config_mode_ = true;
    StartWifiConfigMode();
}

bool WifiBoard::IsInWifiConfigMode() const {
    return in_config_mode_;
}

std::string WifiBoard::GetBoardJson() {
    return "{}";
}

void WifiBoard::OnNetworkEvent(NetworkEvent event, const std::string& data) {
    if (network_event_callback_) {
        network_event_callback_(event, data);
    }
}

void WifiBoard::TryWifiConnect() {
    ESP_LOGI(TAG, "Attempting WiFi connection");
    // WiFi connection handled by esp-wifi-connect component
}

void WifiBoard::StartWifiConfigMode() {
    ESP_LOGI(TAG, "Starting WiFi config AP mode");
    // Config portal handled by esp-wifi-connect component
}

void WifiBoard::OnWifiConnectTimeout(void* arg) {
    auto* self = static_cast<WifiBoard*>(arg);
    ESP_LOGW(TAG, "WiFi connection timeout");
    self->EnterWifiConfigMode();
}
