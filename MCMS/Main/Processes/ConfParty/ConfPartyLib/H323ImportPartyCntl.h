//+========================================================================+
//                            H323ImportPartyCntl.h                        |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323ImportPartyCntl.h                                       |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 13/11/05   | Create                                               |
//+========================================================================+
#ifndef H323IMPORTPARTYCNTL_H_
#define H323IMPORTPARTYCNTL_H_



#include "H323PartyControl.h"
#include "MoveParams.h"


#define IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE				((WORD)305)

class CH323ImportPartyCntl : public CH323PartyCntl 
{
CLASS_TYPE_1(CH323ImportPartyCntl,CH323PartyCntl )
public: 
	// Constructors
	CH323ImportPartyCntl();
	 ~CH323ImportPartyCntl();  
	
	// Initializations 
	void  Create(CMoveIPImportParams* pMoveImportParams);
		 
	CH323ImportPartyCntl& operator=(const CH323ImportPartyCntl& other);
	
	// Operations
	virtual const char*  NameOf() const; 
	virtual void*	 GetMessageMap(); 
	
	// Action
//	void  SetDataForImportH323PartyCntl(CH323PartyCntl& OtherPartyCntl);	
	void  OnAudConnect(CSegment* pParam);
	void  OnTimerConnectToAudioBrdg(CSegment* pParam);
	// Party connect event
	void  OnPartyH323ConnectAll(CSegment* pParam);

	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo)const;
	virtual BYTE IsRemoteAndLocalHasEPCContentOnly();

	virtual void UpdateScmAccordingtoRes(CPartyCntl* pImpPartyCntl);
	
protected:
	

	// Operations	
	// Attributes
	PDECLAR_MESSAGE_MAP                                    
};



#endif /*H323IMPORTPARTYCNTL_H_*/

