// CMediaRecordinglGet.cpp: implementation of the CRsrcDetailGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//
//========   ==============   =====================================================================

#include "CollectorInfo.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "TraceStream.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"

///////////////////////////////////////////////////////////////////////////////////
CInfoTimeInterval::CInfoTimeInterval(): m_start(0,0,0,0,0,0), m_end(0,0,0,0,0,0)
{
	for (int i=0; i<MAX_NUM_OF_COLLECTED_INFO; i++)
	{
		m_collectInfo[i].SetMarkForCollection(FALSE);
		m_collectInfo[i].SetCollectingType((eCollectingType)i);
	}
}
///////////////////////////////////////////////////////////////////////////////////
CInfoTimeInterval::CInfoTimeInterval(CStructTm& start, CStructTm& end):m_start(start), m_end(end)
{
	for (int i=0; i<eCollectingType_network_traffic_capture; i++)
	{
		m_collectInfo[i].SetMarkForCollection(FALSE);
	}
	m_collectInfo[eCollectingType_network_traffic_capture].SetMarkForCollection(FALSE);
	m_collectInfo[eCollectingType_participants_recordings].SetMarkForCollection(FALSE);
	m_collectInfo[eCollectingType_nids].SetMarkForCollection(FALSE);

}



///////////////////////////////////////////////////////////////////////////////////
CInfoTimeInterval::CInfoTimeInterval(const CInfoTimeInterval& rhs):CSerializeObject(rhs),
                                                                   m_start(rhs.m_start), m_end(rhs.m_end)
{
	for (int i=0; i<MAX_NUM_OF_COLLECTED_INFO; i++)
		m_collectInfo[i] = rhs.m_collectInfo[i];
}
////////////////////////////////////////////////////////////////////////////////////
CInfoTimeInterval::~CInfoTimeInterval()
{
}
////////////////////////////////////////////////////////////////////////////////////
const CInfoTimeInterval& CInfoTimeInterval::operator=(const CInfoTimeInterval& other)
{
    if(this != &other)
    {
        m_start = other.m_start;
        m_end   = other.m_end;

    	for (int i=0; i<MAX_NUM_OF_COLLECTED_INFO; i++)
    		m_collectInfo[i] = other.m_collectInfo[i];
    }
    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////
void   CInfoTimeInterval::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("START_TIME", m_start);
	pFatherNode->AddChildNode("END_TIME", m_end);

	CXMLDOMElement* pCollectingInfoList = pFatherNode->AddChildNode("COLLECT_INFO_LIST");

	if (IsJitcMode())
 	{
		m_collectInfo[eCollectingType_nids].SerializeXml(pCollectingInfoList);	//only NIDS files should be collected
	}
	else
	{
		for (int i=0; i<MAX_NUM_OF_COLLECTED_INFO; i++)
		{
			eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

			m_collectInfo[i].SerializeXml(pCollectingInfoList);
		}
	}
};
//////////////////////////////////////////////////////////////////////////////////////
int    CInfoTimeInterval::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action)
{
	int  nStatus = STATUS_OK;
	int i=0;
	CXMLDOMElement *pChildNode = NULL;
	CXMLDOMElement *pCollectInfoNode, *pCollectInfoListNode = NULL;

    GET_VALIDATE_CHILD(pActionNode,"START_TIME",&m_start,DATE_TIME);
    GET_VALIDATE_CHILD(pActionNode,"END_TIME",&m_end,  DATE_TIME);

    GET_CHILD_NODE(pActionNode,"COLLECT_INFO_LIST",pCollectInfoListNode);

    if (pCollectInfoListNode)
    {
        GET_FIRST_CHILD_NODE(pCollectInfoListNode,"COLLECT_TYPE",pCollectInfoNode);

        while (pCollectInfoNode)
        {
        	DWORD tmp = 0;
        	GET_VALIDATE_CHILD(pCollectInfoNode, "TYPE", &tmp, COLLECTING_TYPE_ENUM);
        	// protect array overflow check that tmp smaller than m_collectInfo size
        	if (tmp<MAX_NUM_OF_COLLECTED_INFO)
        		m_collectInfo[tmp].DeSerializeXml(pCollectInfoNode, pszError, NULL);
        	else
        		return STATUS_FAIL;

        	GET_NEXT_CHILD_NODE(pCollectInfoListNode,"COLLECT_TYPE",pCollectInfoNode);
        }
    }

	return nStatus;
}
//////////////////////////////////////////////////////////////////////////////////////
BOOL CInfoTimeInterval::GetIsMarkForCollection (eCollectingType collecting_type)
{
	return m_collectInfo[collecting_type].GetMarkForCollection();
}
//////////////////////////////////////////////////////////////////////////////////////
void CInfoTimeInterval::SetStartAndEndTimeToCurrentTime()
{
	TRACESTR(eLevelInfoNormal) << "CInfoTimeInterval::SetStartAndEndTimeToCurrentTime";
	SystemGetTime(m_start);
	SystemGetTime(m_end);
}

//////////////////////////////////////////////////////////////////////////////////////
void CInfoTimeInterval::RestartCollectingDetails()
{
	TRACESTR(eLevelInfoNormal) << "CInfoTimeInterval::RestartCollectingDetails";

	for (int i=0; i<MAX_NUM_OF_COLLECTED_INFO; i++)
	{

		if (i == eCollectingType_processInfo)
		{
			m_collectInfo[i].SetMarkForCollection(FALSE);
		}
		else
		{
			m_collectInfo[i].SetMarkForCollection(TRUE);

		}
		// for jitec  we need  eCollectingType_nids

		m_collectInfo[i].SetCollectingType((eCollectingType)i);
	}
}


//////////////////////////////////////////////////////////////////////////////////////
BOOL CInfoTimeInterval::IsJitcMode() const
{
	BOOL bJitcMode = FALSE;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", bJitcMode);
	return bJitcMode;
}



//////////////////////////////////////////////////////////////////////////////////////
// New class: CCollectInfo
//////////////////////////////////////////////////////////////////////////////////////
CCollectInfo::CCollectInfo()
{
	m_CollectingType = eCollectingType_processInfo;
	m_bMarkForCollection = FALSE;
	m_estimatedSize = 0;
}

//////////////////////////////////////////////////////////////////////////////////////
CCollectInfo::CCollectInfo(const CCollectInfo& rhs):CSerializeObject(rhs),
		m_CollectingType(rhs.m_CollectingType), m_bMarkForCollection(rhs.m_bMarkForCollection), m_estimatedSize(rhs.m_estimatedSize)
{
}

//////////////////////////////////////////////////////////////////////////////////////
CCollectInfo::~CCollectInfo()
{
}
////////////////////////////////////////////////////////////////////////////////////
const CCollectInfo& CCollectInfo::operator=(const CCollectInfo& other)
{
    if(this != &other)
    {
    	m_CollectingType = other.m_CollectingType;
    	m_bMarkForCollection = other.m_bMarkForCollection;
    	m_estimatedSize = other.m_estimatedSize;
    }
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////
void CCollectInfo::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pCollectTypeNode = pFatherNode->AddChildNode("COLLECT_TYPE");
	pCollectTypeNode->AddChildNode("TYPE", m_CollectingType, COLLECTING_TYPE_ENUM);
	pCollectTypeNode->AddChildNode("MARK_FOR_COLLECTION", m_bMarkForCollection, _BOOL);
//	pFatherNode->AddChildNode("ESTIMATED_SIZE", m_estimatedSize);
}

////////////////////////////////////////////////////////////////////////////////////
int  CCollectInfo::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action)
{
	int nStatus = STATUS_OK;

	DWORD tmp = 0;
	GET_VALIDATE_CHILD(pActionNode, "TYPE", &tmp, COLLECTING_TYPE_ENUM);
	m_CollectingType = (eCollectingType)tmp;

	GET_VALIDATE_CHILD(pActionNode, "MARK_FOR_COLLECTION", &m_bMarkForCollection, _BOOL);

//	GET_VALIDATE_CHILD(pActionNode, "ESTIMATED_SIZE", &m_estimatedSize, _0_TO_DWORD);

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////
BOOL CCollectInfo::GetMarkForCollection()
{
	return m_bMarkForCollection;
}

////////////////////////////////////////////////////////////////////////////////////
void CCollectInfo::SetMarkForCollection(BYTE mark_for_collection)
{
	m_bMarkForCollection = mark_for_collection;
}

////////////////////////////////////////////////////////////////////////////////////
void CCollectInfo::SetCollectingType(eCollectingType collecting_type)
{
	m_CollectingType = collecting_type;
}
