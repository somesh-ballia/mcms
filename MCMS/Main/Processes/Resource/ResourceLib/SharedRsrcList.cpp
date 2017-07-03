#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "EnumsAndDefines.h"
#include "HelperFuncs.h"
#include "SharedRsrcList.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CSharedRsrcList
////////////////////////////////////////////////////////////////////////////
CSharedRsrcList::CSharedRsrcList()
{
	m_numOfConfIds = 0;
}

/////////////////////////////////////////////////////////////////////////////
CSharedRsrcList::~CSharedRsrcList()
{
}

////////////////////////////////////////////////////////////////////////////
CSharedRsrcList::CSharedRsrcList(const CSharedRsrcList &other) : CSerializeObject(other)
{
	m_numOfConfIds = other.m_numOfConfIds;
	m_confIdsArray = other.m_confIdsArray;
}

/////////////////////////////////////////////////////////////////////////////
int CSharedRsrcList::DeSerializeXml(CXMLDOMElement* pListNode, char* pszError, const char* strAction)
{
	CXMLDOMElement *pNode;
	int nStatus = STATUS_OK;
	DWORD confId;

	GET_FIRST_CHILD_NODE(pListNode, "CONF_ID", pNode);

	while (pNode && m_numOfConfIds < MAX_CONF_IN_LIST)
	{
		GET_VALIDATE(pNode, &confId, _0_TO_DWORD);
		m_confIdsArray.push_back(confId);
		m_numOfConfIds++;
		GET_NEXT_CHILD_NODE(pListNode, "CONF_ID", pNode);
	}
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CSharedRsrcList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	PASSERTMSG(TRUE, "CSharedRsrcList::SerializeXml - Should not be called");
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSharedRsrcList::GetConfIdByIndex(DWORD indx) const
{
	if (indx < m_numOfConfIds)
		return m_confIdsArray[indx];

	return 0xFFFF;
}


////////////////////////////////////////////////////////////////////////////
//                        CSharedRsrcConfReport
////////////////////////////////////////////////////////////////////////////
CSharedRsrcConfReport::CSharedRsrcConfReport()
{
	Init();
}

////////////////////////////////////////////////////////////////////////////
CSharedRsrcConfReport::CSharedRsrcConfReport(const CSharedRsrcList* pSharedRsrcList)
{
	m_availablePPM = 0;

	int numOfConf = pSharedRsrcList->GetNumOfConfIds();
	for (int i = 0; i < numOfConf; ++i)
	{
		ConfPortsList confPorts;
		confPorts.confId = pSharedRsrcList->GetConfIdByIndex(i);
		m_ports.insert(confPorts);
	}
}

////////////////////////////////////////////////////////////////////////////
CSharedRsrcConfReport::~CSharedRsrcConfReport()
{
}

////////////////////////////////////////////////////////////////////////////
void CSharedRsrcConfReport::Init()
{
	m_availablePPM = 0;
}

////////////////////////////////////////////////////////////////////////////
ConfMonitorID CSharedRsrcConfReport::GetConfIdByIndex(DWORD ind) const
{
	DWORD i = 0;
	Ports::const_iterator _iiEnd = m_ports.end();
	for (Ports::const_iterator _ii = m_ports.begin(); _ii != _iiEnd; ++_ii, ++i)
		if (i == ind)
			return _ii->confId;

	return 0xFFFF;
}

////////////////////////////////////////////////////////////////////////////
WORD CSharedRsrcConfReport::GetNumPartiesByConfID(ConfMonitorID confId, ePartyResourceTypes type, eRPRTtypes rprt_type)
{
	ConfPortsList confPorts;
	confPorts.confId = confId;
	Ports::iterator _ii = m_ports.find(confPorts);
	PASSERTSTREAM_AND_RETURN_VALUE(_ii == m_ports.end(), "ConfId:" << confId, 0);

	return _ii->parties[type][rprt_type];
}

////////////////////////////////////////////////////////////////////////////
void CSharedRsrcConfReport::SetNumPartiesByConfID(ConfMonitorID confId, ePartyResourceTypes type, eRPRTtypes rprt_type, WORD num_parties)
{
	ConfPortsList confPorts;
	confPorts.confId = confId;
	Ports::iterator _ii = m_ports.find(confPorts);
	if (_ii == m_ports.end())
	{
		confPorts.parties[type][rprt_type] = num_parties;
		m_ports.insert(confPorts);
	}
	else
	{
		const_cast<ConfPortsList&>(*_ii).parties[type][rprt_type] = num_parties;
	}
}

////////////////////////////////////////////////////////////////////////////
static const char* FixedModeResourceReportNames[] =
{
	"audio",
	"CIF",
	"SD",
	"HD720",
	"HD1080"
};

////////////////////////////////////////////////////////////////////////////
static const char* AutoModeResourceReportNames[] =
{
	"audio",
	"video",
	"HD720p30_video"
};

////////////////////////////////////////////////////////////////////////////
void CSharedRsrcConfReport::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pReportConfNode, *pNumPartiesNode;

	const char** reportNames;
	int numOfTypes = 0;
	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		numOfTypes = NUM_OF_PARTY_RESOURCE_TYPES;
		reportNames = FixedModeResourceReportNames;
	}
	else
	{
		numOfTypes = 3; //audio + video + HD720
		reportNames = AutoModeResourceReportNames;
	}

	Ports::const_iterator _iiEnd = m_ports.end();
	for (Ports::const_iterator _ii = m_ports.begin(); _ii != _iiEnd; ++_ii)
	{
		pReportConfNode = pFatherNode->AddChildNode("RSRC_REPORT_RMX_CONF_LIST");
		pNumPartiesNode = pReportConfNode->AddChildNode("CONF_ID", _ii->confId);

		for (int i = 0; i < numOfTypes; i++)
		{
			pNumPartiesNode = pReportConfNode->AddChildNode("RSRC_REPORT_RMX_CONF");

			pNumPartiesNode->AddChildNode("RSRC_REPORT_ITEM", reportNames[i]);
			WORD portNum = _ii->parties[i][(int)TYPE_OCCUPIED];

			if (i == 2)
			{
				pNumPartiesNode->AddChildNode("OCCUPIED", _ii->parties[e_HD720][(int)TYPE_OCCUPIED]);
				pNumPartiesNode->AddChildNode("AVAILABLE_PORTION_PPM", m_availablePPM);

			}
			else
				pNumPartiesNode->AddChildNode("OCCUPIED", _ii->parties[i][(int)TYPE_OCCUPIED]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
int CSharedRsrcConfReport::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	PASSERTMSG(TRUE, "CSharedRsrcConfReport::DeSerializeXml - Should not be called"); //only for sending to API
	return STATUS_OK;
}
