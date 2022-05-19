
# Weight Scale Data Logger

Some ugly C code to capture different measures (like: timestamp, input voltage, iteration, scale reading, ...), store up to a few kB of them in ESP8266 memory, and then flush periodically to an SD card.

The data collector consists of:

 - microcontroller - ESP8266 - had lying it around & needed internet based timestamp for post processing
 - ADS1115 - 4channel, 16bit ADC to read reference voltages
 - BME280 - I2C/SPI temperature, humidity, pressure sensor
 - SD card - to store captured data
 - [optionally] small fan to distribute heat evenly in the environment
 - [optionally] designated reference voltage for cell sensor ADC
 - [variation of] different power supplies 
 - [variation of] different ADC sensors (HX711, NAU7802)
 - shoe box with a lid, some wood, and a bunch of wires


For more details go to: [Researching weight scales - hardware and firmware](https://piotr.westfalewicz.com/blog/2022/05/researching-weight-scales-hardware-and-firmware/).