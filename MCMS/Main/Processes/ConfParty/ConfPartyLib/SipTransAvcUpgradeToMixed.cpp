


/*
 * SipTransAvcUpgradeToMixed.cpp
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

#include "SipTransAvcUpgradeToMixed.h"


extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransAvcUpgradeToMixed)

ONEVENT(PARTY_UPGRADE_TO_MIXED,			IDLE,					CSipTransAvcUpgradeToMixed::OnSipPartyUpgradeToMixed)
ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,	sPARTY_ALLOCATE_TRANSLATOR_ARTS,	CSipTransAvcUpgradeToMixed::OnPartyTranslatorArtsConnected)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_WAITFORUPDATECHANN, CSipTransAvcUpgradeToMixed::OnSipPartyChannelsUpdated) // ey_20866
ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,	sTRANS_CHANNSUPDATEDWAITFORBRIDGESUPGRADE/*sTRANS_WAITFORUPDATECHANN*/, CSipTransAvcUpgradeToMixed::OnSipEndVideoUpgradeToMix)
PEND_MESSAGE_MAP(CSipTransAvcUpgradeToMixed,CSipTransaction);

CSipTransAvcUpgradeToMixed::CSipTransAvcUpgradeToMixed(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = FALSE;
	m_bIsReInvite = FALSE;
}


CSipTransAvcUpgradeToMixed::~CSipTransAvcUpgradeToMixed()
{
}
void CSipTransAvcUpgradeToMixed::OnSipPartyUpgradeToMixed(CSegment* pParam)
{
	TRACEINTO<<"!@# arrived at transaction";
	  CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	  //	m_pSipCntl->ResetChangeModeWithinTransactionValue();
	 m_pSipCntl->PrintChangeModeWithinTransactionValue();
	CRsrcParams* pMrmpRsrcParams = NULL;

	DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams);
    int cnt=0;
		for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
		DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "!@#  translator" );
		if (avcToSvcTranslatorRsrcParams[i])
			     cnt++;
			}






		/*       CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
        if( NULL == pRoutingTbl )
        {
	      TRACEINTO<<"!@# ptr to routing table is NULL";
              PASSERT(103);
        }
	else
	{
	    TRACEINTO<<"!@# printing routing table";
	    pRoutingTbl->DumpTable();
	    }*/
	  m_pSipCntl->SetInternalControllerResource(avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);

	  m_pSipCntl->AddToInternalRoutingTable();
	  /*pRoutingTbl = ::GetpConfPartyRoutingTable();
        if( NULL == pRoutingTbl )
        {
	      TRACEINTO<<"!@# ptr to routing table is NULL";
              PASSERT(103);
        }
	else
	{
	    TRACEINTO<<"!@# printing routing table";
	    pRoutingTbl->DumpTable();
	    }*/
	  //	  bool flag=false;

	  if(m_pTargetMode->GetConfMediaType() == eMixAvcSvc)
	   {
	 	  m_state = sPARTY_ALLOCATE_TRANSLATOR_ARTS;
		  TRACEINTO<<"!@# mixed mode detected";
		  if(cnt>0)
		  {
		      m_pSipCntl->OpenInternalArts(E_NETWORK_TYPE_IP,cnt);
		  }
		  else
		  {
		    if(IsSoftMcu() && m_pTargetMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople) &&  m_pTargetMode->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
		    {
		      TRACEINTO<<"dynMixedPosAck no need to update channels (maybe SoftMcu audio only participant)";
		      OnSipPartyChannelsUpdated(NULL);
		      //OnPartyTranslatorArtsConnected();
		    }
		    else
		    {
		       TRACEINTO<<"!@# dynmixedErr there are no translators"; 
		    }
		  }		  
	   }
	  else
	  {
	    TRACEINTO<<"!@# dynmixedErr mixed mode wasn't detected m_pTargetMode->GetConfMediaType():"<<m_pTargetMode->GetConfMediaType(); /* ey_20866  */
	  }

		POBJDELETE(pMrmpRsrcParams);
	  for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	  {
		 POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	  }
}
void CSipTransAvcUpgradeToMixed::OnPartyTranslatorArtsConnected()
{
	TRACEINTO<<"!@# arts connected";
	m_state=sTRANS_WAITFORUPDATECHANN;
    if (m_pSipCntl->SipUpgradeAvcChannelReq((CSipComMode*)m_pTargetMode,(CSipComMode*)m_pCurrentMode) <= 0)
    {
        TRACEINTO << "!@# dynmixedErr no requests to open internal channels"; // ey_20866
    }
}

void CSipTransAvcUpgradeToMixed::OnSipPartyChannelsUpdated(CSegment* pParam)
{
	TRACEINTO<<"!@# all channels have been  upgraded (arrived at transaction: after channels update)";
	//bridge-14289
	m_pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople), cmCapAudio, cmCapReceive, kRolePeople);
	m_pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);
	m_pCurrentMode->Dump("CSipTransAvcUpgradeToMixed::OnSipPartyChannelsUpdated, Current mode ", eLevelInfoNormal);

	CSegment* pSeg = new CSegment;
	m_state=sTRANS_CHANNSUPDATEDWAITFORBRIDGESUPGRADE;
	SendMessageToParty(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,NULL/*pSeg*/);
	POBJDELETE(pSeg);

}
void CSipTransAvcUpgradeToMixed::OnSipEndVideoUpgradeToMix(CSegment* pParam)
{
	EndTransaction();
	TRACEINTO<<"!@# ";
}

