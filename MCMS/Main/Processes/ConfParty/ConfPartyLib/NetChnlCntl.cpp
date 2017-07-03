/////////////////////////////////////////////////////////////////////////////
//                           CNetChnlCntl functions
/////////////////////////////////////////////////////////////////////////////  
#include "NetChnlCntl.h"
#include "PartyApi.h"
#include "Party.h"
#include "IsdnNetSetup.h"
#include "Trace.h"
#include "ConfPartyRoutingTable.h"
#include "NetCause.h"
#include "NetHardwareInterface.h"
#include "TraceStream.h"
#include "OpcodesMcmsNetQ931.h"
#include "ConfPartyOpcodes.h"
#include "IsdnVideoPartyOut.h"
#include "IpMfaOpcodes.h"
//#include "OpcodesRanges.h" // for NET_ opcodes (until implemented by other sides)


#define BOARD_FULL_CAUSE causNO_CHAN_AVL_VAL


// forget from embbeded names
//const WORD SETUPIND         = SETUP_IND_VAL;  
//const WORD CLEARIND         = CLEAR_IND_VAL;  
//const WORD PROGRESSIND      = PROGRESS_IND_VAL;  
//const WORD ALERTIND         = ALERT_IND_VAL;  
//const WORD PROCEEDIND       = PROCEED_IND_VAL;  
//const WORD CONNECTIND       = CONNECT_IND_VAL;  
//const WORD DISCONNECTIND    = DISCONNECT_IND_VAL;  
//const WORD NETBADREQIND     = NET_BADREQ_IND_VAL;  
//const WORD NETERROR         = NET_ERROR_IND_VAL;  

// NetChnlCntl uses a unique MFA_RESPONSE_TIME, undefining the global one
#ifdef MFA_RESPONSE_TIME
	#undef MFA_RESPONSE_TIME
#endif
const WORD MFA_RESPONSE_TIME = 17;

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

PBEGIN_MESSAGE_MAP(CNetChnlCntl)
						    // party events  
	ONEVENT(CALLOUT         ,CONNECT_HARDWARE         ,CNetChnlCntl::OnPartyCalloutIdle)
    ONEVENT(CONNHARDWARE    ,IDLE                     ,CNetChnlCntl::OnConnectHardwareIdle)
    ONEVENT(CONNHARDWARE    ,DISCONNECTING_NET        ,CNetChnlCntl::OnConnectHardwareIdle)
//  ONEVENTA(CONNECTNETTS   ,CONNECT    ,(AFUNC)CNetChnlCntl::OnPartyConnectTsConnect   ,"CONNECTNETTS","CONNECT")   
//	ONEVENTA(LDISCONNECT   ,IDLE     ,(AFUNC)CNetChnlCntl::OnPartyDisconnectIdle    ,"LDISCONNECT","IDLE")   
    ONEVENT(LDISCONNECT     ,SETUP                    ,CNetChnlCntl::OnPartyDisconnectSetup)
    ONEVENT(LDISCONNECT     ,CONNECT                  ,CNetChnlCntl::OnPartyDisconnectConnect)
    ONEVENT(LDISCONNECT     ,DISCONNECTING_NET        ,CNetChnlCntl::OnPartyDisconnectDisconnectNet)
    ONEVENT(LDISCONNECT     ,ANYCASE                  ,CNetChnlCntl::NullActionFunction)
   						    // local events   
    ONEVENT(SETUPTOUT       ,SETUP                    ,CNetChnlCntl::OnTimerSetup)   
    ONEVENT(CLEARTOUT       ,DISCONNECTING_NET        ,CNetChnlCntl::OnTimerClearDisconnectingNet)   
//  ONEVENTA(CLEARTOUT     ,CONNECT   ,(AFUNC)CNetChnlCntl::OnTimerClear            ,"CLEARTOUT","CONNECT")   
//  ONEVENTA(REDIALTOUT     ,SETUP   ,(AFUNC)CNetChnlCntl::OnTimerRedial            ,"REDIALTOUT","SETUP")   
//
    ONEVENT(ACK_IND         ,CONNECT_HARDWARE         ,CNetChnlCntl::OnAckIndConnectHardware)
    ONEVENT(ACK_IND         ,DISCONNECTING_HARDWARE   ,CNetChnlCntl::OnAckIndDisconnHardware)
    ONEVENT(ACK_IND         ,DISCONNECTING_NET        ,CNetChnlCntl::OnAckIndDisconnHardware)
						    // hdlc events
    ONEVENT(NET_SETUP_IND   ,CONNECT_HARDWARE         ,CNetChnlCntl::OnNetSetupIndIdle)
  
    ONEVENT(NET_CLEAR_IND   ,IDLE                     ,CNetChnlCntl::NullActionFunction)    
    ONEVENT(NET_CLEAR_IND   ,DISCONNECTING_HARDWARE   ,CNetChnlCntl::NullActionFunction)//OnNetClearSetup)
    ONEVENT(NET_CLEAR_IND   ,CONNECT                  ,CNetChnlCntl::OnNetClearConnect)
    ONEVENT(NET_CLEAR_IND   ,DISCONNECTING_NET        ,CNetChnlCntl::OnNetClearDisconnNet)
  
//ONEVENT(NET_DISCONNECT_ACK_IND,SETUP  ,CNetChnlCntl::OnNetDisconnectAckSetup)
//ONEVENT(NET_DISCONNECT_ACK_IND,CONNECT  ,CNetChnlCntl::OnNetDisconnectAckConnect)
    ONEVENT(NET_DISCONNECT_ACK_IND ,DISCONNECTING_NET  ,CNetChnlCntl::OnNetDisconnectAckDiscNet)
	  
    ONEVENT(NET_ALERT_IND          ,ANYCASE            ,CNetChnlCntl::OnNetAlertAnycase)   

    ONEVENT(NET_CONNECT_IND	       ,SETUP              ,CNetChnlCntl::OnNetConnectSetup)  
    ONEVENT(NET_CONNECT_IND	       ,ANYCASE            ,CNetChnlCntl::NullActionFunction)  

    ONEVENT(NET_DISCONNECT_IND     ,SETUP              ,CNetChnlCntl::OnNetDisconnectSetup)
    ONEVENT(NET_DISCONNECT_IND     ,CONNECT_HARDWARE   ,CNetChnlCntl::OnNetDisconnectConnectHardware)
    ONEVENT(NET_DISCONNECT_IND     ,CONNECT            ,CNetChnlCntl::OnNetDisconnectConnect)
    ONEVENT(NET_DISCONNECT_IND     ,DISCONNECTING_NET  ,CNetChnlCntl::NullActionFunction)
    ONEVENT(NET_DISCONNECT_IND     ,DISCONNECTING_HARDWARE ,CNetChnlCntl::NullActionFunction)

    ONEVENT(MFARESPONSE_TOUT       ,CONNECT_HARDWARE       ,CNetChnlCntl::OnMfaTimeoutConnectHardware)
    ONEVENT(MFARESPONSE_TOUT       ,DISCONNECTING_HARDWARE ,CNetChnlCntl::OnMfaTimeoutDisconnectHardware)
    ONEVENT(MFARESPONSE_TOUT       ,DISCONNECTING_NET      ,CNetChnlCntl::OnMfaTimeoutDisconnectHardware)

    ONEVENT(IP_CM_RTCP_MSG_IND,			ANYCASE,		CNetChnlCntl::NullActionFunction)
	ONEVENT(IP_CM_RTCP_RTPFB_IND,		ANYCASE,		CNetChnlCntl::NullActionFunction)
	ONEVENT(IP_CM_RTCP_VSR_IND,			ANYCASE,		CNetChnlCntl::VsrNullActionFunction)
	ONEVENT(IP_CM_RTCP_MS_SVC_PLI_IND,	ANYCASE,		CNetChnlCntl::MsSvcPliNullActionFunction)

PEND_MESSAGE_MAP(CNetChnlCntl,CStateMachine);
  
void CNetChnlCntl::VsrNullActionFunction(CSegment*)
{
    PTRACE2(eLevelInfoNormal,"CNetChnlCntl::VsrNullActionFunction - Name: ",PARTYNAME);
}

void CNetChnlCntl::MsSvcPliNullActionFunction(CSegment*)
{
    PTRACE2(eLevelInfoNormal,"CNetChnlCntl::MsSvcPliNullActionFunction - Name: ",PARTYNAME);
}

const char* AckTableKey::GetChannelTypeAsString()const
{
  switch(m_channelType) {
  case kPstnAudioChnlType:
	return "kPstnAudioChnlTyp";
  case kRtmChnlType:
	return "kRtmChnlType";
  case kIsdnMuxChnlType:
	return "kIsdnMuxChnlType";
  default:
	return "Unknown channel type";
  }
}

/////////////////////////////////////////////////////////////////////////////
CNetChnlCntl::CNetChnlCntl()    // constructor
 
{ 
  m_pNetSetup = new CIsdnNetSetup;
  m_pNetInterface = new CNetHardwareInterface;
  m_seqNum = 0;
  m_callType = 0;
  m_pParty    = NULL;
  m_pTaskApi  = new CPartyApi;
  m_disconnectStatus=statOK;
  m_isClearBeforeDisc=0; 
//  m_pNet      = new CNet;
//  m_pNetDesc  = NEW(CNetConnRsrcDesc); 
	
  VALIDATEMESSAGEMAP; 
}

/////////////////////////////////////////////////////////////////////////////
CNetChnlCntl::~CNetChnlCntl()     // destructor
{
   POBJDELETE(m_pNetSetup);
   POBJDELETE(m_pNetInterface);
   POBJDELETE(m_pTaskApi);
   m_ackStatusTable.clear();
//   POBJDELETE(m_pNet);
//   POBJDELETE(m_pNetDesc);  
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::Create(CRsrcParams& rNetRsrcParams,
						   CIsdnNetSetup & rNetSetup,CParty* pParty,WORD seqNum,WORD callType)
{
//  m_pRsrcTbl = ::GetpRsrcTbl();
//  *m_pNetDesc   = rNetDesc;  
//  m_pTsStream   = pTsStream;
	
    *m_pNetSetup  = rNetSetup;
	m_pParty      = pParty;
	m_seqNum      = seqNum;
  
//	m_pNet->Create(m_pNetDesc);
    m_pNetInterface->Create(rNetRsrcParams);
    
    m_pTaskApi->CreateOnlyApi(pParty->GetRcvMbx(),this);
    m_pTaskApi->SetLocalMbx(pParty->GetLocalQueue());

    m_callType =callType; 
    CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
    if(pRoutingTbl== NULL)
      {
		PASSERT_AND_RETURN(101);
      }
      
    pRoutingTbl->AddStateMachinePointerToRoutingTbl(rNetRsrcParams,m_pTaskApi);
    
//	InitTimer(pParty->GetRcvMbx()); 
}

//void  CNetChnlCntl::ConnectTS()
//{
//
//
//  DispatchEvent(CONNECTNETTS,(void*)m_msgEntries,NULL);  
//}
//
/////////////////////////////////////////////////////////////////////////////
/*void  CNetChnlCntl::Update(CTimeSlotStream* pTsStream,CParty* pParty,WORD seqNum)                                       
{
	// worning for race condition:
	// check that when we remove resource manager from resouce table 
	// that we don't remove other party task api
    CTaskApi* pRsrcTblTaskApi = (CTaskApi*) (m_pRsrcTbl->GetRsrcMngrPtr(m_pNetDesc->GetRsrcId()));
	if(pRsrcTblTaskApi != NULL && pRsrcTblTaskApi!=m_pTaskApi){
		PTRACE2(eLevelError,"CNetChnlCntl::Update remove TaskApi != m_pTaskApi  : Name - ",PARTYNAME);
	}
	// remove party at lobby resources
	m_pRsrcTbl->RemoveRsrcMngrPtr(m_pNetDesc->GetRsrcId());
	m_pTaskApi->DestroyOnlyApi();
	DestroyTimer();
	POBJDELETE(m_pTaskApi);
	// alocate new resources  
	m_pParty      = pParty;
	m_seqNum      = seqNum;
	m_pTaskApi = new CPartyApi;
	m_pTaskApi->CreateOnlyApi(pParty->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx(pParty->GetLocalRcvMbx());
	m_pRsrcTbl->AddRsrcMngrPtr(m_pNetDesc->GetRsrcId(),m_pTaskApi);
	InitTimer(pParty->GetRcvMbx());
	SetStream(pTsStream);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::SetStream(CTimeSlotStream* pTsStream)                                        
{ 
     // close previos stream
	m_pNet->ConnectStream(m_pTsStream,DISCONNECT_NET_TS);     
  POBJDELETE(m_pTsStream); 
  m_pTsStream   = pTsStream;
	m_pNet->ConnectStream(m_pTsStream,CONNECT_NET_TS);   
}
*/
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::Destroy()                                        
{  
  if(CPObject::IsValidPObjectPtr(m_pNetInterface)){
    CRsrcParams* pRsrcParams = m_pNetInterface->GetRsrcParams();
    if(CPObject::IsValidPObjectPtr(pRsrcParams))
      {
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();	
	pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*(m_pNetInterface->GetRsrcParams()));
      }
    else
      {
	PTRACE2(eLevelError,"CNetChnlCntl::Destroy - m_pNetInterface->m_pRsrcParams is not valid, Name - ",PARTYNAME);
      }
  }
  else{
    PTRACE2(eLevelError,"CNetChnlCntl::Destroy - m_pNetInterface is not valid, Name - ",PARTYNAME);
  }
  m_pTaskApi->DestroyOnlyApi();
  //DestroyTimer();
  DeleteAllTimers();
}

/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/*void  CNetChnlCntl::Dump(WORD  switchFlag)
{
  WORD        msgBufSize = 4096;
	char*       msgStr = new char[msgBufSize];
	COstrStream  msg(msgStr,msgBufSize);

  msg.setf(ios::left,ios::adjustfield);
  msg.setf(ios::showbase);

  char*   stateStr = STATEASSTRING;
  char*   partyName = PARTYNAME;
     
	msg << "\nCNetChnlCntl::Dump\n" 
      << "-----------\n" 
      << setw(20) << "this"             << (hex) << (DWORD)this                    << "\n"
      << setw(20) << "m_pParty"         << (hex) << (DWORD)m_pParty                << "\n"     
      << setw(20) << "m_pTsStream"      << (hex) << (DWORD)m_pTsStream             << "\n"
      << setw(20) << "m_pNetDesc  "     << (hex) << (DWORD)m_pNetDesc              << "\n"
      << setw(20) << "m_seqNum   "      << (hex) << m_seqNum                       << "\n"
      << setw(20) << "party name"       << (hex) << partyName                      << "\n"
      << setw(20) << "m_state   "       << (hex) << stateStr                       << "\n";
  
  
	PTRACE(eLevelInfoNormal,msg.str());
	delete [] msgStr;  
}*/

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::Setup()
{
//  DispatchEvent(CALLOUT,(void*)m_msgEntries,NULL);
    DispatchEvent(CALLOUT);  
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnSetup(WORD NetTsConnection)
{
  CSegment* pSeg = new CSegment;
  *pSeg << NetTsConnection;
  DispatchEvent(NET_SETUP_IND,pSeg);
  POBJDELETE(pSeg);
} 
/*
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::SendWinkStart()
{
	SendWinkStartT1();
}
*/
/////////////////////////////////////////////////////////////////////////////
void*  CNetChnlCntl::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::Clear()
{
//  DispatchEvent(LDISCONNECT,(void*)m_msgEntries,NULL);
    DispatchEvent(LDISCONNECT);   
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnPartyCalloutIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyCalloutIdle : Name - ",PARTYNAME);
	
	m_callType = DIALOUT;
	//	m_pNet->ConnectStream(m_pTsStream,CONNECT_NET_TS); 
	
	//#ifdef BOND_DOWNSPEED_SIM	
	//			const char* party_name = m_pParty->GetName();
	//			if( !strncmp(party_name,"DialOutReject",9) ){
	//				if(m_seqNum==PROBLEMATIC_CHANNEL){
	//					PTRACE2(eLevelError,"CNetChnlCntl::OnPartyCalloutIdle : simulate NET_CANNEL_REJECT: Name - ",PARTYNAME);
	//				m_pTaskApi->NetConnect(m_seqNum,statIllegal,0);
	//				return;
	//				}
	//			}
	//#endif // BOND_DOWNSPEED_SIM
	//
	//PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyCalloutIdle : StartTimer(SETUPTOUT) Name - ",PARTYNAME);
	CSmallString sstr; 
	if (m_pNetSetup->m_callType == ((DWORD)ACU_VOICE_SERVICE) ){

		//VNGFE-2617
		DWORD ringingDuration = 0;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetDWORDDataByKey("PSTN_RINGING_DURATION_SECONDS", ringingDuration);
		StartTimer(SETUPTOUT,ringingDuration*SECOND);
		sstr << " value = " << ringingDuration*SECOND << " , Name = " << PARTYNAME;

		//StartTimer(SETUPTOUT,SETUP_PSTN_TOUT);
		//sstr << " value = " << SETUP_PSTN_TOUT << " , Name = " << PARTYNAME;
	}
	else //ISDN
	{
		StartTimer(SETUPTOUT,SETUP_TOUT);
		sstr << " value = " << SETUP_TOUT << " , Name = " << PARTYNAME;
	}
	PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyCalloutIdle : StartTimer(SETUPTOUT) : ",sstr.GetString());

	m_pNetInterface->Setup(*m_pNetSetup);  
	
	m_state= SETUP;                
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetSetupIndIdle(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : Name - ",PARTYNAME);
//   WORD  NetTsConnection;
//   *pParam >> NetTsConnection ; //select if to connect the network TS of this channel
//   if(  (NetTsConnection != TRUE) && (NetTsConnection != FALSE) )
//     PASSERT(NetTsConnection);
	
//   WORD netPSTNDelay1 = ::GetpSystemCfg()->GetNetPSTNDelay();	// delay before connecting in milli seconds
//   WORD netPSTNDelay2 = ::GetpSystemCfg()->GetNetPSTNDelay2();	// delay before connecting in milli seconds
//   WORD netPSTNDelay3 = ::GetpSystemCfg()->GetNetPSTNDelay3();	// delay before connecting in milli seconds
//   WORD netOreder     = ::GetpSystemCfg()->GetNetPSTNOrder();	// delay before connecting in milli seconds
	
//   char buf[32];
	
	
//   if (0 == netOreder) {
//     // delay 1
//     if (0 != netPSTNDelay1) {
//       PSelf self;
//       self.WakeAfter(netPSTNDelay1);
//       sprintf(buf," %d ms",netPSTNDelay1);
//       PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : wake after delay 1 ", buf);
//     }
		
//     if ( NetTsConnection == TRUE)	// default behavour is that we connect the TS here
//       m_pNet->ConnectStream(m_pTsStream,CONNECT_NET_TS);
		
//     // delay 2
//     if ((0 != netPSTNDelay2) && (8 != netPSTNDelay2)) {
//       PSelf self;
//       self.WakeAfter(netPSTNDelay2);
//       sprintf(buf," %d ms",netPSTNDelay2);
//       PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : wake after delay 2 ", buf);
//     }
		
//     if (8 != netPSTNDelay2) {			// 8 is for debug
//       m_pNet->Alert();
//       PTRACE(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : Do Alert ");
//     }
		
//     // delay 3
//     if (0 != netPSTNDelay3) {			// as the old style, without delay
//       PSelf self;
//       self.WakeAfter(netPSTNDelay3);
//       sprintf(buf," %d ms",netPSTNDelay3);
//       PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : wake after delay 3 ", buf);
//     }
//     // connect the line
//     m_pNet->Connect(*m_pNetSetup);
//   }
//   else
//     {
//       // delay 1
//       if (0 != netPSTNDelay1) {
// 	PSelf self;
// 	self.WakeAfter(netPSTNDelay1);
// 	sprintf(buf," %d ms",netPSTNDelay1);
// 	PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : wake after delay 1 ", buf);
//       }
		
  m_pNetInterface->Alert(*m_pNetSetup);		
		
		
//       // delay 2
//       if ((0 != netPSTNDelay2) && (8 != netPSTNDelay2)) {
// 	PSelf self;
// 	self.WakeAfter(netPSTNDelay2);
// 	sprintf(buf," %d ms",netPSTNDelay2);
// 	PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : wake after delay 2 ", buf);
//       }
		
		
  // connect the line
  m_pNetInterface->Connect(*m_pNetSetup);
		
//       // delay 3
//       if (0 != netPSTNDelay3) {			// as the old style, without delay
// 	PSelf self;
// 	self.WakeAfter(netPSTNDelay3);
// 	sprintf(buf," %d ms",netPSTNDelay3);
// 	PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetSetupIndIdle : wake after delay 3 ", buf);
//       }
		
//       if ( NetTsConnection == TRUE)	// default behavour is that we connect the TS here
// 	m_pNet->ConnectStream(m_pTsStream,CONNECT_NET_TS);
		
//     }
	
	
  m_state = CONNECT;
  m_callType = DIALIN;
  TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnNetSetupIndIdle : m_seqNum :  " << m_seqNum << " , Name - " << PARTYNAME;  
//  if(0 == m_seqNum)    
  m_pTaskApi->NetConnect(m_seqNum,statOK);   
}
////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnNetAlertAnycase(CSegment* pParam)
{
	if ( m_callType == DIALIN ) 
	    PTRACE2(eLevelError,"CNetChnlCntl::OnNetAlertAnycase :dial in call got illegal NET_ALERT_IND!!! , Name - ",PARTYNAME);
	  
	  if ( m_callType == DIALOUT )  
	  {
	    PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetAlertAnycase  Name: ", PARTYNAME);

	    //Update the NetSetupParams according to the NET message
	    NET_ALERT_IND_S netConnectInd;
	    memset(&netConnectInd,0,sizeof(NET_ALERT_IND_S));
	    pParam->Get((BYTE*)(&netConnectInd),sizeof(NET_ALERT_IND_S));

	    UpdateNetParams(netConnectInd.net_common_header.span_id,
						netConnectInd.net_common_header.net_connection_id,
						netConnectInd.net_common_header.physical_port_number);
	  }
}
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetConnectSetup(CSegment* pParam)
{
  if ( m_callType == DIALIN ) 
    PTRACE2(eLevelError,"CNetChnlCntl::OnNetConnectSetup : in call got illegal connect!!! , Name - ",PARTYNAME);
  
  if ( m_callType == DIALOUT )  {
    m_state = CONNECT;
    PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetConnectSetup : DeleteTimer(SETUPTOUT) Name - ",PARTYNAME);
    DeleteTimer(SETUPTOUT);

    //Update the NetSetupParams according to the NET message
    NET_CONNECT_IND_S netConnectInd;
    memset(&netConnectInd,0,sizeof(NET_CONNECT_IND_S));
    pParam->Get((BYTE*)(&netConnectInd),sizeof(NET_CONNECT_IND_S));

    UpdateNetParams(netConnectInd.net_common_header.span_id,
					netConnectInd.net_common_header.net_connection_id,
					netConnectInd.net_common_header.physical_port_number);
  }
  
  m_pTaskApi->NetConnect(m_seqNum,statOK);  
  
//#ifdef BOND_DOWNSPEED_SIM	
//	const char* party_name = m_pParty->GetName();
//    if( !strncmp(party_name,"DialOutDisconnect",9) ){
//		if(m_seqNum==PROBLEMATIC_CHANNEL){
//			PTRACE2(eLevelError,"CNetChnlCntl::OnNetConnectSetup : simulate NET_DISCONNECT_CONNECT: Name - ",PARTYNAME);
//			m_isClearBeforeDisc=0;
//			CSegment*  seg = new CSegment;
//			*seg <<(BYTE)0;
//			*seg <<(BYTE)0;
//			*seg <<(BYTE)0;
//			OnNetDisconnectConnect(seg);
//		}
//	}
//#endif // BOND_DOWNSPEED_SIM	

}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetDisconnectSetup(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetDisconnectSetup : Name - ",PARTYNAME);
  
  OnNetDisconnect(pParam);
  
  //if( (m_callType == DIALOUT) && (m_NumOfRetry > 0) ){
    //CNetCause   cause;
    //ON(m_clearReq);
    //DeleteTimer(SETUPTOUT);
    //StartTimer(CLEARTOUT,CLEAR_TOUT);                                    
    //m_pNetInterface->Clear(cause);
  //return;
  //}


  //  BYTE  coding_standard,location,cause;
  //  *pParam >> coding_standard >> location >> cause;
  // worning for race condition - dial out call while dial in call already took the port
  // cause 44 should return to the dial out call
  //if(m_callType == DIALIN && cause == 44){
    // PTRACE2(eLevelError,"CNetChnlCntl::OnNetDisconnectSetup requested circuit/channel not available (44) received for dial in party : Name - ",PARTYNAME);
    //}
    //Disconnect(REMOTE);
    //m_pTaskApi->NetConnect(m_seqNum,statIllegal,cause); 
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetDisconnectConnect(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetDisconnectConnect : Name - ", PARTYNAME);
  OnNetDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetDisconnectConnectHardware(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetDisconnectConnectHardware : Name - ", PARTYNAME);
  if (DIALIN != m_callType )
    {
      TRACESTR (eLevelError) << " CNetChnlCntl::OnNetDisconnectConnectHardware can be only in dialIn!  Name - " << PARTYNAME;
      PASSERT_AND_RETURN(m_pNetInterface->GetPartyRsrcId());
    }
  IsAllAcksReceived();
  DisconnectHardware();
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetDisconnect(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetDisconnect :  Name - ", PARTYNAME);
  BYTE  coding_standard,location,cause;
  NET_DISCONNECT_IND_S netDisInd;
  memset(&netDisInd,0,sizeof(NET_DISCONNECT_IND_S));
  
  pParam->Get((BYTE*)(&netDisInd),sizeof(NET_DISCONNECT_IND_S));
  cause=netDisInd.cause.cause_val;
  UpdateNetParams(netDisInd.net_common_header.span_id,
				  netDisInd.net_common_header.net_connection_id,
				  netDisInd.net_common_header.physical_port_number);
  //*pParam >> coding_standard >> location >> cause;

  //if ( m_disconnectSrc != LOCAL )  // take action only if net was not already disconnected.
  //{
      Disconnect(REMOTE);
      //BYTE cause = 0;
      // warning for race condition - dial out call while dial in call already took the port
      // cause 44 should return to the dial out call
      if(m_callType == DIALIN && cause == 44) {
		  PTRACE2(eLevelError,"CNetChnlCntl::OnNetDisconnect requested circuit/channel not available (44) received for dial in party : Name - ",PARTYNAME);
	  }
	  if (BOARD_FULL_CAUSE == cause) {/* board full case */
		  /* close UDP ports */
		  TRACESTR(eLevelInfoNormal) << " CNetChnlCntl::OnNetDisconnect : BOARD_FULL_CAUSE - StartTimer(MFARESPONSE_TOUT) of "
								 << MFA_RESPONSE_TIME << " seconds, Name - " << PARTYNAME;
		  StartTimer(MFARESPONSE_TOUT, MFA_RESPONSE_TIME * SECOND);
		  /* need to close all opened UDP ports */
		  SendClosePort();

	  } else
		  m_pTaskApi->NetDisConnect(m_seqNum,cause);      
  //}
}
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetClearConnect(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetClearConnect : Name - ",PARTYNAME);

 	ON(m_isClearBeforeDisc);

	NET_CLEAR_IND_S netClearStruct;
	memset(&netClearStruct,0,sizeof(NET_CLEAR_IND_S));
	pParam->Get((BYTE*)(&netClearStruct),sizeof(NET_CLEAR_IND_S));

	NET_DISCONNECT_IND_S netDisconStruct;
	netDisconStruct.net_common_header.span_id = netClearStruct.net_common_header.span_id;
	netDisconStruct.net_common_header.net_connection_id = netClearStruct.net_common_header.net_connection_id;
	netDisconStruct.net_common_header.virtual_port_number = netClearStruct.net_common_header.virtual_port_number;
	netDisconStruct.net_common_header.physical_port_number = netClearStruct.net_common_header.physical_port_number;
	//set the cause value to be NORMAL
	netDisconStruct.cause.cause_val= 16;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)(&netDisconStruct),sizeof(NET_DISCONNECT_IND_S));

	//simulate disconnect process
	DispatchEvent(NET_DISCONNECT_IND, pSeg);

	POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetClearSetup(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetClearSetup : Name - ",PARTYNAME);

  //Means we disconnected form LOCAL - and we already sent NET_SETUP_REQ
  // while waiting for the NET_CONNECT_IND we recieved disconnect_ind
  // if( (m_callType == DIALOUT) && (m_NumOfRetry > 0) ){
//     DeleteTimer(CLEARTOUT);
//     WORD redial_timeout = 20;//(::GetpSystemCfg()->GetBondingRedialDelay())*10;
//     PTRACE2(eLevelError,"CNetChnlCntl::OnNetClearSetup - wait and redial: Name - ",PARTYNAME);
//     StartTimer(REDIALTOUT,redial_timeout); //This code should be opened
//     return;
//   }
  
//   BYTE  coding_standard,location,cause;
  BYTE cause=GetNetClearCause(pParam);
//   *pParam >> coding_standard >> location >> cause;
//   // warning for race condition - dial out call while dial in call already took the port
//   // cause 44 should return to the dial out call
//   if(m_callType == DIALIN && cause == 44){
// 		PTRACE2(eLevelError,"CNetChnlCntl::OnNetClearSetup requested circuit/channel not available (44) received for dial in party : Name - ",PARTYNAME);
//   }
  m_pTaskApi->NetConnect(m_seqNum,statIllegal,cause);
  OnNetClear(cause);
}


/*
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetBadReqSetup(CSegment* pParam)
{
  PTRACE2(eLevelError,"CNetChnlCntl::OnNetBadReqSetup : illegal request!!! Name - ",PARTYNAME);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetBadReqConnect(CSegment* pParam)
{
  PTRACE2(eLevelError,"CNetChnlCntl::OnNetBadReqConnect : illegal request!!! Name - ",PARTYNAME);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetErorrSetup(CSegment* pParam)
{
  PTRACE2(eLevelError,"CNetChnlCntl::OnNetErorrSetup :  Name - ",PARTYNAME);
  WORD  cause;
  *pParam >> cause;
   if ( m_clearReq )  {
    m_state = IDLE;
    DeleteTimer(CLEARTOUT);
    OFF(m_clearReq);
    m_pTaskApi->EndNetDisConnect(m_seqNum,statTout);  
   }
   else {
    DeleteTimer(SETUPTOUT);
    Disconnect(LOCAL);
    m_pTaskApi->NetConnect(m_seqNum,statTout);
   }

}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetErorrConnect(CSegment* pParam)
{
  PTRACE2(eLevelError,"CNetChnlCntl::OnNetErorrConnect :  Name - ",PARTYNAME);
  WORD  cause;
  *pParam >> cause;
}
*/
////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnNetClear(BYTE cause)
{
  TRACESTR (eLevelInfoNormal) << "CNetChnlCntl::OnNetClear :  Name - " << PARTYNAME;
  // if ( m_clearReq ) //In case we already sent CLEAR_REQ
  //     {
  //	if(strcmp(NameOf(),"CNetChnlCntlT1")!=0)  // T1 Cas already disconnected TS (when sending clear req)
  //		m_pNet->ConnectStream(m_pTsStream,DISCONNECT_NET_TS);
  DeleteTimer(CLEARTOUT);
  //OFF(m_clearReq);
  //uDI tmp - Send Close media
  //Udi Tmp - !! This should be moved to get ACK on CLOSE all media
  
  //m_pTaskApi->EndNetDisConnect(m_seqNum,statOK); 
  //    }
  //   else 
  //     {
  //       if(m_state != SETUP) 
  // 	{
  // 	  PASSERT(101);
  // ON(m_isClearBeforeDisc);
  //BYTE  coding_standard = 0,location = 0 ,cause = 16;
  //*pSeg << coding_standard << location << cause;
	  
  //DispatchEvent(NET_DISCONNECT_IND,pSeg);
  //POBJDELETE(pSeg);
  // 	}
  //       else
  // 	{
  // 	  PASSERT(102);
  // 	  m_state = IDLE;
  // 	}
  //     }

  DisconnectHardware();
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::Disconnect(WORD  src)
{
  CNetCause   cause;              
  //m_disconnectSrc = src; 
  //ON(m_clearReq);
  DeleteTimer(SETUPTOUT);
  StartTimer(CLEARTOUT,CLEAR_TOUT);                                    

  m_state=DISCONNECTING_NET;
  // if disconnect_ind arrived first send clear_req, if clear_ind arrived first simulate clear_ind.
  if(!m_isClearBeforeDisc && LOCAL == src)
      m_pNetInterface->Clear(cause,*m_pNetSetup);
  else if (m_isClearBeforeDisc)
      DispatchEvent(NET_CLEAR_IND, NULL);
}


/*
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnPartyConnectTsConnect(CSegment* pParam)
{
	//connect the net TS here, we use when we do not connect the Net TS
  //in 'OnSetup' by using OnSetup(FALSE)
  PASSERT(!m_pNet);
  PASSERT(!m_pTsStream);
  if ( m_pNet && m_pTsStream)
     m_pNet->ConnectStream(m_pTsStream,CONNECT_NET_TS);

}*/
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnTimerSetup(CSegment* pParam)
{
  PTRACE2(eLevelError,"CNetChnlCntl::OnTimerSetup : \'TIME OUT\' , Name - ",PARTYNAME);
  //PASSERT(1);
  m_disconnectStatus=statTout;
  m_pTaskApi->NetConnect(m_seqNum,m_disconnectStatus);
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnTimerClearDisconnectingNet(CSegment* pParam)
{
  TRACESTR (eLevelError) << "CNetChnlCntl::OnTimerClearDisconnectingNet  \'TIME OUT\' , Name - " << PARTYNAME;
  //PASSERT(1);
  m_disconnectStatus=statTout;
  OnNetClear(statTout);
}   
///////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnPartyDisconnectSetup(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyDisconnectSetup : Name - ",PARTYNAME);
  OnPartyDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnPartyDisconnectConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyDisconnectConnect : Name - ",PARTYNAME);
	OnPartyDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnPartyDisconnectDisconnectNet(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyDisconnectDisconnectNet: Name - ",PARTYNAME);
  CNetCause   cause; 
  m_pNetInterface->DisconnectReq(cause,*m_pNetSetup);//Send Disconnect_Ack_req
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnPartyDisconnect(CSegment* pParam)
{
	DeleteTimer(SETUPTOUT);
	//m_NumOfRetry = 0; // to prevent redialing while disconnecting
	//if ( m_disconnectSrc != REMOTE )
	Disconnect(LOCAL);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnPartyDisconnectIdle(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnPartyDisconnectIdle : Name - ",PARTYNAME);
	m_state = SETUP;                  
  OnPartyDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////

void  CNetChnlCntl::OnNetClearDisconnNet(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnNetClearDisconnHardware : Name - ",PARTYNAME);
  OnNetClear(GetNetClearCause(pParam));
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CNetChnlCntl::GetNetClearCause(CSegment* pParam)
{
  if(!pParam)
	  return 0;
  NET_CLEAR_IND_S netClearStruct;
  memset(&netClearStruct,0,sizeof(NET_CLEAR_IND_S));
  pParam->Get((BYTE*)(&netClearStruct),sizeof(NET_CLEAR_IND_S));
  return (netClearStruct.cause.cause_val);
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnNetDisconnectAckDiscNet(CSegment* pParam)
{
  TRACESTR (eLevelInfoNormal) << "CNetChnlCntl::OnNetDisconnectAckDiscNet " << PARTYNAME ;
  OnNetClear(GetNetDisccAckIndCause(pParam));
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnNetDisconnectAckSetup(CSegment* pParam)
{
  TRACESTR (eLevelInfoNormal) << "CNetChnlCntl::OnNetDisconnectAckSetup " << PARTYNAME;
  OnNetClear(GetNetDisccAckIndCause(pParam));
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CNetChnlCntl::GetNetDisccAckIndCause(CSegment* pParam)
{
  NET_DISCONNECT_ACK_IND_S disccAckIndStruct;
  memset(&disccAckIndStruct,0,sizeof(NET_DISCONNECT_ACK_IND_S));
  pParam->Get((BYTE*)(&disccAckIndStruct),sizeof(NET_DISCONNECT_ACK_IND_S));
  return(disccAckIndStruct.cause.cause_val);
}

/*
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::OnTimerRedial(CSegment* pParam){

	PTRACE2(eLevelError,"CNetChnlCntl::OnTimerRedial : Name - ",PARTYNAME);
	if( (m_callType != DIALOUT) || (m_NumOfRetry <= 0) || m_state != SETUP ){
		PTRACE2(eLevelInfoNormal,"CNetChnlCntl::OnTimerRedial - bad request : Name - ",PARTYNAME);
		return;
	}
	m_NumOfRetry--;
	//restart setup timer and send setup request
	StartTimer(SETUPTOUT,SETUP_TOUT);
	m_pNet->Setup(*m_pNetSetup);  
}
/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::DisconnectTS()
{
	if(::isValidPObjectPtr(m_pNet)){
		m_pNet->ConnectStream(m_pTsStream,DISCONNECT_NET_TS);
	}
}

*/
/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnAckIndDisconnHardware(CSegment* pParam)
{
  TRACESTR(eLevelInfoNormal) << " NetChnlCntl::OnAckIndDisconnHardware : Name - " << PARTYNAME;
  OnAckInd(pParam);
}


/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnAckInd(CSegment* pParam)
{
  cmCapDirection channelDirection;
  kChanneltype channelType;
  STATUS ackStatus = STATUS_OK;
  APIU32 ackReason = 0;

  ackStatus = m_pNetInterface->GetAckIndParams(channelType, channelDirection, ackReason, pParam);

  // Update the ACK Table
  AckTableKey ackKey(channelType, channelDirection);
  TableIterator it = m_ackStatusTable.find(ackKey);

  // If we got wrong ACK or we already received an ACK
  if (STATUS_OK != ackStatus || it == m_ackStatusTable.end() || it->second)
  {
    TRACESTR(eLevelError) << " CNetChnlCntl::OnAckInd :  ACK_IND status = " << ackStatus << ", Name = " << PARTYNAME;
    m_pTaskApi->SetFaultyResourcesToPartyControlLevel(STATUS_FAIL);
    PASSERT_AND_RETURN(m_pNetInterface->GetPartyRsrcId());
  }

  // Set the value of the ACK table to TRUE
  it->second = true;

  if (IsAllAcksReceived())
  {
    switch (m_state)
    {
      case CONNECT_HARDWARE:
      {
        OnHardwareConnected();
        break;
      }
      case DISCONNECTING_HARDWARE:
      {
        OnHardwareDisconnected();
        break;
      }
      case DISCONNECTING_NET:
      { //in case of board full we need to send update to Party
        DeleteTimer(CLEARTOUT);
        m_pTaskApi->NetDisConnect(m_seqNum, BOARD_FULL_CAUSE);
        break;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnAckIndConnectHardware(CSegment* pParam)
{
  TRACESTR(eLevelInfoNormal) <<" CNetChnlCntl::OnAckIndConnectHardware : Name - " << PARTYNAME;
  OnAckInd(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::StartNetCntl()
{
  DispatchEvent(CONNHARDWARE);
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnConnectHardwareIdle(CSegment* pParams)
{
  
  TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnConnectHardwareIdle : Name - " << PARTYNAME;
  m_state=CONNECT_HARDWARE;
  
  //Start Opennig the ports
  InitAckTable();

  TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnConnectHardwareIdle StartTimer(MFARESPONSE_TOUT) of "
						  << MFA_RESPONSE_TIME << " seconds , Name - " << PARTYNAME;
  StartTimer(MFARESPONSE_TOUT, MFA_RESPONSE_TIME * SECOND);
  
  CIsdnVideoParty* video_party = dynamic_cast<CIsdnVideoParty*>(m_pParty);

  m_pNetInterface->ConnectNetToAudioMux();
  if (!video_party)
	  m_pNetInterface->ConnectAudioToNet();
  else
	  m_pNetInterface->ConnectMuxToNet();
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnHardwareConnected()
{
  TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnHardwareConnected connId=" << m_pNetInterface->GetRsrcParams()->GetConnectionId() << " , Name - " << PARTYNAME;
  DeleteTimer(MFARESPONSE_TOUT);
  
  if (DIALOUT == m_callType )
    {
      TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnHardwareConnected Dialout call : Name - " << PARTYNAME;
      DispatchEvent(CALLOUT);
    }
  else
    {
      TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnHardwareConnected DialIn call : Name - " << PARTYNAME;
      DispatchEvent(NET_SETUP_IND);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnMfaTimeoutConnectHardware(CSegment* pParams)
{
  TRACESTR (eLevelError) << " CNetChnlCntl::OnMfaTimeoutConnectHardware We did not Recieve Ack for: " << MFA_RESPONSE_TIME << " seconds , Name - " << PARTYNAME;
  PASSERT(m_pNetInterface->GetPartyRsrcId());
  m_pTaskApi->SetFaultyResourcesToPartyControlLevel(STATUS_FAIL);
  IsAllAcksReceived();
  DisconnectHardware();
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnMfaTimeoutDisconnectHardware(CSegment* pParams)
{
  TRACESTR (eLevelError) << " CNetChnlCntl::OnMfaTimeoutDisconnectHardware : We did not Recieve Ack for: " << MFA_RESPONSE_TIME << " seconds , Name - " << PARTYNAME;
  PASSERT(m_pNetInterface->GetPartyRsrcId());
  IsAllAcksReceived(); //Just printing the acks status
  TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnMfaTimeoutDisconnectHardware  : Name - " << PARTYNAME;
  DeleteTimer(MFARESPONSE_TOUT);
  m_pTaskApi->SetFaultyResourcesToPartyControlLevel(STATUS_FAIL);
  m_pTaskApi->EndNetDisConnect(m_seqNum,statTout);
  m_state = IDLE;
}


/////////////////////////////////////////////////////////////////////////////
bool CNetChnlCntl::IsAllAcksReceived()
{
  std::string msgStr;
  unsigned int numberOfAcks=0;
  for(TableIterator it = m_ackStatusTable.begin(); it != m_ackStatusTable.end() ; ++it)
    if (it->second) {
      msgStr += "(";
      msgStr += it->first.GetChannelTypeAsString();
      msgStr += ",";
      msgStr += it->first.GetDirectionAsString();
      msgStr += "), ";
      ++numberOfAcks;
    }
  std::string openOrClose;
  if(CONNECT_HARDWARE == m_state)
    openOrClose="OPEN_UDP";
  else
    openOrClose="CLOSE_UDP";
  
  TRACESTR(eLevelInfoNormal) << " CNetChnlCntl::IsAllAcksReceived for partyId: " << m_pNetInterface->GetPartyRsrcId()
						 <<", Received " << numberOfAcks <<" " << openOrClose << " Acks so far: " << msgStr;
  //Is all ack received
  return ( m_ackStatusTable.size() == numberOfAcks ? true : false );
}

/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::InitAckTable()
{
  m_ackStatusTable.clear();
  CIsdnVideoParty* video_party = dynamic_cast<CIsdnVideoParty*>(m_pParty);
  m_ackStatusTable[AckTableKey(kRtmChnlType,cmCapReceive)]=false;
  m_ackStatusTable[AckTableKey(kRtmChnlType,cmCapTransmit)]=false;
  if (!video_party) {
	m_ackStatusTable[AckTableKey(kPstnAudioChnlType,cmCapReceive)]=false;
	m_ackStatusTable[AckTableKey(kPstnAudioChnlType,cmCapTransmit)]=false;
  }
  else {
	m_ackStatusTable[AckTableKey(kIsdnMuxChnlType,cmCapReceive)]=false;
	m_ackStatusTable[AckTableKey(kIsdnMuxChnlType,cmCapTransmit)]=false;
  }
}
/////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::OnHardwareDisconnected()
{
  TRACESTR (eLevelInfoNormal) << " CNetChnlCntl::OnHardwareDisconnected : Name - " << PARTYNAME;
  DeleteTimer(MFARESPONSE_TOUT);
  m_pTaskApi->EndNetDisConnect(m_seqNum,m_disconnectStatus);
  m_state = IDLE;
}

///////////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::SetNewConfRsrcId(DWORD confRsrcId)
{
  TRACESTR (eLevelInfoNormal) <<" CNetChnlCntl::SetNewConfRsrcId to " << confRsrcId << " , Name - " << PARTYNAME;  
  m_pNetInterface->SetConfRsrcId(confRsrcId);
}

///////////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::DisconnectHardware()
{
  //close All ports
  m_state = DISCONNECTING_HARDWARE;
  TRACESTR (eLevelInfoNormal) <<" CNetChnlCntl::DisconnectHardware StartTimer(MFARESPONSE_TOUT) of " << MFA_RESPONSE_TIME << " seconds , Name - " << PARTYNAME;
  StartTimer(MFARESPONSE_TOUT, MFA_RESPONSE_TIME * SECOND);
  SendClosePort();
}

///////////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::SendClosePort()
{
  TableIterator it = m_ackStatusTable.begin();
  for ( ; it !=  m_ackStatusTable.end() ; ++it)
    if (it->second) {//Close only open ports
	    m_pNetInterface->ClosePort(it->first.m_channelType,it->first.m_direction);
		it->second= false ; //Wait for ack on close port
	}
    else
        it->second= true ; //Do not wait to ACK on closed port
}

///////////////////////////////////////////////////////////////////////////////////
void  CNetChnlCntl::HandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
  DispatchEvent(opCode,pMsg); 
}

///////////////////////////////////////////////////////////////////////////////////
void CNetChnlCntl::UpdateNetParams(WORD spanId, WORD net_connection_id, WORD physical_port_number)
{
  //If we already have values in the port and in the span than do not update
  if (0 != m_pNetSetup->m_physical_port_number && 0 != m_pNetSetup->m_spanId[0] )
    {
      TRACESTR(eLevelInfoNormal)<<"CNetChnlCntl::UpdateNetParams : Not updating SpanId="<< m_pNetSetup->m_spanId[0]
							<<", portID=" << m_pNetSetup->m_physical_port_number << " , Name - " << PARTYNAME;
      return;
    }
  m_pNetSetup->m_spanId[0]=spanId;
  m_pNetSetup->m_net_connection_id=net_connection_id; 
  m_pNetSetup->m_physical_port_number=physical_port_number;
}

/* The function is called after BOARD_FULL */
void CNetChnlCntl::Reconnect(CIsdnNetSetup& rNetSetup)
{
    /* update CIsdnNetSetup */
    *m_pNetSetup = rNetSetup;

	/* send CONFPARTY_CM_OPEN_UDP_PORT_REQ & NET_SETUP_REQ to MPL */
	DispatchEvent(CONNHARDWARE);
}
