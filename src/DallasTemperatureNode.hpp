/**
 * Homie Node for Dallas sensors.
 *
 */

#pragma once

#include <Homie.hpp>
#include <OneWire.h>
#include <DallasTemperature.h>

// Configurable Request
typedef struct __attribute__((packed)) _entry {
  uint8_t index;
  char property[25];
  char propertyName[25];
  char propertyState[25];
  char propertyStateName[25];
  char deviceAddressStr[24];
  DeviceAddress deviceAddress;
} DallasPropertyEntry, *pDallasPropertyEntry;

typedef struct __attribute__((packed)) _container {
  uint8_t entryCount;
  DallasPropertyEntry entries[8];
} DallasProperties, *pDallasProperties;


class DallasTemperatureNode : public HomieNode {

public:
  DallasTemperatureNode(const char* id, const char* name, const char* nType, const uint8_t pin, const int measurementInterval,
                        bool range, uint16_t lower, uint16_t upper);
  DallasTemperatureNode(const char* id, const char* name, const char* nType, const uint8_t pin, const int measurementInterval);
  DallasTemperatureNode(pDallasProperties request, const char* id, const char* name, const char* nType, const uint8_t pin,
                        const int measurementInterval);

  uint8_t       getPin() const { return _pin; }
  void          setMeasurementInterval(unsigned long interval) { _measurementInterval = interval; }
  unsigned long getMeasurementInterval() const { return _measurementInterval; }
  float         getTemperature() const { return _temperature; }

protected:
  void setup() override;
  void loop() override;
  
private:
  // Total number of Sensors
  static const uint8_t MAX_NUM_SENSORS = 12;  

  // suggested rate is 1/60Hz (1m)
  static const int MIN_INTERVAL         = 60;  // in seconds
  static const int MEASUREMENT_INTERVAL = 300;

  const char* cCaption = "• DallasTemperature sensor:";
  const char* cIndent  = "  ◦ ";

  const char* cTemperature     = "temperature";
  const char* cTemperatureName = "Temperature";
  const char* cTemperatureUnit = "°F";

  const char* cHomieNodeState      = "state";
  const char* cHomieNodeStateName  = "State";
  const char* cHomieNodeStateType   = "enum";
  const char* cHomieNodeStateFormat = "OK,Error,InvalidAddress";

  const char* cHomieNodeState_OK    = "OK";
  const char* cHomieNodeState_Error = "Error";
  const char* cHomieNodeState_Address = "InvalidAddress";

  pDallasProperties requestedProperties = NULL;

  /*
  * We'll use this variable to store a found device address
  * Can only initialize it once as definition; 
  * - thus MAX_NUM_SENSORS is required to allocate enough address containers
  * - could be passed as a class params, but having more than are used is not an issue
  */
  DeviceAddress deviceAddress[MAX_NUM_SENSORS];  

  bool _sensorFound = false;

  uint8_t       _pin;
  unsigned long _measurementInterval;
  unsigned long _lastMeasurement;
  int           _rangeCount;
  float         _temperature = NAN;
  char          chMessageBuffer[256];

  OneWire*           oneWire;
  DallasTemperature* sensor;
  uint8_t            numberOfDevices;  // Number of temperature devices found

  void   initializeSensors();
  void   printCaption();
  String address2String(const DeviceAddress deviceAddress);
  String prepareNodeMessage(uint8_t idx, const char* stateValue, float tempValue);
};
