#pragma once

//ESP8266 NodeMCU pins: https://github.com/nodemcu/nodemcu-devkit-v1.0
#include <Arduino.h>
#include <pins_arduino.h>

/* SD card connection
 * Connection Diagram: https://diyi0t.com/sd-card-arduino-esp8266-esp32/
 * No.  |   SD Card - SPI    |   ESP8266 
 * 1    |   CS               |   D8/HCS
 * 2    |   DI(MOSI)         |   D7/HMOSI
 * 3    |   VSS1             |   GND
 * 4    |   VDD              |   +3.3V
 * 5    |   SCLK             |   D5/HSCLK
 * 6    |   VSS2             |   GND
 * 7    |   DO(MISO)         |   D6/HMISO
 * 8    |   -                |   -
 * 9    |   -                |   -
 */
static const uint8_t UC_PIN_SD_CARD = D8;

/*
Special ESP8266 pins: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
and https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
*/
static const uint8_t UC_PIN_HX711_CLK = D0;
static const uint8_t UC_PIN_HX711_DAT = D3;

static const uint8_t I2C_SDA_PIN = D2;
static const uint8_t I2C_SCL_PIN = D1;

static const int UC_ADS1115_ADDR = 0x48;
static const int UC_BME280_ADDR = 0x76;
