/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from main.h
 * @version 0.1
 * @date 2021-04-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef APP_H
#define APP_H

#include <Arduino.h>
/** Add you required includes after Arduino.h */

/** Include the WisBlock-API */
#include <WisBlock-API-V2.h> // Click to install library: http://librarymanager/All#WisBlock-API-V2

// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 0
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)           \
	do                            \
	{                             \
		if (tag)                  \
			PRINTF("[%s] ", tag); \
		PRINTF(__VA_ARGS__);      \
		PRINTF("\n");             \
	} while (0)
#else
#define MYLOG(...)
#endif

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

/** Application function definitions */
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);
void lora_tx_finished(bool success);
void lora_rx_failed(void);

/** Examples for application events */
#define PIR_TRIGGER 0b1000000000000000
#define N_PIR_TRIGGER 0b0111111111111111
#define BUTTON 0b0100000000000000
#define N_BUTTON 0b1011111111111111

// LoRaWan functions
#include "wisblock_cayenne.h"
// Cayenne LPP Channel numbers per sensor value
#define LPP_CHANNEL_BATT 1	  // Base Board
#define LPP_CHANNEL_HUMID_2 6 // RAK1906
#define LPP_CHANNEL_TEMP_2 7  // RAK1906
#define LPP_CHANNEL_PRESS_2 8 // RAK1906
#define LPP_CHANNEL_GAS_2 9	  // RAK1906

extern WisCayenne g_solution_data;

/** Sensor specific functions */
bool init_bme680(void);
void bme680_get();

#endif