//+========================================================================+
//                   BondingCntl.H                                     |
//		     Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BondingCntl.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ron                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  10-2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _BONDING_CNTL_H_
#define _BONDING_CNTL_H_

//==============================================================================================================//

#include "StateMachine.h"
#include "Bonding.h"
#include "BondingPhoneNumbers.h"


class BondingPhoneNumber;
class CBondingHardwareInterface;
class CHardwareInterface;
class CParty;
class CPartyApi;
class CRsrcParams;

//==============================================================================================================//

typedef enum {
    negotiation_timer_failed = 100,
    alignment_timer_failed,
    mux_bond_module_failure,
    mcms_internal_error,
    bonding_failure_cause_last  //should be always last
} BondingFailureCause;

static const char * BondingFailureCauseString[] = 
{
	"negotiation_timer_failed",
	"alignment_timer_failed",
	"mux_bond_module_failure",
	"mcms_internal_error"
};
//==============================================================================================================//

class CBondingCntl : public CStateMachine 
{
CLASS_TYPE_1(CBondingCntl,CStateMachine)

public:

// constructors
CBondingCntl();
CBondingCntl(DWORD call_direction,DWORD number_of_channels, char* additional_bonding_phone_num,CRsrcParams& hardwareInterface,CParty* pParty);
virtual ~CBondingCntl();
    
// CPObject and CStateMachine pure virtual
const char*   NameOf() const;
virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

BOOL  IsBondingEvent(OPCODE code);

// Requests from Party (will creat state machine event)
void Connect();
void AddChannel(); 
void Disconnect();
    
DWORD GetCallDirection()const;
DWORD GetNumberOfChannels()const;
DWORD GetRestrictType()const;

DWORD GetChannelWidthNegitiation()const;
DWORD GetRestrictTypeNegitiation()const;
DWORD GetDownSpeedSupport()const;

DWORD GetNumberOfBondingPhoneNumbers()const;
DWORD GetBondingPhoneNumber(WORD phone_num_index, char* buffer)const;
    
 void Dump()const;
 void Test()const;

//protected:

 // state machine support
 // states 
 enum STATE{IDLE=0, NEGOTIATE, ALIGNMENT,CONNECT, LAST_STATE};

 // action functions
 // legal events in IDLE state
 void OnPartyConnectIdle(CSegment* pParam);
    
 // legal events in NEGOTIATE state
 void OnBondEndNegotiateNegotiate(CSegment* pParam);
 void OnPartyDisconnectNegotiate(CSegment* pParam);
 void OnBondCallFailedNegotiate(CSegment* pParam);
 void OnTimerNegotiationFailedNegotiate(CSegment* pParam);
 void OnBondRequestParamsNegotiate(CSegment* pParam);  
    
 // legal events in ALIGNMENT state
 void OnPartyAdditionalChannelConnectedAlignment(CSegment* pParam);
 void OnBondRemoteLocalAlignmentAlignment(CSegment* pParam);
 void OnPartyDisconnectAlignment(CSegment* pParam);
 void OnBondCallFailedAlignment(CSegment* pParam);
 void OnTimerAlignmentFailedAlignment(CSegment* pParam);
    
 // legal events in CONNECT state
 void OnPartyDisconnectConnect(CSegment* pParam);
 void OnBondCallFailedConnect(CSegment* pParam);
 // Ack
 void OnMplAck(CSegment* pParam);

 void SetNewConfRsrcId(DWORD confRsrcId); //Move update confId    
    


 // attributes (for external use / API)
 DWORD m_call_direction;
 DWORD m_number_of_channels;
 BondingPhoneNumbersList m_phone_numbers_list;
    
 PDECLAR_MESSAGE_MAP   

 // implementation functions
 private:
 // function for OnPartyConnectIdle
 void BondingInit();
 // function for OnBondEndNegotiateNegotiate
 void UpdateBondEndNegotiateParams(CSegment* pParam);
 void SendBondEndNegotiateToParty(DWORD reserved_number_of_channels);
 // function for OnPartyAddChannelAlignment
 BYTE UpdateAddChannelParams(CSegment* pParam);
 void BondingAddChannel();
 // function for OnPartyAddChannelAlignment
 void SendBondConnectToParty();
 // functions for disconnect from party
 void DisconnectBonding();
 // functions for failed from bonding
 void OnBondCallFailed(CSegment* pParam);
    
 DWORD GetBondingAlignmentTimeOut(DWORD number_of_channele);

 void SimulateBondingEndNegotiation();   
    
 // attributes (for internal use / API)

 // api to MPL   
 CBondingHardwareInterface* m_pBondingHardwareInterface;
 // api to party 
 CPartyApi*           m_pTaskApi;
 CParty*              m_pParty;


 DWORD m_number_of_channels_connected;
    
};

//==============================================================================================================//

class TestBondingCntl : public CPObject
{
CLASS_TYPE_1(BondingPhoneNumbersList,CPObject)

    public:

    TestBondingCntl();
    virtual ~TestBondingCntl();

// CPObject and CStateMachine pure virtual
const char*   NameOf() const;

    
    void TestConstructors();
    void TestAll();
    void TestBondingInit();
    
};


//==============================================================================================================//

#endif // _BONDING_CNTL_H_
