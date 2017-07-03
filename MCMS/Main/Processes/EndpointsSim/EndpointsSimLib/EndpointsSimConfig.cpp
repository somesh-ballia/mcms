//+========================================================================+
//               EndpointsSimConfigConfig.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointsSimConfigConfig.cpp                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// EndpointsSimConfigConfig.cpp: configuration of system.
//
//////////////////////////////////////////////////////////////////////


#include <string>
#include <stdio.h>
#include "psosxml.h"
#include "XmlDefines.h"
#include "XmlApi.h"
#include "StrArray.h"
#include "Trace.h"
#include "Macros.h"
#include "StringsMaps.h"

#include "InitCommonStrings.h"
#include "Segment.h"
#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"
#include "StatusesGeneral.h"

#include "ApiStatuses.h"
/////////////////////////////////////////////////////////////////////////////
CEndpointsSimSystemCfg::CEndpointsSimSystemCfg()
{
	// address & port number for Socket connection with CS-API
	strncpy(m_szCsApiIp,"127.0.0.1",IP_ADDRESS_STR_LEN);
	m_wCsApiPortNumber	= 10008;

	// Listen port for GUI
	m_wGuiPortNumber	= 20004;

	m_bEncryptionDialIn = FALSE;
	m_bEncryptionDialOut = TRUE;
	m_errorBitRate = 0;
	m_rejectStatus = 200; // SipCodesOk;
	strncpy(m_szRedirectionSipAddress,"101.101.101.101",H243_NAME_LEN);

	m_bDeleteDialOut     = TRUE;
	m_bIsAvayaGatekeeper = FALSE;
	m_gkBrqTime			 = 0;

	int status = ReadXmlFile();
	if( status != STATUS_OK )
		WriteXmlFile();
		
		
	strncpy(m_szCSIpAddress, "0.0.0.0", IP_ADDRESS_STR_LEN);

 	::SetEpSystemCfg(this);
}

/////////////////////////////////////////////////////////////////////////////
CEndpointsSimSystemCfg::~CEndpointsSimSystemCfg()
{
}

/////////////////////////////////////////////////////////////////////////////
CEndpointsSimSystemCfg::CEndpointsSimSystemCfg(const CEndpointsSimSystemCfg& rOther)
	: CSerializeObject(rOther)
{
	*this = rOther;
}


/////////////////////////////////////////////////////////////////////////////
CEndpointsSimSystemCfg& CEndpointsSimSystemCfg::operator= (const CEndpointsSimSystemCfg& rOther)
{
	if( this == &rOther )
		return *this;

	strncpy(m_szCsApiIp,rOther.m_szCsApiIp,IP_ADDRESS_STR_LEN);
	m_wCsApiPortNumber	= rOther.m_wCsApiPortNumber;

	m_wGuiPortNumber = rOther.m_wGuiPortNumber;

	m_bEncryptionDialIn = rOther.m_bEncryptionDialIn;
	m_bEncryptionDialOut = rOther.m_bEncryptionDialOut;
	m_errorBitRate = rOther.m_errorBitRate;

	m_rejectStatus = 200; // SipCodesOk;
	strncpy(m_szRedirectionSipAddress,"101.101.101.101",H243_NAME_LEN);

	m_bDeleteDialOut     = rOther.m_bDeleteDialOut;
	m_bIsAvayaGatekeeper = rOther.m_bIsAvayaGatekeeper;
	m_gkBrqTime			 = rOther.m_gkBrqTime;
	
	strncpy(m_szCSIpAddress, rOther.m_szCSIpAddress, IP_ADDRESS_STR_LEN);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimSystemCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// create <SYSTEM_DETAILS> section
	CXMLDOMElement* pSimDetailsNode = pFatherNode->AddChildNode("SYSTEM_DETAILS");
	if( NULL == pSimDetailsNode )
		return;

	// <SYSTEM_DETAILS> fields
	pSimDetailsNode->AddChildNode("CS_API_IP_ADDRESS",m_szCsApiIp);
	pSimDetailsNode->AddChildNode("CS_API_PORT", m_wCsApiPortNumber);
	pSimDetailsNode->AddChildNode("GUI_PORT", m_wGuiPortNumber);
	pSimDetailsNode->AddChildNode("ENCRYPT_DIAL_IN", m_bEncryptionDialIn,_BOOL);
	pSimDetailsNode->AddChildNode("ENCRYPT_DIAL_OUT", m_bEncryptionDialOut,_BOOL);

	// create <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> section
	CXMLDOMElement* pRejectNode = pSimDetailsNode->AddChildNode("REJECT_DIAL_OUT");
	if( NULL != pRejectNode )
	{
		// <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> fields
		pRejectNode->AddChildNode("DIAL_OUT_STATUS", m_rejectStatus);
		pRejectNode->AddChildNode("REDIRECTION_SIP_ADDRESS",m_szRedirectionSipAddress);
	}
	pSimDetailsNode->AddChildNode("DELETE_DIAL_OUT_AFTER_DISCONNECT",m_bDeleteDialOut,_BOOL);
	pSimDetailsNode->AddChildNode("GATEKEEPER_IS_AVAYA",m_bIsAvayaGatekeeper,_BOOL);
	pSimDetailsNode->AddChildNode("GATEKEEPER_BRQ_TIME",m_gkBrqTime);
}

/////////////////////////////////////////////////////////////////////////////
int	CEndpointsSimSystemCfg::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int		nStatus			= STATUS_OK;
	char*	pszChildName	= NULL;

	// get <SYSTEM_DETAILS> section
	CXMLDOMElement*		pSimDetailsNode = NULL;
	GET_CHILD_NODE(pActionNode,"SYSTEM_DETAILS",pSimDetailsNode);

	// get fields from <SYSTEM_DETAILS> section
	if( pSimDetailsNode ) {
		GET_VALIDATE_CHILD(pSimDetailsNode,"CS_API_IP_ADDRESS",m_szCsApiIp,_1_TO_IP_ADDRESS_LENGTH);
		GET_VALIDATE_CHILD(pSimDetailsNode,"CS_API_PORT",&m_wCsApiPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"GUI_PORT",&m_wGuiPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"ENCRYPT_DIAL_IN",&m_bEncryptionDialIn,_BOOL);
		GET_VALIDATE_CHILD(pSimDetailsNode,"ENCRYPT_DIAL_OUT",&m_bEncryptionDialOut,_BOOL);

		// get <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> section
		CXMLDOMElement*		pRejectNode = NULL;
		GET_CHILD_NODE(pSimDetailsNode,"REJECT_DIAL_OUT",pRejectNode);

		if( pRejectNode )
		{
			GET_VALIDATE_CHILD(pRejectNode,"DIAL_OUT_STATUS",&m_rejectStatus,_0_TO_DWORD);
			GET_VALIDATE_CHILD(pRejectNode,"REDIRECTION_SIP_ADDRESS",m_szRedirectionSipAddress,_1_TO_H243_NAME_LENGTH);
		}

		GET_VALIDATE_CHILD(pSimDetailsNode,"DELETE_DIAL_OUT_AFTER_DISCONNECT",&m_bDeleteDialOut,_BOOL);
		GET_VALIDATE_CHILD(pSimDetailsNode,"GATEKEEPER_IS_AVAYA",&m_bIsAvayaGatekeeper,_BOOL);
		GET_VALIDATE_CHILD(pSimDetailsNode,"GATEKEEPER_BRQ_TIME",&m_gkBrqTime,_0_TO_DWORD);
	}

	return nStatus;	
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimSystemCfg::Serialize(CSegment& rSegment) const
{
	rSegment << (DWORD)m_wGuiPortNumber;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimSystemCfg::DeSerialize(CSegment& rSegment)
{
	DWORD  temp = 0;
	rSegment >> temp;
	m_wGuiPortNumber = (WORD)temp;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointsSimSystemCfg::IsEqual(const CEndpointsSimSystemCfg& rOther) const
{
	if( m_wGuiPortNumber != rOther.m_wGuiPortNumber )
		return FALSE;

	if( m_bEncryptionDialIn != rOther.m_bEncryptionDialIn )
		return FALSE;

	if( m_bEncryptionDialOut != rOther.m_bEncryptionDialOut )
		return FALSE;

	if( m_bDeleteDialOut != rOther.m_bDeleteDialOut )
		return FALSE;

	if( m_bIsAvayaGatekeeper != rOther.m_bIsAvayaGatekeeper )
		return FALSE;

	if( m_gkBrqTime != rOther.m_gkBrqTime )
		return FALSE;

	return TRUE;
}







