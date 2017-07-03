
#include "DataTypes.h"
#include "Segment.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "ConfApi.h"
#include "IpScm.h"
#include "Party.h"
#include "IPParty.h"

//Party monitoring values
#ifndef NUM_OF_PM_REQ_IN_SEC
#define NUM_OF_PM_REQ_IN_SEC 10
#endif



PBEGIN_MESSAGE_MAP(CIpParty)
  
  ONEVENT(IP_STREAM_VIOLATION,				PARTYCONNECTED, 	CIpParty::OnPartyStreamViolation)

  // Bridge events
  ONEVENT(VIDVALID,							ANYCASE, 			CIpParty::OnVidBrdgValidation)
  ONEVENT(AUDVALID,							ANYCASE, 			CIpParty::OnAudBrdgValidation)

PEND_MESSAGE_MAP(CIpParty,CParty);


///////////////////////////////////////////////////////////////////////////
CIpParty::CIpParty(CIpComMode* pCurrentMode, CIpComMode* pTargetMode)
{
	VALIDATEMESSAGEMAP

	m_pTargetMode = pTargetMode;
	m_pCurrentMode = pCurrentMode;

 	m_bIsPreviewVideoOut = FALSE;
  	m_bIsPreviewVideoIn	 = FALSE;

  	m_RcvPreviewReqParams = NULL;
  	m_TxPreviewReqParams=NULL;
}

///////////////////////////////////////////////////////////////////////////
CIpParty::~CIpParty()
{
	POBJDELETE(m_pCurrentMode);
	POBJDELETE(m_pTargetMode);

	POBJDELETE(m_RcvPreviewReqParams);
	POBJDELETE(m_TxPreviewReqParams);
}

////////////////////////////////////////////////////////////
void CIpParty::Create(CSegment& appParam)
{
	CParty::Create(appParam);
	
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void  CIpParty::InitTask() 
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();	
    DWORD partyTotalConnectionTimer = 0;
    pSysConfig->GetDWORDDataByKey("IP_PARTY_CONN_TIME_TILL_SIGNALING", partyTotalConnectionTimer);

    StartTimer(PARTYCONTOUT, partyTotalConnectionTimer*SECOND);
}	
	
////////////////////////////////////////////////////////////
void CIpParty::OnIpLogicalChannelConnect(CSegment* pParam)
{	
	CPrtMontrBaseParams *pPrtMonitrParams = NULL;

	DWORD  vendorType;
	DWORD  channelType;
	*pParam >> vendorType >> channelType;

	pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
	if (pPrtMonitrParams)
		pPrtMonitrParams->DeSerialize(NATIVE,*pParam);
	LogicalChannelConnect(pPrtMonitrParams,channelType,vendorType);
	POBJDELETE(pPrtMonitrParams);
}

///////////////////////////////////////////////////////////////////////////
void CIpParty::LogicalChannelUpdate(DWORD channelType, DWORD vendorType)
{
	CSegment* pSeg = new CSegment;  

	//VNGFE-6008
	*pSeg << m_pParty->GetName() << vendorType << channelType;

	SerializeNetSetup(channelType,pSeg);

	//VNGR-24921
	m_pConfApi->UpdateDB(NULL, IPLOGICALCHANNELUPDATE, (DWORD)0, 1, pSeg);

	POBJDELETE(pSeg);	
}


////////////////////////////////////////////////////////////////////////////
void CIpParty::LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType)
{
	CSegment* pSeg = new CSegment;  

	*pSeg << vendorType << channelType;

	if (pPrtMonitor)
		pPrtMonitor->Serialize(NATIVE,*pSeg);

	if((EIpChannelType)channelType == SIGNALING)
		LogicalChannelUpdate(channelType,vendorType);

	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELCONNECT,(DWORD) 0,1,pSeg);

	POBJDELETE(pSeg);	
}

////////////////////////////////////////////////////////////////////////////
void CIpParty::OnIpLogicalChannelDisconnect(CSegment* pParam)
{
	DWORD  eChannelType;
	*pParam >> eChannelType;
	LogicalChannelDisconnect(eChannelType);
}

////////////////////////////////////////////////////////////////////////////
void CIpParty::LogicalChannelDisconnect(DWORD eChannelType)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << eChannelType;
	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELDISCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CIpParty::OnIpPartyMonitoring(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CIpParty::OnIpPartyMonitoring : Name - ",PARTYNAME);
	CSegment* pSeg = new CSegment;
	
	WORD channelType;
	BYTE intraSyncFlag;
	BYTE videoBCHSyncFlag;
	BYTE protocolSyncFlag;
	WORD bchOutOfSyncCount;
	WORD protocolOutOfSyncCount;
	
	*pParam 
		>> channelType
		>> intraSyncFlag
		>> videoBCHSyncFlag
		>> bchOutOfSyncCount
		>> protocolSyncFlag
		>> protocolOutOfSyncCount; 
	
	*pSeg 
		<< channelType
		<< intraSyncFlag
		<< videoBCHSyncFlag
		<< bchOutOfSyncCount
		<< protocolSyncFlag
		<< protocolOutOfSyncCount 
		<< (BYTE)1; //bShowMonitoring
	
	m_pConfApi->UpdateDB(this,IPPARTYMONITORING,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
}


////////////////////////////////////////////////////////////////////////////
void CIpParty::NullActionFunction(CSegment* pParam)
{
	CSmallString str;
	str << "Opcode " << m_log[m_current_log_entry].eventStr << " arrived at state " << m_state << "\n";
	PTRACE2(eLevelInfoNormal, "CIpParty::NullActionFunction: ---- ! ---- ",str.GetString());
}

////////////////////////////////////////////////////////////////////////////
void  CIpParty::OnVidBrdgValidation(CSegment* pParam)
{
  WORD onOff = 0;
  *pParam >> onOff;
  WORD value = 0;
  onOff == 1 ?  value = VIA : value = VIS;
  PTRACE2INT(eLevelInfoNormal,"CIpParty::OnVidBrdgValidation : %d", value); 
}
////////////////////////////////////////////////////////////////////////////
void CIpParty::OnAudBrdgValidation(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CIpParty::OnAudBrdgValidation : Name - ",PARTYNAME);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CIpParty::OnPartyStreamViolation(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CIpParty::OnPartyStreamViolation: Name ",m_partyConfName);
	WORD cardStatus;
	WORD reason;
	CSecondaryParams secParamps;
	*pParam >> cardStatus >> reason;
	secParamps.DeSerialize(NATIVE,*pParam);
	SetPartyToSecondary(reason,&secParamps); 
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CIpParty::SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate,
												mcTransportAddress* partyAdd,mcTransportAddress* mcuAdd,DWORD protocol,
												DWORD pmIndex,DWORD vendorType,BYTE IsIce,mcTransportAddress* IcePartyAdd,mcTransportAddress* IceMcuAdd,EIceConnectionType IceConnectionType)
{
	CCapSetInfo capInfo = (CapEnum)protocol;
	CPrtMontrBaseParams* pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
	SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,rate,partyAdd,mcuAdd,(DWORD)capInfo.GetIpCapCode(),pmIndex,IsIce,IcePartyAdd,IceMcuAdd,IceConnectionType);
	LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType,vendorType);				
	POBJDELETE(pPrtMonitrParams);				
}

