//+========================================================================+
//                    UpdatePartyInitParams.h                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UpdatePartyInitParams.h                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef CUPDATEPARTYINITPARAMS_H_
#define CUPDATEPARTYINITPARAMS_H_

#include "PObject.h"
#include "BridgePartyMediaParams.h"
#include "ConfAppBridgeParams.h"
#include "RsrcParams.h"
#include "ConfPartyGlobals.h"
#include "BridgePartyInitParams.h"

class CUpdatePartyInitParams : public CPObject {
CLASS_TYPE_1(CUpdatePartyInitParams,CPObject) 
public:
	CUpdatePartyInitParams ();
	CUpdatePartyInitParams(const CBridgePartyInitParams& bridgePartyInitParams);
	virtual ~CUpdatePartyInitParams ();
	CUpdatePartyInitParams& operator = (const CUpdatePartyInitParams& rOtherUpdatePartyInitParams);
	virtual const char* NameOf() const { return "CUpdatePartyInitParams";}
	
	virtual void AllocateInParams();
	virtual void AllocateOutParams();	
	
	CBridgePartyMediaParams* GetMediaInParams()const{return m_UpdateInParams;}
	CBridgePartyMediaParams* GetMediaOutParams()const{return m_UpdateOutParams;}
	CConfAppBridgeParams*    GetConfAppParams()const{return m_pConfAppBridgeParams;}
	void  InitConfAppParams(const CConfAppBridgeParams* pConfAppParams);
	void SetIsIVR(BYTE IsIVR){m_isIVR = IsIVR;}
	BYTE IsIVR(){return m_isIVR;}
	
protected:
	
	CBridgePartyMediaParams* m_UpdateInParams;
	CBridgePartyMediaParams* m_UpdateOutParams;
	CConfAppBridgeParams* 	 m_pConfAppBridgeParams;
	BYTE				     m_isIVR;
	const char*      		 m_pPartyName;
	const char* 			 m_pConfName;
	
	
};



#endif /*CUPDATEPARTYINITPARAMS_H_*/
