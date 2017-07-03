// Telemetry.h

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include <map>
#include <deque>
#include <queue>
#include <iostream>
#include <algorithm>

#include "Macros.h"
#include "Segment.h"
#include "SNMPDefines.h"
#include "StateMachine.h"

class CTelemetry : public CStateMachine, CNonCopyable
{
  CLASS_TYPE_1(CTelemetry, CStateMachine)

 public:
  CTelemetry();

  // Enables or disables sliding window timer.
  bool EnableSlider(bool enable);

  // Updates data of each element of the slider.
  void Update(const CTelemetryValue& value);

  // Obtains data of the front element of the slider (60 seconds).
  CTelemetryValue Get(eTelemetryType type) const;

 private:
  // A timer with value of "m_interval".
  void                OnTimerCalculate();
  virtual void        Dump(std::ostream& out) const;
  virtual const char* NameOf() const;
  unsigned int        Sum(eTelemetryType t1, eTelemetryType t2) const;
  unsigned int        Ratio(eTelemetryType t1, eTelemetryType t2) const;
  unsigned int        RatioOnSum(eTelemetryType t1, eTelemetryType t2) const;
  static unsigned int CalculateRatio(unsigned int v, unsigned int t);

  std::deque<std::map<eTelemetryType, CTelemetryValue> > m_slider;
  std::map<eTelemetryType, CTelemetryValue>              m_prolonger;

  PDECLAR_MESSAGE_MAP;
};

#endif  // TELEMETRY_H_
