/////////////////////////////////////////////////////////////////////////////
// CRtmIsdnServiceList


#include "RtmIsdnServiceList.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "InternalProcessStatuses.h"
#include "RtmIsdnService.h"
#include "ApiStatuses.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "HlogApi.h"


extern const char* SpanTypeToString(eSpanType theType);


/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceList::CRtmIsdnServiceList()
{
	for (int i=0;i<MAX_ISDN_SERVICES_IN_LIST;i++)
		m_pServiceProvider[i] = NULL; 

	m_numb_of_serv_prov     = 0;
	m_ind_serv              = 0;           
	m_defaultServiceName[0] = '\0';	
	m_updateCounter         = 0;
	m_bChanged=FALSE;

// for RMX
	m_IsServiceAdded 	= false;
}


/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceList::CRtmIsdnServiceList(const CRtmIsdnServiceList &other)
:CSerializeObject(other)
{
	
	for (int i=0;i<MAX_ISDN_SERVICES_IN_LIST;i++)
	{
		if( other.m_pServiceProvider[i]==NULL)
			m_pServiceProvider[i]=NULL;
		else
			m_pServiceProvider[i]= new CRtmIsdnService(*other.m_pServiceProvider[i]);  
	}
	m_numb_of_serv_prov=other.m_numb_of_serv_prov;
	m_ind_serv=other.m_ind_serv; 

	strncpy( m_defaultServiceName,
	         other.m_defaultServiceName,
	         RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );
	m_defaultServiceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
	
	m_updateCounter =  other.m_updateCounter;
	m_bChanged=other.m_bChanged;

	m_IsServiceAdded 	= other.m_IsServiceAdded;
}

///////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceList& CRtmIsdnServiceList::operator=(const CRtmIsdnServiceList& other)
{
	for (int i=0;i<MAX_ISDN_SERVICES_IN_LIST;i++)
	{
		PDELETE(m_pServiceProvider[i]);  
		if( other.m_pServiceProvider[i]==NULL)
			m_pServiceProvider[i]=NULL;
		else
			m_pServiceProvider[i]= new CRtmIsdnService(*other.m_pServiceProvider[i]);  
	}
	m_numb_of_serv_prov=other.m_numb_of_serv_prov;
	m_ind_serv=other.m_ind_serv; 

	strncpy( m_defaultServiceName,
	         other.m_defaultServiceName,
	         RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );
	m_defaultServiceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
	
	m_updateCounter =  other.m_updateCounter;
	m_bChanged=other.m_bChanged;

	m_IsServiceAdded 	= other.m_IsServiceAdded;
	
	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceList::~CRtmIsdnServiceList()
{
	for (int i=0;i<MAX_ISDN_SERVICES_IN_LIST;i++) 
		PDELETE(m_pServiceProvider[i]);
}


/////////////////////////////////////////////////////////////////////////////
/*
void CRtmIsdnServiceList::Serialize(WORD format, COstrStream &m_ostr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	
	int i;
	WORD num_srv_prov = (m_numb_of_serv_prov > 30 && apiNum < 46)?
		30 : m_numb_of_serv_prov;
	
	
	m_ostr <<  num_srv_prov  << "\n";  //NUMBER_OF_RESERVATIONS 
	m_ostr <<  m_defaultServiceName       << "\n";  
	if (m_numb_of_serv_prov!=0)
		m_ostr << "~" << "\n";
	
	for (i=0;i<(int)m_numb_of_serv_prov;i++)
	{  
		if(apiNum < 46 && i >= MAX_SERV_PROVIDERS_IN_LIST)
			break;	
		
		m_pServiceProvider[i]->Serialize(format, m_ostr, apiNum);
		m_ostr << "~" << "\n";
	}
} 

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceList::DeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	
	int i;
	char s;  
	
	m_istr >> m_numb_of_serv_prov;
	
	m_numb_of_serv_prov = (m_numb_of_serv_prov > 30 && apiNum < 46)?
		30 : m_numb_of_serv_prov; // for 2.12
	
	m_istr.ignore(1);

	m_istr.getline(m_defaultServiceName,RTM_ISDN_SERVICE_PROVIDER_NAME_LEN+1,'\n');
	if (m_numb_of_serv_prov!=0)
	{
		m_istr >> s;
		if (s!='~') return;
	}

	for (i=0;i<(int)m_numb_of_serv_prov;i++)
	{  
		if(apiNum < 46 && i >= MAX_SERV_PROVIDERS_IN_LIST)
			break;	
		
		m_pServiceProvider[i]= new CRtmIsdnService;
		m_pServiceProvider[i]->DeSerialize(format, m_istr, apiNum);
		m_istr >> s;
		if (s!='~') return;
	}
} 

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceList::ShortSerialize(WORD format, COstrStream &m_ostr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	
	int i;
	WORD num_srv_prov = (m_numb_of_serv_prov > 30 && apiNum < 46)?
		30 : m_numb_of_serv_prov;
	
	
	m_ostr <<  num_srv_prov  << "\n";  //NUMBER_OF_RESERVATIONS 
	m_ostr <<  m_defaultServiceName       << "\n";  

	if (m_numb_of_serv_prov!=0)
		m_ostr << "~" << "\n";
	
	for (i=0;i<(int)m_numb_of_serv_prov;i++)
	{  
		if(apiNum < 46 && i >= MAX_SERV_PROVIDERS_IN_LIST)
			break;	
		
		m_pServiceProvider[i]->ShortSerialize(format, m_ostr, apiNum);
		m_ostr << "~" << "\n";
	}
} 

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceList::ShortDeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	
	int i;
	char s;  
	
	m_istr >> m_numb_of_serv_prov;
	
	m_numb_of_serv_prov = (m_numb_of_serv_prov > 30 && apiNum < 46)?
		30 : m_numb_of_serv_prov; // for 2.12
	
	m_istr.ignore(1);
	m_istr.getline(m_defaultServiceName,RTM_ISDN_SERVICE_PROVIDER_NAME_LEN+1,'\n');

	if (m_numb_of_serv_prov!=0)
	{
		m_istr >> s;
		if (s!='~') return;
	}

	for (i=0;i<(int)m_numb_of_serv_prov;i++)
	{  
		if(apiNum < 46 && i >= MAX_SERV_PROVIDERS_IN_LIST)
			break;	
		
		m_pServiceProvider[i]= new CRtmIsdnService;
		m_pServiceProvider[i]->ShortDeSerialize(format, m_istr, apiNum);
		m_istr >> s;
		if (s!='~') return;
	}
	
}
*/

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pIsdnServiceListNode;
	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("ISDN_SERVICE_LIST");
		pIsdnServiceListNode = pFatherNode;
	}
	else
	{
		pIsdnServiceListNode = pFatherNode->AddChildNode("ISDN_SERVICE_LIST");
	}
	

	DWORD bChanged = InsertUpdateCntChanged(pIsdnServiceListNode, UPDATE_CNT_BEGIN_END);
	if(FALSE == bChanged)
	{
		return;
	}
	

	pIsdnServiceListNode->AddChildNode("DEFAULT_NAME", m_defaultServiceName);
	
	for (int i=0; i<m_numb_of_serv_prov; i++)
	{
		m_pServiceProvider[i]->SerializeXml(pIsdnServiceListNode, UPDATE_CNT_BEGIN_END);
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceList::SerializeXml(CXMLDOMElement* pFatherNode, DWORD objToken) const
{
	CXMLDOMElement* pIsdnServiceListNode = pFatherNode->AddChildNode("ISDN_SERVICE_LIST");
	
	WORD bChanged = InsertUpdateCntChanged(pIsdnServiceListNode, objToken);
	if(TRUE == bChanged)
	{
		pIsdnServiceListNode->AddChildNode("DEFAULT_NAME", m_defaultServiceName);
		
		for (int i=0; i<m_numb_of_serv_prov; i++)
		{
			m_pServiceProvider[i]->SerializeXml(pIsdnServiceListNode, UPDATE_CNT_BEGIN_END);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_isdn_srv_list.xsd
int CRtmIsdnServiceList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pIsdnSrvNode;

	m_bChanged=TRUE;

	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);	
	GET_VALIDATE_CHILD(pActionNode,"CHANGED",&m_bChanged,_BOOL);	

	m_numb_of_serv_prov = 0;

	GET_VALIDATE_CHILD(pActionNode,"DEFAULT_NAME",m_defaultServiceName,_0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH);

	GET_FIRST_CHILD_NODE(pActionNode,"ISDN_SERVICE",pIsdnSrvNode);

	while (pIsdnSrvNode && m_numb_of_serv_prov < MAX_ISDN_SERVICES_IN_LIST)
	{
		CRtmIsdnService* pIsdnSrv = new CRtmIsdnService;
		nStatus = pIsdnSrv->DeSerializeXml(pIsdnSrvNode, pszError, "");

		if(nStatus != STATUS_OK)
		{
			POBJDELETE(pIsdnSrv);
			return nStatus;
		}
		POBJDELETE(m_pServiceProvider[m_numb_of_serv_prov]);
		m_pServiceProvider[m_numb_of_serv_prov] = pIsdnSrv;
		m_numb_of_serv_prov++;
		GET_NEXT_CHILD_NODE(pActionNode,"ISDN_SERVICE",pIsdnSrvNode);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CRtmIsdnServiceList::GetChanged() const
{
  return m_bChanged;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::Add(CRtmIsdnService &other, const char *file_name)                 
{
	if (m_numb_of_serv_prov>=MAX_ISDN_SERVICES_IN_LIST)
		return  STATUS_NUMBER_OF_ISDN_SERVICES_EXCEEDED;
	
	if ( NOT_FIND != FindService(other) ) 
		return STATUS_SERVICE_PROVIDER_NAME_EXISTS;
	// vngr-24939
	if(strlen(other.GetName()) == 0)
	     return STATUS_ILLEGAL_SERVICE_NAME;
	
	CRtmIsdnPhoneNumberRange* pPhoneRane = other.GetFirstPhoneRange();
	for ( int i=0;
	      ( pPhoneRane && (i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE) );
	      i++) 
	{
		const char* firstNum = pPhoneRane->GetFirstPhoneNumber();
		const char* lastNum  = pPhoneRane->GetLastPhoneNumber();
		CRtmIsdnService* pService = GetService(firstNum, lastNum); 

		if (NULL != pService) 
			return STATUS_PHONE_NUMBER_ALREADY_EXISTS;
				STATUS addPhoneStat = CheckAddPhoneRange(other.GetName(), *pPhoneRane);
		if (STATUS_OK != addPhoneStat)
			return addPhoneStat;
		
/*
		WORD  mcuPhones=FALSE;
		if (other.m_pPhoneNum[i]->GetCategory()==2) 
		{
			if (mcuPhones==FALSE)
				mcuPhones=TRUE;
			else
				return STATUS_MUST_BE_ONLY_ONE_MCU_PHONE_NUMBER;
		}
*/
		pPhoneRane = other.GetNextPhoneRange();
	}
	
	
	if ('\0' == m_defaultServiceName[0]) // defaultService does not exist
	{
		strncpy( m_defaultServiceName,
		         other.GetName(),
		         RTM_ISDN_SERVICE_PROVIDER_NAME_LEN - 1);
		m_defaultServiceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
	}
	
	m_pServiceProvider[m_numb_of_serv_prov] = new CRtmIsdnService(other);
	m_numb_of_serv_prov++;
	m_IsServiceAdded = true;

	IncreaseUpdateCounter();
	WriteXmlFile(file_name);	

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::Update(const CRtmIsdnService&  other)                 
{
	int ind = FindService(other);
	if (NOT_FIND == ind)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;  

	PASSERTSTREAM_AND_RETURN_VALUE(ind >= MAX_ISDN_SERVICES_IN_LIST,
		"ind has invalid value " << ind, STATUS_FAIL);

	POBJDELETE(m_pServiceProvider[ind]);
	m_pServiceProvider[ind] = new CRtmIsdnService(other);
	
	IncreaseUpdateCounter();
	WriteXmlFile(RTM_ISDN_SERVICE_LIST_TMP_PATH);

	return STATUS_OK;  
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::GetCancelServiceStat(const char* serviceName)
{
	STATUS retStat = STATUS_OK;
	
	int ind = FindService(serviceName);
	if (NOT_FIND == ind)
	{
		retStat = STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}
	
	return retStat;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::Cancel( const char* serviceName, bool isToCheckStat/*=true*/,
		                            const char *file_name/*=""*/, bool isBothFiles/*=false*/)
{

    if (m_numb_of_serv_prov > MAX_ISDN_SERVICES_IN_LIST)
        return  STATUS_NUMBER_OF_ISDN_SERVICES_EXCEEDED;
	
    STATUS retStat = STATUS_OK;
    if (true == isToCheckStat)
    	retStat = GetCancelServiceStat(serviceName);
	
    if (STATUS_OK == retStat)
    {
		int ind = FindService(serviceName); 
		if (NOT_FIND != ind) // must exist, as it was already checked at 'GetCancelServiceStat'
		{
			PASSERTSTREAM_AND_RETURN_VALUE(ind >= MAX_ISDN_SERVICES_IN_LIST,
				"ind has invalid value " << ind, STATUS_FAIL);
			
			// delete defaultService if needed
			if ( !strncmp(m_defaultServiceName, serviceName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN) )
				m_defaultServiceName[0] = '\0';
			
			POBJDELETE(m_pServiceProvider[ind]);
			
			int i=0, j=0;
			for (i=0; i<(int)m_numb_of_serv_prov; i++)
			{
				if (m_pServiceProvider[i]==NULL)
					break; 
			}    
			for (j=i; j<(int)m_numb_of_serv_prov-1; j++)
			{
				m_pServiceProvider[j] = m_pServiceProvider[j+1] ;
			}
		
			m_pServiceProvider[m_numb_of_serv_prov-1] = NULL;
			m_numb_of_serv_prov--;
			
			// set other service as defaultService if needed
			if ( ('\0' == m_defaultServiceName[0]) &&  (0 < m_numb_of_serv_prov) )
			{
				strncpy( m_defaultServiceName,
		                 m_pServiceProvider[0]->GetName(),
		                 RTM_ISDN_SERVICE_PROVIDER_NAME_LEN - 1);
		        m_defaultServiceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
			}
			
			// update file(s)
			if (true == isBothFiles)
			{
				WriteXmlFile(RTM_ISDN_SERVICE_LIST_PATH);
				WriteXmlFile(RTM_ISDN_SERVICE_LIST_TMP_PATH);
			}
			else
			{
				WriteXmlFile(file_name);
			}

			IncreaseUpdateCounter();
		} // end if (NOT_FIND != ind)

	} // if (STATUS_OK == retStat)
		
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/*
int CRtmIsdnServiceList::AddOnlyMem(const CRtmIsdnService&  other)                 
{
	if (m_numb_of_serv_prov>=MAX_ISDN_SERVICES_IN_LIST)
		return  STATUS_NUMBER_OF_ISDN_SERVICES_EXCEEDED;
	
	if (FindService(other)!=NOT_FIND) 
		return STATUS_SERVICE_PROVIDER_NAME_EXISTS;  
    
	m_pServiceProvider[m_numb_of_serv_prov] = new CRtmIsdnService(other);
	
	m_numb_of_serv_prov++;
	
	return STATUS_OK;  
}
*/


/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::CheckAddPhoneRange( const char* serviceName,
                                                const CRtmIsdnPhoneNumberRange &phoneRange )
{
	string otherFirst = phoneRange.m_firstPhoneNumber,
	       otherLast  = phoneRange.m_lastPhoneNumber;
	if ( (otherFirst.length() != otherLast.length()) || (otherFirst > otherLast) )
		return STATUS_ILLEGAL_PHONE_RANGE;
	
	if ( (otherFirst != otherLast) && (true == phoneRange.IsMaxNumbersInRangeExceeded()) )
		return STATUS_NUMBER_OF_PHONE_NUMBERS_IN_RANGE_EXCEEDED;

	if ( true == IsPhoneNumberExistsInRanges(phoneRange) )
		return STATUS_PHONE_NUMBER_ALREADY_EXISTS;
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::AddPhoneNumRange( const char* serviceName,
                                              const CRtmIsdnPhoneNumberRange &phoneRange,
                                              const char *file_name)
{
	// ===== 1. general checkings
	STATUS checkStat = CheckAddPhoneRange(serviceName, phoneRange);
	if (STATUS_OK != checkStat)
		return checkStat;
	
	int ind = FindService(serviceName); 
	if (NOT_FIND == ind)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;


	// ===== 2. add the range
	CRtmIsdnService *pService = GetService(ind);
	if (NULL == pService)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;

	STATUS addStat = pService->AddPhoneNumRange(phoneRange);
	if (STATUS_OK == addStat)
	{
		IncreaseUpdateCounter();
		WriteXmlFile(file_name);
	}
	
	return addStat;
}

/////////////////////////////////////////////////////////////////////////////
/*
// update phone - not implemented
STATUS CRtmIsdnServiceList::UpdatePhoneNumRange( const char* serviceName,
                                                 const CRtmIsdnPhoneNumberRange &phoneRange,
                                                 const char *file_name)
{
	int ind = FindService(serviceName); 
	if (NOT_FIND == ind)
	{
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}

	if ( false == IsPhoneNumberExistsInRanges(phoneRange) )
	{
		return STATUS_PHONE_RANGE_NOT_EXISTS;
	}
	
	CRtmIsdnService *pService = GetService(ind);

	STATUS updateStat = pService->UpdatePhoneNumRange(phoneRange);
	if (STATUS_OK == updateStat)
	{
		IncreaseUpdateCounter();
		WriteXmlFile(file_name);
	}
	
	return updateStat;
}
*/




/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::GetUpdatePhoneNumRangeStat( const char* serviceName,
                                                        const CRtmIsdnPhoneNumberRange &phoneRange )
{
	STATUS retStat = STATUS_OK;

	// ===== 1. check if service exists
	int ind = FindService(serviceName); 
	if (NOT_FIND == ind)
	{
		retStat = STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}

	else // service exists
	{
		// ===== 2. check if range exists
		if ( false == IsPhoneNumberExistsInRangesExactly(phoneRange) )
		{
			retStat = STATUS_PHONE_RANGE_NOT_EXISTS;
		}
	}

	return retStat;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnServiceList::DelPhoneNumRange( const char* serviceName,
                                              const CRtmIsdnPhoneNumberRange &phoneRange,
                                              const char *file_name,
                                              const bool isToCheckStat/*=true*/ )
{
	STATUS retStat = STATUS_OK;
	if (true == isToCheckStat)
		retStat = GetUpdatePhoneNumRangeStat(serviceName, phoneRange);

	if (STATUS_OK == retStat)
	{
		int ind = FindService(serviceName); 
		if (NOT_FIND != ind) // must exist, as it was already checked at 'GetUpdatePhoneNumRangeStat'
		{
			CRtmIsdnService *pService = GetService(ind);
			
			if(pService)
				retStat = pService->CancelPhoneNumRange(phoneRange);
			else
				retStat = STATUS_FAIL;
			if (STATUS_OK == retStat)
			{
				IncreaseUpdateCounter();
				WriteXmlFile(file_name);
			}
		}
    }
	
	return retStat;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnServiceList::FindService(const CRtmIsdnService&  other)                 
{
	for (int i=0;i<(int)m_numb_of_serv_prov;i++)
	{
		if (m_pServiceProvider[i]!=NULL) {
			if (! strncmp(m_pServiceProvider[i]->GetName(),other.GetName(),RTM_ISDN_SERVICE_PROVIDER_NAME_LEN))
				return i; 
		}  
	}
	return NOT_FIND; 
}


/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnServiceList::FindService(const char*  name)
{
	for (int i=0;i<(int)m_numb_of_serv_prov;i++)
	{
		if (m_pServiceProvider[i]!=NULL) {
			if (! strncmp(m_pServiceProvider[i]->GetName(),name,RTM_ISDN_SERVICE_PROVIDER_NAME_LEN))
				return i; 
		}  
	}
	return NOT_FIND; 
}


/////////////////////////////////////////////////////////////////////////////
BOOL  CRtmIsdnServiceList::IsEmpty()
{
	if (0 == m_numb_of_serv_prov)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CRtmIsdnServiceList::GetServProvNumber () const                 
{
	return m_numb_of_serv_prov;
}

////////////////////////////////////////////////////
void  CRtmIsdnServiceList::SetServProvNumber(const WORD num)                 
{
	m_numb_of_serv_prov=num;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnService*  CRtmIsdnServiceList::GetCurrentServiceProvider(const char*  name)                 
{                                                                        
	for (int i=0;i<(int)m_numb_of_serv_prov;i++)
	{
		if (m_pServiceProvider[i]!=NULL) {
			if (! strncmp(m_pServiceProvider[i]->GetName(), name, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN))
				return m_pServiceProvider[i]; 
		}        
	}
	return NULL;          // STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnService*  CRtmIsdnServiceList::GetService(const WORD line)                 
{                                                                        
	for (int i=0;i<(int)m_numb_of_serv_prov;i++)
	{
/* TO BE ADDED 
		if (m_pServiceProvider[i]!=NULL) {
			if (m_pServiceProvider[i]->FindSpan(line)!=NOT_FIND)
				return m_pServiceProvider[i]; 
		}        
*/
	}
	return NULL;      
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnService* CRtmIsdnServiceList::GetService(const char* firstNum, const char *latsNum)                 
{                                                                        
	for (int i=0;i<(int)m_numb_of_serv_prov;i++)
	{
		if (m_pServiceProvider[i]!=NULL)
		{
			if ( true == m_pServiceProvider[i]->IsPhonesAlreadyExistsInServiceRanges(firstNum, latsNum) )
				return m_pServiceProvider[i]; 
		}        
	}
	return NULL;      
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnService*  CRtmIsdnServiceList::GetService(const int index) 
{           
	if(index >= 0 && index < MAX_ISDN_SERVICES_IN_LIST)
	{
		return m_pServiceProvider[index];
	}
	else
	{
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnService*  CRtmIsdnServiceList::GetFirstService()  
{           
	m_ind_serv=1;
	return m_pServiceProvider[0]; 
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnService*  CRtmIsdnServiceList::GetNextService() 
{
	CRtmIsdnService *pService = NULL;

    if (m_ind_serv < m_numb_of_serv_prov)
    {
    	pService = m_pServiceProvider[m_ind_serv];
    	m_ind_serv++;
    }

	return pService;                        
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnServiceList::SetDefaultName(const char* name, const char *file_name)                 
{
	strncpy(m_defaultServiceName, name, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN - 1);

	// Cheaper to assign the null than to check: int len = strlen(name);if (RTM_ISDN_SERVICE_PROVIDER_NAME_LEN <= len)
	m_defaultServiceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1] = '\0';

	IncreaseUpdateCounter();
	WriteXmlFile(file_name);
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRtmIsdnServiceList::GetDefaultName () const                 
{
    return m_defaultServiceName;
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnServiceList::IsDefaultExists() const
{
	bool isDefExists = true;
	
	if ( '\0' == m_defaultServiceName[0] )
		isDefExists = false;
	
	return isDefExists;
}                 

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnServiceList::IsDefault(const char* name) const
{
	bool isDef = false;
	
	if ( !strncmp(name, m_defaultServiceName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN) )
		isDef = true;
	
	return isDef;
}                 

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnServiceList::IsPhoneNumberExistsInRanges(const CRtmIsdnPhoneNumberRange &phone)
{
	bool isExist = false;
	
	CRtmIsdnService* pServProv = GetFirstService();
	while (pServProv)
	{
		if (true == pServProv->IsPhonesAlreadyExistsInServiceRanges(phone)) 
		{
			isExist = true;
			break;
		}  
		
		pServProv = GetNextService();
	}
	
	return isExist;
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnServiceList::IsPhoneNumberExistsInRangesExactly(const CRtmIsdnPhoneNumberRange &phone)
{
	bool isExist = false;
	
	CRtmIsdnService* pServProv = GetFirstService();
	while (pServProv)
	{
		if (NOT_FIND != pServProv->FindPhoneRange(phone)) 
		{
			isExist = true;
			break;
		}  
		
		pServProv = GetNextService();
	}
	
	return isExist;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRtmIsdnServiceList::NameOf() const                
{
	return "CRtmIsdnServiceList";
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CRtmIsdnServiceList::GetUpdateCounter() const                
{
	return m_updateCounter;
}

//////////////////////////////////////////////////////////////////////
bool CRtmIsdnServiceList::GetIsServiceAdded()
{
	return m_IsServiceAdded;
}

/////////////////////////////////////////////////////////////////////////////
eSpanType CRtmIsdnServiceList::DeleteServiceWithInconsistentSpanType()
{
	bool isDifferentSpanTypeExists = false;

	CRtmIsdnSpanDefinition* pTmpSpanDef;
	eSpanType retType = eSpanTypeT1,
	          curType = eSpanTypeT1;
	
	
	// ===== 1. get SpanType (for the whole system) - the SpanType of the 1st service
	if (NULL != m_pServiceProvider[0])
	{
		pTmpSpanDef = m_pServiceProvider[0]->GetSpanDef();
		retType = pTmpSpanDef->GetSpanType();
	}

	
	// ===== 2. delete service(s) that has a different SpanType
	for (int i=0; i<(int)m_numb_of_serv_prov; i++)
	{
		if (NULL == m_pServiceProvider[i])
			break;

		pTmpSpanDef = m_pServiceProvider[i]->GetSpanDef();
		curType = pTmpSpanDef->GetSpanType();
		if (curType != retType)
		{
			isDifferentSpanTypeExists = true;
			Cancel( m_pServiceProvider[i]->GetName(), true, "", true );
		}
	}    

	// produce log & fault
	if (true == isDifferentSpanTypeExists)
	{
		CMedString errStr = "Isdn Service with inconsistent Span Type was found and deleted";
		CHlogApi::IsdnServiceInconsistentSpanType( errStr.GetString() );
		
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::DeleteServiceWithInconsistentSpanType"
		                       << "\n" << errStr.GetString()
		                       << "\n(the valid SpanType is " << ::SpanTypeToString(retType) << ")";
	}

	return retType;	
}

/////////////////////////////////////////////////////////////////////////////

//#ifdef __HIGHC__
//STATUS CRtmIsdnServiceList::AddGwRangeToService(int idx, const CGW_PhoneRange& newRange)
//{
//	STATUS status = STATUS_INTERNAL_ERROR;
//	
//	if (m_pServiceProvider[idx] != NULL)
//	{
//		CRtmIsdnService* pServProv = new CRtmIsdnService(*m_pServiceProvider[idx]);
//		status = pServProv->AddGwRange(newRange);
//		if (status == STATUS_OK)
//			status = ::GetpServProvList()->Update(*pServProv);
//		POBJDELETE(pServProv);
//	}
//	
//	return status;
//}
///////////////////////////////////////////////////////////////////////////////
//STATUS CRtmIsdnServiceList::CancelGwRangeFromService(int idx, const CGW_PhoneRange& other)
//{
//	STATUS status = STATUS_INTERNAL_ERROR;
//	
//	if (m_pServiceProvider[idx] != NULL)
//	{
//		CRtmIsdnService* pServProv = new CRtmIsdnService(*m_pServiceProvider[idx]);
//		status = pServProv->CancelGwRange(other);
//		if (status == STATUS_OK)
//			status = ::GetpServProvList()->Update(*pServProv);
//		POBJDELETE(pServProv);
//	}
//	
//	
//	return status;
//}
///////////////////////////////////////////////////////////////////////////////
//STATUS CRtmIsdnServiceList::UpdateGwRangeInService(int idx, const CGW_PhoneRange& other)
//{
//	STATUS status = STATUS_INTERNAL_ERROR;
//	
//	if (m_pServiceProvider[idx] != NULL)
//	{
//		CRtmIsdnService* pServProv = new CRtmIsdnService(*m_pServiceProvider[idx]);
//		status = pServProv->UpdateGwRange(other);
//		if (status == STATUS_OK)
//			status = ::GetpServProvList()->Update(*pServProv);
//		POBJDELETE(pServProv);
//	}
//	
//	return status;
//}
///////////////////////////////////////////////////////////////////////////////
//void CRtmIsdnServiceList::SetGwPhoneRangesListFromGwRanges()
//{
//	for (int i=0; i<MAX_PHONE_RANGES_IN_GW_COFIGURATION; i++)
//	{
//		const CGW_BasicPhoneRange* pBasicRange = ::GetpGW_BasicPhoneRangesList()->m_pBasicPhoneRangesList[i];
//		if (pBasicRange == NULL)
//			return;
//		
//		CGW_PhoneRange* pRange = new CGW_PhoneRange();
//		pRange->SetFirstNumber(pBasicRange->GetFirstNumber());
//        pRange->SetLastNumber(pBasicRange->GetLastNumber());
//		pRange->SetNetServiceName(pBasicRange->GetNetServiceName());
//		
//		int idx = FindService(pBasicRange->m_netServiceName);
//		if (m_pServiceProvider[idx] != NULL)
//			m_pServiceProvider[idx]->AddGwRange(*pRange);
//		PDELETE (pRange);
//	}
//	
//	return;
//}
//#endif

/////////////////////////////////////////////////////////////////////////////
//STATUS CRtmIsdnServiceList::IsPhoneNumberExistsAtAll(CGW_PhoneRange pPhone)
//{
//	STATUS status = STATUS_PHONE_NUMBER_NOT_EXISTS;
//	
//	CRtmIsdnService* pServProv = GetFirstService();
//	while (pServProv)
//	{
//		
//		status = pServProv->IsPhoneNumberExistsAtNETserv(pPhone);
//		if (status != STATUS_PHONE_NUMBER_NOT_EXISTS)
//			return status;
//		
//		status = pServProv->IsPhoneNumberExistsAtGWserv(pPhone);
//		if (status != STATUS_PHONE_NUMBER_NOT_EXISTS)
//			return status;
//		
//		pServProv = GetNextService();
//	}
//	
//	
//	
//	return status;
//}
