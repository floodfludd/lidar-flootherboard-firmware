# Transmission Submodule
The transmission submodule makes use of the SIM7000 LTE module, and the associated library written for it, to upload radar samples to the remote backend on AWS, as well as synchronize the MSP432's clock to the network time.

## Transmission Events
The transmission module waits on two event types: data being available to transmit, and requests to sync with a network time server.

## Data Transmission Process
When another task requests that the transmission module send data, it will queue this data into a waiting FIFO queue, and notify the transmission task itself that data is available. The transmission task will then begin running, and will take all data from the FIFO queue, and attempt to send that data to the backend. Sending the data is done in the following steps:
- Boot up the LTE module
- Structure the water level data into a JSON structure, formatted as follows:
```
{"distance": WATER_LEVEL_DISTANCE, "timestamp": UTC_TIMESTAMP}
```
- Use the LTE Module to make an HTTP POST request to the backend URL using this data as the body. This request also includes an authorization token in the header.

The transmission will be attempted once more if it fails, and then the data will be abandoned (whether the data is saved to the SD card is independent of transmission succeeding.)

## Network Time Sync Process
When another task requests that the transmission task synchronize with network time, the function will notify the transmission task to start up, and the transmission task will use the LTE module to synchronize with a network time server defined in the SIM7000 library. This timestamp will be used to update the real time clock. If the update fails, a fallback value will be used to set the clock.

## SIM7000 LTE Module
See [here](SIM7000.md) for the SIM7000 LTE module documentation. This LTE module library code is where the majority of the details regarding network transmission are implemented.
