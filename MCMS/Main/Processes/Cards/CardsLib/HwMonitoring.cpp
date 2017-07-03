// HwMonitoring.cpp: implementation of the HwMonitoring classes
//
//////////////////////////////////////////////////////////////////////


#include "HwMonitoring.h"
#include "FaultDesc.h"
#include "FaultsDefines.h"
#include "CardsDefines.h"
#include "CardsProcess.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "CardsStatuses.h"
#include "FaultsContainer.h"
#include "TraceStream.h"

extern char* CardTypeToString(APIU32 cardType);

// CCommRes

CCommCard::CCommCard()
{
	m_boardId=0;
	m_subBoardId=0;
	m_displayBoardId=0;
	m_cards_type=eEmpty;
	m_hardVersion.ver_major=99;
	m_hardVersion.ver_minor=99;
	m_hardVersion.ver_release=99;
	m_simul=SIMULATION_NO;
	m_serialNumber[0] = '\0';
	
	memset( m_swVersionsList, 0, sizeof(CM_SW_VERSION_S)*MAX_NUM_OF_SW_VERSIONS );
}



/////////////////////////////////////////////////////////////////////////////
CCommCard::CCommCard( const eCardType  type,
					  const char*      serialNum,
					  const WORD       boardId,
					  const WORD       subBoardId,
					  const WORD       displayBoardId,
					  const VERSION_S  hardVersion,
					  const CM_SW_VERSION_S*  softVersionList,
					  const WORD isSimulation)
{
	m_cards_type=type;
	m_boardId=boardId;
	m_subBoardId=subBoardId;
	m_displayBoardId=displayBoardId;
	m_hardVersion.ver_major=hardVersion.ver_major;
	m_hardVersion.ver_minor=hardVersion.ver_minor;
	m_hardVersion.ver_release=hardVersion.ver_release;
	m_simul=isSimulation;
	
	strncpy(m_serialNumber, serialNum, sizeof(m_serialNumber) - 1 );
	m_serialNumber[sizeof(m_serialNumber) - 1] = '\0';
	memcpy( m_swVersionsList, softVersionList, sizeof(CM_SW_VERSION_S)*MAX_NUM_OF_SW_VERSIONS );
}

CCommCard& CCommCard::operator =(const CCommCard &other)
{
	m_cards_type=other.m_cards_type;
	m_boardId=other.m_boardId;
	m_subBoardId=other.m_subBoardId;
	m_displayBoardId=other.m_displayBoardId;
	m_hardVersion.ver_major=other.m_hardVersion.ver_major;
	m_hardVersion.ver_minor=other.m_hardVersion.ver_minor;
	m_hardVersion.ver_release=other.m_hardVersion.ver_release;
	m_simul=other.m_simul;
	
	strncpy(m_serialNumber, other.m_serialNumber, MPL_SERIAL_NUM_LEN );
	memcpy( m_swVersionsList, other.m_swVersionsList, sizeof(CM_SW_VERSION_S)*MAX_NUM_OF_SW_VERSIONS );
		
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommCard::CCommCard(const CCommCard &other)
:CPObject()
{
	m_boardId=0;
	m_subBoardId=0;
	m_displayBoardId=0;
	m_cards_type=eEmpty;
	m_hardVersion.ver_major=99;
	m_hardVersion.ver_minor=99;
	m_hardVersion.ver_release=99;
	m_simul=SIMULATION_NO;
	m_serialNumber[0] = '\0';
	
	memcpy( m_swVersionsList, other.m_swVersionsList, sizeof(CM_SW_VERSION_S)*MAX_NUM_OF_SW_VERSIONS );

	*this=other;
}

/////////////////////////////////////////////////////////////////////////////
CCommCard::~CCommCard()
{
}

/////////////////////////////////////////////////////////////////////////////
void CCommCard::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pCommonNode = pFatherNode->AddChildNode("CARD_COMMON_DATA");
	
//	WORD card_type = (m_cards_type == CARD_ARROW_MUX_1)? CARD_MUX_PLUS_1 : m_cards_type;
	WORD card_type = m_cards_type;
	
	pCommonNode->AddChildNode("CARD_TYPE",card_type,CARD_TYPE_ENUM);

	if ((card_type == eSwitch) && (m_boardId == FIXED_BOARD_ID_SWITCH))
		pCommonNode->AddChildNode("SLOT_NUMBER",FIXED_DISPLAY_BOARD_ID_SWITCH,BOARD_ENUM);
	else
	   pCommonNode->AddChildNode("SLOT_NUMBER",m_boardId,BOARD_ENUM);

	CXMLDOMElement* pVersionNode = NULL;
/*
	pVersionNode = pCommonNode->AddChildNode("SOFTWARE_VERSION");
	if( pVersionNode )
	{
		pVersionNode->AddChildNode("MAIN",m_softVersion.ver_major);
		pVersionNode->AddChildNode("MAJOR",m_softVersion.ver_major);
		pVersionNode->AddChildNode("MINOR",m_softVersion.ver_minor); // temp: until supported by MPL
	}
*/

	pVersionNode = pCommonNode->AddChildNode("HARDWARE_VERSION");
	if( pVersionNode )
	{
		pVersionNode->AddChildNode("MAIN",m_hardVersion.ver_major);
		pVersionNode->AddChildNode("MAJOR",m_hardVersion.ver_major);
		pVersionNode->AddChildNode("MINOR",m_hardVersion.ver_minor); // temp: until supported by MPL
	}
	
//	pCommonNode->AddChildNode("SERIAL_NUMBER",m_serialNumber);
	pCommonNode->AddChildNode("SIMULATION_MODE",m_simul,_BOOL);


	CXMLDOMElement* pSwVersionsListNode = pCommonNode->AddChildNode("CARD_SW_VERSIONS_LIST");
	if (pSwVersionsListNode)
	{
		int i=0;
		for( i=0; i<MAX_NUM_OF_SW_VERSIONS; i++ )
		{
			CXMLDOMElement* pSwVersionNode = pSwVersionsListNode->AddChildNode("CARD_SW_VERSION");
			if (pSwVersionNode)
			{
				pSwVersionNode->AddChildNode("CARD_SW_VERSION_DESCRIPTOR", (char*)(m_swVersionsList[i].versionDescriptor) );
				pSwVersionNode->AddChildNode("CARD_SW_VERSION_NUMBER",(char*)(m_swVersionsList[i].versionNumber) );
			}
		}	// end loop over MAX_NUM_OF_SW_VERSIONS
	}	// end if (pSwVersionsListNode)

	pCommonNode->AddChildNode("MPL_SERIAL_NUMBER",m_serialNumber);
	pCommonNode->AddChildNode("SUB_BOARD_ID",m_subBoardId,SUB_BOARD_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cards_list.xsd
int CCommCard::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode,"CARD_TYPE",(WORD*)m_cards_type,CARD_TYPE_ENUM);
	if( nStatus )
		return nStatus;

	if  (m_cards_type == eSwitch)
	{
		GET_VALIDATE_CHILD(pActionNode,"SLOT_NUMBER",&m_boardId,BOARD_ENUM);
	    if (m_boardId == FIXED_DISPLAY_BOARD_ID_SWITCH)
	    	m_boardId = FIXED_BOARD_ID_SWITCH;
	}
	else
	   GET_VALIDATE_CHILD(pActionNode,"SLOT_NUMBER",&m_boardId,BOARD_ENUM);

	if( nStatus )
		 return nStatus;
	
	CXMLDOMElement *pChild = NULL;
	
/*
	GET_CHILD_NODE(pActionNode, "SOFTWARE_VERSION", pChild);
	if( pChild )
	{
		GET_VALIDATE_CHILD(pChild,"MAIN",&m_softVersion.ver_major,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChild,"MAJOR",&m_softVersion.ver_major,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChild,"MINOR",&m_softVersion.ver_minor,_0_TO_DWORD);
	}
*/
	
	GET_CHILD_NODE(pActionNode, "HARDWARE_VERSION", pChild);
	if( pChild )
	{
		GET_VALIDATE_CHILD(pChild,"MAIN",&m_hardVersion.ver_major,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChild,"MAJOR",&m_hardVersion.ver_major,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChild,"MINOR",&m_hardVersion.ver_minor,_0_TO_DWORD);
	}
	
//	GET_VALIDATE_CHILD(pActionNode,"SERIAL_NUMBER",&m_serialNumber,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"SIMULATION_MODE",&m_simul,_BOOL);


	GET_CHILD_NODE(pActionNode, "CARD_SW_VERSIONS_LIST", pChild);
	if ( pChild )
	{
		int idx=0;
		CXMLDOMElement*  pSwVersionsListChild = NULL;
		GET_FIRST_CHILD_NODE(pChild, "CARD_SW_VERSION", pSwVersionsListChild);
		while( (pSwVersionsListChild) && (idx < MAX_NUM_OF_SW_VERSIONS) )
		{
			GET_VALIDATE_CHILD(pSwVersionsListChild, "CARD_SW_VERSION_DESCRIPTOR", m_swVersionsList[idx].versionDescriptor, ONE_LINE_BUFFER_LENGTH);
			GET_VALIDATE_CHILD(pSwVersionsListChild, "CARD_SW_VERSION_DESCRIPTOR", m_swVersionsList[idx].versionNumber, ONE_LINE_BUFFER_LENGTH);
			
			GET_NEXT_CHILD_NODE(pActionNode, "CARD_SW_VERSION", pSwVersionsListChild);
			idx++;
		}
	}	// end if (pChild)
	
	GET_VALIDATE_CHILD(pActionNode,"MPL_SERIAL_NUMBER", m_serialNumber, _0_TO_MPL_SERIAL_NUM_LENGTH);

	GET_VALIDATE_CHILD(pActionNode,"SUB_BOARD_ID",&m_subBoardId,SUB_BOARD_ENUM);
	if( nStatus )
		return nStatus;


	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CCommCard::TestValid() const
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
eCardType  CCommCard::GetType () const
{
	return m_cards_type;
}

/////////////////////////////////////////////////////////////////////////////

void  CCommCard::SetOnlyType(const eCardType type)
{
	m_cards_type = type;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommCard::SetType(const eCardType  type)
{
	m_cards_type = type;
}


/////////////////////////////////////////////////////////////////////////////
void CCommCard::SetBoardId(const WORD slotNumber)
{
	m_boardId = slotNumber;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCommCard::GetBoardId () const
{
	return m_boardId;
}

/////////////////////////////////////////////////////////////////////////////
void CCommCard::SetSubBoardId(const WORD slotNumber)
{
	m_subBoardId = slotNumber;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCommCard::GetDisplayBoardId () const
{
	return m_displayBoardId;
}

/////////////////////////////////////////////////////////////////////////////
void CCommCard::SetDisplayBoardId(const WORD slotNumber)
{
	m_displayBoardId = slotNumber;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCommCard::GetSubBoardId () const
{
	return m_subBoardId;
}

/////////////////////////////////////////////////////////////////////////////
/*
const VERSION_S  CCommCard::GetSoftVersion () const
{
	return m_softVersion;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommCard::SetSoftVersion(const VERSION_S softVersion)
{
	m_softVersion.ver_major=softVersion.ver_major;
	m_softVersion.ver_minor=softVersion.ver_minor;
	m_softVersion.ver_release=softVersion.ver_release;
}
*/

/////////////////////////////////////////////////////////////////////////////
const VERSION_S  CCommCard::GetHardVersion () const
{
	return m_hardVersion;
}


/////////////////////////////////////////////////////////////////////////////
void  CCommCard::SetHardVersion(const VERSION_S hardVersion)
{
	m_hardVersion.ver_major=hardVersion.ver_major;
	m_hardVersion.ver_minor=hardVersion.ver_minor;
	m_hardVersion.ver_release=hardVersion.ver_release;
}


/////////////////////////////////////////////////////////////////////////////
char*  CCommCard::GetSerialNumber ()
{
	return m_serialNumber;
}


/////////////////////////////////////////////////////////////////////////////
void  CCommCard::SetSerialNumber(const char* serialNumber)
{
	strncpy( m_serialNumber,
	         serialNumber,
	         sizeof(m_serialNumber) - 1);
	m_serialNumber[sizeof(m_serialNumber) - 1] = '\0';
}

/*
/////////////////////////////////////////////////////////////////////////////
void CCommCard::SetH323Spec(const CCommH323Card&  other)
{
	m_pH323Card = new CCommH323Card(other);
}

/////////////////////////////////////////////////////////////////////////////
CCommH323Card* CCommCard::GetH323Spec()
{
	return m_pH323Card;
}
*/



/////////////////////////////////////////////////////////////////////////////
// CCommDynCard

CCommDynCard::CCommDynCard():CCommCard()
{
	m_state = eNormal;
	m_numb_of_status=0;
	m_numb_of_conf=0;
	m_numb_of_units=0;
	int i;
	for (i=0;i<MAX_STATUS_IN_LIST;i++)
		m_pCardStatus[i] = NULL;
//	for (i=0;i<MAX_CONF_IN_LIST;i++)
//		m_pCardConf[i] = NULL;
//	for (i=0;i<MAX_UNITS_IN_CARD;i++)
//		m_pCardRsrc[i] = NULL;
	
	m_status_ind=0;
	m_conf_ind=0;
	m_rsrc_ind=0;

	m_updateCounter = 0;
	m_bChanged = FALSE;
	
	m_StatusList = new CFaultList;
//	m_StatusList->SetIsItCardStatusesList(YES);
}


/////////////////////////////////////////////////////////////////////////////
CCommDynCard::CCommDynCard( const eCardType  type,
						    const char*      serialNum,
						    const WORD       boardId,
							const WORD       subBoardId,
							const WORD       displayBoardId,
						    const VERSION_S  hardVersion,
							const CM_SW_VERSION_S* softVersionList,
							const WORD isSimulation):
CCommCard( type, serialNum, boardId, subBoardId, displayBoardId, hardVersion, softVersionList, isSimulation)
{
	m_state = eNormal;
	m_numb_of_status=0;
	m_numb_of_conf=0;
	m_numb_of_units=0;
	int i;
	for (i=0;i<MAX_STATUS_IN_LIST;i++)
		m_pCardStatus[i] = NULL;
//	for (i=0;i<MAX_CONF_IN_LIST;i++)
//		m_pCardConf[i] = NULL;
//	for (i=0;i<MAX_UNITS_IN_CARD;i++)
//		m_pCardRsrc[i] = NULL;
	m_status_ind=0;
	m_conf_ind=0;
	m_rsrc_ind=0;

	m_updateCounter = 0;
	m_bChanged = FALSE;
	
	m_StatusList = new CFaultList;
//	m_StatusList->SetIsItCardStatusesList(YES);
}

CCommDynCard& CCommDynCard:: operator =(const CCommDynCard &other)
{
	CCommCard::operator = (other);
	m_state=other.m_state;
	m_numb_of_status=other.m_numb_of_status;
	m_numb_of_conf=other.m_numb_of_conf;
	m_numb_of_units=other.m_numb_of_units;
	int i;
	for (i=0;i<MAX_STATUS_IN_LIST;i++)
	{
		if (m_pCardStatus[i]!=NULL)
			PDELETE(m_pCardStatus[i]);
		if( other.m_pCardStatus[i]!=NULL)
			m_pCardStatus[i]= new CFaultCardDesc(*other.m_pCardStatus[i]);
	}
//	for (i=0;i<MAX_CONF_IN_LIST;i++)
//	{
//		if (m_pCardConf[i]!=NULL)
//			PDELETE(m_pCardConf[i]);
//		if( other.m_pCardConf[i]!=NULL)
//			m_pCardConf[i]= new CCardConf(*other.m_pCardConf[i]);
//	}
//	for (i=0;i<MAX_UNITS_IN_CARD;i++)
//	{
//		if (m_pCardRsrc[i]!=NULL)
//			PDELETE(m_pCardRsrc[i]);	
//		if( other.m_pCardRsrc[i]!=NULL)
//			m_pCardRsrc[i]= new CCardRsrc(*other.m_pCardRsrc[i]);
//	}

	m_status_ind=other.m_status_ind;
	m_conf_ind=other.m_conf_ind;
	m_rsrc_ind=other.m_rsrc_ind;
	
	m_updateCounter = other.m_updateCounter;
	m_bChanged = other.m_bChanged;
	
	//clear m_StatusList
	m_StatusList->Clear();
	
	// copy statuses list
	CFaultElementList::iterator iTer = other.m_StatusList->begin();
	CFaultElementList::iterator iEnd = other.m_StatusList->end();
	while(iEnd != iTer)
	{
		CLogFltElement *elem = *iTer;
		CLogFltElement *newElem = new CLogFltElement(*elem);
		m_StatusList->AddFault(newElem);
		iTer++;
	}
//	m_StatusList->SetIsItCardStatusesList(YES);
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommDynCard::CCommDynCard(const CCommDynCard &other):CCommCard(*(const CCommCard*)&other)
{
	m_state = eNormal;
	m_numb_of_status=0;
	m_numb_of_conf=0;
	m_numb_of_units=0;
	int i;
	for (i=0;i<MAX_STATUS_IN_LIST;i++)
		m_pCardStatus[i] = NULL;
//	for (i=0;i<MAX_CONF_IN_LIST;i++)
//		m_pCardConf[i] = NULL;
//	for (i=0;i<MAX_UNITS_IN_CARD;i++)
//		m_pCardRsrc[i] = NULL;
	
	m_status_ind=0;
	m_conf_ind=0;
	m_rsrc_ind=0;
	
	m_updateCounter = 0;
	m_bChanged = FALSE;

	m_StatusList = new CFaultList;
//	m_StatusList->SetIsItCardStatusesList(YES);

	*this=other;
}

/////////////////////////////////////////////////////////////////////////////
CCommDynCard::~CCommDynCard()
{
	int i;
	for (i=0;i<MAX_STATUS_IN_LIST;i++)
		PDELETE(m_pCardStatus[i]);
//	for (i=0;i<MAX_CONF_IN_LIST;i++)
//		PDELETE(m_pCardConf[i]);
//	for (i=0;i<MAX_UNITS_IN_CARD;i++)
//		PDELETE(m_pCardRsrc[i]);
	POBJDELETE(m_StatusList);
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SerializeXml(CXMLDOMElement* pFatherNode)
{
	//	if( m_cards_type == eEmpty )
	//		return;
	
	CXMLDOMElement* pDescNode = pFatherNode->AddChildNode("CARD_SUMMARY_DESCRIPTOR");
	CCommCard::SerializeXml(pDescNode);
	
	pDescNode->AddChildNode("CARD_STATE",m_state,CARD_STATE_ENUM);
	pDescNode->AddChildNode("NUMBER_CARD_STATUSES",m_numb_of_status); // MAX_STATUS_IN_LIST
	pDescNode->AddChildNode("NUMBER_CARD_CONFERENCES",m_numb_of_conf); // MAX_CONF_IN_LIST

//NEW_STATUS_LIST
//	CXMLDOMElement* pStatusesNode = pDescNode->AddChildNode("CARD_STATUSES_LIST");
//	SerializeXmlStatuses(pStatusesNode);
	SerializeXmlStatuses(pDescNode);

// serialize sequence of Card Resource Descriptors
//	for( int i=0; i<(int)m_numb_of_units; i++ )
//		m_pCardRsrc[i]->SerializeXml(pDescNode);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cards_list.xsd
int CCommDynCard::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pChild = NULL;
	
	GET_MANDATORY_CHILD_NODE(pActionNode, "CARD_COMMON_DATA", pChild);
	
	nStatus = CCommCard::DeSerializeXml(pChild,pszError);
	if( nStatus )
		return nStatus;
	
	GET_VALIDATE_CHILD(pActionNode,"CARD_STATE",(WORD*)m_state,CARD_STATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"NUMBER_CARD_STATUSES",&m_numb_of_status,_0_TO_10_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"NUMBER_CARD_CONFERENCES",&m_numb_of_conf,_0_TO_10_DECIMAL);
	
//NEW_STATUS_LIST
//	GET_CHILD_NODE(pActionNode, "CARD_STATUSES_LIST", pChild);
//	if (pChild)
//	{
//		DeSerializeXmlStatuses(pChild, pszError);
//	}
	DeSerializeXmlStatuses(pActionNode, pszError);



//	m_numb_of_units = 0;
//	for( int i=0; i<MAX_UNITS_IN_CARD; i++ )
//		POBJDELETE(m_pCardRsrc[i]);
	
	// deserialize sequence of Unit Resource descriptors
//	CXMLDOMElement*  pChildNode = NULL;
//	GET_FIRST_CHILD_NODE(pActionNode, "UNIT_RESOURCE_DESCRIPTOR", pChildNode);
//	
//	while( pChildNode  &&  m_numb_of_units < MAX_NUM_OF_UNITS )
//	{
//		CCardRsrc*	pCardRsrc = new CCardRsrc;
//		nStatus = pCardRsrc->DeSerializeXml(pChildNode,pszError);
//		if(nStatus != STATUS_OK)
//		{
//			POBJDELETE(pCardRsrc);
//			return nStatus;
//		}
//		m_pCardRsrc[m_numb_of_units] = pCardRsrc;
//		m_numb_of_units++;
//		GET_NEXT_CHILD_NODE(pActionNode, "UNIT_RESOURCE_DESCRIPTOR", pChildNode);
//	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SerializeXmlStatuses(CXMLDOMElement* pFatherNode/*, DWORD ObjToken*/)
{
/*	BYTE bChanged=FALSE;

	pCardNode->AddChildNode("OBJ_TOKEN", m_updateCounter);
	
	if (ObjToken==0xFFFFFFFF)
		bChanged=TRUE;
	else
	{
		if(m_updateCounter>ObjToken)
			bChanged=TRUE;
	}
	
	pCardNode->AddChildNode("CHANGED", bChanged,_BOOL);
	
	if (bChanged)
	{
		SerializeXml(pCardNode);
		
		if ( eEmpty == m_cards_type )
			pCardNode->AddChildNode( ::CardTypeToString(eEmpty) );
*/
		


//NEW_STATUS_LIST
//		for( int i=0; i<m_numb_of_status; i++ )
//			m_pCardStatus[i]->SerializeXml(pFatherNode);
		
		m_StatusList->SerializeXml(pFatherNode);



/*
	} // end if (bChanged)
*/
}

/////////////////////////////////////////////////////////////////////////////
int CCommDynCard::DeSerializeXmlStatuses(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	
	CXMLDOMElement*	pChild = NULL;
/*	
	m_bChanged=TRUE;
	
	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANGED",&m_bChanged,_BOOL);
	
	if (m_bChanged)
	{

		GET_CHILD_NODE(pActionNode, "CARD_SUMMARY_DESCRIPTOR", pChild);
		if( pChild ) 
		{
			nStatus = DeSerializeXml(pChild,pszError);
			if( nStatus!=STATUS_OK )
				return nStatus;
		}
*/		


//NEW_STATUS_LIST
/*		m_numb_of_status = 0;
		
		GET_FIRST_CHILD_NODE(pActionNode, "FAULT_DESC_CARD", pChild);
		
		while( pChild  &&  m_numb_of_status < MAX_STATUS_IN_LIST ) 
		{
			POBJDELETE(m_pCardStatus[m_numb_of_status]);
			m_pCardStatus[m_numb_of_status] = new CFaultCardDesc;
			nStatus = m_pCardStatus[m_numb_of_status]->DeSerializeXml(pChild,pszError);
			if( nStatus ) 
			{
				POBJDELETE(m_pCardStatus[m_numb_of_status]);
				return nStatus;
			}
			m_numb_of_status++;
			GET_NEXT_CHILD_NODE(pActionNode, "FAULT_DESC_CARD", pChild);
		}
*/	
	m_StatusList->DeSerializeXml(pChild,pszError, "");
		

/*
	} // end if (bChanged)
*/
	
	return STATUS_OK;
}




/////////////////////////////////////////////////////////////////////////////
BYTE  CCommDynCard::GetChanged() const
{
	return m_bChanged;
}
/////////////////////////////////////////////////////////////////////////////
DWORD  CCommDynCard::GetUpdateCounter() const
{
	return m_updateCounter;
}
//////////////////////////////////////////////////////////////////////
void CCommDynCard::IncreaseUpdateCounter()
{
    m_updateCounter++;
    if (m_updateCounter == 0xFFFFFFFF)
        m_updateCounter = 0;

	CCardsProcess* pProcess = (CCardsProcess*)CCardsProcess::GetProcess();
	pProcess->GetCardsMonitoringDB()->IncreaseUpdateCounter();
}

/*
/////////////////////////////////////////////////////////////////////////////
void  CCommDynCard::SetType(const WORD  type)
{
CCommCard::SetType(type);

  if (type==CARD_PRI_48)
  {
  m_pCardConfig[0] = new CCardConfig;
  m_pCardConfig[1] = new CCardConfig;
  m_pCardConfig[0]->SetTypeConfig(T1_A_CONFIG);
  m_pCardConfig[1]->SetTypeConfig(T1_B_CONFIG);
  m_numb_of_config=2;
  }
  
}
*/

/////////////////////////////////////////////////////////////////////////////
int CCommDynCard::AddStatus(const CFaultCardDesc&  other)
{
	if (m_numb_of_status>=MAX_STATUS_IN_LIST)
		return  STATUS_MAX_CARD_STATUS_EXCEEDED;
	
	if (FindStatus(other)!=NOT_FIND)
		return STATUS_CARD_STATUS_EXISTS;
	
	m_pCardStatus[m_numb_of_status] = new CFaultCardDesc(other);
	
	if (m_state != eMajorError) {
		switch (other.GetFaultLevel())  {
		case  MAJOR_ERROR_LEVEL:{   m_state = eMajorError;
			break;
								}
//		case  MINOR_ERROR_LEVEL:{   m_state = eMinorError;
//			break;
//								}
		case  STARTUP_ERROR_LEVEL:{ m_state = eCardStartup;
			break;
								  }
		default:				{   break; }
		}
	}
//	InformSnmpStatusChange( m_state, m_pCardStatus[m_numb_of_status]);
	m_numb_of_status++;
	IncreaseUpdateCounter();
	
	
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int CCommDynCard::AddStatusInStartup(const CFaultCardDesc&  other)
{
	if (m_numb_of_status>=MAX_STATUS_IN_LIST)
		return  STATUS_MAX_CARD_STATUS_EXCEEDED;
	
	if (FindStatus(other)!=NOT_FIND)
		return STATUS_CARD_STATUS_EXISTS;
	
	m_pCardStatus[m_numb_of_status] = new CFaultCardDesc(other);
//	InformSnmpStatusChange((WORD)other.GetFaultLevel() , m_pCardStatus[m_numb_of_status]);
	m_numb_of_status++;
	IncreaseUpdateCounter();
	
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CCommDynCard::FindStatus(const CFaultCardDesc&  other)
{
	for (int i=0;i<(int)m_numb_of_status;i++)
	{
		if (m_pCardStatus[i]!=NULL) {
			if (m_pCardStatus[i]->GetUnitId()==other.GetUnitId() &&
				m_pCardStatus[i]->GetErrorCode()==other.GetErrorCode())
				return i;
		}
	}
	return NOT_FIND;
}


/////////////////////////////////////////////////////////////////////////////
/*
int CCommDynCard::DelStatus(const CFaultCardDesc&  other)
{
	int ind;
	ind=FindStatus(other);
	if (ind==NOT_FIND) return STATUS_CARD_STATUS_NOT_EXISTS;
	
	PDELETE(m_pCardStatus[ind]);
	int i=0;
	for (i=0;i<(int)m_numb_of_status;i++)
	{
		if (m_pCardStatus[i]==NULL)
			break;
	}
	int j =0;
	for (j=i;j<(int)m_numb_of_status-1;j++)
	{
		m_pCardStatus[j]=m_pCardStatus[j+1] ;
	}
	m_pCardStatus[m_numb_of_status-1] = NULL;
	m_numb_of_status--;
	IncreaseUpdateCounter();
	//must be call ChangeCardState().
	return   STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CCommDynCard::DelStatus()
{
	for (int i=0;i<(int)m_numb_of_status;i++)
		PDELETE(m_pCardStatus[i]);
	m_numb_of_status=0;
	IncreaseUpdateCounter();
	return  STATUS_OK;
}
*/
/////////////////////////////////////////////////////////////////////////////
WORD  CCommDynCard::GetNumStatus () const
{
	return m_numb_of_status;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CCommDynCard::GetGlobalStatus () const
{
	if (m_numb_of_status)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CCommDynCard::IsCardConf () const
{
	if (m_numb_of_conf)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CFaultCardDesc*  CCommDynCard::GetFirstStatus()
{
	m_status_ind=1;
	return m_pCardStatus[0];
}

/////////////////////////////////////////////////////////////////////////////
CFaultCardDesc*  CCommDynCard::GetNextStatus()
{
	if (m_status_ind>=m_numb_of_status) return NULL;
	return m_pCardStatus[m_status_ind++];
}

/////////////////////////////////////////////////////////////////////////////
eCardState  CCommDynCard::GetState () const
{
	return m_state;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommDynCard::SetState(const eCardState state)
{
	m_state=state;
//	InformSnmpStatusChange(m_state);
	IncreaseUpdateCounter();
	
}

/////////////////////////////////////////////////////////////////////////////
void  CCommDynCard::SetState()
{
	BYTE faultLevel=0;
	CFaultCardDesc* fault = NULL;
	for (int i=0;i<(int)m_numb_of_status;i++)
	{
		if (m_pCardStatus[i]!=NULL) {
			if (m_pCardStatus[i]->GetFaultLevel()>faultLevel)
			{
				faultLevel=m_pCardStatus[i]->GetFaultLevel();
				fault = m_pCardStatus[i];
			}
		}
	}
	m_state = eNormal;
	switch (faultLevel)  {
	case  MAJOR_ERROR_LEVEL:{   m_state = eMajorError;
		break;
							}
//	case  MINOR_ERROR_LEVEL:{   m_state = eMinorError;
//		break;
//							}
		//case  STARTUP_ERROR_LEVEL:{ m_state = CARD_STARTUP;
		//						 break;
		//						  }
	default:				 {  break; }
	}
//	InformSnmpStatusChange(m_state,fault);
	IncreaseUpdateCounter();
}

/////////////////////////////////////////////////////////////////////////////
char*  CCommDynCard::GetUnitAsString(const CFaultCardDesc&  other)
{
	char* msg = new char[20];
	memset(msg,' ', 20);
	WORD unitId = other.GetUnitId();
/*
	if (unitId==0)
		strncpy(msg,"CARD_MANAGER",20);
	else {
		switch(m_cards_type)	{
		case  CARD_PRI_48 :{ strncpy(msg,"T1_",20);
			if (unitId==1)	 strncat(msg,"A",20);
			if (unitId==2)	 strncat(msg,"B",20);
			break;
						   }
		case  CARD_PRI_64 :{ strncpy(msg,"E1_",20);
			if (unitId==1)	 strncat(msg,"A",20);
			if (unitId==2)	 strncat(msg,"B",20);
			break;
						   }
		case  CARD_ATM_25 :{ strncpy(msg,"ATM_V_NIC_",20);
			if (unitId==1)	 strncat(msg,"25",20);
			break;
						   }
		case  CARD_ATM_155 :{ strncpy(msg,"ATM_V_NIC_",20);
			if (unitId==1)	 strncat(msg,"155",20);
			break;
							}
		case  CARD_H323 :
		case CARD_IP16:
		case CARD_IP32:
		case  CARD_ARROW_IP_PLUS_2:
		case  CARD_ARROW_IP_PLUS_1:{  if (unitId==1)	 strncpy(msg,"STACK_CONTROLLER",20);
			strncpy(msg,"RTP_PROCESSOR_",20);
			if (unitId==2)	 strncat(msg,"1",20);
			if (unitId==3)	 strncat(msg,"2",20);
			if (unitId==4)	 strncat(msg,"3",20);
			break;
								   }
		case  CARD_MUX :
		case  CARD_ARROW_MUX_1:{   strncpy(msg,"MUC_CPU_",20);
			if (unitId==1)	 strncat(msg,"A",20);
			if (unitId==2)	 strncat(msg,"B",20);
			if (unitId==3)	 strncat(msg,"C",20);
			if (unitId==4)	 strncat(msg,"D",20);
			if (unitId==5)	 strncpy(msg,"Xilinx",20);
			break;
							   }
		case  CARD_AUDIO :{  if (unitId==1)	 strncpy(msg,"Main_DSP",20);
			if (unitId>1 && unitId<20)	{
				strncpy(msg,"Codec_",20);
				char s[6];
				sprintf(s,"%i", unitId);
				strncat(msg,s,20);
			}
			break;
						  }
		case  CARD_MUX_PLUS :
		case  CARD_MUX_PLUS_1 :
		case  CARD_MUX_PLUS_2 :{   strncpy(msg,"MUC_CPU_",20);
			if (unitId==1)	 strncat(msg,"A",20);
			if (unitId==2)	 strncat(msg,"B",20);
			if (unitId==3)	 strncat(msg,"C",20);
			if (unitId==4)	 strncat(msg,"D",20);
			if (unitId==5)	 strncpy(msg,"Xilinx",20);
			break;
							   }
			
		default:		{	break; }
		}
	}
*/

	return msg;
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SetDisabledByError(WORD bl)
{
//	for (int i=0;i<(int)m_numb_of_units;i++)
//		m_pCardRsrc[i]->SetDisabledByError(bl);
//	IncreaseUpdateCounter();
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SetDisabledManually(WORD bl)
{
//	for (int i=0;i<(int)m_numb_of_units;i++)
//		m_pCardRsrc[i]->SetDisabledManually(bl);
//	IncreaseUpdateCounter();
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SetUpdateCounter(DWORD dwUpdateCounter)
{
	m_updateCounter = dwUpdateCounter;
}





/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::AddStatus( const BYTE subject, const DWORD errorCode,
                              const BYTE errorLevel, string description,
                              const DWORD userId, const DWORD unitId, const WORD theType )
{
	CFaultDesc *pFault;
	if (FAULT_CARD_SUBJECT == subject)
		pFault = new CFaultCardDesc(subject, errorCode, errorLevel, m_displayBoardId, unitId, theType, description.c_str(), eProcessCards);
	else if (FAULT_UNIT_SUBJECT == subject)
		pFault = new CFaultUnitDesc(subject, errorCode, errorLevel, m_displayBoardId, unitId, theType, description.c_str(), eProcessCards);
	else if (FAULT_MPL_SUBJECT == subject)
		pFault = new CFaultMplDesc(subject, errorCode, errorLevel, theType, description.c_str(), eProcessCards);
	else
		pFault = new CFaultGeneralDesc(subject, errorCode, errorLevel, description.c_str(), eProcessCards);

	char *pszMsg = pFault->SerializeString(0);
	CLogFltElement *fltElement = new CLogFltElement(subject, pszMsg);
	PDELETEA(pszMsg);
	POBJDELETE(pFault);
	
	CStructTm t;
	SystemGetTime(t);
	fltElement->SetTime(t);
	fltElement->SetMask(FAULTS_MASK);
	fltElement->SetCode(errorCode);
	fltElement->SetType(errorLevel);
	fltElement->SetUserId(userId);
	
	m_StatusList->AddFault(fltElement);
	
	m_numb_of_status++;
	IncreaseUpdateCounter();

//	DWORD faultId = (DWORD)fltElement;
//	return faultId;
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::DelStatusByErrorCode(const DWORD errorCode)
{
	m_StatusList->RemoveFaultByErrorCode(errorCode);
	
	m_numb_of_status--;
	IncreaseUpdateCounter();
}

/////////////////////////////////////////////////////////////////////////////
void CCommDynCard::DelStatusByErrorCodeUserId(const DWORD errorCode, const DWORD userId)
{
	m_StatusList->RemoveFaultByErrorCodeUserId(errorCode, userId);
	
	m_numb_of_status--;
	IncreaseUpdateCounter();
}









/////////////////////////////////////////////////////////////////////////////
// CCommCardDB

CCommCardDB::CCommCardDB()
{
	m_pProcess = (CCardsProcess*)CCardsProcess::GetProcess();
	
	for (int i=0; i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS; i++)
	{
		m_pCard[i]= new CCommDynCard;
		m_pCard[i]->SetBoardId( i % MAX_NUM_OF_BOARDS );
		m_pCard[i]->SetSubBoardId( (i / MAX_NUM_OF_BOARDS) + 1 );

	}
	
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		m_unAnswerdKA[i]= 0;
	}

	m_consist=0;
	
	m_updateCounter=0;
	m_bChanged=FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CCommCardDB&  CCommCardDB::operator=( const CCommCardDB& other )
{
	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		PDELETE(m_pCard[i]);
		if( other.m_pCard[i]==NULL)
			m_pCard[i]=NULL;
		else
			m_pCard[i]= new CCommDynCard(*other.m_pCard[i]);
	}

	m_consist=other.m_consist;
	
	m_bChanged= other.m_bChanged;
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommCardDB::CCommCardDB(const CCommCardDB &other)
:CPObject()
{
	m_pProcess = other.m_pProcess;

	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		if( other.m_pCard[i]==NULL)
			m_pCard[i]=NULL;
		else
			m_pCard[i]= new CCommDynCard(*other.m_pCard[i]);
	}
	
	m_consist=other.m_consist;
	
	m_bChanged= other.m_bChanged;
	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CCommCardDB::~CCommCardDB()
{
	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
		PDELETE(m_pCard[i]);
}

/////////////////////////////////////////////////////////////////////////////
void CCommCardDB::SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken)
{
	DWORD bChanged=FALSE;
	CXMLDOMElement* pCardsNode = pFatherNode->AddChildNode("CARDS_LIST");
	pCardsNode->AddChildNode("OBJ_TOKEN",m_updateCounter);
	
	if (ObjToken==0xFFFFFFFF)
		bChanged=TRUE;
	else
	{
		if(m_updateCounter>ObjToken)
			bChanged=TRUE;
		
	}
	
	pCardsNode->AddChildNode("CHANGED",bChanged,_BOOL);	

	if(bChanged)
	{
		int totalNumOfSlots    = m_pProcess->GetNumOfBoards(),
		    numOfNotEmptySlots = GetNumOfNotEmptySlots();

		pCardsNode->AddChildNode("NUMBER_OF_BOARDS", numOfNotEmptySlots);
	
		for(int i=0; i<totalNumOfSlots; i++)
		{
			if( NULL != m_pCard[i] )
			{
				eCardType curCardType = m_pCard[i]->GetType();
				
				if ( (eEmpty != curCardType) &&									// not empty
				     (NUM_OF_CARD_TYPES > curCardType) && (0 <= curCardType) )	// yet legal type
				{
					m_pCard[i]->SerializeXml(pCardsNode);
				}
			} // end if (NULL != m_pCard[i])
		
		} // end loop over m_pCard
	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cards_list.xsd
int CCommCardDB::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	m_bChanged=TRUE;
	
	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);	
	GET_VALIDATE_CHILD(pActionNode,"CHANGED",&m_bChanged,_BOOL);	

	if(m_bChanged)
	{
		int numOfSlots = 0;
		GET_VALIDATE_CHILD(pActionNode,"NUMBER_OF_BOARDS",&numOfSlots,_0_TO_DWORD);

		// deserialize sequence of Card descriptors
		CXMLDOMElement *pChildNode = NULL;
		
		GET_FIRST_CHILD_NODE(pActionNode, "CARD_SUMMARY_DESCRIPTOR", pChildNode);
		
		int cardIndex = 0;
		while( pChildNode  &&  cardIndex < numOfSlots )
		{
			nStatus = m_pCard[cardIndex]->DeSerializeXml(pChildNode,pszError);
			if(nStatus!=STATUS_OK)
				return nStatus;
			cardIndex++;
			GET_NEXT_CHILD_NODE(pActionNode, "CARD_SUMMARY_DESCRIPTOR", pChildNode);
		}
	}
	
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::Add(const CCommDynCard&  other)
{
	int ind = FindSlot(other); // ensure that slot is empty
	if (NOT_FIND != ind)
		return STATUS_BUSY_SLOT_NUMBER;
	
	int idx = GetIdx(other);
	if (NOT_FIND == idx)
		return STATUS_ILLEGAL;

	TRACESTR(eLevelInfoNormal) << "\nCCommCardDB::Add "
	                       << "(boardId " << other.GetBoardId()
	                       << ", subBoardId " << other.GetSubBoardId()
	                       << ", idx " << idx << ")";

	PDELETE(m_pCard[idx]);
	m_pCard[idx] = new CCommDynCard(other);

	//	InformSnmpChange();
	IncreaseUpdateCounter();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::Update(const CCommDynCard&  other)
{
	int ind=FindSlot(other);
	if (NOT_FIND == ind)
		return STATUS_SLOT_EMPTY;
	
	/*
	if (m_pCard[ind]->GetNumConf())
	return STATUS_SLOT_IN_CONF;
	*/
	
	CCommDynCard* pCard = new CCommDynCard(other);
	pCard->SetUpdateCounter(m_pCard[ind]->GetUpdateCounter());
	
	int numb_of_status=m_pCard[ind]->GetNumStatus();
	if (numb_of_status) {
		CFaultCardDesc* pCardDesc = m_pCard[ind]->GetFirstStatus();
		pCard->AddStatus(*pCardDesc);
		for(;;) {
			pCardDesc = m_pCard[ind]->GetNextStatus();
			if (pCardDesc==NULL)
				break;
			pCard->AddStatus(*pCardDesc);
		}
	}
	
//	int numb_of_conf=m_pCard[ind]->GetNumConf();
//	if (numb_of_conf) {
//		CCardConf* pCardConf = m_pCard[ind]->GetFirstConf();
//		pCard->AddConf(*pCardConf);
//		for(;;) {
//			pCardConf = m_pCard[ind]->GetNextConf();
//			if (pCardConf==NULL)
//				break;
//			pCard->AddConf(*pCardConf);
//		}
//	}
	
	pCard->SetState(m_pCard[ind]->GetState());
	
	POBJDELETE(m_pCard[ind]);
	//m_pCard[ind] = new CCommDynCard(other);
	m_pCard[ind] = new CCommDynCard(*pCard);
	POBJDELETE(pCard);
	
//	InformSnmpChange();
	IncreaseUpdateCounter();
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////

int CCommCardDB::Cancel(const WORD boardId, const WORD subBoardId)
{
	int ind=FindSlot(boardId, subBoardId);
	if (NOT_FIND == ind) 
		return STATUS_SLOT_EMPTY;
	
/*
	if ( ::GetpSystemCfg()->IsSnmp() )
	{ 
		//Delete Card from MGC Status table.
		CMgcAPI::CMgcAPIRc   Rc;
		Rc = MgcAPI.DeleteCardFromTable(slotNumber);
		if (CMgcAPI::Ok != Rc)
			TRACESTR(eLevelInfoNormal | SNMP_TRACE) << "Error at:CCommCardDB::Cancel: Can't delete entry from MGC Status table.";
	}
*/

	POBJDELETE(m_pCard[ind]);
	m_pCard[ind]= new CCommDynCard;
	m_pCard[ind]->SetBoardId(boardId);
	m_pCard[ind]->SetSubBoardId(subBoardId);
	
//	InformSnmpChange();
	IncreaseUpdateCounter();
	return   STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::FindSlot(const CCommDynCard&  other)
{
	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		if (m_pCard[i]!=NULL)
		{
			if ( (m_pCard[i]->GetBoardId()    == other.GetBoardId()) &&
				 (m_pCard[i]->GetSubBoardId() == other.GetSubBoardId()) )
			{
				if (m_pCard[i]->GetType()!=eEmpty)
					return i;
				else
					return NOT_FIND;
			}
		}
	}
	return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::FindSlot(const WORD boardId, const WORD subBoardId)
{
	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		if (m_pCard[i]!=NULL)
		{
			if ( (boardId    == m_pCard[i]->GetBoardId()) &&
				 (subBoardId == m_pCard[i]->GetSubBoardId()) )
			{

				if (m_pCard[i]->GetType()!=eEmpty)
					return i;
				else
					return NOT_FIND;
			}
		}
	}
	return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::GetIdx(const CCommDynCard&  other)
{
	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		if (m_pCard[i]!=NULL)
		{
			if ( (m_pCard[i]->GetBoardId()    == other.GetBoardId()) &&
				 (m_pCard[i]->GetSubBoardId() == other.GetSubBoardId()) )
			{
				return i;
			}
		}
	}

	return NOT_FIND;
}

/*
if (m_pCard[slotNumber]->GetType()!=EMPTY)
return slotNumber;
else
return NOT_FIND;
*/

/////////////////////////////////////////////////////////////////////////////
const CCommDynCard*  CCommCardDB::GetCurrentCard(const WORD boardId, const WORD subBoardId)
{
	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		if (m_pCard[i]!=NULL) {
			if ( (boardId    == m_pCard[i]->GetBoardId()) &&
				 (subBoardId == m_pCard[i]->GetSubBoardId()) )
				return m_pCard[i];
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::AddStatusInStartup( const CFaultCardDesc&  other, const WORD boardId, const WORD subBoardId)
{
	int ind=FindSlot(boardId, subBoardId);
	if (NOT_FIND == ind)
		return STATUS_SLOT_EMPTY;

	m_pCard[ind]->AddStatusInStartup(other);
	IncreaseUpdateCounter();

	return   STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::AddStatus( const BYTE  subject, const DWORD error_code,
                            const BYTE faultLevel, const WORD boardId, const WORD subBoardId,
                            const WORD unitId, string description, const WORD theType)
{
	CFaultCardDesc *pNewStatus = new CFaultCardDesc( subject, error_code, faultLevel,
	                                                 boardId, unitId, theType, description.c_str() );
	AddStatus(*pNewStatus, boardId, subBoardId);
	POBJDELETE(pNewStatus);

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::AddStatus( const CFaultCardDesc& other, const WORD boardId, const WORD subBoardId)
{
	int ind=FindSlot(boardId, subBoardId);
	if (NOT_FIND == ind)
		return STATUS_SLOT_EMPTY;

	m_pCard[ind]->AddStatus(other);
	IncreaseUpdateCounter();

	return   STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/*
int CCommCardDB::DelStatus(const BYTE  subject, const DWORD error_code, const BYTE  faultLevel, const WORD boardId, const WORD unitId)
{
	CFaultCardDesc *pNewStatus = new CFaultCardDesc(subject, error_code, faultLevel, boardId, unitId);

	WORD subBoardId = FIXED_SUB_BOARD_ID;
	DelStatus(*pNewStatus, boardId, subBoardId);

	POBJDELETE(pNewStatus);
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::DelStatus(const CFaultCardDesc&  other, const WORD boardId, const WORD subBoardId)
{
	int ind;
	ind=FindSlot(boardId, subBoardId);
	if (ind==NOT_FIND) return STATUS_SLOT_EMPTY;
	m_pCard[ind]->DelStatus(other);
	IncreaseUpdateCounter();
	return   STATUS_OK;
}
*/



/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::AddStatusNew( const BYTE  subject, const DWORD error_code, const BYTE faultLevel,
                               DWORD userId, const WORD boardId, const WORD subBoardId,
                               const WORD unitId, string description, const WORD theType )
{
	int ind = FindSlot(boardId, subBoardId);
	if (ind==NOT_FIND)
		return STATUS_SLOT_EMPTY;

	m_pCard[ind]->AddStatus(subject, error_code, faultLevel, description, userId, unitId, theType);

	IncreaseUpdateCounter();
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::DelStatusNewByErrorCode(const DWORD error_code, const WORD boardId, const WORD subBoardId)
{
	int ind = FindSlot(boardId, subBoardId);
	if (ind==NOT_FIND)
		return STATUS_SLOT_EMPTY;

	m_pCard[ind]->DelStatusByErrorCode(error_code);

	IncreaseUpdateCounter();
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CCommCardDB::DelStatusNewByErrorCodeUserId(const DWORD error_code, const DWORD userId, const WORD boardId, const WORD subBoardId)
{
	int ind = FindSlot(boardId, subBoardId);
	if (ind==NOT_FIND)
		return STATUS_SLOT_EMPTY;

	m_pCard[ind]->DelStatusByErrorCodeUserId(error_code, userId);

	IncreaseUpdateCounter();
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CCommCardDB::SetCardState(WORD boardId, WORD subBoardId, const eCardState cardState)
{
	STATUS retStatus = STATUS_OK;
	
	int ind = FindSlot(boardId, subBoardId);
	if (NOT_FIND == ind || ind >= MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS)
		retStatus = STATUS_SLOT_EMPTY;
	
	if (STATUS_OK == retStatus)
	{
		m_pCard[ind]->SetState(cardState);
	

	}

	IncreaseUpdateCounter();

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
eCardState CCommCardDB::GetCardState(WORD boardId, WORD subBoardId)
{
	eCardState cardState = eNormal;

	int ind = FindSlot(boardId, subBoardId);
	if (NOT_FIND == ind)
		cardState = eSimulation; // a sign for error 
		
	else
	{
		cardState = m_pCard[ind]->GetState();
	}
	
	return cardState;
}

STATUS CCommCardDB::SetCardType(WORD boardId, WORD subBoardId, const eCardType cardType)
{
	STATUS retStatus = STATUS_OK;

	int ind = FindSlot(boardId, subBoardId);
	if (NOT_FIND == ind || ind >= MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS)
		retStatus = STATUS_SLOT_EMPTY;

	if (STATUS_OK == retStatus)
	{
		m_pCard[ind]->SetType(cardType);


	}

	IncreaseUpdateCounter();

	return retStatus;
}
/////////////////////////////////////////////////////////////////////////////
eCardType CCommCardDB::GetCardType(WORD boardId, WORD subBoardId)
{
	eCardType cardType = eEmpty;

	int ind = FindSlot(boardId, subBoardId);
	if (NOT_FIND != ind)
	{
		cardType = m_pCard[ind]->GetType();
	}
	
	return cardType;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CCommCardDB::GetChanged() const
{
	return m_bChanged;
}

/////////////////////////////////////////////////////////////////////////////
void CCommCardDB::IncreaseUpdateCounter()
{
    m_updateCounter++;
    if (m_updateCounter == 0xFFFFFFFF)
        m_updateCounter = 0;
}


/*
/////////////////////////////////////////////////////////////////////////////

WORD CCommCardDB::IsSimulationMode(const WORD slotNumber)
{
	int ind;
	ind=FindSlot(slotNumber);
	if (ind==NOT_FIND) return STATUS_SLOT_EMPTY;
	
	return m_pCard[ind]->IsSimulationMode();
}


*/
/////////////////////////////////////////////////////////////////////////////
WORD CCommCardDB::GetState()
{
	WORD state=MCU_NORMAL_STATE;


	for (int i=0;i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;i++)
	{
		if ((m_pCard[i]->GetState())==eMajorError ||
			(m_pCard[i]->GetState())==eCardStartup ||
			(m_pCard[i]->GetState())==eNoConnection) {
			state=MCU_MAJOR_STATE;
			break;
		}
		if (m_pCard[i]->GetState()==eMinorError )
			state=MCU_MINOR_STATE;
		
	}
	
	
	return state;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCommCardDB::GetNumOfNotEmptySlots() const
{
	WORD numOfNotEmptySlots = 0;
	
	for (int i=0; i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS; i++)
	{
		if (NULL != m_pCard[i])
		{
			eCardType curCardType = m_pCard[i]->GetType();
			
			if ( (eEmpty != curCardType) &&									// not empty
			     (NUM_OF_CARD_TYPES > curCardType) && (0 <= curCardType) )	// yet legal type
			{
				numOfNotEmptySlots++;
			}
		} // end if (NULL != m_pCard[i])
		
	} // end loop over m_pCard

	return numOfNotEmptySlots;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCommCardDB::GetNumOfMediaBoards() const
{
	WORD numOfMediaBoards = 0;
	
	for (int i=0; i<MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS; i++)
	{
		if (NULL != m_pCard[i])
		{
			eCardType curCardType = m_pCard[i]->GetType();
			bool isMediaBoard = m_pProcess->IsMediaCard(curCardType);
			
			if ( true ==  isMediaBoard)
			{
				numOfMediaBoards++;
			}
		}
		
	} // end loop over m_pCard

	return numOfMediaBoards;
}

/////////////////////////////////////////////////////////////////////////////
bool CCommCardDB::IsAnyMediaBoardExistsInDB()
{
    bool isExist = false;
    
    if ( 0 != GetNumOfMediaBoards() )
    {
        isExist = true;
    }

    return isExist;
}

/////////////////////////////////////////////////////////////////////////////
bool CCommCardDB::IsAnyCardExistsInDB()
{
    bool isExist = false;
    
    if ( 0 != GetNumOfNotEmptySlots() )
    {
        isExist = true;
    }

    return isExist;
}

/////////////////////////////////////////////////////////////////////////////
bool CCommCardDB::IsSlotEmpty(WORD boardId, WORD subBoardId)
{
	bool isEmpty = true;
	
	int ind = FindSlot(boardId, subBoardId);
	if (NOT_FIND != ind)
	{
		eCardType cardType = m_pCard[ind]->GetType();

		if (eEmpty != cardType)
			isEmpty = false;
	}

	return isEmpty;
}
/////////////////////////////////////////////////////////////////////////////
void CCommCardDB::SetUnAnswerdKA(WORD boardId,DWORD unAnswerdKA)
{
	if(boardId<MAX_NUM_OF_BOARDS)
		m_unAnswerdKA[boardId] = unAnswerdKA;
}
/////////////////////////////////////////////////////////////////////////////

/*
/////////////////////////////////////////////////////////////////////////////
WORD  CCommCardDB::GetConfigClockMasterSlot () const
{
	return m_config_clock_master_slot;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommCardDB::SetConfigClockMasterSlot(const WORD slotNumber)
{
	IncreaseUpdateCounter();
	m_config_clock_master_slot=slotNumber;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCommCardDB::GetConfigClockBackupSlot () const
{
	return m_config_clock_backup_slot;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommCardDB::SetConfigClockBackupSlot(const WORD slotNumber)
{
	IncreaseUpdateCounter();
	m_config_clock_backup_slot=slotNumber;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCommCardDB::GetConfigClockMasterUnit () const
{
	return m_config_clock_master_unit;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommCardDB::SetConfigClockMasterUnit(const WORD unitId)
{
	IncreaseUpdateCounter();
	m_config_clock_master_unit=unitId;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCommCardDB::GetConfigClockBackupUnit () const
{
	return m_config_clock_backup_unit;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommCardDB::SetConfigClockBackupUnit(const WORD unitId)
{
	IncreaseUpdateCounter();
	m_config_clock_backup_unit=unitId;
}

//////////////////////////////////////////////////////////////////////////////
// CCardConf

CCardConf::CCardConf()
{
  m_name_conf[0]='\0';
  m_confId=0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
CCardConf::CCardConf(const CCardConf &other)
{
  strncpy(m_name_conf, other.m_name_conf,H243_NAME_LEN);
  m_confId=other.m_confId;
}

/////////////////////////////////////////////////////////////////////////////
CCardConf::~CCardConf()
{
}

/////////////////////////////////////////////////////////////////////////////
void CCardConf::Serialize(WORD format, ostream &m_ostr,DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
	 m_ostr <<  m_name_conf << "\n";	//CONF_NAME

  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_name_conf,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << tmp << "\n";	//CONF_NAME
  }
  m_ostr <<  m_confId << "\n";

}


/////////////////////////////////////////////////////////////////////////////
void CCardConf::DeSerialize(WORD format, istream &m_istr ,DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
 if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
	m_istr.getline(m_name_conf,H243_NAME_LEN+1,'\n');
 else
	m_istr.getline(m_name_conf,H243_NAME_LEN_OLD+1,'\n');

 m_istr >>  m_confId;

}

/////////////////////////////////////////////////////////////////////////////
void CCardConf::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pDescNode = pFatherNode->AddChildNode("CARD_CONFERENCE");

	pDescNode->AddChildNode("ID",m_confId);
	pDescNode->AddChildNode("NAME",m_name_conf);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cards_list.xsd
int CCardConf::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"ID",&m_confId,_0_TO_DWORD);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_name_conf,_0_TO_H243_NAME_LENGTH);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CCardConf::GetNameConf () const
{
	return m_name_conf;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardConf::SetNameConf(const char* nameConf)
{
  int len=strlen(nameConf);
  strncpy(m_name_conf, nameConf, H243_NAME_LEN );
  if (len>H243_NAME_LEN-1)
	  m_name_conf[H243_NAME_LEN-1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
void  CCardConf::SetConferenceId(const DWORD confId)
{
  m_confId=confId;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CCardConf::GetConferenceId () const
{
  return m_confId;
}


/////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////
// CCardRsrc

CCardRsrc::CCardRsrc()
{
  m_unitId=0xFFFF;
  m_unitType=0;
  m_unitCfg=0xFF;
  m_unitStatus=0;
  m_portsNumber=0;
  m_activMask1=0;
  m_activMask2=0;
  m_serviceName[0]='\0';
  m_utilization = 0xFFFF;
  m_currentType = 0xFF;
  m_UpdateCounter=0;
}

/////////////////////////////////////////////////////////////////////////////
CCardRsrc::CCardRsrc(const CCardRsrc &other)
{
  m_unitId=other.m_unitId;
  m_unitType=other.m_unitType;
  m_unitCfg=other.m_unitCfg;
  m_unitStatus=other.m_unitStatus;
  m_portsNumber=other.m_portsNumber;
  m_activMask1=other.m_activMask1;
  m_activMask2=other.m_activMask2;
  strncpy(m_serviceName,other.m_serviceName,NET_SERVICE_PROVIDER_NAME_LEN);
  m_utilization = other.m_utilization;
  m_currentType = other.m_currentType;
  m_UpdateCounter=other.m_UpdateCounter;
}

CCardRsrc& CCardRsrc:: operator =(const CCardRsrc &other)
{
  m_unitId=other.m_unitId;
  m_unitType=other.m_unitType;
  m_unitCfg=other.m_unitCfg;
  m_unitStatus=other.m_unitStatus;
  m_portsNumber=other.m_portsNumber;
  m_activMask1=other.m_activMask1;
  m_activMask2=other.m_activMask2;
  strncpy(m_serviceName,other.m_serviceName,NET_SERVICE_PROVIDER_NAME_LEN);
  m_utilization = other.m_utilization;
  m_currentType = other.m_currentType;
  m_UpdateCounter=other.m_UpdateCounter;
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCardRsrc::~CCardRsrc()
{
}

/////////////////////////////////////////////////////////////////////////////
void CCardRsrc::Serialize(WORD format, ostream &m_ostr, DWORD apiNum)
{
  m_ostr <<  m_unitId << "\n";
  m_ostr <<  (WORD)m_unitType << "\n";

  if (m_unitType == BRIDGE_UNIT_TYPE)	// for configuring Audio bridge
  {
	  if ( (apiNum >= API_NUM_AUD_BRDG_6x2) || (format != OPERATOR_MCMS) )
		  m_ostr <<  (WORD)m_unitCfg << "\n";
	  else
		  m_ostr <<  (WORD)0xFF << "\n";
  }
  
  else									// for configuring Mux cpu
	  m_ostr <<  (WORD)m_unitCfg << "\n";

  m_ostr <<  m_serviceName << "\n";
  if (format==OPERATOR_MCMS) {
	m_ostr <<  m_unitStatus << "\n";
	m_ostr <<  m_portsNumber << "\n";
	m_ostr <<  m_activMask1 << "\n";
	m_ostr <<  m_activMask2 << "\n";
    if( apiNum >= API_NUM_H323_FLEXIBLE_PORTS )
        m_ostr << m_utilization << "\n";
  }

  if  ((apiNum >= API_NUM_VIDEO_PLUS_RSRC_REP )||(apiNum==0))
	   m_ostr <<  (WORD)m_currentType << "\n";

}


/////////////////////////////////////////////////////////////////////////////
void CCardRsrc::DeSerialize(WORD format, istream &m_istr, DWORD apiNum)
{
  m_istr >>  m_unitId;
  WORD tmp;
  m_istr >> tmp;
  m_unitType = (BYTE)tmp;
  m_istr >> tmp;
  m_unitCfg = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr.getline(m_serviceName,NET_SERVICE_PROVIDER_NAME_LEN+1,'\n');
  if (format==OPERATOR_MCMS) {
	m_istr >>  m_unitStatus;
	m_istr >>  m_portsNumber;
	m_istr >>  m_activMask1;
	m_istr >>  m_activMask2;
    if( apiNum >= API_NUM_H323_FLEXIBLE_PORTS )
        m_istr >> m_utilization;
  }

  if (( apiNum >= API_NUM_VIDEO_PLUS_RSRC_REP )||(apiNum==0))
  {
	  m_istr >> tmp; 
	  m_currentType = (BYTE)tmp ;
  }
}

/////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pUnitNode = pFatherNode->AddChildNode("UNIT_RESOURCE_DESCRIPTOR");

	pUnitNode->AddChildNode("UNIT_NUMBER",m_unitId);
	pUnitNode->AddChildNode("UNIT_TYPE",m_unitType,UNIT_TYPE_ENUM);
	pUnitNode->AddChildNode("CONFIGURATION_DATA",m_unitCfg);
	pUnitNode->AddChildNode("NETWORK_SERVICE_NAME",m_serviceName);
	CXMLDOMElement* pChild = NULL;
	pChild = pUnitNode->AddChildNode("UNIT_STATUS");
	if( pChild ) {
		pChild->AddChildNode("ID",m_unitStatus);

		char* pszTemp = new char[256];
		memset(pszTemp,0,256);
		if( IsAvailable() )
			strcat(pszTemp,"Available;");
		if( IsActive() )
			strcat(pszTemp,"Active;");
		if( IsDisabledByError() )
			strcat(pszTemp,"Disabled(by error);");
		if( IsDisabledManually() )
			strcat(pszTemp,"Disabled(manually);");
		if( IsDiagnostics() )
			strcat(pszTemp,"Diagnostics;");

		pChild->AddChildNode("DESCRIPTION",pszTemp);
		PDELETEA(pszTemp);
	}
	pUnitNode->AddChildNode("PORTS_NUMBER",m_portsNumber);
	pUnitNode->AddChildNode("ACTIVE_MASK_1",m_activMask1);
	pUnitNode->AddChildNode("ACTIVE_MASK_2",m_activMask2);
	pUnitNode->AddChildNode("UTILIZATION",m_utilization);
	pUnitNode->AddChildNode("CURRENT_TYPE",m_currentType);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cards_list.xsd
int CCardRsrc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"UNIT_NUMBER",&m_unitId,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"UNIT_TYPE",&m_unitType,UNIT_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"CONFIGURATION_DATA",&m_unitCfg,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"NETWORK_SERVICE_NAME",m_serviceName,NET_SERVICE_PROVIDER_NAME_LENGTH);

	CXMLDOMElement*	pChild = NULL;
	GET_CHILD_NODE(pActionNode, "UNIT_STATUS", pChild);
	if( pChild )
		GET_VALIDATE_CHILD(pChild,"ID",&m_unitStatus,_0_TO_DWORD);

	GET_VALIDATE_CHILD(pActionNode,"PORTS_NUMBER",&m_portsNumber,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"ACTIVE_MASK_1",&m_activMask1,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"ACTIVE_MASK_2",&m_activMask2,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"UTILIZATION",&m_utilization,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CURRENT_TYPE",&m_currentType,_0_TO_DWORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetUnitId(const WORD unitId)
{
  m_unitId=unitId;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrc::GetUnitId () const
{
  return m_unitId;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetUnitType(const BYTE unitType)
{
  m_unitType=unitType;
}


/////////////////////////////////////////////////////////////////////////////
BYTE  CCardRsrc::GetUnitType () const
{
  return m_unitType;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetUnitCfg(const BYTE unitCfg)
{
  m_unitCfg=unitCfg;
}


/////////////////////////////////////////////////////////////////////////////
BYTE  CCardRsrc::GetUnitCfg () const
{
  return m_unitCfg;
}


/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetUnitStatus(const DWORD unitStatus)
{
  m_unitStatus=unitStatus;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CCardRsrc::GetUnitStatus () const
{
  return m_unitStatus;
}

/////////////////////////////////////////////////////////////////////////////
void   CCardRsrc::SetActive(WORD bl)
{
   if (bl==FALSE)
	 m_unitStatus &= 0xFFFFFFFE;
   else
	 m_unitStatus |= 0x00000001;
}


/////////////////////////////////////////////////////////////////////////////
void   CCardRsrc::SetDisabledByError(WORD bl)
{
   if (bl==FALSE)
	 m_unitStatus &= 0xFFFFFFFD;
   else
	 m_unitStatus |= 0x00000002;
}



/////////////////////////////////////////////////////////////////////////////
void   CCardRsrc::SetDisabledManually(WORD bl)
{
   if (bl==FALSE)
	 m_unitStatus &= 0xFFFFFFFB;
   else
	 m_unitStatus |= 0x00000004;
}

/////////////////////////////////////////////////////////////////////////////
void   CCardRsrc::SetDiagnostics(WORD bl)
{
   if (bl==FALSE)
	 m_unitStatus &= 0xFFFFFFF7;
   else
	 m_unitStatus |= 0x00000008;
}



/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrc::IsActive() const
{
	if ((m_unitStatus & 0x00000001)==1)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrc::IsDisabledByError() const
{
	if ((m_unitStatus & 0x00000002)==2)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrc::IsDisabledManually() const
{
	if ((m_unitStatus & 0x00000004)==4)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrc::IsDiagnostics() const
{
	  if ((m_unitStatus & 0x00000008)==8)
	  return TRUE;
	else
	  return FALSE;
}

WORD CCardRsrc::IsAvailable() const
{
	if(m_unitStatus==0)
		return TRUE;
	else
		return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetPortsNumber(const WORD portsNum)
{
  m_portsNumber=portsNum;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrc::GetPortsNumber () const
{
  return m_portsNumber;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetActivMask1(const DWORD activMask)
{
  m_activMask1=activMask;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CCardRsrc::GetActivMask1 () const
{
  return m_activMask1;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetActivMask2(const DWORD activMask)
{
  m_activMask2=activMask;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CCardRsrc::GetActivMask2 () const
{
  return m_activMask2;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CCardRsrc::GetName () const
{
	return m_serviceName;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetName(const char* name)
{
  int len=strlen(name);
  strncpy(m_serviceName, name, NET_SERVICE_PROVIDER_NAME_LEN);
  if (len>NET_SERVICE_PROVIDER_NAME_LEN)
	  m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN-1]='\0';
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetUtilization(const DWORD promil)
{
  m_utilization = promil;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CCardRsrc::GetUtilization () const
{
  return m_utilization;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrc::SetCurrentType(const BYTE type)
{
  m_currentType = type;
}


/////////////////////////////////////////////////////////////////////////////
BYTE  CCardRsrc::GetCurrentType () const
{
  return m_currentType;
}

DWORD CCardRsrc::GetUpdateCounter () const
{
  return m_UpdateCounter;
}
void CCardRsrc::IncreaseUpdateCounter () 
{
   m_UpdateCounter++;
}


//////////////////////////////////////////////////////////////////////////////
// CUnitStr

CUnitStr::CUnitStr()
{
  m_slot=0xFF;
  m_unitId=0xFF;
}

/////////////////////////////////////////////////////////////////////////////
CUnitStr::~CUnitStr()
{
}

/////////////////////////////////////////////////////////////////////////////
void CUnitStr::Serialize(WORD format, ostream &m_ostr)
{
  // assuming format = OPERATOR_MCMS

  m_ostr <<  m_slot << "\n";
  m_ostr <<  m_unitId << "\n";

}
/////////////////////////////////////////////////////////////////////////////

void CUnitStr::Serialize(char* userAllocatedBuf)
{
  ostream ostr(userAllocatedBuf,MAX_PRIVATE_NAME_LEN);

  Serialize(OPERATOR_MCMS,ostr);
}

/////////////////////////////////////////////////////////////////////////////
void CUnitStr::DeSerialize(WORD format, istream &m_istr)
{
  // assuming format = OPERATOR_MCMS

  m_istr >>  m_slot;
  m_istr >>  m_unitId;
}

/////////////////////////////////////////////////////////////////////////////

void CUnitStr::DeSerialize(char* str)
{
  istream istr(str,MAX_PRIVATE_NAME_LEN);

  DeSerialize(OPERATOR_MCMS,istr);
}

/////////////////////////////////////////////////////////////////////////////
void CUnitStr::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pUnitNode = pFatherNode->AddChildNode("UNIT_POSITION");

	if( pUnitNode ) {
		pUnitNode->AddChildNode("BOARD_NUMBER",m_slot,BOARD_ENUM);
		pUnitNode->AddChildNode("UNIT_NUMBER",m_unitId,UNIT_ENUM);
	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  trans_card.xsd
int CUnitStr::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"BOARD_NUMBER",&m_slot,BOARD_ENUM);
	if( nStatus )
		return nStatus;
	GET_VALIDATE_CHILD(pActionNode,"UNIT_NUMBER",&m_unitId,UNIT_ENUM);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void  CUnitStr::SetUnitId(const WORD unitId)
{
  m_unitId=unitId;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CUnitStr::GetUnitId () const
{
  return m_unitId;
}

/////////////////////////////////////////////////////////////////////////////
void  CUnitStr::SetSlotNumb(const WORD slot)
{
  m_slot=slot;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CUnitStr::GetSlotNumb () const
{
  return m_slot;
}

/////////////////////////////////////////////////////////////////////////////

void CUnitStr::DeSerializeSpaces(istream& istr)
{
	char buff[20];
	istr.get(buff,19,' ');
  m_slot = atoi(buff);
	istr.ignore(1);
	istr.get(buff,19,' ');
  m_unitId = atoi(buff);

}

void CUnitStr::DeSerializeSpaces(char* s)
{
	istream istr(s,MAX_PRIVATE_NAME_LEN);
	DeSerializeSpaces(istr);
}

void	CUnitStr::SerializeSpaces(char* s)
{
	ostream  ostr(s,MAX_PRIVATE_NAME_LEN);
	SerializeSpaces(ostr);
}

void	CUnitStr::SerializeSpaces(ostream& ostr)
{
	ostr << m_slot << " " << m_unitId << " ";
}



/////////////////////////////////////////////////////////////////////////////
// CUnitList

CUnitList::CUnitList()
{
  m_numb_of_units=0;
  for (int i=0;i<MAX_UNITS_IN_LIST;i++)
  {
	m_pUnit[i] = NULL;
  }
  m_unit_ind=0;
}


/////////////////////////////////////////////////////////////////////////////
CUnitList::CUnitList(const CUnitList &other)
{
  m_numb_of_units=other.m_numb_of_units;
  m_unit_ind=other.m_unit_ind;
  for (int i=0;i<MAX_UNITS_IN_LIST;i++)
  {
	if( other.m_pUnit[i]==NULL)
	   m_pUnit[i]=NULL;
	else
	   m_pUnit[i]= new CUnitStr(*other.m_pUnit[i]);
  }

}

/////////////////////////////////////////////////////////////////////////////
CUnitList::~CUnitList()
{
  for (int i=0;i<MAX_UNITS_IN_LIST;i++)
	PDELETE(m_pUnit[i]);
}


/////////////////////////////////////////////////////////////////////////////
// CUnitList Serialization

void CUnitList::Serialize(WORD format, ostream &m_ostr)
{
  // assuming format = OPERATOR_MCMS

  int i;
  m_ostr <<  m_numb_of_units  << "\n";
  if (m_numb_of_units!=0)
	 m_ostr << "~" << "\n";

  for (i=0;i<(int)m_numb_of_units;i++)
  {
	m_pUnit[i]->Serialize(format, m_ostr);
	m_ostr << "~" << "\n";
  }

}


/////////////////////////////////////////////////////////////////////////////
// CUnitList Deserialization

void CUnitList::DeSerialize(WORD format, istream &m_istr)
{
  // assuming format = OPERATOR_MCMS

  int i;
  char s;

  m_istr >> m_numb_of_units;
  if (m_numb_of_units!=0)
  {
	m_istr >> s;
	if (s!='~') return;
  }
  for (i=0;i<(int)m_numb_of_units;i++)
  {
	m_istr.ignore(1);
	m_pUnit[i]= new CUnitStr;
	m_pUnit[i]->DeSerialize(format, m_istr);
	m_istr >> s;
	if (s!='~') return;
  }

}

/////////////////////////////////////////////////////////////////////////////
void CUnitList::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pUnitsListNode = pFatherNode->AddChildNode("UNITS_LIST");

	for( int i=0; i<m_numb_of_units; i++ )
		m_pUnit[i]->SerializeXml(pUnitsListNode);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  trans_card.xsd
int CUnitList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	m_numb_of_units = 0;
	for( int i=0; i<MAX_UNITS_IN_LIST; i++ )
		POBJDELETE(m_pUnit[i]);

	CXMLDOMElement *pChild = NULL;

	GET_FIRST_MANDATORY_CHILD_NODE(pActionNode, "UNIT_POSITION", pChild);	// at least one should present

	while( pChild  &&  m_numb_of_units < MAX_UNITS_IN_LIST )
	{
		m_pUnit[m_numb_of_units] = new CUnitStr;
		nStatus = m_pUnit[m_numb_of_units]->DeSerializeXml(pChild,pszError);
		
		if( nStatus )
		{
			POBJDELETE(m_pUnit[m_numb_of_units]);
			return nStatus;
		}
		m_numb_of_units++;
		GET_NEXT_CHILD_NODE(pActionNode, "UNIT_POSITION", pChild);
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CUnitList::Add(const CUnitStr&  other)
{
  if (m_numb_of_units>=MAX_UNITS_IN_LIST)
	 return  STATUS_ILLEGAL;

  if (FindUnit(other)!=NOT_FIND)
	return STATUS_UNIT_EXISTS_IN_LIST;


  m_pUnit[m_numb_of_units] = new CUnitStr(other);
  m_numb_of_units++;

  return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

int CUnitList::Cancel(const CUnitStr&  other)
{
  int ind;
  ind=FindUnit(other);
  if (ind==NOT_FIND) return STATUS_UNIT_NOT_EXISTS_IN_LIST;

  PDELETE(m_pUnit[ind]);

  for (int i=0;i<(int)m_numb_of_units;i++)
  {
	  if (m_pUnit[i]==NULL)
		 break;
  }
  for (int j=i;j<(int)m_numb_of_units-1;j++)
  {
	 m_pUnit[j]=m_pUnit[j+1] ;
  }
  m_pUnit[m_numb_of_units-1] = NULL;
  m_numb_of_units--;

  return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CUnitList::FindUnit(const CUnitStr&  other)
{
  for (int i=0;i<MAX_UNITS_IN_LIST;i++)
  {
	 if (m_pUnit[i]!=NULL) {
		 if (m_pUnit[i]->GetSlotNumb()==other.GetSlotNumb() &&
			 m_pUnit[i]->GetUnitId()==other.GetUnitId())
			  return i;
	 }
  }
  return NOT_FIND;
}


/////////////////////////////////////////////////////////////////////////////
CUnitStr*  CUnitList::GetFirstUnit()
{
   m_unit_ind=1;
   return m_pUnit[0];
}

/////////////////////////////////////////////////////////////////////////////
CUnitStr*  CUnitList::GetNextUnit()
{
   if (m_unit_ind>=m_numb_of_units) return NULL;
   return m_pUnit[m_unit_ind++];
}

/////////////////////////////////////////////////////////////////////////////

CUnitStr* CUnitList::operator[](WORD i)
{
	return  m_pUnit[i];
}



//////////////////////////////////////////////////////////////////////////////
// CClockUnitStr

CClockUnitStr::CClockUnitStr()
{
  m_wMaster=0;
}

/////////////////////////////////////////////////////////////////////////////
CClockUnitStr::~CClockUnitStr()
{
}

/////////////////////////////////////////////////////////////////////////////
void CClockUnitStr::Serialize(WORD format, ostream &m_ostr)
{
  // assuming format = OPERATOR_MCMS
  CUnitStr::Serialize(format, m_ostr);
  m_ostr <<  m_wMaster << "\n";
}
/////////////////////////////////////////////////////////////////////////////

void CClockUnitStr::Serialize(char* userAllocatedBuf)
{
  ostream ostr(userAllocatedBuf,MAX_PRIVATE_NAME_LEN);

  Serialize(OPERATOR_MCMS,ostr);
}

/////////////////////////////////////////////////////////////////////////////
void CClockUnitStr::DeSerialize(WORD format, istream &m_istr)
{
  // assuming format = OPERATOR_MCMS
  CUnitStr::DeSerialize(format, m_istr);
  m_istr >>  m_wMaster;
}

/////////////////////////////////////////////////////////////////////////////

void CClockUnitStr::DeSerialize(char* str)
{
  istream istr(str,MAX_PRIVATE_NAME_LEN);

  DeSerialize(OPERATOR_MCMS,istr);
}


/////////////////////////////////////////////////////////////////////////////
void CClockUnitStr::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CUnitStr::SerializeXml(pFatherNode);
	pFatherNode->AddChildNode("CLOCK_TYPE",m_wMaster,CLOCK_CONFIG_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  trans_card.xsd
int CClockUnitStr::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement*  pChild = NULL;
	GET_CHILD_NODE(pActionNode, "UNIT_POSITION", pChild);
	if( pChild ) {
		nStatus = CUnitStr::DeSerializeXml(pChild,pszError);
		if( nStatus )
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode,"CLOCK_TYPE",&m_wMaster,CLOCK_CONFIG_ENUM);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

void CClockUnitStr::DeSerializeSpaces(istream& istr)
{
	CUnitStr::DeSerializeSpaces(istr);
	char buff[20];
	istr.get(buff,19,' ');
	m_wMaster = atoi(buff);
}

void CClockUnitStr::DeSerializeSpaces(char* s)
{
	istream istr(s,MAX_PRIVATE_NAME_LEN);
	DeSerializeSpaces(istr);
}

void CClockUnitStr::SerializeSpaces(ostream& ostr)
{
	CUnitStr::SerializeSpaces(ostr);
	ostr << m_wMaster << " ";
}

void CClockUnitStr::SerializeSpaces(char* s)
{
	ostream  ostr(s,MAX_PRIVATE_NAME_LEN);
	SerializeSpaces(ostr);
}

void CClockUnitStr::SetMaster()
{
	m_wMaster = 0xffff;
}

BYTE CClockUnitStr::IsMaster()
{
	return (m_wMaster == 0xffff);
}

void CClockUnitStr::SetBackup()
{
	m_wMaster = 0;
}

BYTE CClockUnitStr::IsBackup()
{
	return (m_wMaster != 0xffff);
}


/////////////////////////////////////////////////////////////////////////////

void CCommCardDB::InformSnmpChange()
{

    if ( ::GetpSystemCfg()->IsSnmp() && pEntityMibDB && ::isValidPObjectPtr(pEntityMibDB) )
		pEntityMibDB->UpdatePhysicalTable();

}
/////////////////////////////////////////////////////////////////////////////
void CCommCardDB::DeSerialize(WORD format, FILE *infile,  int& status,DWORD apiNum)
{
  char *buf = new char[FIFTY_LINE_BUFFER_LEN];
  long numRead = fread(buf,1,FIFTY_LINE_BUFFER_LEN,infile);
  
  int fcloseReturn = fclose(infile);
  if (FCLOSE_SUCCESS != fcloseReturn)
  {
		perror("\nCCommCardDB::DeSerialize - fclose failed. "); // for printing the errno
  		TRACESTR(eLevelInfoNormal) << "\nCCommCardDB::DeSerialize - failed to close file."
  		                       << " fclose return value: " << fcloseReturn;
  } 

  istream*   pIstr;
  pIstr = new istream(buf);

  m_consist=0;
  DeSerialize(format, *pIstr,apiNum);
  PDELETE(pIstr);

  if (!m_consist)
	status = STATUS_OK;
  else
	status = STATUS_CARDS_FILE_CORRUPTED;

  PDELETEA(buf);
}

void CCommDynCard::InformSnmpStatusChange(WORD newState, CFaultCardDesc* pFault )
{
	if ( ::GetpSystemCfg()->IsSnmp() )
	{ 
		CFaultCardDesc* theFault = new CFaultCardDesc;
		
		if( pFault != NULL )
			*theFault = *pFault;
		

		CMgcAPI::CMgcAPIRc        Rc;
		CMgcAPI::MgcStatusParam   Param;
		
		memset(&Param,0,sizeof(Param));
		switch (m_state)
		{
		case CARD_STARTUP:
			Param.Status = CMgcAPI::StartUp;
			break;
		case CARD_NORMAL:
			Param.Status = CMgcAPI::Normal;
			break;
		case CARD_MINOR_ERROR:
			Param.Status = CMgcAPI::Minor;
			Param.StatusCode = theFault->GetErrorCode();
			memcpy(Param.StatusDescription, theFault->GetErrorCodeAsString(), sizeof(Param.StatusDescription));
			Param.StatusDescription[sizeof(Param.StatusDescription) - 1] ='\0';
			break;
		case CARD_MAJOR_ERROR:
			Param.Status = CMgcAPI::Major;
			Param.StatusCode = theFault->GetErrorCode();
			memcpy(Param.StatusDescription, theFault->GetErrorCodeAsString(), sizeof(Param.StatusDescription));
			Param.StatusDescription[sizeof(Param.StatusDescription) - 1] ='\0';
		}
		
		Rc = MgcAPI.ChangeCardStatus(m_slotNumber, Param);
		if (CMgcAPI::Ok != Rc)
			TRACESTR(eLevelInfoNormal | SNMP_TRACE) << "Error at:InformSnmpStatusChange: Can't change MGC Status table.";
		POBJDELETE(theFault);
	}
}

BYTE  CCommDynCard::GetIsT1casCard() const
{
	// check if one of the spans is configured to t1cas service return true
	// check if one of the spans is configured to ISDN service return false
	for (int i=0;i<(int)m_numb_of_units;i++)
	{
		const char *serviceName = m_pCardRsrc[i]->GetName();

		if (::GetpServProvList()->GetCurrentServiceProvider(serviceName) != NULL) 
		  	     return FALSE;

		if (::GetpT1casServList()->GetCurrentService(serviceName) != NULL) 
		  	     return TRUE;
	}

	// if non of the spans configured to T1cas or ISDN
	// use the default card type for netX cards from the system.cfg
	return (::GetpSystemCfg()->GetNet8DefaultType() == NET8_CARD_TYPE_T1CAS);
}

BYTE  CCommDynCard::GetNetXCardConfiguration() const
{
	// check if one of the spans is configured to t1cas service return 2
	// check if one of the spans is configured to ISDN service return 1
	for (int i=0;i<(int)m_numb_of_units;i++)
	{
		const char *serviceName = m_pCardRsrc[i]->GetName();

		if (::GetpServProvList()->GetCurrentServiceProvider(serviceName) != NULL) 
		  	     return NET8_CARD_TYPE_ISDN;

		if (::GetpT1casServList()->GetCurrentService(serviceName) != NULL) 
		  	     return NET8_CARD_TYPE_T1CAS;
	}
	return NET8_CARD_TYPE_UNKOWN; // null configuration
}
	
BYTE  CCommDynCard::GetIsContainService(const char* name) const
{

	for (int i=0;i<(int)m_numb_of_units;i++)
	{
		const char *serviceName = m_pCardRsrc[i]->GetName();

		if (strcmp(name,serviceName) == 0) 
		  	     return TRUE;
	}
	return FALSE;
}
*/




