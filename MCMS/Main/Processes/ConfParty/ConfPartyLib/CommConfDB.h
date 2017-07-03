#if !defined(_CCommConfDB_H__)
#define _CCommConfDB_H__

#include "SerializeObject.h"
#include "DefinesGeneral.h"
#include "CommConf.h"
#include "ConfParty.h"
#include "InitCommonStrings.h"


////////////////////////////////////////////////////////////////////////////
//                        CCommConfDB
////////////////////////////////////////////////////////////////////////////
class CCommConfDB : public CSerializeObject
{
	CLASS_TYPE_1(CCommConfDB, CSerializeObject)

public:
	                   CCommConfDB();
	                   CCommConfDB(const CCommConfDB& other);
	virtual           ~CCommConfDB();

	const char*        NameOf() const { return "CCommConfDB"; }

	CCommConfDB&       operator=(const CCommConfDB& other);

	void               SerializeXml(CXMLDOMElement*& pFatherNode) const;
	void               SerializeFullXml(CXMLDOMElement* pActionNode, DWORD ObjToken);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	CSerializeObject*  Clone() { return new CCommConfDB; }

	//const char*        GetName() const;

	void               SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken);

	WORD               GetConfNumber() const;
	void               SetConfNumber(const WORD resNum);
	void               SetT1CasConfNumber(const WORD confNum);
	WORD               GetT1CasConfNumber();

	virtual int        Add(CCommConf& other);
	int                Update(CCommConf& other);
	virtual int        Cancel(const ConfMonitorID confId);
	virtual int        Cancel(const char* confName);
	virtual int        CancelRamOnly(const ConfMonitorID confId);
	int                FindId(const ConfMonitorID confId);
	int                FindId(const CCommConf& other);
	int                FindName(const char* name, BYTE is_display_name = 0);
	int                FindName(const CCommConf& other);
	int                SearchPartyName(const char* confName, const char* partyName);
	int                SearchPartyName(const ConfMonitorID confId, const char* partyName);
	int                SearchPartyName(const ConfMonitorID confId, const PartyMonitorID partyId);
	int                SearchPartyVisualName(const ConfMonitorID confId, const char* partyVisualName);
	int                SearchPartyVisualName(const char* confName, const char* partyVisualName);
	int                SearchPartyVisualNameByPartyId(const char* confName, const char* partyVisualName, const PartyMonitorID partyId); //BRIDGE-6030
	int                SearchPartyNameInAllConferences(const char* partyName);
	int                SearchPartyVisualNameInAllConferences(const char* partyVisualName, const PartyMonitorID partyId = 0xFFFF);        //BRIDGE-7248 added const DWORD partyId = 0xFFFF
	void               ResetAdHocProfileId(const DWORD profileId);
	CCommConf*         GetCurrentConf(const char* confName) const;
	CCommConf*         GetCurrentConf(const ConfMonitorID confId) const;
	CCommConf*         GetCurrentConfByNameOrByNumericId(const char* confName) const;
	CCommConf*         GetFirstCommConf();
	CCommConf*         GetNextCommConf();
	CCommConf*         GetFirstCommConf(int& nPos);
	CCommConf*         GetNextCommConf(int& nPos);
	CCommConf*         GetCurrentOnGoingEQ(const char* name_char) const;
	BOOL               IsNumericIDExistInEntryQueueConf(const char* confNumericId) const;
	int                GetConfParty(const char* calledPhoneNumber, CCommConf** pConf);
	ConfMonitorID      GetConfId(const char* confName) const;
	const char*        GetConName(const ConfMonitorID confId) const;
	PartyMonitorID     GetPartyId(const char* confName, const char* partyName) const;
	PartyMonitorID     GetPartyId(const ConfMonitorID confId, const char* partyName) const;
	const char*        GetRecordingLinkPartyName(const ConfMonitorID confId) const;
	PartyMonitorID     GetRecordingLinkPartyId(const ConfMonitorID confId) const;

	const char*        GetPartyName(const ConfMonitorID confId, const PartyMonitorID partyId) const;
	const char*        GetPartyName(const char* confName, const PartyMonitorID partyId) const;
	const CConfParty*  GetCurrentParty(const ConfMonitorID confId, const PartyMonitorID partyId);

	DWORD              GetLastTerminatedConfId()                     { return m_lastTerminatedConfId; }
	void               SetLastTerminatedConfId(ConfMonitorID confId) { m_lastTerminatedConfId = confId; }

	WORD               GetLastTerminatedConfReason();
	void               SetLastTerminatedConfReason(WORD reason);

	void               IncreaseConferenceCounter();
	DWORD              GetConferenceCounter();

	DWORD              GetSummaryUpdateCounter() const;
	void               IncreaseSummaryUpdateCounter();
	DWORD              GetFullUpdateCounter() const;
	void               IncreaseFullUpdateCounter();

	BYTE               GetChanged() const;
	DWORD              GetDeletedHistoryId(int index);

	int                SearchForH323DefinedMatch(mcTransportAddress partyIPaddress, CH323Alias* pSrcH323AliasArray, WORD wSrcNumAlias, CCommConf** pConf, CConfParty** pParty, CH323Alias* pDestH323AliasArray, WORD wDestNumAlias );
	int                SearchForH323ConferenceMatch( const mcTransportAddress* pPartyIPaddress, CH323Alias* pSrcH323AliasArray, WORD wSrcNumAlias, CCommConf** pConf, CConfParty** pParty, CH323Alias* pDestH323AliasArray,
	                                                 WORD wDestNumAlias, WORD useTransitEQ = FALSE, BYTE isH323 = TRUE);

	BOOL               IsMeetingRoomSpecified(CH323Alias* pDestH323AliasArray, WORD wDestNumAlias, BYTE isH323);

	BYTE               IsConfSecured(ConfMonitorID confId);
	BYTE               IsConfLocked(ConfMonitorID confId);
	BYTE               IsLPRConf(ConfMonitorID confId);
	BYTE               GetConnectionType(ConfMonitorID confId, PartyMonitorID partyId);
	const char*        GetConfByMsConversationId(char* msConvId) const;

	int                GetConfFromPartyName(const char* pPartyName, CCommConf*& pResulfConf, WORD& num_founds, WORD start_index = 0);
	void               UpdateParticipantsNum(); // for Sinai
	BYTE               SearchOperatorParty(COperatorConfInfo& partyOperatorConfInfo, CConfParty*& pFoundParty, CCommConf*& pFoundInConf);
	void               DeleteDB();

	CTaskApp*          GetPartyTask(const char* tag);
	DWORD              GetPartyIDByTag(const char* tag);
	DWORD              GetConfIDByTag(const char* tag);

protected:
	// Attributes added for SNMP H.341 MC-MIB
	ConfMonitorID      m_lastTerminatedConfId;    // ID of most recently terminated conference
	DWORD m_lastTerminationReason;   // (1) End time over, (2) Operator Request
	DWORD m_TotalConferenceCounter;  // Counts all confs since system startup

	DWORD m_DeletedIdHistory[1000];
	DWORD m_DeletedCounterHistory[1000];
	DWORD m_LastDeletedIndex;
	BYTE  m_bChanged;

	// Attributes
	WORD       m_index;
	WORD       m_numb_of_conf;      // number of conferences
	CCommConf* m_pConf[MAX_CONF_IN_LIST];

	DWORD m_dwSummaryUpdateCounter; // counts changes for operator updates
	DWORD m_dwFullUpdateCounter;
};

#endif // !defined(_CommConfDB_H__)

