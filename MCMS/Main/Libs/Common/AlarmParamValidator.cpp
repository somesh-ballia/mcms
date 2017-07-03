#include "AlarmParamValidator.h"
#include "FaultsDefines.h"
#include "ObjString.h"



/////////////////////////////////////////////////////////////////////////////
bool CAlarmParamValidator::IsInAlarmRange(int errorCode)
{
    return (AA_RANGE_FIRST < errorCode && errorCode < AA_RANGE_LAST);
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmParamValidator::IsSubjectValid(BYTE subject)
{
    bool res = false;
    switch(subject)
	{
		case  FAULT_STARTUP_SUBJECT :
		case  FAULT_GENERAL_SUBJECT :
		case  FAULT_ASSERT_SUBJECT :
		case  FAULT_FILE_SUBJECT :
		case  FAULT_CARD_SUBJECT :
		case  FAULT_RESERVATION_SUBJECT :
		case  FAULT_CONFERENCE_SUBJECT  :
		case  FAULT_EXCEPTION_SUBJECT :
		case  FAULT_UNIT_SUBJECT :
		case  FAULT_MPL_SUBJECT :
            res = true;
			break;
	}
    return res;
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmParamValidator::ValidateAlarmParams(BYTE subject,
                                               DWORD errorCode)
{
    if(!IsSubjectValid(subject))
    {
        CLargeString message = "Subject not valid\n";
        message << subject;

        FPASSERTMSG(TRUE, message.GetString());
        return false;
    }
    if(!IsInAlarmRange(errorCode))
    {
        CLargeString message = "Error code not in range\n";
        message << errorCode << " - [ " << AA_RANGE_FIRST << ", " << AA_RANGE_LAST << " ]";

        FPASSERTMSG(TRUE, message.GetString());
        return false;
    }
    return true;
}
