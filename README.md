# smartgarden

Project developed on Iot Master's Degree

## Setup

**IMPORTANT**: check the ESP32 GPIO PINs in use, please try to use the same as defined.

- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html

1. Change `ssid` constant value with the WIFI network **name**
2. Change `password` constant value with the WIFI network **password**
3. Change the `device_id` constant value with a new one

## Sensors

- DHT11 (Temperature Humidity) - PIN GPIO14 - https://esp32io.com/tutorials/esp32-temperature-humidity-sensor
- BH1750 - Light intensity sensor - I2C, GPIO21 and GPIO22 - https://randomnerdtutorials.com/esp32-bh1750-ambient-light-sensor/
- SOIL MOISTURE - PIN GPIO33
