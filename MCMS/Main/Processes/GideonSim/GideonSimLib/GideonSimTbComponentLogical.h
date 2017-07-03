//+========================================================================+
//                 GideonSimLogicalModule.h                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLogicalModule.h                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimLogicalModule.h:
//
//////////////////////////////////////////////////////////////////////

#if !defined(__GIDEONSIM_TB_COMPONENT_LOGICAL_)
#define __GIDEONSIM_TB_COMPONENT_LOGICAL_


class CStateMachine;
class CTaskApi;
class CMplMcmsProtocol;
class CSimMfaUnitsList;
//class CIcComponent;
//class CBarakIcComponent;
//class CTbComponent;
class CSimSwitchUnitsList;



// include section
#include "IpCmInd.h"
#include "GideonSimParties.h"
#include "GideonSimLogicalUnit.h"

// define section
#define IVR_SIM_FOLDER_MAIN			"Cfg/IVR/"
#define IVR_SIM_FOLDER_ROLLCALL		"IVRX/RollCall/"
#define IVR_SIM_ROLLCALL_FILE		"/SimRollCall.wav"

#define MAX_IC_IVR_PLAY_MSGS		50



class CTbComponent : public CStateMachine
{
CLASS_TYPE_1(CTbComponent,CStateMachine)
public:
			// Constructors
	CTbComponent(CTaskApp *pOwnerTask);
	virtual ~CTbComponent();
	const char * NameOf() const {return "CTbComponent";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

protected:
			// Utilities

			// Attributes

	PDECLAR_MESSAGE_MAP
};




#endif // !defined(__GIDEONSIM_TB_COMPONENT_LOGICAL_)
