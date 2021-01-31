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

DallasTemperatureNode::DallasTemperatureNode(pDallasProperties request, const char* id, const char* name, const char* nType,
                                             const uint8_t pin, const int measurementInterval)
    : DallasTemperatureNode(id, name, nType, pin, measurementInterval, false, 0U, 0U) {
  requestedProperties = request;
}

DallasTemperatureNode::DallasTemperatureNode(const char* id, const char* name, const char* nType, const uint8_t pin,
                                             const int measurementInterval)
    : DallasTemperatureNode(id, name, nType, pin, measurementInterval, false, 0U, 0U) {}

DallasTemperatureNode::DallasTemperatureNode(const char* id, const char* name, const char* nType, const uint8_t pin,
                                             const int measurementInterval, bool range, uint16_t lower, uint16_t upper)
    : HomieNode(id, name, nType, range, lower, upper) {
  _pin                 = pin;
  _measurementInterval = (measurementInterval > MIN_INTERVAL) ? measurementInterval : MIN_INTERVAL;
  _lastMeasurement     = 0;
  _rangeCount          = 1 + (upper - lower);

  oneWire = new OneWire(_pin);
  sensor  = new DallasTemperature(oneWire);

  // Start up the library
  sensor->begin();

  // set global resolution to 9, 10, 11, or 12 bits
  // sensor->setResolution(12);
}

 /**
  * Called by Homie when Homie.setup() is called; Once!
  */
  void DallasTemperatureNode::setup() {
      initializeSensors();
  
    if(isRange()) {
      advertise(cHomieNodeState).setName(cHomieNodeStateName).setDatatype(cHomieNodeStateType).setFormat(cHomieNodeStateFormat);
      advertise(cTemperature).setName(cTemperatureName).setDatatype("float").setUnit(cTemperatureUnit);

    } else if (NULL != requestedProperties) {
      for (uint8_t i = 0; i < requestedProperties->entryCount; i++) {
        advertise(requestedProperties->entries[i].propertyState)
            .setName(requestedProperties->entries[i].propertyStateName)
            .setDatatype(cHomieNodeStateType)
            .setFormat(cHomieNodeStateFormat);
        advertise(requestedProperties->entries[i].property)
            .setName(requestedProperties->entries[i].propertyName)
            .setDatatype("float")
            .setUnit(cTemperatureUnit);
      }

    } else {
      advertise(cHomieNodeState).setName(cHomieNodeStateName).setDatatype(cHomieNodeStateType).setFormat(cHomieNodeStateFormat);
      advertise(cTemperature).setName(cTemperatureName).setDatatype("string");
    }
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

    // Constrain count to entryCount, if Requested
    if ((NULL != requestedProperties) && (numberOfDevices != requestedProperties->entryCount)) {
      numberOfDevices  = requestedProperties->entryCount;
    }

    // Constrain count to Address Containers in every case
    if (numberOfDevices > MAX_NUM_SENSORS) {
      numberOfDevices = MAX_NUM_SENSORS;
    }

    // report parasite power requirements
    Homie.getLogger() << cIndent 
                      << F("Parasite power is: ") << sensor->isParasitePowerMode() 
                      << endl;

    if (numberOfDevices > 0) {
      Homie.getLogger() << cIndent 
                        << numberOfDevices 
                        << F(" devices found on PIN ") << _pin 
                        << endl;
      
      for (uint8_t i = 0; i < numberOfDevices; i++) {
        // Load the address list sequence

        if (NULL != requestedProperties) {
          // load unless address was provided
          if ('\0' == requestedProperties->entries[i].deviceAddressStr[0]) {
            sensor->getAddress(deviceAddress[i], i);
            HomieInternals::Helpers::byteArrayToHexString(deviceAddress[i], requestedProperties->entries[i].deviceAddressStr, sizeof(DeviceAddress));
          }
          HomieInternals::Helpers::hexStringToByteArray(requestedProperties->entries[i].deviceAddressStr, requestedProperties->entries[i].deviceAddress, sizeof(DeviceAddress));
          HomieInternals::Helpers::hexStringToByteArray(requestedProperties->entries[i].deviceAddressStr, deviceAddress[i], sizeof(DeviceAddress));

          Homie.getLogger() << cIndent 
                          << F("PIN ") << _pin << F(": ") 
                          << F("Device ") << i 
                          << F(" using address ") << requestedProperties->entries[i].deviceAddressStr
                          << ", Property Name: " << requestedProperties->entries[i].property
                          << ", PropertyState Name: " << requestedProperties->entries[i].propertyState
                          << endl;
        } else {            
          sensor->getAddress(deviceAddress[i], i);
          HomieInternals::Helpers::byteArrayToHexString(deviceAddress[i], chMessageBuffer, sizeof(DeviceAddress));
          Homie.getLogger() << cIndent 
                            << F("PIN ") << _pin << F(": ") 
                            << F("Device ") << i 
                            << F(" using address ") << chMessageBuffer 
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
      HomieRange sensorRange = {true, 0};
      DeviceAddress *workingAddress;

      if (numberOfDevices > 0) {
        Homie.getLogger() << F("〽 Sending Temperature: ") << getId() << endl;        
        // call sensors.requestTemperatures() to issue a global temperature
        // request to all devices on the bus
        sensor->requestTemperatures();  // Send the command to get temperature readings
        for (uint8_t i = 0; i < numberOfDevices; i++) {

          if (NULL != requestedProperties) {
            workingAddress = &requestedProperties->entries[i].deviceAddress;
          } else {
            workingAddress = &deviceAddress[i];
          }

          if ( sensor->validAddress(*workingAddress) ) {  // make sure we have an address
            sensorRange.index = i;
            _temperature = sensor->getTempF(*workingAddress);  // According to request

            if ((_temperature > 184.0) || (DEVICE_DISCONNECTED_F == _temperature))
            {
              HomieInternals::Helpers::byteArrayToHexString(*workingAddress, chMessageBuffer, sizeof(DeviceAddress));
              Homie.getLogger() << cIndent 
                                << F("✖ Error reading sensor") 
                                << chMessageBuffer 
                                << ". Request count: " << i
                                << ", value read=" << _temperature << endl;
              if (isRange())
              {
                setProperty(cHomieNodeState)
                    .setRange(sensorRange)
                    .setRetained(true)
                    .send(cHomieNodeState_Error);
              } else if (NULL != requestedProperties) {
                setProperty(requestedProperties->entries[i].propertyState)
                    .setRetained(true)
                    .send(cHomieNodeState_Error);
              } else {
                setProperty(cHomieNodeState)
                    .setRetained(true)
                    .send(prepareNodeMessage(sensorRange.index, cHomieNodeState_Error, 0.0F));
              }
            }
            else
            {
              HomieInternals::Helpers::byteArrayToHexString(*workingAddress, chMessageBuffer, sizeof(DeviceAddress));
              Homie.getLogger() << cIndent 
                                << F("Temperature=") 
                                << _temperature 
                                << " for address=" 
                                << chMessageBuffer 
                                << endl;

              if (isRange()) {
                setProperty(cHomieNodeState)
                    .setRange(sensorRange)
                    .setRetained(true)
                    .send(cHomieNodeState_OK);
                setProperty(cTemperature)
                    .setRange(sensorRange)
                    .setRetained(true)
                    .send(String(_temperature));
              } else if (NULL != requestedProperties) {
                setProperty(requestedProperties->entries[i].property)
                    .setRetained(true)
                    .send(String(_temperature));
                setProperty(requestedProperties->entries[i].propertyState)
                    .setRetained(true)
                    .send(cHomieNodeState_OK);
              } else {
                setProperty(cHomieNodeState)
                    .setRetained(true)
                    .send(prepareNodeMessage(sensorRange.index, cHomieNodeState_OK, 0.0F));
                setProperty(cTemperature)
                    .setRetained(true)
                    .send(prepareNodeMessage(sensorRange.index, NULL, _temperature));
              }
            }
          } else { // if address is invalid
            HomieInternals::Helpers::byteArrayToHexString(*workingAddress, chMessageBuffer, sizeof(DeviceAddress));
            Homie.getLogger() << cIndent 
                              << F("✖ Error reading sensor") 
                              << chMessageBuffer 
                              << ". Request count: " << i
                              << ", Invalid Address!" 
                              << endl;
            if (isRange()) {
              setProperty(cHomieNodeState)
                  .setRange(sensorRange)
                  .setRetained(true)
                  .send(cHomieNodeState_Address);
              } else if (NULL != requestedProperties) {
                setProperty(requestedProperties->entries[i].propertyState)
                    .setRetained(true)
                    .send(cHomieNodeState_Address);
              } else {
                setProperty(cHomieNodeState)
                    .setRetained(true)
                    .send(prepareNodeMessage(sensorRange.index, cHomieNodeState_Address, 0.0F));
              }
          }
        } // loop end
      } else { // Node Failure with no devices
        Homie.getLogger() << F("No Sensor found!") << endl;
        setProperty("$state").send("alert");
        
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

  /**
   * @brief Build JSON messages for non-range mode
   * 
   *  {"1":{"deviceAddress":"28e20b943c1901a3","State":"Ok"}}
   *  {"1":{"deviceAddress":"28e20b943c1901a3","Temperature":72.5}}
   */
  String DallasTemperatureNode::prepareNodeMessage(uint8_t idx, const char* stateValue, float tempValue = 0.0F) {
    if (NULL == stateValue) {
      snprintf(chMessageBuffer, sizeof(chMessageBuffer), "{\"%d\":{\"deviceAddress\":\"%s\",\"Temperature\":%f}}", idx,
               address2String(deviceAddress[idx]).c_str(), tempValue);
    } else {
      snprintf(chMessageBuffer, sizeof(chMessageBuffer), "{\"%d\":{\"deviceAddress\":\"%s\",\"State\":\"%s\"}}", idx,
               address2String(deviceAddress[idx]).c_str(), stateValue);
    }

    Homie.getLogger() << "Payload=" << chMessageBuffer << endl;

    return String(chMessageBuffer);
  }
