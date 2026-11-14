# Windows setup notes

Setting up the M5Stack Fire with Arduino IDE 2.x on Windows. These are the steps that worked, and the problems I hit along the way.

## Steps that worked

1. Install Arduino IDE 2.x from arduino.cc.
2. Open Preferences (Ctrl + ,) and add this URL under Additional boards manager URLs: `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`
3. In Boards Manager, install M5Stack by M5Stack official (version 3.3.10).
4. In Library Manager, install M5Unified with its dependencies.
5. Install the CP210x USB driver (see Problem 2).
6. Select the M5Fire board and the Fire's COM port, then press Upload.

## Problems and fixes

### Problem 1: Two COM ports, both "Unknown", and neither disappears when I unplug the Fire

The IDE's port list showed COM3 and COM4 as "Unknown". Unplugging the Fire changed nothing.

Cause: both ports were Bluetooth links ("Standard Serial over Bluetooth link" in Device Manager), not the Fire.

Fix: ignore COM3 and COM4. The Fire only shows up after its driver is installed.

### Problem 2: Fire not recognized, driver missing

In Device Manager the Fire appeared under Other devices as "CP2104 USB to UART Bridge Controller" with a yellow warning icon. Windows had no driver for it.

Fix:

1. On silabs.com, open the CP210x USB to UART Bridge VCP Drivers page and go to the Downloads tab.
2. Download "CP210x Windows Drivers" (v6.7.6). The newer "CP210x Universal Windows Driver" has no installer, so it needs a manual install through Device Manager.
3. Extract the zip, run `CP210xVCPInstaller_x64.exe`, click Next and accept the license.
4. With the Fire plugged in, the device moves to Ports (COM & LPT) as "Silicon Labs CP210x USB to UART Bridge" with a COM number. It then appears in the Arduino IDE port list.

If the warning icon stays: right-click the device in Device Manager, choose Update driver, then Browse my computer for drivers, and point it at the unzipped folder.

### Problem 3: Arduino IDE says "Unknown" for the board

The IDE can't identify M5Stack boards by itself.

Fix: choose the board manually. Use Select other board and port, search for M5Fire, select it, and pick the Fire's COM port. The bottom right of the IDE then shows "M5Fire on COMx".

### Problem 4: "OpenOCD: GDB Server Quit Unexpectedly"

Error text: `unable to open ftdi device with description '*', serial '*' at bus location '*'`

Cause: I clicked the Debug button (the third icon, with the bug) instead of Upload. Debug looks for a JTAG debugger, which the Fire doesn't have.

Fix: close the gdb-server tab and click Upload, the right arrow (or press Ctrl + U).

### Problem 5: Upload fails with "Could not open COM5, the port is busy", then a boot loop

Error text: `A fatal error occurred: Could not open COM5, the port is busy or doesn't exist` with `PermissionError(13, 'Access is denied')`, and a pop-up saying `Serial port busy`.

After I unplugged and replugged the Fire, Serial Monitor showed the same lines over and over: `rst:0x10 (RTCWDT_RTC_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)` and `flash read err, 1000`. The Fire was restarting in a loop.

Likely cause: the Serial Monitor tab was still holding COM5 open from the previous test, which blocked the upload.

Fix: close the Serial Monitor tab (the X next to its name), unplug the Fire and plug it back in, then upload again. The upload worked and the sketch ran.

If that doesn't work: quit Arduino IDE completely and reopen it, then try Tools, Erase All Flash Before Sketch Upload set to Enabled for one upload (switch it back to Disabled afterwards).

### Problem 6: Black screen after upload, with "invalid magic number 0x5555" and "load partition table error!"

Serial Monitor showed `E (56) qio_mode: Failed to set QIE bit, not enabling QIO mode`, then `flash_parts: partition 0 invalid magic number 0x5555`, `boot: Failed to verify partition table` and `boot: load partition table error!`. The Fire's screen stayed black. It first happened when I uploaded a bigger sketch from a new Arduino IDE window, which started with no board selected.

Things that did not fix it: closing Serial Monitor, and wiping the flash with Erase All Flash Before Sketch Upload.

What fixed it: I changed Tools, Upload Speed from 1500000 to 115200, Flash Mode from QIO to DIO, and Flash Frequency from 80MHz to 40MHz, then uploaded a tiny Hello Fire sketch. The Serial Monitor startup log then showed `mode:DIO` and no partition error. The screen stayed black until I unplugged the USB cable and plugged it back in, and then it came on.

I do not know which of the three settings did the work, since I changed them together. A very fast upload can scramble the write, so Upload Speed is my main suspect.

Lessons: a new Arduino IDE window starts with no board selected (the top bar says "Select Board"), so pick M5Fire on COM5 first. After any upload, if the screen is black, unplug and replug the Fire before assuming the upload failed.

### Problem 7: Upload fails with "Wrong boot mode detected (0x17)! The chip needs to be in download mode."

Error text: `A fatal error occurred: Failed to connect to ESP32: Wrong boot mode detected (0x17)! The chip needs to be in download mode.` The output showed `Connecting......` with dots and then failed. It happened when I uploaded to a Fire that was already running the garden monitor sketch, with the servo and sensors plugged in.

What did not work: unplugging the USB cable and plugging it back in.

What worked: pressing the reset button on the Fire's left side while the output showed `Connecting......`. The upload then went through.

Cause: I do not know for certain. The upload tool restarts the Fire into download mode with the USB lines, and that restart did not happen on its own. The reset button gave it the restart it needed.

If it happens again: start the upload, and press the reset button on the left side when the output shows `Connecting`.

## Problems I hit earlier on a Mac

- "no space left on device" during the board package install. The M5Stack package is several GB once unpacked, so the Mac's disk needs free space.
- Moving the Arduino15 folder to an external drive failed with "operation not supported". The drive was exFAT, which can't create the hard links the toolchain needs. A drive formatted as APFS or NTFS works.

## Checklist for next time

- Use a data USB cable. Some cables only charge.
- Check Device Manager before blaming the IDE. If the Fire is under Other devices, it needs a driver.
- Upload, never Debug.
- Close the Serial Monitor tab before uploading a new sketch.
- Install on a drive formatted as NTFS (Windows) or APFS (Mac).
