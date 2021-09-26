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
#include <WisBlock-API.h> // Click to install library: http://librarymanager/All#WisBlock-API

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
struct env_data_s
{
	uint8_t data_flag0 = 0x07; // 1
	uint8_t data_flag1 = 0x68; // 2
	uint8_t humid_1 = 0;	   // 3
	uint8_t data_flag2 = 0x02; // 4
	uint8_t data_flag3 = 0x67; // 5
	uint8_t temp_1 = 0;		   // 6
	uint8_t temp_2 = 0;		   // 7
	uint8_t data_flag4 = 0x06; // 8
	uint8_t data_flag5 = 0x73; // 9
	uint8_t press_1 = 0;	   // 10
	uint8_t press_2 = 0;	   // 11
	uint8_t data_flag6 = 0x04; // 12
	uint8_t data_flag7 = 0x02; // 13
	uint8_t gas_1 = 0;		   // 14
	uint8_t gas_2 = 0;		   // 15
	uint8_t data_flag8 = 0x08; // 16
	uint8_t data_flag9 = 0x02; // 17
	uint8_t batt_1 = 0;		   // 18
	uint8_t batt_2 = 0;		   // 19
};
extern env_data_s g_env_data;
#define ENV_DATA_LEN 19 // sizeof(g_env_data)

/** Sensor specific functions */
bool init_bme680(void);
void bme680_get();

/** Battery level uinion */
union batt_s
{
	uint16_t batt16 = 0;
	uint8_t batt8[2];
};

#endif