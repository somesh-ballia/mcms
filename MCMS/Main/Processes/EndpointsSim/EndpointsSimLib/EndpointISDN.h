//+========================================================================+
//                     EndpointISDN.h							           |
//				Copyright 2007 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointISDN.h											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yair                                                        |
//+========================================================================+

#ifndef __ENDPOINTISDN_H__
#define __ENDPOINTISDN_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "Endpoint.h"

#include "Q931Structs.h"
#include "Bonding.h"
#include "OpcodesMcmsMux.h"
#include "muxint.h"


////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//
//class CCapSet;
//class CCapSetsList;
//class CH323Behavior;
//class CH323BehaviorList;
//class CMplMcmsProtocol;
class CIsdnChannel;

////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//

/////// for API Commands following //////// 
#define SIM_ISDN_MIN_API_COMMANDS		0

#define SIM_ISDN_SETUP_IND_SENT			1
#define SIM_ISDN_PROGRESS_IND_SENT		2
#define SIM_ISDN_ALERT_IND_SENT			3
#define SIM_ISDN_CONNECT_IND_SENT		4
#define SIM_ISDN_DISCONNECT_IND_SENT	5
#define SIM_ISDN_DISCONNECT_ACK_IND_SENT 6
#define SIM_ISDN_CLEAR_IND_SENT			7
#define SIM_ISDN_SETUP_REQ_RECEIVED		8
#define SIM_ISDN_ALERT_REQ_RECEIVED		9
#define SIM_ISDN_CONNECT_REQ_RECEIVED	10
#define SIM_ISDN_CLEAR_REQ_RECEIVED		11
#define SIM_ISDN_DISCONNECT_ACK_REQ_RECEIVED	12
#define SIM_ISDN_BONDING_INIT_REQ_RECEIVED		13
#define SIM_ISDN_BONDING_END_NEGOTIATION_IND_SENT		14

#define SIM_ISDN_MAX_API_COMMANDS		20


///////////////////////////////////////////

//#define MAX_SIM_ALIAS_NAME_SIZE			30




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
struct H230_ENTRY  {
   BYTE        opcode;
   AFUNC       actFunc;
   char*       opcodeStr;
   char*       descStr;
};

#define  ONH230(opcode, actFunc, opcodestr, descstr) { opcode, FUNC actFunc, opcodestr, descstr } ,
#define  BEGIN_H230_TBL  H230_ENTRY CEndpointISDN::m_H230Entries[] = {
#define  END__H230_TBL { 0, 0, "UNKNOWN", "UNKNOWN" } }; 

class CMuxEpParams 
{
public:
		// constructors
	CMuxEpParams();
	~CMuxEpParams();
	
	void SaveMuxEpParams( DWORD connId, WORD boardId, WORD unitId, WORD portId );
	
	DWORD GetMuxConnectionId(){ return m_muxConnectionId; }
   
public:
	DWORD	m_muxConnectionId;
	DWORD	m_muxBoardId;
	DWORD	m_muxUnitId;
	DWORD	m_muxPortId;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CEndpointISDN : public CEndpoint
{
	CLASS_TYPE_1(CEndpointISDN, CEndpoint)

public:
	// constructors, distructors and operators
	CEndpointISDN( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav );
	CEndpointISDN(const CEndpointISDN& other);
	virtual ~CEndpointISDN();
	CEndpointISDN& operator =(const CEndpointISDN& other);

    virtual const char*  NameOf() const { return "CEndpointISDN"; } 

		// virtual functions
	virtual char* GetIpStr() const { return ""; }
    virtual void GetIpStr(char* str) const { strcpy(str, ""); return; }
    
	virtual void SetIp( const char* pszEpIp );

    virtual void SetIpVersion(DWORD ipVer) {};
	virtual DWORD GetIpVersion() const { return 2;};

    virtual void SetTransportAddress( mcTransportAddress* mcTA ) {};
	virtual mcTransportAddress* GetTransportAddress() { return NULL;}

    virtual DWORD GetIpDword() { return 0; }
	virtual void OnGuiConnectEp();
	virtual void OnGuiDisconnectEp();
	virtual void OnGuiUpdateEp(CSegment* pParam);
	virtual void OnGuiDeleteEp();
	virtual eEndpointsTypes GetEpType() const { return eEndPointIsdn; }
	virtual void Serialize( CSegment& rSegment ) const;
	virtual void SerializeDetails( CSegment& rSegment ) const;
	virtual void DeSerialize(CSegment& rParam);
	virtual void HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol ) {;}
	virtual void HandleIsdnProtocolEvent( const DWORD opcode, BYTE* pData, CSegment* pNetSetupParams=NULL );
	virtual void SendChangeMode() {;}
	virtual BOOL IsH239Call() const { return TRUE; }
	
	virtual DWORD GetCallIndex() const { return m_netConnectionId; } // get Cs CallIndex for IP or net_connection_id for ISDN
	virtual void SetCallIndex(const DWORD nNetConId); // set Cs CallIndex for IP or net_connection_id for ISDN

	CMuxEpParams GetMuxEpParams(){ return m_muxParams; }
	
	BOOL IsMyRtmSpanPort(const DWORD spanId,const DWORD portId );
	virtual BOOL SetNetBoardDetails(const WORD boardId, const WORD subBoardId, const WORD spanId, const WORD portId, const DWORD netConnectionId=(DWORD)-1);
	void CleanNetBoardDetails()
			{ return m_rRtmBoard.CleanDetails(); }
	void SetPhoneNum( const char* pszPhoneNum );
	void SetPhoneNum( const char* pszPhoneNum , int channelIndex);

		// basic functions
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	
	void  OnBondingInitReq(BND_CONNECTION_INIT_REQUEST_S& tBondingInitReq);
	
	void SetDialInChannelsNumber( APIS8	numOfBondingChannels) { m_numOfBondingChannels = numOfBondingChannels; }

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
	void  OnNetSetupReq(NET_SETUP_REQ_S& tSetupReq, CSegment* pNetSetupParams);
//	void  OnBondingInitReq(BND_CONNECTION_INIT_REQUEST_S& tBondingInitReq);
			// disconnect
	void  OnNetClearReq(NET_CLEAR_REQ_S& tClearReq, CSegment* pNetSetupParams);
	void  OnNetDisconnectAckReq(NET_DISCONNECT_ACK_REQ_S& tDiscoAckReq);
	
	void  OnSetEcsReq(SET_ECS_S& tSetEcs);
	
		// indications to party
	void  SendSetupInd(int channelIndex=0);
	void  SendProgressInd();
	void  SendAlertInd(NET_SETUP_REQ_S& tSetupReq, CSegment* pNetSetupParams);
	void  SendConnectInd(CSegment* pNetSetupParams);
	void  SendDisconnectInd();
	void  SendDisconnectAckInd();
	void  SendClearInd(DWORD chanConnectionId/*CSegment* pNetSetupParams*/);
	
	void SendBondingEndNegotiationInd(BND_CONNECTION_INIT_REQUEST_S& tBondingInitReq);
	void SendRemoteEcsInd(BYTE* pRemoteEcsInd, DWORD msgDataLen);	

		// utilities
	void  FillCommonNetHeader(NET_COMMON_PARAM_S& rNetHeader) const;
	void  PrepareNetMessageSeg(CSegment& rMsgSeg, const DWORD opcode, BYTE* pDataBytes, const DWORD dwDataSize, const DWORD chanConnectionId=(DWORD)(-1) /*CSegment* pNetSetupParams=NULL*/) /*const*/;
	
	void  PrepareMuxMessageSeg(CSegment& rMsgSeg, const DWORD opcode, BYTE* pDataBytes, const DWORD dwDataSize) const;
	
	void PreparePhoneNumbers();	
	void SetPhoneNumbers(const BYTE* phoneNumber);
	
	void SetEpConnectionID( const DWORD connID, const DWORD netConnectionId=(DWORD)-1);
	void PrepareH230(CSegment& h230String, CSegment& StringToSend) const;
	void SendH230(CSegment* pParam);

	void  OnMuxH230(CSegment* pParam);
	void  OnMuxH230Mbe(CSegment* pParam);
	void  OnMuxH239Message(BYTE msgLen,CSegment* pParam);
	void  OnRoleTokenOwnerTout(CSegment* pParam);
	void  SendH239TokenWithdrawResponse();
	void  SendH239Message(BYTE SubMessageOpcode, BYTE isAckNack = 0,BYTE addRandNum = 0, BYTE randNum = RAND_NUM_FOR_EP_SIM);
	
	void  OnH230AudioMute(CSegment* pParam);
	void  OnH230AudioActive(CSegment* pParam);
	void  OnH230VideoIndSuppressed(CSegment* pParam);
	void  OnH230VideoActive(CSegment* pParam);
	void  OnH230AssignTerminaNum(CSegment* pParam);
	void  OnH230TerminalIdentity(CSegment* pParam);
	void  GetMcuTerminalNum(CSegment* pParam,BYTE& mcu,BYTE& terminal);
private:
	void SetNetConnectionIDs();
	int FindChannelIndex(DWORD netConnId) const;
	
public:
	CMuxEpParams	m_muxParams;

protected:
	WORD				m_apiArray[SIM_ISDN_MAX_API_COMMANDS];
	CRtmBoardDetails	m_rRtmBoard;
	DWORD				m_netConnectionId;
		
	APIS8				m_numOfBondingChannels;
	int					m_connectedChannelsCounter;
	
	// remote
	//char	m_szPhoneNum[PRI_LIMIT_PHONE_DIGITS_LEN+1];
	
///	char	m_szPhoneNum[MAX_ADDITIONAL_PHONE_NUM][BND_MAX_PHONE_LEN+1];
	
	CIsdnChannel*	m_channelArr[MAX_ADDITIONAL_PHONE_NUM];
		
	char	m_szCliPhoneNum[PRI_LIMIT_PHONE_DIGITS_LEN+1];  // MCU conf number in Dial-out  OR
															// Endpoint number in Dial-in
	static H230_ENTRY  m_H230Entries[];												   	
	
  	PDECLAR_MESSAGE_MAP
};




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CIsdnChannel : public CPObject
{
	CLASS_TYPE_1(CIsdnChannel,CPObject)

public:
		// constructors
	CIsdnChannel();
	virtual ~CIsdnChannel();

		// overrides
    virtual const char*  NameOf() const { return "CIsdnChannel"; } 
    
    void Serialize( CSegment& rSegment ) const;
	void SetPhoneNum( const char* pszPhoneNum );
	char* GetPhoneNum(){ return m_szPhoneNum; }
	
	void SetPhoneNumDigit( const char* pszPhoneNumDigit );
	char* GetPhoneNumDigit(){ return m_szPhoneNumDigit; }
	
	void SetParams(DWORD connectionId, DWORD boardId, DWORD subBoardId, DWORD spanId, DWORD portId);
	
	void SetConnectionId(const DWORD connectionId);
	DWORD GetConnectionId() const;	
	
	void SetState(const enEndpointState state);
	enEndpointState GetState() const;
	
	
	void SetNetConnectionId(const DWORD netConnectionId);
	DWORD GetNetConnectionId() const;
	
	void SetBoardDetails(const WORD boardId, const WORD subBoardId, const WORD spanId, const WORD portId);
	CRtmBoardDetails GetRtmBoardDetails() const;
	
    
protected:
	char				m_szPhoneNum[BND_MAX_PHONE_LEN+1];
	char				m_szPhoneNumDigit[BND_MAX_PHONE_LEN+1];
	enEndpointState		m_connectionState;
	CRtmBoardDetails	m_rRtmBoardDetails;
	
	DWORD				m_connectionId;
	
	DWORD				m_netConnectionId;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#endif // __ENDPOINTISDN_H__


