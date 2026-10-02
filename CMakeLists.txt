# Digilog ESP32-S3-CAM OLED Desk Robot Firmware
# Based on xiaozhi-esp32 architecture
# Target: ESP32-S3 with 16MB flash
cmake_minimum_required(VERSION 3.16)

add_compile_options(-Wno-missing-field-initializers)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)

set(PROJECT_VER "1.0.0")
project(digilog-s3cam-oled)
