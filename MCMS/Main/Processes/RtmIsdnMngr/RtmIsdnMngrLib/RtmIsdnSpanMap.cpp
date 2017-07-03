

#include "RtmIsdnSpanMap.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "TraceStream.h"


/////////////////////////////////////////////////////////////////////////////
// CRtmIsdnSpanMap

CRtmIsdnSpanMap::CRtmIsdnSpanMap()
{
	//defaults
	memset(m_serviceName, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);
	m_isAttachedToService = false;
	
	m_isSpanValid = true;

	m_boardId		= 0xFFFF;
	m_spanId		= 0xFFFF;

	InitSpanStatuses();
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMap::~CRtmIsdnSpanMap()
{
}

///////////////////////////////////////////////////////////////////////////////
//void CRtmIsdnSpanMap::Serialize(WORD format, COstrStream &m_ostr)
//{
//	// assuming format = OPERATOR_MCMS
//	m_ostr << m_lineNumber   << "\n"; 
//	m_ostr << m_lineOrder   << "\n"; 
//	m_ostr << m_dialInGroupId   << "\n"; 
//	m_ostr << m_NFAS_id   << "\n"; 
//} 
//
///////////////////////////////////////////////////////////////////////////////
//void CRtmIsdnSpanMap::DeSerialize(WORD format, CIstrStream &m_istr)
//{
//	// assuming format = OPERATOR_MCMS
//	m_istr >> m_lineNumber;
//	m_istr >> m_lineOrder;
//	m_istr >> m_dialInGroupId;
//	m_istr >> m_NFAS_id;
//} 
/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnSpanMap::SerializeXml m_isSpanValid = "<< (int)m_isSpanValid
			<<" m_spanId "<< m_spanId;

	if ( m_isSpanValid == false && m_spanId>9 && m_spanId<=12)
		{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnSpanMap::SerializeXml we EXIT ";
		  return;
		}




	CXMLDOMElement* pIsdnSpanNode = pFatherNode->AddChildNode("RTM_ISDN_SPAN");
	pIsdnSpanNode->AddChildNode("SLOT_NUMBER",			m_boardId);
	pIsdnSpanNode->AddChildNode("SPAN_ID",				m_spanId);
	pIsdnSpanNode->AddChildNode("SERVICE_NAME",			m_serviceName);
	pIsdnSpanNode->AddChildNode("IS_SPAN_ATTACHED_TO_SERVICE",m_isAttachedToService,_BOOL);
	pIsdnSpanNode->AddChildNode("SPAN_ALARM",			m_spanAlarm,	 SPAN_ALARM_ENUM);
	pIsdnSpanNode->AddChildNode("SPAN_D_CHANNEL_STATE",	m_dChannelState, SPAN_D_CHANNEL_STATE_ENUM);
	pIsdnSpanNode->AddChildNode("SPAN_CLOCKING",		m_clockingState, SPAN_CLOCKING_ENUM);
}
/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMap::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement *pSpanMapNode;
	char* ParentNodeName;
	pActionNode->get_nodeName(&ParentNodeName); 
	if(!strcmp(ParentNodeName, "RTM_ISDN_SPAN")) 
 		pSpanMapNode=pActionNode;   
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "RTM_ISDN_SPAN", pSpanMapNode);

	GET_VALIDATE_CHILD(pSpanMapNode,	"SLOT_NUMBER",			&m_boardId,		_0_TO_WORD);
	GET_VALIDATE_CHILD(pSpanMapNode,	"SPAN_ID",				&m_spanId,		_0_TO_WORD);
	GET_VALIDATE_CHILD(pSpanMapNode,	"SERVICE_NAME",			m_serviceName,	_0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH);
	GET_VALIDATE_CHILD(pSpanMapNode,	"IS_SPAN_ATTACHED_TO_SERVICE", &m_isAttachedToService, _BOOL);

	WORD tmp=0;
	GET_VALIDATE_CHILD(pSpanMapNode,	"SPAN_ALARM",			&tmp,			SPAN_ALARM_ENUM);
	m_spanAlarm = (eSpanAlarmType)tmp;

	GET_VALIDATE_CHILD(pSpanMapNode,	"SPAN_D_CHANNEL_STATE",	&tmp,			SPAN_D_CHANNEL_STATE_ENUM);
	m_dChannelState = (eDChannelStateType)tmp;

	GET_VALIDATE_CHILD(pSpanMapNode,	"SPAN_CLOCKING",		&tmp,			SPAN_CLOCKING_ENUM);
	m_clockingState = (eClockingType)tmp;

	

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMap::InitSpanStatuses()
{
	m_spanAlarm		= eSpanAlarmTypeNone;
    m_dChannelState	= eDChannelStateTypeNotEstablished;
    m_clockingState	= eClockingTypeNone;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CRtmIsdnSpanMap::GetBoardId () const                 
{
    return m_boardId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMap::SetBoardId(const WORD boardId)                 
{

	m_boardId = boardId;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CRtmIsdnSpanMap::GetSpanId () const                 
{
    return m_spanId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMap::SetSpanId(const WORD spanId)                 
{
	m_spanId = spanId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMap::SetIsSpanValid(const bool isSpanValid)
{
	m_isSpanValid = isSpanValid;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRtmIsdnSpanMap::GetServiceName () const
{
    return m_serviceName;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMap::SetServiceName(const char*  serviceName)                 
{
	strncpy(m_serviceName, serviceName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN - 1);

	// Cheaper to simply assign the null than to check: int len = strlen(serviceName);	if (RTM_ISDN_SERVICE_PROVIDER_NAME_LEN <= len)
	m_serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
}


/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnSpanMap::GetIsAttachedToService() const
{
	return m_isAttachedToService;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::SetIsAttachedToService(bool isAttached)
{
	m_isAttachedToService = isAttached;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::DetachFromService()
{
	memset(m_serviceName, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);
	m_isAttachedToService = false;

	InitSpanStatuses();
}

/////////////////////////////////////////////////////////////////////////////
eSpanAlarmType CRtmIsdnSpanMap::GetAlarm()
{
	return m_spanAlarm;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::SetAlarm(eSpanAlarmType newAlarm)
{
	m_spanAlarm = newAlarm;
}

/////////////////////////////////////////////////////////////////////////////
eDChannelStateType CRtmIsdnSpanMap::GetDChannelState()
{
	return m_dChannelState;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::SetDChannelState(eDChannelStateType newState)
{
	m_dChannelState = newState;
}

/////////////////////////////////////////////////////////////////////////////
eClockingType CRtmIsdnSpanMap::GetClocking()
{
	return m_clockingState;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::SetClocking(eClockingType newClocking)
{
	m_clockingState = newClocking;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMap::ConvertToRtmIsdnSpanMapStruct(RTM_ISDN_SPAN_MAP_S &spanMapsStruct)
{
	memset(&spanMapsStruct, 0, sizeof(RTM_ISDN_SPAN_MAP_S));

	memcpy( &(spanMapsStruct.serviceName), &m_serviceName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1 );
	spanMapsStruct.boardId	= m_boardId;
	spanMapsStruct.spanId	= m_spanId;

	return;	
}
