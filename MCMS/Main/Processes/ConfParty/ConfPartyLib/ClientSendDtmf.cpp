// ClientSendDtmf.cpp: implementation of the CClientSendDtmf class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Setting Default IVR Service 
//========   ==============   =====================================================================

#include "NStream.h"
#include "ClientSendDtmf.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CClientSendDtmf::CClientSendDtmf()
{
	m_dtmfDirection = 0;
	m_dtmfStr[0] = '\0';
	m_ConfID = 0;
	m_PartyID = 0;
	m_partyMonitorIdStr[0] = '\0';
	m_confIdStr[0] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
CClientSendDtmf& CClientSendDtmf::operator = (const CClientSendDtmf &other)
{
	strncpy( m_dtmfStr, other.m_dtmfStr, MAX_DTMF_FROM_CLIENT_LEN );
	m_dtmfStr[MAX_DTMF_FROM_CLIENT_LEN-1] = '\0';
	m_dtmfDirection = other.m_dtmfDirection;
	m_ConfID = other.m_ConfID;
	m_PartyID = other.m_PartyID;
	strncpy( m_partyMonitorIdStr, other.m_partyMonitorIdStr, MAX_ID_SIZE_FOR_DTMF );
	m_partyMonitorIdStr[MAX_ID_SIZE_FOR_DTMF-1] = '\0';
	strncpy( m_confIdStr, other.m_confIdStr, MAX_ID_SIZE_FOR_DTMF );
	m_confIdStr[MAX_ID_SIZE_FOR_DTMF-1] = '\0';

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CClientSendDtmf::~CClientSendDtmf()
{
}


///////////////////////////////////////////////////////////////////////////
void CClientSendDtmf::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CClientSendDtmf::DeSerializeXml(CXMLDOMElement* pDtmfNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	
	m_ConfID = 0;
	GET_VALIDATE_CHILD(pDtmfNode,"ID",&m_ConfID,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pDtmfNode,"ID", m_confIdStr,	_1_TO_AV_MSG_SERVICE_NAME_LENGTH);
	m_confIdStr[MAX_ID_SIZE_FOR_DTMF-1] = '\0';
	
	m_PartyID = 0;
	GET_VALIDATE_MANDATORY_CHILD(pDtmfNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pDtmfNode, "PARTY_ID", m_partyMonitorIdStr,	_1_TO_AV_MSG_SERVICE_NAME_LENGTH);
	m_partyMonitorIdStr[MAX_ID_SIZE_FOR_DTMF-1] = '\0';
	
	m_dtmfDirection = 1;
	GET_VALIDATE_CHILD(pDtmfNode,"DTMF_DIRECTION_TO_EP",&m_dtmfDirection,_0_TO_DWORD);
	
	char myTempString[512];	// a long string to be on the safe side.(need to verify the len!!)
	myTempString[0] = '\0';
	GET_VALIDATE_CHILD(pDtmfNode, "DTMF_STRING", myTempString, _1_TO_AV_MSG_SERVICE_NAME_LENGTH);
	SetDtmfString( myTempString );
	

	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
const char* CClientSendDtmf::GetDtmfString()
{
	return m_dtmfStr;
}

void CClientSendDtmf::SetDtmfString( const char *dtmfStr )
{
	if (NULL == dtmfStr)
		return;
		
	int len = strlen( dtmfStr );
	if (len >= MAX_DTMF_FROM_CLIENT_LEN)
		len = MAX_DTMF_FROM_CLIENT_LEN-1;
		
	if (len > 0)
	{
		strncpy( m_dtmfStr, dtmfStr, sizeof(m_dtmfStr) - 1 );
		m_dtmfStr[sizeof(m_dtmfStr) - 1] = '\0';
	}
}
	
void CClientSendDtmf::SetDtmfDirection(int inOut)
{
	m_dtmfDirection = inOut;
}

int  CClientSendDtmf::GetDtmfDirection()
{
	return m_dtmfDirection;
}


/////////////////////////////////////////////////////////////////////////
DWORD  CClientSendDtmf::GetConfID()
{
	return m_ConfID;
}

//////////////////////////////////////////////////////////////////////////
void CClientSendDtmf::SetConfID(DWORD confId)
{
	m_ConfID = confId;
}

//////////////////////////////////////////////////////////////////////////
DWORD  CClientSendDtmf::GetPartyID()
{
	return m_PartyID;
}

//////////////////////////////////////////////////////////////////////////
void CClientSendDtmf::SetPartyID(DWORD partyId)
{
	m_PartyID = partyId;
}

//////////////////////////////////////////////////////////////////////////
const char* CClientSendDtmf::GetConfIdString()
{
	return m_confIdStr;
}

//////////////////////////////////////////////////////////////////////////
const char* CClientSendDtmf::GetPartyMonitorIdString()
{
	return m_partyMonitorIdStr;
}




