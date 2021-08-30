----
----
# _WORK IN PROGRESS_
----
----

# RAK4631-Kit-4-RAK1906
This is an example code for WisBlock Environment sensor ([WisBlock Kit 4](https://store.rakwireless.com/collections/kits-bundles/products/wisblock-kit-4-air-quality-monitor)) with RAK1906 environment sensor

It is based on my low power event driven example [RAK4631-Quick-Start-Examples](https://github.com/beegee-tokyo/RAK4631-Quick-Start-Examples)

----

# Hardware used
- [RAK4631](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/) WisBlock Core module
- [RAK5005-O](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK5005-O/Overview/) WisBlock Base board
- [RAK1906](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1906/Overview/) WisBlock Sensor environment module

----

# Software used
- [PlatformIO](https://platformio.org/install)
- [Adafruit nRF52 BSP](https://docs.platformio.org/en/latest/boards/nordicnrf52/adafruit_feather_nrf52832.html)
- [Patch to use RAK4631 with PlatformIO](https://github.com/RAKWireless/WisBlock/blob/master/PlatformIO/RAK4630/README.md)
- [SX126x-Arduino LoRaWAN library](https://github.com/beegee-tokyo/SX126x-Arduino)
- [Adafruit BME680 Library](https://platformio.org/lib/show/1922/Adafruit%20BME680%20Library)

----


# Setting up LoRaWAN credentials
The LoRaWAN credentials are defined in [main.h](./include/main.h). But this code supports 2 other methods to change the LoRaWAN credentials on the fly:

## 1) Setup over BLE
Using the [My nRF52 Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.my_nrf52_tb) you can connect to the WisBlock over BLE and setup all LoRaWAN parameters like
- Region
- OTAA/ABP
- Confirmed/Unconfirmed message
- ...

More details can be found in the [My nRF52 Toolbox repo](https://github.com/beegee-tokyo/My-nRF52-Toolbox/blob/master/README.md)

## 2) Setup over USB port
Using the AT command interface the WisBlock can be setup over the USB port.

A detailed manual for the AT commands are in [AT-Commands.md](./AT-Commands.md)

----

# Debug options 
Debug output can be controlled by defines in the **`platformio.ini`**    
_**LIB_DEBUG**_ controls debug output of the SX126x-Arduino LoRaWAN library
 - 0 -> No debug outpuy
 - 1 -> Library debug output (not recommended, can have influence on timing)    

_**MY_DEBUG**_ controls debug output of the application itself
 - 0 -> No debug outpuy
 - 1 -> Application debug output

_**CFG_DEBUG**_ controls the debug output of the nRF52 BSP. It is recommended to keep it off

## Example for no debug output and maximum power savings:

```
[env:wiscore_rak4631]
platform = nordicnrf52@8.1.0
board = wiscore_rak4631
framework = arduino
build_flags = 
    ; -DCFG_DEBUG=2
	-DSW_VERSION_1=1 ; major version increase on API change / not backwards compatible
	-DSW_VERSION_2=0 ; minor version increase on API change / backward compatible
	-DSW_VERSION_3=0 ; patch version increase on bugfix, no affect on API
	-DLIB_DEBUG=0    ; 0 Disable LoRaWAN debug output
	-DMY_DEBUG=0     ; 0 Disable application debug output
	-DNO_BLE_LED=1   ; 1 Disable blue LED as BLE notificator
lib_deps = 
	beegee-tokyo/SX126x-Arduino
	adafruit/Adafruit BME680 Library
extra_scripts = pre:rename.py
```

----

# Payload decoder for Chirpstack (and maybe TTN, but not tested):    
```js
function Decode(fPort, bytes, variables) {
	var decoded = {};
	switch (bytes[0])
	{
		case 0x01: // Environment sensor data
			decoded.temperature = (bytes[1] << 8 | bytes[2]) / 100;
			decoded.humidity = (bytes[3] << 8 | + bytes[4]) / 100;
			decoded.pressure = (bytes[8] | (bytes[7] << 8) | (bytes[6] << 16) | (bytes[5] << 24)) / 100;
			decoded.gas = bytes[12] | (bytes[11] << 8) | (bytes[10] << 16) | (bytes[9] << 24);
			break;
		case 0x02: // Temperature and humidity sensor data
			decoded.temperature = (bytes[1] << 8 | bytes[2]) / 100;
			decoded.humidity = (bytes[3] << 8 | + bytes[4]) / 100;
			decoded.batt = (bytes[5] << 8 | bytes[6]) / 100;
			break;
		case 0x03: // Ambient light sensor data
			decoded.light = (bytes[4] | (bytes[3] << 8) | (bytes[2] << 16) | (bytes[1] << 24)) / 10;
			decoded.batt = (bytes[5] << 8 | bytes[6]) / 100;
			break;
		case 0x04: // No sensor data, just counter
			decoded.cnt = bytes[4] | (bytes[3] << 8) | (bytes[2] << 16) | (bytes[1] << 24);
			break;
		case 0x20: // Solar panel data
			decoded.current = (bytes[1] << 8 | bytes[2]);
			decoded.today = (bytes[3] << 8 | + bytes[4]);
			break;
		case 0x30: // Accelerometer sensor
        	if (bytes[1] == 0) {
				decoded.x_move = "no";
            } else {
				decoded.x_move = "yes";
            }
        	if (bytes[2] == 0) {
				decoded.y_move = "no";
            } else {
				decoded.y_move = "yes";
            }
        	if (bytes[3] == 0) {
				decoded.z_move = "no";
            } else {
				decoded.z_move = "yes";
            }
			break;
		default:
			decoded.unknown = "Unknown data format";
			break;
	}
	return decoded;
}
```

# Visualization on DataCake

[Real time data](https://app.datacake.de/dashboard/d/b6acccc0-2264-42d4-aec9-94148d7eb76f)

![Snapshot](./assets/datacake.png)


