// The role of CPartyRsrcDesc class is to identify all party resources and to provide
// resource information.                

// notes :                    
//  The object can be created:
//    1. by resource allocator.
//////////////////////////////////////////////////////////////////////////////

#ifndef _PARTYRSRCDESC
#define _PARTYRSRCDESC
#include "ConfApi.h"
#include "PObject.h"
#include "RsrcDesc.h"
#include "RsrcParams.h"

class CIsdnSpanOrderPerConnection;

template class std::vector < CRsrcDesc > ;
template class std::vector < CIsdnSpanOrderPerConnection* > ;
template class std::vector < DWORD > ;
template class std::vector < WORD > ;


typedef std::vector< CRsrcDesc > VECTOR_OF_RSRC_DESC;

class CPartyRsrcDesc : public CPObject
{
CLASS_TYPE_1(CPartyRsrcDesc, CPObject)
public:             
	
	// Constructors
	CPartyRsrcDesc();
	CPartyRsrcDesc(const CPartyRsrcDesc&);
	virtual ~CPartyRsrcDesc(); 
	virtual const CPartyRsrcDesc& operator=(const CPartyRsrcDesc&);

	virtual void  Serialize(WORD format,CSegment &seg); 
	virtual void  DeSerialize(WORD format,CSegment &seg); 
	virtual const char* NameOf() const {return "CPartyRsrcDesc";}
	
	friend WORD operator==(const CPartyRsrcDesc& lhs,const CPartyRsrcDesc& rhs);    
	
	DWORD GetStatus()const						{return m_status;}
	ConfRsrcID GetConfRsrcId()const				{return m_confRsrcId;}
	PartyRsrcID GetPartyRsrcId()const			{return m_partyRsrcId;}
	eVideoPartyType GetVideoPartyType() const   {return m_videoPartyType;}
	void SetVideoPartyType(eVideoPartyType videoPartyType) {m_videoPartyType = videoPartyType;}
	eNetworkPartyType GetNetworkPartyType() const {return m_networkPartyType;}
	
	VECTOR_OF_RSRC_DESC* GetRsrcVector()const;	
	void InsertToGlobalRsrcRoutingTbl();
	void DeleteFromGlobalRsrcRoutingTbl();
	ConnectionID GetConnectionId(eLogicalResourceTypes);
	CRsrcDesc GetRsrcDesc(eLogicalResourceTypes);
	CRsrcParams GetRsrcParams(eLogicalResourceTypes);
	void SetPartyRsrcId(PartyRsrcID partyRsrcId)	{m_partyRsrcId = partyRsrcId;}
	void SetConfRsrcId(ConfRsrcID confRsrcId)	{m_confRsrcId = confRsrcId;}
	virtual void DumpToTrace();

	WORD GetRsrcDesc(CRsrcDesc& rsrcDesc,eLogicalResourceTypes lrt,WORD item_number=1);
	WORD GetRsrcParams(CRsrcParams& rsrcParams,eLogicalResourceTypes lrt,WORD item_number=1);
	WORD GetRsrcDesc(CRsrcDesc& rsrcDesc,ConnectionID connectionId);
	void AddNewRsrcDesc(CRsrcDesc *pRsrcDesc);
	void DeleteRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt);
	void AddAllRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt);
	void DeleteAllRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt);

	void DeleteDummyRsrcDescAccordingToLogicalResourceType(eLogicalResourceTypes lrt);
	void DeleteRsrcDescFromVector(eLogicalResourceTypes lrt);

	void SetRoomId(WORD room){m_RoomId = room;}
	WORD GetRoomId() const	{return m_RoomId;}
	void SetConfMediaType(eConfMediaType confMediaType){m_ConfMediaType=confMediaType;}
	eConfMediaType GetConfMediaType(){return m_ConfMediaType;}
    void SetIsAvcVswInMixedMode(BOOL isAvcVswInMixedMode) {m_isAvcVswInMixedMode = isAvcVswInMixedMode;}
    BOOL GetIsAvcVswInMixedMode() {return m_isAvcVswInMixedMode;}

	// Attributes             
	
protected:
	
	// Attributes
	DWORD					m_status;
	ConfRsrcID				m_confRsrcId;  
	PartyRsrcID  			m_partyRsrcId;
	eNetworkPartyType		m_networkPartyType;
	eVideoPartyType			m_videoPartyType;
	VECTOR_OF_RSRC_DESC*	m_pRsrcDescVector;
	WORD					m_RoomId;
	eConfMediaType          m_ConfMediaType;
	BOOL                    m_isAvcVswInMixedMode;

	// Operations   
	
};

class CIsdnSpanOrderPerConnection : public CPObject
{
	CLASS_TYPE_1(CIsdnSpanOrderPerConnection, CPObject)
	public:             
		
		// Constructors
		CIsdnSpanOrderPerConnection();
		CIsdnSpanOrderPerConnection(const CIsdnSpanOrderPerConnection& other);
		virtual ~CIsdnSpanOrderPerConnection(); 
		virtual const CIsdnSpanOrderPerConnection& operator=(const CIsdnSpanOrderPerConnection&);
		virtual const char*  NameOf() const;
	
		void  Serialize(WORD format,CSegment &seg); 
		void  DeSerialize(WORD format,CSegment &seg);
		void  DumpToTrace(CLargeString& cstr);
		
		void SetBoardId(WORD boardId);
		void SetConnectionId(DWORD connectionId);
		void AddSpanList(WORD spanList);
		
		WORD GetBoardId() {return m_boardId;}
		DWORD GetConnectionId() {return m_ConnectionId;}
		std::vector<WORD>* GetSpansListVector(){return m_spansListVector;}
		
	private:
		WORD m_boardId;
		DWORD m_ConnectionId;
		std::vector<WORD>* m_spansListVector;
};

class CIsdnPartyRsrcDesc : public CPartyRsrcDesc
{
CLASS_TYPE_1(CIsdnPartyRsrcDesc, CPartyRsrcDesc)
public:
	CIsdnPartyRsrcDesc();
	CIsdnPartyRsrcDesc(const CIsdnPartyRsrcDesc&);
	virtual ~CIsdnPartyRsrcDesc(); 
	virtual const CIsdnPartyRsrcDesc& operator=(const CIsdnPartyRsrcDesc&);
	virtual const char*  NameOf() const;
	
	virtual void  Serialize(WORD format,CSegment &seg); 
	virtual void  DeSerialize(WORD format,CSegment &seg);

	const char* GetTmpPhoneNumber() const {return tmpPhoneNumber;}
	std::vector<CIsdnSpanOrderPerConnection*>* GetSpanOrderPerConnectionVector() { return m_pIsdnParamsVector;}

	void DumpToTrace(); 
	WORD GetNumAllocatedChannels()const { return m_numAllocatedChannels; }
	
	DWORD GetSsrcAudio() {return m_ssrcAudio;}
	void  SetSsrcAudio(DWORD ssrcAudio);

protected:
	void ClearInternalVector();
	std::vector<CIsdnSpanOrderPerConnection*>* m_pIsdnParamsVector;
	char tmpPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN];
	WORD m_numAllocatedChannels;
	DWORD m_ssrcAudio;
};
#endif//_PARTYRSRCDESC

