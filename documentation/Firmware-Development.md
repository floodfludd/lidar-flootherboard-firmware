# Developing the Firmware
This firmware is based on a Real Time OS, and splits different logical modules of the code into separate "Tasks". Each task communicates with the other task through signalling mechanisms called "Events".

## Real Time OS Concepts
Real time operating systems are based on a lightweight version of a classic operating system, and are primarily designed to allow multiple functions to run concurrently (although code does not run truly concurrently, each separate function is switched between very often, so as to create the illusion several functions run at once)

TI's RTOS implements tasks, each of which has a starting entry function that then runs from there. Tasks can all access the same variables, and can communicate with each other using them. They can also wait on other tasks by "pending" on events, and then another task can "post" to that event to wake up the waiting task.

Tasks have several states:
- Running: the task is the active code running on the processor
- Ready: the task is not running, but is ready to execute when selected to do so
- Blocked: the task is waiting for an event, whether that be an event from another task, some form of I/O, or a timer
- Exited: the task has exited and will not run again.

## System Tasks
This firmware implements several tasks (click on each task to learn more): 
- [Radar task](Radar.md): waits for a task to request a radar sample, then takes a sample. "Posts" to the events of the storage and transmission task when data is available
- [Transmission task](Transmission.md): waits for a request to transmit data, and then uses the LTE module to transmit it
- [Main Task](Main-Task.md): Periodically prompts the radar task to sample data, and the transmission task to synchronize the real time clock
- [Storage task](Storage.md): Waits for data to be available, then stores it to the SD card
- [CLI task](CLI.md): waits for user commands on the UART CLI, then executes them.

## Device Pin Assignments
Although the MSP432 is mounted to a custom board and at this point changing pin assignments would be somewhat pointless, it is still possible. The file `flood_msp432_firmware.syscfg` can be edited using the standalone SYSConfig tool from TI, or the one integrated into Code Composer. This tool allows pin assignments to be changed, and exposes them by predefined C identifiers usable within the code.

