/*
 * ContactNode.hpp
 * Homie Node for a Contact switch
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#pragma once

#include <Homie.hpp>
#include "SensorNode.hpp"

#define DEFAULTPIN -1
#define DEBOUNCE_TIME 200

class ContactNode : public SensorNode
{
public:
  typedef std::function<void(bool)> TContactCallback;
  ContactNode(const char *id, const char *name, const int contactPin = DEFAULTPIN, TContactCallback contactCallback = NULL, const unsigned long measurementInterval = MEASUREMENT_INTERVAL);
  void onChange(TContactCallback contactCallback);
  void          setMeasurementInterval(unsigned long interval) { _measurementInterval = interval; }
  unsigned long getMeasurementInterval() const { return _measurementInterval; }

private:
  const char *cCaption = "• %s contact pin[%d]:";
//  const unsigned long MIN_INTERVAL         = 60;  // in seconds
  static const unsigned long MEASUREMENT_INTERVAL = 300;
  unsigned long _measurementInterval;
  unsigned long _lastMeasurement;
  int _contactPin;
  TContactCallback _contactCallback;

  // Use invalid values for last states to force sending initial state...
  int _lastInputState = -1; // Input pin state.
  int _lastSentState = -1;  // Last pin state sent
  bool _stateChangeHandled = false;
  unsigned long _stateChangedTime = 0;

  bool debouncePin(void);
  void handleStateChange(bool open);

protected:
  int getContactPin();
  virtual void loop() override;
  virtual void setup() override;
  virtual void setupPin();
  virtual byte readPin();

};
