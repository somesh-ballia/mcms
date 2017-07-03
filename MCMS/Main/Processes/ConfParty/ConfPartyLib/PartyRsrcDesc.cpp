//+========================================================================+
//                            URSRCDES.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PartyRsrcDesc.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |		    |                                                     |
//+========================================================================+
#include "NStream.h"
#include "PartyRsrcDesc.h"
#include "AllocateStructs.h"
#include "ConfPartyRoutingTable.h"
#include "Trace.h"
#include "ObjString.h"
#include "StatusesGeneral.h"
#include "EnumsToStrings.h"


//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern char* LogicalResourceTypeToString(APIU32 logicalResourceType);
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );


/////////////////////////////////////////////////////////////////////////////

CPartyRsrcDesc::CPartyRsrcDesc()          // constructor
{
	m_status = STATUS_OK;
	m_confRsrcId = 0;
	m_partyRsrcId = 0;
	m_pRsrcDescVector = new VECTOR_OF_RSRC_DESC;
	m_networkPartyType = eNetwork_party_type_none;
	m_videoPartyType = eVideo_party_type_none;
	m_RoomId = 0xFFFF;
	m_ConfMediaType=eConfMediaType_dummy;
	m_isAvcVswInMixedMode = 0;
}

/////////////////////////////////////////////////////////////////////////////
CPartyRsrcDesc::~CPartyRsrcDesc()     // destructor
{   
	PDELETE(m_pRsrcDescVector);
	//PTRACE(eLevelInfoNormal, "~CPartyRsrcDesc Dtor");
}
/////////////////////////////////////////////////////////////////////////////
CPartyRsrcDesc::CPartyRsrcDesc(const CPartyRsrcDesc& rhs)
:CPObject(rhs)
{
	//PTRACE(eLevelInfoNormal, "CPartyRsrcDesc CopyCtor");
	m_status			= rhs.m_status;
	m_confRsrcId		= rhs.m_confRsrcId;
	m_partyRsrcId		= rhs.m_partyRsrcId;
	if(rhs.m_pRsrcDescVector)
		m_pRsrcDescVector = new VECTOR_OF_RSRC_DESC(*rhs.m_pRsrcDescVector);
	else
		m_pRsrcDescVector = NULL;
	m_networkPartyType  = rhs.m_networkPartyType;
	m_videoPartyType    = rhs.m_videoPartyType;
	m_RoomId			= rhs.m_RoomId;
	m_ConfMediaType     = rhs.m_ConfMediaType;
	m_isAvcVswInMixedMode = rhs.m_isAvcVswInMixedMode;
}
/////////////////////////////////////////////////////////////////////////////
const CPartyRsrcDesc& CPartyRsrcDesc::operator=(const CPartyRsrcDesc& rhs)
{
	if ( &rhs == this ) return *this;
	//PTRACE(eLevelInfoNormal, "CPartyRsrcDesc::operator=");
	m_status			= rhs.m_status;
	m_confRsrcId		= rhs.m_confRsrcId;
	m_partyRsrcId		= rhs.m_partyRsrcId;
	if(rhs.m_pRsrcDescVector)
		m_pRsrcDescVector = new VECTOR_OF_RSRC_DESC(*rhs.m_pRsrcDescVector);
	else
		m_pRsrcDescVector = NULL;	
	m_networkPartyType  = rhs.m_networkPartyType;
	m_videoPartyType    = rhs.m_videoPartyType;
	m_RoomId			= rhs.m_RoomId;
	m_ConfMediaType     = rhs.m_ConfMediaType;
	m_isAvcVswInMixedMode = rhs.m_isAvcVswInMixedMode;
	
	return *this;	
}

/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
WORD operator==(const CPartyRsrcDesc& lhs,const CPartyRsrcDesc& rhs)
{
	return (lhs.m_partyRsrcId == rhs.m_partyRsrcId);
}
/////////////////////////////////////////////////////////////////////////
VECTOR_OF_RSRC_DESC* CPartyRsrcDesc::GetRsrcVector()const
{
	return m_pRsrcDescVector;
}
////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcDesc::Serialize(WORD format,CSegment &seg)
{                         	
	switch ( format )  
	{    
		
	case NATIVE :
		{     
			seg << m_status
				<< m_confRsrcId
				<< m_partyRsrcId;
			if(m_pRsrcDescVector)
			{
				seg << (DWORD)m_pRsrcDescVector->size();
				seg << (BYTE)m_networkPartyType;
				seg << (BYTE)m_videoPartyType;
				for(WORD i=0; i<m_pRsrcDescVector->size(); i++)
				{
					((*m_pRsrcDescVector)[i]).Serialize(format, seg);
				}
			}
			else
			{
				seg << (DWORD)0;
				seg << (BYTE)m_networkPartyType;
				seg << (BYTE)m_videoPartyType;
			}
			seg << m_RoomId;
			seg << (BYTE)m_ConfMediaType;
			break;
		}
	case SERIALEMBD :
		{
			//Insert to RSRC ALLOCATOR API STRUCT
			break;
		}
	default :
		{
			break;   
		}
	}       
}
/////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcDesc::DeSerialize(WORD format,CSegment &seg)
{                         
	switch ( format ) 
	{    
		
	case NATIVE :
		{
			BYTE tempNetworkPartyType; 
			BYTE tempVideoPartyType;
			BYTE tempConfMediaType;
			DWORD sizeOfVector;
			seg >> m_status
				>> m_confRsrcId
				>> m_partyRsrcId
				>> sizeOfVector
				>> tempNetworkPartyType
				>> tempVideoPartyType;
				
				m_networkPartyType = (eNetworkPartyType) tempNetworkPartyType;
				m_videoPartyType = (eVideoPartyType) tempVideoPartyType;
			if(m_pRsrcDescVector)
			{
				PDELETE(m_pRsrcDescVector);
			}
			if(sizeOfVector)
			{
				m_pRsrcDescVector = new VECTOR_OF_RSRC_DESC;
				for(WORD i=0; i<sizeOfVector; i++)
				{
					CRsrcDesc tempRsrc;
					tempRsrc.DeSerialize(format, seg);
					m_pRsrcDescVector->push_back(tempRsrc);
				}
			}
			seg >> m_RoomId
			    >>  tempConfMediaType;
			m_ConfMediaType=(eConfMediaType)tempConfMediaType;
			break;
		}
	case SERIALEMBD :
		{
			//Retrieve from RSRC ALLOCATOR API STRUCT
			DWORD  structLen = sizeof(ALLOC_PARTY_IND_PARAMS_S_BASE);
			ALLOC_PARTY_IND_PARAMS_S_BASE  tAllocatePartyIndParamsBase;
			memset(&tAllocatePartyIndParamsBase,0,structLen);
			seg.Get((BYTE*)(&tAllocatePartyIndParamsBase),structLen);
			
			m_status = tAllocatePartyIndParamsBase.status;
			m_partyRsrcId = tAllocatePartyIndParamsBase.rsrc_party_id;
			m_confRsrcId = tAllocatePartyIndParamsBase.rsrc_conf_id;
			m_networkPartyType = tAllocatePartyIndParamsBase.networkPartyType;
			m_videoPartyType = tAllocatePartyIndParamsBase.videoPartyType;
			DWORD numOfRsrc = tAllocatePartyIndParamsBase.numRsrcs;
			m_RoomId = tAllocatePartyIndParamsBase.room_id;
			m_ConfMediaType=tAllocatePartyIndParamsBase.confMediaType;
			m_isAvcVswInMixedMode=tAllocatePartyIndParamsBase.isAvcVswInMixedMode;

			DWORD connectionId;
			eLogicalResourceTypes lrt;
			
			for(DWORD i= 0; i< numOfRsrc; i++)
			{
				connectionId = tAllocatePartyIndParamsBase.allocatedRrcs[i].connectionId;
				lrt = tAllocatePartyIndParamsBase.allocatedRrcs[i].logicalRsrcType;
				if(m_pRsrcDescVector)
				{
					m_pRsrcDescVector->push_back(CRsrcDesc(connectionId, lrt));
				}
			}
			
			break;
		}
	default :
		{
			break;   
		}		
	}       
}
/////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcDesc::DumpToTrace()
{
	CMedString *pStr = new CMedString;
	*pStr <<"CPartyRsrcDesc::DumpToTrace() - ALLOC_PARTY_IND_PARAMS_S:\n"
		<< "status           =   "<< CProcessBase::GetProcess()->GetStatusAsString(m_status).c_str() <<'\n'
		<< "rsrc_conf_id     =   "<< m_confRsrcId <<'\n'
		<< "rsrc_party_id    =   "<< m_partyRsrcId <<'\n'
		<< "numRsrcs         =   "<< (WORD)(m_pRsrcDescVector ? m_pRsrcDescVector->size() : 0) <<'\n'
		<< "networkPartyType =   "<< eNetworkPartyTypeNames[m_networkPartyType] <<'\n'
		<< "videoPartyType   =   "<< eVideoPartyTypeNames[m_videoPartyType]<<'\n'
		<< "room_Id   	     =   "<< m_RoomId <<'\n'
		<< "confMediaType    =   "<< m_ConfMediaType <<'\n'
		<< "isAvcVswInMixedMode ="<< (WORD)m_isAvcVswInMixedMode <<'\n';

	for(WORD i=0; m_pRsrcDescVector && i<m_pRsrcDescVector->size(); i++)
	{
		*pStr  << (i+1) << ". connId = "<< (*m_pRsrcDescVector)[i].GetConnectionId()
			   <<" : logicalRsrcType = "
			   << ::LogicalResourceTypeToString( (*m_pRsrcDescVector)[i].GetLogicalRsrcType() ) << '\n';
	}

	PTRACE(eLevelInfoNormal, pStr->GetString());
	POBJDELETE(pStr);
}
/////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcDesc::InsertToGlobalRsrcRoutingTbl()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	CPartyRsrcRoutingTblKey routingKey;
	DWORD connectionId;
	eLogicalResourceTypes lrt;

	for(WORD i= 0; i< m_pRsrcDescVector->size(); ++i )
	{ 
		connectionId = (*m_pRsrcDescVector)[i].GetConnectionId();
		lrt = (*m_pRsrcDescVector)[i].GetLogicalRsrcType();
		routingKey = CPartyRsrcRoutingTblKey(connectionId, m_partyRsrcId, lrt);
		pRoutingTbl->AddPartyRsrcDesc(routingKey);
	}
}
////////////////////////////////////////////////////////////////////////////   
void CPartyRsrcDesc::DeleteFromGlobalRsrcRoutingTbl()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	pRoutingTbl->RemoveAllPartyRsrcs( m_partyRsrcId);
}
/////////////////////////////////////////////////////////////////////////////   
ConnectionID CPartyRsrcDesc::GetConnectionId(eLogicalResourceTypes lrt)
{
	for(WORD i= 0; i< m_pRsrcDescVector->size(); ++i )
	{ 
		if(lrt == (*m_pRsrcDescVector)[i].GetLogicalRsrcType())
			return((*m_pRsrcDescVector)[i].GetConnectionId());
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////   
CRsrcDesc CPartyRsrcDesc::GetRsrcDesc(eLogicalResourceTypes lrt)
{
	for(WORD i= 0; i< m_pRsrcDescVector->size(); ++i )
	{ 
		if(lrt == (*m_pRsrcDescVector)[i].GetLogicalRsrcType())
			return((*m_pRsrcDescVector)[i]);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////   
CRsrcParams CPartyRsrcDesc::GetRsrcParams(eLogicalResourceTypes lrt)
{
	CRsrcParams RsrcParams;
	RsrcParams.SetRsrcDesc(GetRsrcDesc(lrt));
	RsrcParams.SetConfRsrcId(m_confRsrcId);
	RsrcParams.SetPartyRsrcId(m_partyRsrcId);
	return(RsrcParams);
}
/////////////////////////////////////////////////////////////////////////////
WORD CPartyRsrcDesc::GetRsrcDesc(CRsrcDesc& rsrcDesc,eLogicalResourceTypes lrt,WORD item_number)
{
    WORD found = 0;
	for(WORD i= 0; i< m_pRsrcDescVector->size(); ++i )
	{ 
		if(lrt == (*m_pRsrcDescVector)[i].GetLogicalRsrcType()){
            rsrcDesc = (*m_pRsrcDescVector)[i];
            found++ ;
            if(found==item_number)
                break;
            
        }
	}
    if(found!=item_number){
        found=0;
    }
    return found;
    
}
/////////////////////////////////////////////////////////////////////////////
WORD CPartyRsrcDesc::GetRsrcDesc(CRsrcDesc& rsrcDesc,ConnectionID connectionId)
{
    WORD found = 0;
	for(WORD i= 0; i< m_pRsrcDescVector->size(); ++i )
	{
        if(connectionId == (*m_pRsrcDescVector)[i].GetConnectionId()){
            rsrcDesc = (*m_pRsrcDescVector)[i];
            found=1 ;
        }
	}
    return found;
}
/////////////////////////////////////////////////////////////////////////////   
WORD CPartyRsrcDesc::GetRsrcParams(CRsrcParams& rsrcParams,eLogicalResourceTypes lrt,WORD item_number)
{
    WORD found = 0;
    CRsrcDesc searchRsrc;
    found = GetRsrcDesc(searchRsrc,lrt,item_number);
    if(found){
        rsrcParams.SetRsrcDesc(searchRsrc);
    }
	rsrcParams.SetConfRsrcId(m_confRsrcId);
	rsrcParams.SetPartyRsrcId(m_partyRsrcId);
	rsrcParams.SetRoomId(m_RoomId);
	return found;
}
/////////////////////////////////////////////////////////////////////////////
void CPartyRsrcDesc::AddNewRsrcDesc(CRsrcDesc *pRsrcDesc)
{
	if(m_pRsrcDescVector)
		m_pRsrcDescVector->push_back(*pRsrcDesc);
	else
		DBGPASSERT(1);

	// insert to global table
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL) {
		PASSERT_AND_RETURN(101);
	}

	CPartyRsrcRoutingTblKey routingKey;
	DWORD connectionId = pRsrcDesc->GetConnectionId();
	eLogicalResourceTypes lrt = pRsrcDesc->GetLogicalRsrcType();

	routingKey = CPartyRsrcRoutingTblKey(connectionId, m_partyRsrcId, lrt);
	pRoutingTbl->AddPartyRsrcDesc(routingKey);
}
/////////////////////////////////////////////////////////////////////////////
void CPartyRsrcDesc::DeleteRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt)
{
	// Remove from Table:
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL) {
		PASSERT_AND_RETURN(101);
	}

	CRsrcParams pRsrcParams;
	WORD found = GetRsrcParams(pRsrcParams, lrt);
	if (found)
	{
		pRoutingTbl->RemovePartyRsrc(pRsrcParams);
		// Remove from vector:
		DeleteRsrcDescFromVector(lrt);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
void CPartyRsrcDesc::DeleteAllRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt)
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL) {
		PASSERT_AND_RETURN(101);
	}

	VECTOR_OF_RSRC_DESC_POINTERS*  pVectorRsrcDesc = pRoutingTbl->GetAllPartyIdLogicalTypeRsrcs(m_partyRsrcId, lrt);
	VECTOR_OF_RSRC_DESC_POINTERS::iterator itr	=  pVectorRsrcDesc->begin();
	while ( itr != pVectorRsrcDesc->end() )
	{
		CRsrcParams RsrcParams;
		RsrcParams.SetRsrcDesc(*(*itr));
		RsrcParams.SetConfRsrcId(m_confRsrcId);
		RsrcParams.SetPartyRsrcId(m_partyRsrcId);

		pRoutingTbl->RemovePartyRsrc(RsrcParams);
		itr++;
	}
	POBJDELETE(pVectorRsrcDesc);
}
///////////////////////////////////////////////////////////////////////////////////////////
void CPartyRsrcDesc::AddAllRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt)
{
	WORD size = m_pRsrcDescVector->size();
	for(WORD i= 0; i< size; ++i )
	{
		if(lrt == (*m_pRsrcDescVector)[i].GetLogicalRsrcType())
			AddNewRsrcDesc(&((*m_pRsrcDescVector)[i]));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
void CPartyRsrcDesc::DeleteDummyRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt)
{
	// vngr-24826
	// since eLogical_VSW_dummy_encoder and eLogical_VSW_dummy_decoder
	// are not in m_pRsrcDescVector assert jumped
	// the fix: use eLogical_video_encoder / decoder in order to get the conf id and party id

	// Remove from Table:
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (pRoutingTbl== NULL) {
		PASSERT_AND_RETURN(101);
	}

	eLogicalResourceTypes lrt_temp = lrt;
	CRsrcParams aRsrcParams;
	if (lrt == eLogical_VSW_dummy_encoder)
		lrt_temp = eLogical_video_encoder;
	else if (lrt == eLogical_VSW_dummy_decoder)
		lrt_temp = eLogical_video_decoder;

	WORD found = GetRsrcParams(aRsrcParams, lrt_temp); // will get party id and conf id
	TRACEINTO << "CPartyRsrcDesc::DeleteDummyRsrcDescAccordingToLogicalResourceType: lrt: " << lrt << ", aRsrcParams: " << aRsrcParams.GetPartyRsrcId() << ", found:" << found;
	if (found)
	{
		aRsrcParams.SetLogicalRsrcType(lrt);
		pRoutingTbl->RemovePartyRsrc(aRsrcParams);
	}
}

///////////////////////////////////////////////////////////////
void CPartyRsrcDesc::DeleteRsrcDescFromVector(eLogicalResourceTypes lrt)
{
	if (m_pRsrcDescVector)
	{
		VECTOR_OF_RSRC_DESC::iterator itr;

		itr = m_pRsrcDescVector->begin();
		for ( ; itr != m_pRsrcDescVector->end(); ++itr)
		{
			CRsrcDesc temp = (*itr);
			if (lrt == temp.GetLogicalRsrcType())
				break;
		}

		if (itr != m_pRsrcDescVector->end())
			m_pRsrcDescVector->erase(itr);
		else
			PTRACE(eLevelInfoNormal,"CPartyRsrcDesc::DeleteRsrcDescFromVector : Not found");
	}
	else
		DBGPASSERT(1);
}
//////////////////////////////////////////////////////////////////////
// Class CIsdnPartyRsrcDesc
CIsdnPartyRsrcDesc::CIsdnPartyRsrcDesc()          // constructor
{
	m_pIsdnParamsVector = new std::vector<CIsdnSpanOrderPerConnection*>;
	memset(tmpPhoneNumber,'\0',ISDN_PHONE_NUMBER_DIGITS_LEN);
	//tmpPhoneNumber[0]='\0';
	m_numAllocatedChannels = 0;
	m_ssrcAudio = 0;
}

/////////////////////////////////////////////////////////////////////////////
CIsdnPartyRsrcDesc::~CIsdnPartyRsrcDesc()     // destructor
{   
	//PTRACE(eLevelInfoNormal, "CIsdnPartyRsrcDesc Dtor");
	ClearInternalVector();
	
	PDELETE(m_pIsdnParamsVector);
}
/////////////////////////////////////////////////////////////////////////////
CIsdnPartyRsrcDesc::CIsdnPartyRsrcDesc(const CIsdnPartyRsrcDesc& other):CPartyRsrcDesc(other)
{
	//PTRACE(eLevelInfoNormal, "CIsdnPartyRsrcDesc Copy Ctor");
    //*this = other;
	m_pIsdnParamsVector = new std::vector<CIsdnSpanOrderPerConnection*>;
	if(other.m_pIsdnParamsVector)
	{
		std::vector<CIsdnSpanOrderPerConnection*>::iterator it;
		for(it=other.m_pIsdnParamsVector->begin(); it != other.m_pIsdnParamsVector->end();++it)
		{
			CIsdnSpanOrderPerConnection* tmpSpan = new CIsdnSpanOrderPerConnection(**it);
			m_pIsdnParamsVector->push_back(tmpSpan);
		}
	}
	
	 if (other.tmpPhoneNumber)
	 {
		 strncpy(tmpPhoneNumber,other.tmpPhoneNumber,ISDN_PHONE_NUMBER_DIGITS_LEN);
		 tmpPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN-1]= '\0';
	 }
	 m_numAllocatedChannels = other.m_numAllocatedChannels;
	 m_ssrcAudio = other.m_ssrcAudio;
}
/////////////////////////////////////////////////////////////////////////////
const CIsdnPartyRsrcDesc& CIsdnPartyRsrcDesc::operator=(const CIsdnPartyRsrcDesc& rhs)
{
	//PTRACE(eLevelInfoNormal, "CIsdnPartyRsrcDesc Operator=");
	if ( &rhs == this ) return *this;
	
	if(rhs.m_pIsdnParamsVector)
	{
	    PDELETE(m_pIsdnParamsVector);
		m_pIsdnParamsVector = new std::vector<CIsdnSpanOrderPerConnection*>;
		std::vector<CIsdnSpanOrderPerConnection*>::iterator it;
		for(it=rhs.m_pIsdnParamsVector->begin(); it != rhs.m_pIsdnParamsVector->end();++it)
		{
			CIsdnSpanOrderPerConnection* tmpSpan = new CIsdnSpanOrderPerConnection(**it);
			m_pIsdnParamsVector->push_back(tmpSpan);
		}
	}
	if (rhs.tmpPhoneNumber)
	{
		strncpy(tmpPhoneNumber,rhs.tmpPhoneNumber,ISDN_PHONE_NUMBER_DIGITS_LEN);
		tmpPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN-1]= '\0';
	}

	m_numAllocatedChannels = rhs.m_numAllocatedChannels;
	return *this;	
}

/////////////////////////////////////////////////////////////////////////////
const char* CIsdnPartyRsrcDesc::NameOf()  const
{
	return "CIsdnPartyRsrcDesc";
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnPartyRsrcDesc::Serialize(WORD format,CSegment &seg)
{
	CPartyRsrcDesc::Serialize(format, seg);
	switch ( format ) 
		{  
		case NATIVE:
			{
				PTRACE(eLevelInfoNormal, "CIsdnPartyRsrcDesc::Serialize - NATIVE for internal use only partyCntl<->party");
				seg << tmpPhoneNumber;
				if(m_pIsdnParamsVector)
				{
					seg << (DWORD)m_pIsdnParamsVector->size();
					for(WORD i=0; i<m_pIsdnParamsVector->size(); i++)
					{
						(m_pIsdnParamsVector->at(i))->Serialize(format, seg);
					}
				}
				else
				{
					seg << (DWORD)0;
				}
				break;
			}
		case SERIALEMBD :
			{
				//do nothing
				break;
			}
		default:
			{
				break;
			}
		}
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnPartyRsrcDesc::DeSerialize(WORD format,CSegment &seg)
{
    
	switch ( format ) 
	{  
	case NATIVE:
		{
			CPartyRsrcDesc::DeSerialize(format, seg);
			PTRACE(eLevelInfoNormal,"CIsdnPartyRsrcDesc::DeSerialize - NATIVE for internal use only partyCntl<->party");
			DWORD paramsVectorLen = 0;
			seg >> tmpPhoneNumber;
			seg >> paramsVectorLen;
			if(paramsVectorLen)
			{			
				if(m_pIsdnParamsVector)
				{
					ClearInternalVector();
					PDELETE(m_pIsdnParamsVector);
				}
				m_pIsdnParamsVector = new std::vector<CIsdnSpanOrderPerConnection*>;
				for(WORD i=0; i<paramsVectorLen; i++)
				{
					CIsdnSpanOrderPerConnection* tmpSpan = new CIsdnSpanOrderPerConnection();
					tmpSpan->DeSerialize(format, seg);
					m_pIsdnParamsVector->push_back(tmpSpan);
				}
			}
			break;
		}
	case SERIALEMBD :
		{
			CSegment segm_copy (seg);
			CPartyRsrcDesc::DeSerialize(format, segm_copy);
			//Retrieve from RSRC ALLOCATOR API STRUCT
			DWORD  structLen = sizeof(ALLOC_PARTY_IND_PARAMS_S);
			ALLOC_PARTY_IND_PARAMS_S  tAllocatePartyIndParams;
			memset(&tAllocatePartyIndParams,0,structLen);
			seg.Get((BYTE*)(&tAllocatePartyIndParams),structLen);
			
			strncpy(tmpPhoneNumber,tAllocatePartyIndParams.isdnParams.BondingTemporaryPhoneNumber,ISDN_PHONE_NUMBER_DIGITS_LEN);
			tmpPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN-1]='\0';
				
			for(DWORD i=0; i< NUM_E1_PORTS; i++)
			{
				SPANS_ORDER_LIST_PER_PORT tSpansOrderListPerPort = tAllocatePartyIndParams.isdnParams.spans_order.port_spans_list[i];
				CIsdnSpanOrderPerConnection* tmpSpan = new CIsdnSpanOrderPerConnection();
				tmpSpan->SetBoardId(tSpansOrderListPerPort.board_id);
				tmpSpan->SetConnectionId(tSpansOrderListPerPort.conn_id);
				for (int k=0;k<MAX_NUM_SPANS_ORDER;k++)
					tmpSpan->AddSpanList(tSpansOrderListPerPort.spans_list[k]);
				
				m_pIsdnParamsVector->push_back(tmpSpan);
			}
			m_numAllocatedChannels = tAllocatePartyIndParams.isdnParams.num_of_isdn_ports;
			m_ssrcAudio = tAllocatePartyIndParams.svcParams.m_ssrcAudio;
			break;
		}
		default :
		{
			break;   
		}		
	}       
}
////////////////////////////////////////////////////////////////////////
void CIsdnPartyRsrcDesc::DumpToTrace()
{
	CPartyRsrcDesc::DumpToTrace();
	
	CLargeString cstr;
	
	cstr << "CIsdnPartyRsrcDesc::DumpToTrace()" << "\n";
	
	if (tmpPhoneNumber)
		cstr << "Party Temp Phone Number: " << tmpPhoneNumber << "\n";
	
	cstr << "SpanOrder Per Board Vector:" << "\n";
	for(WORD i = 0 ; m_pIsdnParamsVector && i < m_pIsdnParamsVector->size(); i++)
		(m_pIsdnParamsVector->at(i))->DumpToTrace(cstr);
		
	cstr << "numAllocatedChannels = " << m_numAllocatedChannels << "\n";
	PTRACE (eLevelInfoNormal, cstr.GetString());
}
////////////////////////////////////////////////////////////////////////
void CIsdnPartyRsrcDesc::ClearInternalVector()
{
	
	std::vector<CIsdnSpanOrderPerConnection*>::iterator it;
	for(it=m_pIsdnParamsVector->begin(); it != m_pIsdnParamsVector->end();++it)
	    POBJDELETE(*it);
		    
		  
	m_pIsdnParamsVector->clear();
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnPartyRsrcDesc::SetSsrcAudio(DWORD ssrcAudio)
{
	m_ssrcAudio = ssrcAudio;
}
////////////////////////////////////////////////////////////////////////
//			
//			Class CIsdnSpanOrderPerConnection
//	CIsdnSpanOrderPerConnection();
//  CIsdnSpanOrderPerConnection(const CIsdnSpanOrderPerConnection& other);
//  virtual ~CIsdnSpanOrderPerConnection(); 
//  virtual const CIsdnSpanOrderPerConnection& operator=(const CIsdnSpanOrderPerConnection&);
//  virtual const char*  NameOf() const;
//				
//	std::vector<DWORD>* m_ConnectionIdVector;
//  std::vector<WORD>* m_spansListVector;
////////////////////////////////////////////////////////////////////////
CIsdnSpanOrderPerConnection::CIsdnSpanOrderPerConnection()          // constructor
{
	m_spansListVector = new std::vector<WORD>;
	m_ConnectionId = 0;
	m_boardId = 0;
}
/////////////////////////////////////////////////////////////////////////////
CIsdnSpanOrderPerConnection::~CIsdnSpanOrderPerConnection()     // destructor
{   
	if (m_spansListVector)
	    m_spansListVector->clear();

	PDELETE(m_spansListVector);
}
/////////////////////////////////////////////////////////////////////////////
CIsdnSpanOrderPerConnection::CIsdnSpanOrderPerConnection(const CIsdnSpanOrderPerConnection& other)
:CPObject(other)
{
  	if(other.m_spansListVector)
		m_spansListVector = new std::vector<WORD>(*other.m_spansListVector);
	else
	    m_spansListVector = new std::vector<WORD>;
	
	m_boardId = other.m_boardId;
	m_ConnectionId = other.m_ConnectionId;
	
}
/////////////////////////////////////////////////////////////////////////////
const CIsdnSpanOrderPerConnection& CIsdnSpanOrderPerConnection::operator=(const CIsdnSpanOrderPerConnection& rhs)
{
	if ( &rhs == this ) return *this;
	
	
	if(rhs.m_spansListVector)
	{
	    PDELETE(m_spansListVector);
		m_spansListVector = new std::vector<WORD>(*rhs.m_spansListVector);
	}
	
	m_boardId = rhs.m_boardId;
	m_ConnectionId = rhs.m_ConnectionId;
	
	return *this;	
}

/////////////////////////////////////////////////////////////////////////////
const char*   CIsdnSpanOrderPerConnection::NameOf()  const
{
	return "CIsdnSpanOrderPerConnection";
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnSpanOrderPerConnection::Serialize(WORD format,CSegment &seg)
{
	//CLargeString cstr;
	//cstr << "CIsdnSpanOrderPerConnection::Serialize" << "\n";
	
	WORD spanListVectorSize = m_spansListVector? m_spansListVector->size() : 0 ;
	//cstr << "pushing to seg (WORD) m_boardId: " << m_boardId << "\n";
	seg << (WORD)m_boardId;
	
	seg << m_ConnectionId;
	
	//cstr << "\npushing to seg (DWORD) spanListVectorSize: " << spanListVectorSize << "\n";
	seg << (DWORD)spanListVectorSize;
	//cstr << "pushing to seg spanListVectorSize: \n";
	for (int i = 0 ; i < spanListVectorSize;i++)
	{
		//cstr << "[" << i << "] = " << m_spansListVector->at(i) << " ,";
		seg << (WORD)m_spansListVector->at(i);
	}
	//PTRACE(eLevelInfoNormal,cstr.GetString());
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnSpanOrderPerConnection::DeSerialize(WORD format,CSegment &seg)
{
	//CLargeString cstr;
	//cstr << "CIsdnSpanOrderPerConnection::DeSerialize" << "\n";
	
	DWORD spanListVectorSize = 0; 
	seg >> m_boardId;
	
	seg >> m_ConnectionId;
	
	//deseirialize m_spansListVector
	seg >> spanListVectorSize;
	//cstr << "\npulling from seg (DWORD) spanListVectorSize: " << spanListVectorSize << "\n";
	if(m_spansListVector)
	{
		m_spansListVector->clear();

		if(spanListVectorSize)
		{
			for(WORD i=0; i<spanListVectorSize; i++)
			{
				WORD tmpSpan;
				seg >> tmpSpan;
				m_spansListVector->push_back(tmpSpan);
				//cstr << "[" << i << "] = " << m_spansListVector->at(i) << " ,";
			}
		}
	}
	//PTRACE(eLevelInfoNormal,cstr.GetString());
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnSpanOrderPerConnection::DumpToTrace(CLargeString& cstr)
{
	if (m_ConnectionId)
	{
		cstr << "Connection Id: " << m_ConnectionId << "\n";
		cstr << "Board Id: " << m_boardId << "\n";

		cstr << "Span List Vector =  { ";
		for (WORD j = 0;j < m_spansListVector->size();j++)
		{
			cstr << m_spansListVector->at(j);
			if (j != m_spansListVector->size() - 1)
				cstr << ",";
		}
		cstr << " } \n";
	}
		
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnSpanOrderPerConnection::SetBoardId(WORD boardId)
{
	m_boardId = boardId;
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnSpanOrderPerConnection::SetConnectionId(DWORD connectionId)
{
	m_ConnectionId = connectionId;
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnSpanOrderPerConnection::AddSpanList(WORD spanList)
{
	m_spansListVector->push_back(spanList);
}




