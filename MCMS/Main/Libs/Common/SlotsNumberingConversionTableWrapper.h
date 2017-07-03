// SlotsNumberingConversionTableWrapper.h: interface for the CSlotsNumberingConversionTableWrapper class.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SLOTSNUMBERINGCONVERSIONTABLEWRAPPER_H_
#define SLOTSNUMBERINGCONVERSIONTABLEWRAPPER_H_



#include "PObject.h"
#include "CardsStructs.h"

using namespace std;


class CSlotsNumberingConversionTableWrapper : public CPObject
{

CLASS_TYPE_1(CSlotsNumberingConversionTableWrapper, CPObject)

public:
	CSlotsNumberingConversionTableWrapper ();
	virtual ~CSlotsNumberingConversionTableWrapper ();
	const char* NameOf() const {return "CSlotsNumberingConversionTableWrapper";}
//	virtual void Dump(ostream& msg);

	CSlotsNumberingConversionTableWrapper& operator = (const CSlotsNumberingConversionTableWrapper &other);

	SLOTS_NUMBERING_CONVERSION_TABLE_S* GetStruct();
	void								SetStruct(const SLOTS_NUMBERING_CONVERSION_TABLE_S* pStruct);

	DWORD GetDisplayBoardId(const DWORD boardId, const DWORD subBoardId);
	DWORD GetBoardId(const DWORD displayBoardId, const DWORD subBoardId);

	void PrintData(const string theCaller);


protected:
	SLOTS_NUMBERING_CONVERSION_TABLE_S m_struct;
};




#endif /*SLOTSNUMBERINGCONVERSIONTABLEWRAPPER_H_*/
