# Debug Command Line
## Commands
The debug command line is intended to facilitate easy debugging during development and deployment of the system. It is accessible while the system is booted via the UART pins `UCA0TX` and `UCA0RX`. The command line provides a variety of commands, which each have help strings. The command list is also documented here:
|   Command|         Syntax  |       Description         | 
| ----     | --              | -----                     |
| help     | `help`          | Prints CLI help           |
| unmount  | `unmount`       | Unmounts the attached SD card, forcing cached files to flush to disk before doing so. |
| sensor_testdata | `sensor_testdata [distance]` | Forces the transmission task to send a bogus water level measurement with the current timestamp to the backend (`distance` is the measurement) |
| mount    |  `mount`        | (Re)mounts an SD card to the system | 
| showcfg  | `showcfg`         | shows the current configuration loaded from the SD card |
| searchSIM | `searchSIM`    | Tries a variety of baudrates to attempt to connect to the SIM7000 module. Rarely useful |
| synctime  | `synctime`     | Forces system clock to sync with network time server |
|radarsample| `radarsample`   | Forces the radar board to take a sample |
| reset     | `reset`         | Resets (reboots) the chip               |
| setradaroffset | `setradaroffset` | Forces the radar to recalibrate its offset value |
| setradarlogging| `setradarlogging [enabled / disabled]` | Enables or disables radar logging. If on, the radar board will print all successful water level samples to the UART command line. Disabled by default.| 

## Accessing the CLI
The CLI runs via UART, so a tool like Putty will work for Windows, or Minicom for Linux. You'll need to know the COM number (Windows) or device name (Linux) of your MSP432 UART debugger to connect. The UART runs at 115200 baud, with 8N1

## Troubleshooting
If you can't see the debug command line at boot, first **wait**. The command line prompt `->` does not immediately show up after boot (due to task priority for the CLI being low). If the command line does not show up after this, check your wiring. Remember that with UART, the `TX` line should connect to the other device's `RX` line (called a *null modem*). If in doubt, Often times just flipping the order of the `TX` and `RX` lines works (guess and check is very effective here). Connecting a **shared ground** between the UART debugger and the development board is also required.
