//+========================================================================+
//                   GideonSimCardApi.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardApi.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include <string>

#include "Trace.h"

#include "GideonSim.h"
#include "GideonSimConfig.h"
#include "GideonSimCardApi.h"
#include "GideonSimCardTask.h"



/////////////////////////////////////////////////////////////////////////////
CSimCardApi::CSimCardApi()	// constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimCardApi::~CSimCardApi()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void  CSimCardApi::Create(COsQueue& creatorRcvMbx,CCardCfg* pCardCfg)
{
	WORD   additionalFlag = 0;
	WORD   portNumber = 0;

	WORD   additionalParamRtmCardType = 0;

	char   szIpAddress[IP_ADDRESS_STR_LEN];
	strncpy(szIpAddress,::GetGideonSystemCfg()->GetMplApiIpAddress(),sizeof(szIpAddress) - 1);
	szIpAddress[sizeof(szIpAddress) - 1] = '\0';

	void (*entryPoint)(void*) = gideonSimMfaCardEntryPoint;

	switch( pCardCfg->GetCardType() ) {
		case eCardSwitch:
			entryPoint = gideonSimSwitchCardEntryPoint;
			portNumber = ::GetGideonSystemCfg()->GetMplApiSwitchPortNumber();
			break;
		case eCardMfa:
			entryPoint = gideonSimMfaCardEntryPoint;
			portNumber = ::GetGideonSystemCfg()->GetMplApiMfaPortNumber();
			additionalFlag = ((CCardCfgMfa*)pCardCfg)->GetRtmAttached(); /* MFA has RTM subcard */
			break;
		case eCardBarak:
			entryPoint = gideonSimBarakCardEntryPoint;
			portNumber = ::GetGideonSystemCfg()->GetMplApiMfaPortNumber();
			additionalFlag = ((CCardCfgBarak*)pCardCfg)->GetRtmAttached(); /* Barak has RTM subcard */
			additionalParamRtmCardType = ((CCardCfgBarak*)pCardCfg)->GetRtmCardType(); /* Barak has RTM subcard */
			break;
		case eCardGideonLite:
			entryPoint = gideonSimGideonLiteCardEntryPoint;
			portNumber = ::GetGideonSystemCfg()->GetMplApiMfaPortNumber();
			additionalFlag = ((CCardCfgMfa*)pCardCfg)->GetRtmAttached(); /* MFA has RTM subcard */
			additionalParamRtmCardType = ((CCardCfgBarak*)pCardCfg)->GetRtmCardType(); /* Barak has RTM subcard */

			break;
		case eCardBreeze:
			entryPoint = gideonSimBreezeCardEntryPoint;
			portNumber = ::GetGideonSystemCfg()->GetMplApiMfaPortNumber();
			additionalFlag = ((CCardCfgBreeze*)pCardCfg)->GetRtmAttached(); /* Breeze has RTM subcard */
			additionalParamRtmCardType = ((CCardCfgBarak*)pCardCfg)->GetRtmCardType(); /* Barak has RTM subcard */

			break;
		case eCardMpmRx:
		    entryPoint = gideonSimBreezeCardEntryPoint;
		    portNumber = ::GetGideonSystemCfg()->GetMplApiMfaPortNumber();
		    additionalFlag = ((CCardCfgMpmRx*)pCardCfg)->GetRtmAttached(); /* MPM-Rx has RTM subcard */
			additionalParamRtmCardType = ((CCardCfgBarak*)pCardCfg)->GetRtmCardType(); /* Barak has RTM subcard */

		    break;
		default:
			PASSERT(100+pCardCfg->GetCardType());
			return;
	}

	CTaskApi::Create(creatorRcvMbx); // set default stack param i.e. creator rsv mbx 

	//  put all card data
	m_appParam	<< pCardCfg->GetBoardId()
				<< pCardCfg->GetSerialNumber()
				<< (char*)szIpAddress
				<< portNumber
				<< additionalFlag
				<<additionalParamRtmCardType;

	LoadApp(entryPoint);
}

/////////////////////////////////////////////////////////////////////////////
void  CSimCardApi::Connect()
{
	CSegment*  seg = new CSegment;

	SendMsg(seg,CONNECT_CARD);
}

/////////////////////////////////////////////////////////////////////////////
void  CSimCardApi::UpdateBurnRate(DWORD rate,eBurnTypes burnType)
{

	CSegment*  seg = new CSegment;
	*seg << rate;
	*seg << (DWORD)burnType;

	SendMsg(seg,UPDATE_BURN_RATE);
}

/////////////////////////////////////////////////////////////////////////////
void  CSimCardApi::SendBurnAction(eBurnActionTypes burnActionType, eBurnTypes burnType)
{

	CSegment*  seg = new CSegment;
	*seg << (DWORD)burnActionType;
	*seg << (DWORD)burnType;

	SendMsg(seg,SET_BURN_ACTIONS);
}

