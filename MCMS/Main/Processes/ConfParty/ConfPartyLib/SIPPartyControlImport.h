//+========================================================================+
//                            SIPImportPartyCntl.h                        |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPImportPartyCntl.h                                       |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 13/11/05   | Create                                               |
//+========================================================================+
#ifndef SIPIMPORTPARTYCNTL_H_
#define SIPIMPORTPARTYCNTL_H_



#include "SIPPartyControl.h"
#include "MoveParams.h"


#define IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE				((WORD)305)
#define IMPORT_PARTY_CONNECT_TO_VIDEO_BRIDGE				((WORD)306)
#define IMPORT_PARTY_CONNECT_TO_FECC_BRIDGE					((WORD)307)

class CSipImportPartyCntl : public CSipPartyCntl 
{
CLASS_TYPE_1(CSipImportPartyCntl,CSipPartyCntl )
public: 
	// Constructors
	CSipImportPartyCntl();
	 ~CSipImportPartyCntl();  
	
	// Initializations 
	void  Create(CMoveIPImportParams* pMoveImportParams);
		 
	CSipImportPartyCntl& operator=(const CSipImportPartyCntl& other);
	
	// Operations
	virtual const char*  NameOf() const; 
	virtual void*	 GetMessageMap(); 
	
	// Action
//	void  SetDataForImportH323PartyCntl(CH323PartyCntl& OtherPartyCntl);	
	void  CheckBridgeConnection();
	void  OnAudConnect(CSegment* pParam);
	void  OnVideoBrdgConnected(CSegment* pParam);
	void  OnFeccBrdgCon(CSegment* pParam);
	void  OnTimerConnectToAudioBrdg(CSegment* pParam);
	void  OnTimerConnectToVideoBrdg(CSegment* pParam);
	void  OnTimerConnectToFeccBrdg(CSegment* pParam);

	bool IsImportDone() const {return m_isImportDone;}

	virtual void UpdateScmAccordingtoRes(CPartyCntl* pImpPartyCntl);

protected:
	// Operations	
	// Attributes
	PDECLAR_MESSAGE_MAP;
	bool m_isImportDone; //BRIDGE_13498
};
//CSS Adhoc enhancement
class CSipPluginImportPartyCntl: public CSipImportPartyCntl
{
CLASS_TYPE_1(CSipPluginImportPartyCntl, CSipImportPartyCntl)
public:
	// Constructors
	CSipPluginImportPartyCntl();
	virtual ~CSipPluginImportPartyCntl();
	virtual const char*  NameOf() const;
	
    // Operations
    	void  Create(CMoveIPImportParams* pMoveImportParams);
	virtual DWORD GetPossibleContentRate() const;
	virtual DWORD GetMinContentPartyRate(DWORD currContentRate);
protected:
	//
	PDECLAR_MESSAGE_MAP
};


#endif /*SIPIMPORTPARTYCNTL_H_*/

