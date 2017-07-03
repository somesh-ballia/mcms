#if !defined(_SnmpUtils__)
#define _SnmpUtils__

#include <vector>
using namespace std;


class CLogFltElement;


class CSnmpUtils
{
public:
    static void UpdateAlarmActiveLastChanged();
    static void SendAlarmListSnmpTrap(const vector<CLogFltElement> & alarmVector, bool isDeleted, int rmxStatus);
    static void SendSnmpTrap(const CLogFltElement & ptrAlarm, bool isDeleted, int rmxStatus);
    static int ConvertRmxStatusToSnmpFormat(int mcuState);
    
    
private:
    // disabled
    CSnmpUtils();   
};





#endif //_SnmpUtils__


