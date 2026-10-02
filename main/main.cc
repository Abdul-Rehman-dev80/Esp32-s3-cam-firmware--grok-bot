#include "application.h"
#include "boards/digilog-s3cam-oled/digilog_s3cam_oled_board.h"

#include <esp_log.h>
#include <nvs_flash.h>

static const char* TAG = "main";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Digilog S3CAM OLED Desk Robot starting...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create board
    auto* board = new DigilogS3CamOledBoard();

    // Initialize and run application
    auto& app = Application::GetInstance();
    app.Initialize(board);
    app.Run();

    ESP_LOGI(TAG, "Application started successfully");

    // Main loop handled by FreeRTOS tasks
}
