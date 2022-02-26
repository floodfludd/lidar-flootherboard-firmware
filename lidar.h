/**
 *  @file lidar.h
 *  Implements communication with attached lidar sensor to read water level
 *  This file should implement communication with an attached sensor, and
 *  support a notification method to make other tasks aware new sensor data
 *  was acquired
 *
 *  Created on: Feb 22, 2022
 *  Author: Samuel Ramirez, danieldegrasse
 */

#ifndef LIDAR_H_
#define LIDAR_H_

#define garmin_ADDRESS 0x62

#include <xdc/std.h>

/**
 * Configuration values for the sensor task
 */
#define LIDAR_TASK_STACK_MEM 2048 /**< size of the task stack */
#define LIDAR_TASK_PRIORITY 1     /**< priority of task */


/**
 * Enum to store Lidar's registers
 */
enum
  {
    ACQ_COMMANDS = 0x00,
    STATUS = 0x01,
    ACQUISITION_COUNT = 0x05,
    FULL_DELAY_LOW = 0x10,
    FULL_DELAY_HIGH = 0x11,
    UNIT_ID_0 = 0x16,
    UNIT_ID_1 = 0x17,
    UNIT_ID_2 = 0x18,
    UNIT_ID_3 = 0x19,
    I2C_SEC_ADDR = 0x1A,
    I2C_CONFIG = 0x1B,
    DETECTION_SENSITIVITY = 0x1C,
    LIB_VERSION = 0x30,
    CORR_DATA = 0x52,
    CP_VER_LO = 0x72,
    CP_VER_HI = 0x73,
    BOARD_TEMPERATURE = 0xE0,
    HARDWARE_VERSION = 0xE1,
    POWER_MODE = 0xE2,
    MEASUREMENT_INTERVAL = 0xE3,
    FACTORY_RESET = 0xE4,
    QUICK_TERMINATION = 0xE5,
    START_BOOTLOADER = 0xE6,
    ENABLE_FLASH_STORAGE = 0xEA,
    HIGH_ACCURACY_MODE = 0xEB,
    SOC_TEMPERATURE = 0xEC,
    ENABLE_ANT_RADIO = 0xF0,
  };

/**
 * This function should perform any initialization the lidar sensor model needs
 * This includes setting up peripherals (like I2C).
 * This is analogous to the setup() function in for Arduino
 */
void lidar_init();

/**
 * This function should run forever, and poll any sensor data from the radar
 * sensor. It should then notify other processes that new sensor data is
 * available
 * @param arg0: Unused
 * @param arg1: Unused
 */
//void lidar_run(UArg arg0, UArg arg1);

/**
 * Asynchronously runs sampling code for lidar, getting one sample
 * and transmitting it
 */
//void sample_lidar();

/**
 * Forces the lidar to calibrate.
 */
//void force_calibration();

/**
 * Enables (or disables) lidar sample logging
 * @param enabled: should samples be logged to the CLI
 */
//void configure_sample_logging(bool enabled);



#endif /* LIDAR_H_ */
