// AlarmStrDeclaration.h

#ifndef ALARM_STR_DECLARATION_H_
#define ALARM_STR_DECLARATION_H_

#define BEGIN_ALARM_TABLE()\
	void InitAlarmTable()\
  {\
    CAlarmStringConverter& db = CAlarmStringConverter::Instance();

#define ADD_ALARM_TO_ALARM_STR_TABLE(code, desc)\
  db.AddAlarm(code, #code, desc, true);

#define ADD_FAULT_TO_ALARM_STR_TABLE(code, desc)\
  db.AddAlarm(code, #code, desc, false);

#define END_ALARM_TABLE()\
	}

void InitAlarmTable();

#endif // ALARM_STR_DECLARATION_H_
