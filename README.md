# Digilog ESP32-S3-CAM OLED Desk Robot Firmware

A Xiaozhi-compatible firmware port for the Digilog ESP32-S3-CAM desk robot with OLED face display and dual pan/tilt servos.

## Overview

This project implements a custom board (`digilog-s3cam-oled`) for the Xiaozhi ESP32 voice assistant platform. It's designed for a desk robot with:

- **ESP32-S3** with 16MB flash and PSRAM
- **0.96" SSD1306 OLED** display (128x64, I2C) with animated robot face
- **OV2640 camera** (onboard ESP32-S3-CAM module)
- **INMP441 I2S microphone**
- **MAX98357A I2S amplifier**
- **Dual SG90 servos** for pan (yaw) and tilt (pitch) control

## Hardware Pinout

See [PINOUT.md](PINOUT.md) for the complete GPIO mapping.

### Quick Reference

| Function | GPIO |
|----------|------|
| OLED SDA | 41 |
| OLED SCL | 42 |
| Mic WS | 1 |
| Mic SCK | 2 |
| Mic SD | 14 |
| Amp DIN | 39 |
| Amp BCLK | 40 |
| Amp LRC | 21 |
| Servo Yaw (Base) | 47 |
| Servo Pitch (Head) | 45 |

Camera pins follow the standard ESP32-S3-CAM module pinout (see `config.h`).

## Features

### MCP Tools (Voice/AI Control)

The robot exposes these MCP tools for AI assistant control:

**Base/Yaw Servo:**
- `self.base.set_angle` - Set absolute angle (0-180, 90=center)
- `self.base.turn_left` - Turn left by degrees
- `self.base.turn_right` - Turn right by degrees
- `self.base.center` - Center to 90°

**Head/Pitch Servo:**
- `self.head.set_angle` - Set absolute angle (0-180, 90=level)
- `self.head.look_up` - Tilt up by degrees
- `self.head.look_down` - Tilt down by degrees
- `self.head.center` - Center to 90°

**Camera:**
- `self.camera.take_photo` - Capture JPEG image

### DigilogFace Display

The OLED shows an animated robot face with emotions:
- Neutral, Happy, Sad, Thinking, Surprised, Angry, Confused, Winking, Sleeping, Listening, Speaking

Eyes and mouth animate based on device state and AI responses.

## Building

### Prerequisites

- ESP-IDF v5.3+ (v6.1 recommended for full compatibility)
- Python 3.8+

### Setup

```bash
# Clone the repository
git clone https://github.com/Abdul-Rehman-dev80/Esp32-s3-cam-firmware--grok-bot.git
cd Esp32-s3-cam-firmware--grok-bot

# Set up ESP-IDF environment
. $IDF_PATH/export.sh

# Set target to ESP32-S3
idf.py set-target esp32s3

# Configure (optional - defaults are set)
idf.py menuconfig

# Build
idf.py build
```

### Configuration Options

In `menuconfig` under "Digilog Desk Robot":

- **OLED Display Type**: SSD1306 128x32, 128x64, or SH1106 128x64
- **Servo Default Angles**: Center positions for yaw/pitch
- **DigilogFace Enable**: Toggle animated face vs text UI

## Flashing

### Flash Addresses (16MB Flash)

| Image | Address |
|-------|---------|
| bootloader | 0x0 |
| partition table | 0x8000 |
| ota_data | 0xD000 |
| app (ota_0) | 0x20000 |
| assets | 0x800000 |

### Using idf.py

```bash
# Flash everything
idf.py -p /dev/ttyUSB0 flash

# Flash app only (preserves NVS/WiFi config)
idf.py -p /dev/ttyUSB0 app-flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor
```

### Using esptool

```bash
# Full flash
python -m esptool --chip esp32s3 -p /dev/ttyUSB0 -b 115200 \
    --before default-reset --after hard-reset \
    write_flash --flash_mode dio --flash_freq 40m --flash_size 16MB \
    0x0 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0xd000 build/ota_data_initial.bin \
    0x20000 build/digilog-s3cam-oled.bin
```

### Download Mode

1. Plug in USB-C
2. Hold **BOOT** button
3. Tap **RST** button, then release **BOOT**
4. Board is now in download mode

## Partition Table

```
# Name,   Type, SubType,  Offset,   Size
nvs,      data, nvs,      0x9000,   16K
otadata,  data, ota,      0xD000,   8K
phy_init, data, phy,      0xF000,   4K
ota_0,    app,  ota_0,    0x20000,  4032K
ota_1,    app,  ota_1,    0x410000, 4032K
assets,   data, spiffs,   0x800000, 8192K
```

Supports dual OTA (A/B updates) with 8MB assets partition for wake word models and fonts.

## Project Structure

```
├── CMakeLists.txt
├── sdkconfig.defaults
├── sdkconfig.defaults.esp32s3
├── partitions/
│   └── digilog_16mb.csv
├── main/
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   ├── Kconfig.projbuild
│   ├── main.cc
│   ├── application.cc/h
│   ├── mcp_server.cc/h
│   ├── boards/
│   │   ├── common/
│   │   │   ├── board.h
│   │   │   ├── wifi_board.cc/h
│   │   │   ├── button.cc/h
│   │   │   └── esp32_camera.cc/h
│   │   └── digilog-s3cam-oled/
│   │       ├── config.h
│   │       ├── config.json
│   │       ├── digilog_s3cam_oled_board.cc/h
│   │       └── digilog_face_display.cc/h
│   ├── display/
│   │   └── oled_display.cc/h
│   ├── audio/codecs/
│   │   └── no_audio_codec.cc/h
│   └── led/
│       └── single_led.cc/h
├── PINOUT.md
├── RECOVERY_REPORT.md
└── FLASH-GUIDE.md
```

## Known Gaps / Hardware Bring-up Notes

1. **Camera pins**: Based on bread-compact-wifi-s3cam upstream reference. If your ESP32-S3-CAM module differs, update `config.h`.

2. **OLED I2C address**: Defaults to 0x3C. Some modules use 0x3D - update `DISPLAY_I2C_ADDR` if needed.

3. **Servo calibration**: SG90 pulse widths (500-2500µs) are typical values. Adjust `SERVO_MIN_PULSE_US` / `SERVO_MAX_PULSE_US` in `config.h` for your servos.

4. **External servo power**: SG90 servos should be powered from external 5V, not the ESP32's 3.3V rail. Only signal wires connect to GPIO47/45.

5. **Assets partition**: Wake word models and fonts must be flashed to 0x800000. Use Xiaozhi assets build tools or the recovered `generated_assets.bin`.

6. **WiFi provisioning**: First boot creates a WiFi hotspot for configuration. Connect and browse to http://192.168.4.1

## Recovery Information

See [RECOVERY_REPORT.md](RECOVERY_REPORT.md) for reverse-engineering notes from the original Digilog V2.48 firmware.

## License

This is a clean-room reimplementation based on the public Xiaozhi ESP32 project architecture. Original Digilog firmware was proprietary; this port does not contain any proprietary code.

## Credits

- [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) - Base voice assistant platform
- Digilog desk robot hardware design (original vendor)
