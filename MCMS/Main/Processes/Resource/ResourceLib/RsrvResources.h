#if !defined(AFX_RSRVESOURCES_H__DA95D6C6_46D3_4708_ADDB_CB6AB72DC5A6__INCLUDED_)
#define AFX_RSRVESOURCES_H__DA95D6C6_46D3_4708_ADDB_CB6AB72DC5A6__INCLUDED_

#include <bitset>
#include <set>
#include "PObject.h"
#include "Trace.h"
#include "DataTypes.h"
#include "Macros.h"
#include "StatusesGeneral.h"
#include "AllocateStructs.h"
#include "SystemResources.h"
#include "InnerStructs.h"

class CCommResApi;

////////////////////////////////////////////////////////////////////////////
//                        CIntervalRsrvAmount
////////////////////////////////////////////////////////////////////////////
class CIntervalRsrvAmount : public CPObject
{
	CLASS_TYPE_1(CInterval, CPObject)

public:
	                    CIntervalRsrvAmount();
	virtual            ~CIntervalRsrvAmount();
	virtual const char* NameOf() const { return "CIntervalRsrvAmount"; }

	void                Init();

	void                SetNID(const char* pNID);
	char*               GetNID() {return m_NID;}

	BOOL                m_bIsRealConference; // is it a real conference, or just a meeting room
	BOOL                m_VSW_AUTO;

	STATUS              AddServicePhone(const CServicePhoneStr& other);
	CServicePhoneStr*   GetServicePhone() const;

	CPartiesResources   m_PartiesResources;

private:
	char                m_NID[NUMERIC_CONF_ID_MAX_LEN];

	CServicePhoneStr*   m_pServicePhoneStr;
};


////////////////////////////////////////////////////////////////////////////
//                        CInterval
////////////////////////////////////////////////////////////////////////////
class CInterval : public CPObject
{
	CLASS_TYPE_1(CInterval, CPObject)

public:
	                    CInterval(DWORD interval, CPartiesResources& totalPartiesResources, WORD total_conferences, WORD total_auto_vsw);
	virtual            ~CInterval();
	virtual const char* NameOf() const { return "CInterval"; }

	STATUS              Reserve(CIntervalRsrvAmount& amount);
	STATUS              UnReserve(CIntervalRsrvAmount& amount);

	friend bool         operator==(const CInterval& lhs, const CInterval& rhs);
	friend bool         operator<(const CInterval& lhs, const CInterval& rhs);

	DWORD               m_interval;

	CPartiesResources   m_totalPartiesResources;

	WORD                m_total_conferences;
	WORD                m_total_auto_vsw_conf;

private:
	CInterval();
};


////////////////////////////////////////////////////////////////////////////
//                        CConfRsrvRsrc
////////////////////////////////////////////////////////////////////////////
class CConfRsrvRsrc : public CPObject
{
	CLASS_TYPE_1(CConfRsrvRsrc, CPObject)

public:
	                    CConfRsrvRsrc(CCommResApi* pCommResApi, BOOL isFromPassive = FALSE);
	                    CConfRsrvRsrc(const CConfRsrvRsrc& other);
	virtual            ~CConfRsrvRsrc();
	const char*         NameOf() const { return "CConfRsrvRsrc"; }

	friend WORD         operator==(const CConfRsrvRsrc&, const CConfRsrvRsrc&);
	friend bool         operator<(const CConfRsrvRsrc&, const CConfRsrvRsrc&);

	DWORD               GetMonitorConfId() const               { return m_monitorConfId; }
	void                SetMonitorConfId(DWORD monitorConfId)  { m_monitorConfId = monitorConfId; }
	STATUS              GetStatus() const                      { return m_ResStatus; }
	void                SetStatus(STATUS status)               { m_ResStatus = status; }
	DWORD               GetRepeatedConfId() const              { return m_repeatedConfId; }
	void                SetRepeatedConfId(DWORD repeatedConfId){ m_repeatedConfId = repeatedConfId; }
	const char*         GetNID() const                         { return m_NID; }
	void                SetNID(const char* pNID);
	WORD                GetMinNumVideoParties() const          { return m_minNumVideoPartiesInConf; }
	DWORD               GetStartInterval() const               { return m_startInterval; }
	void                SetStartInterval(DWORD startInterval)  { m_startInterval = startInterval; }

	DWORD               GetEndInterval() const                 { return m_endInterval; }
	void                SetEndInterval(DWORD endInterval)      { m_endInterval = endInterval; }

	BOOL                GetFromPassive() const                 { return m_isFromPassive; }
	void                SetFromPassive(BOOL isFromPassive)     { m_isFromPassive = isFromPassive; }

	BOOL                IsPermanent() const                    { return m_isPermanent; }
	void                SetPermanent(BOOL isPermanent)         { m_isPermanent = isPermanent; }

	BOOL                IsAutoVSW() const                      { return m_isVSW_AUTO; }

	// reservation related functionality
	void                GetConferenceNeededAmount(CIntervalRsrvAmount& confNeededAmount, BOOL bCountPartyResourcesOnlyIfStatusOK = FALSE);

	STATUS              FillFieldsNeededForReservator(CCommResApi* pCommResApi);
	STATUS              CalculateReservationPartyResources();

	STATUS              AddServicePhone(const CServicePhoneStr& other);
	STATUS              DeleteServicePhone(const CServicePhoneStr& other);
	void                AddServicePhoneNumber(const char* netServiceName, const char* phoneNumber);
	int                 FindServicePhone(const CServicePhoneStr& other);
	CServicePhoneStr*   GetFirstServicePhone();
	CServicePhoneStr*   GetNextServicePhone();

	WORD                GetLogicalWeightOfMinNumVideoParties() const;
	DWORD               GetIpServiceIdForMinParties() const {return m_ipServiceIdForMinParties;}

private:
	char                m_NID[NUMERIC_CONF_ID_MAX_LEN];
	STATUS              m_ResStatus;
	DWORD               m_monitorConfId;
	DWORD               m_repeatedConfId;
	WORD                m_minNumAudioPartiesInConf;
	WORD                m_minNumVideoPartiesInConf;
	eVideoPartyType     m_maxVideoPartyTypeInConf;
	CPartiesResources   m_PartiesResources;
	DWORD               m_startInterval;
	DWORD               m_endInterval;
	BOOL                m_isFromPassive;
	BOOL                m_isPermanent;
	BOOL                m_isVSW_AUTO;
	CServicePhoneStr*   m_pServicePhoneStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
	WORD                m_numServicePhoneStr;
	WORD                m_ind_service_phone;
	WORD                m_ipServiceIdForMinParties;
};

#endif // !defined(AFX_SYSTEMRESOURCES_H__DA95D6C6_46D3_4708_ADDB_CB6AB72DC5A6__INCLUDED_)

