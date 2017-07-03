//+========================================================================+
//                     EndpointPstn.h							           |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointPstn.h											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//+========================================================================+

#ifndef __ENDPOINTPSTN_H__
#define __ENDPOINTPSTN_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "Q931Structs.h"
#include "Endpoint.h"


////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//
//class CCapSet;
//class CCapSetsList;
//class CH323Behavior;
//class CH323BehaviorList;
//class CMplMcmsProtocol;


////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//

/////// for API Commands following //////// 
#define SIM_PSTN_MIN_API_COMMANDS		0
////----
#define SIM_PSTN_SETUP_IND_SENT			1
#define SIM_PSTN_PROGRESS_IND_SENT		2
#define SIM_PSTN_ALERT_IND_SENT			3
#define SIM_PSTN_CONNECT_IND_SENT		4
#define SIM_PSTN_DISCONNECT_IND_SENT	5
#define SIM_PSTN_DISCONNECT_ACK_IND_SENT 6
#define SIM_PSTN_CLEAR_IND_SENT			7
#define SIM_PSTN_SETUP_REQ_RECEIVED		8
#define SIM_PSTN_ALERT_REQ_RECEIVED		9
#define SIM_PSTN_CONNECT_REQ_RECEIVED	10
#define SIM_PSTN_CLEAR_REQ_RECEIVED		11
#define SIM_PSTN_DISCONNECT_ACK_REQ_RECEIVED	12
//// add here...
////----
#define SIM_PSTN_MAX_API_COMMANDS		20

///////////////////////////////////////////

//#define MAX_SIM_ALIAS_NAME_SIZE			30



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEndpointPstn : public CEndpoint
{
	CLASS_TYPE_1(CEndpointPstn,CEndpoint)

public:
	// constructors, distructors and operators
	CEndpointPstn( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav );
	virtual const char* NameOf() const { return "CEndpointPstn";}
	CEndpointPstn(const CEndpointPstn& other);
	virtual ~CEndpointPstn();
	CEndpointPstn& operator =(const CEndpointPstn& other);


		// virtual functions
	virtual char* GetIpStr() const { return ""; }
	virtual void GetIpStr(char* str) const { strcpy(str, ""); return; }
	
	virtual void SetIp( const char* pszEpIp );
	virtual DWORD GetIpDword() { return 0; }
	
	virtual void SetIpVersion(DWORD ipVer) {};
	virtual DWORD GetIpVersion() const { return 2;};
	
	virtual void SetTransportAddress( mcTransportAddress* mcTA ) {};
	virtual mcTransportAddress* GetTransportAddress() { return NULL;}
	
	virtual void OnGuiConnectEp();
	virtual void OnGuiDisconnectEp();
	virtual void OnGuiUpdateEp(CSegment* pParam);
	virtual void OnGuiDeleteEp();
	virtual eEndpointsTypes GetEpType() const { return eEndPointPstn; }
	virtual void Serialize( CSegment& rSegment ) const;
	virtual void SerializeDetails( CSegment& rSegment ) const;
	virtual void DeSerialize(CSegment& rParam);
	virtual void HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol ) {;}
	virtual void HandleIsdnProtocolEvent( const DWORD opcode, BYTE* pData, CSegment* pNetSetupParams=NULL );
	virtual void SendChangeMode() {;}

	virtual DWORD GetCallIndex() const { return m_netConnectionId; } // get Cs CallIndex for IP or net_connection_id for ISDN
	virtual void SetCallIndex(const DWORD nNetConId); // set Cs CallIndex for IP or net_connection_id for ISDN

	BOOL IsMyRtmSpanPort(const DWORD spanId,const DWORD portId );
	virtual BOOL SetNetBoardDetails(const WORD boardId, const WORD subBoardId, const WORD spanId, const WORD portId, const DWORD netConnectionId=(DWORD)-1)
			{ return m_rRtmBoard.SetBoardDetails(boardId, subBoardId, spanId, portId); }
	void CleanNetBoardDetails()
			{ return m_rRtmBoard.CleanDetails(); }
	void SetPhoneNum( const char* pszPhoneNum );

		// basic functions
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

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
			// dial-in
	void  OnNetAlertReq(NET_ALERT_REQ_S& tAlertReq);
//	void  OnNetProgressReq(NET_ SETUP_REQ_S& tSetupReq);
	void  OnNetConnectReq(NET_CONNECT_REQ_S& tConnectReq);
			// dial-out
	void  OnNetSetupReq(NET_SETUP_REQ_S& tSetupReq);
			// disconnect
	void  OnNetClearReq(NET_CLEAR_REQ_S& tClearReq);
	void  OnNetDisconnectAckReq(NET_DISCONNECT_ACK_REQ_S& tDiscoAckReq);

		// indications to party
	void  SendSetupInd();
	void  SendProgressInd();
	void  SendAlertInd();
	void  SendConnectInd();
	void  SendDisconnectInd();
	void  SendDisconnectAckInd();
	void  SendClearInd();

		// utilities
//	void  	SipChangeSdpForSimulation(sipSdpAndHeadersSt* sdpAndHeaders);
	void  FillCommonNetHeader(NET_COMMON_PARAM_S& rNetHeader) const;
	void  PrepareNetMessageSeg(CSegment& rMsgSeg,const DWORD opcode,BYTE* pDataBytes,const DWORD dwDataSize) const;

protected:
	WORD				m_apiArray[SIM_PSTN_MAX_API_COMMANDS];
	CRtmBoardDetails	m_rRtmBoard;
	DWORD				m_netConnectionId;
		// remote
	char	m_szPhoneNum[PRI_LIMIT_PHONE_DIGITS_LEN+1];
	char	m_szCliPhoneNum[PRI_LIMIT_PHONE_DIGITS_LEN+1];
//	DWORD	m_IpH225Remote;
	
  	PDECLAR_MESSAGE_MAP
};

#endif // __ENDPOINTPSTN_H__ 





