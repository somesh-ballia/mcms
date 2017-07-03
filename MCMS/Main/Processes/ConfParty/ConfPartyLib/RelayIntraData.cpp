#include "RelayIntraData.h"
#include "StlUtils.h"

#define BOOL2STR(B) (B?"true":"false")
#define STR2BOOL(S) (strcmp(S,"true")?false:true)

///////////////////////////////////////////////////
RelayIntraParam::RelayIntraParam()
:m_partyRsrcId(0)
,m_bIsGdr(false), m_bIsSsrc(false)
{
}

///////////////////////////////////////////////////
RelayIntraParam::RelayIntraParam(const RelayIntraParam &other)
: CPObject(other)
,m_partyRsrcId(other.m_partyRsrcId)
,m_bIsGdr(other.m_bIsGdr)
,m_bIsSsrc(other.m_bIsSsrc)
{
	m_listSsrc.assign(other.m_listSsrc.begin(),
				other.m_listSsrc.end());
}

///////////////////////////////////////////////////
RelayIntraParam::~RelayIntraParam()
{
	m_listSsrc.clear();
}

///////////////////////////////////////////////////
void RelayIntraParam::InitDefaults()
{
	m_partyRsrcId = 0;
	m_listSsrc.clear();
	m_bIsGdr = false;
	m_bIsSsrc = false;
}

///////////////////////////////////////////////////
RelayIntraParam& RelayIntraParam::operator=(const RelayIntraParam &other)
{
	if (this == &other)
		return *this;

	InitDefaults();

	m_partyRsrcId = other.m_partyRsrcId;
	m_listSsrc.assign(other.m_listSsrc.begin(), other.m_listSsrc.end());
	m_bIsGdr = other.m_bIsGdr;
	m_bIsSsrc = other.m_bIsSsrc;

	return *this;
}

///////////////////////////////////////////////////
bool RelayIntraParam::operator==(const RelayIntraParam& other) const
{
	if (m_partyRsrcId != other.m_partyRsrcId)
		return false;
	if (m_listSsrc.size() != other.m_listSsrc.size() )
		return false;
	if (m_bIsGdr != other.m_bIsGdr)
		return false;
    if (m_bIsSsrc != other.m_bIsSsrc)
        return false;

	if (!std::equal(m_listSsrc.begin(),
					m_listSsrc.end(),
					other.m_listSsrc.begin()))
		 return false;

	return true;
}

///////////////////////////////////////////////////
bool RelayIntraParam::operator!=(const RelayIntraParam& other) const
{
	return !(*this == other);
}

///////////////////////////////////////////////////
void RelayIntraParam::Serialize(CSegment* pSeg) const
{
	*pSeg << m_partyRsrcId;
	*pSeg << (DWORD)m_bIsGdr;
	*pSeg << (DWORD)m_bIsSsrc;

	DWORD size_listSsrc = m_listSsrc.size();
	*pSeg << size_listSsrc;
	std::list<unsigned int>::const_iterator it = m_listSsrc.begin();
	for ( ; it != m_listSsrc.end(); ++it)
		*pSeg << *it;

}

///////////////////////////////////////////////////
void RelayIntraParam::DeSerialize(CSegment* pSeg)
{
	InitDefaults();

	(*pSeg) >> m_partyRsrcId;

	DWORD dwTmp;
	(*pSeg) >> dwTmp;
	m_bIsGdr = (dwTmp ? true : false);
    (*pSeg) >> dwTmp;
    m_bIsSsrc = (dwTmp ? true : false);


	m_listSsrc.clear();
	unsigned int size_listSsrc = 0;
	(*pSeg) >> size_listSsrc;

	for (unsigned int i=0; i < size_listSsrc; ++i)
	{
		unsigned int ssrc = 0;
		(*pSeg) >> ssrc;
		m_listSsrc.push_back(ssrc);
	}
}

std::string RelayIntraParam::ToString() const
{
	std::string sRetVal = "PartyRsrcId = ";
	sRetVal += CStlUtils::ValueToString(m_partyRsrcId);
	sRetVal += std::string("Is GDR = ") + (m_bIsGdr ? "true" : "false") + "; ";
    sRetVal += std::string("Is Ssrc = ") + (m_bIsSsrc ? "true" : "false") + "; ";
	sRetVal += ";    list Ssrc: ";
	sRetVal += CStlUtils::ContainerToString(m_listSsrc);
	return sRetVal;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
AskForRelayIntra::AskForRelayIntra()
:m_isImmediately(false), m_isAllowSuppression(false)
{
	m_relayIntraParameters.clear();
}

///////////////////////////////////////////////////
AskForRelayIntra::AskForRelayIntra(const AskForRelayIntra &other)
: CPObject(other), m_relayIntraParameters(other.m_relayIntraParameters)
,m_isImmediately(other.m_isImmediately),m_isAllowSuppression(other.m_isAllowSuppression)
{
}

///////////////////////////////////////////////////
AskForRelayIntra::~AskForRelayIntra()
{
}

///////////////////////////////////////////////////
void AskForRelayIntra::InitDefaults()
{

	//ep intra parameters
	m_relayIntraParameters.clear();

	//is immediately
	m_isImmediately = false;

	m_isAllowSuppression = false;
}

///////////////////////////////////////////////////
AskForRelayIntra& AskForRelayIntra::operator=(const AskForRelayIntra &other)
{
	if (this == &other)
		return *this;

	InitDefaults();

	//ep intra parameters
	m_relayIntraParameters = other.m_relayIntraParameters;

	//is immediately
	m_isImmediately = other.m_isImmediately;

	m_isAllowSuppression = other.m_isAllowSuppression;

	return *this;
}

///////////////////////////////////////////////////
bool AskForRelayIntra::operator==(const AskForRelayIntra& other) const
{

	//ep intra parameters
	if (m_relayIntraParameters != other.m_relayIntraParameters)
		return false;

	//is immediately
	if (m_isImmediately != other.m_isImmediately)
		return false;
	if (m_isAllowSuppression != other.m_isAllowSuppression)
		return false;
	return true;
}

///////////////////////////////////////////////////
bool AskForRelayIntra::operator!=(const AskForRelayIntra& other) const
{
	return !(*this == other);
}

void AskForRelayIntra::Serialize(CSegment* pSeg) const
{
	*pSeg << (DWORD)m_isImmediately;
	*pSeg << (DWORD)m_isAllowSuppression;

	DWORD size_listRelayIntraParams = m_relayIntraParameters.size();
	*pSeg << size_listRelayIntraParams;
	std::list<RelayIntraParam>::const_iterator it = m_relayIntraParameters.begin();
	for ( ; it != m_relayIntraParameters.end(); ++it)
		it->Serialize(pSeg);
}

void AskForRelayIntra::DeSerialize(CSegment* pSeg)
{
	InitDefaults();

	DWORD dwTmp;
	*pSeg >> dwTmp;
	m_isImmediately = (dwTmp ? true : false);
	*pSeg >> dwTmp;
	m_isAllowSuppression = (dwTmp ? true : false);

	m_relayIntraParameters.clear();
	unsigned int size_listRelayIntraParams = 0;
	*pSeg >> size_listRelayIntraParams;
	for (unsigned int i=0; i < size_listRelayIntraParams; ++i)
	{
		RelayIntraParam temp_listRelayIntraParams;
		temp_listRelayIntraParams.DeSerialize(pSeg);
		m_relayIntraParameters.push_back(temp_listRelayIntraParams);
	}
}

std::string AskForRelayIntra::ToString() const
{
	std::string sRetVal = "RelayIntraParams:\n";
	std::list<RelayIntraParam>::const_iterator it = m_relayIntraParameters.begin();
	for ( ; it != m_relayIntraParameters.end(); ++it)
	{
		sRetVal += it->ToString();
		sRetVal += '\n';
	}
	sRetVal += std::string("Is immediately = ") + (m_isImmediately ? "true" : "false") + "\n";
	sRetVal += std::string("Is suppression allowed = ") + (m_isAllowSuppression ? "true" : "false") + "\n";
	return sRetVal;
}
IntraSuppressionParams::IntraSuppressionParams()
: m_ssrc(0), m_bIsGdr(false), m_bIsSsrc(false), m_bIsSuppressionActive(false), m_bIsRequestReceivedDuringSuppression(false), m_suppressionExpirationTime(0)
{

}

IntraSuppressionParams::IntraSuppressionParams(unsigned int ssrc, bool isGdr, bool isSsrc)
: m_ssrc(ssrc), m_bIsGdr(isGdr), m_bIsSsrc(isSsrc), m_bIsSuppressionActive(false), m_bIsRequestReceivedDuringSuppression(false), m_suppressionExpirationTime(0)
{

}

void IntraSuppressionParams::SetExpirationTime(DWORD expiration)
{
	m_suppressionExpirationTime = expiration;
}

void IntraSuppressionParams::SetSuppressionFlag(bool isActive)
{
	m_bIsSuppressionActive = isActive;
}


void IntraSuppressionParams::ActivateSuppression(DWORD durationInMilliSeconds)
{
	DWORD currentTime = SystemGetTickCount().GetMiliseconds();
	SetExpirationTime(currentTime+durationInMilliSeconds);
	SetSuppressionFlag(true);
	m_bIsRequestReceivedDuringSuppression = false;
}

void IntraSuppressionParams::ClearSuppression()
{
	SetExpirationTime(0);
	SetSuppressionFlag(false);
	m_bIsGdr = m_bIsSsrc = m_bIsRequestReceivedDuringSuppression = false;
}
