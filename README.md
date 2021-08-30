# LoRaWAN node for environment sensing

This app uses the RAK1906 to measure temperature, humidity, barometric pressure and air quality (as gas resistance) and transmit the data over LoRaWAN.

This example has BLE enabled, so you can setup the LoRaWAN parameters over BLE. 
Debug output is enabled as well. If you want to measure current consumption, you should disable the debug output in platformio.ini by setting `MY_DEBUG` to 0:
```ini
build_flags = 
	-DMY_DEBUG=0
```

Payload decoder for Chirpstack (and maybe TTN, but not tested):    
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