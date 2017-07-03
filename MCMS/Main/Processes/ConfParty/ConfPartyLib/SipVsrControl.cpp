/*
 * SipVsrControl.cpp
 *
 *  Created on: Jul 7, 2013
 *      Author: abental
 */

#include <algorithm>
#include "OpcodesRanges.h"

#include "Trace.h"
#include "TraceHeader.h"
#include "ObjString.h"
#include "ConfPartyOpcodes.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "SIPControl.h"
#include "SipVsrControl.h"
#include "VideoApiDefinitions.h"
#include "IPUtils.h"

PBEGIN_MESSAGE_MAP(CSipVsrCtrl)

ONEVENT(IP_CM_RTCP_VSR_IND,			ANYCASE, 			CSipVsrCtrl::OnIpCmVsrMessageInd)

ONEVENT(VSR_INIT_ACK_TOUT,			IDLE,	 		CSipVsrCtrl::OnIpCmVsrInappropriateEvent)
ONEVENT(VSR_INIT_ACK_TOUT,			sVSR_READY, 	CSipVsrCtrl::OnIpCmVsrInappropriateEvent)
ONEVENT(VSR_INIT_ACK_TOUT,			sVSR_INIT, 		CSipVsrCtrl::OnIpCmVsrInitAckTout)


PEND_MESSAGE_MAP(CSipVsrCtrl,CStateMachine);

void CSipVsrCtrl::OnIpCmVsrInappropriateEvent(CSegment* pParam)
{
	CSmallString log;

	if (pParam)
	{
		DWORD opcode;
		*pParam >> opcode;
		log << "Opcode: " << opcode;
	}
	else
	{
		log << "Unextractable opcode";
	}

	log << " arrived at state: " << m_state;
	PTRACE2(eLevelInfoNormal, "CSipVsrCtrl::OnIpCmVsrInappropriateEvent - ", log.GetString());
}

////////////////////////////////////////////////////////////////////////////
CSipVsrCtrl::CSipVsrCtrl(CTaskApp *pOwnerTask,CPartyApi* pPartyApi) : CStateMachine(pOwnerTask)
{
	m_vsrPending    = FALSE;
	m_pMfaInterface = NULL;
	m_pPartyApi     = pPartyApi;
    m_state         = IDLE;
   	NewRxVsr(); 			//Flagging last-Rx-Vsr holder as uninitialized

	VALIDATEMESSAGEMAP;
}

////////////////////////////////////////////////////////////////////////////
CSipVsrCtrl::~CSipVsrCtrl()
{
	if (IsValidTimer(VSR_INIT_ACK_TOUT))
	{
		DeleteTimer(VSR_INIT_ACK_TOUT);
	}
}

////////////////////////////////////////////////////////////////////////////
CSipVsrCtrlP2P::CSipVsrCtrlP2P(CTaskApp *pOwnerTask,CPartyApi* pPartyApi) : CSipVsrCtrl(pOwnerTask,pPartyApi)
{
	m_requestId = 1;
	m_lastTxVsr.num_vsrs_params = (APIU8)   NUMERIC_NULL;
	m_mplInit.bIsAVMCU          = (UINT32)  NUMERIC_NULL;
	VALIDATEMESSAGEMAP;
}

////////////////////////////////////////////////////////////////////////////
CSipVsrCtrlAvMcu::CSipVsrCtrlAvMcu(CTaskApp *pOwnerTask,CPartyApi* pPartyApi) : CSipVsrCtrl(pOwnerTask,pPartyApi)
{
	for(int i = 0; i < MAX_STREAM_LYNC_2013_CONN; ++i)
	{
		m_requestId[i] = 1;
		m_lastTxVsr.st_vsrs_single_stream[i].num_vsrs_params = (APIU8) NUMERIC_NULL;
	}
	m_lastTxVsr.num_vsrs_streams	= (UINT32) NUMERIC_NULL;
	m_dmuxMsg.bIsAVMCU 				= (UINT32) NUMERIC_NULL;
	m_isEncrypted = FALSE;
	VALIDATEMESSAGEMAP;
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrl::IncorrectUsage(const char* caller) const
{
	CMedString log;
	log << caller << " may not be called from " << NameOf();
	PTRACE2(eLevelError,"CSipVsrCtrl::IncorrectUsageLog - : ", log.GetString());
	DBGPASSERT(1);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi)   // Not callable, pure virtual with implementation
{
	TRACEINTO;
	if (m_state != IDLE)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipVsrCtrl::Initialize - received Initialize command at state : ", m_state);
	}
	m_pMfaInterface = pMfaInterface;
	m_pPartyApi 	= pPartyApi;
	m_state 		= sVSR_INIT;
	if (IsValidTimer(VSR_INIT_ACK_TOUT)) DeleteTimer(VSR_INIT_ACK_TOUT);
	StartTimer(VSR_INIT_ACK_TOUT, MFA_RESPONSE_TIME * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrlP2P::Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcBasicLync2013InfoReq& initInfo)
{
	CSipVsrCtrl::Initialize(pMfaInterface, pPartyApi);
	SendMsSvcInitMsg(reinterpret_cast<BYTE*>(&initInfo));
	m_mplInit = initInfo;
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrlAvMcu::Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcAvMcuLync2013InfoReq& initInfo)
{
	TRACEINTO;
	//===============
	// Sending init
	//===============
	CSipVsrCtrl::Initialize(pMfaInterface, pPartyApi);
	SendMsSvcInitMsg(reinterpret_cast<BYTE*>(&initInfo));

	//=======================================================================
	// Initializations and their retries are much less real time than dmux,
	// so keeping data in dmux format
	//=======================================================================
	m_dmuxMsg.bIsAVMCU 	= initInfo.bIsAVMCU;
	m_dmuxMsg.bIsIceParty	= initInfo.bIsIceParty;
	for (int i = 0; i < min(MAX_STREAM_DMUX_LYNC_CONN, MAX_STREAM_LYNC_2013_CONN); ++i)
	{
		m_dmuxMsg.rxConnectedParties[i] = initInfo.ConnectedParties[i];

	}

	m_isEncrypted = (initInfo.bIsEncrypted != 0);

	for (int i = 0; i < MAX_STREAM_LYNC_2013_CONN; ++i)
	{
		m_wasEverActive[i] = FALSE;
	}

}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::InitializeAcked()
{
	DeleteTimer(VSR_INIT_ACK_TOUT);
	m_state = sVSR_READY;
	PTRACE(eLevelInfoNormal, "CSipVsrCtrl::InitializeAcked");

	if (m_vsrPending)
	{
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::InitializeAcked - Resending pending VSR");
		ResendVsrMsg();
	}
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::OnIpCmVsrInitAckTout(CSegment* pParam)
{
	PTRACE(eLevelError, "CSipVsrCtrl::OnIpCmVsrInitAckTout - Init process timed out, resending");
	ResendMsSvcInitMsg();
	StartTimer(VSR_INIT_ACK_TOUT, MFA_RESPONSE_TIME * SECOND);
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrlAvMcu::ResendMsSvcInitMsg() const
{
	TRACEINTO;
	BOOL ret = FALSE;

	if (m_dmuxMsg.bIsAVMCU != (DWORD) NUMERIC_NULL)
	{
		//===========================================
		// Database is legit, creating init message
		//===========================================
		mcAvMcuLync2013InfoReq initMsg;
		initMsg.bIsAVMCU 	= m_dmuxMsg.bIsAVMCU;
		initMsg.bIsIceParty	= m_dmuxMsg.bIsIceParty;
		for (int i = 0; i < min(MAX_STREAM_DMUX_LYNC_CONN, MAX_STREAM_LYNC_2013_CONN); ++i)
		{
			initMsg.ConnectedParties[i] = m_dmuxMsg.rxConnectedParties[i];
		}

		//=================
		// Resending init
		//=================
		ret = SendMsSvcInitMsg((BYTE*)&initMsg);
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode) const
{
	DWORD ret = 0;
	if (IsValidPObjectPtr(m_pMfaInterface))
	{

		CSegment msg;
		if (pStructure)
		{
			msg.Put(pStructure ,structureSize);
		}

		ret = m_pMfaInterface->SendMsgToMPL(opcode, &msg);
	}
	else
	{
		PTRACE(eLevelError, "CSipVsrCtrl::SendMsgToMpl - no MFA interface supplied");
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::MaxPixelsPerAspectRatio(BYTE aspectWidth, BYTE aspectHeight, DWORD width, DWORD height) const
{
	DWORD maxPixels;
	if (/* Dropping exact calculation per algo's request, and therefore: */ 0 && aspectWidth && aspectHeight)
	{
		DWORD adjWidth 	= min(width, height*aspectWidth/aspectHeight);
		DWORD adjHeight	= min(height, width*aspectHeight/aspectWidth);
		maxPixels = adjWidth*adjHeight;
	}
	else
	{
		maxPixels = width*height;
	}

	return maxPixels;
}
////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::GetVsrQRIndexFromFlag() const
{
	DWORD         nQrIndex  = 0;
	CSysConfig*   sysConfig  = NULL;
	CProcessBase* pProcess   = CProcessBase::GetProcess();

	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();

	if(sysConfig)
		sysConfig->GetDWORDDataByKey("RTCP_VSR_QRH_INDEX_FOR_FEC", nQrIndex);



	return nQrIndex;
}
////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::InterpolateEntryData(TCmRtcpVsrEntry& cmVsrEntry, int entryNum) const
{
	//==============================================
	// Filling fields according to rules from algo
	//==============================================
	for (int i = 1; i < VSR_NUM_OF_BITRATE_RANGES; ++i) 	cmVsrEntry.bitRateHistogram[i] 			= 0;
	cmVsrEntry.bitRateHistogram[0] 			= 1;

	//for (int i = 1; i < VSR_NUM_OF_QUALITY_LEVELS; ++i) 	cmVsrEntry.qualityReportHistogram[i] 	= 0;
	//DWORD nQRIndex = 3; //GetVsrQRIndexFromFlag();
	//if(nQRIndex >= VSR_NUM_OF_QUALITY_LEVELS) nQRIndex = 0;
	//cmVsrEntry.qualityReportHistogram[nQRIndex] = 1;

	cmVsrEntry.bitRatePerLevel 				= 10000;
	cmVsrEntry.flags						= (cmVsrEntry.payloadType == MS_SVC_PT_RTV? 4 : 0);
	cmVsrEntry.dummyPadding					= 0;

	//========================================================================
	// May/Must instances, filled under the assumption that we can transcode
	// (At the moment may/must is determined at the VSR controller level)
	//========================================================================
	if (entryNum == 1)
	{
		cmVsrEntry.numberOfMayInstances			= 0;
		cmVsrEntry.numberOfMustInstances		= 1;
	}
	else
	{
		cmVsrEntry.numberOfMayInstances			= 1;
		cmVsrEntry.numberOfMustInstances		= 0;
	}

	//=====================================================
	// Aspect ratio - code is ready for width != height
	// Checking by bit order, to give a matching priority
	//=====================================================
	if (cmVsrEntry.aspectRatioBitMask & MS_SVC_FOUR_BY_THREE)
	{
		cmVsrEntry.maximumNumberOfPixels = MaxPixelsPerAspectRatio(4, 3, cmVsrEntry.maxWidth, cmVsrEntry.maxHeight);
	}
	else if (cmVsrEntry.aspectRatioBitMask & MS_SVC_SIXTEEN_BY_NINE)
	{
		cmVsrEntry.maximumNumberOfPixels = MaxPixelsPerAspectRatio(16, 9, cmVsrEntry.maxWidth, cmVsrEntry.maxHeight);
	}
	else if (cmVsrEntry.aspectRatioBitMask & MS_SVC_ONE_BY_ONE)
	{
		cmVsrEntry.maximumNumberOfPixels = MaxPixelsPerAspectRatio(1, 1, cmVsrEntry.maxWidth, cmVsrEntry.maxHeight);
	}
	else if (cmVsrEntry.aspectRatioBitMask & MS_SVC_THREE_BY_FOUR)
	{
		cmVsrEntry.maximumNumberOfPixels = MaxPixelsPerAspectRatio(3, 4, cmVsrEntry.maxWidth, cmVsrEntry.maxHeight);
	}
	else if (cmVsrEntry.aspectRatioBitMask & MS_SVC_NINE_BY_SIXTEEN)
	{
		cmVsrEntry.maximumNumberOfPixels = MaxPixelsPerAspectRatio(9, 16, cmVsrEntry.maxWidth, cmVsrEntry.maxHeight);
	}
	else if (cmVsrEntry.aspectRatioBitMask & MS_SVC_TWENTY_BY_THREE)
	{
		cmVsrEntry.maximumNumberOfPixels = MaxPixelsPerAspectRatio(20, 3, cmVsrEntry.maxWidth, cmVsrEntry.maxHeight);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipVsrCtrl::InterpolateEntryData - max pixels will be height*width as aspect ratio did not match known values: ", cmVsrEntry.aspectRatioBitMask);
		cmVsrEntry.maximumNumberOfPixels = cmVsrEntry.maxWidth * cmVsrEntry.maxHeight;
	}
}



////////////////////////////////////////////////////////////////////////////
CSipVsrCtrl::TxVsrPolicy_t CSipVsrCtrl::TxVsrPolicy()
{
	TxVsrPolicy_t policy = eIgnoreTxVsr;

	//=====================================================================================
	// VSR is either sent immediately, or marked to be sent when initialization completes
	//=====================================================================================
	if (m_state == sVSR_INIT)
	{
		policy = ePendVsr;
		PTRACE(eLevelInfoNormal,"CSipVsrCtrl::TxVsrPolicy - Controller initialization not yet acked, delaying VSR");
	}
	else if (m_state == sVSR_READY)
	{
		PTRACE(eLevelInfoNormal,"CSipVsrCtrl::TxVsrPolicy - VSR(s) are allowed to be sent immediately");
		policy = eSendVsrToRemote;
	}
	else if (m_state == IDLE)
	{
		PTRACE2INT(eLevelError,"CSipVsrCtrl::TxVsrPolicy - Ignoring VSR in state", m_state);
	}

	return policy;
}


////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrlP2P::SendVsr(const ST_VSR_SINGLE_STREAM& vsr)
{
	BOOL ret = FALSE;

	//============================
	// Recording VSR and sending
	//============================
	m_lastTxVsr = vsr;
	TxVsrPolicy_t vsrPolicy = TxVsrPolicy();
	switch(vsrPolicy)
	{
	case ePendVsr:
		m_vsrPending = TRUE;
		break;

	case eSendVsrToRemote:
		ret = SendVsrInMplFormat(vsr, 0);
		break;

	default:
		PTRACE2INT(eLevelError, "CSipVsrCtrlP2P::SendVsr - Ignoring VSR for policy: ", vsrPolicy);
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrl::ComparePartyVsrs(const ST_VSR_SINGLE_STREAM& lVsr, const ST_VSR_SINGLE_STREAM& rVsr) const
{
	BOOL ret = FALSE;

	if (lVsr.num_vsrs_params == rVsr.num_vsrs_params)
	{
		if (rVsr.num_vsrs_params <= VSR_MAX_ENTRIES)
		{
			//========================================================
			// Preparing to compare VSR-s: Ignoring unfilled entries
			// (as it will be faster than full-struct memcmp)
			//========================================================
			const int unusedEntriesSize = (VSR_MAX_ENTRIES - rVsr.num_vsrs_params) * sizeof(ST_VSR_PARAMS);
			const int sizeToCompare = sizeof(ST_VSR_SINGLE_STREAM) - unusedEntriesSize;

			//==================
			// Comparing VSR-s
			//==================
			if (memcmp(&lVsr, &rVsr, sizeToCompare))
			{
				PTRACE(eLevelInfoNormal, "CSipVsrCtrl::ComparePartyVsrs - VSR content differs");
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CSipVsrCtrl::ComparePartyVsrs - VSR content identical");
				ret = TRUE;
			}
		}
		else
		{
			PTRACE2INT(eLevelError, "CSipVsrCtrl::ComparePartyVsrs - VSR number of entries exceeds maximum: ",
					   	   	   	   	rVsr.num_vsrs_params);
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::ComparePartyVsrs - VSR entries number differs");
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrlAvMcu::MarkForSignificantChanges(const ST_VSR_MUTILPLE_STREAMS& vsr, BOOL* sigChangeArr) const
{
	DWORD 	newVsrNum		= vsr.num_vsrs_streams;
	DWORD 	OldVsrNum		= m_lastTxVsr.num_vsrs_streams;
	BOOL	ret				= FALSE;

	if (newVsrNum <= MAX_STREAM_LYNC_2013_CONN && (m_vsrPending || (DWORD) NUMERIC_NULL == OldVsrNum))
	{
		//===============================================================
		// First VSR request is always significantly changed, naturally
		//===============================================================
		PTRACE(eLevelInfoNormal, "CSipVsrCtrlAvMcu::MarkForSignificantChanges - First SendVsr, naturally a significant change");
		memset(sigChangeArr, TRUE, sizeof(sigChangeArr));
		ret = TRUE;
	}
	else if (newVsrNum <= MAX_STREAM_LYNC_2013_CONN && OldVsrNum == newVsrNum)
	{
		//=================================================
		// Analyzing differences between old and new VSRs
		//=================================================
		for (DWORD i = 0; i < newVsrNum; ++i)
		{
			const ST_VSR_SINGLE_STREAM& oldVsr = m_lastTxVsr.st_vsrs_single_stream[i];
			const ST_VSR_SINGLE_STREAM& newVsr = vsr.st_vsrs_single_stream[i];

			//=============================================================
			// MSI or party ID information change is a significant change
			//=============================================================
			if (oldVsr.msi != newVsr.msi || oldVsr.partyId != newVsr.partyId)
			{
				ret = TRUE;
				sigChangeArr[i] = TRUE;
				CMedString log;
				log << "CSipVsrCtrlAvMcu::MarkForSignificantChanges - Significant change for slot[" << i << "]: oldVsr.msi["
					<< oldVsr.msi << "] newVsr.msi[" << newVsr.msi << "] // oldVsr.partyId[" << oldVsr.partyId
					<< "] newVsr.partyId[" << newVsr.partyId << "]";
				PTRACE(eLevelInfoNormal, log.GetString());
			}
			else
			{
				sigChangeArr[i] = FALSE;
			}
		}
	}
 	else
	{
 		//=================
 		// Error handling
 		//=================
		memset(sigChangeArr, FALSE, sizeof(sigChangeArr));
		CSmallString log;
		log << "CSipVsrCtrlAvMcu::MarkForSignificantChanges - Problem with VSR number, OldVsrNum["
											<< OldVsrNum << "] newVsrNum[" << newVsrNum << "]";
		PTRACE(eLevelError, log.GetString());
	}

	return ret;
}

BOOL CSipVsrCtrlAvMcu::SendDmux(const ST_VSR_MUTILPLE_STREAMS& vsr)
{
	//========================================================================================
	// Preparing Dmux, activating and adding information to active MSIs and disabling others
	//========================================================================================
	for(DWORD i = 0; i < min(vsr.num_vsrs_streams, (DWORD) MAX_STREAM_LYNC_2013_CONN); ++i)
	{
		SingleStreamDesc& dmuxParty = m_dmuxMsg.rxConnectedParties[i];
		const ST_VSR_SINGLE_STREAM& newVsr = vsr.st_vsrs_single_stream[i];
		dmuxParty.unPartyId	= newVsr.partyId;
		if (newVsr.partyId != 0 && newVsr.partyId != (DWORD) NUMERIC_NULL && newVsr.msi != (DWORD) NUMERIC_NULL && newVsr.num_vsrs_params != (DWORD) NUMERIC_NULL)
		{
			if(i != 0/*is not main party*/ && TRUE == m_wasEverActive[i] && FALSE == dmuxParty.bIsActive  && m_isEncrypted)
			{
				dmuxParty.bUpdateArtInfo = TRUE;
			}
			dmuxParty.bIsActive = TRUE;
			dmuxParty.lyncMSI	= newVsr.msi;

			m_wasEverActive[i] = TRUE;
		}
		else
		{
			dmuxParty.bIsActive = FALSE;
			dmuxParty.lyncMSI	= VSR_SOURCE_NONE;
			dmuxParty.bUpdateArtInfo = FALSE;
		}
	}

	//=======================
	// Sending Dmux message
	//=======================
	PTRACE(eLevelInfoNormal, "CSipVsrCtrlAvMcu::SendDmux - Sending dmux");
	return (STATUS_OK == SendMsgToMpl(reinterpret_cast<BYTE*>(&m_dmuxMsg), sizeof(m_dmuxMsg), CONFPARTY_CM_DMUX_ON_AVMCU_CALL_REQ));
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrlAvMcu::SendVsr(const ST_VSR_MUTILPLE_STREAMS& vsr)
{
	BOOL 			ret 		= FALSE;
	TxVsrPolicy_t 	vsrPolicy	= TxVsrPolicy();

	if (vsrPolicy == eSendVsrToRemote)
	{
		//=======================================
		// Preparing analyze which VSRs to send
		//=======================================
		DWORD NewVsrNum		= vsr.num_vsrs_streams;
		DWORD OldVsrNum		= m_lastTxVsr.num_vsrs_streams;
		BOOL vsrsToSend[MAX_STREAM_LYNC_2013_CONN];

		//========================================================================================
		// Mark significant changes in VSRs and if any significant changes were found, send dmux
		//========================================================================================
		if (MarkForSignificantChanges(vsr, vsrsToSend))
		{
			SendDmux(vsr);
		}

		//=================================================================================
		// Vsr number has to be in range and should not change (except for the first VSR)
		//=================================================================================
		if (NewVsrNum > MAX_STREAM_LYNC_2013_CONN || (OldVsrNum != NewVsrNum && (DWORD) NUMERIC_NULL != OldVsrNum))
		{
			CSmallString log;
			log << "CSipVsrCtrlAvMcu::SendVsr - Problem with VSR number, OldVsrNum["
				<< OldVsrNum << "] NewVsrNum[" << NewVsrNum << "]";
			PTRACE(eLevelError, log.GetString());
		}
		//========================================================================================
		// Non significant changes for Dmux may still trigger a VSR Tx.  Analyzing non-new VSRs.
		//========================================================================================
		else if (!m_vsrPending && (DWORD) NUMERIC_NULL != OldVsrNum)
		{
			//=================================================
			// Analyzing differences between old and new VSRs
			//=================================================
			for (DWORD i = 0; i < NewVsrNum; ++i)
			{
				//=======================
				// Sending altered VSRs
				//=======================
				if (!vsrsToSend[i] && !ComparePartyVsrs(m_lastTxVsr.st_vsrs_single_stream[i], vsr.st_vsrs_single_stream[i]))
				{
					vsrsToSend[i] = TRUE;
				}
			}
		}

		//===============
		// Sending VSRs
		//===============
		for (DWORD i = 0; i < MAX_STREAM_LYNC_2013_CONN; ++i)
		{
			if (vsrsToSend[i])
			{
				ret = SendVsrInMplFormat(vsr.st_vsrs_single_stream[i], i);
			}
		}
	}
	else
	{
		m_vsrPending = (ePendVsr == vsrPolicy);
	}

	//=====================
	// Recording last VSR
	//=====================
	m_lastTxVsr = vsr;

	return ret;
}

////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipVsrCtrl::PacketlossOnRecDirectionNeedToSendVsr(DWORD ssrc, DWORD fecPercent)
{
	BOOL 	ret = FALSE;
	DWORD 	slotNum;

	ST_VSR_SINGLE_STREAM* pTxVsr = GetLastTxVsr(ssrc, slotNum);

	if (pTxVsr == NULL || pTxVsr->num_vsrs_params == (APIU8) NUMERIC_NULL)
		TRACEINTO << "LYNC2013_FEC_RED: pLastTxVsr is NULL will not send vsr (that caused by packetLoss of:" << fecPercent << ", ssrc:" << ssrc << ")";
	else
	{
		//The default value of the system flag RTCP_VSR_QRH_INDEX_FOR_FEC for QR index is 0.
		//if the flag is set to >0 that means we will consider its value. else, we will take the fecPrecent as QR index.
		DWORD nQRIndex = GetVsrQRIndexFromFlag(); //DWORD nQRIndex = fecPercent;

		if (nQRIndex == 0)
			nQRIndex = fecPercent;

		if (nQRIndex > 0)
			nQRIndex = VSR_NUM_OF_QUALITY_LEVELS-1;

		DWORD numOfEntries = pTxVsr->num_vsrs_params;
		if (numOfEntries > VSR_MAX_ENTRIES)
			numOfEntries = VSR_MAX_ENTRIES;

		if (pTxVsr->st_vsrs_params[0].qualityReportHistogram[nQRIndex] != 1)   //this is not matter if it is rtv/H264 because they are have the same value
		{
			for (DWORD i=0; i<numOfEntries; ++i)
			{
				//ST_VSR_PARAMS& vsrEntry = pTxVsr->st_vsrs_params[i];

				for (int j=0; j<VSR_NUM_OF_QUALITY_LEVELS; ++j)
					pTxVsr->st_vsrs_params[i].qualityReportHistogram[j] = 0;

				pTxVsr->st_vsrs_params[i].qualityReportHistogram[nQRIndex] = 1;
			}

			TRACEINTO << "LYNC2013_FEC_RED: will send vsr that caused by packetLoss of:" << fecPercent << ", nQRIndex:" << nQRIndex << ", ssrc:" << ssrc;

			ret = SendVsrInMplFormat(*pTxVsr, slotNum);
		}
		else
			TRACEINTO << "LYNC2013_FEC_RED: will not send vsr because there is no change in the qualityReportHistogram. packetLoss of:" << fecPercent << ", nQRIndex:" << nQRIndex << ", ssrc:" << ssrc;

	}

}
////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrl::VideoSync(DWORD SSRC, BOOL isSynched)
{
	BOOL 	ret = FALSE;
	DWORD 	slotNum;
	ST_VSR_SINGLE_STREAM* pLastTxVsr = GetLastTxVsr(SSRC, slotNum);

	if(NULL == pLastTxVsr || pLastTxVsr->num_vsrs_params == (APIU8) NUMERIC_NULL)
	{
		//=======================================================================
		// Video may not be sync-d prior to first VSR request, this is expected
		//=======================================================================
		PTRACE2INT(eLevelInfoNormal,"CSipVsrCtrl::VideoSync - No need to resend, as no VSR was sent yet for SSRC ", SSRC);
	}
	else
	{
		//======================================
		// Updating sync state and sending vsr
		//======================================
		CSmallString log;
		log << "SSRC[" << SSRC << "] isSynched[" << isSynched << "]";
		PTRACE2(eLevelInfoNormal,"CSipVsrCtrl::VideoSync - Resending VSR SSRC, ", log.GetString());
		pLastTxVsr -> key_frame = !isSynched;
		ret = SendVsrInMplFormat(*pLastTxVsr, slotNum);
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::SipFRToVsrFR(DWORD sipFR) const
{
	DWORD vsrFR = 1;
	if (sipFR >= 60)
	{
		vsrFR <<= 6;
	}
	else if (sipFR >= 50)
	{
		vsrFR <<= 5;
	}
	else if (sipFR >= 30)
	{
		vsrFR <<= 4;
	}
	else if (sipFR >= 25)
	{
		vsrFR <<= 3;
	}
	else if (sipFR >= 15)
	{
		vsrFR <<= 2;
	}
	else if (sipFR >= 13)
	{
		vsrFR <<= 1;
	}
	// 5 FPS and below is a no go
	else if (sipFR < 5)
	{
		vsrFR = 0;
	}

	return vsrFR;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::VsrFRToSipFR(DWORD vsrFR) const
{
	DWORD sipFR = 0;
	if (vsrFR & (1<<6))
	{
		sipFR = 60;
	}
	else if (vsrFR & (1<<5))
	{
		sipFR = 50;
	}
	else if (vsrFR & (1<<4))
	{
		sipFR = 30;
	}
	else if (vsrFR & (1<<3))
	{
		sipFR = 25;
	}
	else if (vsrFR & (1<<2))
	{
		sipFR = 15;
	}
	else if (vsrFR & (1<<1))
	{
		sipFR = 13;
	}
	// Unsupported condition (allowing 7 fps)
	else if (vsrFR & (1<<0))
	{
		sipFR = 7;
	}

	return sipFR;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::SipAspectToVsrAspect(DWORD sipAspect) const
{
	DWORD vsrAspect = 0;
	if (sipAspect == E_VIDEO_RES_ASPECT_RATIO_4_3)
	{
		vsrAspect = MS_SVC_FOUR_BY_THREE;
	}
	else if (sipAspect == E_VIDEO_RES_ASPECT_RATIO_16_9)
	{
		vsrAspect = MS_SVC_SIXTEEN_BY_NINE;
	}

	return vsrAspect;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrl::VsrAspectToSipAspect(DWORD vsrAspect) const
{
	DWORD sipAspect = E_VIDEO_RES_ASPECT_RATIO_4_3;   // Note that when received aspect mask does not contain expected values this func will be forcing 4*3

	if (vsrAspect & MS_SVC_FOUR_BY_THREE)
	{
		sipAspect = E_VIDEO_RES_ASPECT_RATIO_4_3;
	}
	else if (vsrAspect & MS_SVC_SIXTEEN_BY_NINE)
	{
		sipAspect = E_VIDEO_RES_ASPECT_RATIO_16_9;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipVsrCtrl::vsrAspectTosipAspect - Forcing 4*3 since received aspect mask did not contain expected aspects: ", vsrAspect);
	}

	return sipAspect;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrl::SendVsrInMplFormat(const ST_VSR_SINGLE_STREAM& srcVsr, DWORD vsrSlot)
{
	//===================
	// Filling VSR info
	//===================
	TCmRtcpVsrMsg 				cmVsrMsg;
	TCmRtcpVsrInfo& 			cmVsrInfo 	= cmVsrMsg.cmRtcpVsrInfo;
	memset(&cmVsrMsg.cmRtcpHeader,0,sizeof(cmVsrMsg.cmRtcpHeader));

	//=====================
	// Filling VSR header
	//=====================
	cmVsrInfo.senderSSRC		= srcVsr.sender_ssrc;
	cmVsrInfo.MSI 				= srcVsr.msi;
	cmVsrInfo.keyFrame			= srcVsr.key_frame;
	cmVsrInfo.requestId			= GetReqId(vsrSlot);
	if (cmVsrInfo.requestId != (APIU16) NUMERIC_NULL) SetReqId(vsrSlot, cmVsrInfo.requestId + 1);

	//================================
	// Determining number of entries
	//================================
	if (srcVsr.num_vsrs_params <= VSR_MAX_ENTRIES)
	{
		cmVsrInfo.numberOfEntries = srcVsr.num_vsrs_params;
	}
	else
	{
		CMedString log;
		log << "CSipVsrCtrl::SendVsrInMplFormat - Supplied number of entries [" << srcVsr.num_vsrs_params <<
				"] exceeds maximum allowed, lowering to [" << VSR_MAX_ENTRIES << "]";

		PTRACE(eLevelError, log.GetString());
		cmVsrInfo.numberOfEntries = VSR_MAX_ENTRIES;
	}

	//============================
	// Filling VSR media entries
	//============================
	for (DWORD entryNdx = 0; entryNdx < cmVsrInfo.numberOfEntries; ++entryNdx)
	{
		const ST_VSR_PARAMS& srcVsrEntry	= srcVsr.st_vsrs_params[entryNdx];
		TCmRtcpVsrEntry& cmVsrEntry 		= cmVsrInfo.VSREntry[entryNdx];

		cmVsrEntry.payloadType 			= srcVsrEntry.payload_type;
		cmVsrEntry.aspectRatioBitMask	= SipAspectToVsrAspect(srcVsrEntry.aspect_ratio);
		cmVsrEntry.maxWidth				= srcVsrEntry.max_width;
		cmVsrEntry.maxHeight			= srcVsrEntry.max_height;
		cmVsrEntry.frameRateBitmask		= SipFRToVsrFR(srcVsrEntry.frame_rate);
		cmVsrEntry.minBitRate			= srcVsrEntry.min_bitrate;

		for (int j=0; j<VSR_NUM_OF_QUALITY_LEVELS; ++j)
			cmVsrEntry.qualityReportHistogram[j] = srcVsrEntry.qualityReportHistogram[j];
		
		//==========================
		// Adding calculated data
		//==========================
		InterpolateEntryData(cmVsrEntry, cmVsrInfo.numberOfEntries);
	}


	//======================
	// Sending VSR message
	//======================
	CMedString log;
	log << 	"CSipVsrCtrl::SendVsrInMplFormat - Sending VSR for SSRC[" << cmVsrInfo.senderSSRC << "], MSI[" << cmVsrInfo.MSI << "], keyFrame[" << cmVsrInfo.keyFrame
						<< "], requestId[" << cmVsrInfo.requestId << "], number of entries to be sent:" << cmVsrInfo.numberOfEntries;
	PTRACE(eLevelInfoNormal, log.GetString());

	SendMsgToMpl(reinterpret_cast<BYTE*>(&cmVsrMsg), sizeof(cmVsrMsg), IP_CM_RTCP_VSR_REQ);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipVsrCtrl::SendMsSvcInitMsg(BYTE* msg, int size, DWORD opcode) const
{
	BOOL ret = FALSE;

	if (m_state != IDLE && (opcode == CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ || opcode == CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ))
	{
		CSmallString log;
		log << 	"CSipVsrCtrl::SendMsSvcInitMsg - Sending message for opcode [" << opcode << "]";
		PTRACE(eLevelInfoNormal, log.GetString());

		SendMsgToMpl(msg, size, opcode);
		BOOL ret = TRUE;
	}
	//=============================
	// Incorrect usage conditions
	//=============================
	else
	{
		CMedString log;
		log << "CSipVsrCtrl::SendMsSvcInitMsg - Not sending, combination of state[" << m_state << "] and opcode[" << opcode << "] is illegal for an init message";
		PTRACE(eLevelInfoNormal,log.GetString());
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::SendVsrToParty(const TCmRtcpVsrInfo& vsrInfo)
{
	//=============================================
	// Converting embedded values to party domain
	//=============================================
	ST_VSR_SINGLE_STREAM sipCntlVsr;

	sipCntlVsr.key_frame		= vsrInfo.keyFrame;
	sipCntlVsr.msi				= vsrInfo.MSI;
	sipCntlVsr.partyId			= GetPartyId(vsrInfo.senderSSRC);
	sipCntlVsr.sender_ssrc		= vsrInfo.senderSSRC;

	//================================
	// Determining number of entries
	//================================
	if (vsrInfo.numberOfEntries <= VSR_MAX_ENTRIES)
	{
		sipCntlVsr.num_vsrs_params = vsrInfo.numberOfEntries;
	}
	else
	{
		CMedString log;
		log << "CSipVsrCtrl::SendVsrToParty - Supplied number of entries [" << vsrInfo.numberOfEntries <<
				"] exceeds maximum allowed, lowering to [" << VSR_MAX_ENTRIES << "]";

		PTRACE(eLevelError, log.GetString());
		sipCntlVsr.num_vsrs_params = VSR_MAX_ENTRIES;
	}

	//=====================================
	// Converting entries to party domain
	//=====================================
	for(DWORD entryNdx = 0; entryNdx < sipCntlVsr.num_vsrs_params; ++entryNdx)
	{
		const TCmRtcpVsrEntry&	cmRtcpVsrEntry	= vsrInfo.VSREntry[entryNdx];
		ST_VSR_PARAMS&			sipCntlVsrEntry	= sipCntlVsr.st_vsrs_params[entryNdx];

		sipCntlVsrEntry.aspect_ratio	= VsrAspectToSipAspect(cmRtcpVsrEntry.aspectRatioBitMask);
		sipCntlVsrEntry.min_bitrate		= cmRtcpVsrEntry.minBitRate;
		sipCntlVsrEntry.frame_rate		= VsrFRToSipFR(cmRtcpVsrEntry.frameRateBitmask);
		sipCntlVsrEntry.max_height		= cmRtcpVsrEntry.maxHeight;
		sipCntlVsrEntry.max_width		= cmRtcpVsrEntry.maxWidth;
		sipCntlVsrEntry.payload_type	= cmRtcpVsrEntry.payloadType;
		sipCntlVsrEntry.maxNumOfPixels	= cmRtcpVsrEntry.maximumNumberOfPixels;
		sipCntlVsrEntry.num_of_may		= cmRtcpVsrEntry.numberOfMayInstances;
		sipCntlVsrEntry.num_of_must		= cmRtcpVsrEntry.numberOfMustInstances;

		for (int j=0; j<VSR_NUM_OF_QUALITY_LEVELS; ++j)
			sipCntlVsrEntry.qualityReportHistogram[j] = cmRtcpVsrEntry.qualityReportHistogram[j];
	}

	//==================
	// Sending message
	//==================
	CMedString log;
	log << 	"CSipVsrCtrl::SendVsrToParty - Received VSR for SSRC[" << sipCntlVsr.sender_ssrc << "], MSI[" << sipCntlVsr.msi << "], keyFrame[" << sipCntlVsr.key_frame <<
						"], partyId[" << sipCntlVsr.partyId << "], number of entries received:" << sipCntlVsr.num_vsrs_params;
	PTRACE(eLevelInfoNormal, log.GetString());
	if (m_pPartyApi) m_pPartyApi -> SendVsrMsgIndToSipParty(&sipCntlVsr);
}


//////////////////////////////////////////////////////////////////////////////////
// Note that "dummy" fields in the left VSR will be forced to equal right ones
BOOL CSipVsrCtrl::CompareEmbVsrs(TCmRtcpVsrInfo& lVsr, const TCmRtcpVsrInfo& rVsr)
{
	BOOL ret = FALSE;

	if (lVsr.numberOfEntries == rVsr.numberOfEntries)
	{
		if (rVsr.numberOfEntries <= VSR_MAX_ENTRIES)
		{
			//=================================================================
			// Preparing to compare VSR-s: Ignoring dummy fields and unfilled
			// (much faster than full-struct memcmp)
			//=================================================================
			int sizeToCompare = sizeof(TCmRtcpVsrInfo);

			if (VSR_MAX_ENTRIES > rVsr.numberOfEntries)  //KW
			{
				sizeToCompare -= (VSR_MAX_ENTRIES - rVsr.numberOfEntries) * sizeof(TCmRtcpVsrEntry);
			}
			for (int i = 0; i < rVsr.numberOfEntries; ++i)
			{
				// Dummy padding is not real info that needs to be presereved, so overwriting it for comparison
				//===============================================================================================
				lVsr.VSREntry[i].dummyPadding = rVsr.VSREntry[i].dummyPadding;
			}

			//==================
			// Comparing VSR-s
			//==================
			if (memcmp(&lVsr, &rVsr, sizeToCompare))
			{
				PTRACE(eLevelInfoNormal, "CSipVsrCtrl::CompareEmbVsrs - VSR content differs");
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CSipVsrCtrl::CompareEmbVsrs - VSR content identical");
				ret = TRUE;
			}
		}
		else
		{
			PTRACE2INT(eLevelError, "CSipVsrCtrl::CompareEmbVsrs - VSR number of entries exceeds maximum: ",
									rVsr.numberOfEntries);
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::CompareEmbVsrs - VSR entries number differs");
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////
CSipVsrCtrl::RxVsrPolicy_t CSipVsrCtrl::RxVsrPolicy(const TCmRtcpVsrInfo& newVsr)
{
	RxVsrPolicy_t policy = eIgnoreRxVsr;

	if (IsFirstRxVsr())
	{
		//==============================================
		// First received VSR will always be forwarded
		//==============================================
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::RxVsrPolicy - received first VSR, sending to party unconditionally");
		policy = eSendVsrToParty;
	}
	else if (newVsr.keyFrame != m_lastRxVsr.keyFrame)
	{
		if (newVsr.keyFrame)
		{
			//==========================================================================================
			// If key frame has changed to positive, handling policy will be at least to send an intra
			//==========================================================================================
			PTRACE(eLevelInfoNormal, "CSipVsrCtrl::RxVsrPolicy - key-frame field is toggled to enabled");
			policy = eRaiseIntraReq;
		}

		//==============================================================
		// Comparing old and new VSR-s, ignoring key-frame differences
		//==============================================================
		m_lastRxVsr.keyFrame = newVsr.keyFrame;
		if (!CompareEmbVsrs(m_lastRxVsr, newVsr))
		{
			PTRACE(eLevelInfoNormal, "CSipVsrCtrl::RxVsrPolicy - video description changed");
			policy = eSendVsrToParty;
		}
	}
	else
	{
		//===================================================================================
		// Embedded doesn't resend identical VSR-s, hence if the key-frame was not changed,
		// then it is definitely something else that changed in the VSR
		//===================================================================================
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::RxVsrPolicy - key-frame has not been changed, VSR change is therefore in video description");
		policy = eSendVsrToParty;
	}

	m_lastRxVsr = newVsr;

	PTRACE2INT(eLevelInfoNormal, "CSipVsrCtrl::RxVsrPolicy - VSR receipt concluded in policy number ", policy);

	return policy;
}

////////////////////////////////////////////////////////////////////////////
void CSipVsrCtrl::OnIpCmVsrMessageInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "LYNC2013_FEC_RED: CSipVsrCtrl::OnIpCmVsrMessageInd");

	DBGPASSERT_AND_RETURN(!pParam);

	TCmRtcpVsrMsg cmVsrMsg;
	memset(&cmVsrMsg,0,sizeof(TCmRtcpVsrMsg));
	pParam->Get(reinterpret_cast<BYTE*>(&cmVsrMsg), sizeof(TCmRtcpVsrMsg));
	TCmRtcpVsrInfo& vsrInfo = cmVsrMsg.cmRtcpVsrInfo;

	//===============================================
	// Determining what to do with the received VSR
	//===============================================
	RxVsrPolicy_t vsrPolicy = RxVsrPolicy(vsrInfo);
	switch(vsrPolicy)
	{
	case eSendVsrToParty:
	{
		//================
		// Full VSR sent
		//================
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::OnIpCmVsrMessageInd - RxVsrPolicy determined that the received VSR should be forwarded to the party");
		SendVsrToParty(vsrInfo);
		break;
	}

	case eRaiseIntraReq:
	{
		//====================================
		// Generating a remote intra request
		//====================================
		CSegment seg;
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::OnIpCmVsrMessageInd - RxVsrPolicy determined that the received VSR nets as an Intra request");
		seg << (WORD)Fast_Update << (WORD)kRolePeople << (WORD)eSyncLost << FALSE /*bIsGradualIntra*/ ;
		if (m_pPartyApi) m_pPartyApi->IpRmtH230(&seg);
		break;
	}

	case eIgnoreRxVsr:
	{
		PTRACE(eLevelInfoNormal, "CSipVsrCtrl::OnIpCmVsrMessageInd - RxVsrPolicy determined that nothing should be done for the received VSR");
		break;
	}

	default:
		PTRACE2INT(eLevelInfoNormal, "CSipVsrCtrl::OnIpCmVsrMessageInd - unexpected result from RxVsrPolicy: ", vsrPolicy);
	}
}


//////////////////////////////////////
void CSipVsrCtrl::TriggerRcvVsr()
{
	if (!IsFirstRxVsr())
	{
		//=======================================================================================
		// Resending VSR, first was probably lost due to a pending transaction in the SIP party
		//=======================================================================================
		SendVsrToParty(m_lastRxVsr);
	}
	else
	{
		//=======================================================================================
		// Resending VSR, first was probably lost due to a pending transaction in the SIP party
		//=======================================================================================
		PTRACE(eLevelError, "CSipVsrCtrl::TriggerRcvVsr - no VSR stored!");
	}
}

//////////////////////////////////////////////////////////////////
ST_VSR_SINGLE_STREAM* CSipVsrCtrlAvMcu::GetLastTxVsr(DWORD SSRC, DWORD& slotNum)
{
	ST_VSR_SINGLE_STREAM* ret = NULL;

	for (slotNum = 0; slotNum < min(m_lastTxVsr.num_vsrs_streams, (DWORD) MAX_STREAM_LYNC_2013_CONN) && NULL == ret; ++slotNum)
	{
		ST_VSR_SINGLE_STREAM* pVsr = &(m_lastTxVsr.st_vsrs_single_stream[slotNum]);
		if (pVsr->num_vsrs_params != (DWORD) NUMERIC_NULL && pVsr->sender_ssrc == SSRC)
		{
			ret = pVsr;
			break;
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////
DWORD CSipVsrCtrlAvMcu::GetPartyId(DWORD SSRC) const
{
	DWORD partyId = (DWORD) NUMERIC_NULL;
	for (int i = 0; i < MAX_STREAM_DMUX_LYNC_CONN && (DWORD) NUMERIC_NULL == partyId; ++i)
	{
		const SingleStreamDesc& streamDesc = m_dmuxMsg.rxConnectedParties[i];
		if (SSRC >= streamDesc.localSSRCRange[0] && SSRC <= streamDesc.localSSRCRange[1])
		{
			partyId = streamDesc.unPartyId;
		}
	}
	return partyId;
}
