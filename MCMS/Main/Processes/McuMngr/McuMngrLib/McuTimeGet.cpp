// CMcuStateGet.cpp: implementation of the CMcuStateGet class.
//////////////////////////////////////////////////////////////////////////


#include "McuTimeGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StructTm.h"
#include "SystemTime.h"
#include "PlcmTimeConfig.h"
#include "StringsLen.h"
#include "ApiStatuses.h"
#include "psosxml.h"
#include "TraceStream.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuTimeGet::CMcuTimeGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuTimeGet& CMcuTimeGet::operator = (const CMcuTimeGet &other)
{
	if(this == &other){
	    return *this;
	}


	TRACESTR(eLevelInfoNormal) << "inside CCMcuTimeGet::other.operator =" << other.m_apiFormat;
	m_apiFormat = other.m_apiFormat;
//		m_apiFormat = eRestApi;


	TRACESTR(eLevelInfoNormal) << "inside CCMcuTimeGet::operator =" << m_apiFormat;
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuTimeGet::~CMcuTimeGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CMcuTimeGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CSystemTime Time;
	m_pProcess->GetMcuTime(Time);
	TRACESTR(eLevelInfoNormal) << "CMcuTimeGet::SerializeXml, m_apiFormat=" << m_apiFormat;
//	m_apiFormat=eRestApi;
	if (m_apiFormat==eRestApi)
	{
		PlcmTimeConfig plcmTime;
		plcmTime.m_entityTag = Time.GetUpdateCounter();
		plcmTime.m_useNtpServer = Time.GetIsNTP();
		const CStructTm* pMCUTime = Time.GetMCUTime();
		char dateStr[128];
		sprintf(dateStr, "%04u-%02u-%02uT%02u:%02u:%02u",
				pMCUTime->m_year, pMCUTime->m_mon, pMCUTime->m_day,
				pMCUTime->m_hour, pMCUTime->m_min, pMCUTime->m_sec);
		plcmTime.m_currentTime = dateStr;

		char ipAddressStr[IP_ADDRESS_LEN];
		for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
		{
			if ("" ==  Time.GetNTP_IPAddress(i))
				continue;
			//SystemDWORDToIpString(Time.GetNTP_IPAddress(i), ipAddressStr);
			//ipAddressStr = Time.GetNTP_IPAddress(i);
			//plcmTime.m_ntpServerList.push_back(ipAddressStr);
			plcmTime.m_ntpServerList.push_back(Time.GetNTP_IPAddress(i).c_str());
		}
		m_apiBaseObjRest = ApiBaseObjectPtr(&plcmTime);
	}
	else
	{
		Time.SerializeXml(pActionsNode);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuTimeGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
//	BYTE tmp = (BYTE)m_apiFormat;
//	GET_VALIDATE_CHILD(pNode, "API_FORMAT", &tmp, API_FORMAT_TYPE_ENUM); //Eran M new Format for Generally all Objects

	WORD tmp = 0;
	GET_VALIDATE_CHILD(pNode,"API_FORMAT",&tmp,API_FORMAT_TYPE_ENUM);

	if (nStatus == STATUS_OK)
	{
		m_apiFormat = (eApiFormat)tmp;
	}
//	return nStatus;
	return STATUS_OK;
}
