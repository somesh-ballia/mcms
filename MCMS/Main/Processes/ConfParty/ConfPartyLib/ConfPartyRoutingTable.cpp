	///3 Options for lookup:
	// 1.Party Id + Connection Id (When Connection ID != 0)
	// 2.Party Id + LRT (When Both Valid)
	// 4.Conf Id - (When only Conf Id Valid) - we look for any LRT that is a confID rsrc (audio_enc, audio_dec, video_enc, video_dec)

#include "ConfPartyRoutingTable.h"
#include "Trace.h"
#include "MplMcmsStructs.h"
#include "StatusesGeneral.h"
#include "ObjString.h"
#include "TaskApi.h"
#include "HostCommonDefinitions.h"

extern char* LogicalResourceTypeToString(APIU32 logicalResourceType);

static int gCounterIterationLessThanForTest = 0;
int GetCounterIterationLessThanForTest(); 
void SetCounterIterationLessThanForTest(int num);

/////////////////////////////////////////////////////////////////////////////
/// Global Functions
int GetCounterIterationLessThanForTest() 
{
  return gCounterIterationLessThanForTest;
}
/////////////////////////////////////////////////////////////////////////////
void SetCounterIterationLessThanForTest(int num) 
{
  gCounterIterationLessThanForTest = num;
}

/////////////////////////////////////////////////////////////////////////////
// constructor
CPartyRsrcRoutingTblKey::CPartyRsrcRoutingTblKey(ConnectionID ConnId,PartyRsrcID PartyId,eLogicalResourceTypes LRT)
{
	m_PartyRsrcID	= PartyId;
	m_pRsrcDesc		= new CRsrcDesc(ConnId, LRT);
}

/////////////////////////////////////////////////////////////////////////////
CPartyRsrcRoutingTblKey::~CPartyRsrcRoutingTblKey()     // destructor
{	
	POBJDELETE(m_pRsrcDesc);
}
/////////////////////////////////////////////////////////////////////////////
CPartyRsrcRoutingTblKey::CPartyRsrcRoutingTblKey(const CPartyRsrcRoutingTblKey& other)
:CPObject(other)
{
	m_PartyRsrcID = other.m_PartyRsrcID;
	m_pRsrcDesc = NULL;
	if (other.GetRsrcDesc())
		m_pRsrcDesc = new CRsrcDesc(*(other.GetRsrcDesc()));
}
/////////////////////////////////////////////////////////////////////////////
CPartyRsrcRoutingTblKey::CPartyRsrcRoutingTblKey(const CRsrcParams& other)
:CPObject(other)
{
	m_PartyRsrcID = other.GetPartyRsrcId();
	m_pRsrcDesc = NULL;
	if (other.GetRsrcDesc())
	{
		m_pRsrcDesc = new CRsrcDesc(*(other.GetRsrcDesc()));
	}
}
/////////////////////////////////////////////////////////////////////////////
const CPartyRsrcRoutingTblKey& CPartyRsrcRoutingTblKey::operator=(const CPartyRsrcRoutingTblKey& other)
{
	if ( &other == this ) return *this;

	m_PartyRsrcID	= other.m_PartyRsrcID;
	POBJDELETE(m_pRsrcDesc);
	if(other.GetRsrcDesc())
	{
		m_pRsrcDesc = new CRsrcDesc(*(other.GetRsrcDesc()));
	}
	
	return *this;	
}
/////////////////////////////////////////////////////////////////////////////
bool operator<(const CPartyRsrcRoutingTblKey& k1 ,const CPartyRsrcRoutingTblKey& k2)
{	
	gCounterIterationLessThanForTest++;//tdd test only

	if(k1.GetPartyRsrcId()<k2.GetPartyRsrcId())
		return true;
	else if(k1.GetPartyRsrcId()>k2.GetPartyRsrcId())
		return false;
	else ////k1.m_PartyRsrcID == k2.m_PartyRsrcID
	{
		return (k1.GetConnectionId()<k2.GetConnectionId());	
	}
}
///////////////////////////////////////////////////////////////////////////////
void CPartyRsrcRoutingTblKey::Print() const
{
	CMedString str;
	str << "m_PartyRsrcID = " <<m_PartyRsrcID;
	if(m_pRsrcDesc)
	{
		str << " ; lrt = " <<m_pRsrcDesc->GetLogicalRsrcType();
		str << " ; connectionId = " <<m_pRsrcDesc->GetConnectionId();
	}
	
	PTRACE2(eLevelInfoNormal, "CPartyRsrcRoutingTblKey::Print\n",str.GetString());  
	
}
///////////////////////////////////////////////////////////////////////////////
DWORD CPartyRsrcRoutingTblKey::GetPartyRsrcId() const
{
	return m_PartyRsrcID;
}
///////////////////////////////////////////////////////////////////////////////
DWORD CPartyRsrcRoutingTblKey::GetConnectionId() const
{
	if(m_pRsrcDesc)
		return m_pRsrcDesc->GetConnectionId();
	else
	{
		PASSERT(101);
		return 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
eLogicalResourceTypes CPartyRsrcRoutingTblKey::GetLogicalRsrcType() const
{
	if(m_pRsrcDesc)
		return m_pRsrcDesc->GetLogicalRsrcType();
	else
	{
		PASSERT(101);
		return eLogical_res_none;
	}
}
/////////////////////////////////////////////////////////////////////////////
CRsrcDesc* CPartyRsrcRoutingTblKey::GetRsrcDesc() const
{
	return m_pRsrcDesc;
}
///////////////////////////////////////////////////////////////////////////////
void CPartyRsrcRoutingTblKey::SetPartyRsrcId(PartyRsrcID partyID) 
{
	m_PartyRsrcID = partyID;
}
///////////////////////////////////////////////////////////////////////////////
void CPartyRsrcRoutingTblKey::SetConnectionId(ConnectionID conID) 
{
	if(m_pRsrcDesc)
		m_pRsrcDesc->SetConnectionId(conID);
	else
	{
		PASSERT(101);
	}
}
///////////////////////////////////////////////////////////////////////////////
void CPartyRsrcRoutingTblKey::SetLogicalRsrcType(eLogicalResourceTypes LRT) 
{
	if(m_pRsrcDesc)
		m_pRsrcDesc->SetLogicalRsrcType(LRT);
	else
	{
		PASSERT(101);
	}
}
///////////////////////////////////////////////////////////////////////////////
void CPartyRsrcRoutingTblKey::SetRsrcDesc(CRsrcDesc Rsrc) 
{
	if(m_pRsrcDesc)
		*m_pRsrcDesc = Rsrc;
	else
	{
		PASSERT(101);
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcRoutingTblKey::Serialize(WORD format,CSegment &seg)
{                         
	switch ( format )  
	{    
		
	case NATIVE :
		{     
			
			seg << m_PartyRsrcID;
			m_pRsrcDesc->Serialize(format, seg);	
		}
		
	default :
		{
			break;   
		}
	}       
}

/////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcRoutingTblKey::DeSerialize(WORD format,CSegment &seg)
{                         
	switch ( format ) 
	{    
		
	case NATIVE :
		{    
			seg >> m_PartyRsrcID;
				m_pRsrcDesc->DeSerialize(format, seg);
		}
	default :
		{
			break;   
		}		
	}       
}

/////////////////////////////////////////////////////////////////////////////
///////////////     Class CConfPartyRoutingTable    /////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CConfPartyRoutingTable::CConfPartyRoutingTable()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CConfPartyRoutingTable::~CConfPartyRoutingTable()     // destructor
{
	for( PART_RSRC_ROUTING_TBL::iterator rtIt = m_RoutingTbl.begin();
	 rtIt !=  m_RoutingTbl.end() ; ++rtIt)
	 	POBJDELETE(rtIt->second);
	
	for( CONF_RSRC_ROUTING_TBL::iterator confIt = m_confRoutingTbl.begin();
	 confIt !=  m_confRoutingTbl.end() ; ++confIt)
	 	POBJDELETE(confIt->second);
	 
	m_RoutingTbl.clear()    ;
	m_confRoutingTbl.clear();
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void*  CConfPartyRoutingTable::GetRsrcMngrPtrByConfId(ConfRsrcID confID)
{
	CONF_RSRC_ROUTING_TBL::iterator itr;
	itr = FindInConfRsrcRoutingTbl(confID);
		
	if(itr != m_confRoutingTbl.end())
	{	
		return ((*itr).second);
	}
	else
	{ 
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void*  CConfPartyRoutingTable::GetRsrcMngrPtrByPartyId(PartyRsrcID partyID)
{	
	PART_RSRC_ROUTING_TBL::iterator itr;
	itr = FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(partyID, DUMMY_CONNECTION_ID);
		
	if(itr != m_RoutingTbl.end())
	{	
		return ((*itr).second);
	}
	else
	{ 
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void*  CConfPartyRoutingTable::GetRsrcMngrPtrByPartyIdAndConnectionId(PartyRsrcID partyID, ConnectionID connectionID)
{
	PART_RSRC_ROUTING_TBL::iterator itr;
	itr = FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(partyID, connectionID);
		
	if(itr != m_RoutingTbl.end())
	{
		return ((*itr).second);
	}
	else
	{
		return NULL;
	}

}

/////////////////////////////////////////////////////////////////////////////
eLogicalResourceTypes  CConfPartyRoutingTable::GetLRTByPartyIdAndConnectionId(PartyRsrcID partyID, ConnectionID connectionID)
{
	PART_RSRC_ROUTING_TBL::iterator itr;
	CPartyRsrcRoutingTblKey key;
	itr = FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(partyID, connectionID);

	if(itr != m_RoutingTbl.end())
	{
		key=((*itr).first);
		return key.GetLogicalRsrcType();
	}
	return eLogical_res_none;

}



/////////////////////////////////////////////////////////////////////////////
PART_RSRC_ROUTING_TBL::iterator  CConfPartyRoutingTable::FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(PartyRsrcID partyID, ConnectionID connectionID, eLogicalResourceTypes LRT)
{
	CPartyRsrcRoutingTblKey RoutingTblKey(connectionID, partyID, LRT);
	
	PART_RSRC_ROUTING_TBL::iterator itr;
	
	return(m_RoutingTbl.find(RoutingTblKey));
}

/////////////////////////////////////////////////////////////////////////////
PART_RSRC_ROUTING_TBL::iterator  CConfPartyRoutingTable::FindInPartyRsrcRoutingTblByPartyIdAndLRT(PartyRsrcID partyID, eLogicalResourceTypes LRT)
{
	PART_RSRC_ROUTING_TBL::iterator itr;

	CPartyRsrcRoutingTblKey RoutingTblKey(0, partyID, LRT);	
	itr = m_RoutingTbl.lower_bound(RoutingTblKey);

	for( ;itr!= m_RoutingTbl.end(); ++itr )
	{	
		if(RoutingTblKey.GetPartyRsrcId()!= (*itr).first.GetPartyRsrcId())//check out of bounds
		{
			return(m_RoutingTbl.end());
		}
			
		if(RoutingTblKey.GetLogicalRsrcType() == (*itr).first.GetLogicalRsrcType())
		{
			return (itr);
		}			
	}
	return(m_RoutingTbl.end());
}

/////////////////////////////////////////////////////////////////////////////
CONF_RSRC_ROUTING_TBL::iterator  CConfPartyRoutingTable::FindInConfRsrcRoutingTbl(ConfRsrcID confID)
{
	CONF_RSRC_ROUTING_TBL::iterator itr;

	return(m_confRoutingTbl.find(confID));
}

/////////////////////////////////////////////////////////////////////////////
int  CConfPartyRoutingTable::AddPartyRsrcDesc(CPartyRsrcRoutingTblKey PartyRoutingTblKey)  
{	
	PTRACE(eLevelInfoNormal, "AddPartyRsrcDesc");

	m_RoutingTbl.insert(PART_RSRC_ROUTING_TBL::value_type(PartyRoutingTblKey, NULL));
	PartyRoutingTblKey.Print();
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int  CConfPartyRoutingTable::UpdatePartyRsrcIdInRoutingTbl(CPartyRsrcRoutingTblKey& key, PartyRsrcID newPartyID)
{
	PTRACE(eLevelInfoNormal,"CConfPartyRoutingTable::UpdatePartyRsrcIdInRoutingTbl Before:");
	DumpTable();
	key.Print();


//	PTRACE(eLevelInfoNormal,"CConfPartyRoutingTable::UpdatePartyRsrcIdInRoutingTbl Before:");
//	DumpTable();
	PART_RSRC_ROUTING_TBL::iterator itr = FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(key.GetPartyRsrcId(), key.GetConnectionId(), key.GetLogicalRsrcType());
	int status;
	if(itr != m_RoutingTbl.end())
	{
		CPartyRsrcRoutingTblKey newKey = itr->first;
		newKey.SetPartyRsrcId(newPartyID);
		CTaskApi * pTaskApi = itr->second;
		m_RoutingTbl.erase(itr);
		m_RoutingTbl.insert(PART_RSRC_ROUTING_TBL::value_type(newKey, pTaskApi));
		
		
		
		status = STATUS_OK;
	}
	else
	{
		status = STATUS_FAIL;
		CMedString cstr;
		cstr << "CConfPartyRoutingTable::UpdatePartyRsrcIdInRoutingTbl Failed! (detailes: CPartyRsrcRoutingTblKey: PartyRsrcId = "
			 << key.GetPartyRsrcId() << "LogicalRsrcType: " << ::LogicalResourceTypeToString(key.GetLogicalRsrcType()) << "  new party id: " << newPartyID; 
		
		PTRACE(eLevelInfoNormal,cstr.GetString());
		DumpTable();
		key.Print();
	}

//	PTRACE(eLevelInfoNormal,"CConfPartyRoutingTable::UpdatePartyRsrcIdInRoutingTbl After:");
//	DumpTable();
	
	return status;
		
}
/////////////////////////////////////////////////////////////////////////////
int  CConfPartyRoutingTable::AddStateMachinePointerToRoutingTbl(CRsrcParams PartyRoutingTblKey,
                                                                CTaskApi * pOtherTaskApi)  
{ 

	PART_RSRC_ROUTING_TBL::iterator itr = FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(PartyRoutingTblKey.GetPartyRsrcId(), PartyRoutingTblKey.GetConnectionId());
	
	if(itr != m_RoutingTbl.end())
	{	
		CTaskApi * pOldTaskApi = itr->second;
		POBJDELETE(pOldTaskApi);
		(*itr).second = new CTaskApi(*pOtherTaskApi);
		return STATUS_OK;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "AddStateMachinePointerToRoutingTbl GetPartyRsrcId=", PartyRoutingTblKey.GetPartyRsrcId());
		PTRACE2INT(eLevelInfoNormal, "AddStateMachinePointerToRoutingTbl GetConnectionId=", PartyRoutingTblKey.GetConnectionId());
		return STATUS_FAIL;
	}
}

/////////////////////////////////////////////////////////////////////////////
CRsrcDesc*  CConfPartyRoutingTable::AddStateMachinePointerToRoutingTbl(PartyRsrcID partyID, eLogicalResourceTypes LRT, CTaskApi * pOtherTaskApi)  
{
	PART_RSRC_ROUTING_TBL::iterator itr = FindInPartyRsrcRoutingTblByPartyIdAndLRT(partyID, LRT);
	
	if(itr != m_RoutingTbl.end())
	{	
		CTaskApi * pOldTaskApi = itr->second;
		POBJDELETE(pOldTaskApi);
        (*itr).second = new CTaskApi(*pOtherTaskApi);
		return (((*itr).first).GetRsrcDesc());
	}
	else
	{
		return NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////
int  CConfPartyRoutingTable::RemoveStateMachinePointerFromRoutingTbl(CRsrcParams RoutingTblKey)  
{
	PART_RSRC_ROUTING_TBL::iterator itr = m_RoutingTbl.find(CPartyRsrcRoutingTblKey(RoutingTblKey));
	
	if(itr != m_RoutingTbl.end())
	{	
		CTaskApi * pTaskApi = itr->second;
		POBJDELETE(pTaskApi);
		(*itr).second = NULL; 
		return STATUS_OK;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "m_RoutingTbl.find fails");
		return STATUS_FAIL;
	}
}

/////////////////////////////////////////////////////////////////////////////
CRsrcDesc*  CConfPartyRoutingTable::GetPartyRsrcDesc(PartyRsrcID partyID, eLogicalResourceTypes LRT)
{
	PART_RSRC_ROUTING_TBL::iterator itr = FindInPartyRsrcRoutingTblByPartyIdAndLRT(partyID, LRT);
	
	if(itr != m_RoutingTbl.end())
	{	
		return (((*itr).first).GetRsrcDesc());
	}
	else
	{
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
VECTOR_OF_RSRC_DESC_POINTERS*  CConfPartyRoutingTable::GetAllPartyRsrcs(PartyRsrcID partyID)
{
	VECTOR_OF_RSRC_DESC_POINTERS* pOutPutVector = new VECTOR_OF_RSRC_DESC_POINTERS;
		
	PART_RSRC_ROUTING_TBL::iterator itr;

	CPartyRsrcRoutingTblKey RoutingTblKey(0, partyID, eLogical_res_none);	
	itr = m_RoutingTbl.lower_bound(RoutingTblKey);
	if (itr == m_RoutingTbl.end())
	{
		if (partyID)
			DBGPASSERT(partyID);
		else
			DBGPASSERT(1);
		return pOutPutVector;
	}
	
	for( ;itr!= m_RoutingTbl.end(); ++itr )
	{	
		if(RoutingTblKey.GetPartyRsrcId()!= (*itr).first.GetPartyRsrcId())//check out of bounds
		{
			return pOutPutVector;
		}
		else
		{
			pOutPutVector->push_back(((*itr).first).GetRsrcDesc());
		}			
	}
	return pOutPutVector;	
}
/////////////////////////////////////////////////////////////////////////////
void  CConfPartyRoutingTable::GetAllRsrcsFromLogicalType(PART_RSRC_ROUTING_TBL& rTbl, eLogicalResourceTypes LRT)
{
	PART_RSRC_ROUTING_TBL::iterator itr;

	for(itr= m_RoutingTbl.begin() ;itr!= m_RoutingTbl.end(); ++itr )
	{
		if(LRT == (*itr).first.GetLogicalRsrcType())
		{
			rTbl.insert(PART_RSRC_ROUTING_TBL::value_type(itr->first, itr->second));
		}
	}


}
/////////////////////////////////////////////////////////////////////////////
void  CConfPartyRoutingTable::RemovePartyRsrc(CRsrcParams RoutingTblKey)
{
	PART_RSRC_ROUTING_TBL::iterator itr = m_RoutingTbl.find(CPartyRsrcRoutingTblKey(RoutingTblKey));
		
	if (itr != m_RoutingTbl.end())
	{
		CTaskApi * pTaskApi = itr->second;
		POBJDELETE(pTaskApi);
		(*itr).second = NULL; 
		m_RoutingTbl.erase(itr);
	}
	else
	{
		CMedString cstr;
		cstr << "CConfPartyRoutingTable::RemovePartyRsrc can't find rsrc in table "
			 <<	"(connection id: " << RoutingTblKey.GetConnectionId() << ","
			 << " party rsrc id:"  << RoutingTblKey.GetPartyRsrcId()  << ","
			 << " logical resource type: " << ::LogicalResourceTypeToString(RoutingTblKey.GetLogicalRsrcType());
		
		PTRACE(eLevelInfoNormal,cstr.GetString());
		DBGPASSERT(1);
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CConfPartyRoutingTable::RemoveAllPartyRsrcs(PartyRsrcID partyID)
{
	PART_RSRC_ROUTING_TBL::iterator itr;
	PART_RSRC_ROUTING_TBL::iterator removing_itr;

	CPartyRsrcRoutingTblKey RoutingTblKey(0, partyID, eLogical_res_none);	
	
	itr = m_RoutingTbl.lower_bound(RoutingTblKey);
	if (itr == m_RoutingTbl.end())
	{
		if (partyID)
			DBGPASSERT_AND_RETURN(partyID);
		else
			DBGPASSERT_AND_RETURN(1);
	}

	while ( itr!= m_RoutingTbl.end() )
	{	
		if(RoutingTblKey.GetPartyRsrcId()!= (*itr).first.GetPartyRsrcId())//check out of bounds
		{
			return;
		}
		else
		{
			CTaskApi * pTaskApi = itr->second;
			POBJDELETE(pTaskApi);
			(*itr).second = NULL; 
			removing_itr = itr;
			++itr;
			m_RoutingTbl.erase(removing_itr);
		}			
	}
}
/////////////////////////////////////////////////////////////////////////////
int  CConfPartyRoutingTable::AddConfToRoutingTbl(ConfRsrcID ConfRsrcId,  CTaskApi * pOtherTaskApi)
{
	CTaskApi * pTaskApi = new CTaskApi(*pOtherTaskApi);
	m_confRoutingTbl.insert(CONF_RSRC_ROUTING_TBL::value_type(ConfRsrcId, pTaskApi));
	
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int  CConfPartyRoutingTable::RemoveConfFromRoutingTbl(ConfRsrcID ConfRsrcId)
{
	CONF_RSRC_ROUTING_TBL::iterator itr = m_confRoutingTbl.find(ConfRsrcId);
	
	if ( itr == m_confRoutingTbl.end())
		return STATUS_FAIL;
		
	CTaskApi * pTaskApi = itr->second;
	POBJDELETE(pTaskApi);
	(*itr).second = NULL;
	
	m_confRoutingTbl.erase(itr);
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void  CConfPartyRoutingTable::DumpTable()
{
	PART_RSRC_ROUTING_TBL::iterator itr;
	DWORD partyId, connectionId;
	CTaskApi * pTaskApi;
	CManDefinedString cstr(m_RoutingTbl.size() * 100);
	cstr << "\t party\t connection id \t\t\t LRT\t\t state machine \n"; 
	for (itr = m_RoutingTbl.begin();itr != m_RoutingTbl.end(); ++itr)
	{
		partyId = itr->first.GetPartyRsrcId();
		connectionId = itr->first.GetConnectionId();
		cstr << "\t";
		if (partyId < 100)
		{
			cstr << "\t";
			cstr << partyId;
			cstr << "\t";
		}
		else
		{
			cstr << partyId;
		}
		cstr << "\t";
		if (connectionId < 100)
		{
			cstr << "\t";
			cstr << connectionId;
			cstr << "\t";
		}
		else
		{
			cstr << connectionId;
		}
		
		 cstr << "\t" << ::LogicalResourceTypeToString(itr->first.GetLogicalRsrcType());
		 pTaskApi= itr->second;
		 cstr << "\t" << (DWORD)pTaskApi;
		 cstr << "\n";
		
		
	}
	if(cstr.GetString())
		PTRACE(eLevelInfoNormal,cstr.GetString());
	else
		PASSERTMSG(1, "GetString() return NULL"); 
}

VECTOR_OF_RSRC_DESC_POINTERS*  CConfPartyRoutingTable::GetAllPartyIdLogicalTypeRsrcs(PartyRsrcID partyID, eLogicalResourceTypes LRT)
{
	PTRACE(eLevelDebug, "CConfPartyRoutingTable::GetAllPartyIdLogicalTypeRsrcs " );

	VECTOR_OF_RSRC_DESC_POINTERS* pOutPutVector = new VECTOR_OF_RSRC_DESC_POINTERS;


	PART_RSRC_ROUTING_TBL::iterator itr;

	CPartyRsrcRoutingTblKey RoutingTblKey(0, partyID, LRT);
	itr = m_RoutingTbl.lower_bound(RoutingTblKey);

	for( ;itr!= m_RoutingTbl.end(); ++itr )
	{
		if (RoutingTblKey.GetPartyRsrcId()!= (*itr).first.GetPartyRsrcId())    			// check out of bounds (PartyID)
			return pOutPutVector;

		if(RoutingTblKey.GetLogicalRsrcType() == (*itr).first.GetLogicalRsrcType())     // check out of bounds (LogicalRsrc)
		{
			pOutPutVector->push_back(((*itr).first).GetRsrcDesc());
		}
	}
	return(pOutPutVector);


	/*
	VECTOR_OF_RSRC_DESC_POINTERS* pOutPutVector = new VECTOR_OF_RSRC_DESC_POINTERS;

	PART_RSRC_ROUTING_TBL::iterator myMixItr;
	std::pair <PART_RSRC_ROUTING_TBL::iterator, PART_RSRC_ROUTING_TBL::iterator> itrRange;

	CPartyRsrcRoutingTblKey RoutingTblKey(0, partyID, LRT);
	itrRange = m_RoutingTbl.equal_range(RoutingTblKey);
	if (itrRange.first == m_RoutingTbl.end())
	{
		PTRACE(eLogLevelDEBUG, "CConfPartyRoutingTable::GetAllPartyIdLogicalTypeRsrcs - KeyInRsrcTableNotFound 2" );
		if (partyID)
			DBGPASSERT(partyID);
		else
			DBGPASSERT(1);
		return pOutPutVector;
	}

	myMixItr=itrRange.first;
	if (true) {
		CPartyRsrcRoutingTblKey* prtRsrcRKey = (CPartyRsrcRoutingTblKey*)(&(*myMixItr).first);
		if (prtRsrcRKey)
		{
			const PartyRsrcID pid = prtRsrcRKey->GetPartyRsrcId();
			const ConnectionID cid = prtRsrcRKey->GetConnectionId();
			eLogicalResourceTypes elg = prtRsrcRKey->GetLogicalRsrcType();

			int x = 5;
			x += 1;

		}

		for(  ; myMixItr!= itrRange.second; ++myMixItr )
		{
			if((RoutingTblKey.GetPartyRsrcId()!= (*myMixItr).first.GetPartyRsrcId()) || (LRT != (*myMixItr).first.GetLogicalRsrcType())) //check out of bounds
			{
				return pOutPutVector;
			}
			else
			{
				pOutPutVector->push_back(((*myMixItr).first).GetRsrcDesc());
			}
		}
	}
	return pOutPutVector;
*/
}



