// SlotNumConvertor.h: interface for the CSlotNumberConvertor class.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(_SLOT_NUM_CONVERTOR_H)
#define _SLOT_NUM_CONVERTOR_H


#include "PObject.h"



class CSlotNumberConvertor : public CPObject
{
CLASS_TYPE_1(CSlotNumberConvertor, CPObject)

public:
	CSlotNumberConvertor ();
	virtual ~CSlotNumberConvertor ();
	virtual const char* NameOf() const { return "CSlotNumberConvertor";}

	int   GetLogicalSlotNumber(const int physicalSlotNum);
	int   GetPhysicalSlotNumber(const int logicalSlotNum);
	void  SetSlotNumber(const int logicalSlotNum, const int physicalSlotNum);

private:

	// '+1' since the list contains boardId 0 (for switch) plus MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS slots.
	int   m_slotNumConvertorArray[MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS + 1];
};


#endif // !defined(_SLOT_NUM_CONVERTOR_H)
*/
