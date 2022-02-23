# Radar Submodule
This module implements the code needed to communicate with the Radar daughterboard, the [IWR1443BOOST](https://www.ti.com/tool/IWR1443BOOST).

## Radar Run Loop
The radar implements a run loop, that has the following steps:
- Check if the radar board has been calibrated, and if not wait for the user to request calibration (via button press or CLI command)
- Wait for the user to request calibration, or to request a sample
- Power on the radar board
- Send configuration commands to the radar board to set up the radar system
- Read a configurable number of samples from the radar board
- Average all these samples
- If the board has been calibrated, use the calibration value with the averaged data to determine the water level. If not, use the averaged data as the calibration value
- Add a timestamp to the data
- Request the transmission and storage modules to upload and save the data

## Radar calibration
The radar board calibration establishes a "zero" point, that measures the height above the ground that the system is mounted at. This value is then saved to the SD card within the `config.txt` file, and on future samples it will be used as an offset. Samples will have the offset subtracted from them, so a shorter distance sample indicates that the water level has risen closer to the device.

Calibration is prompted by pressing the `GPIO0` button on the board. Additionally, the CLI supports forcing a calibration.

## Booting the Radar
This is very simple. The radar board allows an external device to power on (or off) it's power management IC, so the firmware simply has to power on the PMIC to start the radar board.

## Radar configuration
Radar configuration commands are simply strings sent over the UART connection, followed by a `\r\n` to indicate a newline to the radar board. The firmware waits for the radar board to acknowledge the command with a `Done` response, then sends another configuration command.

## Radar sampling
The radar board sends samples in the following packets:
```
struct Radar_Packet {
    uint16_t header; // Two bytes
    float distance; // Four bytes
};
```
These 6 byte packages hold a 2 byte "magic" header (defined in the radar firmware and the MSP432 firmware), and a 4 byte float that indicates the distance the radar board reports it detects the ground (or water) at in meters.

Once the radar board is configured, it will send samples until powered off (roughly once every 1/2 second). Once the MSP432 has received enough of these samples, it will average them into a final value, and offset the value to produce a water level as described in the "Configuration" section