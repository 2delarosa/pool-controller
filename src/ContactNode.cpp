/*
 * ContactNode.cpp
 * Homie Node for a Contact switch
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#include "ContactNode.hpp"

ContactNode::ContactNode(const char *id,
                         const char *name,
                         const int contactPin,
                         TContactCallback contactCallback,
                         const unsigned long measurementInterval)
    : SensorNode(id, name, "Contact")
//      _contactPin(contactPin),
//      _contactCallback(contactCallback),
//      _measurementInterval(measurementInterval)
{
  _contactPin = contactPin;
  _contactCallback = contactCallback;
  _measurementInterval = measurementInterval;
  asprintf(&_caption, cCaption, name, contactPin);
}

int ContactNode::getContactPin()
{
  return _contactPin;
}

byte ContactNode::readPin()
{
  return digitalRead(_contactPin);
}

// Debounce input pin.
bool ContactNode::debouncePin(void)
{
  byte inputState = readPin();
  if (inputState != _lastInputState)
  {
    _stateChangedTime = millis();
    _stateChangeHandled = false;
    _lastInputState = inputState;
#ifdef DEBUG
    Homie.getLogger() << "State Changed to " << inputState << endl;
#endif
  }
  else
  {
    unsigned long dt = millis() - _stateChangedTime;
    if (dt >= DEBOUNCE_TIME && !_stateChangeHandled)
    {
#ifdef DEBUG
      Homie.getLogger() << "State Stable for " << dt << "ms" << endl;
#endif
      _stateChangeHandled = true;
      return true;
    }
  }
  return false;
}

void ContactNode::handleStateChange(bool open)
{
  if (Homie.isConnected())
  {
    setProperty("open").send(open ? "true" : "false");
  }
  if (_contactCallback)
  {
    _contactCallback(open);
  }

  printCaption();
  Homie.getLogger() << cIndent << "is " << (open ? "open" : "closed") << endl;
}

void ContactNode::onChange(TContactCallback contactCallback)
{
  _contactCallback = contactCallback;
}

void ContactNode::loop()
{
  if (millis() - _lastMeasurement >= _measurementInterval * 1000UL || _lastMeasurement == 0) {
    _lastMeasurement = millis();
    if (_contactPin > DEFAULTPIN)
    {
      if (debouncePin() && (_lastSentState != _lastInputState))
      {
        handleStateChange(_lastInputState == HIGH);
        _lastSentState = _lastInputState;
      }
    }
    Homie.getLogger() << F("〽 Contact Status: ") << getId() << F(" switch: ") << (_lastSentState ? "open" : "closed") << endl;
  }
}

void ContactNode::setupPin()
{
  pinMode(_contactPin, INPUT_PULLUP);
}

void ContactNode::setup()
{
  advertise("open").setDatatype("boolean");

  printCaption();

  if (_contactPin > DEFAULTPIN)
  {
    setupPin();
  }
}
