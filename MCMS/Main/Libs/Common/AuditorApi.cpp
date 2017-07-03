#include "AuditorApi.h"
#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "ObjString.h"




extern const char* MainEntityToString(APIU32 entityType);







/*--------------------------------------------------------------------------------------
  CFreeDataNode implementation
--------------------------------------------------------------------------------------*/

CFreeDataNode::CFreeDataNode()
{
    m_DataType = eFreeDataTypeText;
}

CFreeDataNode::CFreeDataNode(const string & xmlTag, eFreeDataType dataType, const string & data)
        : m_XmlTag(xmlTag), m_DataType(dataType), m_Data(data)
{
}

CFreeDataNode::~CFreeDataNode()
{
}

void CFreeDataNode::operator = (const CFreeDataNode & other)
{
    m_XmlTag = other.m_XmlTag;
    m_Data = other.m_Data;
    m_DataType = other.m_DataType;
}

void CFreeDataNode::Serialize(WORD format, CSegment *pSeg)
{
    *pSeg << m_XmlTag;
    *pSeg << (DWORD)m_DataType;
    *pSeg << m_Data;
}

void CFreeDataNode::DeSerialize(WORD format, CSegment *pSeg)
{
    *pSeg >> m_XmlTag;

    DWORD tmp = 0;
    *pSeg >> tmp;
    m_DataType = (eFreeDataType)tmp;

    *pSeg >> m_Data;
}

void CFreeDataNode::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    if(!m_XmlTag.empty())
    {
        CObjString::ReplaceChar((char*)m_XmlTag.c_str(), ' ', '_');
        CObjString::ReplaceChar((char*)m_XmlTag.c_str(), '[', '_');
        CObjString::ReplaceChar((char*)m_XmlTag.c_str(), ']', '_');

        CXMLDOMElement * pNode = pFatherNode->AddChildNode(m_XmlTag.c_str());
        pNode->AddChildNode("TYPE", m_DataType, AUDIT_DATA_TYPE_GROUP_ENUM);
        //VNGR-11254
        if( m_XmlTag.compare("Target_file_name") == 0 )
        	pNode->AddChildNode("DATA", m_Data.c_str(), TRUE);
        else
        	pNode->AddChildNode("DATA", m_Data.c_str(), FALSE);
    }
}

int CFreeDataNode::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    return STATUS_OK;
}

void CFreeDataNode::SetData(const string & xmlTag, eFreeDataType dataType, const string & data)
{
    m_XmlTag = xmlTag;
    m_DataType = dataType;
    m_Data = data;

    int maxDataLen = 0;

    // one free data node is bound to be up to maxDataLen bytes - a bit more then IP service, the longest event.
    // IP service is enlarged in RMX4000 (contains four addresses of Media boards)
    eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
    if (eProductTypeRMX4000 == curProductType)
    	maxDataLen = 34 * 1024;
    else
    	maxDataLen = 32 * 1024;


    const int dataLen = m_Data.length();
    if(maxDataLen < dataLen)
    {
        m_Data.erase(maxDataLen, dataLen - 1);

        CSmallString message = "Audit free Data is too long ";
        message << dataLen << ", max is " << maxDataLen << "data size is : "<<dataLen;

        PASSERTMSG(TRUE, message.GetString());
    }
}








/*--------------------------------------------------------------------------------------
  CFreeData implementation
--------------------------------------------------------------------------------------*/

CFreeData::CFreeData()
{
}

CFreeData::CFreeData(CFreeDataNode & freeDataNode1)
        : m_FreeDataNode1(freeDataNode1)
{
}

CFreeData::CFreeData(CFreeDataNode & freeDataNode1, CFreeDataNode & freeDataNode2)
        :m_FreeDataNode1(freeDataNode1), m_FreeDataNode2(freeDataNode2)
{
}

CFreeData::~CFreeData()
{
}

void CFreeData::operator = (const CFreeData & other)
{
    m_FreeDataNode1 = other.m_FreeDataNode1;
    m_FreeDataNode2 = other.m_FreeDataNode2;
}

void CFreeData::Serialize(WORD format, CSegment *pSeg)
{
    m_FreeDataNode1.Serialize(0, pSeg);
    m_FreeDataNode2.Serialize(0, pSeg);
}

void CFreeData::DeSerialize(WORD format, CSegment *pSeg)
{
    m_FreeDataNode1.DeSerialize(0, pSeg);
    m_FreeDataNode2.DeSerialize(0, pSeg);
}

void CFreeData::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement *pNode = pFatherNode->AddChildNode("FREE_DATA");
    m_FreeDataNode1.SerializeXml(pNode);
    m_FreeDataNode2.SerializeXml(pNode);
}

int  CFreeData::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    return STATUS_OK;
}















/*--------------------------------------------------------------------------------------
  CAuditHdrWrapper implementation
--------------------------------------------------------------------------------------*/


CAuditHdrWrapper::CAuditHdrWrapper(const AUDIT_EVENT_HEADER_S & hdr, const CFreeData & freeData)
{
    SetHdr(hdr);
    SetFreeData(freeData);
    m_SequenceNum = 0;
}



CAuditHdrWrapper::~CAuditHdrWrapper()
{}

/////////////////////////////////////////////////////////////////////////////
void CAuditHdrWrapper::Serialize(WORD format, CSegment *pSeg)
{
    pSeg->Put((BYTE*)&m_Hdr, sizeof(AUDIT_EVENT_HEADER_S));
//    *pSeg << m_FreeData;
    m_FreeData.Serialize(0, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAuditHdrWrapper::DeSerialize(WORD format, CSegment *pSeg)
{
    pSeg->Get((BYTE*)&m_Hdr, sizeof(AUDIT_EVENT_HEADER_S));
//    *pSeg >> m_FreeData;
    m_FreeData.DeSerialize(0, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAuditHdrWrapper::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement *eventNode = NULL;
    if(NULL == pFatherNode)
    {
        pFatherNode = new CXMLDOMElement("AUDIT_EVENT");
        eventNode = pFatherNode;
    }
    else
    {
        eventNode = pFatherNode->AddChildNode("AUDIT_EVENT");
    }

    eventNode->AddChildNode("SEQ_NUMBER", m_SequenceNum);
    eventNode->AddChildNode("DATE_TIME", m_EventTime);
    eventNode->AddChildNode("USER_ID", m_Hdr.userIdName);

    string strModule = MainEntityToString(m_Hdr.reportModule);
//    strModule += CProcessBase::GetProcessName(CProcessBase::GetProcess()->GetProcessType());

    eventNode->AddChildNode("REPORTER", strModule);

    eventNode->AddChildNode("WORK_STATION", m_Hdr.workStation);
    eventNode->AddChildNode("IP_ADDRESS", m_Hdr.ipAddress);
    eventNode->AddChildNode("TYPE", m_Hdr.eventType, AUDIT_TYPE_GROUP_ENUM);
    eventNode->AddChildNode("ACTION", m_Hdr.action);
    eventNode->AddChildNode("STATUS", m_Hdr.status, AUDIT_STATUS_GROUP_ENUM);
    eventNode->AddChildNode("DESCRIPTION", m_Hdr.description);
    eventNode->AddChildNode("DESCRIPTION_EX", m_Hdr.descriptionEx);

    m_FreeData.SerializeXml(eventNode);
}

/////////////////////////////////////////////////////////////////////////////
int  CAuditHdrWrapper::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    STATUS nStatus = STATUS_OK;

    GET_VALIDATE_MANDATORY_CHILD(pActionNode, "SEQ_NUMBER", &m_SequenceNum, _0_TO_DWORD);
//   GET_VALIDATE_MANDATORY_CHILD(pActionNode, "DATE_TIME", &m_EventTime, DATE_TIME);

    // TBD ASAP, don't have time, I need seq num only.

    return STATUS_OK;
}








/*--------------------------------------------------------------------------------------
  CAuditorApi implementation
--------------------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////
CAuditorApi::CAuditorApi()
        :CManagerApi(eProcessAuditor)
{
}

/////////////////////////////////////////////////////////////////////////////
CAuditorApi::~CAuditorApi()
{
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuditorApi::SendEventMcms(const AUDIT_EVENT_HEADER_S &auditHdr, const CFreeData & freeData)
{
    return SendEvent(AUDIT_EVENT_MCMS, auditHdr, freeData);
}


/////////////////////////////////////////////////////////////////////////////
STATUS CAuditorApi::SendEventOutsider(const AUDIT_EVENT_HEADER_S &auditHdr, const CFreeData & freeData)
{
    return SendEvent(AUDIT_EVENT_OUTSIDER, auditHdr, freeData);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuditorApi::SendEvent(OPCODE opcode, const AUDIT_EVENT_HEADER_S &auditHdr, const CFreeData & freeData)
{
    CAuditHdrWrapper wrapper((AUDIT_EVENT_HEADER_S &)auditHdr, freeData);
    CSegment *pSeg = new CSegment;
    wrapper.Serialize(0, pSeg);

    STATUS status = SendMsg(pSeg, opcode);
    return status;
}

/////////////////////////////////////////////////////////////////////////////
void CAuditorApi::PrepareAuditHeader(AUDIT_EVENT_HEADER_S & outAuditHdr,
                                     const string & userIdName,
                                     APIU32 reportModule,
                                     const string & workStation,
                                     const string & ipAddress,
                                     APIU32 eventType,
                                     APIU32 status,
                                     const string & action,
                                     const string & description,
                                     const string & failure_description,
                                     const string & descriptionEx)
{
    outAuditHdr.reportModule = reportModule;
    outAuditHdr.eventType = eventType;
    outAuditHdr.status = status;

    strncpy(outAuditHdr.userIdName, userIdName.c_str(), MAX_AUDIT_USER_NAME_LEN - 1);
    outAuditHdr.userIdName[MAX_AUDIT_USER_NAME_LEN - 1] = '\0';

    strncpy(outAuditHdr.workStation, workStation.c_str(), MAX_AUDIT_WORKSTATION_NAME_LEN - 1);
    outAuditHdr.workStation[MAX_AUDIT_WORKSTATION_NAME_LEN - 1] = '\0';

    strncpy(outAuditHdr.ipAddress, ipAddress.c_str(), MAX_AUDIT_IP_ADDRESS_LEN - 1);
    outAuditHdr.ipAddress[MAX_AUDIT_IP_ADDRESS_LEN - 1] = '\0';

    strncpy(outAuditHdr.action, action.c_str(), MAX_AUDIT_DESCRIPTION_EX_LEN - 1);
    outAuditHdr.action[MAX_AUDIT_DESCRIPTION_EX_LEN - 1] = '\0';

    if(eAuditEventStatusOk == status)
    {
		strncpy(outAuditHdr.description, description.c_str(), MAX_AUDIT_DESCRIPTION_LEN - 1);
		outAuditHdr.description[MAX_AUDIT_DESCRIPTION_LEN - 1] = '\0';
    }
    else
    {
    	strncpy(outAuditHdr.description, failure_description.c_str(), MAX_AUDIT_DESCRIPTION_LEN - 1);
    	outAuditHdr.description[MAX_AUDIT_DESCRIPTION_LEN - 1] = '\0';
    }
    strncpy(outAuditHdr.descriptionEx, descriptionEx.c_str(), MAX_AUDIT_DESCRIPTION_EX_LEN - 1);
    outAuditHdr.descriptionEx[MAX_AUDIT_DESCRIPTION_EX_LEN - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
void CAuditorApi::PrepareFreeData(CFreeData & outFreeData,
                                  const string & xmlTag1,
                                  eFreeDataType data1Type,
                                  const string & data1,
                                  const string & xmlTag2,
                                  eFreeDataType data2Type,
                                  const string & data2)
{
    CFreeDataNode & node1 = outFreeData.GetdataNode1();
    node1.SetData(xmlTag1, data1Type, data1);

    CFreeDataNode & node2 = outFreeData.GetdataNode2();
    node2.SetData(xmlTag2, data2Type, data2);
}
