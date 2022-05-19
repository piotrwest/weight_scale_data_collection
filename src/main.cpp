#include <Arduino.h>
#include "SPI.h"
#include <log4arduino.h>
#include "hardwareConsts.h"
#include "SimpleSdCard.h"
#include <CStringBuilder.h>
#include "ESP8266TrueRandom.h"
#include "HX711.h"
#include "ADS1X15.h"
#include <NTPClient.h>
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"
#include <Adafruit_BME280.h>
#include "wifiIdPass.h"
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"
#include "Wire.h"

#define NAME_POWER "PowerBank"
#define NAME_SHIELD "NoShield"
#define NAME_WEIGHT "0g"
#define NAME_ADS "wADS"
#define NAME_BME280 "wBME280"
#define NAME_FAN "noFAN"
#define NAME_CAPS "wCaps"
#define NAME_DESCRIPTION "NAU7802_10SPSsleeped_3V_PCBTraces_Tesss"
#define MAX_SPS 12 // will limit maximum samples per second by introducing sleep between measures

// log file name
#define TEST_NAME NAME_POWER "_" NAME_SHIELD "_" NAME_WEIGHT "_" NAME_ADS "_" NAME_BME280 "_" NAME_FAN "_" NAME_CAPS "_" NAME_DESCRIPTION "_"

#define LOG_FILE_PREFIX TEST_NAME
#define LOG_FILE_SUFFIX ".csv"

// loop variables
bool LAST_WRITTEN_LED = false;
unsigned long long ITERATION = 1;
uint64_t PREVIOUS_MEASURE_MICROS = micros64();

// cross loop buffer size to avoid frequent SD flushes
#define FLUSH_EVERY_X_ITERATIONS 100
#define SINGLE_LINE_SIZE 256
#define LOOP_BUFFER_SIZE SINGLE_LINE_SIZE * FLUSH_EVERY_X_ITERATIONS
#define LOOP_BUFFER_LOW_SIZE_WARN (SINGLE_LINE_SIZE * FLUSH_EVERY_X_ITERATIONS)/10
char buff[LOOP_BUFFER_SIZE];
CStringBuilder sb(buff, sizeof(buff));

#define CSV_HEADER "iteration,timestamp,testName,millis,freeHeap,vIn,vHX711ref,temperature,humidity,pressure,scale,scaleGain,tempNanRepeat\r\n"

// #define USE_HX_711

#ifdef USE_HX_711
HX711 scale;
#else
NAU7802 nauScale;
#endif

#define SCALE_GAIN 128 //remember to change NAU7802 gain when using nau

ADS1115 ADS(UC_ADS1115_ADDR);
float ADS_MULTIPLIER_ANALOG_0 = 2.00683; // based on res. voltage divider
float ADS_MULTIPLIER_ANALOG_1 = 1.99765; // based on res. voltage divider

Adafruit_BME280 bme;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

SimpleSdCard simpleSdCard;
// #define PRINT_VALS_EVERY_LOOP

#define errorAndHalt(s) \
  LOGS(F(s));           \
  while (true)          \
  {                     \
  }

//------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  LOG_INIT(&Serial);

  simpleSdCard.safeBegin();

  // set LED for work in progress indication
  pinMode(LED_BUILTIN, OUTPUT);

  //ADS1115 setup
  if (!ADS.begin())
  {
    errorAndHalt("ADS.begin failed");
  }
  ADS.setGain(1);

#ifdef USE_HX_711
  // HX711 - weight scale setup
  scale.begin(UC_PIN_HX711_DAT, UC_PIN_HX711_CLK, SCALE_GAIN);
#else
  if (!nauScale.begin(Wire, true))
  {
    errorAndHalt("nauScale.begin failed");
  }
  nauScale.setLDO(NAU7802_LDO_3V0);
  nauScale.setGain(NAU7802_GAIN_128); //Gain can be set to 1, 2, 4, 8, 16, 32, 64, or 128.
  nauScale.setSampleRate(NAU7802_SPS_10); //Sample rate can be set to 10, 20, 40, 80, or 320Hz

  if(!nauScale.calibrateAFE())
  {
    errorAndHalt("nauScale.calibrateAFE failed");
  }
#endif

  //BME280 setup
  if (!bme.begin(UC_BME280_ADDR))
  {
    errorAndHalt("bme.begin failed");
  }
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X8, //temp
                  Adafruit_BME280::SAMPLING_X8, //press
                  Adafruit_BME280::SAMPLING_X8, //hum
                  Adafruit_BME280::FILTER_X2,
                  Adafruit_BME280::STANDBY_MS_20);

  //Connect to WiFi, sync time and disconnect
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    LOG("Connecting to: %s...", WIFI_ID);
    delay(200);
    digitalWrite(LED_BUILTIN, true);
    delay(200);
    digitalWrite(LED_BUILTIN, false);
  }
  timeClient.begin();
  while (!timeClient.update())
  {
    LOG("Getting time...");
    delay(200);
    digitalWrite(LED_BUILTIN, true);
    delay(200);
    digitalWrite(LED_BUILTIN, false);
  }
  WiFi.disconnect(false);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  LOG("Memory FreeHeap before open SD: %u bytes", ESP.getFreeHeap());
  // open/create/append
  if (!(simpleSdCard.getFile())->open((LOG_FILE_PREFIX + String(timeClient.getEpochTime()) + LOG_FILE_SUFFIX).c_str(), O_RDWR | O_CREAT | O_AT_END | O_AT_END))
  {
    errorAndHalt("file.open failed");
  }
  (simpleSdCard.getFile())->write(CSV_HEADER);
  LOG("Memory FreeHeap after open SD: %u bytes", ESP.getFreeHeap());
}

void capture_measures(CStringBuilder sb)
{
#ifdef PRINT_VALS_EVERY_LOOP
  LOG("Iteration: %llu", ITERATION);
#endif
  sb.print(ITERATION);
  sb.print(',');

  unsigned long long timestamp = ((unsigned long long)timeClient.getEpochTime() * 1000) + (millis() % 1000);
#ifdef PRINT_VALS_EVERY_LOOP
  LOG("Time: %llu", timestamp);
#endif
  sb.print(timestamp);
  sb.print(',');

#ifdef PRINT_VALS_EVERY_LOOP
  LOG(TEST_NAME);
#endif
  sb.print(TEST_NAME);
  sb.print(',');

#ifdef PRINT_VALS_EVERY_LOOP
  LOG("millis: %ld", millis());
#endif
  sb.print(millis());
  sb.print(',');

#ifdef PRINT_VALS_EVERY_LOOP
  LOG("ESP: %ld", ESP.getFreeHeap());
#endif
  sb.print(ESP.getFreeHeap());
  sb.print(',');

  int16_t val_0 = ADS.readADC(0);
  int16_t val_1 = ADS.readADC(1);
  float f = ADS.toVoltage(1); // voltage factor
  float v_in = val_0 * f * ADS_MULTIPLIER_ANALOG_0;
  float v_hx711_ref = val_1 * f * ADS_MULTIPLIER_ANALOG_1;
#ifdef PRINT_VALS_EVERY_LOOP
  LOG("V_in: %.6f", v_in);
  LOG("V_HX711_ref: %.6f", v_hx711_ref);
#endif
  sb.printf("%.6f,", v_in);
  sb.printf("%.6f,", v_hx711_ref);

  int tempNanRepeat = -1;
  float temperature = NAN;
  while (true) {
    temperature = bme.readTemperature(); // sometimes temperature is returned as NAN
    tempNanRepeat += 1;
    if (!isnan(temperature)) {
      break;
    }
    if (tempNanRepeat > 50) { // trying to capture it only for 50ms
      break;
    }
    delay(1);
  }
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
#ifdef PRINT_VALS_EVERY_LOOP
  LOG("temperature: %.6f", temperature);
  LOG("humidity: %.6f", humidity);
  LOG("pressure: %.6f", pressure);
#endif
  sb.printf("%.6f,", temperature);
  sb.printf("%.6f,", humidity);
  sb.printf("%.6f,", pressure);

  uint64_t microsDiff = micros64() - PREVIOUS_MEASURE_MICROS;
  if (microsDiff < ((1*1000*1000)/MAX_SPS)) {
    delayMicroseconds(((1*1000*1000)/MAX_SPS)-microsDiff);
  }
#ifdef USE_HX_711
  long scaleRead = scale.read();
#else
  long scaleRead = nauScale.getReading();
#endif
#ifdef PRINT_VALS_EVERY_LOOP
  LOG("scale: %ld", scaleRead);
#endif
  sb.printf("%ld,", scaleRead);
  PREVIOUS_MEASURE_MICROS = micros64();

#ifdef PRINT_VALS_EVERY_LOOP
  LOG("scale gain: %d", SCALE_GAIN);
#endif
  sb.printf("%d,", SCALE_GAIN);

#ifdef PRINT_VALS_EVERY_LOOP
  LOG("tempNanRepeat: %d", tempNanRepeat);
#endif
  sb.printf("%d", tempNanRepeat);
  sb.print("\r\n");
}

void loop()
{
  sb.reset();
  capture_measures(sb);
  if (sb.availableForWrite() < LOOP_BUFFER_LOW_SIZE_WARN)
  {
    LOG("ERROR!! INTER-LOOP BUFFER IS TO SMALL!! AVAILABLE SPACE FOR WRITE LEFT: %d", sb.availableForWrite());
  }
  (simpleSdCard.getFile())->write(buff);

  ITERATION += 1;
  if (ITERATION % FLUSH_EVERY_X_ITERATIONS == 0)
  {
    (simpleSdCard.getFile())->flush();
    LAST_WRITTEN_LED = !LAST_WRITTEN_LED;
    digitalWrite(LED_BUILTIN, LAST_WRITTEN_LED);
  }
  // on next iteration after flush, to track progress, print buffer
  if (ITERATION % FLUSH_EVERY_X_ITERATIONS == 1)
  {
    LOG("CAPTURED: %s", buff);
  }
}