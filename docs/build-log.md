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

Next: read temperature, humidity and pressure from the ENV II.
