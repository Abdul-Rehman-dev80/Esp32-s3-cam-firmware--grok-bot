# Digilog ESP32-S3-CAM Face Dual-Servo — Firmware Recovery Report

**Analyzed:** 2026-10-02 (Asia/Karachi)  
**Inputs:** `/workspace/digilog-v248/digilog-face-dual-servo-*.bin` + `Digilog-FLASH-GUIDE.txt`  
**Goal:** Recover enough identity, layout, and stack evidence to redesign + publish a GitHub project (analysis only; no repo created).

---

## Executive summary

| Item | Finding | Confidence |
|------|---------|------------|
| **Base project** | [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) (`project_name=xiaozhi`) | **High** |
| **Custom board ID** | `digilog-s3cam-oled` (private / not on public GitHub) | **High** |
| **App version** | `2.5.0` (folder nickname “V2.48”; binary `esp_app_desc` says **2.5.0**) | **High** |
| **ESP-IDF** | **v6.1** | **High** |
| **Build time** | `Sep 18 2026 17:10:35` | **High** |
| **Display** | SSD1306 OLED over I2C + custom **DigilogFace** LVGL UI (robot face / emotions) | **High** |
| **Servos** | Dual SG90 via LEDC @ 50 Hz: **yaw (base)** + **pitch (head)**, center 90°, range 0–180 | **High** |
| **Camera** | OV2640 / `Esp32Camera` + MCP `self.camera.take_photo` | **High** |
| **Audio** | `NoAudioCodec` (I2S mic/amp, **no** ES8311-class codec in binary) | **High** |
| **GPIO pin numbers** | **Not recoverable from strings** (logged as `GPIO%d`) | **Unknown** |
| **Public Digilog fork** | **None found** (GitHub code/repo search for `digilog-s3cam-oled` / `DigilogFace` = 0 hits) | **High** |

**Bottom line:** Treat this as a **custom board port** of Xiaozhi 2.5.0 on IDF 6.1. Rebuild by forking upstream and adding `main/boards/digilog-s3cam-oled/` (sources named in the binary). Pins must come from schematic, PCB silk, or live probing / UART logs — **do not invent them**.

---

## 1. What is recoverable vs not

### Recoverable (strong evidence)

- Project lineage: Xiaozhi (`xiaozhi` app name, `78__esp-*` managed components, OTA URL, MCP tool schema).
- Board directory and source filenames:
  - `./main/boards/digilog-s3cam-oled/digilog_s3cam_oled_board.cc`
  - `./main/boards/digilog-s3cam-oled/digilog_face_display.cc`
- Classes: `DigilogS3CamOledBoard`, `DigilogFaceDisplay`, `DigilogFace`.
- Feature set: Wi-Fi, MQTT/WebSocket protocols, OTA/assets download, wake word (ESP-SR), camera, dual-servo MCP tools, OLED face emotions.
- Flash / partition map (parsed from partition-table.bin).
- Assets package contents: wake-word models + Noto font + `index.json` (no face bitmaps in this assets blob — face UI is compiled into the app).
- Compile metadata: version, IDF, date/time.

### Partially recoverable

- SSD1306 I2C address: **not printed in strings**; upstream Xiaozhi OLED boards use **0x3C** — **likely but unconfirmed**.
- OLED geometry: flash guide says **0.96"**; upstream OLED boards use 128×32 or 128×64 — **height unknown** without probing/`menuconfig`.
- Audio I2S pin map: `NoAudioCodecSimplex` / `Duplex` both linked — which mode Digilog uses is not string-obvious.
- Camera DVP pins: typical ESP32-S3-CAM / OV2640 layout *may* match upstream `bread-compact-wifi-s3cam`, but Digilog’s OLED+dual-servo wiring can differ — **do not copy blindly**.

### Not recoverable from these binaries alone

- Exact GPIO for servos, OLED SDA/SCL, I2S, buttons, LED, camera.
- Original private git history / Digilog vendor repo (not public).
- Full face animation bitmaps / LVGL object tree (compiled; no separate face assets in `generated_assets.bin`).
- Exact `kAnimPeriodMs`, LEDC channel indices, duty calc (constants, not strings).
- NVS Wi-Fi credentials / activation tokens (not in these image files).

---

## 2. Partition map

Parsed from `digilog-face-dual-servo-partition-table.bin` (ESP-IDF binary table, magic `0x50AA`, MD5 present).

| Name | Type | Subtype | Offset | Size |
|------|------|---------|--------|------|
| `nvs` | data | nvs | `0x9000` | 16 KB |
| `otadata` | data | ota | `0xD000` | 8 KB |
| `phy_init` | data | phy | `0xF000` | 4 KB |
| `ota_0` | app | ota_0 | `0x20000` | 4032 KB |
| `ota_1` | app | ota_1 | `0x410000` | 4032 KB |
| `assets` | data | spiffs\* | `0x800000` | 8192 KB |

\*Subtype byte is SPIFFS in the table; Xiaozhi treats this as the **assets** mmap partition (models/fonts), not a classic SPIFFS FS for app files.

**Flash guide alignment (16 MB):**

| Image | Address |
|-------|---------|
| bootloader | `0x0` |
| partition table | `0x8000` |
| ota_data | `0xD000` |
| app | `0x20000` |
| assets | `0x800000` |

App image size on disk ≈ 2.54 MB (fits in 4032 KB OTA slot). Assets file ≈ 548 KB (fits in 8 MB assets slot).

**Bootloader note:** Standard ESP32-S3 IDF bootloader strings; no Digilog-specific branding required for redesign.

---

## 3. Key strings / stack evidence

### Identity

- MCP `serverInfo.name` = **`digilog-s3cam-oled`**
- `esp_app_desc`: `project_name=xiaozhi`, `version=2.5.0`, `idf_ver=v6.1`, `Sep 18 2026 17:10:35`
- OTA: `https://api.tenclass.net/xiaozhi/ota/` (official Xiaozhi cloud)
- Wi-Fi config portal (embedded HTML + `Hotspot:` / `Config URL:` / `http://192.168.4.1`)

### Source tree paths (RTTI / assert paths)

```
./main/main.cc
./main/settings.cc
./main/boards/common/button.cc
./main/boards/digilog-s3cam-oled/digilog_s3cam_oled_board.cc
./main/boards/digilog-s3cam-oled/digilog_face_display.cc
./main/display/lvgl_display/lvgl_display.cc
./main/audio/codecs/no_audio_codec.cc
./main/audio/engines/afe_audio_engine.cc
./main/audio/wake_words/custom_wake_word.cc
./main/led/single_led.cc
./managed_components/78__esp-wifi-connect/...
./managed_components/78__esp-ml307/...
./managed_components/espressif__esp-sr/...
./managed_components/espressif__esp_lvgl_port/.../lvgl9/...
```

### Display / UI

- `InitializeDisplayI2c`, `InitializeSsd1306Display`, `esp_lcd_new_panel_ssd1306`
- `DigilogFaceDisplay::SetupUI`, `digilog_face`, `anim_timer_`, `kAnimPeriodMs`
- Emotions wired to face: `happy`, `laughing`, `funny`, `loving`, `confident`, `delicious`, `winking`, `relaxed`, `sad`, `crying`, `embarrassed`, `angry`, `surprise`/`surprised`, `shocked`, `thinking`, `confused`
- LVGL 9 via `esp_lvgl_port` (`taskLVGL`, `Initialize LVGL`, `Adding OLED display`)
- Also contains generic `OledDisplay` / `LvglDisplay` / `EmojiCollection` symbols (Xiaozhi display stack)

### Servo / MCP (Digilog-specific)

Log line pattern:

`SG90 yaw on GPIO%d + pitch on GPIO%d @ 50 Hz (center 90°)`

| MCP tool | Role |
|----------|------|
| `self.base.set_angle` | Yaw absolute 0–180 (90 = center/forward) |
| `self.base.turn_left` / `turn_right` | Relative yaw |
| `self.base.center` | Yaw → 90 |
| `self.head.set_angle` | Pitch absolute 0–180 |
| `self.head.look_up` / `look_down` | Relative pitch |
| `self.head.center` | Pitch → 90 |

Implementation: `SetServo1Angle` / `SetServo2Angle`, `InitializeServos`, `ledc_timer_config` + two `ledc_channel_config` (Servo1 = yaw/base, Servo2 = pitch/head).

### Camera / audio / SR

- `Esp32Camera`, `OV2640`, `self.camera.take_photo`, multipart `camera.jpg`
- `NoAudioCodec` / `NoAudioCodecSimplex` / `NoAudioCodecDuplex`
- ESP-SR AFE + wake word; assets model **`wn9_nihaoxiaozhi_tts`** / wakenet9 family
- Standard Xiaozhi MCP: `self.get_system_info`, `self.reboot`, `self.upgrade_firmware`, `self.screen.*`, `self.audio_speaker.set_volume`, `self.assets.set_download_url`, …

### Assets (`generated_assets.bin`)

Header: file count **3**. Embedded `index.json`:

```json
{
  "version": 1,
  "srmodels": "srmodels.bin",
  "text_font": "font_noto_sans_common_14_1.bin",
  "text_font_meta": {
    "charset": "common",
    "size": 14,
    "bpp": 1,
    "bundle": "noto-v1"
  }
}
```

Files: `font_noto_sans_common_14_1.bin`, `srmodels.bin`, `index.json`.  
**No** PNG/GIF face packs in this assets image — robot face is **in-app** (`DigilogFaceDisplay`).

---

## 4. Likely hardware / software stack

```
ESP32-S3 (16 MB flash) + PSRAM (assumed for camera; confirm on board)
├── Wi-Fi → Xiaozhi MQTT/WebSocket (api.tenclass.net OTA)
├── I2C → SSD1306 0.96" OLED → DigilogFace (LVGL9)
├── I2S → mic + amp via NoAudioCodec (no ES83xx in binary)
├── LEDC @ 50 Hz → SG90 #1 yaw (base) + SG90 #2 pitch (head)
├── DVP camera → OV2640 (Esp32Camera)
├── BOOT button (typical GPIO0; unconfirmed)
├── Optional SingleLed / RMT LED strip symbols present (may be unused / GPIO_NUM_NC)
└── assets @ 0x800000 → ESP-SR models + 14px font
```

### GPIO / pins — status

| Function | Status |
|----------|--------|
| Servo yaw / pitch GPIOs | **UNKNOWN** (only `GPIO%d` in log format) |
| OLED SDA / SCL | **UNKNOWN** |
| SSD1306 addr | **Likely 0x3C** (upstream default); not proven in this binary |
| I2S mic/spk | **UNKNOWN** |
| Camera DVP / SCCB | **UNKNOWN** (ESP32-S3-CAM “classic” pinouts are a starting guess only) |
| LED / extra buttons | **UNKNOWN** |

**Inference rule used:** pin numbers are only reported if found in strings or solidly inferred. None were found as literals.

---

## 5. Closest public base & redesign starting point

### Upstream

1. **Primary fork target:** `https://github.com/78/xiaozhi-esp32`  
   - Prefer a tag/commit near **app 2.5.0** built with **ESP-IDF 6.1** (matches `esp_app_desc`).  
   - Current mainline docs mention IDF 6.0.2+; this binary is explicitly **v6.1**.

2. **Closest upstream boards to copy patterns from (not Digilog’s private pins):**

   | Upstream board | Steal from it |
   |----------------|---------------|
   | `main/boards/bread-compact-wifi/` | SSD1306 + I2C init, `OledDisplay`, `NoAudioCodec`, Wi-Fi board skeleton |
   | `main/boards/bread-compact-wifi-s3cam/` | OV2640 / `Esp32Camera` bring-up pattern on ESP32-S3-CAM |
   | Docs: `docs/custom-board.md` | Official custom-board checklist |

3. **Not the same product:** public “AI EYE” forks (SmartArduino / dual round GC9A01 eyes) are different hardware (dual color LCDs). Digilog here is **one 0.96" SSD1306 + dual pan/tilt servos**.

4. **Digilog-specific code is proprietary to this binary** — reimplement `digilog_s3cam_oled_board.cc` + `digilog_face_display.cc` against Xiaozhi board APIs using behavior recovered above.

### Recommended rebuild approach

1. Clone `78/xiaozhi-esp32`, checkout version ≈ 2.5.0, `idf.py set-target esp32s3`.
2. Add board `main/boards/digilog-s3cam-oled/` per custom-board guide (`config.h`, `config.json`, `*_board.cc`, Kconfig entry).
3. Display: SSD1306 I2C + **custom** `DigilogFaceDisplay` (emotion → face frames / LVGL objects). Optionally start with stock `OledDisplay` text UI, then replace with face UI.
4. Servos: LEDC 50 Hz, two channels; register MCP tools matching the names above (`self.base.*`, `self.head.*`).
5. Camera: port from `bread-compact-wifi-s3cam` after pin audit.
6. Audio: `NoAudioCodecSimplex` (most common for separate mic/amp) unless duplex is confirmed.
7. Partition CSV: mirror the table in §2 (dual OTA + 8 MB assets) for 16 MB flash.
8. Assets: generate with Xiaozhi assets pipeline (`srmodels` + font) or reuse extracted `generated_assets.bin` for bring-up.
9. **Pin discovery (blocking):** schematic, continuity to headers, or flash a minimal pin-scanner / read UART boot log from a working unit (`SG90 yaw on GPIOX + pitch on GPIOY`).

### Naming note (V2.48 vs 2.5.0)

- Workspace / product nickname: **V2.48** (face dual-servo lineage; related Dual Servo V2.6 / Single V2.14 folders mentioned by user).
- Embedded `esp_app_desc.version`: **2.5.0**.
- Flash guide (2026-09-19) describes a **stable pre-face** text Xiaozhi UI; **this** image is the **face** build (`DigilogFace`). Guide warns wrong face images can reboot-loop — keep partition/assets matched when flashing.

---

## 6. Concrete ESP-IDF project scaffold file list

Suggested tree for a new public redesign repo (fork of Xiaozhi + Digilog board):

```
xiaozhi-esp32/   # or digilog-s3cam-oled/
├── CMakeLists.txt
├── idf_component.yml                 # 78/esp-wifi-connect, 78/esp-ml307, esp-sr, esp_lvgl_port, …
├── sdkconfig.defaults
├── sdkconfig.defaults.esp32s3
├── partitions/                       # or root partitions.csv
│   └── digilog_16mb.csv              # nvs/otadata/phy/ota_0/ota_1/assets as in §2
├── docs/
│   ├── HARDWARE.md                   # pin table TBD — fill after probe
│   ├── FLASH.md                      # from Digilog-FLASH-GUIDE
│   └── RECOVERY_REPORT.md            # this file
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig.projbuild             # BOARD_TYPE_DIGILOG_S3CAM_OLED
│   ├── main.cc
│   ├── boards/
│   │   ├── digilog-s3cam-oled/
│   │   │   ├── config.h              # ALL pins — UNKNOWN until measured
│   │   │   ├── config.json           # release.py / build variant
│   │   │   ├── digilog_s3cam_oled_board.cc
│   │   │   ├── digilog_face_display.cc / .h
│   │   │   └── README.md
│   │   └── common/                   # from upstream
│   ├── display/                      # upstream lvgl/oled + DigilogFace
│   ├── audio/                        # NoAudioCodec, AFE, wake word
│   ├── led/
│   └── ...
├── managed_components/               # resolved by IDF component manager
└── scripts/
    └── release / assets build notes
```

**Minimum new Digilog files to author:**

1. `config.h` — pins, `DISPLAY_WIDTH/HEIGHT`, servo GPIOs, I2S, camera macros  
2. `digilog_s3cam_oled_board.cc` — `WifiBoard` subclass: I2C OLED, servos, camera, MCP registration, audio codec  
3. `digilog_face_display.cc` — emotion → animated face on SSD1306  
4. Kconfig + CMake board selection  
5. `partitions` CSV for 16 MB  
6. Hardware doc with measured pins  

**Do not invent pin numbers in `config.h` for GitHub until verified.**

---

## 7. Companion artifact

Filtered unique strings: [`extracted_strings.txt`](./extracted_strings.txt)

---

## 8. Next build steps (practical order)

1. Fork `78/xiaozhi-esp32` @ ~2.5.0 / IDF 6.1.  
2. Probe or obtain Digilog pin map (UART log from working firmware is fastest for servo GPIOs).  
3. Scaffold `digilog-s3cam-oled` from `bread-compact-wifi` + camera bits from `bread-compact-wifi-s3cam`.  
4. Bring up **text OLED + Wi-Fi + wake word** first (match “stable” guide behavior).  
5. Add dual-servo MCP tools; verify with `self.base.*` / `self.head.*`.  
6. Add camera photo MCP.  
7. Replace stock OLED UI with DigilogFace emotions.  
8. Package assets; flash per guide addresses; publish redesign repo **without** claiming upstream Digilog sources you do not have.

---

*Report generated from binary forensics only. No GitHub repository was created.*
