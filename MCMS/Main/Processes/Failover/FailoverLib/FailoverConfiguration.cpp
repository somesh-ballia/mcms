/*
 * FailoverConfiguration.cpp
 *
 *  Created on: Aug 26, 2009
 *      Author: yael
 */

#include <ostream>
#include <istream>
#include "FailoverConfiguration.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "ConfPartyApiDefines.h"
#include "Segment.h"
#include "FailoverDefines.h"
#include "Trace.h"
#include "TraceStream.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFailoverConfiguration::CFailoverConfiguration()
{
	strncpy(m_strOtherRmxIp,"",IP_ADDRESS_STR_LEN);
	m_strOtherRmxPort = 0;
	m_bIsEnabled = FALSE;
	m_statusType = eFailoverStatusNone;
	m_HotBackupType=eMaster;
	m_bManualTrigger = FALSE;

	FAILOVER_EVENT_INFO eventInfo;

	eventInfo.eventType = eFailoverMgtPortFailure;
	eventInfo.bEnabled  = TRUE;
	eventInfo.szDescription= "Lost connection with management port";
	m_mapEventTrigger[eFailoverMgtPortFailure] = eventInfo;
	
	eventInfo.eventType = eFailoverMpmCardFailure;
	eventInfo.bEnabled  = FALSE;
	eventInfo.szDescription= "Lost Connection with MPM Board";
	m_mapEventTrigger[eFailoverMpmCardFailure] = eventInfo;

	eventInfo.eventType = eFailoverSignalPortFailure;
	eventInfo.bEnabled  = FALSE;
	eventInfo.szDescription= "Lost connection with Signaling port";
	m_mapEventTrigger[eFailoverSignalPortFailure] = eventInfo;	

	eventInfo.eventType = eFailoverIsdnCardFailure;
	eventInfo.bEnabled	= FALSE;
	eventInfo.szDescription= "Lost connection with ISDN card";
	m_mapEventTrigger[eFailoverIsdnCardFailure] = eventInfo;

}

/////////////////////////////////////////////////////////////////////////////
CFailoverConfiguration& CFailoverConfiguration::operator = (const CFailoverConfiguration &other)
{
	m_bIsEnabled = other.m_bIsEnabled;
	strncpy(m_strOtherRmxIp,other.m_strOtherRmxIp,IP_ADDRESS_STR_LEN);
	m_strOtherRmxPort = other.m_strOtherRmxPort;
	m_HotBackupType = other.m_HotBackupType;

	m_statusType = other.m_statusType;

	m_mapEventTrigger = other.m_mapEventTrigger;
	m_bManualTrigger = other.m_bManualTrigger;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CFailoverConfiguration::~CFailoverConfiguration()
{

}

///////////////////////////////////////////////////////////////////////////
void CFailoverConfiguration::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pTempNode = NULL;
	CXMLDOMElement *pTempListNode = NULL;
	CXMLDOMElement *pTempInfoNode = NULL;

	eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
	
	pTempNode = pFatherNode->AddChildNode("HOTBACKUP_DATA");
	pTempNode->AddChildNode("HOTBACKUP_TYPE",m_HotBackupType,FAILOVER_TYPE);
	pTempNode->AddChildNode("HOTBACKUP_ENABLED",m_bIsEnabled,_BOOL);
	pTempNode->AddChildNode("PAIR_IP",m_strOtherRmxIp);
	pTempNode->AddChildNode("PAIR_PORT",m_strOtherRmxPort,_0_TO_DWORD);
	pTempNode->AddChildNode("STATUS",m_statusType,FAILOVER_STATUS_TYPE);

	pTempListNode = pTempNode->AddChildNode("HOTBACKUP_EVENT_LIST");

	std::map<DWORD,FAILOVER_EVENT_INFO>::const_iterator it;

	for(it = m_mapEventTrigger.begin();it != m_mapEventTrigger.end();it++)
	{
	    if (eProductTypeGesher == curProductType || eProductTypeEdgeAxis == curProductType)
		{
			if (eFailoverMpmCardFailure == it->second.eventType ||
				eFailoverSignalPortFailure == it->second.eventType ||
				eFailoverIsdnCardFailure == it->second.eventType)
			{
				continue;
			}
		}
		if (eProductTypeNinja == curProductType)
		{
			/*BRIDGE-9355:  For NINJA we support signal port lost connection trigger*/
			if (eFailoverMpmCardFailure == it->second.eventType)
			{
				continue;
			}
		}	


        pTempInfoNode = pTempListNode->AddChildNode("EVENT_INFO");

		pTempInfoNode->AddChildNode("EVENT_NAME",it->second.eventType,FAILOVER_EVENT_TRIGGER);
		pTempInfoNode->AddChildNode("CHECKED",it->second.bEnabled,_BOOL);
		pTempInfoNode->AddChildNode("EVENT_DESCRIPTION",it->second.szDescription);
	}	
	pTempNode->AddChildNode("MANUAL_TRIGGER",FALSE,_BOOL);		/*always tell false for EMA*/
}

//////////////////////////////////////////////////////////
int CFailoverConfiguration::DeSerializeXml(CXMLDOMElement *pNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pFailoverDataNode=NULL;

	CXMLDOMElement *pEventListNode=NULL;
	CXMLDOMElement *pEventInfoNode=NULL;

	GET_CHILD_NODE(pNode, "HOTBACKUP_DATA", pFailoverDataNode);
	if(pFailoverDataNode != NULL)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverConfiguration::DeSerializeXml: child nodes";
		GET_VALIDATE_CHILD(pFailoverDataNode,"HOTBACKUP_TYPE",&m_HotBackupType,FAILOVER_TYPE);
		GET_VALIDATE_CHILD(pFailoverDataNode,"HOTBACKUP_ENABLED",&m_bIsEnabled,_BOOL);
		GET_VALIDATE_CHILD(pFailoverDataNode,"PAIR_IP",m_strOtherRmxIp,_1_TO_IP_ADDRESS_LENGTH);
		GET_VALIDATE_CHILD(pFailoverDataNode,"PAIR_PORT",&m_strOtherRmxPort,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pFailoverDataNode,"STATUS",&m_statusType,FAILOVER_STATUS_TYPE);

		//-------------- Event list --------------
		GET_CHILD_NODE(pFailoverDataNode, "HOTBACKUP_EVENT_LIST", pEventListNode);
		
		if (pEventListNode)
		{
			GET_FIRST_CHILD_NODE(pEventListNode,"EVENT_INFO",pEventInfoNode);
			while(pEventInfoNode)
			{
				FAILOVER_EVENT_INFO eventInfo;
				
				GET_VALIDATE_CHILD(pEventInfoNode,"EVENT_NAME",&eventInfo.eventType,FAILOVER_EVENT_TRIGGER);
				GET_VALIDATE_CHILD(pEventInfoNode,"CHECKED",&eventInfo.bEnabled,_BOOL);
				GET_VALIDATE_ASCII_CHILD(pEventInfoNode,"EVENT_DESCRIPTION",eventInfo.szDescription,_0_TO_FAILOVER_STRING_LENGTH);

				m_mapEventTrigger[eventInfo.eventType] = eventInfo;
				
				GET_NEXT_CHILD_NODE(pEventListNode,"EVENT_INFO",pEventInfoNode);
			}
		}
		GET_VALIDATE_CHILD(pFailoverDataNode,"MANUAL_TRIGGER",&m_bManualTrigger,_BOOL);
	}
	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CFailoverConfiguration::DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* strAction)
{
	return DeSerializeXml(pNode, pszError);
}
