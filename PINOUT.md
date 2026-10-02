# Digilog ESP32-S3-CAM pinout (user wiring notes, 2026-10-02)

Source: user-provided wiring screenshot for Xiaozhi / Digilog S3-CAM desk robot.

## Display (I2C OLED)
- SDA: GPIO41
- SCL: GPIO42
- VCC: 3.3V, GND: GND
- Driver evidence from firmware: SSD1306 (addr likely 0x3C, unconfirmed in binary)

## Mic (INMP441, I2S)
- WS: GPIO1
- SCK: GPIO2
- SD: GPIO14
- L/R: GND (left channel)
- VDD: 3.3V, GND: GND

## Amp (MAX98357A, I2S)
- DIN: GPIO39
- BCLK: GPIO40
- LRC: GPIO21
- VIN: 5V (or 3.3V if module supports), GND: GND

## Servos (SG90, LEDC 50 Hz)
- Servo1 yaw/base signal: GPIO47
- Servo2 pitch/head signal: GPIO45 (notes labeled "future use"; dual-servo firmware uses second servo for head pitch — treat 45 as pitch unless overridden)
- Power: external 5V supply, not ESP32 3.3V rail
- Logic: 0–180°, center 90° (from firmware MCP)

## Camera
- Onboard ESP32-S3-CAM / OV2640 — no jumpers

## Audio stack note (from firmware RE)
- NoAudioCodec simplex/duplex present; this pinout is classic NoAudioCodecSimplex (separate mic + amp I2S lines)
