#ifndef _SimpleSdCard_H_
#define _SimpleSdCard_H_

#include "hardwareConsts.h"

#include <Arduino.h>
#include "SdFat.h"
#include "sdios.h"
#include "FreeStack.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = UC_PIN_SD_CARD;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(52) //ESP8266 freq.

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif // HAS_SDIO_CLASS

//==============================================================================
// End of configuration constants.
//------------------------------------------------------------------------------

// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))

class SimpleSdCard
{

#if SD_FAT_TYPE == 0
  SdFat sd;
  File file;
#elif SD_FAT_TYPE == 1
  SdFat32 sd;
  File32 file;
#elif SD_FAT_TYPE == 2
  SdExFat sd;
  ExFile file;
#elif SD_FAT_TYPE == 3
  SdFs sd;
  FsFile file;
#else // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE

  // Serial output stream
  ArduinoOutStream cout = ArduinoOutStream(Serial);
  void cidDmp();

public:
  SimpleSdCard();

  void safeBegin();

#if SD_FAT_TYPE == 0
  File * getFile();
#elif SD_FAT_TYPE == 1
  File32 * getFile();
#elif SD_FAT_TYPE == 2
  ExFile * getFile();
#elif SD_FAT_TYPE == 3
  FsFile * getFile();
#else // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE
};

#endif
