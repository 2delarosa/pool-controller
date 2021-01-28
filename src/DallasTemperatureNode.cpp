/**
 * Homie Node for Maxime Temperature sensors.
 *
 * This Node supports the following devices :
 *  * DS18B20
 *  * DS18S20 - Please note there appears to be an issue with this series.
 *  * DS1822
 *  * DS1820
 *  * MAX31820
 *
 * You will need a pull-up resistor of about 5 KOhm between the 1-Wire data line and your 5V power.
 * If you are using the DS18B20, ground pins 1 and 3. The centre pin is the data line '1-wire'.
 *
 * Used lib:
 * https://github.com/milesburton/Arduino-Temperature-Control-Library
 * https://www.milesburton.com/Dallas_Temperature_Control_Library
 *
 */
#include "DallasTemperatureNode.hpp"

DallasTemperatureNode::DallasTemperatureNode(const char* id, const char* name, const uint8_t pin, const int measurementInterval)    
    : DallasTemperatureNode(id, name, pin, measurementInterval, false, 0U, 0U) {
}

DallasTemperatureNode::DallasTemperatureNode(const char* id, const char* name, const uint8_t pin, const int measurementInterval,
                                             bool range, uint16_t lower, uint16_t upper)
    : HomieNode(id, name, "temperature", range, lower, upper) {
  _pin                 = pin;
  _measurementInterval = (measurementInterval > MIN_INTERVAL) ? measurementInterval : MIN_INTERVAL;
  _lastMeasurement     = 0;
  _rangeCount          = upper - lower;

  oneWire = new OneWire(_pin);
  sensor  = new DallasTemperature(oneWire);

  // Start up the library
  sensor->begin();

  // set global resolution to 9, 10, 11, or 12 bits
  //sensor->setResolution(12);
}

/**
 * Called by Homie when Homie.setup() is called; Once!
 */
  void DallasTemperatureNode::setup() {    
    advertise(cHomieNodeState).setName(cHomieNodeStateName);
    if (isRange()) {
      advertise(cTemperature).setName(cTemperatureRangeName).setDatatype("float").setUnit(cTemperatureUnit);
    } else {
      advertise(cTemperature).setName(cTemperatureName).setDatatype("float").setUnit(cTemperatureUnit);
    }
    initializeSensors();
  }

/**
 * Called by HomieSetup() and HomieLoop() as needed
 * - HomieSetup is not ONLINE, so no sends or adverts
 */
  void DallasTemperatureNode::initializeSensors() {
    // Grab a count of devices on the wire
    numberOfDevices = sensor->getDeviceCount();

    // Constrain count to range, if range
    if ((numberOfDevices > 0) && isRange() && (numberOfDevices > _rangeCount)) {
      numberOfDevices = _rangeCount;
    }

    // Constrain count to Address Containers in every case
    if (numberOfDevices > MAX_NUM_SENSORS) {
      numberOfDevices = MAX_NUM_SENSORS;
    }

    // report parasite power requirements
    Homie.getLogger() << cIndent << F("Parasite power is: ") << sensor->isParasitePowerMode() << endl;

    if (numberOfDevices > 0) {
      Homie.getLogger() << cIndent << numberOfDevices << F(" devices found on PIN ") << _pin << endl;

      for (uint8_t i = 0; i < numberOfDevices; i++) {
        // Load the address list sequence
        if (sensor->getAddress(deviceAddress[i], i)) {
          String adr = address2String(deviceAddress[i]);
          Homie.getLogger() << cIndent 
                            << F("PIN ") << _pin << F(": ") 
                            << F("Device ") << i 
                            << F(" using address ") << adr
                            << endl;
        }
      }
    }
  }

/**
 * Called by Homie when homie is connected and in run mode
 */
  void DallasTemperatureNode::loop() {
    if (millis() - _lastMeasurement >= _measurementInterval * 1000UL || _lastMeasurement == 0) {
      _lastMeasurement = millis();

      if (numberOfDevices > 0) {
        Homie.getLogger() << F("〽 Sending Temperature: ") << getId() << endl;
        HomieRange sensorRange = {true, 0};
        // call sensors.requestTemperatures() to issue a global temperature
        // request to all devices on the bus
        sensor->requestTemperatures();  // Send the command to get temperature readings
        for (uint8_t i = 0; i < numberOfDevices; i++) {

          if ( sensor->validAddress(deviceAddress[i]) ) {  // make sure we have an address
            sensorRange.index = i;
            _temperature           = sensor->getTempF(deviceAddress[i]);  // Changed getTempC to getTempF
            if (DEVICE_DISCONNECTED_C == _temperature) {
              Homie.getLogger() << cIndent
                                << F("✖ Error reading sensor") 
                                << address2String(deviceAddress[i]) 
                                << ". Request count: " << i
                                << endl;

              setProperty(cHomieNodeState).send(cHomieNodeState_Error);
            } else {
              Homie.getLogger() << cIndent 
                                << F("Temperature=") 
                                << _temperature
                                << "for address=" 
                                << address2String(deviceAddress[i]) 
                                << endl;

              if (isRange()) {
                setProperty(cTemperature).setRange(sensorRange).send(String(_temperature));
              }else {
                setProperty(cTemperature).send(String(_temperature));
              }
                setProperty(cHomieNodeState).send(cHomieNodeState_OK);
              }
          }
        }
      } else {

        Homie.getLogger() << F("No Sensor found!") << endl;
        setProperty(cHomieNodeState).send(cHomieNodeState_Error);
        //re-init
        initializeSensors();
      }
    }
  }

  /**
 *
 */
  void DallasTemperatureNode::printCaption() { Homie.getLogger() << cCaption << endl; }

  /**
 *
 */
  String DallasTemperatureNode::address2String(const DeviceAddress deviceAddress) {
    String adr;

    for (uint8_t i = 0; i < 8; i++) {
      // zero pad the address if necessary

      if (deviceAddress[i] < 16) {
        adr = adr + F("0");
      }
      adr = adr + String(deviceAddress[i], HEX);
    }

    return adr;
  }
