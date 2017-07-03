//+========================================================================+
//                   BondingHardwareInterface.h                            |
//		     Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BondingHardwareInterface.h                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ron                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  10-2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _BONDING_INTERFACE_
#define _BONDING_INTERFACE_

#include "HardwareInterface.h"
#include "Bonding.h"

class BondingPhoneNumbersList;



class CBondingHardwareInterface : public CHardwareInterface
{
	CLASS_TYPE_1(CBondingHardwareInterface, CHardwareInterface)
        
public:
    // constructors
    
//  	CAudioHardwareInterface(ConnectionID ConnectionId = DUMMY_CONNECTION_ID,
// 							PartyRsrcID ParId = DUMMY_PARTY_ID,
// 							ConfRsrcID ConfId = DUMMY_CONF_ID,
// 							eLogicalResourceTypes LRT = eLogical_res_none);

// 		CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, DUMMY_PARTY_ID, m_confRsrcID, eLogical_res_none);
// 		m_pAudioHardwareInterface = new CAudioHardwareInterface();	
// 		m_pAudioHardwareInterface->Create(&rsrcParams);

    
    CBondingHardwareInterface(ConnectionID ConnectionId,PartyRsrcID ParId,ConfRsrcID ConfId,eLogicalResourceTypes LRT);
    CBondingHardwareInterface(CRsrcParams& rsrcDesc);
    virtual ~CBondingHardwareInterface();
    
    // CPObject and CStateMachine pure virtual
	virtual const char*  NameOf() const;

    // API functions
    void BondingInit(DWORD direction,DWORD num_of_channels,DWORD restrict ,DWORD channel_width_negotiate,DWORD restrict_negotiate,DWORD downspeed_support,BondingPhoneNumbersList additional_phone_numbers);
    void AddChannel(DWORD call_direction,DWORD number_of_channels_connected,DWORD restrict_type);
    void AbortCall();
    void ResponseNegotiationParamsRequest(DWORD call_direction,DWORD number_of_channels,DWORD restrict_type,BondingPhoneNumbersList& additional_phone_numbers);
    
    
private:
    
    // fill general structs
    void SetCallParams(BND_CALL_PARAMS_S& callParams,DWORD direction,DWORD num_of_channels,DWORD restrict);
    void SetNegotiationParams(BND_NEGOTIATION_PARAMS_S& negotiationParams,DWORD channel_width_negotiate,DWORD restrict_negotiate,DWORD downspeed_support);
    WORD SetAdditionalPhoneNum(BND_PHONE_NUM_S& additional_dial_in_phone_num, DWORD direction, BondingPhoneNumbersList& additional_phone_numbers);
    void FillBondingPhoneList(BND_PHONE_LIST_S& ackPhoneList,BondingPhoneNumbersList& additional_phone_numbers,WORD number_of_phones );
    
    
    
};


#endif // _BONDING_INTERFACE_
