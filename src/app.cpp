/**
 * @file app.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Application specific functions. Mandatory to have init_app(),
 *        app_event_handler(), ble_data_handler(), lora_data_handler()
 *        and lora_tx_finished()
 * @version 0.1
 * @date 2021-04-23
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "app.h"

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK-ENV";

/** Packet buffer for sending */
WisCayenne g_solution_data(255);

/** Send Fail counter **/
uint8_t send_fail = 0;

/** Flag for low battery protection */
bool low_batt_protection = false;

/**
 * @brief Application specific setup functions
 *
 */
void setup_app(void)
{
	// Called in the very beginning of setup
	/**************************************************************/
	/**************************************************************/
	/// \todo set enable_ble to true if you want to enable BLE
	/// \todo if true, BLE is enabled and advertise 60 seconds
	/// \todo after reset or power-up
	/**************************************************************/
	/**************************************************************/
	g_enable_ble = true;
}

/**
 * @brief Application specific initializations
 *
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	// Reset the packet
	g_solution_data.reset();

	// Add your application specific initialization here
	if (!init_bme680())
	{
		return false;
	}

	return true;
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		/**************************************************************/
		/**************************************************************/
		/// \todo Just as example, if BLE is enabled and you want
		/// \todo to restart advertising on an event you can call
		/// \todo restart_advertising(uint16_t timeout); to advertise
		/// \todo for another <timeout> seconds
		/**************************************************************/
		/**************************************************************/
		if (g_enable_ble)
		{
			restart_advertising(15);
		}

		// Reset the packet
		g_solution_data.reset();

		if (!low_batt_protection)
		{
			// Get BME680 data
			bme680_get();
		}

		// Get battery level
		float batt_level_f = read_batt();
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);

		// Protection against battery drain
		if (batt_level_f < 2900)
		{
			// Battery is very low, change send time to 1 hour to protect battery
			low_batt_protection = true; // Set low_batt_protection active
			api_timer_restart(1 * 60 * 60 * 1000);
			MYLOG("APP", "Battery protection activated");
		}
		else if ((batt_level_f > 4100) && low_batt_protection)
		{
			// Battery is higher than 4V, change send time back to original setting
			low_batt_protection = false;
			api_timer_restart(g_lorawan_settings.send_repeat_time);
			MYLOG("APP", "Battery protection deactivated");
		}

		if (g_lorawan_settings.lorawan_enable)
		{
			// Enqueue the packet
			lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
			switch (result)
			{
			case LMH_SUCCESS:
				MYLOG("APP", "Packet enqueued");
				break;
			case LMH_BUSY:
				MYLOG("APP", "LoRa transceiver is busy");
				break;
			case LMH_ERROR:
				MYLOG("APP", "Packet error, too big to send with current DR");
				break;
			}
		}
		else
		{
			uint8_t packet_buffer[g_solution_data.getSize() + 8];
			memcpy(packet_buffer, g_lorawan_settings.node_device_eui, 8);
			memcpy(&packet_buffer[8], g_solution_data.getBuffer(), g_solution_data.getSize());

			// Send packet over LoRa
			if (send_p2p_packet(packet_buffer, g_solution_data.getSize() + 8))
			{
				MYLOG("APP", "P2P packet enqueued");
			}
			else
			{
				MYLOG("APP", "P2P packet too big");
			}
		}
	}
}

/**
 * @brief Handle BLE UART data
 *
 */
void ble_data_handler(void)
{
	if (g_enable_ble)
	{
		// BLE UART data handling
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			/**************************************************************/
			/**************************************************************/
			/// \todo BLE UART data arrived
			/// \todo parse them here
			/**************************************************************/
			/**************************************************************/
			g_task_event_type &= N_BLE_DATA;
			String uart_rx_buff = g_ble_uart.readStringUntil('\n');

			uart_rx_buff.toUpperCase();

			MYLOG("BLE", "BLE Received %s", uart_rx_buff.c_str());
		}
	}
}

/**
 * @brief Handle received LoRa Data
 *
 */
void lora_data_handler(void)
{
	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRaWAN TX cycle (including RX1 and RX2 window) finished
		/// \todo can be used to enable next sending
		/// \todo if confirmed packet sending, g_rx_fin_result holds the result of the transmission
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_TX_FIN;

		MYLOG("APP", "LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

		if (!g_rx_fin_result)
		{
			// Increase fail send counter
			send_fail++;

			if (send_fail == 10)
			{
				// Too many failed sendings, reset node and try to rejoin
				delay(100);
				sd_nvic_SystemReset();
			}
		}
	}
	// LoRa Join finished handling
	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
	{
		g_task_event_type &= N_LORA_JOIN_FIN;
		if (g_join_result)
		{
			MYLOG("APP", "Successfully joined network");
		}
		else
		{
			MYLOG("APP", "Join network failed");
			/// \todo here join could be restarted.
			lmh_join();
		}
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRa data arrived
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");
		char log_buff[g_rx_data_len * 3] = {0};
		uint8_t log_idx = 0;
		for (int idx = 0; idx < g_rx_data_len; idx++)
		{
			sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
			log_idx += 3;
		}
		MYLOG("APP", "%s", log_buff);

		/**************************************************************/
		/**************************************************************/
		/// \todo Just an example, if BLE is enabled and BLE UART
		/// \todo is connected you can send the received data
		/// \todo for debugging
		/**************************************************************/
		/**************************************************************/
		if (g_ble_uart_is_connected && g_enable_ble)
		{
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				g_ble_uart.printf("%02X ", g_rx_lora_data[idx]);
			}
			g_ble_uart.println("");
		}
	}
}
