#include <string.h>
#include "NStream.h"
#include "ConfParty.h"
#include "psosxml.h"
#include "H221Str.h"
#include "H323StrCap.h"
#include "PartyMonitor.h"
#include "IpChannelDetails.h"
#include "H323GkStatus.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "ApiStatuses.h"
#include "AttendedStruct.h"
#include "psosxml.h"

////////////////////////////////////////////////////////////////////////////
//                        CAttendedStruct
////////////////////////////////////////////////////////////////////////////
CAttendedStruct::CAttendedStruct()
{

	//BRIDGE-12482, instead of STATUS_PRATY_INCONF
	m_ordinary_party = STATUS_PARTY_NONE;
}

//--------------------------------------------------------------------------
CAttendedStruct::CAttendedStruct(const CAttendedStruct& other)
	: CPObject(other)
{
	m_ordinary_party = other.m_ordinary_party;
}

//--------------------------------------------------------------------------
CAttendedStruct& CAttendedStruct::operator =(const CAttendedStruct& other)
{
	m_ordinary_party = other.m_ordinary_party;

	return *this;
}
//--------------------------------------------------------------------------
CAttendedStruct::~CAttendedStruct()
{
}

//--------------------------------------------------------------------------
void CAttendedStruct::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAttendedNode = pFatherNode->AddChildNode("ATTENDED");

	pAttendedNode->AddChildNode("ATTENDING_STATE", m_ordinary_party, ATTENDING_STATE_ENUM);
}

//--------------------------------------------------------------------------
int CAttendedStruct::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "ATTENDING_STATE", &m_ordinary_party, ATTENDING_STATE_ENUM);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CAttendedStruct::GetOrdinaryParty() const
{
	return m_ordinary_party;
}

//--------------------------------------------------------------------------
void CAttendedStruct::SetOrdinaryParty(const BYTE ordinary_party)
{
	TRACEINTO << "was: " << m_ordinary_party << ", new: " << ordinary_party;
	m_ordinary_party = ordinary_party;
}

//--------------------------------------------------------------------------
const char* CAttendedStruct::NameOf() const
{
	return "CAttendedStruct";
}
