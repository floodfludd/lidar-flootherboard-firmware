/**
 *  @file lidar.c
 *  Implements communication with attached lidar sensor to read water level
 *  This file should implement communication with an attached sensor, and
 *  support a notification method to make other tasks aware new sensor data
 *  was acquired
 *
 *  Created on: Feb 22, 2022
 *  Author: Samuel Ramirez, danieldegrasse
 */

/* XDC Module Headers */
#include <xdc/runtime/System.h>
#include <xdc/std.h>

/* BIOS Module headers */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Task.h>

/* Ti Driver Headers */
#include "ti_drivers_config.h"
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/I2C.h>

/* Standard libraries */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cli.h"
#include "common.h"
#include "storage.h"
#include "transmission.h"
#include "lidar.h"

/** Constant values board should reply with */
#define LIDAR_CMD_DONE "Done"     /**< the board successfully ran a command */
#define LIDAR_PROMPT "garmin:/>" /**< new command prompt from the board */

#define LIDAR_BAUDRATE 115200      /**< UART Baudrate of Radar sensor */
#define LIDAR_PACKET_HEADER 0xBEEF /**< "Magic" header on every radar packet   \
                                    */
#define LIDAR_DONE_TIMEOUT 7000 /**< how many ms to wait for commands to run   \
                                 */
#define LIDAR_READ_TIMEOUT 100  /**< how many ms to wait for data from system  \
                                 */
#define LIDAR_SAMPLE_DELAY 500  /**< how many ms to wait for radar sample */
#define LIDAR_BOARD_CMD_DELAY 2 /**< how many ms to wait between commands */
#define RANGE_DELTA_MIN                                                        \
    0.8 /**< how many meters below offset to allow samples */
#define RANGE_DELTA_MAX                                                        \
    0.2 /**< how many meters above offset to allow samples */
/**
 * minimum number of samples to be averaged for a TX to go through, should
 * radar fail to return a sample
 */
#define LIDAR_MIN_SAMPLES 1
#define LIDAR_READ_NODATA -2   /**< Radar did not return any data */
#define LIDAR_READ_NOPACKET -1 /**< Radar did not return a valid packet */
#define LIDAR_SUCCESS 0        /**< Radar returned data */
#define LIDAR_THRESHOLD 0      /**< Minimum value radar should return */
/**
 * max height expected (~30 ft height at 10m), throws out bogus values of
 * high magnitude (in case of bit errors on UART)
 */
#define LIDAR_UPPER_THRESHOLD 10
/** how many ticks to delay between LED flashes during calibration */
#define CALIBRATION_BLINK_DELAY 750

#define EVT_LIDAR_SAMPLE Event_Id_00    /**< Radar sample event ID */
#define EVT_LIDAR_CALIBRATE Event_Id_01 /**< Radar calibration event ID */

/**
 * Lidar data packet. Only holds distance. Note the use of struct packing
 */
typedef struct __attribute((__packed__)) LidarPacket {
    uint16_t header; /**< "Magic" Header value should go here */
    float distance;  /**< Actual distance from Lidar board */
} LidarPacket; /**< Lidar data packet */

/* Global variables */
static I2C_Params garminParams;
static I2C_Handle garmin_i2c;
static Event_Handle lidarEventHandle;
static I2C_Transaction garminTransaction = {0};

// Should the wait for GPIO0 to be pressed during calibration be bypassed
static bool button_press_bypassed = false;
// Should each lidar sample be logged to the CLI
static bool log_lidar_samples = true;

/* Static functions */

static int read_lidar_distance(float *distance);
static int turn_on(uint8_t accuracy);
static int turn_off(void);
/*
static int configure_lidar();
static int wait_data(const char *expected, int len);
static int read_lidar_distance(float *distance);
static void boot_lidar();
static void powerdown_lidar();
static int wait_boot();
static int wait_packet();
static void end_calibration_wait(uint8_t index);
*/

/**
 * This function should perform any initialization the lidar sensor model needs
 * This includes setting up peripherals (like I2C).
 * This is analogous to the setup() function in for Arduino
 */
void lidar_init() {
    // Set up the I2C to read data from the sensor
    I2C_Params_init(&garminParams);
    garminParams.bitRate = I2C_400kHz;
    garminParams.transferMode = I2C_MODE_BLOCKING;

    lidarEventHandle = Event_create(NULL, NULL);
    if (!lidarEventHandle) {
        System_abort("Could not create lidar event\n");
    }

    /** FIX ME **/
    // I think the following lines are about controlling the power pins on the sensor to minimize power
    // So it would probably be smart to use them for lidar too
    // We would need to figure out if we need to reconnect the I2C junk first tho if that gets taken care of
    // Whenever the box goes to sleep

    // Configure the Radar PMIC GPIO to power down the radar board
    GPIO_setConfig(CONFIG_GPIO_RADAR_PMIC_EN,
                   GPIO_CFG_OUTPUT | GPIO_CFG_OUT_LOW);
    // Also configure D1 indicator LED as low output
    GPIO_setConfig(CONFIG_D1_LED, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_LOW);
    // Configure the button as a pulled down input
    /*
    GPIO_setConfig(CONFIG_GPIO0_BTN,
                   GPIO_CFG_INPUT | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING);
    // install Button callback
    GPIO_setCallback(CONFIG_GPIO0_BTN, end_calibration_wait);
    */
    // Open the sensor
    garmin_i2c = I2C_open(CONFIG_I2C_0, &garminParams);
    if (garmin_i2c == NULL) {
        System_abort("sensor I2C init failed, exiting\n");
    }

    GPIO_write(CONFIG_D1_LED, 1);
    System_printf("sensor init done\n");

}

void lidar_run(UArg arg0, UArg arg1) {
    int num_samples;
    UInt events;
    float num_samples_avg;
    float sum;
    SensorDataPacket packet;
    System_printf("sensor task starting\n");
    Watchdog_clear(watchdogHandle);
    cli_log("Lidar task starting with distance offset of %.2f\n",
            program_config.lidar_sample_offset);
    /*
     * Run Loop.
     * 1. Wait for a sample to be requested
     * 2. Boot the radar board
     * 3. Configure the board
     * 4. Take a defined number of samples
     * 5. Power down the board
     */
    while (1) {
        //if (program_config.lidar_sample_offset == 0.0) {
            // Where calibration code would go
            // Don't want to include it yet so as to make things simpler
        //} else {
            events = Event_pend(lidarEventHandle, Event_Id_NONE,
                                EVT_LIDAR_SAMPLE, BIOS_WAIT_FOREVER);
        //}
        // Only run if one of the events that were called was EVT_LIDAR_SAMPLE from sample_lidar in main
        if (events & EVT_LIDAR_SAMPLE) {
            // For now, let's assume lidar is always on
            // boot_lidar would usually be something like switching from ALWAYS_ON for high accuracy mode to ASYNCHRONOUS_MODE to preserve power
            //boot_lidar();

            /*
             // We don't need to wait for lidar to boot, since it is always on
            if (wait_boot() < 0) {
                System_printf("lidar did not boot\n");
                System_flush();
                Watchdog_clear(watchdogHandle);
                cli_log("Timed out waiting for radar boot\n");
                powerdown_lidar();
                continue; // Do not run rest of loop
            }
             // Don't need to configure lidar, assuming it is ALWAYS_ON
            if (configure_radar() < 0) {
                System_printf("Failed to configure radar module\n");
                System_flush();
                cli_log("Failed to configure radar module\n");
                powerdown_radar();
                continue; // Do not run rest of loop
            }

            // Wait for lidar board to produce data.
            // We also don't need since it waits for a magic number header to be produced before actually reading
            if (wait_packet() < 0) {
                System_printf("Did not get data from radar board\n");
                Watchdog_clear(watchdogHandle);
                System_flush();
                cli_log("Radar board is not producing data\n");
                powerdown_radar();
                continue; // Do not run rest of loop
            }
            */
            struct timespec ts;
            float distance;
            num_samples = program_config.lidar_sample_count;
            num_samples_avg = 0;
            sum = 0;
            while (num_samples--) {
                Task_sleep(LIDAR_SAMPLE_DELAY);
                num_samples_avg = num_samples_avg + 1.0;
                if (read_lidar_distance(&distance) != LIDAR_SUCCESS) {
                    Watchdog_clear(watchdogHandle);
                    System_printf("Failed to get sample from lidar board\n");
                    System_flush();
                    cli_log("Did not get sample from lidar board\n");
                    break; // Exit loop
                }
                sum = sum + distance;
            }
            if (sum != 0 && num_samples_avg > LIDAR_MIN_SAMPLES) {
                if (program_config.lidar_sample_offset == 0.0) {
                    // This would also be used for calibrating the lidar sample offset
                    // Not gonna worry about this for now
                } else {
                    // Send and store radar data
                    packet.distance = sum / num_samples_avg;

                    // We won't be using this for now, since it appears to get the delta of change of water
                    // subtract distance from offset
                    //packet.distance = program_config.lidar_sample_offset - packet.distance;
                    // If a negative value was read, just zero it out (currently
                    // disabled) if (packet.distance < 0) packet.distance = 0;
                    clock_gettime(CLOCK_REALTIME, &ts);
                    packet.timestamp = ts.tv_sec;
                    if (log_lidar_samples) {
                        System_printf("printf: %.03f\n", packet.distance);
                        cli_log("cli_log: %.03f\n", packet.distance);
                        System_flush();
                    }
                    store_sensor_data(&packet);
                    transmit_sensor_data(&packet);
                }
            }
            // Assume lidar is always on for now
            //powerdown_lidar();
        }
    }
}

void sample_lidar(void) {
    if (!program_config.lidar_module_enabled) {
        return;
    }
    Event_post(lidarEventHandle, EVT_LIDAR_SAMPLE);
}

static int read_lidar_distance(float *distance) {

    uint8_t writeData[2] = {ACQ_COMMANDS, 0x04};
    uint8_t readData[2];

    garminTransaction.slaveAddress = garmin_ADDRESS;      // Where we send to
    garminTransaction.writeBuf = writeData;                // What we send
    garminTransaction.writeCount = 2;                      // How many bytes to send
    if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
        return LIDAR_READ_NODATA;
    }

    System_printf("Before data is ready\n");
    System_flush();
    do {
        writeData[0] = STATUS;
        garminTransaction.slaveAddress = garmin_ADDRESS;      // Where we send to
        garminTransaction.writeBuf = writeData;                // What we send
        garminTransaction.writeCount = 1;                      // How many bytes to send
        garminTransaction.readBuf = readData;                  // Where to store data
        garminTransaction.readCount = 1;                       // How many bytes to read
        if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
            return LIDAR_READ_NODATA;
        }
    } while(readData[0] & 0x1 == 1);

    System_printf("After data is ready\n");
    System_flush();
    writeData[0] = 0x10;
    garminTransaction.slaveAddress = garmin_ADDRESS;      // Where we send to
    garminTransaction.writeBuf = writeData;                // What we send
    garminTransaction.writeCount = 1;                      // How many bytes to send
    garminTransaction.readBuf = readData;                  // Where to store data
    garminTransaction.readCount = 2;                       // How many bytes to read
    if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
        return LIDAR_READ_NODATA;
    }

    *distance = (readData[1] << 8) | readData[0];

    /*
    if (*distance < LIDAR_THRESHOLD ||
        *distance > LIDAR_UPPER_THRESHOLD) {
            return LIDAR_READ_NOPACKET;
    }
    */

    return LIDAR_SUCCESS;
}

static int turn_on (uint8_t accuracy) {
	// turns on always on mode and turns on high accuracy mode
	uint8_t writeData0[2] = {POWER_MODE, 0xFF};
	garminTransaction.slaveAddress = garmin_ADDRESS;
	garminTransaction.writeBuf = writeData0;
	garminTransaction.writeCount = 2;
    if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
        return LIDAR_READ_NODATA;
    }

	uint8_t writeData1[2] = {HIGH_ACCURACY_MODE, accuracy};
	garminTransaction.slaveAddress = garmin_ADDRESS;
	garminTransaction.writeBuf = writeData1;
	garminTransaction.writeCount = 2;
	if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
	    return LIDAR_READ_NODATA;
	}

	return LIDAR_SUCCESS;
}

static int turn_off (void) {
	// turn off high accuracy mode and turns on asynchronous mode
	uint8_t writeData0[2] = {HIGH_ACCURACY_MODE, 0x00};
	garminTransaction.slaveAddress = garmin_ADDRESS;
	garminTransaction.writeBuf = writeData0;
	garminTransaction.writeCount = 2;
	if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
	    return LIDAR_READ_NODATA;
	}

	uint8_t writeData1[2] = {POWER_MODE, 0x00};
	garminTransaction.slaveAddress = garmin_ADDRESS;
	garminTransaction.writeBuf = writeData1;
	garminTransaction.writeCount = 2;
	if (!I2C_transfer(garmin_i2c, &garminTransaction)) {
	    return LIDAR_READ_NODATA;
	}

	return LIDAR_SUCCESS;
}
