#ifndef MPLAPISPECIALCOMMANDHANDLER_H_
#define MPLAPISPECIALCOMMANDHANDLER_H_

#include <map>
#include "PObject.h"
#include "AllocateStructs.h"
#include "TBStructs.h"
#include "IpCmReq.h"

class CMplApiProcess;
class CMplApiSpecialCommandHandler;
class CMplMcmsProtocol;


typedef STATUS (CMplApiSpecialCommandHandler::*HandleSpecialRequest)(void *param);
typedef std::map<OPCODE, HandleSpecialRequest> CSpecialHandlerMethodMap;




class CMplApiSpecialCommandHandler : public CPObject
{
CLASS_TYPE_1(CMplApiSpecialCommandHandler, CPObject)
public:
	CMplApiSpecialCommandHandler(CMplMcmsProtocol &mplMcmsProt);
	virtual ~CMplApiSpecialCommandHandler();

	virtual const char* NameOf() const { return "CMplApiSpecialCommandHandler";}
	STATUS HandeSpecialCommand(void* param = NULL);

	static bool Init();

    void SetPhysicalInfoHeader(const ConnToCardTableEntry* pConnToCardTableEntry);

  DWORD GetPhysicalInfoHeaderByRsrcTypeCntrltype(eResourceTypes resourceType,BYTE searchByCntrlType, ECntrlType cntrlType, ConnToCardTableEntry** entriesFound,DWORD max_entries_to_search);
  const ConnToCardTableEntry* GetPhysicalInfoHeaderByRsrcTypeAndBoardId(eResourceTypes rsrcType);


private:
	// disabled
	CMplApiSpecialCommandHandler(const CMplApiSpecialCommandHandler&);
	CMplApiSpecialCommandHandler& operator=(const CMplApiSpecialCommandHandler&);

	STATUS CreatePartyReq(void* param = NULL);
	STATUS ClosePortReq(void* param = NULL);
	STATUS DeletePartyReq(void* param = NULL);
	STATUS AckInd(void* param = NULL);
	STATUS TBConnectDisconnectReq(void* param = NULL);
	STATUS H323CMUpdateOpenUDPAddrReq(void* param = NULL);
	STATUS H323CMCloseUDPPortReq(void* param = NULL);
	STATUS VideoEncoderChangeLayoutReq(void* param = NULL);
	STATUS VideoUpdateDecoderResolutionReq(void* param = NULL);
	STATUS VideoEncoderDSPSmartSwitchChangeLayoutReq(void* param = NULL);

	STATUS TBStartPreviewReq(void* param = NULL);
	STATUS TBStopPreviewReq(void* param = NULL);

	STATUS ACRequest(void* param = NULL);
	STATUS ACShadowRequest(void* param = NULL);
	STATUS IVRRequest(void* param = NULL);

	STATUS MoveRsrcArt(void* param = NULL);
	STATUS MoveRsrcVideoEnc(void* param = NULL);
	STATUS MoveRsrcVideoDec(void* param = NULL);
	STATUS MoveRsrcNet(void* param = NULL);
	STATUS MoveRsrcMrmp(void* param = NULL);

	STATUS SetNetReqData(DWORD structSize,BYTE * pStruct, bool isDescriptorCanBeMissing=false, bool isNewSetupHeader=false,bool isVirtualPortFromChannelid=false);
	STATUS NetSetupReq(void* param = NULL);
	STATUS NetClearReq(void* param = NULL);
	STATUS NetDisconnectAckReq(void* param = NULL);
	STATUS NetAlertReq(void* param = NULL);
	STATUS NetConnectReq(void* param = NULL);
	STATUS RtcpMsgReq(void* param = NULL);

	STATUS TBConnectDisconnectPCMReq(void* param = NULL);
	STATUS PCMMessage(void* param = NULL);
	STATUS RtcpFlowControlMsgReq(void* param = NULL);

	//ICE
	STATUS IceGeneralReq(void* param = NULL);

	STATUS TBVideoPreferenceReq(void* param = NULL);
	STATUS TBReceiverBandWidthReq(void* param = NULL);
	STATUS OpenChannelReq(void* param = NULL);
	STATUS UpdateChannelReq(void* param = NULL);
	STATUS RtpFeccTokenResponseReq(void* param = NULL);


	//TIP
	STATUS TipStartNegotiationReq(void* param = NULL);
	STATUS TipEndNegotiationReq(void* param = NULL);
	STATUS TipUpdateCloseReq(void* param = NULL);
	STATUS TipKillObjectReq(void* param = NULL);
	STATUS TipContentMsgReq(void* param = NULL);
	STATUS PartyMonitoringReq(void* param = NULL);
	STATUS TipKillTipContextReq(void* param = NULL);


	const ConnToCardTableEntry* GetPhysicalInfoHeaderByRsrcType(eResourceTypes resourceType);
    const ConnToCardTableEntry* GetPhysicalInfoHeaderByRsrcTypeCntrltype(eResourceTypes resourceType, ECntrlType cntrlType);
	const ConnToCardTableEntry* GetPhysicalInfoHeaderByPartyIDRsrcType(DWORD party_id, eResourceTypes rsrcType, eLogicalResourceTypes lrt=eLogical_res_none);
	const ConnToCardTableEntry* GetPhysicalInfoHeaderByConfAndPartyID(DWORD conf_id,DWORD party_id, eResourceTypes rsrcType, eLogicalResourceTypes lrt=eLogical_res_none);
    const ConnToCardTableEntry* GetPhysicalInfoHeaderByConnectionID(DWORD connection_id);
    const ConnToCardTableEntry* GetPhysicalInfoHeaderByLogicalRsrcType(eLogicalResourceTypes logicalResourceType);
    const ConnToCardTableEntry* GetPhysicalInfoHeaderByConfAndLogicalRsrcType(DWORD conf_id, eLogicalResourceTypes logicalResourceType);
    const ConnToCardTableEntry* GetMainARTphysicalInfoHeaderByConfAndPartyID(DWORD conf_id, DWORD party_id);


    void SetPhysicalResourceInfo(const ConnToCardTableEntry* pConnToCardTableEntry,
                                 PHYSICAL_RESOURCE_INFO_S &PhysicalResourceInfo,bool setChannlId=false)const;

	STATUS FillPhysicalHeaderByResourceType(eResourceTypes type);
    STATUS FillPhysicalHeaderByResourceTypeCntrlType(eResourceTypes resourceType, ECntrlType cntrlType);
	STATUS FillPhysicalHeaderByPartyIdResourceType(eResourceTypes type, eLogicalResourceTypes lrt=eLogical_res_none);

	//MRMP messages
	STATUS AddRemoveVideoOperationPointSetReq(void* param = NULL);

	void PrintAssert(eResourceTypes type)const;
  void PrintAssert(int confId, int partyId, OPCODE opcode)const;
	void PrintAssert(DWORD partyId, eResourceTypes type)const;
    void PrintAssert(eResourceTypes type, ECntrlType cntrlType, OPCODE opcode)const;
    void PrintAssert(eLogicalResourceTypes lrt) const;

	void SetPSTNPhysicalInfoHeader(const ConnToCardTableEntry & localDesc);
	void SetPSTNContent(const ConnToCardTableEntry & localDesc
			    ,TOpenUdpPortOrUpdateUdpAddrMessageStruct & openUdpStruct);
	void SetPSTNContent(const ConnToCardTableEntry& localDesc,  TCloseUdpPortMessageStruct& openUdpStruct);

	//BFCP
	STATUS TBBfcpMessageReq(void* param = NULL);
	STATUS TBPacketLossReq(void* param = NULL);

	//VSR
	STATUS VsrMsgReq(void* param = NULL);

	//MS SVC PLI
	STATUS MsSvcPliMsgReq(void* param = NULL);

	// MS SVC Init
	STATUS MsSvcInitDesc(SingleStreamDesc* pDesc, const BYTE noOfStreamDescs, const ConnToCardTableEntry& connToCardTableEntry);
    STATUS MsSvcP2PInitReq(void* param = NULL);
    STATUS MsSvcAvMcuInitReq(void* param = NULL);

	// MS SVC Mux/Dmux
    STATUS MsSvcMuxDmuxDesc(SingleStreamDesc* pDesc, const BYTE noOfStreamDescs);
    STATUS MsSvcDspInfoForPartyMonitoringReqDesc(DspInfoForPartyMonitoringReq* pDesc, const BYTE noOfStreamDescs);
    STATUS MsSvcAvMcuMuxReq(void* param = NULL);
    STATUS MsSvcAvMcuDmuxReq(void* param = NULL);
    STATUS MsSvcAvMcuMonitoringReq(void* param= NULL);

	// DTLS
	STATUS DtlsStartReq(void* param);
	STATUS DtlsCloseReq(void* param);

	CMplMcmsProtocol 	&m_MplMcmsProt;

	static CSpecialHandlerMethodMap m_SpecialHandlerMap;
	static CMplApiProcess *m_pMplApiProcess;
	static bool m_IsInit;
};

#endif /*MPLAPISPECIALCOMMANDHANDLER_H_*/
