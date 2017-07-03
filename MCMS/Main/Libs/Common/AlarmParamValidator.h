#ifndef ALARMPARAMVALIDATOR_H_
#define ALARMPARAMVALIDATOR_H_

#include <string>
using namespace std;

#include "DataTypes.h"


class CAlarmParamValidator
{
public:

    static bool ValidateAlarmParams(BYTE subject,
                                        DWORD errorCode);

    static bool IsInAlarmRange(int errorCode);
    static bool IsSubjectValid(BYTE subject);
    
private:
    CAlarmParamValidator();
    
};





#endif /*ALARMPARAMVALIDATOR_H_*/
