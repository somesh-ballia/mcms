#ifndef SYSTEM_ALARM_PROCESSOR_H_
#define SYSTEM_ALARM_PROCESSOR_H_

#include "BaseProcessor.h"

class SystemAlarmProcessor : public BaseProcessor
{
public:
    SystemAlarmProcessor();
    virtual ~SystemAlarmProcessor();
    void SystemAddAlarm(const std::string & parameter);
    void SystemDelAlarm(const std::string & parameters);
    void SystemAddEp(const std::string & parameters);
    void SystemDelEp(const std::string & parameters);
    void SystemWithoutAlarmEps();
    void SystemWithoutAlarmNoEp();
    virtual void InitCliTable();
private:
    bool with_alarms;
    bool with_eps;
};

#endif

