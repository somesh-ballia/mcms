#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ProcessList.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "ProcessItemData.h"
#include "Trace.h"
#include "TraceStream.h"


/////////////////////////////////////////////////////////////////////////////
CProcessList::CProcessList()
{
}

/////////////////////////////////////////////////////////////////////////////
CProcessList::CProcessList(const CProcessList &other)
	 : CSerializeObject(other)
{
	*this = other;
}

void CProcessList::CopyValue(CProcessList* pOther)
{
	if (pOther == NULL || (this == pOther))
		return;
	
	vector<CProcessItemData*>& pOtherItemVec = pOther->GetProcessInfo();

	size_t otherLen = pOtherItemVec.size();
	size_t len = m_processItemDataVec.size();

	for (size_t j=0 ; j<len; ++j)
	{
		for (size_t i=0 ; i<otherLen; ++i)
		{
			if (m_processItemDataVec[j]->GetName() == pOtherItemVec[i]->GetName())
			{
				m_processItemDataVec[j]->SetEnabled(pOtherItemVec[i]->IsEnabled());
				//m_processItemDataVec[j]->SetName(pOtherItemVec[j]->GetName());
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
CProcessList& CProcessList::operator = (const CProcessList& other)
{
	if (this == &other)
		return *this;

	size_t len = m_processItemDataVec.size();
	for (size_t i=0 ; i<len; ++i)
	{	
		delete m_processItemDataVec[i];
		m_processItemDataVec[i] = NULL;
	}
	
	len = other.m_processItemDataVec.size();
	m_processItemDataVec.resize(len,NULL);

	for (size_t i=0 ; i<len; ++i)
		m_processItemDataVec[i] = new CProcessItemData(*(other.m_processItemDataVec[i]));

	return *this;
}

/////////////////////////////////////////////////////////////////////////////	
CProcessList::~CProcessList()
{
	size_t len = m_processItemDataVec.size();

	for (size_t i=0; i<len; ++i)
		delete m_processItemDataVec[i];

	m_processItemDataVec.clear();
}

/////////////////////////////////////////////////////////////////////////////
void CProcessList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pProcessListNode = pFatherNode->AddChildNode("PROCESS_LIST");

	size_t len = m_processItemDataVec.size();
	for (size_t i = 0 ; i<len ; ++i)
	{
		CXMLDOMElement* pProcessItemNode = pProcessListNode->AddChildNode("PROCESS_INFO");
		m_processItemDataVec[i]->SerializeXml(pProcessItemNode);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CProcessList::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pProcessListNode=NULL, *pTempNode=NULL;
	
	GET_CHILD_NODE(pActionNode, "PROCESS_LIST", pProcessListNode);
	
	if (pProcessListNode)
	{

		GET_FIRST_CHILD_NODE(pProcessListNode,"PROCESS_INFO",pTempNode);

		unsigned int i =0;
		while(pTempNode)
		{
			if (i >= m_processItemDataVec.size())
				return -1;
			
			CProcessItemData* pProcessItem = m_processItemDataVec[i];
			i ++;
			if (pProcessItem)
			{
				if ((nStatus = pProcessItem->DeSerializeXml(pTempNode, pszError, action)) != STATUS_OK)
					return nStatus;
			}
			else
				DBGPASSERT(100);

			GET_NEXT_CHILD_NODE(pProcessListNode,"PROCESS_INFO",pTempNode);
				
		}


	}
		
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
vector<CProcessItemData*>&	CProcessList::GetProcessInfo()
{
	return m_processItemDataVec;
}

/////////////////////////////////////////////////////////////////////////////
CProcessItemData*	CProcessList::GetProcessItemByProcessIndex(int processIndex)
{
	size_t len = m_processItemDataVec.size();
	for (size_t i = 0 ; i<len; ++i)
	{
		if (m_processItemDataVec[i]->GetName() == (unsigned int)processIndex)
			return m_processItemDataVec[i];
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CProcessList::AddProcessItem(CProcessItemData* processItem)
{
	m_processItemDataVec.push_back(processItem);
}

/////////////////////////////////////////////////////////////////////////////
bool CProcessList::IsSpecifiedProcessChecked(int SpecifiedProcessInd, const char* moduleName, int src_id)
{
	if (!m_processItemDataVec.empty())
	{
		CProcessItemData* pItem = GetProcessItemByProcessIndex(SpecifiedProcessInd);
		if (NULL == pItem)
		{
			//printf("could not find process index name %d in module %s in src_id %d\n",SpecifiedProcessInd,moduleName,src_id);
			TRACEINTO << "Could not find process index_name:" << SpecifiedProcessInd << " in module:" << moduleName << " in src_id:" << src_id;

			// Returns true in any case, workaround for BRIDGE-11198.
			return true;
		}
		return pItem->IsEnabled();
	}

	return false;
}

