# System Introduction
This firmware was created using the MSP432 SimpleLink SDK and TI's Code composer. It is designed to keep power consumption as low as possible, and to keep all attached daughterboards powered off as much as possible. It implements the "glue logic" that handles sampling data from the radar subsystem, processing the data, and uploading the data to the backend hosted on AWS for viewing using the AWS hosted fronted.

## Design Rationale
Code Composer and TI's SDK were selected because they took advantage of all the power saving features of the MSP432, and allowed the code to be modular. One of the principle goals of this firmware is to allow the MSP432 to idle in its low power state as much as possible, and TI's firmware supports this implicitly (if all system tasks are blocked, the MSP432 will enter a low power state)

## System Data Flow
The system processes data using the following steps. For each step, click on the link for additional details
- [System Boots and configures the radar and LTE module](Main-Task.md)
- [System "calibrates" the radar module if required](Radar.md)
- [After a delay, the system boots the radar module and samples distance data](Radar.md)
- [The firmware processes the radar distance data into a water level, and adds a timestamp](Radar.md)
- [The firmware forwards the water level data to the storage module, and it is saved to the SD card](Storage.md)
- [The firmware forwards the water level data to the transmission module, and it is uploaded to the backend.](Transmission.md)