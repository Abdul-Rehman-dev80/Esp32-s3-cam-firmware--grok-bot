# Digilog S3CAM OLED Main Component

set(SOURCES
    "main.cc"
    "boards/common/board.cc"
    "boards/common/wifi_board.cc"
    "boards/common/button.cc"
    "boards/common/esp32_camera.cc"
    "audio/codecs/no_audio_codec.cc"
    "display/oled_display.cc"
    "display/lvgl_display/lvgl_display.cc"
    "display/digilog_face_display.cc"
    "led/single_led.cc"
    "mcp_server.cc"
    "application.cc"
    "ota.cc"
    "settings.cc"
    "assets.cc"
)

set(INCLUDE_DIRS
    "."
    "boards/common"
    "boards/digilog-s3cam-oled"
    "display"
    "display/lvgl_display"
    "audio"
    "audio/codecs"
    "led"
)

# Board-specific sources
set(BOARD_DIR "digilog-s3cam-oled")
file(GLOB BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}/*.c
)
list(APPEND SOURCES ${BOARD_SOURCES})

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    WHOLE_ARCHIVE
    PRIV_REQUIRES
        esp_pm
        esp_psram
        esp_netif
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        esp_driver_ledc
        esp_app_format
        app_update
        spi_flash
        mbedtls
        bt
        nvs_flash
)

# Board definitions
target_compile_definitions(${COMPONENT_LIB}
    PRIVATE BOARD_TYPE=\"digilog-s3cam-oled\"
    PRIVATE BOARD_NAME=\"digilog-s3cam-oled\"
    PRIVATE BOARD_MANUFACTURER=\"digilog\"
)
