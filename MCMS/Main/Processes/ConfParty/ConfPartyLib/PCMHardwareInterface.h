//+========================================================================+
//                   PCMHardwareInterface.h                            |
//		     Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       PCMHardwareInterface.h                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ron                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  10-2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _PCM_HARDWARE_INTERFACE_
#define _PCM_HARDWARE_INTERFACE_

#include "HardwareInterface.h"

class CPCMHardwareInterface : public CHardwareInterface
{
	CLASS_TYPE_1(CPCMHardwareInterface, CHardwareInterface)
        
public:
     
    CPCMHardwareInterface(ConnectionID ConnectionId,PartyRsrcID ParId,ConfRsrcID ConfId,eLogicalResourceTypes LRT);
    CPCMHardwareInterface(CRsrcParams& rsrcDesc);
    virtual ~CPCMHardwareInterface();
    
    // CPObject and CStateMachine pure virtual
	virtual const char*  NameOf() const {return "CPCMHardwareInterface";}
	
	void SendMessageToPCM(DWORD opcode, const char* msgStr);
	void SendConnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId);
	void SendDisconnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId);

    
};


#endif // _BONDING_INTERFACE_
