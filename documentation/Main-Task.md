# Main Firmware Task
This task is the "driver" of the firmware. During bootup, it handles module initialization, and once the MSP432 is running it handles periodically sampling data from the Radar board, as well as synchronizing with network time to prevent clock drift

## Bootup
During boot, the main firmware task is not a "task" per se, but simply the `main` function. This function will call the initialization routines for the Transmission, CLI, Radar, and Storage modules, which will set up the radar and LTE boards, as well as load saved configuration parameters from the SD card. The functions called here are much like the `setup` function in an Arduino IDE.

Once the `setup` phase is complete, the main function starts the CLI, Transmission, Radar, Storage, and finally Main tasks. These tasks each have run loops, similar to the `loop` function on an Arduino, but each task has its own `loop`. 

## Run Loop/Driver
The main task will synchronize the system time with a network clock using the transmission module, then will take a radar sample (the radar module itself will handle forwarding this sample to the storage and transmission modules for saving and uploading). 

Periodically, the main task will again prompt the radar board to sample again. Roughly every few days, the main task will also prompt the transmission task to synchronize the system clock to prevent clock drift. The main task will also force the storage module to write cached log and water level data to the disk periodically, in case of an unexpected loss of power.