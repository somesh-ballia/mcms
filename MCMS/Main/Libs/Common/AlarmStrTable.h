// AlarmStrTable.h 

#ifndef ALARM_STR_TABLE_H_
#define ALARM_STR_TABLE_H_

#include <string>
#include <iostream>

#include "ConvertorBase.h"

class CXMLDOMElement;

// First field is name, second is description.
typedef std::pair<std::string, std::string> AlarmValue;

class CAlarmStringConverter : public CConvertorBase<int, AlarmValue>
{
 public:
  static CAlarmStringConverter& Instance();

  virtual const char* NameOf() const { return "CAlarmStringConverter"; }
  void                AddAlarm(int         code,
                               const char* name,
                               const char* desc,
                               bool        is_alarm);
  const std::string&  GetName(int code) const;
  const std::string&  GetDesc(int code) const;

  void                SerializeFaultsAndAlarms(CXMLDOMElement* alarms,
																							 CXMLDOMElement* faults) const;

  const std::map<int, AlarmValue>& GetMap() const {return *this;}

  void Dump(std::ostream& out) const;

 private:
                      CAlarmStringConverter();
  static void         AddNode(CXMLDOMElement* node,
                              const char*     code,
                              const char*     val);
};

// Use these functions to extract information of an Alarm.
const char* GetAlarmName(int code);
const char* GetAlarmDescription(int code);

// Makes a DOM of active alarms and faults.
void SerializeFaultsAndActiveAlarms(CXMLDOMElement* alarms,
                                    CXMLDOMElement* faults);

#endif // ALARM_STR_TABLE_H_

