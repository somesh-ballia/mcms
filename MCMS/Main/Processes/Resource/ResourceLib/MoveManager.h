#ifndef CMOVEMANAGER_H_
#define CMOVEMANAGER_H_

#include "PObject.h"
#include "InnerStructs.h"
#include "CardsStructs.h"

class CMoveManager: public CPObject
{
CLASS_TYPE_1(CMoveManager,CPObject)

public:
	CMoveManager();
	virtual ~CMoveManager();
	const char* NameOf() const { return "CMoveManager"; }

	static STATUS ReconfigureUnit(int boardId, int unitId, eUnitType unitType);
	void ReceivedReconfigureUnitInd(UNIT_RECONFIG_S* pIndication);
};

#endif /*CMOVEMANAGER_H_*/
