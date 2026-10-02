# Digilog ESP32-S3 desk robot — flash guide (stable firmware)

Board: Digilog ESP32-S3-CAM + 0.96" OLED + mic/amp + SG90 servo  
Stable firmware folder: `C:\Users\Mani\Downloads\`  
Chip: **ESP32-S3** | Flash: **16MB** | Port (USB): usually **COM3** (USB Serial Device)

---

## Files to keep (stable)

| File | Role |
|------|------|
| `digilog-servo-stable-merged.bin` | **Easiest** — whole image in one file |
| `digilog-servo-stable-app.bin` | App only |
| `digilog-servo-stable-bootloader.bin` | Bootloader |
| `digilog-servo-stable-partition.bin` | Partition table |
| `digilog-servo-stable-ota_data.bin` | Forces boot from ota_0 |
| `digilog-servo-stable-assets.bin` | Sounds / models / assets |

Do **not** flash old `xz_*.bin`, `digilog-s3cam-oled-merged.bin`, or other cam board merges.

---

## Put the board in download mode

1. Plug the **USB** Type-C (the one that shows as COM3 / USB Serial Device).  
2. Hold **BOOT**.  
3. Tap **RST**, then release **BOOT**.  
4. Board waits for download (OLED may be blank).

---

## Method A — Flash Download Tool (one file) ★ recommended

Tool: **ESP32S3 FLASH DOWNLOAD TOOL** (SPIDownload tab)

1. Open the tool and pick **ESP32-S3** if asked.  
2. Row 1 only:
   - Check the box  
   - Path: `C:\Users\Mani\Downloads\digilog-servo-stable-merged.bin`  
   - Address: **`0x0`**  
3. SPIFlash config:
   - **SPI SPEED:** 40MHz (or 80MHz if it stays stable)  
   - **SPI MODE:** **DIO**  
   - Check **DoNotChgBin**  
4. Download panel:
   - **COM:** COM3 (or whatever COM the USB port is)  
   - **BAUD:** **115200** (safer on this board; 460800 often drops mid-write)  
5. Click **START**. Wait until it finishes green / DONE.  
6. Tap **RST** once (do **not** hold BOOT) to boot.

---

## Method B — Flash Download Tool (separate files)

Use this if you only need to refresh the app, or Method A fails.

Check these rows (paths under Downloads):

| File | Address |
|------|---------|
| `digilog-servo-stable-bootloader.bin` | `0x0` |
| `digilog-servo-stable-partition.bin` | `0x8000` |
| `digilog-servo-stable-ota_data.bin` | `0xd000` |
| `digilog-servo-stable-app.bin` | `0x20000` |
| `digilog-servo-stable-assets.bin` | `0x800000` |

Same SPI MODE **DIO**, **DoNotChgBin**, COM3, baud **115200**, then **START**, then tap **RST**.

---

## Method C — Command line (esptool)

```bat
python -m esptool --chip esp32s3 -p COM3 -b 115200 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-freq 40m --flash-size 16MB 0x0 C:\Users\Mani\Downloads\digilog-servo-stable-merged.bin
```

Or app-only (keeps Wi‑Fi NVS if you don’t erase):

```bat
python -m esptool --chip esp32s3 -p COM3 -b 115200 write-flash --flash-mode dio --flash-freq 40m --flash-size 16MB 0x20000 C:\Users\Mani\Downloads\digilog-servo-stable-app.bin 0xd000 C:\Users\Mani\Downloads\digilog-servo-stable-ota_data.bin
```

---

## After flashing — what “good” looks like

- OLED shows normal **text** Xiaozhi UI (not robot face).  
- Connects to Wi‑Fi, activates, stays running (no reboot loop).  
- Servo MCP tools still available (look left / right / set angle).

If it activates then reboots forever, you may have flashed the wrong/old face image — reflash **`digilog-servo-stable-merged.bin`** at **0x0**.

---

## UART note

If USB mid-flash keeps failing, use the **UART** Type-C after installing the **CH343** driver, pick that new COM port, same addresses/files. Baud can often be higher on UART (e.g. 460800).

---

## Quick checklist

- [ ] File is `digilog-servo-stable-merged.bin` (not another merged-binary)  
- [ ] Address `0x0`  
- [ ] Chip ESP32-S3, mode DIO, DoNotChgBin  
- [ ] COM correct, baud 115200  
- [ ] BOOT+RST into download mode before START  
- [ ] Tap RST only after DONE  

Saved: 2026-09-19 — Digilog stable pre-face + servo build.
