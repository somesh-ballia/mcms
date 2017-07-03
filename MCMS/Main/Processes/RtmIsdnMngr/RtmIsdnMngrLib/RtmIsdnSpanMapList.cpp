
#include "RtmIsdnSpanMapList.h"
#include "RtmIsdnSpanMap.h"
#include "RtmIsdnMngrProcess.h"
#include "RtmIsdnServiceList.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "TraceStream.h"
#include "ObjString.h"
#include "HlogApi.h"



/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapList::CRtmIsdnSpanMapList()
{
	int curMaxNumOfRtmIsdnBoards = 0;
	int firstRtmIsdnBoardId = -1;
	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
	m_ind_spanMap		= 0;
    m_updateCounter		= 0;	
	m_bChanged			= FALSE;
	m_numb_of_spanMaps	= 0;
	m_maxNumOfSpanMaps = 0;

	// 14/12/06: always a full list of spans!
	// 20/12/06: a full, static list, numbered 1-12 for each board

	// calculate m_maxNumOfSpanMaps

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnSpanMapList::CRtmIsdnSpanMapList current product type = "<<curProductType;


    switch (curProductType)
    {
      case eProductTypeRMX2000:
      case eProductTypeSoftMCU:
      case eProductTypeGesher:
     
      case eProductTypeSoftMCUMfw:
      case eProductTypeEdgeAxis:
      case eProductTypeCallGeneratorSoftMCU:
    	   curMaxNumOfRtmIsdnBoards = MAX_NUM_OF_RTM_ISDN_BOARDS_RMX2000;
    	   m_maxNumOfSpanMaps = MAX_ISDN_SPAN_MAPS_IN_BOARD * curMaxNumOfRtmIsdnBoards;
    	   break;
      case eProductTypeRMX4000:

    	   curMaxNumOfRtmIsdnBoards = MAX_NUM_OF_RTM_ISDN_BOARDS_RMX4000;
    	   m_maxNumOfSpanMaps = MAX_ISDN_SPAN_MAPS_IN_BOARD * curMaxNumOfRtmIsdnBoards;
    	   break;
      case eProductTypeRMX1500:
    	   curMaxNumOfRtmIsdnBoards = MAX_NUM_OF_RTM_ISDN_BOARDS_RMX1500;
    	   firstRtmIsdnBoardId	= m_pProcess->Get1stRtmIsdnBoardId();
    	   /****************************************************************/
    	    /* VNGR-15891 07.07.10 fixed by Rachel Cohen                   */
    	    /* when RMX1500 max number of spans should be 4                */
    	    /***************************************************************/
    	   m_maxNumOfSpanMaps = MAX_ISDN_SPAN_MAPS_IN_BOARD_RMX1500 * curMaxNumOfRtmIsdnBoards;
    	   break;
      case eProductTypeNinja:
    	   curMaxNumOfRtmIsdnBoards = MAX_NUM_OF_RTM_ISDN_BOARDS_RMX1800;
    	   firstRtmIsdnBoardId	= m_pProcess->Get1stRtmIsdnBoardId();
    	   m_maxNumOfSpanMaps = MAX_ISDN_SPAN_MAPS_IN_BOARD_RMX1800 * curMaxNumOfRtmIsdnBoards;
    	   break;
      default:
    		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnSpanMapList::CRtmIsdnSpanMapList"
    			                       << "\nProduct type not valid FAIL";


    }


    if (MAX_ISDN_SPAN_MAPS_IN_LIST < m_maxNumOfSpanMaps) // illegal!
    {
    	m_maxNumOfSpanMaps = MAX_ISDN_SPAN_MAPS_IN_LIST;
    }

	for( int i=0 ; i < m_maxNumOfSpanMaps; i++ )
    {

		m_pSpanMap[i] = new CRtmIsdnSpanMap();
		m_numb_of_spanMaps++;

		if (curProductType == eProductTypeRMX1500)
		{
			m_pSpanMap[i]->SetBoardId(firstRtmIsdnBoardId);
			m_pSpanMap[i]->SetSpanId(i+1);
		}
		else
		{
		SetSpanMapBoardID(i);
		SetSpanMapSpanID(i);
		}
    }




	/*for( int i=0; i<(int)m_numb_of_spanMaps; i++ )
	    {
	        if ( m_pSpanMap[i] != NULL )
	        {

	        	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnSpanMapList::CRtmIsdnSpanMapList \nindex " << i << " \nboard id "<<m_pSpanMap[i]->GetBoardId()
	        	    			                       << "\nSpanId " << m_pSpanMap[i]->GetSpanId();



	        }
	    }*/



}



/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::SetSpanMapBoardID(const int i)
{
	int firstRtmIsdnBoardId		= m_pProcess->Get1stRtmIsdnBoardId(),
		secondRtmIsdnBoardId	= m_pProcess->Get2ndRtmIsdnBoardId(),
		thirdRtmIsdnBoardId		= m_pProcess->Get3rdRtmIsdnBoardId(),
		fourthRtmIsdnBoardId	= m_pProcess->Get4thRtmIsdnBoardId();


	if ( 0 == (i / MAX_ISDN_SPAN_MAPS_IN_BOARD) )
	{
		m_pSpanMap[i]->SetBoardId(firstRtmIsdnBoardId);

	}
	else if ( 1 == (i / MAX_ISDN_SPAN_MAPS_IN_BOARD) )
	{
		m_pSpanMap[i]->SetBoardId(secondRtmIsdnBoardId);
	}
	else if ( 2 == (i / MAX_ISDN_SPAN_MAPS_IN_BOARD) )
	{
		m_pSpanMap[i]->SetBoardId(thirdRtmIsdnBoardId);
	}
	else if ( 3 == (i / MAX_ISDN_SPAN_MAPS_IN_BOARD) )
	{
		m_pSpanMap[i]->SetBoardId(fourthRtmIsdnBoardId);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::SetSpanMapSpanID(const int i)
{
	WORD curSpanId = (i % MAX_ISDN_SPAN_MAPS_IN_BOARD) + 1; // so the range of IDs is 1-12
	m_pSpanMap[i]->SetSpanId(curSpanId);
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapList::CRtmIsdnSpanMapList( const CRtmIsdnSpanMapList &other )
:CSerializeObject(other)
{
	m_maxNumOfSpanMaps	= other.m_maxNumOfSpanMaps;
    m_numb_of_spanMaps	= other.m_numb_of_spanMaps;
    m_ind_spanMap		= other.m_ind_spanMap;
    
	m_updateCounter 	= other.m_updateCounter;
	m_bChanged			= other.m_bChanged;

    for( int i=0 ; i < m_maxNumOfSpanMaps; i++ )
    {
        if( NULL == other.m_pSpanMap[i] )
            m_pSpanMap[i] = NULL;
        else
            m_pSpanMap[i] = new CRtmIsdnSpanMap( *other.m_pSpanMap[i] );
    }
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapList&  CRtmIsdnSpanMapList::operator=( const CRtmIsdnSpanMapList& other )
{
	if(this == &other)
	{
		return *this;
	}
	
    m_numb_of_spanMaps	= other.m_numb_of_spanMaps;
    m_ind_spanMap		= other.m_ind_spanMap;

    m_updateCounter 	= other.m_updateCounter;
	m_bChanged			= other.m_bChanged;
	
	
    for( int i=0; i<m_maxNumOfSpanMaps; i++ )
    {
        POBJDELETE(m_pSpanMap[i]);
    }

    m_maxNumOfSpanMaps	= other.m_maxNumOfSpanMaps;
	
    for( int i=0 ; i < m_maxNumOfSpanMaps; i++ )
    {
    	if (NULL == other.m_pSpanMap[i])
    		m_pSpanMap[i] = NULL;
    	else
    		m_pSpanMap[i] = new CRtmIsdnSpanMap(*other.m_pSpanMap[i]);
    }
    
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapList::~CRtmIsdnSpanMapList()
{
    for( int i=0; i<m_maxNumOfSpanMaps; i++ )
    {
        POBJDELETE(m_pSpanMap[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pSpanMapListNode;	
 
	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("RTM_ISDN_SPAN_LIST");
		pSpanMapListNode = pFatherNode;
	}
	else
	{
		pSpanMapListNode = pFatherNode->AddChildNode("RTM_ISDN_SPAN_LIST");
	}
	
	DWORD bChanged = InsertUpdateCntChanged(pSpanMapListNode, UPDATE_CNT_BEGIN_END);
	if(FALSE == bChanged)
	{
		return;
	}
	
	for (int i=0; i<m_numb_of_spanMaps; i++)
	{
		m_pSpanMap[i]->SerializeXml(pSpanMapListNode);
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::SerializeXml(CXMLDOMElement* pFatherNode, DWORD objToken)const
{
	CXMLDOMElement* pSpanMapListNode = pFatherNode->AddChildNode("RTM_ISDN_SPAN_LIST");
	
	WORD bChanged = InsertUpdateCntChanged(pSpanMapListNode, objToken);
	if(TRUE == bChanged)
	{
		for (int i=0; i<m_numb_of_spanMaps; i++)
		{
			m_pSpanMap[i]->SerializeXml(pSpanMapListNode);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// schema file name:  obj_ip_srv_list.xsd
int CRtmIsdnSpanMapList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pSpanMapNode;
	m_bChanged=TRUE;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);	
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"CHANGED",&m_bChanged,_BOOL);	

	m_numb_of_spanMaps = 0;

	GET_FIRST_CHILD_NODE(pActionNode,"RTM_ISDN_SPAN",pSpanMapNode);
	while ( pSpanMapNode &&
			(m_numb_of_spanMaps < m_maxNumOfSpanMaps) &&
			(m_numb_of_spanMaps < MAX_ISDN_SPAN_MAPS_IN_LIST) )
	{
		CRtmIsdnSpanMap* pSpanMap = new CRtmIsdnSpanMap;
		nStatus = pSpanMap->DeSerializeXml(pSpanMapNode, pszError, "");

		if(nStatus != STATUS_OK)
		{
			POBJDELETE(pSpanMap);
			return nStatus;
		}

		POBJDELETE(m_pSpanMap[m_numb_of_spanMaps]);
		m_pSpanMap[m_numb_of_spanMaps] = pSpanMap;
		m_numb_of_spanMaps++;

		GET_NEXT_CHILD_NODE(pActionNode,"RTM_ISDN_SPAN",pSpanMapNode);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::ReadXmlFile( const char * file_name,
	                    eFailReadingFileActiveAlarmType activeAlarmType,
	                    eFailReadingFileOperationType operationType,
	                    int activeAlarmId)
{
	STATUS status = CSerializeObject::ReadXmlFile(file_name, activeAlarmType, operationType, activeAlarmId);
	InitParams();
	return status;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::InitParams()
{
	for(int i=0; i<m_maxNumOfSpanMaps; i++)
	{
		if (m_pSpanMap[i])
		{
			m_pSpanMap[i]->InitSpanStatuses();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMapList::GetMaxNumOfSpanMaps() const
{
	return m_maxNumOfSpanMaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD CRtmIsdnSpanMapList::GetSpanMapNumber() const
{
    return m_numb_of_spanMaps;
}

////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::SetSpanMapNumber( const WORD num )
{
    m_numb_of_spanMaps = num;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CRtmIsdnSpanMapList::GetChanged() const
{
  return m_bChanged;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CRtmIsdnSpanMapList::GetUpdateCounter() const
{
	return m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapList::SetUpdateCounter(DWORD cnt)
{
	m_updateCounter = cnt;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::GetUpdateSpanMapStat( const CRtmIsdnSpanMap& other )
{
	STATUS retStat = STATUS_OK;

	// ===== 1. check legality
	int nameExist = strcmp(other.GetServiceName(), "");
	bool isAttachedToService = other.GetIsAttachedToService();
	if ( (!nameExist && (true  == isAttachedToService)) ||	// no serviceName but span is 'attached'
	     ( nameExist && (false == isAttachedToService)) )	// serviceName exists but span is 'not attached'
	{
		retStat = STATUS_MISMATCH_SERVICE_NAME_AND_SPAN_ATTACHED;
	}
	
	else // 'other' is legal
	{
		// ===== 2. check if exists
		int ind = FindSpanMap( other );
		if (NOT_FIND == ind)
			retStat = STATUS_SPAN_MAP_NOT_EXISTS;
	}

	return retStat;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::UpdateSpanMap( const CRtmIsdnSpanMap& other, bool const isToCheckStat/*=true*/ )
{
    STATUS retStat = STATUS_OK;
	
    if (true == isToCheckStat)
        retStat = GetUpdateSpanMapStat(other);
    
    if (STATUS_OK == retStat)
    {
        int ind = FindSpanMap( other );
		
        if (NOT_FIND != ind) // must exist, as it was already checked at 'GetUpdateSpanMapStat'
        {
            PASSERTSTREAM_AND_RETURN_VALUE(ind >= MAX_ISDN_SPAN_MAPS_IN_LIST,
                "ind has invalid value " << ind, STATUS_FAIL);
            
            CRtmIsdnSpanMap* pNewSpanMap = new CRtmIsdnSpanMap(other);
        
            POBJDELETE(m_pSpanMap[ind]);
            m_pSpanMap[ind] = pNewSpanMap;
        
            IncreaseUpdateCounter();
            WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);
        }
    }

    return retStat;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::DetachSpanMap( const SPAN_DISABLE_S& other ,bool isSpanValid)
{
	//TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::DetachSpanMap"

			   //                    << "\nboardId " << other.boardId << " spanId "<< other.spanId << " isSpanValid " << (int)isSpanValid;

    int ind = FindSpanMap(other.boardId, other.spanId);
	
    if(ind == NOT_FIND )
        return STATUS_SPAN_MAP_NOT_EXISTS;

    PASSERTSTREAM_AND_RETURN_VALUE(ind >= MAX_ISDN_SPAN_MAPS_IN_LIST,
        "ind has invalid value " << ind, STATUS_FAIL);
    
    STATUS status = STATUS_OK;
    m_pSpanMap[ind]->DetachFromService();

    m_pSpanMap[ind]->SetIsSpanValid(isSpanValid);

    IncreaseUpdateCounter();
    WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);

    return status;
}



/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::SetIsSpanMapValid( const SPAN_DISABLE_S& other ,WORD boardId)
{
	//TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::SetIsSpanMapValid"

		//	                       << "\nboardId " << other.boardId << " spanId "<< other.spanId << " boardId " << (int)boardId;

    int ind = FindSpanMap(other.boardId, other.spanId);

    if(ind == NOT_FIND )
    {

    	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::SetIsSpanMapValid NOT FIND";


    	if (m_numb_of_spanMaps < m_maxNumOfSpanMaps)

    	{
    		CRtmIsdnSpanMap* pSpanMap = new CRtmIsdnSpanMap();

    		pSpanMap->SetBoardId(other.boardId);
    		pSpanMap->SetSpanId(other.spanId);

    		POBJDELETE(m_pSpanMap[m_numb_of_spanMaps]);
    		m_pSpanMap[m_numb_of_spanMaps] = pSpanMap;

    		m_numb_of_spanMaps++;

    		  IncreaseUpdateCounter();
    		    WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);

    	}

          return STATUS_SPAN_MAP_UPDATED;
    }



    return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::DetachSpanOfNonExistingService()
{
	CRtmIsdnServiceList *pServiceList = m_pProcess->GetServiceListOriginal();
	
	for (int i=0; i<(int)m_numb_of_spanMaps; i++)
	{
		CRtmIsdnSpanMap *pCurSpanMap = m_pSpanMap[i];
		if ( pCurSpanMap && (true == pCurSpanMap->GetIsAttachedToService()) )
		{
			int ind = pServiceList->FindService( pCurSpanMap->GetServiceName() );
			if (NOT_FIND == ind)
			{
				// Produce a Fault
				char mes[400];
				snprintf( mes, sizeof(mes), "BoardId %d, SpanId %d is attached to Service %s, which does not exist; span is detached",
				         pCurSpanMap->GetBoardId(), pCurSpanMap->GetSpanId(), pCurSpanMap->GetServiceName() ); 
				CHlogApi::ServiceAttachedToSpanNotExists(mes);

				// Detach
				pCurSpanMap->DetachFromService();
				pCurSpanMap->InitSpanStatuses();
			}
		}
	}
    
    IncreaseUpdateCounter();
	WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);

    return  STATUS_OK; 
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::DetachAllSpanMapsOfService(const char* serviceName)
{
	for (int i=0; i<(int)m_numb_of_spanMaps; i++)
	{
		CRtmIsdnSpanMap *pCurSpanMap = m_pSpanMap[i];

		if (pCurSpanMap)
		{
			if ( !strcmp(serviceName, pCurSpanMap->GetServiceName()) )
			{
				pCurSpanMap->DetachFromService();
				pCurSpanMap->InitSpanStatuses();
			}
		}
	}

    IncreaseUpdateCounter();
	WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);

    return  STATUS_OK; 
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMapList::GetNumOfAttachedSpansOnBoard(WORD boardId)
{
	int numOfAttached = 0;

	for (int i=0; i<(int)m_numb_of_spanMaps; i++)
	{
		CRtmIsdnSpanMap *pCurSpanMap = m_pSpanMap[i];

		if ( pCurSpanMap &&
			 (pCurSpanMap->GetBoardId() == boardId) &&
			 (true == pCurSpanMap->GetIsAttachedToService()) )
		{
			numOfAttached++;
		}
	}
    
    return  numOfAttached; 
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::DetachExceededNumOfConfiguredSpans(const int maxAllowedNumber)
{
	bool isExceeded = false;
	int configuredAccumulator_Board1 = 0,
	    configuredAccumulator_Board2 = 0,
	    configuredAccumulator_Board3 = 0,
	    configuredAccumulator_Board4 = 0;


	for (int i=0; i<(int)m_numb_of_spanMaps; i++)
	{
		CRtmIsdnSpanMap *pCurSpanMap = m_pSpanMap[i];
		if ( pCurSpanMap && (true == pCurSpanMap->GetIsAttachedToService()) )
		{
			isExceeded = DetachSpecSpanIfExceeded( pCurSpanMap,
						                           configuredAccumulator_Board1,
						                           configuredAccumulator_Board2,
						                           configuredAccumulator_Board3,
						                           configuredAccumulator_Board4,
						                           maxAllowedNumber );
		}
	} // end loop over spans


	if (true == isExceeded)
	{
	    IncreaseUpdateCounter();
		WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);

		// produce log & fault
		CMedString errStr = "Allowed number of configured ISDN spans has reached; additional spans were detached";
		CHlogApi::NumOfConfiguredIsdnSpansExceeded( errStr.GetString() );
		
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::DetachExceededNumOfConfiguredSpans"
		                       << "\n" << errStr.GetString()
		                       << "\n(the allowed number is " << maxAllowedNumber << ")";
	}
	
    return  STATUS_OK; 
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnSpanMapList::DetachSpecSpanIfExceeded( CRtmIsdnSpanMap *pCurSpanMap,
		                                            int &configuredAccumulator_Board1,
		                                            int &configuredAccumulator_Board2,
		                                            int &configuredAccumulator_Board3,
		                                            int &configuredAccumulator_Board4,
		                                            const int maxAllowedNumber )
{
	int firstRtmIsdnBoardId	= m_pProcess->Get1stRtmIsdnBoardId(),
		secondRtmIsdnBoardId	= m_pProcess->Get2ndRtmIsdnBoardId(),
		thirdRtmIsdnBoardId	= m_pProcess->Get3rdRtmIsdnBoardId(),
		fourthRtmIsdnBoardId	= m_pProcess->Get4thRtmIsdnBoardId();

	bool isExceeded = false;

	WORD boardId = pCurSpanMap->GetBoardId();
	if (firstRtmIsdnBoardId == boardId)
	{
		configuredAccumulator_Board1++;
	}
	else if (secondRtmIsdnBoardId == boardId)
	{
		configuredAccumulator_Board2++;
	}
	else if (thirdRtmIsdnBoardId == boardId)
	{
		configuredAccumulator_Board3++;
	}
	else if (fourthRtmIsdnBoardId == boardId)
	{
		configuredAccumulator_Board4++;
	}

	
	if ( ((firstRtmIsdnBoardId	== boardId) && (configuredAccumulator_Board1 > maxAllowedNumber)) ||
		 ((secondRtmIsdnBoardId	== boardId) && (configuredAccumulator_Board2 > maxAllowedNumber)) ||
		 ((thirdRtmIsdnBoardId	== boardId) && (configuredAccumulator_Board3 > maxAllowedNumber)) ||
		 ((fourthRtmIsdnBoardId	== boardId) && (configuredAccumulator_Board4 > maxAllowedNumber)) )
	{
		isExceeded = true;
		pCurSpanMap->DetachFromService();
		pCurSpanMap->InitSpanStatuses();
	}
	
	return isExceeded;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CRtmIsdnSpanMapList::IsAnySpanAttachedToService()
{
	BOOL isAattached = NO;

	for (int i=0; i<(int)m_numb_of_spanMaps; i++)
	{
		CRtmIsdnSpanMap *pCurSpanMap = m_pSpanMap[i];

		if ( pCurSpanMap && (true == pCurSpanMap->GetIsAttachedToService()) )
		{
			isAattached = YES;
			break;
		}
	}
    
    return  isAattached; 
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnSpanMapList::IsAnySpanAttachedOnBoard(WORD boardId)
{
	bool isAttached = false;

	if ( 0 < GetNumOfAttachedSpansOnBoard(boardId) )
		isAttached = true;
	
	return isAttached;
}

/////////////////////////////////////////////////////////////////////////////
/*
STATUS CRtmIsdnSpanMapList::AddSpanMap(const CRtmIsdnSpanMap &pSpanMap)
{
	if(MAX_ISDN_SPAN_MAPS_IN_LIST <= m_numb_of_spanMaps)
	{
		TRACESTR(eLevelInfoNormal) << "\nAddSpanMap::HandleSetAddSpan - Too many span maps";
        return STATUS_NUMBER_OF_ISDN_SPAN_MAPS_EXCEEDED;
	}

    if( FindSpanMap(pSpanMap) != NOT_FIND )
    {
        return STATUS_SPAN_MAP_ALREADY_EXISTS;
    }
       
    m_pSpanMap[m_numb_of_spanMaps] = new CRtmIsdnSpanMap(pSpanMap);
    m_numb_of_spanMaps++;
  
	IncreaseUpdateCounter();
	WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);
	
    return STATUS_OK;    
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::CancelSpanMap( const WORD boardId, const WORD spanId )
{
    int ind = FindSpanMap( boardId, spanId );
    if(ind == NOT_FIND )
        return STATUS_SPAN_MAP_NOT_EXISTS;
    
    POBJDELETE(m_pSpanMap[ind]);
    
	int i=0, j=0;
    for( i=0; i<(int)m_numb_of_spanMaps; i++ )
    {
        if(m_pSpanMap[i] == NULL )
            break;
    }
    for( int j=i; j<(int)m_numb_of_spanMaps-1; j++ )
    {
        m_pSpanMap[j] = m_pSpanMap[j+1] ;
    }
    m_pSpanMap[m_numb_of_spanMaps-1] = NULL;
    m_numb_of_spanMaps--;
    
    IncreaseUpdateCounter();
	WriteXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH);

    return  STATUS_OK; 
}
*/

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnSpanMapList::UpdateSpanStatus(const RTM_ISDN_SPAN_STATUS_MCMS_S &other)
{
    int ind = FindSpanMap(other.boardId, other.spanId);
    
    if(ind == NOT_FIND )
        return STATUS_SPAN_MAP_NOT_EXISTS;

    PASSERTSTREAM_AND_RETURN_VALUE(ind >= MAX_ISDN_SPAN_MAPS_IN_LIST,
        "ind has invalid value " << ind, STATUS_FAIL);
    
    m_pSpanMap[ind]->SetAlarm( (eSpanAlarmType)other.alarm_status );
    m_pSpanMap[ind]->SetDChannelState( (eDChannelStateType)other.d_chnl_status );
    m_pSpanMap[ind]->SetClocking( (eClockingType)other.clocking_status );
  
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMapList::FindSpanMap( const WORD boardId, const WORD spanId )const
{
	//TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::FindSpanMap"
	 //       			                       << "\nboardId" << boardId
	  //      			                       << "\nspanId " << spanId ;
	for( int i=0; i<(int)m_numb_of_spanMaps; i++ )
    {
        if ( m_pSpanMap[i] != NULL )
        {

        	//TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnServiceList::FindSpanMap in loop"
        			     //                  <<"\nindex " << i
        			             //          << "\nboardId" << m_pSpanMap[i]->GetBoardId()
        			              //         << "\nspanId " << m_pSpanMap[i]->GetSpanId() ;


            if ( (m_pSpanMap[i]->GetBoardId() == boardId) &&
                 (m_pSpanMap[i]->GetSpanId()  == spanId) )
            {
            	return i;
            }
        }
    }

    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMapList::FindSpanMap(const CRtmIsdnSpanMap& other)const
{
    for( int i=0; i<(int)m_numb_of_spanMaps; i++ )
    {
        if ( m_pSpanMap[i] != NULL )
        {
            if ( (m_pSpanMap[i]->GetBoardId() == other.GetBoardId()) &&
                 (m_pSpanMap[i]->GetSpanId()  == other.GetSpanId()) )
            {
            	return i;
            }
        }
    }

    return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMap*  CRtmIsdnSpanMapList::GetSpanMap( const WORD boardId, const WORD spanId )
{
    CRtmIsdnSpanMap *pSpanMap = NULL;

    int index = FindSpanMap(boardId, spanId);
    
    if(NOT_FIND != index)
    {
        PASSERTSTREAM_AND_RETURN_VALUE(index >= MAX_ISDN_SPAN_MAPS_IN_LIST,
        "ind has invalid value " << index, NULL);
            
        pSpanMap = m_pSpanMap[index];
    }
    
    return pSpanMap;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMap*  CRtmIsdnSpanMapList::GetFirstSpanMap()
{
    m_ind_spanMap = 1;
    return m_pSpanMap[0];
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMap*  CRtmIsdnSpanMapList::GetNextSpanMap()
{
	CRtmIsdnSpanMap *pSpanMap = NULL;

    if (m_ind_spanMap < m_numb_of_spanMaps)
    {
    	pSpanMap = m_pSpanMap[m_ind_spanMap];
    	m_ind_spanMap++;
    }

    return pSpanMap;
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnSpanMapList::IsThereAnySpanConfigured(const WORD boardId) const
{
	bool isSpanConfigured = false;
	
	for( int i=0; i<(int)m_numb_of_spanMaps; i++ )
    {
        if ( (m_pSpanMap[i] != NULL) &&
        	 (m_pSpanMap[i]->GetBoardId() == boardId) )
        {
            if ( true == m_pSpanMap[i]->GetIsAttachedToService() )
            {
            	isSpanConfigured = true;
            	break;
            }
        }
    } // end loop over spans
	
	return isSpanConfigured;
}

/////////////////////////////////////////////////////////////////////////////
eCardsClockSourceStateType CRtmIsdnSpanMapList::GetCardsClockSourceState(const WORD boardId) const
{
	eCardsClockSourceStateType clockSourceState = eCardsClockSourceState_ok;
	int numOfClocks = 0;

	for( int i=0; i<(int)m_numb_of_spanMaps; i++ )
    {
        if ( (m_pSpanMap[i] != NULL) &&
        	 (m_pSpanMap[i]->GetBoardId() == boardId) )
        {
            if ( (eClockingTypePrimary == m_pSpanMap[i]->GetClocking()) ||
                 (eClockingTypeBackup  == m_pSpanMap[i]->GetClocking()) )
            {
            	numOfClocks++;
            }
        }
    } // end loop over spans

	
    if (0 == numOfClocks)
    	clockSourceState = eCardsClockSourceState_noClock;
    else if (1 == numOfClocks)
    	clockSourceState = eCardsClockSourceState_singleClock;
    else
    	clockSourceState = eCardsClockSourceState_ok;
   
    
    return clockSourceState;
}
