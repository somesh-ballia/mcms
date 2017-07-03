#include "AuditEventContainer.h"
#include "psosxml.h"
#include "AuditorDefines.h"
#include "AuditorApi.h"
#include "Macros.h"
#include "psosxml.h"



///////////////////////////////////////////////////////
CAuditEventContainer::CAuditEventContainer(DWORD maxNumEvents)
        : m_MaxEventNum(maxNumEvents)
{
    
}

///////////////////////////////////////////////////////
CAuditEventContainer::~CAuditEventContainer()
{
    while(0 < m_AuditEventQueue.size())
    {        
        CAuditHdrWrapper *pDelete = Dequeue();
        PDELETE(pDelete);
    }
}

///////////////////////////////////////////////////////
void CAuditEventContainer::SerializeXml(CXMLDOMElement *&pFatherNode, DWORD objToken) const
{
    BYTE bChanged = InsertUpdateCntChanged(pFatherNode, objToken);
	if (!bChanged)
	{
		return;
	}

    SerializeXml(pFatherNode);
}

///////////////////////////////////////////////////////
void CAuditEventContainer::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pFileListNode = NULL;
    if(NULL == pFatherNode)
    {
        pFatherNode = new CXMLDOMElement(AUDIT_EVENT_LIST_XML_TAG);
        pFileListNode = pFatherNode;
    }
    else
    {
        pFileListNode = pFatherNode->AddChildNode(AUDIT_EVENT_LIST_XML_TAG);
    }
    
    for(CAuditEventQueue::const_iterator iTer = m_AuditEventQueue.begin();
        iTer != m_AuditEventQueue.end() ;
        iTer++)
    {
        CAuditHdrWrapper *pEvent = *iTer;
        pEvent->SerializeXml(pFileListNode);
    }
}

///////////////////////////////////////////////////////
int CAuditEventContainer::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus = STATUS_OK;
    CXMLDOMElement *pEventListNode = NULL;

//     GET_VALIDATE_CHILD(pActionNode, "AUDIT_EVENT_LIST", pEventListNode);
//     if(NULL == pEventListNode)
//     {
//         return nStatus;
//     }

    CFreeData freeData;
    AUDIT_EVENT_HEADER_S hdr;

	memset(&hdr, 0, sizeof(hdr));
	
    CAuditHdrWrapper hdrWrapper(hdr, freeData);
        
    CXMLDOMElement *pEventNode = NULL;
    GET_FIRST_CHILD_NODE(pActionNode,"AUDIT_EVENT", pEventNode);
    while(NULL != pEventNode)
    {
        nStatus = hdrWrapper.DeSerializeXml(pEventNode, pszError, action);
        if(STATUS_OK != nStatus)
        {
            return nStatus;
        }
        AddEvent(hdrWrapper);
        
        GET_NEXT_CHILD_NODE(pActionNode,"AUDIT_EVENT", pEventNode);
    }
    
    return nStatus;
}

///////////////////////////////////////////////////////
void CAuditEventContainer::AddEvent(const CAuditHdrWrapper & event)
{
    CAuditHdrWrapper *pEvent = new CAuditHdrWrapper(event);
    Enqueue(pEvent);
    
    if(m_AuditEventQueue.size() > m_MaxEventNum)
    {        
        CAuditHdrWrapper *pDelete = Dequeue();
        PDELETE(pDelete);
    }
}

///////////////////////////////////////////////////////
void CAuditEventContainer::Enqueue(CAuditHdrWrapper * pEvent)
{
    m_AuditEventQueue.push_back(pEvent);
}

///////////////////////////////////////////////////////
CAuditHdrWrapper* CAuditEventContainer::Dequeue()
{
    CAuditHdrWrapper *pOldEvent = m_AuditEventQueue.front();
    m_AuditEventQueue.pop_front();
    return pOldEvent;
}

///////////////////////////////////////////////////////
DWORD CAuditEventContainer::GetBigestSequenceNumber()const
{
    DWORD maxSeqNum = 0;
    for(CAuditEventQueue::const_iterator iTer = m_AuditEventQueue.begin();
        iTer != m_AuditEventQueue.end() ;
        iTer++)
    {
        CAuditHdrWrapper *pEvent = *iTer;
        DWORD currentSeqNum = pEvent->GetSequenceNum();
        if(maxSeqNum < currentSeqNum)
        {
            maxSeqNum = currentSeqNum;
        }
    }
    return maxSeqNum;
}

