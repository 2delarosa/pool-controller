/**
 * Smart Swimming Pool - Pool Contoller
 *
 * Main entry of sketch.
 */

#include <Arduino.h>
#include <Homie.h>
#include <SPI.h>
#include "DallasTemperatureNode.hpp"
#include "ESP32TemperatureNode.hpp"
#include "RelayModuleNode.hpp"
#include "OperationModeNode.hpp"
#include "Rule.hpp"
#include "RuleManu.hpp"
#include "RuleAuto.hpp"
#include "RuleBoost.hpp"
#include "RuleTimer.hpp"
#include "ContactNode.hpp"

#include "LoggerNode.hpp"
#include "TimeClientHelper.hpp"


#ifdef ESP32
const uint8_t PIN_DS_SOLAR = 15;  // Pin of Temp-Sensor Solar
const uint8_t PIN_DS_POOL  = 16;  // Pin of Temp-Sensor Pool

const uint8_t PIN_RELAY_POOL  = 18;
const uint8_t PIN_RELAY_SOLAR = 19;
#elif defined(ESP8266)

const uint8_t DTN_RANGE_LOWER = 0;
const uint8_t DTN_RANGE_UPPER = 3;

// see: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
const uint8_t PIN_DS_SOLAR = D3;  // Pin of Temp-Sensor Solar
const uint8_t PIN_DS_POOL  = D4;  // Pin of Temp-Sensor Pool

const uint8_t PIN_RELAY_POOL  = D2;
const uint8_t PIN_RELAY_SOLAR = D1;
const uint8_t PIN_RELAY_PLIGHTS = D5;
const uint8_t PIN_RELAY_HEATER = D6;
const uint8_t PIN_RELAY_SUCTION = D7;
const uint8_t PIN_RELAY_RETURN = D8;
const uint8_t PIN_CONTACT = RX; // GPIO 3

#endif
const uint8_t TEMP_READ_INTERVALL = 30;  //Sekunden zwischen Updates der Temperaturen.


HomieSetting<long> loopIntervalSetting("loop-interval", "The processing interval in seconds");

HomieSetting<double> temperatureMaxPoolSetting("temperature-max-pool", "Maximum temperature of solar");
HomieSetting<double> temperatureMinSolarSetting("temperature-min-solar", "Minimum temperature of solar");
HomieSetting<double> temperatureHysteresisSetting("temperature-hysteresis", "Temperature hysteresis");

HomieSetting<const char*> operationModeSetting("operation-mode", "Operational Mode");

LoggerNode LN;

/*
15:23:17.400 >   ◦ Temperature=74.97 for address=28e20b943c1901a3
15:23:17.414 >   ◦ Temperature=75.43 for address=2866fc543c19015b
15:23:17.427 >   ◦ Temperature=75.31 for address=28f957453c19013a
15:23:17.440 >   ◦ Temperature=74.52 for address=28fd883f3c190164
*/
DallasProperties poolRequestUnKnown = {5, 
  { //   add "" empty in the hex address field for as-is assignment           format: 28e20b943c1901a3
    {0, "tempSucPool",   "Pool Suction Temp",  "tempSucPoolState",   "Pool Suction State",  ""},
    {1, "tempRetPool",   "Pool Return Temp",   "tempRetPoolState",   "Pool Return State",   ""},
    {2, "tempSucSpa",    "Pool Suction Temp",  "tempSucSpaState",    "Pool Suction State",  ""},
    {3, "tempRetSpa",    "SPA Return Temp",    "tempRetSpaState",    "SPA Return State",    ""},
    {4, "tempRetHeater", "Heater Return Temp", "tempRetHeaterState", "Heater Return State", ""}
  }
};

DallasProperties poolRequest = {5, 
  { //   add the hex addresses in the proper order for EVERY entry                   format: 28e20b943c1901a3
    {0, "tempSucPool",   "Pool Suction Temp",  "tempSucPoolState",   "Pool Suction State",  "2866fc543c19015b"},
    {1, "tempRetPool",   "Pool Return Temp",   "tempRetPoolState",   "Pool Return State",   "28f957453c19013a"},
    {2, "tempSucSpa",    "Pool Suction Temp",  "tempSucSpaState",    "Pool Suction State",  "28e20b943c1901a3"},
    {3, "tempRetSpa",    "SPA Return Temp",    "tempRetSpaState",    "SPA Return State",    "28fd883f3c190164"},
    {4, "tempRetHeater", "Heater Return Temp", "tempRetHeaterState", "Heater Return State", "28f957453c19013a"}
  }
};

DallasTemperatureNode solarTemperatureNode("solar-temp", "Solar Temperature", "Ambient", PIN_DS_SOLAR, TEMP_READ_INTERVALL);

DallasTemperatureNode poolTemperatureNode(&poolRequestUnKnown, "pool-temp", "Pool Temperature", "Ambient", PIN_DS_POOL, TEMP_READ_INTERVALL);  // Dynamic
// DallasTemperatureNode poolTemperatureNode("pool-temp", "Pool Temperature", "Ambient", PIN_DS_POOL, TEMP_READ_INTERVALL); // JSON
// DallasTemperatureNode poolTemperatureNode("pool-temp", "Pool Temperature", "Ambient", PIN_DS_POOL, TEMP_READ_INTERVALL, true, DTN_RANGE_LOWER, DTN_RANGE_UPPER); // HomieRange

#ifdef ESP32
ESP32TemperatureNode ctrlTemperatureNode("controller-temp", "Controller Temperature", TEMP_READ_INTERVALL);
#endif

RelayModuleNode poolPumpNode("pool-pump", "Pool Pump", PIN_RELAY_POOL);
RelayModuleNode solarPumpNode("solar-pump", "Solar Pump", PIN_RELAY_SOLAR);
RelayModuleNode poolLightNode("pool-lights", "Pool Lights", PIN_RELAY_PLIGHTS);
RelayModuleNode poolHeaterNode("pool-heater", "Heater", PIN_RELAY_HEATER);
RelayModuleNode poolSuctionNode("pool-suction", "Suction", PIN_RELAY_SUCTION);
RelayModuleNode poolReturnNode("pool-return", "Return ", PIN_RELAY_RETURN);

ContactNode contactNode("waterflow", "Water Flow", PIN_CONTACT);

OperationModeNode operationModeNode("operation-mode", "Operation Mode");

unsigned long _measurementInterval = 10;
unsigned long _lastMeasurement;

/**
 * Homie Setup handler.
 * Only called when wifi and mqtt are connected.
 */
void setupHandler() {

  // set mesurement intervals
  long _loopInterval = loopIntervalSetting.get();

  solarTemperatureNode.setMeasurementInterval(_loopInterval);
  poolTemperatureNode.setMeasurementInterval(_loopInterval);

  poolPumpNode.setMeasurementInterval(_loopInterval);
  solarPumpNode.setMeasurementInterval(_loopInterval);
  poolLightNode.setMeasurementInterval(_loopInterval);
  poolHeaterNode.setMeasurementInterval(_loopInterval);
  poolSuctionNode.setMeasurementInterval(_loopInterval);
  poolReturnNode.setMeasurementInterval(_loopInterval);

//  contactNode.setMeasurementInterval(_loopInterval);

#ifdef ESP32
  ctrlTemperatureNode.setMeasurementInterval(_loopInterval);
#endif

  operationModeNode.setMode(operationModeSetting.get());
  operationModeNode.setPoolMaxTemperatur(temperatureMaxPoolSetting.get());
  operationModeNode.setSolarMinTemperature(temperatureMinSolarSetting.get());
  operationModeNode.setTemperaturHysteresis(temperatureHysteresisSetting.get());
  TimerSetting ts = operationModeNode.getTimerSetting(); //TODO: Configurable
  ts.timerStartHour = 10;
  ts.timerStartMinutes = 30;
  ts.timerEndHour = 17;
  ts.timerEndMinutes = 30;
  operationModeNode.setTimerSetting(ts);

  operationModeNode.setPoolTemperaturNode(&poolTemperatureNode);
  operationModeNode.setSolarTemperatureNode(&solarTemperatureNode);

  // add the rules
  RuleAuto* autoRule = new RuleAuto(&solarPumpNode, &poolPumpNode);
  operationModeNode.addRule(autoRule);

  RuleManu* manuRule = new RuleManu();
  operationModeNode.addRule(manuRule);

  RuleBoost* boostRule = new RuleBoost(&solarPumpNode, &poolPumpNode);
  operationModeNode.addRule(boostRule);

  RuleTimer* timerRule = new RuleTimer(&solarPumpNode, &poolPumpNode);
  operationModeNode.addRule(timerRule);

  _lastMeasurement = 0;
}

/**
 * Startup of controller.
 */
void setup() {
  Serial.begin(SERIAL_SPEED);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Homie.setLoggingPrinter(&Serial);

  Homie_setFirmware("pool-controller", "2.0.0");
  Homie_setBrand("smart-swimmingpool");

  //WiFi.setSleepMode(WIFI_NONE_SLEEP); //see: https://github.com/esp8266/Arduino/issues/5083

  //default intervall of sending Temperature values
  loopIntervalSetting.setDefaultValue(TEMP_READ_INTERVALL).setValidator([](long candidate) {
    return (candidate >= 0) && (candidate <= 300);
  });

  temperatureMaxPoolSetting.setDefaultValue(75.5).setValidator(
      [](long candidate) { return (candidate >= 0) && (candidate <= 100); });

  temperatureMinSolarSetting.setDefaultValue(100.0).setValidator(
      [](long candidate) { return (candidate >= 0) && (candidate <= 120); });

  temperatureHysteresisSetting.setDefaultValue(1.0).setValidator(
      [](long candidate) { return (candidate >= 0) && (candidate <= 10); });

  operationModeSetting.setDefaultValue("manu").setValidator([](const char* candidate) {
    return (strcmp(candidate, "auto")) || (strcmp(candidate, "manu")) || (strcmp(candidate, "boost"));
  });

  //Homie.disableLogging();
  Homie.setSetupFunction(setupHandler);

  LN.log(__PRETTY_FUNCTION__, LoggerNode::DEBUG, "Before Homie setup())");
  Homie.setup();

  LN.logf(__PRETTY_FUNCTION__, LoggerNode::DEBUG, "Free heap: %d", ESP.getFreeHeap());
  Homie.getLogger() << F("Free heap: ") << ESP.getFreeHeap() << endl;
}

/**
 * Main loop of ESP.
 */
void loop() {

  Homie.loop();
}
