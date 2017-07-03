//
//     Implementation of Faults elements
//+========================================================================+


#include "Macros.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "StructTm.h"
#include "NStream.h"
#include "Segment.h"
#include "FaultsDefines.h"
#include "FaultDesc.h"
#include "HlogElement.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "SystemFunctions.h"
#include "AlarmStrTable.h"
#include <iomanip>



//////////////////////////////////////////////////////////////////////////////
// CHlogElement

CHlogElement::CHlogElement()
{
    memset(&m_hlog, 0, sizeof(hlogType));
    SystemGetTime(m_hlog.time);
    m_referenceCounter	= 0;
}

/////////////////////////////////////////////////////////////////////////////
CHlogElement::CHlogElement(const WORD code)
{
    memset(&m_hlog, 0, sizeof(hlogType));
	m_hlog.code			= code;
    SystemGetTime(m_hlog.time);
    m_referenceCounter	= 0;
}

/////////////////////////////////////////////////////////////////////////////
CHlogElement::~CHlogElement()
{
}

/////////////////////////////////////////////////////////////////////////////
size_t CHlogElement::Sizeof() 
{ 
	return sizeof(hlogType); // m_referenceCounter excluded
}

////////////////////////////////////////////////////////////////////////////
char* CHlogElement::Serialize() const //Asccii serialization
{
	COstrStream*  pOstr;

	pOstr = new COstrStream();

	*pOstr	<< m_hlog.code;
	*pOstr	<< "  "; //blanks
	*pOstr	<< m_hlog.subjectMask;
	*pOstr	<< "  "; //blanks
	*pOstr	<< m_hlog.faultId;
	*pOstr	<< "  "; //blanks

	CStructTm tm(m_hlog.time);
	tm.Serialize(*pOstr);

	int nMsgLen = pOstr->str().length();
	if( nMsgLen >= SIZE_STREAM )
		nMsgLen = SIZE_STREAM-1;

	char* pszMsg = new char[nMsgLen+1];
	memset(pszMsg,' ',nMsgLen);
	memcpy(pszMsg,pOstr->str().c_str(),nMsgLen); 
	pszMsg[nMsgLen]='\0';

	PDELETE(pOstr);

	return pszMsg;
}

////////////////////////////////////////////////////////////////////////////
void CHlogElement::Serialize(COstrStream& oStr) const //Asccii serialization
{
	oStr << m_hlog.code;
	oStr << "  "; //blanks
	oStr << m_hlog.subjectMask;
	oStr << "  "; //blanks
	oStr << m_hlog.faultId;
	oStr << "  "; //blanks

	CStructTm tm(m_hlog.time);
	tm.Serialize(oStr);
}

////////////////////////////////////////////////////////////////////////////
void  CHlogElement::DeSerialize(char* p) // ascii Deserialization
{
	CIstrStream iStr(p);
	DeSerialize(iStr);
}

////////////////////////////////////////////////////////////////////////////
void  CHlogElement::DeSerialize(CIstrStream& iStr) // ascii DeSerialize
{
	iStr >> m_hlog.code;
	iStr.ignore(1,(int)' ');
	iStr >> m_hlog.subjectMask;
	iStr.ignore(1,(int)' ');
	iStr >> m_hlog.faultId;

	CStructTm tm;
	tm.DeSerialize(iStr);

	m_hlog.time = tm;
	iStr.ignore(1);
}

////////////////////////////////////////////////////////////////////////////
BYTE* CHlogElement::Serialize(WORD& n) const//Binary serialization
{
	n = sizeof(hlogType);
	BYTE* p = new BYTE[n];
	memcpy(p, &m_hlog, n);
	return p;
}

////////////////////////////////////////////////////////////////////////////
void  CHlogElement::DeSerialize(BYTE* p) // binary DeSerialize
{
	memcpy(&m_hlog, p, sizeof(hlogType));
}

////////////////////////////////////////////////////////////////////////////
void CHlogElement::Serialize(CSegment& seg) const
{ 
	WORD n;
	BYTE* p = Serialize(n);
	seg.Put(p,n);
	PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CHlogElement::DeSerialize(CSegment& seg)
{
	BYTE* p = new BYTE[Sizeof()];
	seg.Get(p,Sizeof());
	DeSerialize(p);
    PDELETEA(p)
}

/////////////////////////////////////////////////////////////////////////////
void CHlogElement::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement *pHLogNode = NULL;
    if(NULL == pFatherNode)
    {
        pFatherNode = new CXMLDOMElement("HLOG_ELEMENT");
        pHLogNode = pFatherNode;
    }
    else
    {
        pHLogNode = pFatherNode->AddChildNode("HLOG_ELEMENT");
    }
	
	SerializeXmlCommon(pHLogNode);
}

/////////////////////////////////////////////////////////////////////////////
void CHlogElement::SerializeXmlCommon(CXMLDOMElement*& pHLogNode) const
{
//	CXMLDOMElement* pCommonParamsNode  = pHLogNode->AddChildNode("HLOG_PARAMS");
	pHLogNode->AddChildNode("TIME_STAMP",m_hlog.time);

//	if( pCommonParamsNode ) {
//		pCommonParamsNode->AddChildNode("HLOG_OPCODE",m_hlog.code);
//		pCommonParamsNode->AddChildNode("SUBJECT_MASK",m_hlog.subjectMask);
//		pCommonParamsNode->AddChildNode("TIME_STAMP",m_hlog.time);
//	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CHlogElement::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;
   
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CHlogElement::DeSerialize_Xml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	
	if( nStatus )
	{
		//file
	/*	SetFileNum(fileNum);
		DWORD currentFaultId =GetFaultId();
					
			if (((int)currentFaultId % 10/) != fileNum)
			{
				int r=0;//for debug
				
			}
		*/
		return nStatus;
	}
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int CHlogElement::DeSerializeXmlCommon(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"TIME_STAMP",&(m_hlog.time),DATE_TIME);
//	CXMLDOMElement *pChildNode = NULL;
//	pActionNode->getChildNodeByName(&pChildNode,"HLOG_PARAMS");

//	if( pChildNode ) {
//		GET_VALIDATE_CHILD(pChildNode,"HLOG_OPCODE",&(m_hlog.code),"0_TO_DWORD");
//		if( nStatus )
//			return nStatus;

//		GET_VALIDATE_CHILD(pChildNode,"SUBJECT_MASK",&(m_hlog.subjectMask),"0_TO_DWORD");
//		if( nStatus )
//			return nStatus;

//		GET_VALIDATE_CHILD(pChildNode,"TIME_STAMP",&(m_hlog.time),"");
//		if( nStatus )
//			return nStatus;
//	} else {
//		return STATUS_NODE_MISSING;
//	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void  CHlogElement::SetTime(const CStructTm& tm) 
{
	m_hlog.time = tm;
}

////////////////////////////////////////////////////////////////////////////
CHlogElement* CHlogElement::Copy() const
{
	return new CHlogElement(*this);
}

////////////////////////////////////////////////////////////////////////////
CHlogElement::CHlogElement(const CHlogElement& rhs): CSerializeObject(rhs)
{
    m_hlog.subjectMask = rhs.m_hlog.subjectMask;
    m_hlog.code = rhs.m_hlog.code;
    m_hlog.type = rhs.m_hlog.type;
    m_hlog.faultId = rhs.m_hlog.faultId;
    m_hlog.time = rhs.m_hlog.time;   // don't memcpy classes with virtual tables
    
	m_referenceCounter = 0;
}

////////////////////////////////////////////////////////////////////////////
CHlogElement& CHlogElement::operator=(const CHlogElement& other)
{
    memcpy((void*)&m_hlog, (void*)&(other.m_hlog), sizeof(hlogType));
	return *this;
}
////////////////////////////////////////////////////////////////////////////
//bool operator<( const CHlogElement &other ) const; 
bool CHlogElement::operator < (const CHlogElement &other)const
{
		//BOOL rval = FALSE;
	bool rval =0;
	if ( m_hlog.faultId < other.m_hlog.faultId )
		//rval = TRUE;
		rval = 1;
	   
	return rval;
}
////////////////////////////////////////////////////////////////////////////
/*bool CHlogElement::operator==(const CHlogElement& other)const
{
	bool bRes = true;//CSerializeObject::operator==(other);
	if(false == bRes)
	{
		return false;
	}

	int iRes = memcmp((void*)&m_hlog, (void*)&(other.m_hlog), sizeof(hlogType));
	bRes = (iRes == 0);
	return bRes;
}*/
///////////////////////////////////////////////////////////////////////////
bool CHlogElement::operator==(const CHlogElement &other)const
//bool operator==(const CHlogElement& other)const
{	//BOOL rval = FALSE;
	bool rval =0;
	if ( m_hlog.faultId == other.m_hlog.faultId )
		//rval = TRUE;
	   rval = 1;
	return rval;
	
}

///////////////////////////////////////////////////////////////////////////////////////
/*bool operator==(const CHlogElement & lhs,const CHlogElement & rhs)const
{
	return (lhs.m_hlog.faultId == rhs.m_hlog.faultId);
}
*/
////////////////////////////////////////////////////////////////////////////
BYTE CHlogElement::IsFault() const
{
	if (GetCode() >= MIN_FAULTS_CODE && GetCode() <= MAX_FAULTS_CODE)
		return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
/*bool CHlogElement::operator < (const CHlogElement & l, const CHlogElement & r)
{
    cout << "hello";
    
    return (l.m_hlog.faultId < r.m_hlog.faultId);
}*/
	//////////////////////////////////////////////////////////////////////////////
/*bool CHlogElement::operator < (const CHlogElement & l)const
{
    cout << "hello";
    
    return (l.m_hlog.faultId );
}*/
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------
/*WORD operator==(const CMediaIpParameters& lhs,const CMediaIpParameters& rhs)
{
	return (lhs.m_mediaIpParamsStruct.serviceId == rhs.m_mediaIpParamsStruct.serviceId);
}
*/
// ------------------------------------------------------------
/*bool CHlogElement::operator<(const CHlogElement& lhs,const CHlogElement& rhs)const
{
	return lhs.m_hlog.faultId < rhs.m_hlog.faultId;
}
*/

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CHlogElement* NewCHlogElement(const WORD code)
{
	CHlogElement* newElm = NULL;
	
	if (code+1 >= MIN_COMMON_CODE+1 && code <= MAX_COMMON_CODE)
	{
		newElm = new CHlogElement;
	}
	else if (code >= MIN_FAULTS_CODE && code <= MAX_FAULTS_CODE)
	{
		newElm = new CLogFltElement;
	}
	else if (code >= MIN_NET_CODE && code <= MAX_NET_CODE)
	{
		newElm = new CLogNetElement;
	}
	else if (code >= MIN_ACCOUNT_CODE && code <= MAX_ACCOUNT_CODE)
	{
		newElm = new CLogAccntElement;
	}

	return newElm;
}

////////////////////////////////////////////////////////////////////////////
void CHlogElement::SetFaultId(DWORD fid)
{
	m_hlog.faultId = fid;
}

////////////////////////////////////////////////////////////////////////////
DWORD CHlogElement::GetFaultId() const
{
	return m_hlog.faultId;
}

////////////////////////////////////////////////////////////////////////////
void CHlogElement::IncreaseReferenceCounter()
{
	m_referenceCounter++;
}

////////////////////////////////////////////////////////////////////////////
void CHlogElement::DecreaseReferenceCounter()
{
	m_referenceCounter--;
	
	if ( 0 > m_referenceCounter)
		m_referenceCounter = 0;
}

////////////////////////////////////////////////////////////////////////////
int CHlogElement::GetReferenceCounter()
{
	return m_referenceCounter;
}

/////////////////////////////////////////////////////////////////////////////
CLogFltElement::CLogFltElement()
{
	memset(&m_hlogData, 0, sizeof(hlogFltType));
}

/////////////////////////////////////////////////////////////////////////////
CLogFltElement::CLogFltElement(const CLogFltElement& rhs) : CHlogElement(rhs)
{
	memcpy(&m_hlogData, &(rhs.m_hlogData), sizeof(hlogFltType));
}

////////////////////////////////////////////////////////////////////////////
CLogFltElement& CLogFltElement::operator=(const CLogFltElement& other)
{
	CHlogElement::operator=(other);
	memcpy(&m_hlogData, &(other.m_hlogData), sizeof(hlogFltType));

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CLogFltElement::operator==(const CLogFltElement& other)
{
	bool bRes = CHlogElement::operator==(other);
	if(false == bRes)
	{
		return false;
	}
	
	int iRes = memcmp((void*)&m_hlogData, (void*)&(other.m_hlogData), sizeof(hlogFltType));
	bRes = (iRes == 0);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
bool CLogFltElement::operator!=(const CLogFltElement& other)
{
	bool bRes = CLogFltElement::operator==(other);
	return !bRes;
}

/////////////////////////////////////////////////////////////////////////////
CLogFltElement::CLogFltElement(const WORD code, const char* description):CHlogElement(code)
{
  memset(&m_hlogData, 0, sizeof(hlogFltType));

  m_hlog.code = code;
  strncpy(m_hlogData.description, description, sizeof(m_hlogData.description)-1);
  m_hlogData.description[sizeof(m_hlogData.description)-1] = '\0';
}

////////////////////////////////////////////////////////////////////////////
CLogFltElement::~CLogFltElement() // Override
{
}

////////////////////////////////////////////////////////////////////////////
void CLogFltElement::Dump(std::ostream &ostr) const
{
	char pTempStr[FAULT_DESCRIPTION_LENGTH];
	memcpy(pTempStr,m_hlogData.description,FAULT_DESCRIPTION_LENGTH);
	pTempStr[FAULT_DESCRIPTION_LENGTH-1] = 0;

	CFaultDesc rFaultDescTemp;
	rFaultDescTemp.DeSerializeString(pTempStr);
	
	const eProcessType processType 	= rFaultDescTemp.GetProcessName();
	const char *processName 		= CProcessBase::GetProcessName(processType);
	const WORD errorCode 			= GetCode();
	const char *desription 			= GetDescription();
	
	ostr << processName << ":"
         << GetAlarmName(errorCode) << " : "
         << GetAlarmDescription(errorCode) << " : "
         << desription;	
}

void CLogFltElement::DumpAsString(std::ostream &ostr) const
{
	char pTempStr[FAULT_DESCRIPTION_LENGTH];
	memcpy(pTempStr,m_hlogData.description,FAULT_DESCRIPTION_LENGTH);
	pTempStr[FAULT_DESCRIPTION_LENGTH-1] = 0;

	CFaultDesc rFaultDescTemp;
	rFaultDescTemp.DeSerializeString(pTempStr);

	const eProcessType processType 	= rFaultDescTemp.GetProcessName();
	const char *processName 		= CProcessBase::GetProcessName(processType);
	const WORD errorCode 			= GetCode();

	std::string	faultDescription;
	std::string	faultCategory;
	std::string	faultLevel;

	DWORD faultErrorcode = 0;

    std::ostringstream timeString;

	CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(m_hlogData.description);
	if(NULL != pFaultDesc)
	{
		const char *pDesk = pFaultDesc->GetDescription();
		if(NULL != pDesk)
		{
			faultDescription = (string)pDesk ;
		}
		faultCategory = pFaultDesc->GetSubjectAsString();

		faultLevel = pFaultDesc->GetLevelAsString();

		faultErrorcode = pFaultDesc->GetErrorCode();

		PDELETE(pFaultDesc);
	}

	// in this writing - there is segmantation in soft mcu
	//timeString << m_hlog.time;

	CStructTm tempTime =m_hlog.time;

	timeString << tempTime;

	ostr  << std::left << std::setw(6) << std::setfill(' ') << GetFaultId()
		  << std::left << std::setw(22) << std::setfill(' ') << timeString.str()
		  << std::left << std::setw(16) << std::setfill(' ') << faultCategory
		  << std::left << std::setw(16) << std::setfill(' ') << faultLevel
		  << std::left << std::setw(16) << std::setfill(' ')  << faultErrorcode
		  << std::left << std::setw(22) << std::setfill(' ') << processName
		  << std::left << std::setw(32) << std::setfill(' ')  << faultDescription
		  << "\r\n";

}


////////////////////////////////////////////////////////////////////////////
void  CLogFltElement::DeSerialize(BYTE* p)//Override
{
	CHlogElement::DeSerialize(p);
	memcpy(&m_hlogData,p + CHlogElement::Sizeof() , sizeof(hlogFltType));
}

////////////////////////////////////////////////////////////////////////////
BYTE* CLogFltElement::Serialize(WORD& n)const//Override
{
	//format parameter does not matter.
	WORD   m = 0;
	BYTE*  q = CHlogElement::Serialize(m);
	n = m + sizeof (hlogFltType);
	
	BYTE*  p = new BYTE[n];
	memcpy(p , q , m );
	memcpy(p + m , &m_hlogData , sizeof(hlogFltType) );

	PDELETEA(q);
	return p;
}

////////////////////////////////////////////////////////////////////////////
void CLogFltElement::Serialize(COstrStream& oStr)const//Override
{
	CHlogElement::Serialize(oStr);
	oStr << m_hlogData.description;
	oStr << '\n';
}

////////////////////////////////////////////////////////////////////////////
char* CLogFltElement::Serialize()const//Override
{
	COstrStream*  pOstr = new COstrStream();

	CHlogElement::Serialize(*pOstr);

	*pOstr << m_hlogData.description;
	*pOstr << '\n';

	int   nMsgLen = pOstr->str().length();
	if( nMsgLen >= SIZE_STREAM )
		nMsgLen = SIZE_STREAM-1;
	
	char* pszMsg = new char[nMsgLen+1];
	memset(pszMsg,' ', nMsgLen);
	memcpy(pszMsg,pOstr->str().c_str(),nMsgLen);
	pszMsg[nMsgLen]='\0';

	PDELETE(pOstr);

	return pszMsg;
}

////////////////////////////////////////////////////////////////////////////
void CLogFltElement::DeSerialize(CIstrStream& iStr)//Override
{
	CHlogElement::DeSerialize(iStr);
	iStr.getline(m_hlogData.description,FAULT_DESCRIPTION_LENGTH+1,'\n');
}

////////////////////////////////////////////////////////////////////////////
void  CLogFltElement::DeSerialize(char* p)//Override
{
	//// object master -- CHlogElement::DeSerialize (format, p);
	CIstrStream  iStr(p);
	DeSerialize(iStr);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
void CLogFltElement::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement *pFaultNode = NULL;
    if(NULL == pFatherNode)
    {
        pFatherNode = new CXMLDOMElement("LOG_FAULT_ELEMENT");
        pFaultNode = pFatherNode;
    }
    else
    {
        pFaultNode = pFatherNode->AddChildNode("LOG_FAULT_ELEMENT");
    }
    
    SerializeXmlCommon(pFaultNode);

	CFaultDesc* pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(m_hlogData.description);
    if(NULL != pFaultDesc)
    {
        pFaultDesc->SetFaultId(m_hlog.faultId); //m_hlogData.index);   
        pFaultDesc->SerializeXml(pFaultNode);
        POBJDELETE(pFaultDesc);
    }
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CLogFltElement::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
//	if (NO == m_isItCardStatus)
//	{
		nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
		if( nStatus )
			return nStatus;
//	}

//    CHlogElement::DeSerializeXml(pActionNode, pszError, action);
    
	CFaultDesc* pFaultDesc = NULL;
	m_hlogData.description[0] = '\0';

	CXMLDOMElement* pChildNode = NULL;

	if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_STARTUP") ) {
		pFaultDesc = new CFaultDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_ASSERT") ) {
		pFaultDesc = new CFaultAssertDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_CARD") ) {
		pFaultDesc = new CFaultCardDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_EXCEPTION") ) {
		pFaultDesc = new CFaultExceptHandlDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_FILE") ) {
		pFaultDesc = new CFaultFileDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_GENERAL") ) {
		pFaultDesc = new CFaultGeneralDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_RESERVATION") ) {
		pFaultDesc = new CFaultReservationDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_CONFERENCE") ) {
		pFaultDesc = new CFaultReservationDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_UNIT") ) {
		pFaultDesc = new CFaultUnitDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_MPL") ) {
		pFaultDesc = new CFaultMplDesc;
	} else {
		return STATUS_OBJECT_NOT_RECOGNIZED;
	}

	nStatus = pFaultDesc->DeSerializeXml(pChildNode,pszError,action);
	if( nStatus ) {
		POBJDELETE(pFaultDesc);
		return nStatus;
	}

	char *pStr = pFaultDesc->SerializeString(0);
	strncpy(m_hlogData.description,pStr,sizeof(m_hlogData.description) - 1);
	m_hlogData.description[sizeof(m_hlogData.description) - 1] = '\0';
	m_hlog.faultId = pFaultDesc->GetFaultId();

	PDELETEA(pStr);
	POBJDELETE(pFaultDesc);
	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*int CLogFltElement::DeSerializeXml_(CXMLDOMElement *pActionNode,char *pszError,const char* action,const char * fileName)
{
	int nStatus = STATUS_OK;
//	if (NO == m_isItCardStatus)
//	{
		nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
		if( nStatus )
			return nStatus;
//	}

//    CHlogElement::DeSerializeXml(pActionNode, pszError, action);
    
	CFaultDesc* pFaultDesc = NULL;
	m_hlogData.description[0] = '\0';

	CXMLDOMElement* pChildNode = NULL;

	if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_STARTUP") ) {
		pFaultDesc = new CFaultDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_ASSERT") ) {
		pFaultDesc = new CFaultAssertDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_CARD") ) {
		pFaultDesc = new CFaultCardDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_EXCEPTION") ) {
		pFaultDesc = new CFaultExceptHandlDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_FILE") ) {
		pFaultDesc = new CFaultFileDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_GENERAL") ) {
		pFaultDesc = new CFaultGeneralDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_RESERVATION") ) {
		pFaultDesc = new CFaultReservationDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_CONFERENCE") ) {
		pFaultDesc = new CFaultReservationDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_UNIT") ) {
		pFaultDesc = new CFaultUnitDesc;
	} else if( SEC_OK == pActionNode->getChildNodeByName(&pChildNode,"FAULT_DESC_MPL") ) {
		pFaultDesc = new CFaultMplDesc;
	} else {
		return STATUS_OBJECT_NOT_RECOGNIZED;
	}

	nStatus = pFaultDesc->DeSerializeXml(pChildNode,pszError,action);
	if( nStatus ) {
		POBJDELETE(pFaultDesc);
		return nStatus;
	}

	char *pStr = pFaultDesc->SerializeString(0);
	strncpy(m_hlogData.description,pStr,FAULT_DESCRIPTION_LENGTH);
	m_hlogData.description[FAULT_DESCRIPTION_LENGTH-1] = '\0';
	m_hlog.faultId = pFaultDesc->GetFaultId();

	PDELETEA(pStr);
	POBJDELETE(pFaultDesc);
	return STATUS_OK;
}
*/
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
size_t CLogFltElement::Sizeof()
{
	return CHlogElement::Sizeof() + sizeof(hlogFltType);
}

/////////////////////////////////////////////////////////////////////////////
void CLogFltElement::SetDescription(const char* desc) 
{
	strncpy(m_hlogData.description,desc,sizeof(m_hlogData.description) - 1);
	m_hlogData.description[sizeof(m_hlogData.description) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
void CLogFltElement::GetCleanDescription(string & outDescription)const
{
    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(m_hlogData.description);
    if(NULL != pFaultDesc)
    {    
        const char *pDesk = pFaultDesc->GetDescription();
        if(NULL != pDesk)
        {
            outDescription = pDesk;
        }
        PDELETE(pFaultDesc);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CLogFltElement::UpdateDescription(const char* desc)
{
    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(m_hlogData.description);
    
    if(pFaultDesc == NULL)
    {
    	FPTRACE(eLevelInfoNormal,"CLogFltElement::UpdateDescription pFaultDesc is NULL!!");
    	return;
    }
    pFaultDesc->SetDescription(desc);
    char *pszMsg = pFaultDesc->SerializeString(0);
    SetDescription(pszMsg);
    PDELETEA(pszMsg);
    PDELETE(pFaultDesc);
}

/////////////////////////////////////////////////////////////////////////////
CHlogElement* CLogFltElement::Copy() const//Override
{
	return new CLogFltElement(*this);
}

/////////////////////////////////////////////////////////////////////////////
bool CLogFltElement::IsSimilar(const CLogFltElement & other)const
{
    if(this == &other)
    {
        return true;
    }

    if(m_hlog.subjectMask != other.m_hlog.subjectMask)
    {
        return false;
    }
    
    if(m_hlog.code != other.m_hlog.code)
    {
        return false;
    }

    int res = strncmp(m_hlogData.description, other.m_hlogData.description, FAULT_DESCRIPTION_LENGTH);
    if(0 != res)
    {
        return false;
    }
    
    return true;
}



/////////////////////////////////////////////////////////////////////////////
CLogNetElement::CLogNetElement()
{
}

/////////////////////////////////////////////////////////////////////////////
size_t CLogNetElement::Sizeof()//Override
{
	return CHlogElement::Sizeof () + 	sizeof(hlogNetType);
}

/////////////////////////////////////////////////////////////////////////////
void CLogNetElement::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pNetNode = pFatherNode->AddChildNode("LOG_NET_ELEMENT");
	SerializeXmlCommon(pNetNode);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CLogNetElement::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
CLogAccntElement::CLogAccntElement()
{
}

/////////////////////////////////////////////////////////////////////////////
size_t CLogAccntElement::Sizeof()//Override
{
	return CHlogElement::Sizeof() + sizeof(hlogAccntType);
}

/////////////////////////////////////////////////////////////////////////////
void CLogAccntElement::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pLogNode = pFatherNode->AddChildNode("LOG_ACCNT_ELEMENT");
	SerializeXmlCommon(pLogNode);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CLogAccntElement::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}

