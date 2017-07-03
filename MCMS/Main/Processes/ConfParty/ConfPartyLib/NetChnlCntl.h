#ifndef _NETCHNLCNTL
#define _NETCHNLCNTL


#include <map>

//#include "NetWorkChnlCntl.h"
#include "StateMachine.h"
#include "ConfDef.h" //declarations SETUP & CONNECT state
#include "NetHardwareInterface.h"
#include "OpcodesMcmsCommon.h"

class CParty;
class CPartyApi;

//States
const WORD CONNECT_HARDWARE       = 4;
const WORD DISCONNECTING_NET      = 5;
const WORD DISCONNECTING_HARDWARE = 6;
//--------------------------------------------


//const WORD SETUP        = 1;
const WORD ONSETUP      = 2;
//const WORD CONNECT      = 3;  

const WORD CALLOUT           = 100;
const WORD LDISCONNECT       = 101;
const WORD SETUPTOUT         = 103;
const WORD CLEARTOUT         = 104;
const WORD CONNECTNETTS      = 105;
							// atm events
const  WORD TRANSPHONE2ADDR  = 106;
const  WORD TRANSLATETOUT    = 107;
							// delay between redial
const WORD REDIALTOUT		 = 108;
const WORD CONNECT_ONLY_REQ	 = 109;
const WORD CONNHARDWARE      = 110;													 

                             // timers size
const DWORD SETUP_TOUT		= 15*SECOND;
const DWORD SETUP_PSTN_TOUT	= 45*SECOND;

//const DWORD TRANSLATE_TOUT  = 60*SECOND;                           
const DWORD CLEAR_TOUT  = 15*SECOND;                             
//const DWORD CLEAR_T1_WINK_TOUT = 60*SECOND;
							//retry setup
#define NUM_OF_RETRY  1;                             

//------------------------- Acks Table Key --------------

struct AckTableKey
{
  
public:
  AckTableKey(kChanneltype channelType,cmCapDirection direction)
    :m_channelType(channelType),m_direction(direction){}
  
  bool operator < (const AckTableKey & other)const
  {
    if (m_channelType < other.m_channelType)
      return true;
    else if ( m_channelType == other.m_channelType &&
	      m_direction < other.m_direction)
      return true;
    return false;
  }

  const char* GetChannelTypeAsString()const;
  const char* GetDirectionAsString()const{return ( (m_direction == cmCapReceive) ? "Receive": "Transmit");}
  
  kChanneltype m_channelType;
  cmCapDirection m_direction;
};
//------------------------------------------------------


class CNetChnlCntl : public CStateMachine  {
	CLASS_TYPE_1(CNetChnlCntl,CStateMachine )
public:
  typedef std::map< AckTableKey, bool > ChannelTypeToAckTable;
  typedef ChannelTypeToAckTable::iterator TableIterator;
  
	// Constructors
	virtual const char* NameOf() const { return "CNetChnlCntl";}
	CNetChnlCntl();
	virtual ~CNetChnlCntl();  
	 
	// Initializations  
	void  Create(CRsrcParams& rNetRsrcParams,CIsdnNetSetup& rNetSetup,CParty* pParty,WORD seqNum,WORD callType);
	//void  Update(CTimeSlotStream* pTsStream,CParty* pParty,WORD seqNum);						 
	// void  SetStream(CTimeSlotStream* pTsStream);						 
	void  Destroy();                                                
//	virtual void  ConnectTS();
//	virtual void  DisconnectTS();
	// Operations
	virtual void  HandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode); 
// 	virtual void   Dump(WORD swithcFlag = 0);
	virtual void*          GetMessageMap(); 
	//  Api
	virtual void  Setup(); 
	virtual void  OnSetup(WORD NetTsConnection = TRUE);
	virtual void  Clear(); 
//	virtual void  ClearOnly() {};
//	virtual void  SendWinkStart();
//	virtual void  SendWinkStartT1() {};
	void StartNetCntl();
	
	// Action functions
	void VsrNullActionFunction(CSegment*);
	void MsSvcPliNullActionFunction(CSegment*);
	virtual void  OnPartyCalloutIdle(CSegment* pParam);
	virtual void  OnNetSetupIndIdle(CSegment* pParam);
	void  OnNetConnectSetup(CSegment* pParam);
	void  OnNetDisconnectSetup(CSegment* pParam);  
	void  OnNetDisconnectConnect(CSegment* pParam);
	void  OnNetDisconnectConnectHardware(CSegment* pParam);
	void  OnNetClearSetup(CSegment* pParam);
	void  OnNetClearConnect(CSegment* pParam);
	void  OnNetClearDisconnNet(CSegment* pParam);
	void  OnNetDisconnectAckConnect(CSegment* pParam);
	void  OnNetDisconnectAckSetup(CSegment* pParam);
	void  OnNetDisconnectAckDiscNet(CSegment* pParam);
	void  OnNetAlertAnycase(CSegment* pParam);
	
	BYTE  GetNetClearCause(CSegment* pParam);
	BYTE  GetNetDisccAckIndCause(CSegment* pParam);
	
	
	void OnAckIndConnectHardware(CSegment* pParam);
	void OnAckIndDisconnHardware(CSegment* pParam);
	void OnAckInd(CSegment* pParam);
	void OnHardwareDisconnected();
	
	//	void  OnNetBadReqSetup(CSegment* pParam);
//	void  OnNetBadReqConnect(CSegment* pParam);
//	virtual void  OnNetErorrSetup(CSegment* pParam);
//	virtual void  OnNetErorrConnect(CSegment* pParam);
	void OnTimerSetup(CSegment* pParam);
	void OnTimerClearDisconnectingNet(CSegment* pParam);
//	void  OnTimerRedial(CSegment* pParam);
	void OnMfaTimeoutConnectHardware(CSegment* pParams);
	void OnMfaTimeoutDisconnectHardware(CSegment* pParams);
	
	void  OnPartyDisconnectSetup(CSegment* pParam);
	virtual void  OnPartyDisconnectConnect(CSegment* pParam);
	void OnPartyDisconnectDisconnectNet(CSegment* pParam);
	void  OnPartyDisconnectIdle(CSegment* pParam);
	void  OnPartyDisconnect(CSegment* pParam); 
	void OnConnectHardwareIdle(CSegment* pParams);
	void OnHardwareConnected();
	// Action function utils
	virtual void  Disconnect(WORD src);
	void  OnNetDisconnect(CSegment* pParam);
//	void  OnPartyConnectTsConnect(CSegment* pParam);
	void  OnNetClear(BYTE cause);
	bool IsAllAcksReceived();
	void InitAckTable();
	void SetNewConfRsrcId(DWORD confRsrcId); //Move update confId

	WORD IsDisconnected(){ return ((m_state == IDLE ) ? TRUE :FALSE) ;}
	WORD IsConnected(){return ((m_state == CONNECT ) ? TRUE :FALSE) ;}

	void Reconnect(CIsdnNetSetup& rNetSetup);

	void SetUpdated(BOOL upd) { m_updated = upd; }
	BOOL IsUpdated() const { return  m_updated; }

	const CIsdnNetSetup* GetIsdnNetSetup() { return m_pNetSetup; }

//#ifdef BOND_DOWNSPEED_SIM
//	void  SimulateDisconnectFromRemote();
//#endif // BOND_DOWNSPEED_SIM
	
protected:
	void DisconnectHardware();
	void UpdateNetParams(WORD spanId,WORD net_connection_id,WORD physical_port_number);
	void SendClosePort();

	// Attributes
	//	CNet*                m_pNet;
	CNetHardwareInterface*   m_pNetInterface;
	ChannelTypeToAckTable    m_ackStatusTable;
	WORD                 m_seqNum;
	WORD                 m_callType;
	CIsdnNetSetup*       m_pNetSetup;
	CPartyApi*           m_pTaskApi;
	CParty*              m_pParty;
	BYTE m_disconnectStatus;
	
	BOOL m_updated; 
	BOOL m_isClearBeforeDisc;

	PDECLAR_MESSAGE_MAP
								// Operations
};

#endif //_NETCHNLCNTL


