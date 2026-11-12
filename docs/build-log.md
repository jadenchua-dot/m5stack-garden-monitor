# Build log

Short dated notes on each stage: what I did, what happened, what I fixed. Working code goes in `firmware/`, one folder per milestone. Problems with the Windows setup are in `setup-windows.md`.

## 2026-11-12: Setup

- Installed Arduino IDE 2.x, the M5Stack board package, M5Unified and the CP210x driver.
- Uploaded `firmware/hello_fire`. The screen and the three buttons work.
- Problems and fixes: see `setup-windows.md`.

## 2026-11-12: Stage 1, I2C scan

- Sketch: `firmware/i2c_scan`.
- Result on Port A through the HUB unit: 0x29 (ToF), 0x44 and 0x76 (ENV II). The scan also lists 0x10, 0x68 and 0x75, which are the Fire's own internal devices.
- Verified, and recorded in `wiring.md`.

## 2026-11-13: Stage 2, ENV II readings

- Sketch: `firmware/env_ii_test`. Installed the Adafruit BMP280 library (3.0.0). The SHT30 is read directly over I2C and the BMP280 through the library.
- Result in Serial Monitor at 115200 baud: 23.7 C, 57.6 % humidity, 1007.5 hPa, steady from reading to reading. No problems.
- The readings print to Serial Monitor only. Showing them on the Fire's screen is a later step.

Next: calibrate the Earth unit (dry and wet values).
