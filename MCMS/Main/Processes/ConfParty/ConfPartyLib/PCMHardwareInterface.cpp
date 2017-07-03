
#include "PCMHardwareInterface.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "TBStructs.h"

//==============================================================================================================//
// CPCMHardwareInterface implementation
//==============================================================================================================//

//==============================================================================================================//
// constructors
//==============================================================================================================//
CPCMHardwareInterface::CPCMHardwareInterface(ConnectionID ConnectionId,
								 PartyRsrcID ParId,
								 ConfRsrcID ConfId,
								 eLogicalResourceTypes LRT)
{
	m_pRsrcParams = new CRsrcParams(ConnectionId, ParId , ConfId, LRT);
}
//==============================================================================================================//
CPCMHardwareInterface::CPCMHardwareInterface(CRsrcParams& hardwareInterface)
{
    m_pRsrcParams = new CRsrcParams(hardwareInterface);
}

//==============================================================================================================//
CPCMHardwareInterface::~CPCMHardwareInterface()
{
}

//==============================================================================================================//
// API functions 
//==============================================================================================================//
void CPCMHardwareInterface::SendMessageToPCM(DWORD opcode, const char* msgStr)
{
	CSegment* paramSeg = new CSegment;
	if (msgStr)
	{
		DWORD msgLen = strlen(msgStr);
		paramSeg->Put((BYTE*)msgStr,msgLen+1);
	//	paramSeg->DumpHex();

		SendMsgToMPL(opcode, paramSeg);
	}
			
	POBJDELETE(paramSeg);
	
}

//==============================================================================================================//
//-----------------------------------------------------------------------------------------------------
void CPCMHardwareInterface::SendConnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId)
{
	TB_MSG_CONNECT_PCM_S tConnectPcmStruct;

	memset(&tConnectPcmStruct,0,sizeof(TB_MSG_CONNECT_PCM_S));

	tConnectPcmStruct.physical_port1.connection_id = PcmEncoderConnectionId;
	tConnectPcmStruct.physical_port1.party_id = GetPartyRsrcId();
	tConnectPcmStruct.pcm_process_id = pcmMenuId;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tConnectPcmStruct),sizeof(TB_MSG_CONNECT_PCM_S));

	SendMsgToMPL(TB_MSG_CONNECT_PCM_REQ, pMsg);

	POBJDELETE(pMsg);
}
//-----------------------------------------------------------------------------------------------------
void CPCMHardwareInterface::SendDisconnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId)
{
	TB_MSG_CONNECT_PCM_S tConnectPcmStruct;
	memset(&tConnectPcmStruct,0,sizeof(TB_MSG_CONNECT_PCM_S));

	tConnectPcmStruct.physical_port1.connection_id = PcmEncoderConnectionId;
	tConnectPcmStruct.physical_port1.party_id = GetPartyRsrcId(); //temp eitan
	tConnectPcmStruct.pcm_process_id = pcmMenuId;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tConnectPcmStruct),sizeof(TB_MSG_CONNECT_PCM_S));

	SendMsgToMPL(TB_MSG_DISCONNECT_PCM_REQ, pMsg);

	POBJDELETE(pMsg);

}
