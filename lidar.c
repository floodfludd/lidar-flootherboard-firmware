/*
 * lidar.c
 *
 *  Created on: Feb 22, 2022
 *      Author: chrom
 */


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
#define LIDAR_MIN_SAMPLES 30
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
static bool log_lidar_samples = false;

/* Static functions */
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



