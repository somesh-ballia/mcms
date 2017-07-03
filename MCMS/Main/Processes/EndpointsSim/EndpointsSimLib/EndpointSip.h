//+========================================================================+
//                     EndpointSip.h							           |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointSip.h											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//+========================================================================+

#ifndef __ENDPOINTHSIP_H__
#define __ENDPOINTHSIP_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
//#include "H323CsInd.h"
//#include "H323CsReq.h"
#include "SipStructures.h"
#include "Endpoint.h"

////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//
class CCapSet;
class CCapSetsList;
class CH323Behavior;
class CH323BehaviorList;
class CMplMcmsProtocol;


////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//

/////// for API Commands following ////////
#define SIM_SIP_MIN_API_COMMANDS		0
//----
#define SIM_SIP_RCV_INVITE				1
#define SIM_SIP_SND_INVITE				2
#define SIM_SIP_RCV_INVITE_RESPONSE		3
#define SIM_SIP_SND_INVITE_RESPONSE		4
#define SIM_SIP_RCV_INVITE_ACK			5
#define SIM_SIP_SND_INVITE_ACK			6
#define SIM_SIP_RCV_REINVITE			7
#define SIM_SIP_SND_REINVITE			8
#define SIM_SIP_RCV_BYE					9
#define SIM_SIP_SND_BYE					10
// add here...
//----
#define SIM_SIP_MAX_API_COMMANDS		20

///////////////////////////////////////////

#define MAX_SIM_ALIAS_NAME_SIZE			30



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEndpointSip : public CEndpoint
{
	CLASS_TYPE_1(CEndpointSip, CEndpoint)
public:
	// constructors, distructors and operators
	CEndpointSip( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav );
	CEndpointSip(const CEndpointSip& other);
	virtual const char* NameOf() const { return "CEndpointSip";}
	virtual ~CEndpointSip();
	CEndpointSip& operator =(const CEndpointSip& other);

		// virtual functions
		//virtual const char* GetIpStr() const { return m_IpH225RemoteStr; }
	virtual char* GetIpStr() const;	
	virtual void GetIpStr(char* str) const;
	
	virtual void SetIp( const char* pszEpIp );
	
		//virtual const DWORD GetIpDword() { return m_IpH225Remote; }
	virtual DWORD GetIpDword() { return 0; }
	
	virtual void SetIpVersion(DWORD ipVer);
	virtual DWORD GetIpVersion() const;
	
	virtual void SetTransportAddress( mcTransportAddress* mcTA ) {};
	virtual mcTransportAddress* GetTransportAddress() { return NULL;}
	
	virtual void OnGuiConnectEp();
	virtual void OnGuiDisconnectEp();
	virtual void OnGuiUpdateEp(CSegment* pParam);
	virtual void OnGuiDeleteEp();
	virtual eEndpointsTypes GetEpType() const { return eEndPointSip; }
//	virtual void SetArrayIndex( const WORD ind );
	virtual void Serialize( CSegment& rSegment ) const;
	virtual void SerializeDetails( CSegment& rSegment ) const;
	virtual void DeSerialize(CSegment& rParam);
	virtual void HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol );
	virtual void SendChangeMode();
	virtual void Mute(WORD wAudioMuteByPort, WORD wVideoMuteByPort,
						char *szAudioMuteByDirection, char *szVideoMuteByDirection,
						char *szAudioMuteByInactive, char *szVideoMuteByInactive);

	BOOL IsSIPEPNameIncludesSIM_TIP(const char* _EPName);
	void SetIsTIPEP(BOOL isTIPEP) {m_isTIPEP = isTIPEP;}
	BOOL GetIsTIPEP()const { return m_isTIPEP;}

	//void SetIsMRE(BOOL isMRE) {m_isMRE = isMRE;}
	BOOL GetIsMRE()const { return m_isMRE;}

	//void SetIsMRC(BOOL isMRC) {m_isMRC = isMRC;}
	BOOL GetIsMRC()const { return m_isMRC;}

		// basic functions
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

		// action functions
	void	StartCallOffering();
	void    SetUserAgent(const char* pszConfName);
	virtual void UpdateChannels(const bool au=true,const bool vi=true,const bool fecc=true,const bool h239=true,
						const BYTE recapMode=0,const char* pszManufacturerName="No change", const CCapSet *pCapSet=0);
	virtual BOOL IsLprCall() const { return TRUE; }

protected:
		// virtual functions
	virtual void CleanAfterDisconnect();
	virtual BOOL IsConnectionCompleted() const;
	virtual void Disconnect( CMplMcmsProtocol* pMplProtocol );
	virtual void SendMuteIndication() const;
	virtual void SendUnmuteIndication() const;
	virtual BOOL IsFeccCall() const;
	virtual void SendFeccTokenRequestIndication() const;
	virtual void SendFeccTokenReleaseIndication() const;
	virtual void SendFeccKeyRequestIndication(const char* pszDtmf) const;
	virtual void SendH239TokenRequestIndication();
	virtual void SendH239TokenReleaseIndication();
	virtual void SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout);

		// action functions
			// dial-in
	void	OnCsCallInviteResponseReq( CMplMcmsProtocol* pMplProtocol );
			// dial-out
	void	OnCsCallInviteReq( CMplMcmsProtocol* pMplProtocol );
	void	OnCsCallReInviteReq( CMplMcmsProtocol* pMplProtocol );
	void	OnCsCallInviteAckReq( CMplMcmsProtocol* pMplProtocol );
			// disconnect
	void	OnCsCallByeReq( CMplMcmsProtocol* pMplProtocol );
	void	OnCsCallByeOkReq( CMplMcmsProtocol* pMplProtocol );

	void    SendInfoFlowControlToParty(APIU32 videoType,APIU32 mediaDirection ,APIU32  rate);
		// indications to party
	void	SendCsInviteInd();
	void	SendCsInviteResponseInd( CMplMcmsProtocol* pMplProtocol, const BOOL isReinvite );
	void	SendCsReInviteResponseInd( CMplMcmsProtocol* pMplProtocol );
	void	SendCsInviteAckInd( CMplMcmsProtocol* pMplProtocol ) const;
	void	SendCsByeInd( CMplMcmsProtocol* pMplProtocol ) const;
	void	SendCsByeAckInd( CMplMcmsProtocol* pMplProtocol ) const;
	void    SendCsReInviteIndDynamic();
	void	SendCsReinviteIndDummy() const;
	void	SendCsMuteReinviteInd(WORD wAudioMuteByPort, WORD wVideoMuteByPort,
						char *szAudioMuteByDirection, char *szVideoMuteByDirection,
						char *szAudioMuteByInactive, char *szVideoMuteByInactive);
	void	SendBfcpTransportInd( CMplMcmsProtocol* pMplProtocol ) const;
	void    SendCsReinviteInd( CMplMcmsProtocol* pMplProtocol );

		// utilities
	void  	SipChangeSdpForSimulation(sipSdpAndHeadersSt* sdpAndHeaders);
	void	SipBuildDummySdpAndHeadersForSimulation(sipSdpAndHeadersSt* sdpAndHeaders) const;
	void	SipBuildMutedDummySdpAndHeadersForSimulation(sipSdpAndHeadersSt* sdpAndHeaders,
										WORD wAudioMuteByPort, WORD wVideoMuteByPort,
						char *szAudioMuteByDirection, char *szVideoMuteByDirection,
						char *szAudioMuteByInactive, char *szVideoMuteByInactive) const;
	DWORD	CreateSipHeadersBuffer(BYTE** ppHeadersBuffer,const DWORD rejectStatus) const;
	DWORD	CreateSdpStructBuffer(BYTE** ppCapsAndHeadersBuffer,
				const CCapSet& rCapSet,const CCommonComMode& rCommMode,
				const BOOL isFirstCap,
				const DWORD rejectStatus, const BOOL isMuteValues, WORD wAudioMuteByPort = 0, WORD wVideoMuteByPort = 0,
				char *szAudioMuteByDirection = 0, char *szVideoMuteByDirection = 0,
				char *szAudioMuteByInactive = 0, char *szVideoMuteByInactive = 0) const;
	BOOL    NeedReinviteForBfcp() const;

	const char* GetUserAgent() const { return m_UserAgent; };
protected:
	
	
	
	mcTransportAddress	m_H225RemoteIpAddress;
	
	WORD	m_apiArray[SIM_SIP_MAX_API_COMMANDS];
	char			m_UserAgent[IP_STRING_LEN];
	BOOL    m_isTIPEP;
	BOOL    m_isMRE;
	BOOL    m_isMRC;
		// remote
//	char	m_IpH225RemoteStr[16];
//  DWORD	m_IpH225Remote;




  	PDECLAR_MESSAGE_MAP
};

#endif // __ENDPOINTHSIP_H__





