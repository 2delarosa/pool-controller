
#pragma once

#include "Timer.hpp"

class Rule {

public:
  Rule() : _poolTemp(0.0), _solarTemp(0.0), _poolMaxTemp(0.0), _solarMinTemp(0.0), _hysteresis(0.0){};

  void  setPoolTemperatur(float temp) { _poolTemp = temp; };
  float getPoolTemperature() { return _poolTemp; };
  void  setSolarTemperatur(float temp) { _solarTemp = temp; };
  float getSolarTemperature() { return _solarTemp; };

  void  setPoolMaxTemperatur(float temp) { _poolMaxTemp = temp; };
  float getPoolMaxTemperature() { return _poolMaxTemp; };

  void  setSolarMinTemperature(float temp) { _solarMinTemp = temp; };
  float getSolarMinTemperature() { return _solarMinTemp; };

  void  setTemperaturHysteresis(float temp) { _hysteresis = temp; };
  float getTemperaturHysteresis() { return _hysteresis; };

  void  setTimerSetting(TimerSetting setting) { _timerSetting = setting; };
  TimerSetting getTimerSetting() { return _timerSetting; };


  /**
   * get the Mode for which the Rule is created.
   */
  virtual const char* getMode();
  virtual void        loop();

protected:
  float _poolTemp;
  float _solarTemp;

  float _poolMaxTemp;
  float _solarMinTemp;

  float _hysteresis;

  TimerSetting _timerSetting;
};
