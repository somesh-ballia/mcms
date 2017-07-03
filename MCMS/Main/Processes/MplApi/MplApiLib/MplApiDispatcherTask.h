// DispatcherTask.h: interface for the CDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MPL_API_DISPATCHERTASK_H__)
#define _MPL_API_DISPATCHERTASK_H__

#include "DispatcherTask.h"


class CPairOfSockets;
class CMplApiProcess;
class CMplMcmsProtocol;
class CMplMcmsProtocolTracer;
class CMplApiMasterSlaveReqAsyncHandler;


#include "AllocateStructs.h"
#include "SharedHeader.h"
#include "SharedMemory.h"
#include "SharedMemoryMap.h"
//for the SD feture
#include "VideoStructs.h"
#include "VideoApiDefinitions.h"
#include "OpcodesMcmsCardMngrTB.h"



class CMplApiDispatcherTask : public CDispatcherTask
{
CLASS_TYPE_1(CMplApiDispatcherTask,CDispatcherTask )
public:

	CMplApiDispatcherTask();
	virtual ~CMplApiDispatcherTask();
	virtual const char* NameOf() const { return "CMplApiDispatcherTask";}


	virtual void InitTask();

	void*  GetMessageMap();
	void OnMplApiCloseCardConnection(CSegment* pMsg);
	void   OnBasicMsgToMplApi(CSegment* pMsg);
	void   onCleanMasterSlaveRequestTout(CSegment* pMsg);

	BOOL         IsSingleton() const {return YES;}
    CPairOfSockets* m_Mpl_Api_Card2SocketTable[MAX_NUM_OF_BOARDS];

  virtual int  GetTaskMbxBufferSize() const {return 4096 * 1024 - 1;}

private:
	void SendMoveRsrcReq(CMplMcmsProtocol *mplPrtcl, OPCODE opcode, eLogicalResourceTypes lrt = eLogical_res_none);
	void SendMessageToTxSocket(CMplMcmsProtocol *mplPrtcl);
	void SendByLogicResources(CMplMcmsProtocol *mplPrtcl);
	void SendToAllActiveCards(CMplMcmsProtocol *mplPrtcl);
	void SendToCard(WORD boardId, CMplMcmsProtocol *mplPrtcl);
	BOOL HandleVideoEncoderMessages(CMplMcmsProtocol *mplPrtcl);
	BOOL HandleOpenVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);
	void HandleUpdateVideoEncoderMessages(CMplMcmsProtocol * mplMcmsPrtcl, ECntrlType videoEncoderCntlType);
	BOOL HandleConnectVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);
	BOOL HandleCloseVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);

	void SendOpenMasterSlaveVideoEncoderMsgsForLBandFullEnc(CMplMcmsProtocol *mplPrtcl);
	void SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplPrtcl, ECntrlType videoEncoderCntlType);
	DWORD CreateAndSendTBOpenPortReq(const ENCODER_PARAM_S *pEncoderParamStruct, EVideoEncoderType rsrcCntlType,ConnToCardTableEntry& pConnToCardTableEntry,bool forceSeqNum = false, DWORD seqNum =(DWORD)(-1));
	void CreateAndSendTBConnentReq(APIU32 connectionID1,APIU32 partyID1, APIU32 connectionID2,APIU32 partyID2,ConnToCardTableEntry& pConnToCardTableEntry);
	void UpdateVideoEncoderType(CMplMcmsProtocol *mplPrtcl, EVideoEncoderType encoderType);
// 1080_60
	void SendUpdateMasterSlaveVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol * mplMcmsPrtcl, ECntrlType videoEncoderCntlType);
	void CreateAndSendTBGenericReq(CMplMcmsProtocol *mplMcmsPrtcl, EVideoEncoderType rsrcCntlType,ConnToCardTableEntry& pConnToCardTableEntry);
	void SendGenericVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);
	BOOL HandleGenericVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);

	void SendGenericUpdateVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);
	BOOL HandleGenericUpdateVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType);



	void SendConnectMasterSlaveVideoEncoderMsgForLBandFullEnc(CMplMcmsProtocol *mplPrtcl);
	void SendCloseMasterSlaveVideoEncoderMsgsForLBandFullEnc(CMplMcmsProtocol *mplPrtcl);
	void SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplPrtcl, ECntrlType masterEncoderCntlType); //FOR SIMULATION FLOW ONLY
	EVideoEncoderType TranslateVideoEncoderCntlTypeToApi(ECntrlType eVideoEncoderCntlType);
	ECntrlType GetSlaveCntlTypeFromMasterCntlType(ECntrlType eMasterCntlType);


	void CreateAndSendTBClosePortReq(ConnToCardTableEntry& pConnToCardTableEntry);
    void SendByAudioCntrRulls(CMplMcmsProtocol *mplPrtcl);
    //void HandleMessageSendToCard(CMplMcmsProtocol *mplPrtcl, BOOL isShoudBeFoundInDB);
    void SpreadACMessageToAllCards(CMplMcmsProtocol *mplPrtcl);
    void SendACMessageToNewCard(CMplMcmsProtocol *mplPrtcl);
	ConnToCardTableEntry* GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(DWORD conf_id,DWORD party_id, ECntrlType rsrc_cntl_type);
	const ConnToCardTableEntry* GetPhysicalInfoHeaderByConfIdPartyIdAndLogicalRsrcType(DWORD confId, DWORD partyId, eLogicalResourceTypes logicalResourceType);

	BOOL IsVideoEncoderMessage(const CMplMcmsProtocol *mplMcmsPrtcl)const ;
	BOOL IsAudioCntrlOpcode(const CMplMcmsProtocol *mplMcmsPrtcl)const;
	virtual void AddFilterOpcodePoint();
	BYTE GetGenericEncoderDecoderRsrcType(BYTE logicRsrcType)const;

	CMplApiMasterSlaveReqAsyncHandler* GetMasterSlaveReqHandler();
	void SetMasterSlaveRequestHandlerSubIdData(DWORD opcode, DWORD reqId, DWORD subReqId);

	void HandleMultiplePartiesChangeLayoutRequest(CMplMcmsProtocol& mplPrtcl);
	void HandleMultiplePartiesIndicationIconChangeRequest(CMplMcmsProtocol& mplPrtcl);

	bool IsAudioRelayDecoderOnRmx(CMplMcmsProtocol *mplPrtcl);
	void SendUpdateMuteToMrmpAndMpl(CMplMcmsProtocol *pMplMcmsProtocol);

	CMplApiProcess* m_pProcess;
	CMplMcmsProtocolTracer *m_MplMcmsProtocolTracer;
	//BOOL IsH263HighBbIntra();

	PDECLAR_MESSAGE_MAP

};

#endif // !defined(_MPL_API_DISPATCHERTASK_H__)
