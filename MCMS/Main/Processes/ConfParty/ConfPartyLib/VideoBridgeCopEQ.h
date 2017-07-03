//+========================================================================+
//                     VideoBridgeCop.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridgeCopEQ.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: 		                                                       |
//-------------------------------------------------------------------------|
// Who  | Date    | Description			                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef VIDEOBRIDGECOPEQ_H_
#define VIDEOBRIDGECOPEQ_H_
#include "VideoBridgeCop.h"
/*#include "VideoHardwareInterface.h"
#include "VideoBridgeCopEncoder.h"
#include "VideoBridgeCopDecoder.h"
#include "COP_ConfParty_Defs.h"
#include "ChangeLayoutActions.h"
#include "COP_Layout_definitions.h"
*/

//class CChangeLayoutActions;

/*typedef struct
{
	DWORD	          artPartyId;
	EMediaDirection   disconnectDirection;
 }sDisconnectingParty;
typedef std::vector< sDisconnectingParty> DISCONNECTING_VECTOR;


#define CONNECT_COP_VB_TOUT   					((WORD)210)
#define SMART_SWITCH_TOUT						((WORD)211)
#define	SWITCH_TIME_OUT_COP_VALUE				3*SECOND
#define CONNECT_COP_VB_TIME_OUT_VALUE   		5*SECOND
#define	COP_BRIDGE_DISCONNECT_TIME_OUT_VALUE	5*SECOND



#define LAYOUT_NOT_CHANGED 			0
#define LAYOUT_CHANGED_ATTRIBUTES 	1
#define LAYOUT_CHANGED				2


#define PCM_RESOURCE_INDEX NUM_OF_LEVEL_ENCODERS

#define CONNECT_PCM_ENCODER_TIMER 205
*/
class CVideoBridgeCOPEq : public CVideoBridgeCOP
{
CLASS_TYPE_1(CVideoBridgeCOPEq,CVideoBridgeCOP)

public:
	// constructors & destructors
	CVideoBridgeCOPEq ();
	virtual ~CVideoBridgeCOPEq ();
	virtual void Create (const CVideoBridgeInitParams* pVideoBridgeInitParams);
  	virtual void InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);
  	virtual WORD IsValidState(WORD state)const;
  	virtual WORD IsValidEvent(OPCODE event)const;

	// 	object & state machine virtual functions
	virtual const char*	NameOf () const;
	virtual void*  GetMessageMap();

	virtual void OnConfTerminateDISCONNECTING(CSegment* pParam);

protected:
	virtual void OnConfDeletePartyFromConfCONNECTED(CSegment * pParam);

	virtual void OnConfUpdateVideoMute(CSegment* pParam);

	virtual void OnConfVideoRefresh(CSegment* pParam);
	virtual void RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp *pParty);

	virtual void OnConfUpdateFlowControlRate(CSegment* pParam);

private:
	PDECLAR_MESSAGE_MAP
};

#endif /*VIDEOBRIDGECOPEQ_H_*/
