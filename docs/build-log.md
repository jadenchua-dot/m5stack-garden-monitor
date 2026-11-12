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

## 2026-11-13: Stage 3, Earth unit (in progress)

- Sketch: `firmware/earth_calibration`. The Earth unit is on Port B and the raw value shows on the Fire's screen.
- Dry in air: 4095, the maximum the analog input can read.
- Wet tissue wrapped around the prongs: the value falls steadily. It read 2270 when I noted it.
- Problem: the first upload failed with "port is busy", and the Fire then boot-looped. Closing the Serial Monitor tab and replugging fixed it. Details in `setup-windows.md`, Problem 5.
- Prongs in a cup of water, up to the line on the board: the value hovers around 2010 to 2030, so about 2020.
- Calibration so far: dry 4095, water about 2020. The wet tissue reading (2270) sits between the two, as expected.
- Still to do: a reading in damp soil, to set the watering threshold.

- No soil on hand, so I set a provisional dry threshold at raw 3300 (about 38% of the dry-to-water range). The pump waters when the reading is above it. It is a guess, based on soil holding less water than a cup, and I will tune it in real soil. The pump cooldown limits the damage if it is off.

## 2026-11-13: Stage 4, servo valve

- Decision: use the servo from the kit as the water valve, instead of a DC motor with a transistor switch. The servo has fewer parts, and the motor's voltage was unknown. The transistor pump design stays in `wiring.md` as an optional upgrade.
- Sketch: `firmware/servo_test`. The servo sweeps between two angles every 1.5 seconds. Works, with the servo signal on G17 (Port C).
- Repo updated to match: README, wiring notes and the data flow diagram now show the servo.

## 2026-11-13: Stage 5, watering logic

- Sketch: `firmware/watering_logic`. The Earth unit is read once a second. Five dry readings in a row (above raw 3300) open the servo valve for 5 seconds, then a 60 second cooldown blocks the next cycle. Button A forces a test cycle. The test times are short on purpose. A real plot needs a cooldown of 30 minutes or more.
- Problem: the Fire shut off after a cycle and then repeated the cycle, so the countdown never finished. A reset wipes the program's memory, so it counted dry reads again and reopened the valve.
- Cause: the servo arm was pushing against the Lego frame. A stalled servo draws far more current, which dropped the power to the Fire.
- Fix: move the arm so it clears the frame at both ends of the swing. I also made the servo move in 1 degree steps and added a reset reason print to Serial Monitor, to help diagnose any future resets.
- Result: works. The valve opens with the sensor in air, stays shut with the sensor in water, and the cooldown counts down.

## 2026-11-13: Stage 6, ToF water level

- Sketch: `firmware/tof_test`. Needs the Adafruit VL53L0X library. The first compile failed because the library was not installed and a stray "a" had been typed before the include line. Installing the library and deleting the "a" fixed it.
- Result: distance readings follow my hand, and "Out of range" appears when nothing is close. "ToF ready" did not show because it prints once at start, before the Serial Monitor opened. It is not a fault.
- Sketch: `firmware/water_level_warning`. Turns the distance into a tank level percentage and shows a red LOW WATER warning below 20%. It uses M5GFX for the screen instead of M5Unified, so the screen code does not touch the I2C bus the ToF uses. Works with no problems.
- The full and empty distances (50 mm and 200 mm) are guesses until the tank is built.

- Problem 6 in `setup-windows.md`: a bigger sketch uploaded from a new window gave a partition table error and a black screen. Slower upload speed, DIO flash mode and a replug fixed it.

## 2026-11-13: Stage 7, everything in one sketch

- Sketch: `firmware/garden_monitor`. Reads the Earth unit, ENV II and ToF, shows them on the screen, and opens the servo valve when the soil stays dry. It skips watering when the tank is low or the air is above 90% humidity. Works with all parts plugged in.
- Problem: the screen blinks when the servo moves. The servo draws a burst of current and the 5 V supply sags. The Fire keeps running.
- Fix: unplugged the PC cable and ran the Fire from its built-in battery. The blink went away, so the laptop USB port was not supplying enough current. The battery is small, so for a longer run the Fire needs a wall charger (1A or more) plugged in, or a separate 5 V supply for the servo.

Next: build the Lego tank and valve arm, then WiFi logging.
