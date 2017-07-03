//+========================================================================+
//                       BridgeMoveParams.H                           	   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:   BridgeMoveParams.H                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                       	   |
//-------------------------------------------------------------------------|
// Who  | Date  November-2005  | Description                               |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _CBridgeMoveParams_H_
#define _CBridgeMoveParams_H_

#include "AudioBridgePartyCntl.h"
#include "VideoBridgePartyCntl.h"

class CBridgeMoveParams : public CPObject
{
CLASS_TYPE_1(CBridgeMoveParams, CPObject)
public:
	CBridgeMoveParams();
	~CBridgeMoveParams();
	CBridgeMoveParams(const CBridgeMoveParams &other);
	virtual const char* NameOf() const { return "CBridgeMoveParams";}
	CBridgeMoveParams& operator = (const CBridgeMoveParams& other);
	
	void SetAudioBridgePartyCntlOnExport(CAudioBridgePartyCntl*);
	void SetVideoBridgePartyCntlOnExport(CVideoBridgePartyCntl*);
		
	CAudioBridgePartyCntl* GetAndResetAudioBridgePartyCntlOnImport();
	CVideoBridgePartyCntl* GetAndResetVideoBridgePartyCntlOnImport();

	CVideoBridgePartyCntl* UnregisterBridgePartyCntlsInTask();
	CVideoBridgePartyCntl* RegisterBridgePartyCntlsInTask(CTaskApp * pTask=NULL);

protected:
	CAudioBridgePartyCntl *m_pAudioBridgePartyCntl;
	CVideoBridgePartyCntl *m_pVideoBridgePartyCntl;

};


#endif //_CBridgeMoveParams_H_

