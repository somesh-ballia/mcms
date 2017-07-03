// OperMask.h: interface for the COperMusk class.
//
//////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date         Updated By         Description
//
//3/6/05		Yoella				Porting to Carmel
//========   ==============   =====================================================================

#ifndef _OperMask_H__
#define _OperMask_H__

#include "PObject.h"
#include "ConfPartyApiDefines.h"

#define  MAX_OPER_IN_MASK 32
#define  MAX_DWORD_MASKS  (MAX_OPERATORS_IN_MCU/MAX_OPER_IN_MASK+1)



//----------------------------------------------------------------------------
// COperMask
class COperMask : public CPObject
{
CLASS_TYPE_1(COperMask, CPObject)	
public:
	   //Constructors
    COperMask();                                                                            
    COperMask(const COperMask &other);
	COperMask& operator = (const COperMask& other);
    virtual ~COperMask();

    // Operations
    virtual const char* NameOf() const { return "COperMask";} 
    // Implementation
	
	void SetAllBitsOn();
	void SetAllBitsOff();
		
	void SetInfoBit(WORD bit, WORD onOff);
	WORD GetInfoBit(WORD bit) const;

protected:
		 // Attributes
	DWORD  m_infoMask[MAX_DWORD_MASKS]; 

private:
};



#endif // !defined(_OperMask_H__)

