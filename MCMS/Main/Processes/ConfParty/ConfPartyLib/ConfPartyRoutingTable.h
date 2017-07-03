// ConfPartyRoutingTable.h: interface for the CConfPartyRoutingTable class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_ConfPartyRoutingTable_H__)
#define _ConfPartyRoutingTable_H__

#include <map>
#include <vector>
#include "RsrcParams.h"
#include "RsrcDesc.h"
#include "HostCommonDefinitions.h"
#include "MplMcmsStructs.h"

class CPartyRsrcRoutingTblKey;
class CTaskApi;

template class std::vector < CRsrcDesc* > ;

typedef std::multimap<CPartyRsrcRoutingTblKey, CTaskApi * > PART_RSRC_ROUTING_TBL;
typedef std::map<ConfRsrcID, CTaskApi *> CONF_RSRC_ROUTING_TBL;

typedef std::vector<CRsrcDesc*> VECTOR_OF_RSRC_DESC_POINTERS;


class CPartyRsrcRoutingTblKey : public CPObject
{
	CLASS_TYPE_1(CPartyRsrcRoutingTblKey,CPObject)
public: 
	
	// Constructors
	CPartyRsrcRoutingTblKey(ConnectionID ConnectionId = DUMMY_CONNECTION_ID,
							PartyRsrcID ParId = DUMMY_PARTY_ID,
							eLogicalResourceTypes LRT = eLogical_res_none);
	CPartyRsrcRoutingTblKey(const CPartyRsrcRoutingTblKey&);
	CPartyRsrcRoutingTblKey(const CRsrcParams&);
	virtual const char* NameOf() const { return "CPartyRsrcRoutingTblKey";}

	virtual ~CPartyRsrcRoutingTblKey(); 
	virtual const CPartyRsrcRoutingTblKey& operator=(const CPartyRsrcRoutingTblKey&);

	void Print() const;
	
	// Operations
	PartyRsrcID GetPartyRsrcId() const;
	ConnectionID GetConnectionId() const;
	eLogicalResourceTypes GetLogicalRsrcType() const;
	
	CRsrcDesc* GetRsrcDesc() const;
	void SetConnectionId(ConnectionID);
	void SetPartyRsrcId(PartyRsrcID);
	void SetLogicalRsrcType(eLogicalResourceTypes);
	void SetRsrcDesc(CRsrcDesc);
	
	virtual void  Serialize(WORD format,CSegment &seg); 
	virtual void  DeSerialize(WORD format,CSegment &seg); 
	
	friend bool operator<(const CPartyRsrcRoutingTblKey& lhs,const CPartyRsrcRoutingTblKey& rhs);    
	
protected:
	// Attributes             
	PartyRsrcID		m_PartyRsrcID;
	//Created as pointer to enable inheritance by specific resource types in future
	CRsrcDesc*		m_pRsrcDesc; 
	// Operations   
};





class CConfPartyRoutingTable : public CPObject
{
CLASS_TYPE_1(CConfPartyRoutingTable,CPObject )

public:								// Constructors
	CConfPartyRoutingTable();
	virtual ~CConfPartyRoutingTable();  
	virtual const char* NameOf() const { return "CConfPartyRoutingTable";}

	// Operations								
	
	void* GetRsrcMngrPtrByPartyIdAndConnectionId(PartyRsrcID partyID, ConnectionID connectionID); //Retreives TaskApi Ptr
	eLogicalResourceTypes  GetLRTByPartyIdAndConnectionId(PartyRsrcID partyID, ConnectionID connectionID);
	void* GetRsrcMngrPtrByPartyId(PartyRsrcID partyID);
	void* GetRsrcMngrPtrByConfId(ConfRsrcID confID);

	int AddPartyRsrcDesc(CPartyRsrcRoutingTblKey key);
	int UpdatePartyRsrcIdInRoutingTbl(CPartyRsrcRoutingTblKey& key, PartyRsrcID newPartyID);
	
	int AddStateMachinePointerToRoutingTbl(CRsrcParams key, CTaskApi *);		
	CRsrcDesc*  AddStateMachinePointerToRoutingTbl(PartyRsrcID partyID, eLogicalResourceTypes LRT, CTaskApi *);
	
	int RemoveStateMachinePointerFromRoutingTbl(CRsrcParams key);

	CRsrcDesc* GetPartyRsrcDesc(PartyRsrcID partyID, eLogicalResourceTypes LRT);

	VECTOR_OF_RSRC_DESC_POINTERS*  GetAllPartyRsrcs(PartyRsrcID partyID);
	VECTOR_OF_RSRC_DESC_POINTERS*  GetAllPartyIdLogicalTypeRsrcs(PartyRsrcID partyID, eLogicalResourceTypes LRT);
	void  GetAllRsrcsFromLogicalType(PART_RSRC_ROUTING_TBL& pTbl, eLogicalResourceTypes LRT);
	void RemoveAllPartyRsrcs(PartyRsrcID partyID);
	
	void RemovePartyRsrc(CRsrcParams RoutingTblKey);

	int  AddConfToRoutingTbl(ConfRsrcID ConfRsrcId,  CTaskApi * );
	int  RemoveConfFromRoutingTbl(ConfRsrcID ConfRsrcId);
	
	void DumpTable();
private:
	PART_RSRC_ROUTING_TBL::iterator  FindInPartyRsrcRoutingTblByPartyIdAndConnectionId(PartyRsrcID partyID, ConnectionID connectionID,eLogicalResourceTypes LRT = eLogical_res_none);
	PART_RSRC_ROUTING_TBL::iterator  FindInPartyRsrcRoutingTblByPartyIdAndLRT(PartyRsrcID partyID, eLogicalResourceTypes LRT);
	CONF_RSRC_ROUTING_TBL::iterator  FindInConfRsrcRoutingTbl(ConfRsrcID confID);
	
	PART_RSRC_ROUTING_TBL m_RoutingTbl;
	CONF_RSRC_ROUTING_TBL m_confRoutingTbl;
};
#endif //_ConfPartyRoutingTable_H__
