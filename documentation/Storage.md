# Storage Module
The storage module handles all interfacing with the attached SD card. It is responsible for loading configuration data from the SD card, saving the calibration offset from the radar system on the SD card, storing water level data, and saving system logs to the SD card.

## Configuration File
The storage module supports a *very* rudimentary text file parsing system, that loads a configuration file from the disk. The configuration file must have configuration keys formatted in the following manner:
```
Key : Value
```
Furthermore, lines starting with `#` will be ignored, as will blank lines. Lines cannot exceed 80 characters due to the size of the input buffer in the code.

Configuration parameters can be found within generated documentation, or within the `storage.c` file itself.

## Radar offset
The radar offset is stored within the configuration file, but has the unique distinction of being a parameter the firmware will set itself. When the parameter is `0` or not present, the radar module will calibrate itself and save a new offset. The offset is saved by reading the configuration file line by line, and writing each line out to a new file. When the line with the radar offset is encountered, the new radar offset will be written for the value instead of the previous value of `0`. Finally, the new file will be copied over the old one.

## Storing Water Level Data
When water level data is stored, the storage module will queue the water level data packet, then notify the storage task that data is available. The data packet will be formatted to be stored into a CSV file with the timestamp and water level, then the data will be written to the CSV file.

## Log Data
System log data will be written to the UART CLI, but also will be written to a log file on the disk, along with timestamp values for each log entry.

## SD card management
The storage module also supports mounting an unmounting the SD card, so that files on the SD card can be edited without a need to reboot the system. If you'd like to remove the SD card, unmounting it first is best to ensure data isn't lost.