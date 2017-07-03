//+========================================================================+
//                     EndpointH323.h							           |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointH323.h											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//+========================================================================+

#ifndef __ENDPOINTH323_H__
#define __ENDPOINTH323_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "H323CsInd.h"
#include "H323CsReq.h"
#include "IpCsContentRoleToken.h"
#include "Endpoint.h"


////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//
class CCapSet;
class CCapSetsList;
class CH323Behavior;
class CH323BehaviorList;


////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//

/////// for API Commands following //////// 
#define SIM_H323_MIN_API_COMMANDS		0
//----
#define SIM_H323_RCV_SETUP				1
#define SIM_H323_SND_RING_BACK_CNTL		2
#define SIM_H323_SND_CALL_CONNECTED		3
#define SIM_H323_RCV_CREATE_CNTL		4
#define SIM_H323_SND_CAP_CNTL			5
#define SIM_H323_SND_CAP_RESPONSE		6
#define SIM_H323_SND_CNTL_CONNECTED		7
#define SIM_H323_RCV_INCOMING_A_CHNL	8  // receive incoming audio channel
#define SIM_H323_RCV_INCOMING_V_CHNL	9  // receive incoming video channel
#define SIM_H323_RCV_INCOMING_F_CHNL	10 // receive incoming fecc channel
#define SIM_H323_SND_INCOMING_A_CHNL	11 // send incoming audio channel
#define SIM_H323_SND_INCOMING_V_CHNL	12 // send incoming video channel
#define SIM_H323_SND_INCOMING_F_CHNL	13 // send incoming fecc channel
#define SIM_H323_RCV_OUTGOING_A_CHNL	14 // receive outgoing audio channel
#define SIM_H323_RCV_OUTGOING_V_CHNL	15 // receive outgoing video channel
#define SIM_H323_RCV_OUTGOING_F_CHNL	16 // receive outgoing fecc channel
#define SIM_H323_SND_OUTGOING_A_CHNL	17 // send outgoing audio channel
#define SIM_H323_SND_OUTGOING_V_CHNL	18 // send outgoing video channel
#define SIM_H323_SND_OUTGOING_F_CHNL	19 // send outgoing fecc channel
#define SIM_H323_RCV_CALL_ANSWER		20
#define SIM_H323_RCV_CALL_DROP			21
#define SIM_H323_RCV_INCOMING_239_CHNL  22 // receive incoming H.239 channel
#define SIM_H323_SND_INCOMING_239_CHNL	23 // send incoming H.239 channel
#define SIM_H323_RCV_OUTGOING_239_CHNL	24 // receive outgoing H.239 channel
#define SIM_H323_SND_OUTGOING_239_CHNL	25 // send outgoing H.239 channel
// add here...
//----
#define SIM_H323_MAX_API_COMMANDS		30

//StateMachine Internal Event and Timer Event
enum EndpointH323Event {
	SIM_H323_TOUT_DELAY_SEND_INCOMING_CHANNEL_IND = 323000
	//add new event here...
};

///////////////////////////////////////////

#define MAX_SIM_ALIAS_NAME_SIZE			30

const BYTE SIM_RECAP_MODE_NONE   = 0;
const BYTE SIM_RECAP_MODE_BEFORE = 1;
const BYTE SIM_RECAP_MODE_AFTER  = 2;

class CMplMcmsProtocol;


typedef struct {
	DWORD	channelType;
	DWORD	channelRole;
	DWORD	channelIndex;
	DWORD	channelMcIndex;
	DWORD	channelDirection;
	BYTE	openedInSim;
	BYTE	openedInConf;
} TChannelDetails;

enum enChannelTypes {
	eAudioChannelIn  = 0,
	eAudioChannelOut,
	eVideoChannelIn,
	eVideoChannelOut,
	eFeccChannelIn,
	eFeccChannelOut,
	eH239ChannelIn,
	eH239ChannelOut,
	eNumOfChannels // should be last
};

enum enDiscoInitiator {
	eInitiatorMe   = 0,
	eInitiatorConf = 1,
	eInitiatorUnknown = 2 // last
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEndpointH323 : public CEndpoint
{
	CLASS_TYPE_1(CEndpointH323,CEndpoint)

public:
	// constructors, distructors and operators
	CEndpointH323( CTaskApi* pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav );
	virtual const char* NameOf() const { return "CEndpointH323";}
	CEndpointH323(const CEndpointH323& other);
	virtual ~CEndpointH323();
	CEndpointH323& operator =(const CEndpointH323& other);

		// virtual functions
		//virtual const char* GetIpStr() { return m_IpH225RemoteStr; }
	virtual char* GetIpStr() const;
	virtual void GetIpStr(char* str) const;

	virtual void SetIp( const char* pszEpIp );
	
		//virtual const DWORD GetIpDword() { return m_IpH225Remote; }
	virtual DWORD GetIpDword() { return 0; }
	
	virtual void SetIpVersion(DWORD ipVer);
	virtual DWORD GetIpVersion() const;
	void SetSourcePartyAlias(char* aliasName);

	virtual void SetTransportAddress( mcTransportAddress* mcTA );
	virtual mcTransportAddress* GetTransportAddress() { return &m_H225RemoteIpAddress; }	
	
	virtual void OnGuiConnectEp();
	virtual void OnGuiDisconnectEp();
	virtual void OnGuiUpdateEp(CSegment* pParam);
	virtual void OnGuiDeleteEp();
	virtual eEndpointsTypes GetEpType() const { return eEndPointH323; }
	virtual void SetArrayIndex( const WORD ind );
	virtual void Serialize( CSegment& rSegment ) const;
	virtual void SerializeDetails( CSegment& rSegment ) const;
	virtual void DeSerialize(CSegment& rParam);
	virtual void HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol );
	virtual void SendChangeMode() {};//temporary
	virtual BOOL IsFeccCall() const;
	virtual BOOL IsH239Call() const;
	virtual void UpdateChannels(const bool au=true,const bool vi=true,const bool fecc=true,const bool h239=true,
						const BYTE recapMode=0,const char* pszManufacturerName="No change", const CCapSet *pCapSet=0);
	virtual BOOL IsLprCall() const;

		// basic functions
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	void	StartCallOffering();
	static void SetChannelDetails( 	TChannelDetails* pChannel,
									const DWORD channelType,
									const DWORD channelRole,
									const DWORD channelIndex,
									const DWORD channelMcIndex,
									const DWORD channelDirection,
									const BOOL openedInSim  = FALSE,
									const BOOL openedInConf = FALSE );

protected:
		// virtual functions
	virtual void CleanAfterDisconnect();
	virtual BOOL IsConnectionCompleted() const;
	virtual void Disconnect( CMplMcmsProtocol* pMplProtocol );
	virtual void SendMuteIndication() const;
	virtual void SendUnmuteIndication() const;
	virtual void SendFeccTokenRequestIndication() const;
	virtual void SendFeccTokenReleaseIndication() const;
	virtual void SendFeccKeyRequestIndication(const char* pszDtmf) const;
	virtual void SendH239TokenRequestIndication();
	virtual void SendH239TokenReleaseIndication();
	virtual void SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout);

		// action functions
	virtual void OnCallAnswerReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCsCallSetupReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCsCreateCntlReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCsReCapReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCsIncomingChnlResponse( CMplMcmsProtocol* pMplProtocol );
	virtual void OnOutgoingChnlReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnChnlDropReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCallDropReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCallCloseConfirmReq( CMplMcmsProtocol* pMplProtocol );
	virtual void OnRoleTokenReq(CMplMcmsProtocol* pMplProtocol);
	virtual void OnChannelOnReq(CMplMcmsProtocol* pMplProtocol){};
	virtual void OnChnlNewRateReq(CMplMcmsProtocol* pMplProtocol);
	void	OnRoleTokenOwnerTout(CSegment* pParam);
	void OnTimerDelaySendIncomingChanInd(CSegment* pParam);
	void    OnRssCmdReq( CMplMcmsProtocol* pMplProtocol );
	

		// other functions
	void 	UpdateIncomingChannByNewCap(TChannelDetails *pChannel, const CCapSet &rCapCommon);
	int 	GetActionForChannel(bool isOpened, DWORD prevPT, DWORD newPY);
	int		IsLeagaParametersCallSetup( mcReqCallSetup *req );
	void	SendCsCallRingBackInd( CMplMcmsProtocol* pMplProtocol );
	virtual void SendCsCallConnectedInd( CMplMcmsProtocol* pMplProtocol );
	int		IsLeagaParametersCreateCntl( mcReqCreateControl *req );
	void	SendCapabilitiesInd( const CCapSet& cap, CMplMcmsProtocol* pMplProtocol );
//	void	SendCapabilitiesIndOld( CMplMcmsProtocol* pMplProtocol );
	void	SendCsCapResponseInd( CMplMcmsProtocol* pMplProtocol );
	void	SendCsCntlConnectedInd( CMplMcmsProtocol* pMplProtocol, APIU32 masterSlaveStat=cmMSSlave );
	int		IsLegalParametersIncomingChnlResponse( mcReqIncomingChannelResponse *req );
	void	SendIncomingChannelInd( CMplMcmsProtocol* pMplProtocol, TChannelDetails* pChannel, const CCapSet& rCapCommon );
	void	SendIncomingChannelConnectedInd( CMplMcmsProtocol* pMplProtocol, const TChannelDetails* pChannel );
	void	SendOutgoingChannelResponseInd( CMplMcmsProtocol* pMplProtocol, TChannelDetails* pChannel );
	void	SendStartChnlCloseInd(const CMplMcmsProtocol* pMplProtocol,TChannelDetails* pChannel);
	void	SendChannelCloseInd(const CMplMcmsProtocol* pMplProtocol,TChannelDetails* pChannel);
	void	WhenAllChannelsClosed( CMplMcmsProtocol* pMplProtocol );
	void	SendCallIdleInd( CMplMcmsProtocol* pMplProtocol );
	//int		RetrieveIPFromString( WORD srcOrDest);
	void    FillMcTransportAddress( mcXmlTransportAddress* pAddrs,
										APIU32 ipVersion,
										APIU32 ip,
										APIU32 port,
										APIU32 distribution,
										APIU32 transportType);
										
	void    FillMcTransportAddress( mcXmlTransportAddress* pAddrs,
									mcTransportAddress mcTA);
										
	void	SendCallOfferingInd();
	void	GetUniqueConfId( char* unique );
	void	GetUniqueCallId( char* unique );
	void InsertIPandPortToString( char* addressStr,
								  char* prefix,
								  char* ip,
								  char* port,
								  char* aliasName,
								  char* aliasPar1,
								  char *aliasPar2 );
	TChannelDetails*  GetChannel(const DWORD channelType,const DWORD channelRole,const DWORD channelDirection);
	TChannelDetails*  GetChannelByIndex(const DWORD channelIndex);
	void SendH239TokenWithdrawResponse();
	void SendFacilityInd( CMplMcmsProtocol* pMplProtocol );
	
	
	void SetIpV6Address();
	
	
	enScopeId GetLocalScopeId();
	enScopeId GetRemoteScopeId();
	

	bool IsIncomingChannCanOpen(TChannelDetails *chann);

protected:
	DWORD	m_conferenceType;
	DWORD	m_maxRate;
	WORD	m_apiArray[SIM_H323_MAX_API_COMMANDS];

		// alias
	char	m_targetAliasNamePar1[MAX_SIM_ALIAS_NAME_SIZE];
	char	m_targetAliasNamePar2[MAX_SIM_ALIAS_NAME_SIZE];
	char	m_targetAliasNameOrTel[16];
	char	m_sourceAliasNamePar[MAX_SIM_ALIAS_NAME_SIZE];
	char	m_sourceAliasNameOrTel[H243_NAME_LEN];
	
			// channels details
	TChannelDetails		m_taChannels[eNumOfChannels];
	DWORD	m_isReopenChannel;
// v4.1C <--> v6 merge temp!!!
	DWORD	m_lenReopenTimer;

		// remote
/*	char	m_IpH225RemoteStr[16];
	char	m_IpH245RemoteStr[16];
	char	m_IpRtpRemoteStr[16];

		// gets from MCMS
	char	m_IpH225LocalStr[16];
	char	m_IpH245LocalStr[16];
	char	m_IpRtpLocalStr[16];

	DWORD	m_isReopenChannel;
	DWORD	m_lenReopenTimer;
		// port
	DWORD	m_portH245Remote;
	DWORD	m_portH245Local;
	DWORD	m_portH225Remote;
	DWORD	m_portH225Local;
	DWORD	m_portRtpRemote;
	DWORD	m_portRtpLocal;
		// Ip
	DWORD	m_IpRtpRemote;
	DWORD	m_IpRtpLocal;
	DWORD	m_IpH245Remote;
	DWORD	m_IpH245Local;
	DWORD	m_IpH225Remote;
	DWORD	m_IpH225Local;*/

	
		// IpV6	
	// H225 - Call Signaling address (and RAS=Remote Access Service)
	mcTransportAddress	m_H225RemoteIpAddress;
	mcTransportAddress	m_H225LocalIpAddress;
	
	// H245 - Control Channel address
	mcTransportAddress	m_H245RemoteIpAddress;
	mcTransportAddress	m_H245LocalIpAddress;

	// RTP - UDP address
	mcTransportAddress	m_rtpRemoteIpAddress;
	mcTransportAddress	m_rtpLocalIpAddress;




		// open
	mcReqCallSetup					m_mcReqCallSetup;
	mcReqCreateControl				m_mcReqCreateControl;
	mcReqOutgoingChannel			m_outGoingChannelReq;
	mcReqIncomingChannelResponse	m_mcIncomingChannelResponse;
	mcReqCallAnswer					m_mcReqCallAnswer;

		// close
	mcReqChannelDrop				m_mcReqChannelDrop;
	mcReqCallDrop					m_mcReqCallDrop;

	
	BOOL	m_isLPRCap;

		// 	encryption
	encTokensHeaderStruct* m_pEncryptionStruct;

		// disconnect initiator
	enDiscoInitiator	m_enDiscoInitiator;

//	DWORD	m_nMcuId;
//	DWORD	m_nTerminalId;
//	ERoleTokenOpcode  m_enRoleTokenLastCmd;

  	PDECLAR_MESSAGE_MAP
};

#endif // __ENDPOINTH323_H__ 






