//+========================================================================+
//                       PartyConnection.CPP                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PartyConnection.CPP                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 16/12/96   |                                                      |
// Talya| 18.5.05   | Inserted to Carmel                                   |
//+========================================================================+

#include "NStream.h"
#include "PartyConnection.h"
#include "AddIsdnVoicePartyCntl.h"
#include "ConnectedIsdnVoicePartyCntl.h"
#include "DelIsdnVoicePartyCntl.h"
#include "AddIsdnPartyCntl.h"
#include "DelIsdnPartyCntl.h"
#include "IsdnChangeModePartyCntl.h"
#include "AudioBridgeInterface.h"
#include "H323AddPartyControl.h"
#include "H323ChangeModePartyControl.h"
#include "H323DelPartyControl.h"
#include "H323ExportPartyCntl.h"
#include "H323ImportPartyCntl.h"
#include "ExportIsdnVoicePartyCntl.h"
#include "ImportIsdnVoicePartyCntl.h"
#include "ExportIsdnPartyCntl.h"
#include "ImportIsdnPartyCntl.h"
//SIP includes
#include "DataTypes.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SIPPartyControl.h"
#include "SIPPartyControlAdd.h"
#include "SIPPartyControlChangeMode.h"
#include "SIPPartyControlDelete.h"
#include "SIPPartyControlExport.h"
#include "SIPPartyControlImport.h"
#include "SIPSlavePartyControl.h"
#include "SIPSlavePartyControlChangeMode.h"
#include "SIPPluginPartyControlAdd.h"
#include "SIPPartyControlChangeMode.h"
#include "SIPPluginPartyControlChangeMode.h"
#include "MSSlaveSipPartyCntlAdd.h"
#include "MSSlaveChangeModeSipPartyCntl.h"

#include "SysConfig.h"
#include "SysConfigKeys.h"

//#ifdef CONTENT_LOAD
#include "Stopper.h"
//#endif

#define LOGINFO "PartyId:" << GetPartyRsrcId() << ", PartyName:" << GetName()

PBEGIN_MESSAGE_MAP(CPartyConnection)
	ONEVENT(ADDASSIPCONTENTTOUT, ANYCASE, CPartyConnection::AddAsSipContenttout)
PEND_MESSAGE_MAP(CPartyConnection,CStateMachine);


/////////////////////////////////////////////////////////////////////////////
CPartyConnection::CPartyConnection()
{
	m_pPartyCntl = NULL;

	VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection::~CPartyConnection()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::Destroy()
{
	DeleteAllTimers();
	if (m_pPartyCntl)
	{
		m_pPartyCntl->Destroy();
		POBJDELETE(m_pPartyCntl);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}
/*
/////////////////////////////////////////////////////////////////////////////
WORD operator==(const CPartyConnection& first, const CPartyConnection& second)
{
	WORD rc = 0;

	if (first.m_pPartyCntl)       rc = 1;
	if (second.m_pPartyCntl)      rc += 2;

	if (first.m_name[0] != '\0')  rc += 4;
	if (second.m_name[0] != '\0') rc += 9;

	if (first.m_partyId != 0)     rc += 17;
	if (second.m_partyId != 0)    rc += 34;

	switch (rc)
	{
		case 1:
			return (first.m_pPartyCntl->GetPartyTaskApp() == second.m_pParty) ? 1 : 0;

		case 2:
			return (second.m_pPartyCntl->GetPartyTaskApp() == first.m_pParty) ? 1 : 0;

		case 3:
		case 7:
			return (first.m_pPartyCntl->GetPartyTaskApp() == second.m_pPartyCntl->GetPartyTaskApp()) ? 1 : 0;

		case 6:
			return (!strcmp(first.m_name, second.m_pPartyCntl->GetName())) ? 1 : 0;

		case 10:
			return (!strcmp(second.m_name, first.m_pPartyCntl->GetName())) ? 1 : 0;

		case 19:
			return (first.m_partyId == second.m_pPartyCntl->GetPartyRsrcId()) ? 1 : 0;

		case 35:
			return (first.m_pPartyCntl->GetPartyRsrcId() == second.m_partyId) ? 1 : 0;
	}

	return 0;
}
*/
/////////////////////////////////////////////////////////////////////////////
//
//                          C O N N E C T
//
/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ConnectPSTN(
		CConf* pConf,
		CNetSetup* pNetSetUp,
		COsQueue* pConfRcvMbx,
		CAudioBridgeInterface* pAudioBridgeInterface,
		CConfAppMngrInterface* pConfAppMngrInterface,
		COsQueue* pPartyRcvMbx,
		CTaskApp* pParty,
		WORD termNum,
		const char* telNum,
		WORD type,
		const char* partyName,
		const char* confName,
		const char* password,
		ConfMonitorID monitorConfId,
		PartyMonitorID monitorPartyId,
		ENetworkType networkType,
		BYTE voice,
		BYTE audioVolume,
		const char *service_provider,
		WORD stby,
		WORD connectDelay,
		const char *AV_service_name,
		WORD welcome_msg_time,
		BYTE IsRecording)
{
	CIsdnNetSetup* pIsdnNetSetUp = dynamic_cast<CIsdnNetSetup *>(pNetSetUp);
	PASSERT_AND_RETURN(!pIsdnNetSetUp);

	m_pPartyCntl = new CAddIsdnVoicePartyCntl;

	// Party is recognizing according to NetInterfaceType : ISDN_INTERFACE_TYPE or ATM_INTERFACE_TYPE
	m_pPartyCntl->SetInterfaceType(ISDN_INTERFACE_TYPE);  // if T1-CAS then will be overwrite within the Create function

	((CAddIsdnVoicePartyCntl*)m_pPartyCntl)->Create(
		pConf,
		pIsdnNetSetUp,
		pConfRcvMbx,
		pAudioBridgeInterface,
		pConfAppMngrInterface,
		pPartyRcvMbx,
		pParty,
		termNum,
		telNum,
		type,
		partyName,
		confName,
		password,
		monitorConfId,
		monitorPartyId,
		networkType,
		voice,
		audioVolume,
		service_provider,
		stby,
		connectDelay,
		AV_service_name,
		welcome_msg_time,
		IsRecording);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ConnectIsdn(
		CConf* pConf,
		CIsdnNetSetup* pNetSetUp,
		CCapH320* pCap,
		CComMode* pScm,
		CComMode* pTransmitScm,
		COsQueue* pConfRcvMbx,
		CAudioBridgeInterface* pAudioBridgeInterface,
		CVideoBridgeInterface* pVideoBridgeInterface,
		CConfAppMngrInterface* pConfAppMngrInterface,
		CFECCBridge* pFECCBridge,
		CContentBridge* pContentBridge,
		CTerminalNumberingManager* pTerminalNumberingManager,
		COsQueue* pPartyRcvMbx,
		CTaskApp* pParty,
		WORD termNum,
		BYTE chnlWidth,
		WORD numChnl,
		WORD type,
		const char* partyName,
		const char* confName,
		ConfMonitorID monitorConfId,
		PartyMonitorID monitorPartyId,
		const char* serviceName,
		ENetworkType networkType,
		WORD nodeType,
		BYTE voice,
		WORD stby,
		DWORD connectDelay,
		eTelePresencePartyType eTelePresenceMode,
		eSubCPtype bySubCPtype,
		WORD isUndefParty)
{
	CIsdnNetSetup* pIsdnNetSetUp = dynamic_cast<CIsdnNetSetup *>(pNetSetUp);
	PASSERT_AND_RETURN(!pIsdnNetSetUp);

	m_pPartyCntl = new CAddIsdnPartyCntl;

	// Party is recognizing according to NetInterfaceType : ISDN_INTERFACE_TYPE or ATM_INTERFACE_TYPE
	m_pPartyCntl->SetInterfaceType(ISDN_INTERFACE_TYPE);  // if T1-CAS then will be overwrite within the Create function
	((CAddIsdnPartyCntl*)m_pPartyCntl)->Create(
		pConf,
		pIsdnNetSetUp,
		pCap, pScm,
		pTransmitScm,
		pConfRcvMbx,
		pAudioBridgeInterface,
		pVideoBridgeInterface,
		pConfAppMngrInterface,
		pFECCBridge,
		pContentBridge,
		pTerminalNumberingManager,
		pPartyRcvMbx,
		pParty,
		termNum,
		chnlWidth,
		numChnl,
		type,
		partyName,
		confName,
		monitorConfId,
		monitorPartyId,
		serviceName,
		networkType,
		nodeType,
		voice,
		stby,
		connectDelay,
		eTelePresenceMode);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ConnectIP(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	BYTE interfaceType = partyControInitParam.pConfParty->GetNetInterfaceType();
	BYTE bIsH323 = (interfaceType == H323_INTERFACE_TYPE);

	if (bIsH323)
	{
		m_pPartyCntl = new CH323AddPartyCntl;
		((CH323AddPartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	}
	else if (partyControlDataParams.lyncEpType == Lync_Addon)
	{
		m_pPartyCntl = new CSipPluginAddPartyCntl;
		((CSipPluginAddPartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	}
	else if (partyControlDataParams.tipPartyType != eTipNone)
	{
		m_pPartyCntl = new CSipSlavePartyCntl;
		((CSipSlavePartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	}
	else if (partyControlDataParams.lyncEpType == AvMcuLync2013Slave)
	{
		m_pPartyCntl = new CMSSlaveSipAddPartyCntl;
		((CMSSlaveSipAddPartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	}
	else
	{
		m_pPartyCntl = new CSipAddPartyCntl;
		((CSipAddPartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ConnectAVMCUIP(CSipNetSetup* pNetSetup, PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams, sipSdpAndHeaders* pSdpAndHeaders)
{
	m_pPartyCntl = new CSipAddPartyCntl;
	m_pPartyCntl->SetAvMcuLinkType(eAvMcuLinkMain);

	((CSipAddPartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	((CSipAddPartyCntl*)m_pPartyCntl)->SetMsConfInviteReq(pSdpAndHeaders, pNetSetup);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ConnectDMAAVMCUIP(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams, const char* FocusUri)
{
	m_pPartyCntl = new CSipAddPartyCntl;
	m_pPartyCntl->SetAvMcuLinkType(eAvMcuLinkMain);
	((CSipAddPartyCntl*)m_pPartyCntl)->Create(partyControInitParam, partyControlDataParams);
	((CSipAddPartyCntl*)m_pPartyCntl)->SetFocusUri();

}

/////////////////////////////////////////////////////////////////////////////
//
//                          R E C O N N E C T
//
/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ReconnectPstn(const char* confName, const char* password, COsQueue* pConfRcvMbx, WORD termNum, WORD WelcomMode, BYTE isRecording, WORD redialInterval)
{
	CAddIsdnVoicePartyCntl* pReconnectPartyCntl = new CAddIsdnVoicePartyCntl;
	*(CIsdnPartyCntl*)(pReconnectPartyCntl) = *(dynamic_cast<CIsdnPartyCntl *>(m_pPartyCntl));
	POBJDELETE(m_pPartyCntl);
	m_pPartyCntl = pReconnectPartyCntl;
	((CAddIsdnVoicePartyCntl*)m_pPartyCntl)->Reconnect(confName, password, pConfRcvMbx, termNum, WelcomMode, isRecording, redialInterval);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ReconnectIsdn(const char* confName, CCapH320* pCap, CComMode* pScm, CComMode* pTransmitScm, COsQueue* pConfRcvMbx, WORD termNum, BYTE isRecording, WORD redialInterval, WORD connectDelay)
{
	CAddIsdnPartyCntl* pReconnectPartyCntl = new CAddIsdnPartyCntl;
	*(CIsdnPartyCntl*)(pReconnectPartyCntl) = *(dynamic_cast<CIsdnPartyCntl*>(m_pPartyCntl));
	POBJDELETE(m_pPartyCntl);
	m_pPartyCntl = pReconnectPartyCntl;
	((CAddIsdnPartyCntl*)m_pPartyCntl)->Reconnect(confName, pCap, pScm, pTransmitScm, pConfRcvMbx, termNum, isRecording, redialInterval, connectDelay);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ReconnectH323(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	CH323AddPartyCntl* pReconnectPartyCntl = new CH323AddPartyCntl;
	*pReconnectPartyCntl = (CH323PartyCntl&)*m_pPartyCntl;
	POBJDELETE(m_pPartyCntl);
	m_pPartyCntl = pReconnectPartyCntl;
	((CH323AddPartyCntl*)m_pPartyCntl)->Reconnect(partyControInitParam, partyControlDataParams);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ReconnectSip(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters &partyControlDataParams)
{
	CSipAddPartyCntl* pReconnectPartyCntl = new CSipAddPartyCntl;
	m_pPartyCntl->Destroy();
	*pReconnectPartyCntl = (CSipAddPartyCntl&)*m_pPartyCntl;
	POBJDELETE(m_pPartyCntl);
	m_pPartyCntl = pReconnectPartyCntl;
	((CSipAddPartyCntl*)m_pPartyCntl)->Reconnect(partyControInitParam, partyControlDataParams);
}


/////////////////////////////////////////////////////////////////////////////
//
//                          C H A N G E   S C M
//
/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ChangePSTNScm()
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CDelIsdnVoicePartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	// bug fix - move failed and party stack in source conf.
	if (!strcmp(m_pPartyCntl->NameOf(), "CExportIsdnVoicePartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while export";
		return;
	}

	TRACEINTO << LOGINFO;

	CConnectedIsdnVoicePartyCntl* pConnectedPartyCntl = new CConnectedIsdnVoicePartyCntl;
	*((CIsdnPartyCntl*)pConnectedPartyCntl) = *(dynamic_cast<CIsdnPartyCntl*>(m_pPartyCntl));
	POBJDELETE(m_pPartyCntl);
	m_pPartyCntl = pConnectedPartyCntl;
	((CConnectedIsdnVoicePartyCntl*)m_pPartyCntl)->ChangeScm();
}

/////////////////////////////////////////////////////////////////////////
void CPartyConnection::ChangeIsdnScm(CComMode* pTargetTransmitScm, CComMode* pTargetReceiveScm)
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CDelIsdnPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	// bug fix - move failed and party stack in source conf.
	if (!strcmp(m_pPartyCntl->NameOf(), "CExportIsdnPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while export";
		return;
	}

	TRACEINTO << LOGINFO;

	if (!strcmp(m_pPartyCntl->NameOf(), "CIsdnChangeModePartyCntl")) // party in state of change mode
		((CIsdnChangeModeCntl*)m_pPartyCntl)->ChangeScm(pTargetTransmitScm, pTargetReceiveScm);
	else if (!m_pPartyCntl->IsFullBitRateConnected()) // party not finished add
		PTRACE(eLevelInfoNormal, "CPartyConnection::ChangeIsdnScm PARTY NOT FINISH ADD - DO NOTHING!");
	else
	{    // party finished add state
		CIsdnChangeModeCntl* pIsdnChangeModePartyCntl = new CIsdnChangeModeCntl();
		(CIsdnChangeModeCntl&)*pIsdnChangeModePartyCntl = (CIsdnPartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pIsdnChangeModePartyCntl;
		((CIsdnChangeModeCntl*)m_pPartyCntl)->ChangeScm(pTargetTransmitScm, pTargetReceiveScm);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ChangeH323Scm(CComModeH323* pH323Scm, EChangeMediaType eChangeMediaType)
{
//#ifdef CONTENT_LOAD
	CStopper _stopper("CPartyConnection::ChangeH323Scm", "Duration");
//#endif

	if (!strcmp(m_pPartyCntl->NameOf(), "CH323DelPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	// added in order to prevent the lost of the PARTYEXPORT message (end export on source conference)
	// that causes the conf not to reset the m_isMoveInProgress flag and block all moves (ron)
	if (!strcmp(m_pPartyCntl->NameOf(), "CH323ExportPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while export";
		return;
	}

	TRACEINTO << LOGINFO;

	if (!strcmp(m_pPartyCntl->NameOf(), "CH323ChangeModePartyCntl") || !strcmp(m_pPartyCntl->GetRTType(), "CH323ChangeModeCntl"))
		// H323 party in state of change mode
		((CH323ChangeModeCntl*)m_pPartyCntl)->ChangeScm(pH323Scm, eChangeMediaType);
	else
	{
		if (!m_pPartyCntl->IsFullBitRateConnected() && !strcmp(m_pPartyCntl->NameOf(), "CH323AddPartyCntl")) // party not finished add
		{
			((CH323AddPartyCntl*)m_pPartyCntl)->ChangeScm(pH323Scm, eChangeMediaType);
		}
		else
		{
			// party finished add state, addition for G&G
			CH323ChangeModeCntl* pChangeModePartyCntl = new CH323ChangeModeCntl;
			*pChangeModePartyCntl = (CH323PartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pChangeModePartyCntl;
			((CH323ChangeModeCntl*)m_pPartyCntl)->ChangeScm(pH323Scm, eChangeMediaType);
		}
	}
//#ifdef CONTENT_LOAD
	_stopper.Stop();
//#endif
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ChangeSipScm(CIpComMode* pScm, BYTE IsAsSipContentEnable)
{
//#ifdef CONTENT_LOAD
	CStopper _stopper("CPartyConnection::ChangeH323Scm", "Duration");
//#endif

	if (!strcmp(m_pPartyCntl->NameOf(), "CSipDelPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	if (!strcmp(m_pPartyCntl->NameOf(), "CSipExportPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while export";
		return;
	}

	if (!strcmp(m_pPartyCntl->NameOf(), "CSipChangeModePartyCntl") ||
	    !strcmp(m_pPartyCntl->NameOf(), "CSipSlaveChangeModePartyCntl") ||
	    !strcmp(m_pPartyCntl->NameOf(), "CSipPluginChangeModePartyCntl") ||
	    !strcmp(m_pPartyCntl->NameOf(), "CSipChangeModeLyncPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Party in change SCM";

		((CSipChangeModePartyCntl*)m_pPartyCntl)->ChangeScm(pScm, IsAsSipContentEnable);
	}
	else if (m_pPartyCntl->IsFullBitRateConnected() == NO && strcmp(m_pPartyCntl->NameOf(), "CSipSlavePartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Party not finished add";

		((CSipAddPartyCntl*)m_pPartyCntl)->ChangeScm(pScm, IsAsSipContentEnable);
	}
	else if (!strcmp(m_pPartyCntl->NameOf(), "CSipSlavePartyCntl")) //Slave didn't finish connecting and receive change mode. ??
	{
		if (m_pPartyCntl->IsFullBitRateConnected() == NO) //slave party not finished add
		{
			TRACEINTO << LOGINFO << " - Slave party not finished add";

			((CSipSlavePartyCntl*)m_pPartyCntl)->ChangeScm(pScm);
		}
		else
		{
			TRACEINTO << LOGINFO << " - Move slave party to change SCM";

			CSipSlaveChangeModePartyCntl* pSlaveChangeModePartyCntl = new CSipSlaveChangeModePartyCntl;
			(CSipPartyCntl&)*pSlaveChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pSlaveChangeModePartyCntl;
			((CSipSlaveChangeModePartyCntl*)m_pPartyCntl)->ChangeScm(pScm, IsAsSipContentEnable);
		}
	}
	else if (!strcmp(m_pPartyCntl->NameOf(), "CSipPluginAddPartyCntl") ||
		!strcmp(m_pPartyCntl->NameOf(), "CSipPluginImportPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Move party to plug-in change SCM";

		CSipPluginChangeModePartyCntl* pChangeModePartyCntl = new CSipPluginChangeModePartyCntl;
		(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pChangeModePartyCntl;
		((CSipPluginChangeModePartyCntl*)m_pPartyCntl)->ChangeScm(pScm);
	}
	else if (!strcmp(m_pPartyCntl->NameOf(), "CMSSlaveSipAddPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Move MS slave party to MS slave party change SCM";

		CMSSlaveChangeModeSipPartyCntl* pChangeModePartyCntl = new CMSSlaveChangeModeSipPartyCntl;
		(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pChangeModePartyCntl;
		((CMSSlaveChangeModeSipPartyCntl*)m_pPartyCntl)->ChangeScm(pScm);
	}
	else if ((!strcmp(m_pPartyCntl->NameOf(), "CSipImportPartyCntl")) &&  (!((CSipImportPartyCntl*)m_pPartyCntl)->IsImportDone()) ) //BRIDGE-13498 - if the party is still in import, do not change mode
	{
		TRACEINTO << LOGINFO << "Not allowed while import";
	}
	else
	{
		if (m_pPartyCntl->GetIsTipCall() && m_pPartyCntl->IsTipSlavePartyType())
		{
			TRACEINTO << LOGINFO << " - Move slave party to change SCM";

			CSipSlaveChangeModePartyCntl* pChangeModePartyCntl = new CSipSlaveChangeModePartyCntl;
			(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pChangeModePartyCntl;
			((CSipSlaveChangeModePartyCntl*)m_pPartyCntl)->ChangeScm(pScm, IsAsSipContentEnable);
		}
		else
		{
			TRACEINTO << LOGINFO << " - Move party to change SCM";

			if (m_pPartyCntl->IsLync())
			{
				CSipChangeModeLyncPartyCntl* pChangeModePartyCntl = new CSipChangeModeLyncPartyCntl;
				(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
				POBJDELETE(m_pPartyCntl);
				m_pPartyCntl = pChangeModePartyCntl;
			}
			else
			{
				CSipChangeModePartyCntl* pChangeModePartyCntl = new CSipChangeModePartyCntl;
				(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
				POBJDELETE(m_pPartyCntl);
				m_pPartyCntl = pChangeModePartyCntl;
			}
			((CSipChangeModePartyCntl*)m_pPartyCntl)->ChangeScm(pScm, IsAsSipContentEnable);
		}
	}
//#ifdef CONTENT_LOAD
	_stopper.Dump();
//#endif
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::UpdateAVMCUPartyToStartMedia()
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CSipDelPartyCntl") || !strcmp(m_pPartyCntl->NameOf(), "CSipExportPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	if ((!strcmp(m_pPartyCntl->NameOf(), "CSipChangeModePartyCntl")) || 		//sip party in state of change mode
	    (!strcmp(m_pPartyCntl->NameOf(), "CSipSlaveChangeModePartyCntl")))
	{
		TRACEINTO << LOGINFO << " - Party in change SCM";

		((CSipChangeModePartyCntl*)m_pPartyCntl)->ActiveMedia();
	}
	else if ((m_pPartyCntl->IsFullBitRateConnected() == NO) && strcmp(m_pPartyCntl->NameOf(), "CSipSlavePartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Party not finished add";

		((CSipAddPartyCntl*)m_pPartyCntl)->ActiveMedia();
	}
	else
	{
		TRACEINTO << LOGINFO << " - Move party to change SCM";

		if (m_pPartyCntl->IsLync())
		{
			CSipChangeModeLyncPartyCntl* pChangeModePartyCntl = new CSipChangeModeLyncPartyCntl;
			(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pChangeModePartyCntl;
		}
		else
		{
			CSipChangeModePartyCntl* pChangeModePartyCntl = new CSipChangeModePartyCntl;
			(CSipPartyCntl&)*pChangeModePartyCntl = (const CSipPartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pChangeModePartyCntl;
		}
		((CSipChangeModePartyCntl*)m_pPartyCntl)->ActiveMedia();

	}

}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//                          D I S C O N N E C T
//
/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::DisconnectPstn(WORD mode, DWORD disconnectionDelay, BOOL isViolent, DWORD taskId)
{

	if (!strcmp(m_pPartyCntl->NameOf(), "CDelIsdnVoicePartyCntl"))
	{
		//CDelIsdnVoicePartyCntl already exists, do not create a new object
		//(that will cause double activation of the process and Timer handling problems)
		//Instead, just send the disconnect command to the existing object
		PTRACE(eLevelInfoNormal, "CPartyConnection::Disconnect called while disconnecting");
	}
	else
	{
		CDelIsdnVoicePartyCntl* pDelVoicePartyCntl = new CDelIsdnVoicePartyCntl;
		*((CIsdnPartyCntl*)(pDelVoicePartyCntl)) = *(dynamic_cast<CIsdnPartyCntl*>(m_pPartyCntl));
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pDelVoicePartyCntl;
	}

	if (isViolent)
	{
		((CDelIsdnVoicePartyCntl*)m_pPartyCntl)->SetIsViolentDestroy(true);
		((CDelIsdnVoicePartyCntl*)m_pPartyCntl)->SetPartyTaskId(taskId);
	}
	((CDelIsdnVoicePartyCntl*)m_pPartyCntl)->DisconnectPSTN(mode, disconnectionDelay);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::DisconnectIsdn(WORD mode, DWORD disconnectionDelay, BOOL isViolent, DWORD taskId)
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CDelIsdnPartyCntl"))
	{
		//CDelIsdnPartyCntl already exists, do not create a new object
		//(that will cause double activation of the process and Timer handling problems)
		//Instead, just send the disconnect command to the existing object
		PTRACE(eLevelInfoNormal, "CPartyConnection::Disconnect called while disconnecting");
	}
	else
	{
		CDelIsdnPartyCntl* pDelPartyCntl = new CDelIsdnPartyCntl;
		*((CIsdnPartyCntl*)(pDelPartyCntl)) = *(dynamic_cast<CIsdnPartyCntl*>(m_pPartyCntl));
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pDelPartyCntl;
	}

	if (isViolent)
	{
		((CDelIsdnPartyCntl*)m_pPartyCntl)->SetIsViolentDestroy(true);
		((CDelIsdnPartyCntl*)m_pPartyCntl)->SetPartyTaskId(taskId);
	}
	((CDelIsdnPartyCntl*)m_pPartyCntl)->DisconnectISDN(mode, disconnectionDelay);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::DisconnectH323(WORD mode, WORD isSetRedailToZero, DWORD disconnectionDelay, BOOL isViolent, DWORD taskId) //shiraITP - 125
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CH323DelPartyCntl"))
	{
		//CDelH323PartyCntl already exists, do not create a new object
		//(that will cause double activation of the process and timer handling problems).
		//Instead, just send the disconnect command to the existing object
		PTRACE(eLevelInfoNormal, "CPartyConnection::DisconnectH323 - called while disconnecting");
	}
	else
	{
		CH323DelPartyCntl* pH323DelPartyCntl = new CH323DelPartyCntl;
		*pH323DelPartyCntl = (CH323PartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pH323DelPartyCntl;
	}
	if (isSetRedailToZero)
		((CH323DelPartyCntl*)m_pPartyCntl)->SetRedailCounterToZero();

	if (isViolent)
	{
		((CH323DelPartyCntl*)m_pPartyCntl)->SetIsViolentDestroy(true);
		((CH323DelPartyCntl*)m_pPartyCntl)->SetPartyTaskId(taskId);
	}
	((CH323DelPartyCntl*)m_pPartyCntl)->DisconnectH323(mode, disconnectionDelay);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::DisconnectSip(WORD mode, WORD cause, const char* alternativeAddrStr, DWORD disconnectionDelay, BOOL isViolent, DWORD taskId)
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CSipDelPartyCntl"))
	{
		//CDelH323PartyCntl already exists, do not create a new object
		//(that will cause double activation of the process and timer handling problems).
		//Instead, just send the disconnect command to the existing object
		PTRACE(eLevelInfoNormal, "CPartyConnection::DisconnectSip - called while disconnecting");
	}
	else
	{
		CSipDelPartyCntl* pSipDelPartyCntl = new CSipDelPartyCntl;
		(CSipPartyCntl&)*pSipDelPartyCntl = (CSipPartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pSipDelPartyCntl;
	}
	if (isViolent)
	{
		((CSipDelPartyCntl*)m_pPartyCntl)->SetIsViolentDestroy(true);
		((CSipDelPartyCntl*)m_pPartyCntl)->SetPartyTaskId(taskId);
	}
	((CSipDelPartyCntl*)m_pPartyCntl)->Disconnect(mode, cause, alternativeAddrStr, disconnectionDelay);
}


/////////////////////////////////////////////////////////////////////////////
//
//                          E X P O R T
//
/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::Export(COsQueue* pDestRcvMbx, void* pConfParty, ConfMonitorID destConfId, PartyMonitorID destPartyId, EMoveType eCurMoveType)
{
	if (GetInterfaceType() == H323_INTERFACE_TYPE)
	{
		if (!strcmp(m_pPartyCntl->NameOf(), "CH323AddPartyCntl"))
		{
			((CH323PartyCntl*)m_pPartyCntl)->SetAddPartyStateBeforeMove(((CH323PartyCntl*)m_pPartyCntl)->GetState());
		}

		CH323ExportPartyCntl* pH323ExportPartyCntl = new CH323ExportPartyCntl;
		*pH323ExportPartyCntl = (CH323PartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pH323ExportPartyCntl;
		((CH323ExportPartyCntl*)m_pPartyCntl)->Transfer(pDestRcvMbx, pConfParty, destConfId, destPartyId, eCurMoveType);
	}
	else if (GetInterfaceType() == SIP_INTERFACE_TYPE)
	{
		CSipExportPartyCntl* pSipExportPartyCntl = new CSipExportPartyCntl;
		(CSipPartyCntl&)*pSipExportPartyCntl = (CSipPartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pSipExportPartyCntl;
		((CSipExportPartyCntl*)m_pPartyCntl)->Transfer(pDestRcvMbx, pConfParty, destConfId, destPartyId, eCurMoveType);
	}
	else if (ISDN_INTERFACE_TYPE == GetInterfaceType())
	{
		if (GetVoice())  // PSTN
		{
			CExportIsdnVoicePartyCntl* pExportPartyCntl = new CExportIsdnVoicePartyCntl;
			(CIsdnPartyCntl&)*pExportPartyCntl = (CIsdnPartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pExportPartyCntl;
			((CExportIsdnVoicePartyCntl*)m_pPartyCntl)->Transfer(pDestRcvMbx, pConfParty, destConfId, destPartyId, eCurMoveType);
		}
		else //ISDN
		{
			CExportIsdnPartyCntl* pExportPartyCntl = new CExportIsdnPartyCntl;
			(CIsdnPartyCntl&)*pExportPartyCntl = (CIsdnPartyCntl&)*m_pPartyCntl;
			POBJDELETE(m_pPartyCntl);
			m_pPartyCntl = pExportPartyCntl;
			((CExportIsdnPartyCntl*)m_pPartyCntl)->Transfer(pDestRcvMbx, pConfParty, destConfId, destPartyId, eCurMoveType);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//                          I M P O R T
//
/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ImportPSTN(CMoveIPImportParams* pMoveImportParams)
{
	if (pMoveImportParams->GetInterfaceType() != ISDN_INTERFACE_TYPE)
	{
		PASSERT(1);
		return;
	}

	m_pPartyCntl = new CImportIsdnVoicePartyCntl;
	((CImportIsdnVoicePartyCntl*)m_pPartyCntl)->Create(pMoveImportParams);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ImportISDN(CMoveIPImportParams* pMoveImportParams)
{
	if (pMoveImportParams->GetInterfaceType() != ISDN_INTERFACE_TYPE)
	{
		PASSERT(1);
		return;
	}

	m_pPartyCntl = new CImportIsdnPartyCntl;
	((CImportIsdnPartyCntl*)m_pPartyCntl)->Create(pMoveImportParams);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ImportIp(CMoveIPImportParams* pMoveImportParams, BOOL  isCSSPlugin)
{
	if (pMoveImportParams->GetInterfaceType() == H323_INTERFACE_TYPE)
	{
		m_pPartyCntl = new CH323ImportPartyCntl;
		((CH323ImportPartyCntl*)m_pPartyCntl)->Create(pMoveImportParams);
	}
	else if (pMoveImportParams->GetInterfaceType() == SIP_INTERFACE_TYPE)
	{
		if(TRUE == isCSSPlugin)
		{
			m_pPartyCntl = new CSipPluginImportPartyCntl;
			((CSipPluginImportPartyCntl*)m_pPartyCntl)->Create(pMoveImportParams);
		}
		else
		{
			m_pPartyCntl = new CSipImportPartyCntl;
			((CSipImportPartyCntl*)m_pPartyCntl)->Create(pMoveImportParams);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CPartyConnection::ContinueAddPartyAfterMoveIfNeeded()
{
	TRACEINTO << LOGINFO;

	if (!strcmp(m_pPartyCntl->NameOf(), "CH323ImportPartyCntl"))
	{
		if (((CH323PartyCntl*)m_pPartyCntl)->IsAdditionalRsrcActivated())
			return FALSE;

		CH323AddPartyCntl* pH323AddPartyCntl = new CH323AddPartyCntl;
		*pH323AddPartyCntl = (CH323PartyCntl&)*m_pPartyCntl;
		POBJDELETE(m_pPartyCntl);
		m_pPartyCntl = pH323AddPartyCntl;
		((CH323AddPartyCntl*)m_pPartyCntl)->ContinueAddPartyAfterMove();
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CIpComMode* CPartyConnection::GetCurrentIpScm()
{
	CIpPartyCntl* pIpPartyCntl = (CIpPartyCntl*)(m_pPartyCntl);
	return pIpPartyCntl->GetCurrentMode();
}

/////////////////////////////////////////////////////////////////////////////
CIpComMode* CPartyConnection::GetInitialIpScm()
{
	CIpPartyCntl* pIpPartyCntl = (CIpPartyCntl*)(m_pPartyCntl);
	return pIpPartyCntl->GetInitialMode();
}

/////////////////////////////////////////////////////////////////////////////
CComMode* CPartyConnection::GetIsdnTargetTransmitScm()
{
	CIsdnPartyCntl* pIsdnPartyCntl = (CIsdnPartyCntl*)(m_pPartyCntl);
	return pIsdnPartyCntl->GetTargetTransmitScm();
}

/////////////////////////////////////////////////////////////////////////////
CComMode* CPartyConnection::GetIsdnTargetReceiveScm()
{
	CIsdnPartyCntl* pIsdnPartyCntl = (CIsdnPartyCntl*)(m_pPartyCntl);
	return pIsdnPartyCntl->GetTargetReceiveScm();
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::StartCGContent()
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		TRACEINTO << LOGINFO << " - Failed, system is not CG";
		return;
	}

	if (!strcmp(m_pPartyCntl->NameOf(), "CH323ChangeModePartyCntl") || m_pPartyCntl->IsTypeOf("CH323ChangeModeCntl"))
		((CH323ChangeModeCntl*)m_pPartyCntl)->OnCGSendStartContent();
	else
		TRACEINTO << LOGINFO << " - Failed, party control is not in change mode, party control state = stop";
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::StopCGContent()
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		TRACEINTO << LOGINFO << " - Failed, system is not CG";
		return;
	}

	if ((!strcmp(m_pPartyCntl->NameOf(), "CH323ChangeModePartyCntl")) || (m_pPartyCntl->IsTypeOf("CH323ChangeModeCntl")))
		((CH323ChangeModeCntl*)m_pPartyCntl)->OnCGSendStopContent();
	else
		TRACEINTO << LOGINFO << " - Failed, party control is not in change mode, party control state = stop";
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnection::AddPartyChannel(CIsdnNetSetup& netSetUp, WORD channelNum)
{
	if (strcmp(m_pPartyCntl->NameOf(), "CAddIsdnPartyCntl") == 0)
		((CAddIsdnPartyCntl*)m_pPartyCntl)->AddPartyChannel(netSetUp, channelNum);
	else
		TRACEINTO << LOGINFO << " - Failed, add party channel without CAddIsdnPartyCntl";
}

////////////////////////////////////////////////////////////////////////////
BYTE CPartyConnection::ProceedToChangeContentMode()
{
	if (!m_pPartyCntl->IsFullBitRateConnected() && !strcmp(m_pPartyCntl->NameOf(), "CH323AddPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, can't proceed";
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ChangeSipfromTipToNonTip(CIpComMode* pScm, PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CSipDelPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	TRACEINTO << LOGINFO;

	CSipChangeModePartyCntl* pIpPartyCntl = (CSipChangeModePartyCntl*)(m_pPartyCntl);
	pIpPartyCntl->ChangeSipfromTipToNonTip(pScm, partyControInitParam, partyControlDataParams);
}

////////////////////////////////////////////////////////////////////////////
void CPartyConnection::ChangeSipfromIceToNoneIce(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	if (!strcmp(m_pPartyCntl->NameOf(), "CSipDelPartyCntl"))
	{
		TRACEINTO << LOGINFO << " - Failed, not allowed while disconnecting";
		return;
	}

	TRACEINTO << LOGINFO;

	CSipPartyCntl* pIpPartyCntl = (CSipPartyCntl*)(m_pPartyCntl);
	pIpPartyCntl->ChangeSipfromIceToNoneIce(partyControInitParam, partyControlDataParams);
}

////////////////////////////////////////////////////////////////////////////
void CPartyConnection::SetTimerForAsSipAddContentIfNeeded()
{
	DWORD ContentDelay;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_AS_SIP_CONTENT_ADD_DELAY, ContentDelay);

	TRACEINTO << LOGINFO << ", ContentDelay:" << ContentDelay;

	StartTimer(ADDASSIPCONTENTTOUT, ContentDelay * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CPartyConnection::AddAsSipContenttout(CSegment* pParam)
{
	TRACEINTO << LOGINFO;

	if (!strcmp(m_pPartyCntl->NameOf(), "CSipChangeModePartyCntl"))
		((CSipChangeModePartyCntl*)m_pPartyCntl)->AddContentToScm();
}
