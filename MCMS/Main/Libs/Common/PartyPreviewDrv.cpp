#include "PartyPreviewDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ConfPartyApiDefines.h"


////////////////////////////////////////////////////////////////////////////
//                        CPartyPreviewDrv
////////////////////////////////////////////////////////////////////////////
CPartyPreviewDrv::CPartyPreviewDrv()
{
	monitor_conf_id   = 0xFFFFFFFF;
	monitor_party_id  = 0xFFFFFFFF;
	m_Direction       = 0;
	m_RemoteIPAddress = 0xFFFFFFFF;
	m_VideoPort       = 0xFFFF;
	m_AudioPort       = 0xFFFF;
}

//--------------------------------------------------------------------------
CPartyPreviewDrv::CPartyPreviewDrv(CPartyPreviewDrv& other) : CSerializeObject(other)
{
	monitor_conf_id   = other.monitor_conf_id;
	monitor_party_id  = other.monitor_party_id;
	m_Direction       = other.m_Direction;
	m_RemoteIPAddress = other.m_RemoteIPAddress;
	m_VideoPort       = other.m_VideoPort;
	m_AudioPort       = other.m_AudioPort;
}

//--------------------------------------------------------------------------
CPartyPreviewDrv& CPartyPreviewDrv::operator =(const CPartyPreviewDrv& other)
{
	monitor_conf_id   = other.monitor_conf_id;
	monitor_party_id  = other.monitor_party_id;
	m_Direction       = other.m_Direction;
	m_RemoteIPAddress = other.m_RemoteIPAddress;
	m_VideoPort       = other.m_VideoPort;
	m_AudioPort       = other.m_AudioPort;
	return *this;
}

//--------------------------------------------------------------------------
CPartyPreviewDrv::~CPartyPreviewDrv()
{
}

//--------------------------------------------------------------------------
int CPartyPreviewDrv::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const char* strAction)
{
	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode, pszError, numAction);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CPartyPreviewDrv::convertStrActionToNumber(const char* strAction)
{
	int numAction = UNKNOWN_ACTION;
	if (!strncmp("START_PREVIEW", strAction, 13))
		numAction = START_PARTY_PTRVIEW;

	return numAction;
}

//--------------------------------------------------------------------------
int CPartyPreviewDrv::DeSerializeXml(CXMLDOMElement* pForceNode, char* pszError, int nAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pForceNode, "ID", &monitor_conf_id, _0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode, "PARTY_ID", &monitor_party_id, _0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode, "VIDEO_DIRECTION", &m_Direction, DIRECTION_TYPE_ENUM);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode, "CLIENT_IP", &m_RemoteIPAddress, IP_ADDRESS);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode, "VIDEO_PORT", &m_VideoPort, _0_TO_WORD);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode, "AUDIO_PORT", &m_AudioPort, _0_TO_WORD);

	return nStatus;
}

//--------------------------------------------------------------------------
void CPartyPreviewDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("ID", monitor_conf_id, _0_TO_DWORD);
	pFatherNode->AddChildNode("PARTY_ID", monitor_party_id, _0_TO_DWORD);
	pFatherNode->AddChildNode("VIDEO_DIRECTION", m_Direction, DIRECTION_TYPE_ENUM);

	if (m_RemoteIPAddress)
		pFatherNode->AddChildNode("CLIENT_IP", m_RemoteIPAddress, IP_ADDRESS);

	if (m_VideoPort)
		pFatherNode->AddChildNode("VIDEO_PORT", m_VideoPort, _0_TO_WORD);

	if (m_AudioPort)
		pFatherNode->AddChildNode("AUDIO_PORT", m_AudioPort, _0_TO_WORD);
}

//--------------------------------------------------------------------------
void CPartyPreviewDrv::Serialize(WORD format, std::ostream& m_ostr) const
{
	if (format == NATIVE)
	{
		m_ostr << monitor_conf_id << "\n";
		m_ostr << monitor_party_id << "\n";
		m_ostr << m_RemoteIPAddress << "\n";
		m_ostr << m_VideoPort << "\n";
		m_ostr << m_AudioPort << "\n";
		m_ostr << m_Direction << "\n";
	}
}

//--------------------------------------------------------------------------
void CPartyPreviewDrv::DeSerialize(WORD format, std::istream& m_istr)
{
	if (format == NATIVE)
	{
		m_istr >> monitor_conf_id;
		m_istr >> monitor_party_id;
		m_istr >> m_RemoteIPAddress;
		m_istr >> m_VideoPort;
		m_istr >> m_AudioPort;
		m_istr >> m_Direction;
	}
}
