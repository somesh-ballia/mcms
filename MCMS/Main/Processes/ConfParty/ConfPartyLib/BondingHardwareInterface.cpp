
#include "BondingHardwareInterface.h"
#include "OpcodesMcmsBonding.h"
#include "BondingCntl.h"
#include <sstream>
using namespace std;


//==============================================================================================================//
// CBondingHardwareInterface implementation
//==============================================================================================================//

//==============================================================================================================//
// CPObject and CStateMachine pure virtual
//==============================================================================================================//
const char*  CBondingHardwareInterface::NameOf() const
{
    return "CBondingHardwareInterface";
}
//==============================================================================================================//
// constructors
//==============================================================================================================//
CBondingHardwareInterface::CBondingHardwareInterface(ConnectionID ConnectionId,
								 PartyRsrcID ParId,
								 ConfRsrcID ConfId,
								 eLogicalResourceTypes LRT)
{
	m_pRsrcParams = new CRsrcParams(ConnectionId, ParId , ConfId, LRT);
}
//==============================================================================================================//
CBondingHardwareInterface::CBondingHardwareInterface(CRsrcParams& hardwareInterface)
{
    m_pRsrcParams = new CRsrcParams(hardwareInterface);
}

//==============================================================================================================//
CBondingHardwareInterface::~CBondingHardwareInterface()
{
}

//==============================================================================================================//
// API functions 
//==============================================================================================================//
void CBondingHardwareInterface::BondingInit(DWORD direction,DWORD num_of_channels,DWORD restrict ,DWORD channel_width_negotiate,DWORD restrict_negotiate,DWORD downspeed_support,BondingPhoneNumbersList additional_phone_numbers)
{
    BND_CONNECTION_INIT_REQUEST_S mplBondingInitStruct;
	memset(&mplBondingInitStruct, 0, sizeof(BND_CONNECTION_INIT_REQUEST_S));

//    PTRACE(eLevelInfoNormal,"***** CBondingHardwareInterface::BondingInit simulate number of channels = 4 *******");
//    num_of_channels=4;
    
    
    SetCallParams(mplBondingInitStruct.callParams,direction,num_of_channels,restrict);
    SetNegotiationParams(mplBondingInitStruct.negotiationParams,channel_width_negotiate,restrict_negotiate,downspeed_support);
    SetAdditionalPhoneNum(mplBondingInitStruct.additional_dial_in_phone_num,direction,additional_phone_numbers);

	CSegment* pMsgToMpl = new CSegment;
	//fill the buffer with the content
	pMsgToMpl->Put( (BYTE*)(&mplBondingInitStruct), sizeof(BND_CONNECTION_INIT_REQUEST_S) );
	// send the message
	SendMsgToMPL( BND_CONNECTION_INIT, pMsgToMpl );	
	POBJDELETE(pMsgToMpl);
}
//==============================================================================================================//
void CBondingHardwareInterface::AddChannel(DWORD call_direction,DWORD number_of_channels_connected,DWORD restrict_type)
{
    BND_ADD_CHANNEL_REQUEST_S sAddChannel;
	memset(&sAddChannel, 0, sizeof(BND_ADD_CHANNEL_REQUEST_S));
	
    SetCallParams(sAddChannel.callParams,call_direction,number_of_channels_connected,restrict_type);
    
	CSegment* pMsgToMpl = new CSegment;
	//fill the buffer with the content
	pMsgToMpl->Put( (BYTE*)(&sAddChannel), sizeof(BND_ADD_CHANNEL_REQUEST_S) );
	// send the message
	SendMsgToMPL( BND_ADD_CHANNEL, pMsgToMpl );	
	POBJDELETE(pMsgToMpl);
}
//==============================================================================================================//
void CBondingHardwareInterface::AbortCall()
{
    // opcode only (no struct)
	SendMsgToMPL( BND_ABORT_CALL, NULL );	
}
//==============================================================================================================//
void CBondingHardwareInterface::ResponseNegotiationParamsRequest(DWORD call_direction,DWORD number_of_channels,DWORD restrict_type,BondingPhoneNumbersList& additional_phone_numbers)
{
    BND_ACK_PARAMS_REQUEST_S sAckParams;
	memset(&sAckParams, 0, sizeof(BND_ACK_PARAMS_REQUEST_S));
	
	
    SetCallParams(sAckParams.callParams,call_direction,number_of_channels,restrict_type);
    FillBondingPhoneList(sAckParams.phoneList,additional_phone_numbers,number_of_channels);
    
	CSegment* pMsgToMpl = new CSegment;
	//fill the buffer with the content
	pMsgToMpl->Put( (BYTE*)(&sAckParams), sizeof(BND_ACK_PARAMS_REQUEST_S) );
	// send the message
	SendMsgToMPL( BND_ACK_PARAMS, pMsgToMpl );	
	POBJDELETE(pMsgToMpl);
}
//==============================================================================================================//



//==============================================================================================================//
// private functions - fill general structs 
//==============================================================================================================//
void CBondingHardwareInterface::SetCallParams(BND_CALL_PARAMS_S& callParams,DWORD direction,DWORD num_of_channels,DWORD restrict)
{
    callParams.direction = (APIS8)direction;
    callParams.NumOfBndChnls = (APIS8)num_of_channels;
    callParams.restrictType = (APIS8)restrict;
    callParams.dummy = 0;
}
//==============================================================================================================//
void CBondingHardwareInterface::SetNegotiationParams(BND_NEGOTIATION_PARAMS_S& negotiationParams,DWORD channel_width_negotiate,DWORD restrict_negotiate,DWORD downspeed_support)
{
    negotiationParams.channelWidthNegotiation = (APIS8)channel_width_negotiate;
    negotiationParams.restrictTypeNegotiation = (APIS8)restrict_negotiate;
    negotiationParams.IsDownspeedSupport = (APIS8)downspeed_support;
    negotiationParams.dummy = 0;
}
//==============================================================================================================//
WORD CBondingHardwareInterface::SetAdditionalPhoneNum(BND_PHONE_NUM_S& additional_dial_in_phone_num, DWORD direction, BondingPhoneNumbersList& additional_phone_numbers)
{
    WORD num_of_digits_copied = 0;
    if(direction != DIRECTION_DIAL_IN)
    {
        PTRACE(eLevelInfoNormal,"CBondingHardwareInterface::SetAdditionalPhoneNum do nothing if call is not dial in");
        return num_of_digits_copied;
    }
    DWORD num_of_phone_numbers = additional_phone_numbers.GetNumOfPhoneNumbers();
    if(num_of_phone_numbers == 0)
    {
        PTRACE(eLevelInfoNormal,"CBondingHardwareInterface::SetAdditionalPhoneNum no additional number to set");
        return num_of_digits_copied;
    }

    BondingPhoneNumber temp_bonding_phone_num(*additional_phone_numbers.GetFirstOfPhoneNumbers());
    temp_bonding_phone_num.String2Emb();
    num_of_digits_copied = temp_bonding_phone_num.CopyToBuffer(additional_dial_in_phone_num.digits);
    additional_dial_in_phone_num.dummy = 0;
    return num_of_digits_copied;
}
//==============================================================================================================//
void CBondingHardwareInterface::FillBondingPhoneList(BND_PHONE_LIST_S& ackPhoneList,BondingPhoneNumbersList& additional_phone_numbers,WORD number_of_phones )
{
    // take bonding temporary phone number (first) and convert it embedded digits
    //BondingPhoneNumber* temp_bonding_phone_num = additional_phone_numbers.GetFirstOfPhoneNumbers();
    BondingPhoneNumber temp_bonding_phone_num(*additional_phone_numbers.GetFirstOfPhoneNumbers());
    
    ostringstream str;
    str << "temp phone num = ";
    temp_bonding_phone_num.Dump(str);    
    PTRACE2(eLevelInfoNormal,"CBondingHardwareInterface::FillBondingPhoneList: ",str.str().c_str());
    
    temp_bonding_phone_num.String2Emb();
    // set it in additional channels array
    for(int add_phone_index=0;add_phone_index<number_of_phones-1;add_phone_index++)
    {
        temp_bonding_phone_num.CopyToBuffer(ackPhoneList.startOfPhoneList[add_phone_index].digits);
        ackPhoneList.startOfPhoneList[add_phone_index].dummy = 0;
    }
    ackPhoneList.numberOfPhoneNums = number_of_phones;
    ackPhoneList.dummy[0]=0;ackPhoneList.dummy[1]=0;ackPhoneList.dummy[2]=0;
    
    
            
}
//==============================================================================================================//
