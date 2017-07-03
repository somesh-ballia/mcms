//+========================================================================+
//                            IpControl.cpp                                |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpControl.cpp                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 27/02/13   | This file contains								   |
//     |            |                                                      |
//+========================================================================+
#include <arpa/inet.h>
#include <stdlib.h>


#include "Segment.h"
#include "StateMachine.h"
#include "PartyApi.h"
#include "ProcessBase.h"
#include "ConfPartyProcess.h"

#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "IpCsOpcodes.h"
#include "Conf.h"
#include "IpAddressDefinitions.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyGlobals.h"
#include "StatusesGeneral.h"
#include "SIPCommon.h"
#include "SipScm.h"
#include "SipCaps.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "Party.h"
#include "IPParty.h"
#include "IpMfaOpcodes.h"
#include "CsInterface.h"
#include "SIPParty.h"
#include "SIPControl.h"
#include "SipUtils.h"
#include "H264Util.h"
#include "IpRtpInd.h"
#include "IpCmInd.h"
#include "ConfPartyGlobals.h"
#include "IpServiceListManager.h"
#include "IpCommonUtilTrace.h"
#include "SystemFunctions.h"
#include "HostCommonDefinitions.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "IpCmReq.h"
#include "IpAddress.h"
#include "ManagerApi.h"
#include "WrappersResource.h"  //to be removed noa
#include "OpcodesMcmsCardMngrICE.h"
#include "IVRCntl.h"
#include "IpCmReq.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsAudio.h"
#include "BFCPH239Translator.h"
#include "RtvVideoMode.h"
#include "OpcodesMcmsCardMngrTIP.h"
#include "TipStructs.h"
#include "TipUtils.h"
#include "OpcodesRanges.h"
#include "BfcpStructs.h"
#include "SipProxyTaskApi.h"
#include "ScpNotificationWrapper.h"
#include "IpService.h"
#include "MrcStructs.h"
#include "ScpPipeMappingNotification.h"

#include "PrecedenceSettings.h"
#include "AudRequestStructs.h"
//CDR_MCCF:
#include "IpChannelDetails.h"
#include "PartyMonitor.h"
#include "MccfCdrPackageResponse.h"

#include "OpcodesMcmsCardMngrTB.h"

#include "ArtRequestStructs.h"

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

PBEGIN_MESSAGE_MAP(CIpCntl)


PEND_MESSAGE_MAP(CIpCntl,CStateMachine);


//////////////////////////////////////////////////////////////////////
CIpCntl::CIpCntl(CTaskApp *pOwnerTask)
        :CStateMachine(pOwnerTask)
{
	m_pCsRsrcDesc = NULL;
	m_pCsInterface = new CCsInterface;

	m_pMfaRsrcDesc = NULL;
	m_pMfaInterface = new CHardwareInterface;

	m_pMrmpRsrcDesc = NULL;
    m_pMrmpInterface = new CHardwareInterface;

	for(int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
    {
//    	m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface = new CHardwareInterface;
		m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface = NULL;
    	m_AvcToSvcTranslatorInterface[i].state=STATE_OFF;
        m_pAvcToSvcTranslatorRsrcDesc[i] = NULL;
    }

    m_MfaReqIds.clear();
    m_MfaReqCounter = 0;

    m_avcToSvcVideoEncoder1ConnId = DUMMY_CONNECTION_ID;
    m_avcToSvcVideoEncoder2ConnId = DUMMY_CONNECTION_ID;

	VALIDATEMESSAGEMAP;
}


////////////////////////////////////////////
CIpCntl::~CIpCntl()
{
	POBJDELETE(m_pCsRsrcDesc);
	POBJDELETE(m_pCsInterface);

    POBJDELETE(m_pMfaRsrcDesc);
	POBJDELETE(m_pMfaInterface);

	POBJDELETE(m_pMrmpRsrcDesc);
	POBJDELETE(m_pMrmpInterface);

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
    	POBJDELETE(m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface);
    	POBJDELETE(m_pAvcToSvcTranslatorRsrcDesc[i]);
    }

    m_MfaReqIds.clear();

}

//////////////////////////////////////////////////////////////////////
void CIpCntl::SetControllerResource(CRsrcParams* MfaRsrcParams, CRsrcParams* CsRsrcParams, UdpAddresses sUdpAddressesParams)
{
	POBJDELETE(m_pMfaRsrcDesc);
	POBJDELETE(m_pCsRsrcDesc);
	memcpy(&m_UdpAddressesParams, &sUdpAddressesParams, sizeof(UdpAddresses));

	m_pMfaRsrcDesc = new CRsrcParams(*MfaRsrcParams);
	m_pCsRsrcDesc = new CRsrcParams(*CsRsrcParams);

	m_pCsRsrcDesc->SetLogicalRsrcType(eLogical_ip_signaling);
	m_pCsInterface->Create(m_pCsRsrcDesc);

	m_pMfaInterface->Create(m_pMfaRsrcDesc);
	m_pMfaInterface->SetRoomId( m_RoomId );
}


//////////////////////////////////////////////////////////////////////////////////
void CIpCntl::SetInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams)
{
	if (MrmpRsrcParams)
	{
		TRACEINTO << "MrmpRsrcParams OK";
		if(!m_pMrmpRsrcDesc)
		{
			m_pMrmpRsrcDesc = new CRsrcParams(*MrmpRsrcParams);
			m_pMrmpInterface->Create(m_pMrmpRsrcDesc);
			m_pMrmpInterface->SetRoomId(m_RoomId);
		}
		else
		{
			TRACEINTO<< "mix_mode: m_pMrmpRsrcDesc already exists";
		}
	}
	else
	{
		TRACEINTO << "mix_mode: MrmpRsrcParams = NULL. It will not be updated.";
	}

	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		POBJDELETE(m_pAvcToSvcTranslatorRsrcDesc[i]);
		POBJDELETE(m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface);
	}

	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		if (avcToSvcTranslatorRsrcParams && avcToSvcTranslatorRsrcParams[i])
		{
			m_pAvcToSvcTranslatorRsrcDesc[i] = new CRsrcParams(*avcToSvcTranslatorRsrcParams[i]);
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface = new CHardwareInterface;
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->Create(m_pAvcToSvcTranslatorRsrcDesc[i]);
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->SetRoomId( m_RoomId );
			TRACEINTO<<"mix_mode:  avcToSvcTranslatorRsrcParams["<<i<<"] OK m_RoomId=" << m_RoomId;
		}
		else
		{
			TRACEINTO<<"mix_mode: avcToSvcTranslatorRsrcParams["<<i<<"] NOT OK";
		}
	}
}

//////////////////////////////////////////////////////////
void CIpCntl::RemoveFromRsrcTbl() const
{
	PTRACE(eLevelInfoNormal, "CIpCntl::RemoveFromRsrcTbl");
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if (pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	if (m_pCsRsrcDesc != NULL)
	{
		if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pCsRsrcDesc))
		{
			TRACEINTO << "pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pCsRsrcDesc)) FAILS";
			DBGPASSERT(105);
		}
	}
    if (m_pMfaRsrcDesc != NULL)
    {
        if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pMfaRsrcDesc))
        {
        	TRACEINTO << "pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pMfaRsrcDesc)) FAILS";
            DBGPASSERT(105);
        }
    }
	if(m_pMrmpRsrcDesc != NULL)
	{
		if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pMrmpRsrcDesc))
		{
			TRACEINTO << "pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pMrmpRsrcDesc)) FAILS";
			DBGPASSERT(105);
		}
	}

	for(int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
    {
		if (m_pAvcToSvcTranslatorRsrcDesc[i] != NULL)
		{
			if (STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pAvcToSvcTranslatorRsrcDesc[i]))
			{
				TRACEINTO << "pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pAvcToSvcTranslatorRsrcDesc["<<(i)<<"]) FAILS";
				DBGPASSERT(105);
			}
		}
    }
}

//////////////////////////////////////////////////////////////////////////////////
void CIpCntl::AddToInternalRoutingTable()
{
	WORD status;
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if (GetPartyApi() && CPObject::IsValidPObjectPtr(GetPartyApi()))
	{
		if (!m_pMrmpRsrcDesc)
		{
			TRACEINTO << "m_pMrmpRsrcDesc is NULL";
		}
		else
		{
			status = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pMrmpRsrcDesc, GetPartyApi());
			if (status != STATUS_OK)
			{
				TRACEINTO << "mix_mode: FAILED pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pMrmpRsrcDesc,GetPartyApi())";
			   	DBGPASSERT(status);
				//SystemCoreDump(FALSE);
			}
		}

		for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
			if (m_pAvcToSvcTranslatorRsrcDesc[i])
			{
				status=pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pAvcToSvcTranslatorRsrcDesc[i], GetPartyApi());

				if (status != STATUS_OK)
				{
					TRACEINTO<<"mix_mode: FAILED pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pAvcToSvcTranslatorRsrcDesc["<<i<<"],GetPartyApi())";
					DBGPASSERT(status);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
void CIpCntl::SetNewConfRsrcId(DWORD confRsrcId)
{
	m_pCsInterface->SetConfRsrcId(confRsrcId);
    m_pMfaInterface->SetConfRsrcId(confRsrcId);
	m_pMrmpInterface->SetConfRsrcId(confRsrcId);
    for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
    	if (m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
    		m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->SetConfRsrcId(confRsrcId);
    }
}

///////////////////////////////////////////////////////////////////////////////////
void CIpCntl::AddToRoutingTable()
{
	WORD status;
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL)
	{
	    PASSERTMSG(101,"CIpCntl::AddToRoutingTable - pRoutingTbl not valid");
		PASSERT_AND_RETURN(101);
	}

	if (GetPartyApi() && CPObject::IsValidPObjectPtr(GetPartyApi()))
	{
		if (m_pCsRsrcDesc)
		{
			status=pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsRsrcDesc,GetPartyApi());
			if (status != STATUS_OK)
			{
				TRACEINTO << "FAILED pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsRsrcDesc,GetPartyApi())";
				DBGPASSERT(status);
			}
		}
        if (m_pMfaRsrcDesc)
        {
            status=pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pMfaRsrcDesc,GetPartyApi());
			if (status != STATUS_OK)
			{
				TRACEINTO << "FAILED pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pMfaRsrcDesc,GetPartyApi())";
				DBGPASSERT(status);
			}
        }

        AddToInternalRoutingTable();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIpCntl::SetTipRoomId(DWORD tipRoomId)
{
	m_RoomId = tipRoomId;
	if( m_pMrmpInterface )
		m_pMrmpInterface->SetRoomId( tipRoomId );
    if( m_pMfaInterface )
        m_pMfaInterface->SetRoomId( tipRoomId );

    for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
		if(m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->SetRoomId( tipRoomId );
        TRACEINTO<<"mix_mode:  avcToSvcTranslatorRsrcParams["<<i<<"] OK m_RoomId=" << m_RoomId;
    }
}

///////////////////////////////////////////////////////////////////////////////////
CHardwareInterface* CIpCntl::GetHWInterface(DWORD opcode,BOOL isInternal,BYTE index)
{
	if (IsMrmpOpcode(opcode))
	{
		if (!m_pMrmpInterface)
		{
	        TRACEINTO << "m_pMrmpInterface Is Null";
	        DBGPASSERT(opcode);
		}

 		return m_pMrmpInterface;
	}

	if (isInternal == TRUE)
	{
		if (!m_AvcToSvcTranslatorInterface[index].pAvcToSvcTranslatorInterface )
		{
	        TRACEINTO << " m_AvcToSvcTranslatorInterface[" << (int)index << "].pAvcToSvcTranslatorInterface Is Null";
	        DBGPASSERT(opcode);
	        return NULL;
		}

		return m_AvcToSvcTranslatorInterface[index].pAvcToSvcTranslatorInterface;
	}

	return m_pMfaInterface;
}

///////////////////////////////////////////////////////////////////////////////////
int CIpCntl::OpenInternalArts(ENetworkType networkType, int avcToSvcTranslatorCnt)
{
	DWORD maxVideoTxBitsPer10ms = 64000;

	TOpenArtReq* pOpenArtReq = new TOpenArtReq;
	memset(pOpenArtReq, 0, sizeof(TOpenArtReq));
	pOpenArtReq->enNetworkType = networkType;
	pOpenArtReq->unVideoTxMaxNumberOfBitsPer10ms = maxVideoTxBitsPer10ms;
	pOpenArtReq->ConnectContent = GetIsMrcCall();
	DWORD seqNum = 0;
	pOpenArtReq->nMediaMode = eMediaModeTranscoding;

	DWORD confId = GetParty()->GetConfId();
	DWORD partyId = GetParty()->GetPartyRsrcID();
	TRACEINTO << "mix_mode: dynMixedPosAck request to open internal arts";

	int numOfArtsToOpen = 0;
	for (int index=0; index<avcToSvcTranslatorCnt; ++index)
    {
		if (!m_AvcToSvcTranslatorInterface[index].pAvcToSvcTranslatorInterface || m_AvcToSvcTranslatorInterface[index].state!=STATE_OFF)
			continue;

		m_AvcToSvcTranslatorInterface[index].state=STATE_CONNECTING;
		seqNum = SendMsgToMpl((BYTE*)(pOpenArtReq), sizeof(TOpenArtReq), TB_MSG_OPEN_PORT_REQ, TRUE, index);
		m_AvcToSvcTranslatorInterface[index].seqNum = seqNum;
		numOfArtsToOpen++;
		TRACEINTO << "mix_mode: m_AvcToSvcTranslatorInterface["<<index<<"].seqNum ="<<m_AvcToSvcTranslatorInterface[index].seqNum;
    }
	
	PDELETE(pOpenArtReq);

	return numOfArtsToOpen;
}

///////////////////////////////////////////////////////////////////////////////////
bool CIpCntl::IsInternalArtConnId(ConnectionID ConnId)
{
	bool found=false;

	for (int i=0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && !found; ++i)
	{
		if (!m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
			continue;

		TRACEINTO << "mix_mode: m_AvcToSvcTranslatorInterface[" << i << "].seqNum:" << m_AvcToSvcTranslatorInterface[i].seqNum;

		if (ConnId == m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId())
		{
			found=true;
		}
	}

	return found;
}

///////////////////////////////////////////////////////////////////////////////////
//int CIpCntl::GetNumberOfExistingInternalArts()
//{
//	int num=0;
//
//	for (int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS ;++i)
//	{
//		if (m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
//		{
//			num++;
//		}
//	}
//	return num;
//}


///////////////////////////////////////////////////////////////////////////////////
int CIpCntl::GetNumberOfActiveInternalArts()
{
	int num=0;

	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS ;++i)
	{
		if (m_AvcToSvcTranslatorInterface[i].state == STATE_ON)
		{
			num++;
		}
	}
	
	return num;
}

///////////////////////////////////////////////////////////////////////////////////
bool CIpCntl::GetArtInterfaceByConnId(ConnectionID ConnId, InternalTranslatorInterface*& curTranslatorInterface)
{
	bool found=false;
	curTranslatorInterface = NULL;

	for(int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && !found;++i)
	{
		if (!m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
			continue;

		if (ConnId == m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId())
		{
			found=true;
			curTranslatorInterface = &m_AvcToSvcTranslatorInterface[i];
		}
	}

	if(!found)
	{
		TRACEINTO << "mix_mode: didn't find ART with connectionId:"<<ConnId;
	}
	return found;
}

///////////////////////////////////////////////////////////////////////////////////
bool CIpCntl::IsAtLeastOneInternalTranslatorArtConnected()
{
	bool flag=false;

	// for MRC calls there are no translators
	if (GetIsMrcCall())
		return false;

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && flag==false;++i)
	{
		if(m_AvcToSvcTranslatorInterface[i].state == STATE_ON)
		{
			flag=true;
		}
	}
	return flag;
}

///////////////////////////////////////////////////////////////////////////////////
void CIpCntl::CloseTranslatorArts(int numberOfArtsToClose)
{
	if (!GetParty())
	{
        TRACEINTO << "mix_mode: No party?!?!?!?. numberOfArtsToClose=" << numberOfArtsToClose;
		return;
	}

	DWORD confId = GetParty()->GetConfId();
	DWORD partyId = GetParty()->GetPartyRsrcID();
	DWORD seqNum = 0;

	TRACEINTO << "mix_mode: numberOfArtsToClose=" << numberOfArtsToClose;

	for (int index = NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS-1; index >= NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS-numberOfArtsToClose; --index)
	{
	    TRACEINTO << "mix_mode: Checking ART index=" << index;
		if (m_AvcToSvcTranslatorInterface[index].state == STATE_ON)
		{
		    TRACEINTO << "mix_mode: ART is opened. Close it. index =" << index;
	        m_AvcToSvcTranslatorInterface[index].state=STATE_DISCONNECTING;
			seqNum=SendMsgToMpl(NULL,0,TB_MSG_CLOSE_PORT_REQ,TRUE,index);
			m_AvcToSvcTranslatorInterface[index].seqNum=seqNum;
			TRACEINTO << "dynMixedPosAck: mix_mode: closing art: " << index;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
void CIpCntl::CloseInternalArtByConnId(ConnectionID ConnId)
{
    if (!GetParty())
    {
        TRACEINTO << "mix_mode: No party?!?!?!?. ConnId=" << ConnId;
		
        return;
    }

    bool found=false;
    int index = 0;
	
    for (int i=0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && !found; ++i)
    {
        if (!m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
            continue;

        if (ConnId == m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId())
        {
            found=true;
            index = i;
        }
    }
	
    if (!found)
    {
        TRACEINTO << "mix_mode: No ART! ConnId=" << ConnId;
		
        return;
    }

    DWORD confId = GetParty()->GetConfId();
    DWORD partyId = GetParty()->GetPartyRsrcID();
    DWORD seqNum = 0;

    TRACEINTO << "mix_mode: Checking ART index=" << index;

    if (m_AvcToSvcTranslatorInterface[index].state == STATE_ON)
    {
        TRACEINTO << "mix_mode: ART is opened. Close it. index =" << index;
        m_AvcToSvcTranslatorInterface[index].state=STATE_DISCONNECTING;
        seqNum = SendMsgToMpl(NULL, 0, TB_MSG_CLOSE_PORT_REQ, TRUE, index);
        m_AvcToSvcTranslatorInterface[index].seqNum = seqNum;
    }
    else
    {
        TRACEINTO << "mix_mode: ART is closed. ConnId=" << ConnId;
		
        return;
    }
}


///////////////////////////////////////////////////////////////////////////////////
bool CIpCntl::AreAllInternalArtsDisconnected()
{
	bool flag=false;
	if (!IsAtLeastOneInternalTranslatorArtConnected() && !IsAtLeastOneInternalArtDisconnecting())
	{
		TRACEINTO << "!@# dynMixedPosAck all internal arts are disconnected";
		flag=true;
	}
	else
	{
		TRACEINTO<<"!@# m_MfaReqCounter:" << m_MfaReqCounter;
	}
	return flag;
}

///////////////////////////////////////////////////////////////////////////////////
bool CIpCntl::IsAtLeastOneInternalArtDisconnecting()
{
    bool flag = false;
    for (int index = 0; index < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++index)
    {
        if (m_AvcToSvcTranslatorInterface[index].pAvcToSvcTranslatorInterface && m_AvcToSvcTranslatorInterface[index].state == STATE_DISCONNECTING)
        {
            TRACEINTO << "mix_mode: ART #" << index << " is still disconnecting";
            return true;
        }
    }
    TRACEINTO << "mix_mode: No disconnecting ARTs";
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
bool CIpCntl::AreAllInternalArtsConnected()
{
	bool flag = false;

	for (int index = 0; index < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++index)
	{
		if (m_AvcToSvcTranslatorInterface[index].pAvcToSvcTranslatorInterface && m_AvcToSvcTranslatorInterface[index].state != STATE_ON)
		{
			return false;
		}
	}
	TRACEINTOFUNC<<"!@# dynMixedPosAck all internal arts are connected";
	return true;
}

////////////////////////////////////////////////////////////
DWORD CIpCntl::GetAvcToSvcArtConnectionId(int aIndex)
{
    if (aIndex >= NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS)
    {
        TRACEINTO << "Index is too large aIndex = " << aIndex;
        return INVALID;
    }

    if (!m_AvcToSvcTranslatorInterface[aIndex].pAvcToSvcTranslatorInterface)
    {
        TRACEINTO << "There is no translator ART for aIndex = " << aIndex;
        return INVALID;
    }

    return m_AvcToSvcTranslatorInterface[aIndex].pAvcToSvcTranslatorInterface->GetConnectionId();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpCntl::FillMrmpOpenChannelPhysicalIdInfo(MrmpOpenChannelRequestStruct* pStruct, kChanneltype channelType, cmCapDirection channelDirection, bool isHdVswInMixMode)
{
	//Fill the info for the connection id of the physical info for audio decoder/encoder or video decoder/encoder in case of mix
	DWORD partyId = m_pMrmpInterface->GetPartyRsrcId();
	CRsrcDesc *pRsrcDesc = NULL;
	eLogicalResourceTypes eLrt[MAX_NUMBER_OF_PHYSICAL_RESOURCES];

	// initialize values
    pStruct->physicalId[0].connection_id =INVALID;
    pStruct->physicalId[0].party_id = INVALID;
    pStruct->m_allocatedPhysicalResources = 0;

    for (int i = 0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; i++)
    {
        eLrt[i] = eLogical_res_none;
    }

    // AVC party in mix mode
    if (GetTargetConfMediaType() == eMixAvcSvc && !GetIsMrcCall())
    {

		if (channelType == kIpAudioChnlType)
        {
            // leave audio softMCU flow as before "Mix for RMX" feature
            if(IsSoftMcu())
            {
                if(channelDirection==cmCapReceive)
                    eLrt[0] = eLogical_relay_audio_decoder;
                else if(channelDirection ==cmCapTransmit)
                    eLrt[0] = eLogical_relay_audio_encoder;
                pStruct->m_allocatedPhysicalResources = 1;
            }
            else
            {// direct packets to ART and listen to ART
                if (m_AvcToSvcTranslatorInterface[AUDIO_AVC_TO_SVC_TRANSLATOR_INDEX].pAvcToSvcTranslatorInterface && m_AvcToSvcTranslatorInterface[AUDIO_AVC_TO_SVC_TRANSLATOR_INDEX].state == STATE_ON)
                {
                    eLrt[0] = m_AvcToSvcTranslatorInterface[AUDIO_AVC_TO_SVC_TRANSLATOR_INDEX].pAvcToSvcTranslatorInterface->GetLogicalRsrcType();
                    pStruct->m_allocatedPhysicalResources = 1;
                }
                else
                {
                    TRACEINTO << "mix_mode: no ART for audio AvcToSvc"; // @#@ assert here
                    return;
                }
            }

        }
        else if (channelType == kIpVideoChnlType && channelDirection == cmCapReceive) // video receive
        {
            int i = 0;
            for (i = 0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; i++)
            {
               if (m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface && m_AvcToSvcTranslatorInterface[i].state == STATE_ON)
               {
                   if (IsSoftMcu())
                   {
                       if (i == 0)
                           eLrt[i] = eLogical_relay_avc_to_svc_video_encoder_1;
                       else
                           eLrt[i] = eLogical_relay_avc_to_svc_video_encoder_2;
                   }
                   else
                   {// direct packets to ART and listen to ART
                       eLrt[i] = m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetLogicalRsrcType();
                   }
                   pStruct->m_allocatedPhysicalResources++;
               }
            }

            // add the VSW stream if it exists
            if (isHdVswInMixMode)
            {
            	TRACEINTO << "isHdVswInMixMode=true";
            	eLrt[pStruct->m_allocatedPhysicalResources] = eLogical_rtp;
            	pStruct->m_allocatedPhysicalResources++;

            }
        }
    }
    else
    {// not mix mode
        // SVC
        if (GetIsMrcCall())
        {
            // leave audio softMCU flow as before "Mix for RMX" feature
            if(channelType == kIpAudioChnlType && IsSoftMcu())
            {
                if(channelDirection==cmCapReceive)
                    eLrt[0] = eLogical_relay_audio_decoder;
                else if(channelDirection ==cmCapTransmit)
                    eLrt[0] = eLogical_relay_audio_encoder;
            }
            else
            {// direct packets to ART and listen to ART
                eLrt[0] = eLogical_rtp;
            }
            pStruct->m_allocatedPhysicalResources = 1;
        }
        else
        {
            eLrt[0] = eLogical_rtp;
            pStruct->m_allocatedPhysicalResources = 1;
        }
    }


	for (unsigned int i = 0; i < pStruct->m_allocatedPhysicalResources; i++)
	{
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLrt[i]);
		if(IsValidPObjectPtr(pRsrcDesc))
		{
			pStruct->physicalId[i].connection_id = pRsrcDesc->GetConnectionId();
			pStruct->physicalId[i].party_id = partyId;

			TRACEINTOFUNC << "partyid: " << pStruct->physicalId[i].party_id << ", physicalId # " << i <<  ": connection_id = " << pStruct->physicalId[i].connection_id;

		}
	}

    // save the avcToSvcEncoders connId
    pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLogical_relay_avc_to_svc_video_encoder_1);
    if(IsValidPObjectPtr(pRsrcDesc))
    {
        m_avcToSvcVideoEncoder1ConnId = pRsrcDesc->GetConnectionId();
    }
    else
    {
        m_avcToSvcVideoEncoder1ConnId = DUMMY_CONNECTION_ID;
    }

    pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLogical_relay_avc_to_svc_video_encoder_2);
    if(IsValidPObjectPtr(pRsrcDesc))
    {
        m_avcToSvcVideoEncoder2ConnId = pRsrcDesc->GetConnectionId();
    }
    else
    {
        m_avcToSvcVideoEncoder2ConnId = DUMMY_CONNECTION_ID;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool  CIpCntl::ShouldUpdateMrmpPhysicalIdInfo()
{
    ConnectionID avcToSvcVideoEncoder1ConnId = DUMMY_CONNECTION_ID;
    ConnectionID avcToSvcVideoEncoder2ConnId = DUMMY_CONNECTION_ID;

    CRsrcDesc *pRsrcDesc = NULL;
    DWORD partyId = m_pMrmpInterface->GetPartyRsrcId();
    pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLogical_relay_avc_to_svc_video_encoder_1);
    if(IsValidPObjectPtr(pRsrcDesc))
    {
        avcToSvcVideoEncoder1ConnId = pRsrcDesc->GetConnectionId();
    }
    else
    {// do not send update if encoder was freed
        avcToSvcVideoEncoder1ConnId = m_avcToSvcVideoEncoder1ConnId;
    }

    pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLogical_relay_avc_to_svc_video_encoder_2);
    if(IsValidPObjectPtr(pRsrcDesc))
    {
        avcToSvcVideoEncoder2ConnId = pRsrcDesc->GetConnectionId();
    }
    else
    {// do not send update if encoder was freed
        avcToSvcVideoEncoder2ConnId = m_avcToSvcVideoEncoder2ConnId;
    }

    TRACEINTO << "m_avcToSvcVideoEncoder1ConnId=" << m_avcToSvcVideoEncoder1ConnId << " avcToSvcVideoEncoder1ConnId=" << avcToSvcVideoEncoder1ConnId
            << " m_avcToSvcVideoEncoder2ConnId=" << m_avcToSvcVideoEncoder2ConnId << " avcToSvcVideoEncoder2ConnId=" << avcToSvcVideoEncoder2ConnId;

    if (avcToSvcVideoEncoder1ConnId == m_avcToSvcVideoEncoder1ConnId && avcToSvcVideoEncoder2ConnId == m_avcToSvcVideoEncoder2ConnId)
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CIpCntl::UpdateInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams)
{
    UpdateDownInternalControllerResource(avcToSvcTranslatorRsrcParams,MrmpRsrcParams);
    UpdateUpInternalControllerResource(avcToSvcTranslatorRsrcParams,MrmpRsrcParams);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpCntl::UpdateDownInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams)
{
    if( m_pMrmpRsrcDesc!=NULL && MrmpRsrcParams==NULL) // need to add MRMP
    {
        POBJDELETE(m_pMrmpRsrcDesc);
        POBJDELETE(m_pMrmpInterface);
    }

    for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
        if(m_pAvcToSvcTranslatorRsrcDesc[i]!=NULL && avcToSvcTranslatorRsrcParams[i]==NULL )
        {
            POBJDELETE(m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface);
            POBJDELETE(m_pAvcToSvcTranslatorRsrcDesc[i]);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpCntl::UpdateUpInternalControllerResource(CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* MrmpRsrcParams)
{
          
  if( m_pMrmpRsrcDesc==NULL && MrmpRsrcParams!=NULL) // need to add MRMP
  {
      m_pMrmpRsrcDesc = new CRsrcParams(*MrmpRsrcParams);
      m_pMrmpInterface->Create(m_pMrmpRsrcDesc);
      m_pMrmpInterface->SetRoomId(m_RoomId);
  }

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		if(m_pAvcToSvcTranslatorRsrcDesc[i]==NULL && avcToSvcTranslatorRsrcParams[i]!=NULL )
		{
			m_pAvcToSvcTranslatorRsrcDesc[i] = new CRsrcParams(*avcToSvcTranslatorRsrcParams[i]);
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface = new CHardwareInterface;
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->Create(m_pAvcToSvcTranslatorRsrcDesc[i]);
			m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->SetRoomId( m_RoomId );
			TRACEINTO<<"mix_mode: avcToSvcTranslatorRsrcParams["<<i<<"] OK m_RoomId=" << m_RoomId;
		}
	}  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpCntl::RemoveArtByConnId(ConnectionID ConnId)
{

    TRACEINTO << "Removing ConnectionId = " << ConnId;
    bool found=false;
    int index = 0;
    for (int i=0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && !found; ++i)
    {
        if (!m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
            continue;

        if (ConnId == m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId())
        {
            found=true;
            index = i;
        }
    }
    if (!found)
        return;

    TRACEINTO << "Removing ConnectionId = " << ConnId << " index=" << index;
    CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
    if (pRoutingTbl== NULL)
    {
        PASSERT_AND_RETURN(101);
    }

    if (STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pAvcToSvcTranslatorRsrcDesc[index]))
    {
        TRACEINTO << "pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pAvcToSvcTranslatorRsrcDesc["<<(index)<<"]) FAILS";
        DBGPASSERT(105);
    }
    POBJDELETE(m_pAvcToSvcTranslatorRsrcDesc[index]);

    // remove the interface
    POBJDELETE(m_AvcToSvcTranslatorInterface[index].pAvcToSvcTranslatorInterface);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIpCntl::SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode, BOOL isInternal, BYTE index)
{
    CSegment* pMsg = NULL;
    DWORD seqNum = 0;
    if (pStructure)
    {
        pMsg = new CSegment;
        pMsg->Put(pStructure ,structureSize);

//        CProcessBase* pProcess = CProcessBase::GetProcess();
//        TRACEINTO << "structureSize:" << structureSize << " opcode:" << pProcess->GetOpcodeAsString(opcode).c_str() << " isInternal:" << (isInternal==TRUE? " TRUE":" FALSE") << " index:" << (int)index;
    }

    CHardwareInterface *hwInterface = GetHWInterface(opcode, isInternal, index);
    if (!hwInterface)
    {
        POBJDELETE(pMsg);
        return 0;
    }

    seqNum = hwInterface->SendMsgToMPL(opcode,pMsg);

    if (ShouldInitTimerForSendMsg(opcode))
    {// we don't check Ack for stream on and stream off.
        if (IsValidTimer(MFARESPONSE_TOUT))
            DeleteTimer(MFARESPONSE_TOUT);
        StartTimer(MFARESPONSE_TOUT, MFA_RESPONSE_TIME * SECOND);
        m_MfaReqCounter++;
        m_MfaReqIds.push_back(seqNum);
        TRACEINTO << "new m_MfaReqCounter=" << m_MfaReqCounter << " opcode= " << opcode << " seq_num=" << seqNum;

        POBJDELETE(pMsg);
        return seqNum;
    }
    POBJDELETE(pMsg);

    return 0;
}

/////////////////////////////////////////////////////////////////
void CIpCntl::ReduceReqCounter(ACK_IND_S* pAckIndStruct)
{
    if (!pAckIndStruct)
        return;

    if (!ShouldInitTimerForSendMsg(pAckIndStruct->ack_base.ack_opcode))
        return;

    if (m_MfaReqCounter == 0)
    {
        m_MfaReqIds.clear();
        PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(),"CSipCntl::ReduceReqCounter - counter less than 0");
    }
    else
    {
        m_MfaReqCounter--;
        DWORD requestId = pAckIndStruct->ack_base.ack_seq_num;
        m_MfaReqIds.remove(requestId);

        TRACEINTO << "new m_MfaReqCounter=" << m_MfaReqCounter << " opcode= " << pAckIndStruct->ack_base.ack_opcode << " requestId = " << requestId;
    }

    if (m_MfaReqCounter > 0)
        return;

    InitAllChannelsSeqNum();

    if (IsValidTimer(MFARESPONSE_TOUT))
        DeleteTimer(MFARESPONSE_TOUT);
}
