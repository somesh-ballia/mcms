
#include "BondingCntl.h"
#include "BondingHardwareInterface.h"
#include "ConfPartyOpcodes.h"
#include "Party.h"
#include "PartyApi.h"
#include "OpcodesMcmsBonding.h"
#include "BondingPhoneNumbers.h"
#include "ConfPartyRoutingTable.h"

#include <sstream>
using namespace std;
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

#define DEBUG_IGNORE_TIMERS 0

// enum STATE{IDLE=0,NEGOTIATE,ALIGNMENT,CONNECT}; defined in CBondingCntl class

// bonding state machine events
// because opcodes defined in different files with different names convensions, i redefined them here

// events from party defined in file Processes/ConfParty/ConfPartyLib/ConfPartyOpcodes.h
// party ==> bond-control events
#define CONNECT             BOND_CONNECT
#define DISCONNECT          BOND_DISCONNECT
#define ADD_CHANNEL         BOND_ADD_CHANNEL

// events from bonding defined in file MCMS/McmIncld/MPL/Card/PhysicalPortART/MUX/Bonding.h
#define END_NEGOTIATE        BND_END_NEGOTIATION
#define ALIGNED              BND_REMOTE_LOCAL_ALIGNMENT
#define CALL_FAILED          BND_CALL_FAIL
#define CALL_DISCONNECTED    BND_CALL_DISCONNECTED

// timers / internal event defined here
#define BONDING_NEGOTIATION_TIMER      8001
#define BONDING_ALIGNMENT_TIMER        8002

// todo - update timeouts
#define BONDING_NEGOTIATION_TIMEOUT   15*SECOND
#define BONDING_ALIGNMENT_TIMEOUT     60

PBEGIN_MESSAGE_MAP(CBondingCntl)

    ONEVENT(CONNECT                             ,IDLE         ,CBondingCntl::OnPartyConnectIdle)

    ONEVENT(END_NEGOTIATE                       ,NEGOTIATE    ,CBondingCntl::OnBondEndNegotiateNegotiate)
    ONEVENT(DISCONNECT                          ,NEGOTIATE    ,CBondingCntl::OnPartyDisconnectNegotiate)
    ONEVENT(CALL_FAILED                         ,NEGOTIATE    ,CBondingCntl::OnBondCallFailedNegotiate)
    ONEVENT(BONDING_NEGOTIATION_TIMER           ,NEGOTIATE    ,CBondingCntl::OnTimerNegotiationFailedNegotiate)

    ONEVENT(BND_REQ_PARAMS                      ,NEGOTIATE    ,CBondingCntl::OnBondRequestParamsNegotiate)



    ONEVENT(ADD_CHANNEL                         ,ALIGNMENT    ,CBondingCntl::OnPartyAdditionalChannelConnectedAlignment)
    ONEVENT(ADD_CHANNEL                         ,NEGOTIATE    ,CBondingCntl::OnPartyAdditionalChannelConnectedAlignment)
    ONEVENT(ALIGNED          ,ALIGNMENT    ,CBondingCntl::OnBondRemoteLocalAlignmentAlignment)
    ONEVENT(DISCONNECT                          ,ALIGNMENT    ,CBondingCntl::OnPartyDisconnectAlignment)
    ONEVENT(CALL_FAILED                       ,ALIGNMENT    ,CBondingCntl::OnBondCallFailedAlignment)

    ONEVENT(DISCONNECT                          ,CONNECT      ,CBondingCntl::OnPartyDisconnectConnect)
    ONEVENT(CALL_FAILED                       ,CONNECT      ,CBondingCntl::OnBondCallFailedConnect)
    ONEVENT(BONDING_ALIGNMENT_TIMER             ,ALIGNMENT     ,CBondingCntl::OnTimerAlignmentFailedAlignment)

    ONEVENT(ACK_IND					            ,ANYCASE       ,CBondingCntl::OnMplAck)

//    ONEVENT(CALL_DISCONNECTED           ,NEGOTIATE    ,CBondingCntl::OnBondFailedNegotiate)
//    ONEVENT(CALL_DISCONNECTED        ,ALIGNMENT    ,CBondingCntl::OnBondFailedAlinmente)
//    ONEVENT(BND_CALL_DISCONNECTED     ,CONNECT      ,CBondingCntl::OnBondFailedConnect)

PEND_MESSAGE_MAP(CBondingCntl,CStateMachine);


//==============================================================================================================//
// CBondingCntl implementation
//==============================================================================================================//

//==============================================================================================================//
// CPObject and CStateMachine pure virtual
//==============================================================================================================//
const char*   CBondingCntl::NameOf() const
{
  return "CBondingCntl";
}

//==============================================================================================================//
void CBondingCntl::HandleEvent (CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

//==============================================================================================================//
// constructors and distructors
//==============================================================================================================//
CBondingCntl::CBondingCntl()
        :m_call_direction(DIRECTION_DIAL_IN), m_number_of_channels(0),m_pBondingHardwareInterface(NULL),
         m_pTaskApi(NULL),m_pParty(NULL),m_number_of_channels_connected(0)
{
	    VALIDATEMESSAGEMAP;
}
//==============================================================================================================//
CBondingCntl::CBondingCntl(DWORD call_direction,DWORD number_of_channels, char* additional_bonding_phone_num,CRsrcParams& hardwareInterface,CParty* pParty  )
        :m_call_direction(call_direction), m_number_of_channels(number_of_channels),m_number_of_channels_connected(0)
{
    // init temporary phone number
    if(additional_bonding_phone_num != NULL)
    {
        m_phone_numbers_list.AddPhoneNumber(additional_bonding_phone_num);
        m_pBondingHardwareInterface = new CBondingHardwareInterface(hardwareInterface);//rons -VALGRIND
    }
    else
    {
	    // init hardware interface
	    m_pBondingHardwareInterface = new CBondingHardwareInterface(hardwareInterface);
    }

    // init party API
    m_pParty      = pParty;
    m_pTaskApi  = new CPartyApi;
    m_pTaskApi->CreateOnlyApi(pParty->GetRcvMbx(),this);
    m_pTaskApi->SetLocalMbx(pParty->GetLocalQueue());

//       CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
//      if(pRoutingTbl== NULL)
//        {
//  		PASSERT_AND_RETURN(101);
//        }
//      pRoutingTbl->AddStateMachinePointerToRoutingTbl(hardwareInterface,m_pTaskApi);

    VALIDATEMESSAGEMAP;
}
//==============================================================================================================//
CBondingCntl::~CBondingCntl()
{
	m_phone_numbers_list.CleanPhoneList();
    POBJDELETE(m_pBondingHardwareInterface);
    POBJDELETE(m_pTaskApi);

}

//==============================================================================================================//
// Get functions
//==============================================================================================================//
DWORD CBondingCntl::GetCallDirection()const
{
    return m_call_direction;
}
//==============================================================================================================//
DWORD CBondingCntl::GetNumberOfChannels()const
{
    return m_number_of_channels;
}
//==============================================================================================================//

DWORD CBondingCntl::GetRestrictType()const
{
    return 1; //non_restrict
}
//==============================================================================================================//
DWORD CBondingCntl::GetChannelWidthNegitiation()const
{
    return 1; //negitiatable
}
//==============================================================================================================//
DWORD CBondingCntl::GetRestrictTypeNegitiation()const
{
     return 0; //not negitiatable
}
//==============================================================================================================//
DWORD CBondingCntl::GetDownSpeedSupport()const
{
    return 0; //not supported
}

//==============================================================================================================//
// State machine action functions
//==============================================================================================================//
void CBondingCntl::OnPartyConnectIdle(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnPartyConnectIdle : Name - ",PARTYNAME);
    // sending bonding init to the bonding module
    BondingInit();

    ostringstream str;
    m_phone_numbers_list.Dump(str);
	str << ", Name - " << PARTYNAME;
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnPartyConnectIdle: Dump m_phone_numbers_list \n ",str.str().c_str());

    // enter NEGOTIATE state
    m_number_of_channels_connected++;
    m_state = NEGOTIATE;
    StartTimer(BONDING_NEGOTIATION_TIMER,BONDING_NEGOTIATION_TIMEOUT);

//    SimulateBondingEndNegotiation();

}
//==============================================================================================================//
void CBondingCntl::OnBondEndNegotiateNegotiate(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondEndNegotiateNegotiate : Name - ",PARTYNAME);
    // end NEGOTIATE state
    DeleteTimer(BONDING_NEGOTIATION_TIMER);
    // pars and update negotiation params from EP
    DWORD reserved_number_of_channels = m_number_of_channels; // remember reserved number of channels
    UpdateBondEndNegotiateParams(pParam);
	WORD xfer_mode = CXferMode::GetXferModeByNumChannels(m_number_of_channels);
	if( 0 == xfer_mode ) {
	    //disconnect if EP has num channels that is not supported for bonding
	    CSegment* pSeg = new CSegment();
		*pSeg << (DWORD)mux_bond_module_failure;

		OnBondCallFailed(pSeg);
		POBJDELETE(pSeg);
	} else {
	    // send party params and additional phone # (dial out)
	    SendBondEndNegotiateToParty(reserved_number_of_channels);
		// enter ALIGNMENT state
		m_state = ALIGNMENT;
		StartTimer(BONDING_ALIGNMENT_TIMER,GetBondingAlignmentTimeOut(m_number_of_channels));
	}
}
//==============================================================================================================//
void  CBondingCntl::OnBondRequestParamsNegotiate(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondRequestParamsNegotiate : Name - ",PARTYNAME);
    // 1) update call parameters
    BYTE direction=0;BYTE NumOfBndChnls=0;BYTE restrictType=0;BYTE dummy=0;
    *pParam >> direction >> NumOfBndChnls >> restrictType >> dummy ;

    if(direction!=DIRECTION_DIAL_IN)
    {
        PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondRequestParamsNegotiate - illega direction changed to DIRECTION_DIAL_IN , Name - ",PARTYNAME);
        direction=DIRECTION_DIAL_IN;
    }
    if(NumOfBndChnls>MAX_ADDITIONAL_PHONE_NUM)
    {
        PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondRequestParamsNegotiate - illega NumOfBndChnls changed to MAX_ADDITIONAL_PHONE_NUM , Name - ",PARTYNAME);
        NumOfBndChnls=MAX_ADDITIONAL_PHONE_NUM;
    }
    if(NumOfBndChnls<m_number_of_channels){
        PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondRequestParamsNegotiate - bonding requested different num of channels then call rate  , Name - ",PARTYNAME);
    }else if(NumOfBndChnls>m_number_of_channels){
        PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondRequestParamsNegotiate - bonding requested num of channels > then call rate  - set to call rate , Name - ",PARTYNAME);
        NumOfBndChnls = m_number_of_channels;
    }


    // 2) send to MPL
    m_pBondingHardwareInterface->ResponseNegotiationParamsRequest(direction,NumOfBndChnls,restrictType,m_phone_numbers_list);
}
//==============================================================================================================//
void CBondingCntl::OnPartyDisconnectNegotiate(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnPartyDisconnectNegotiate, Name - ",PARTYNAME);
    // end NEGOTIATE state
    DeleteTimer(BONDING_NEGOTIATION_TIMER);
    DisconnectBonding();
    m_state = IDLE;
}
//==============================================================================================================//
void CBondingCntl::OnBondCallFailedNegotiate(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondCallFailedNegotiate, Name - ",PARTYNAME);
    // end NEGOTIATE state
    DeleteTimer(BONDING_NEGOTIATION_TIMER);

    OnBondCallFailed(pParam);


    m_state = IDLE;
}
//==============================================================================================================//
void CBondingCntl::OnBondCallFailedAlignment(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondCallFailedAlignment, Name - ",PARTYNAME);

    DeleteTimer(BONDING_ALIGNMENT_TIMER);

    OnBondCallFailed(pParam);


    m_state = IDLE;
}
//==============================================================================================================//
void CBondingCntl::OnBondCallFailedConnect(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondCallFailedConnect, Name - ",PARTYNAME);

    DeleteTimer(BONDING_ALIGNMENT_TIMER);

    OnBondCallFailed(pParam);

    m_state = IDLE;
}
//==============================================================================================================//
void CBondingCntl::OnTimerNegotiationFailedNegotiate(CSegment* pParam)
{
//     if(DEBUG_IGNORE_TIMERS)
//     {
//         PTRACE(eLevelInfoNormal,"CBondingCntl::OnTimerNegotiationFailedNegotiate - ignored for debug");
//         return;
//     }

   PTRACE2(eLevelInfoNormal,"CBondingCntl::OnTimerNegotiationFailedNegotiate, Name - ",PARTYNAME);
   CSegment* pSeg = new CSegment();
   *pSeg << (DWORD)negotiation_timer_failed;

   OnBondCallFailed(pSeg);

   POBJDELETE(pSeg);
}
//==============================================================================================================//
void CBondingCntl::OnTimerAlignmentFailedAlignment(CSegment* pParam)
{
    if(DEBUG_IGNORE_TIMERS)
    {
        PTRACE2(eLevelInfoNormal,"CBondingCntl::OnTimerAlignmentFailedAlignment - ignored for debug, Name - ",PARTYNAME);
        return;
    }

   PTRACE2(eLevelInfoNormal,"CBondingCntl::OnTimerAlignmentFailedAlignment, Name - ",PARTYNAME);
   CSegment* pSeg = new CSegment();
   *pSeg << (DWORD)alignment_timer_failed;
   OnBondCallFailed(pSeg);
   POBJDELETE(pSeg);
}

//==============================================================================================================//
void CBondingCntl::OnBondCallFailed(CSegment* pParam)
{
    DWORD failure_cause = 0;
    *pParam >> failure_cause;
    DWORD index = failure_cause - (DWORD)negotiation_timer_failed;
    
    CSmallString str;
    str << "failure_cause = ";
    // Romeme klocwork
    if (index < bonding_failure_cause_last && (index < (bonding_failure_cause_last- negotiation_timer_failed)))
    	str << BondingFailureCauseString[index];
    else
    	str << "UNKNOWN";

    str << ", Name - " << PARTYNAME;

    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondCallFailed: ",str.GetString());
    // send abort to bond
    DisconnectBonding();
    // send to party bonding failed
    m_pTaskApi->BondingFailed(BONDING_FAILURE);
}
//==============================================================================================================//
void CBondingCntl::OnPartyAdditionalChannelConnectedAlignment(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnPartyAddChannelAlignment, Name - ",PARTYNAME);

    // pars and update channel params
    BYTE legal_num_of_additional_channel = UpdateAddChannelParams(pParam);
    // connect additional channel to bonding
    if(legal_num_of_additional_channel==YES){
        m_pBondingHardwareInterface->AddChannel(m_call_direction,m_number_of_channels_connected,GetRestrictType());
    }
}
//==============================================================================================================//
void  CBondingCntl::OnBondRemoteLocalAlignmentAlignment(CSegment* pParam)
{
   PTRACE2(eLevelInfoNormal,"CBondingCntl::OnBondRemoteLocalAlignmentAlignment, Name - ",PARTYNAME);
    // end ALIGNMENT state
    DeleteTimer(BONDING_ALIGNMENT_TIMER);
    // send to party bond connected
    m_pTaskApi->BondAligned();
    // enter ALIGNMENT state
    m_state = CONNECT;
}

//==============================================================================================================//
void CBondingCntl::OnPartyDisconnectAlignment(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnPartyDisconnectAlinment, Name - ",PARTYNAME);
    DeleteTimer(BONDING_ALIGNMENT_TIMER);
    DisconnectBonding();
    m_state = IDLE;
}
//==============================================================================================================//
void CBondingCntl::OnPartyDisconnectConnect(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::OnPartyDisconnectConnect, Name - ",PARTYNAME);
    DisconnectBonding();
    m_state = IDLE;
}
//==============================================================================================================//

void CBondingCntl::BondingInit()
{
    m_pBondingHardwareInterface->BondingInit(m_call_direction,m_number_of_channels,GetRestrictType(),GetChannelWidthNegitiation(),GetRestrictTypeNegitiation(),GetDownSpeedSupport(),m_phone_numbers_list);

}

//==============================================================================================================//

void CBondingCntl::SimulateBondingEndNegotiation()
{
    PTRACE2(eLevelError,"CBondingCntl::SimulateBondingEndNegotiation, Name - ",PARTYNAME);
    CSegment* pSeg = new CSegment();

    DWORD direction=1/*dial out*/;DWORD NumOfBndChnls=6;DWORD restrictType=0;
    *pSeg << direction << NumOfBndChnls << restrictType;
    *pSeg << (DWORD)(NumOfBndChnls-1);// additional phone numbers
    for(WORD i=1;i<NumOfBndChnls;i++)
    {
        for(int j=0;j<7;j++)
        {
            *pSeg << (BYTE)'2';
        }
    }

    OnBondEndNegotiateNegotiate(pSeg);
    POBJDELETE(pSeg);
}
//==============================================================================================================//


//==============================================================================================================//
void CBondingCntl::UpdateBondEndNegotiateParams(CSegment* pParam)
{
    // 1) update call parameters
    BYTE direction=0;BYTE NumOfBndChnls=0;BYTE restrictType=0;BYTE dummy=0;
    *pParam >> direction >> NumOfBndChnls >> restrictType >> dummy ;
    // we update only number of channels, other parameters should not be changed in bonding negotiation (restrict not supported)
    if(NumOfBndChnls>30){
        PASSERT(NumOfBndChnls);
        NumOfBndChnls = 1;// total number of channels (1=>initial channel)
    }

    m_number_of_channels = NumOfBndChnls;
    if(m_number_of_channels==0)
    {
        DBGPASSERT(1);
        PTRACE2(eLevelInfoNormal,"CBondingCntl::UpdateBondEndNegotiateParams - illegal m_number_of_channels==0 , Name - ",PARTYNAME);
        m_number_of_channels=1;
    }

    if(direction != m_call_direction) // error trace for wrong call direction
    {
        ostringstream str;
        str << "m_call_direction = " << m_call_direction << " , negotiate_direction = " << direction << " , Name - " << PARTYNAME;
        PTRACE2(eLevelInfoNormal,"CBondingCntl::UpdateBondEndNegotiateParams: ",str.str().c_str());
    }
    // 2) update phone numbers list
    if(m_call_direction == DIRECTION_DIAL_OUT)
    {
        ostringstream str;
		str << " Name - " << PARTYNAME << "\n";
        char additional_phone_num[BND_MAX_PHONE_LEN+1];
        memset(additional_phone_num,'\0',BND_MAX_PHONE_LEN+1);
        BYTE digit = '\0';
        for(WORD phone_number_index=0;phone_number_index<m_number_of_channels-1;phone_number_index++)
        {
            for(WORD digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++)
            {
                *pParam >> digit;
//                str << digit_index << "(" << digit << ") ";
                additional_phone_num[digit_index] = (char)digit;
            }
            *pParam >> digit; // dummy
            additional_phone_num[BND_MAX_PHONE_LEN] = '\0';
            m_phone_numbers_list.AddPhoneNumber(additional_phone_num,1);// 1=> convert embedded number (BYTE)1 to string '1'

            char bonding_number[BND_MAX_PHONE_LEN+1];
            memset(bonding_number,'\0',BND_MAX_PHONE_LEN+1);
            GetBondingPhoneNumber(phone_number_index+1,bonding_number);
            bonding_number[BND_MAX_PHONE_LEN]='\0';
            str << "additional_phone_num[" << (phone_number_index+1) <<"] = " << bonding_number << "\n";
        }

		str << "num_of_phone_numbers = " << (m_number_of_channels-1) << "\n";

        PTRACE2(eLevelInfoNormal,"CBondingCntl::UpdateBondEndNegotiateParams: ",str.str().c_str());
    }
}
//==============================================================================================================//
void CBondingCntl::SendBondEndNegotiateToParty(DWORD reserved_number_of_channels)
{
    PTRACE2(eLevelInfoNormal,"CBondingCntl::SendBondEndNegotiateToParty , Name - ",PARTYNAME);
    BYTE need_reallocate = 0;
    DWORD number_of_channels_to_reallocate = m_number_of_channels; // m_number_of_channels already updated from EP
    if(reserved_number_of_channels != m_number_of_channels)
    {
        need_reallocate = 1;
    }
    if(IsValidPObjectPtr(m_pTaskApi))
    {

        m_pTaskApi->BondingEndNegotiation(need_reallocate,number_of_channels_to_reallocate/*,m_phone_numbers_list*/);
    }else{
        PTRACE2(eLevelInfoNormal,"CBondingCntl::SendBondEndNegotiateToParty - pTaskApi is not valid, Name - ",PARTYNAME);
    }


}
//==============================================================================================================//
BYTE CBondingCntl::UpdateAddChannelParams(CSegment* pParam)
{
    BYTE isLegalChannel = YES;

    ostringstream str;
    str << "channel " << m_number_of_channels_connected+1 << " of " << m_number_of_channels << " connected , Name - " << PARTYNAME;

    if(m_number_of_channels_connected<m_number_of_channels)
    {
        m_number_of_channels_connected++;
        PTRACE2(eLevelInfoNormal,"CBondingCntl::UpdateAddChannelParams: ",str.str().c_str());
        return isLegalChannel;
    } else {
        isLegalChannel = NO;
        str << "\n illegal number of additional channel connected - failed to update channel";
        PTRACE2(eLevelError,"CBondingCntl::UpdateAddChannelParams: ",str.str().c_str());
        return isLegalChannel;
    }
}

//==============================================================================================================//
DWORD CBondingCntl::GetBondingAlignmentTimeOut(DWORD number_of_channele)
{
    DWORD alignmentTout = 20*SECOND + number_of_channele*4*SECOND;
    return alignmentTout;
}
//==============================================================================================================//

void  CBondingCntl::Dump()const
{
    ostringstream str;
    str << " Name - " << PARTYNAME << "\n" << "m_call_direction = ";
    if(m_call_direction == DIRECTION_DIAL_IN){
        str << "dial-in \n";
    }else if(m_call_direction == DIRECTION_DIAL_OUT){
        str << "dial-out \n";
    }else{
        str << m_call_direction << "\n";
    }
    str << "m_number_of_channels = " << m_number_of_channels << "\n";
    str << "m_phone_numbers_list: \n";
    m_phone_numbers_list.Dump(str);
//    str << "m_pBondingHardwareInterface (ptr = " << m_pBondingHardwareInterface << "):\n";
//  m_pBondingHardwareInterface.Dump();
    PTRACE2(eLevelInfoNormal,"CBondingCntl::Dump: ",str.str().c_str());
}
//==============================================================================================================//
void CBondingCntl::DisconnectBonding()
{
    m_pBondingHardwareInterface->AbortCall();
}
//==============================================================================================================//
DWORD CBondingCntl::GetBondingPhoneNumber(WORD phone_num_index, char* buffer)const
{
    DWORD num_of_digits_copied = 0;
    WORD list_size = m_phone_numbers_list.m_phone_numbers_vector.size();

    if(list_size>phone_num_index)
    {

        BondingPhoneNumber* pPhone = m_phone_numbers_list.m_phone_numbers_vector.at(phone_num_index);
        num_of_digits_copied =  pPhone->CopyToBuffer(buffer);
    }else{
        ostringstream str;
        str << "list_size = " << list_size << " , phone_num_index = " << phone_num_index << ", Name - " << PARTYNAME;
        PTRACE2(eLevelInfoNormal,"CBondingCntl::GetBondingPhoneNumber - illigal index: ",str.str().c_str());
    }


    return num_of_digits_copied;
}



//==============================================================================================================//
// Requests from Party (will creat state machine event)
//==============================================================================================================//
void CBondingCntl::Connect()
{
    DispatchEvent(CONNECT,NULL);
}
//==============================================================================================================//
void CBondingCntl::AddChannel()
{
    DispatchEvent(ADD_CHANNEL,NULL);
}
//==============================================================================================================//
void CBondingCntl::Disconnect()
{
	if(m_state != IDLE)
		DispatchEvent(DISCONNECT,NULL);
}
//==============================================================================================================//





//PBEGIN_MESSAGE_MAP(TestBondingCntl)

    // ONEVENT(CONNECT    ,IDLE    ,TestBondingCntl::NullActionFunction)

    //ONEVENT(END_NEGOTIATE    ,NEGOTIATE    ,CBondingCntl::OnBondEndNegotiateNegotiate)
    //ONEVENT(DISCONNECT       ,NEGOTIATE    ,CBondingCntl::OnPartyDisconnectNegotiate)

    //ONEVENT(ADD_CHANNEL      ,ALIGNMENT    ,CBondingCntl::OnPartyAddChannelAlignment)
    //ONEVENT(DISCONNECT       ,ALIGNMENT    ,CBondingCntl::OnPartyDisconnectAlinment)

    //ONEVENT(DISCONNECT       ,CONNECT     ,CBondingCntl::OnPartyDisconnectConnect)

//PEND_MESSAGE_MAP(TestBondingCntl,CStateMachine);

//==============================================================================================================//

TestBondingCntl::TestBondingCntl()
{
    // VALIDATEMESSAGEMAP;
}

//==============================================================================================================//

TestBondingCntl::~TestBondingCntl()
{

}
//==============================================================================================================//
// CPObject and CStateMachine pure virtual
//==============================================================================================================//
const char*   TestBondingCntl::NameOf() const
{
  return "TestBondingCntl";
}
//==============================================================================================================//

void TestBondingCntl::TestAll()
{
    TestConstructors();
    TestBondingInit();
}

//==============================================================================================================//

void TestBondingCntl::TestConstructors()
{

    CBondingCntl testBondCntl;
    testBondCntl.Dump();


    DWORD call_direction = DIRECTION_DIAL_IN;
    DWORD number_of_channels = 6;
    char additional_bonding_phone_num[BND_MAX_PHONE_LEN+1]= {'1','2','3','4','5','6','7','\0'};
    //CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, DUMMY_PARTY_ID, m_confRsrcID, eLogical_res_none);
    CRsrcParams rsrcParams(33, 22 , 11 , eLogical_mux);
    CParty* pParty = new CParty();


    CBondingCntl testBondCntl1(call_direction,number_of_channels,additional_bonding_phone_num,rsrcParams,pParty);
    testBondCntl1.Dump();
    delete pParty;

}

//==============================================================================================================//

 void TestBondingCntl::TestBondingInit()
 {

    DWORD call_direction = DIRECTION_DIAL_OUT;
    DWORD number_of_channels = 6;
    char additional_bonding_phone_num[BND_MAX_PHONE_LEN+1]= {'1','2','3','4','5','6','7','\0'};
    //CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, DUMMY_PARTY_ID, m_confRsrcID, eLogical_res_none);
    CRsrcParams rsrcParams(33, 22 , 11 , eLogical_mux);
    CParty* pParty = new CParty();


    CBondingCntl testBondCntl(call_direction,number_of_channels,additional_bonding_phone_num,rsrcParams,pParty);
    testBondCntl.Dump();
    CSegment* pParam = NULL;

    testBondCntl.OnPartyConnectIdle(pParam);

    call_direction = DIRECTION_DIAL_IN;
    number_of_channels = 30;
    CBondingCntl testBondCntl1(call_direction,number_of_channels,additional_bonding_phone_num,rsrcParams,pParty);
    testBondCntl1.OnPartyConnectIdle(pParam);

    delete pParty;

}
//==============================================================================================================//

void CBondingCntl::OnMplAck(CSegment* pParam)
{
	ACK_IND_S* pAckIndStruct = new ACK_IND_S;
	*pParam >> pAckIndStruct->ack_base.ack_opcode
			>> pAckIndStruct->ack_base.ack_seq_num
			>> pAckIndStruct->ack_base.status
			>> pAckIndStruct->ack_base.reason
			>> pAckIndStruct->media_type
			>> pAckIndStruct->media_direction;

    ostringstream str;
	str << "Opcode = " << pAckIndStruct->ack_base.ack_opcode << " , Status = " << pAckIndStruct->ack_base.status<< ", Name - " << PARTYNAME;
	PTRACE2(eLevelInfoNormal,"CBondingCntl::OnMplAck - ACK Received: ",str.str().c_str());

	PDELETE(pAckIndStruct);
}


//==============================================================================================================//
BOOL CBondingCntl::IsBondingEvent(OPCODE event)
{
	const PMSG_MAP* pMsgMap = GetMessageMap();
	const PMSG_MAP_ENTRY* pEntry = pMsgMap? pMsgMap->m_entries : NULL;
	while ( pEntry && pEntry->actFunc != 0 )
	{
	    if ( pEntry->event == event )
			return TRUE;

		pEntry++;
	}
	return FALSE;
}
//==============================================================================================================//
void CBondingCntl::SetNewConfRsrcId(DWORD confRsrcId)//Move update confId
{
    m_pBondingHardwareInterface->SetConfRsrcId(confRsrcId);
}
//==============================================================================================================//
