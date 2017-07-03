#include "SystemTime.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StructTm.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"



/////////////////////////////////////////////////////////////////////////////
// CSystemTime

const string CSystemTime::NA_IPV6_ADDRESS = "::";

CSystemTime::CSystemTime()
{
	m_pMCUTime = new CStructTm;
	m_GMT_offset_sign=0;
	m_GMT_offset=0;
	//The following two vars are active only in case of XP
	int i=0;
	m_bIsNTP = 0;
	for (i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		m_NTP_IP_ADDRESS[i]= "";//0xFFFFFFFF;
		m_NTP_IPV6_ADDRESS[i][0] = '\0';
		m_NtpServerStatus[i]=eNtpServerNotConfigured;
		m_numFailuresSinceConnecting[i]= 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
CSystemTime::CSystemTime(const CSystemTime &other)
:CSerializeObject(other)
{
	if( other.m_pMCUTime==NULL)
		m_pMCUTime=NULL;
	else
		m_pMCUTime = new CStructTm(*other.m_pMCUTime);
	m_GMT_offset_sign=other.m_GMT_offset_sign;
	m_GMT_offset=other.m_GMT_offset;

	int i=0;
	m_bIsNTP = other.m_bIsNTP;
	for (i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		m_NTP_IP_ADDRESS[i]=other.m_NTP_IP_ADDRESS[i];
		strncpy(m_NTP_IPV6_ADDRESS[i], other.m_NTP_IPV6_ADDRESS[i], IPV6_ADDRESS_LEN);
		m_NtpServerStatus[i]=other.m_NtpServerStatus[i];
		m_numFailuresSinceConnecting[i]=other.m_numFailuresSinceConnecting[i];
	}

	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CSystemTime& CSystemTime::operator = (const CSystemTime &other)
{
	if(this == &other){
		return *this;
	}

	PrintParams("CSystemTime::operator = - before assigning");


	PDELETE(m_pMCUTime);
	if( other.m_pMCUTime==NULL)
		m_pMCUTime=NULL;
	else
		m_pMCUTime = new CStructTm(*other.m_pMCUTime);
	m_GMT_offset_sign=other.m_GMT_offset_sign;
	m_GMT_offset=other.m_GMT_offset;

	int i=0;
	m_bIsNTP = other.m_bIsNTP;
	for (i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		m_NTP_IP_ADDRESS[i]=other.m_NTP_IP_ADDRESS[i];
		strncpy(m_NTP_IPV6_ADDRESS[i], other.m_NTP_IPV6_ADDRESS[i], IPV6_ADDRESS_LEN);
		m_NtpServerStatus[i]=other.m_NtpServerStatus[i];
		m_numFailuresSinceConnecting[i]=other.m_numFailuresSinceConnecting[i];

	}

	m_updateCounter = other.m_updateCounter;
	PrintParams("CSystemTime::operator = - after assigning");

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSystemTime::~CSystemTime()
{
	PDELETE(m_pMCUTime);
}

/////////////////////////////////////////////////////////////////////////////
void CSystemTime::SetParams(const CSystemTime &other)
{
	PrintParams("CSystemTime::SetParams - before setting");


	m_GMT_offset_sign=other.m_GMT_offset_sign;
	m_GMT_offset=other.m_GMT_offset;

	int i=0;
	m_bIsNTP = other.m_bIsNTP;
	for (i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		m_NTP_IP_ADDRESS[i]=other.m_NTP_IP_ADDRESS[i];
		strncpy(m_NTP_IPV6_ADDRESS[i], other.m_NTP_IPV6_ADDRESS[i], IPV6_ADDRESS_LEN);
		m_NtpServerStatus[i]=other.m_NtpServerStatus[i];
		m_numFailuresSinceConnecting[i] = other.m_numFailuresSinceConnecting[i];
	}

	m_updateCounter = other.m_updateCounter;
	PrintParams("CSystemTime::SetParams - after setting");
}


////////////////////////////////////////////////////////////////////////////
void CSystemTime::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	char dateStr[128];
	char ipAddressStr[IP_ADDRESS_LEN];
	CXMLDOMElement* pMcuTimeNode;


	//	PlcmTimeConfig tc;
	//	tc.m_useNtpServer = m_bIsNTP;
	//	sprintf(dateStr, "%04u-%02u-%02uT%02u:%02u:%02u",
	//			m_pMCUTime->m_year, m_pMCUTime->m_mon, m_pMCUTime->m_day,
	//			m_pMCUTime->m_hour, m_pMCUTime->m_min, m_pMCUTime->m_sec);
	//	tc.m_currentTime = dateStr;
	//	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	//	{
	//		SystemDWORDToIpString(GetNTP_IPAddress(i), ipAddressStr);
	//		tc.m_ntpServerList.push_back(ipAddressStr);
	//	}
	//	tc.WriteToXmlFile((MCU_TMP_DIR+"/PlcmTimeConfig.txt").c_str(), true);
	//	std::ostringstream xmlTosend;
	//	xmlTosend << tc;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("MCU_TIME");
		pMcuTimeNode = pFatherNode;
	}
	else
	{
		pMcuTimeNode = pFatherNode->AddChildNode("MCU_TIME");
	}

	pMcuTimeNode->AddChildNode("MCU_BASE_TIME",*m_pMCUTime);
	pMcuTimeNode->AddChildNode("GMT_OFFSET_SIGN",m_GMT_offset_sign,_BOOL);
	pMcuTimeNode->AddChildNode("GMT_OFFSET",m_GMT_offset);
	pMcuTimeNode->AddChildNode("IS_NTP" , m_bIsNTP, _BOOL);

	// --- for compatibility reasons
	pMcuTimeNode->AddChildNode("NTP_IP_ADDRESS", m_NTP_IP_ADDRESS[0].c_str(),ONE_LINE_BUFFER_LENGTH);
	// ---

	for (int i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		CXMLDOMElement* pNtpServerParamsNode = pMcuTimeNode->AddChildNode("NTP_SERVERS_PARAMETERS");

		pNtpServerParamsNode->AddChildNode("NTP_IP_ADDRESS", m_NTP_IP_ADDRESS[i].c_str(), ONE_LINE_BUFFER_LENGTH);
		pNtpServerParamsNode->AddChildNode("NTP_IPV6_ADDRESS", m_NTP_IPV6_ADDRESS[i], _0_TO_IPV6_ADDRESS_LENGTH);
		pNtpServerParamsNode->AddChildNode("NTP_SERVER_STATUS", (WORD)(m_NtpServerStatus[i]), NTP_SERVER_STATUS_ENUM);
	}
}

///////////////////////////////////////////////////////////////////////////////
int  CSystemTime::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	PrintParams("CSystemTime::DeSerializeXml - before deserializing");


	int nStatus=STATUS_OK;

	CXMLDOMElement* pMcuTimeNode;
	char* ParentNodeName;
	BOOL bReadTime=TRUE;


	pActionNode->get_nodeName(&ParentNodeName);
	if(!strcmp(ParentNodeName, "MCU_TIME"))
	{
		pMcuTimeNode=pActionNode;
		bReadTime=FALSE;//no need to read time not relevant when loading from file
	}
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "MCU_TIME", pMcuTimeNode);



	if (pMcuTimeNode)
	{
		GET_VALIDATE_CHILD(pMcuTimeNode,"IS_NTP",&m_bIsNTP,_BOOL);
		if(bReadTime)
		{
			if(m_bIsNTP == false)
				GET_VALIDATE_CHILD(pMcuTimeNode,"MCU_BASE_TIME",m_pMCUTime,DATE_TIME);

		}

		GET_VALIDATE_CHILD(pMcuTimeNode,"GMT_OFFSET_SIGN",&m_GMT_offset_sign,_BOOL);

		GET_VALIDATE_CHILD(pMcuTimeNode,"GMT_OFFSET",&m_GMT_offset,_0_TO_BYTE);


		CXMLDOMElement *pIPNode;
		int i=0;

		for (i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
		{
			//m_NTP_IP_ADDRESS[i]=0xFFFFFFFF;
			m_NTP_IP_ADDRESS[i]="";
			strcpy(m_NTP_IPV6_ADDRESS[i], NA_IPV6_ADDRESS.c_str());

		}

		// --- for compatibility reasons
		GET_VALIDATE_CHILD(pMcuTimeNode, "NTP_IP_ADDRESS", m_NTP_IP_ADDRESS[0], ONE_LINE_BUFFER_LENGTH);
		// ---

		for (i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
		{
			m_NtpServerStatus[i] = eNtpServerNotConfigured;
		}
		GET_FIRST_CHILD_NODE(pMcuTimeNode, "NTP_SERVERS_PARAMETERS", pIPNode);

		i=0;
		while ( (pIPNode) && (i < NTP_MAX_NUM_OF_SERVERS) )
		{
			GET_VALIDATE_CHILD(pIPNode, "NTP_IP_ADDRESS",    m_NTP_IP_ADDRESS[i], ONE_LINE_BUFFER_LENGTH);

			CXMLDOMElement *pNTPIPV6Child = NULL;
			GET_CHILD_NODE(pIPNode, "NTP_IPV6_ADDRESS", pNTPIPV6Child);

			if (pNTPIPV6Child)
			{
				GET_VALIDATE_CHILD(pIPNode, "NTP_IPV6_ADDRESS",  m_NTP_IPV6_ADDRESS[i],        _0_TO_IPV6_ADDRESS_LENGTH);
			}
			else
			{
				TRACESTR(eLevelInfoNormal) << "\nNTP_IPV6_ADDRESS does not exist. Setting :: instead";
				strcpy(m_NTP_IPV6_ADDRESS[i], NA_IPV6_ADDRESS.c_str());

			}


			GET_VALIDATE_CHILD(pIPNode, "NTP_SERVER_STATUS", (WORD*)(&m_NtpServerStatus[i]), NTP_SERVER_STATUS_ENUM);

			GET_NEXT_CHILD_NODE(pMcuTimeNode, "NTP_SERVERS_PARAMETERS", pIPNode);
			i++;
		}
	}


	PrintParams("CSystemTime::DeSerializeXml - after deserializing");

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CSystemTime::GetGMTOffsetSign () const
{
	return m_GMT_offset_sign;
}


/////////////////////////////////////////////////////////////////////////////
BYTE  CSystemTime::GetGMTOffset () const
{
	return m_GMT_offset;
}


/////////////////////////////////////////////////////////////////////////////
void  CSystemTime::GetGMTOffset (BYTE & GMT_offset_sign, // 0 for '-' 1 for '+'
		BYTE & GMT_offset_hours, //  0-15 hours
		BYTE & GMT_offset_minutes // 0,5,10,15,...,55 in minutes
) const
{
	GMT_offset_sign = m_GMT_offset_sign;
	GMT_offset_hours = m_GMT_offset % 16;
	GMT_offset_minutes = (m_GMT_offset / 16) * 5;
}



/////////////////////////////////////////////////////////////////////////////
const CStructTm*  CSystemTime::GetMCUTime()  const
{
	return m_pMCUTime;
}


/////////////////////////////////////////////////////////////////////////////
const char*  CSystemTime::NameOf() const
{
	return "CSystemTime";
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CSystemTime::GetIsNTP () const
{
	return m_bIsNTP;
}

/////////////////////////////////////////////////////////////////////////////
void  CSystemTime::SetIsNTP (const BYTE  Is_NTP)
{
	m_bIsNTP = Is_NTP;
}

/////////////////////////////////////////////////////////////////////////////
std::string  CSystemTime::GetNTP_IPAddress (int index) const
{
	if (index >= NTP_MAX_NUM_OF_SERVERS)
		return "";
	else
		return m_NTP_IP_ADDRESS[index];
}

/////////////////////////////////////////////////////////////////////////////
const char*  CSystemTime::GetNTP_IPv6_Address (int index) const
{
	if (index >= NTP_MAX_NUM_OF_SERVERS)
		return "";
	else
		return m_NTP_IPV6_ADDRESS[index];
}

bool CSystemTime::IsNTPIpv6AddressesEmpty() const
{
	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	{
		if (m_NTP_IPV6_ADDRESS[i] && strcmp(m_NTP_IPV6_ADDRESS[i],"::") != 0)
			return false;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////
void CSystemTime::UpdateCurrentTime()
{
	CStructTm *pMCUTime = new CStructTm();
	SystemGetTime(*pMCUTime);
	*m_pMCUTime=*pMCUTime;
	delete pMCUTime;
}

/////////////////////////////////////////////////////////////////////////////
eNtpServerStatus CSystemTime::GetNtpServerStatus (int index) const
{
	if (index < 0 || index >= NTP_MAX_NUM_OF_SERVERS)
		return eNtpServerNotConfigured;
	else
		return m_NtpServerStatus[index];
}

/////////////////////////////////////////////////////////////////////////////
void CSystemTime::SetNtpServerStatus(int index, eNtpServerStatus serverStatus)
{
	if ( (index >= 0) && (index < NTP_MAX_NUM_OF_SERVERS) )
		m_NtpServerStatus[index] = serverStatus;
}

/////////////////////////////////////////////////////////////////////////////

WORD 	CSystemTime::GetNumFailuresSinceConnecting(int index) const
{

	if (index < 0 || index >= NTP_MAX_NUM_OF_SERVERS)
	{
		return 0;
	}
	else
	{
		return m_numFailuresSinceConnecting[index];
	}
}

/////////////////////////////////////////////////////////////////////////////

void	CSystemTime::SetNumFailuresSinceConnecting(int index, WORD numFailures)
{
	if ( (index >= 0) && (index < NTP_MAX_NUM_OF_SERVERS) )
	{
		m_numFailuresSinceConnecting[index] = numFailures;
	}
}



/////////////////////////////////////////////////////////////////////////////
void CSystemTime::PrintParams(string theCaller)
{
	// ===== 1. preparations
	char timeBuffer[256];
	m_pMCUTime->DumpToBuffer(timeBuffer);

	char   cOffsetSign	= ( m_GMT_offset_sign	?	'+'		:	'-'		);
	string sIsNtp		= ( m_bIsNTP			?	"yes"	:	"no"	);

	string sNtpServers = "";
	char sTmp[256];
	char ipAddressStr[IP_ADDRESS_LEN];
	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	{
		//SystemDWORDToIpString(m_NTP_IP_ADDRESS[i], ipAddressStr);

		snprintf( sTmp, sizeof(sTmp), "\nServer %d: ipv4 address: %s, ipv6 address: %s status: %s numOfFailureSinceConnecting: %d",
				i, m_NTP_IP_ADDRESS[i].c_str(), m_NTP_IPV6_ADDRESS[i], GetNtpServerStatusStr(m_NtpServerStatus[i]), m_numFailuresSinceConnecting[i] );

		sNtpServers += sTmp;

	}


	// ===== 2. printing
	TRACESTR(eLevelInfoNormal) << "\nCSystemTime::PrintParams (caller: " <<  theCaller << ")"
			<< "\nDate & Time (GMT): " <<  timeBuffer
			<< "\nGMT offset:  " << cOffsetSign << (int)m_GMT_offset
			<< "\nNTP enabling: " << sIsNtp
			<< sNtpServers;
}

bool CSystemTime::operator == (const CSystemTime& other)
{

	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	{
		if(m_NTP_IP_ADDRESS[i] != other.m_NTP_IP_ADDRESS[i])
		{
			return false;
		}
	}
	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	{
		if (m_NTP_IPV6_ADDRESS[i] && strcmp(m_NTP_IPV6_ADDRESS[i], other.m_NTP_IPV6_ADDRESS[i]))
			return false;
	}

	if(m_bIsNTP != other.m_bIsNTP)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSystemTime::m_bIsNTP != other.m_bIsNTP";
		return false;
	}
	if(m_GMT_offset_sign != other.m_GMT_offset_sign)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSystemTime::m_GMT_offset_sign != other.m_GMT_offset_sign";
		return false;
	}

	if(m_GMT_offset != other.m_GMT_offset)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSystemTime::m_GMT_offset != other.m_GMT_offset";
		return false;
	}

	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	{
		if(m_NtpServerStatus[i] != other.m_NtpServerStatus[i])
		{
			TRACESTR(eLevelInfoNormal) << "\nCSystemTime::m_NtpServerStatus[i] != other.m_NtpServerStatus[i]";
			return false;
		}
	}
	for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
	{
		if(m_numFailuresSinceConnecting[i] != other.m_numFailuresSinceConnecting[i])
		{
			TRACESTR(eLevelInfoNormal) << "\nCSystemTime::m_numFailuresSinceConnecting[i] != other.m_numFailuresSinceConnecting[i]";
			return false;
		}
	}
	return true;
}

bool CSystemTime::operator != (const CSystemTime& other)
{
	return !operator ==(other);
}


