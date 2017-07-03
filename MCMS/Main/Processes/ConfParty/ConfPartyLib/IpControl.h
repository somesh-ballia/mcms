//+========================================================================+
//                            IpControl.h                                  |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpControl.h                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 27/02/13   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __IP_CONTROL__
#define __IP_CONTROL__

#include "ArtDefinitions.h"

#define OPEN_CHANNEL_NORMAL_MODE 1
#define OPEN_CHANNEL_MIX_MODE 2
#define OPEN_CHANNEL_VSW_RELAY 3
#define AUDIO_AVC_TO_SVC_TRANSLATOR_INDEX (0)
#define VIDEO_AVC_TO_SVC_TRANSLATOR_INDEX (1)

typedef enum{
		STATE_OFF=0,
		STATE_CONNECTING,
        STATE_DISCONNECTING,
		STATE_ON
}STATE_T;

typedef struct{
	CHardwareInterface* pAvcToSvcTranslatorInterface;
	STATE_T state; // on-off
	DWORD seqNum;
	//DWORD connId;
}InternalTranslatorInterface;


class CIpCntl : public CStateMachine
{
CLASS_TYPE_1(CIpCntl, CStateMachine)

public:

	CIpCntl(CTaskApp * pOwnerTask);
	virtual const char* NameOf() const { return "CIpCntl";}
	virtual ~CIpCntl();

	virtual void* GetMessageMap() {return (void*)m_msgEntries;}
//	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
//	virtual BOOL  DispatchEvent(OPCODE event,CSegment* pParam);

	virtual void RemoveFromRsrcTbl() const;
	virtual void SetInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams = NULL);
	virtual void AddToInternalRoutingTable();
	virtual void SetNewConfRsrcId(DWORD confRsrcId);
	virtual void AddToRoutingTable();
	virtual void SetControllerResource(CRsrcParams* MfaRsrcParams, CRsrcParams* CsRsrcParams, UdpAddresses sUdpAddressesParams);
	//UdpAddresses GetUdpAddressParams() {return m_UdpAddressesParams;};
	virtual void SetTipRoomId(DWORD tipRoomId);

	virtual CHardwareInterface* GetHWInterface(DWORD opcode,BOOL isInternal,BYTE index=0);
//    virtual bool GetMfaAckOnArtStatus(CSegment* pParam);
    virtual bool AreAllInternalArtsConnected();
    virtual int  OpenInternalArts(ENetworkType networkType,int cnt=2);
    virtual bool GetArtInterfaceByConnId(ConnectionID ConnId, InternalTranslatorInterface*& curTranslatorInterface);
    virtual bool IsAtLeastOneInternalTranslatorArtConnected();
    virtual void CloseTranslatorArts(int numberOfArtsToClose = NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS);
    virtual void CloseInternalArtByConnId(ConnectionID ConnId);
    virtual bool AreAllInternalArtsDisconnected();
    virtual bool IsAtLeastOneInternalArtDisconnecting();

    //    virtual BYTE SipUpgradeAvcChannelReq(CSipComMode* pTargetMode,CSipComMode* pCurrentMode);
    virtual bool IsInternalArtConnId(ConnectionID ConnId);
    virtual int GetNumberOfActiveInternalArts();
    virtual DWORD GetAvcToSvcArtConnectionId(int aIndex);

    void UpdateDownInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams);
    void UpdateUpInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams);
    void UpdateInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams);
    void RemoveArtByConnId(ConnectionID ConnId);
    virtual bool ShouldUpdateMrmpPhysicalIdInfo();

protected:

	virtual CPartyApi*	GetPartyApi() = 0;
	virtual CParty*		GetParty() = 0;
	virtual DWORD 		SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode, BOOL isInternal=FALSE, BYTE index=0);
	virtual DWORD       ShouldInitTimerForSendMsg(DWORD opcode) = 0;
	virtual void        ReduceReqCounter(ACK_IND_S* pAckIndStruct);
    virtual void        InitAllChannelsSeqNum() = 0;

	virtual eConfMediaType GetTargetConfMediaType() = 0;
	virtual BYTE GetIsMrcCall() const {return FALSE;}

	virtual void FillMrmpOpenChannelPhysicalIdInfo(MrmpOpenChannelRequestStruct* pStruct, kChanneltype channelType, cmCapDirection channelDirection, bool isHdVswInMixMode);

    PDECLAR_MESSAGE_MAP

	CCsInterface*					m_pCsInterface;
	CRsrcParams*					m_pCsRsrcDesc;

	CHardwareInterface*				m_pMfaInterface;
	CRsrcParams*					m_pMfaRsrcDesc;

    InternalTranslatorInterface		m_AvcToSvcTranslatorInterface[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	CRsrcParams*                    m_pAvcToSvcTranslatorRsrcDesc[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];

    CHardwareInterface*				m_pMrmpInterface;
    CRsrcParams*                    m_pMrmpRsrcDesc;

	UdpAddresses					m_UdpAddressesParams;
	WORD							m_RoomId;
	DWORD 							m_MfaReqCounter;  //Mfa_Ack counter
	std::list <DWORD>               m_MfaReqIds;

	ConnectionID m_avcToSvcVideoEncoder1ConnId;
    ConnectionID m_avcToSvcVideoEncoder2ConnId;
};


#endif
