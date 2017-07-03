//+========================================================================+
//                     BridgePartyList.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ReceptionList.h	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | July-2005  |                                                      |
//+========================================================================+

#if !defined(_ReceptionList_H__)
#define _ReceptionList_H__


#include <vector>
#include "PObject.h"
#include "Reception.h"



typedef std::vector <CReception*> RECEPTION_LIST;

class CReceptionList : public CPObject
{
CLASS_TYPE_1(CReceptionList,CPObject)
public: 
	
	// Constructors
	CReceptionList();
	virtual const char* NameOf() const { return "CReceptionList";}
	virtual ~CReceptionList(); 
	CReceptionList (const CReceptionList& rBridgePartyList);

	// Overloaded operators
	CReceptionList&	operator= (const CReceptionList& rOther);
	
	// Operations   

	CReception*    Find(CReception* pReception);      
	CReception*    Remove(CReception* pReception); 
	void		   ClearAndDestroy(void);
	EStat          Insert(CReception* pReception);
	EStat          Update(CReception*  pNewReception); 
	DWORD		   Size(void);
	CReception*	   At(DWORD index);
	CReception*    FindSameMRReceptionConfOnAirNO(char* targetConfName);
	bool MarkVoiceReceptionAsDisconnected(DWORD spanId, DWORD physicalPort);
	CReception*    FindReceptionWithSameMsConversationIdConfOnAirNO(char* msConversationId);
	void 			PrintReceptionList();
private:	
	// Attributes             
	RECEPTION_LIST		  m_receptionList;

	// Internal use operations             
	RECEPTION_LIST::iterator FindPosition(const CReception* pReception);
};


#endif /* _ReceptionList_H__ */
