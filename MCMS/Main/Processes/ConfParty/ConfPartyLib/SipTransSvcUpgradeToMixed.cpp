/*
 * SipTransSvcUpgradeToMixed.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: enissim
 */






#include "Segment.h"
#include "StateMachine.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "Trace.h"
#include "Macros.h"
#include "NStream.h"
#include "NetSetup.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
#include "SipDefinitions.h"
#include "SIPCommon.h"
#include "SIPInternals.h"
#include "SipUtils.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "CsInterface.h"
#include "SipScm.h"
#include "SipCall.h"
#include "ConfApi.h"
#include "IPParty.h"
#include "SIPControl.h"
#include "Lobby.h"
#include "SIPParty.h"
#include "ConfPartyGlobals.h"
#include "IpServiceListManager.h"
#include "IPParty.h"
#include "SIPTransaction.h"
#include "SIPTransInviteWithSdpInd.h"

#include "SipTransSvcUpgradeToMixed.h"


////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransSvcUpgradeToMixed)

ONEVENT(PARTY_UPGRADE_TO_MIXED,				IDLE,							CSipTransSvcUpgradeToMixed::OnSipPartyUpgradeToMixed)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_WAITFORUPDATECHANN, 		CSipTransSvcUpgradeToMixed::OnSipPartyChannelsUpdated)
ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,	sTRANS_CHANNSUPDATEDWAITFORBRIDGESUPGRADE/*sTRANS_WAITFORUPDATECHANN*/, CSipTransSvcUpgradeToMixed::OnSipEndVideoUpgradeToMix)


PEND_MESSAGE_MAP(CSipTransSvcUpgradeToMixed,CSipTransaction);

CSipTransSvcUpgradeToMixed::CSipTransSvcUpgradeToMixed(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = FALSE;
	m_bIsReInvite = FALSE;
}

CSipTransSvcUpgradeToMixed::~CSipTransSvcUpgradeToMixed()
{
}

void CSipTransSvcUpgradeToMixed::OnSipPartyUpgradeToMixed(CSegment* pParam)
{
	TRACEINTO<<"!@# arrived at transaction";
	m_state=sTRANS_WAITFORUPDATECHANN;
	SendUpdateSvcChannelsToMixed((CSipComMode*)m_pTargetMode);
}

void CSipTransSvcUpgradeToMixed::OnSipPartyChannelsUpdated(CSegment* pParam)
{
	TRACEINTO<<"!@# All channels are updated (arrived at transaction: after channels update)";
	CSegment* pSeg = new CSegment;
	m_state=sTRANS_CHANNSUPDATEDWAITFORBRIDGESUPGRADE;
	SendMessageToParty(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,NULL/*pSeg*/);
	POBJDELETE(pSeg);
}

void CSipTransSvcUpgradeToMixed::SendUpdateSvcChannelsToMixed(CSipComMode* pTargetMode)
{
	ERoleLabel eRole=kRolePeople;

	CSipChannel* pCurChannel = NULL;
	cmCapDataType mediaTypeArr[]={cmCapVideo,cmCapAudio};
	EIpChannelType chanArr;

	for (unsigned int i=0;i<sizeof(mediaTypeArr)/sizeof(cmCapDataType);++i)
	{
		TRACEINTO << "!@# index i: " << i;
		pCurChannel	= m_pSipCntl->GetCallObj()->GetChannel(true, mediaTypeArr[i], cmCapReceive,eRole);

		if (pCurChannel)
		{
			chanArr = ::CalcChannelType(mediaTypeArr[i], FALSE, eRole);
			TRACEINTO << "!@# request to upgrade: " << ::GetTypeStr(mediaTypeArr[i]) 
				<< " " << ::GetDirectionStr(pCurChannel->GetDirection()) 
				<< " channel chanArr: " << IpChannelTypeToString(chanArr);
			m_pSipCntl->SipUpgradeSvcChannelReq(pTargetMode, chanArr, kChannelParams,NO, YES);
		}
	}
}

void CSipTransSvcUpgradeToMixed::OnSipEndVideoUpgradeToMix(CSegment* pParam)
{
	EndTransaction();
	TRACEINTO<<"!@# ";
}

