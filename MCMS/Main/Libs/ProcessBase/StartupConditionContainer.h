#ifndef STARTUPCONDITIONCONTAINER_H_
#define STARTUPCONDITIONCONTAINER_H_

#include <vector>
#include "DataTypes.h"
#include "ActiveAlarmDefines.h"

using namespace std;




typedef vector<CStartupCondition*> CStartupConditionVector;

class CStartupConditionContainer : public CStartupConditionVector
{
public:
	CStartupConditionContainer();
	virtual ~CStartupConditionContainer();
	
	DWORD AddCondition(CStartupCondition *cond);
	virtual const char* NameOf() const { return "CStartupConditionContainer";}
	bool UpdateConditionById(DWORD id, eStartupConditionStatus status);
	bool UpdateConditionByErrorCode(WORD errorCode, eStartupConditionStatus status);
	bool UpdateConditionDescriptionByErrorCode(WORD errorCode, const string & description);
	
	bool IsThereStatus(eStartupConditionStatus status);
	void SwitchStatus(eStartupConditionStatus statusFrom, eStartupConditionStatus statusTo);
	
	CStartupConditionVector::iterator FindByErrorCode(WORD errorCode);
	CStartupConditionVector::iterator GetEnd(){return end();}
	
private:
	// disabled
	CStartupConditionContainer(const CStartupConditionContainer&);
	CStartupConditionContainer& operator=(const CStartupConditionContainer&);
};

#endif /*STARTUPCONDITIONCONTAINER_H_*/
