#include "application.h"
#include <esp_log.h>

static const char* TAG = "Application";

Application& Application::GetInstance() {
    static Application instance;
    return instance;
}

void Application::Initialize(Board* board) {
    board_ = board;
    ESP_LOGI(TAG, "Application initialized");
}

void Application::Run() {
    ESP_LOGI(TAG, "Application running");
    state_ = kDeviceStateIdle;
}

void Application::ToggleChatState() {
    switch (state_) {
        case kDeviceStateIdle:
            StartListening();
            break;
        case kDeviceStateListening:
            StopListening();
            break;
        default:
            break;
    }
}

void Application::StartListening() {
    ESP_LOGI(TAG, "Start listening");
    state_ = kDeviceStateListening;
}

void Application::StopListening() {
    ESP_LOGI(TAG, "Stop listening");
    state_ = kDeviceStateIdle;
}
