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

## Problems I hit earlier on a Mac

- "no space left on device" during the board package install. The M5Stack package is several GB once unpacked, so the Mac's disk needs free space.
- Moving the Arduino15 folder to an external drive failed with "operation not supported". The drive was exFAT, which can't create the hard links the toolchain needs. A drive formatted as APFS or NTFS works.

## Checklist for next time

- Use a data USB cable. Some cables only charge.
- Check Device Manager before blaming the IDE. If the Fire is under Other devices, it needs a driver.
- Upload, never Debug.
- Install on a drive formatted as NTFS (Windows) or APFS (Mac).
