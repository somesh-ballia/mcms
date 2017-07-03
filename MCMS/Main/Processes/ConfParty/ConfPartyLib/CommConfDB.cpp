// CommConf.cpp: implementation of the CCommConf class.
////////////////////////////////////////////////////////////////////////
//Revisions and Updates:
//
//Date         Updated By         Description
//
//3/6/05		Yoella				Porting to Carmel
//========   ==============   =====================================================================

#include <string.h>
#include "NStream.h"
#include "psosxml.h"
#include "CommConfDB.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "ConfPartyStatuses.h"
#include "IpServiceListManager.h"
#include "SysConfigKeys.h"
#include "OperatorConfInfo.h"

extern CIpServiceListManager* GetIpServiceListMngr();

////////////////////////////////////////////////////////////////////////////
//                        CCommConfDB
////////////////////////////////////////////////////////////////////////////
CCommConfDB::CCommConfDB()
{
	m_index        = 0;
	m_numb_of_conf = 0;
	int i;
	for (i = 0; i < MAX_CONF_IN_LIST; i++)
		m_pConf[i] = NULL;

	m_dwSummaryUpdateCounter = 0;
	m_dwFullUpdateCounter    = 0;
	m_lastTerminatedConfId   = 0;  // ID of most recently terminated conference
	m_lastTerminationReason  = 1;  // (1) End time over, (2) Operator Request
	m_TotalConferenceCounter = 0;
	for (i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = 0;
		m_DeletedIdHistory[i]      = 0;
	}

	m_LastDeletedIndex = 0;
	m_bChanged         = FALSE;
}

//--------------------------------------------------------------------------
CCommConfDB::CCommConfDB(const CCommConfDB& other)
	: CSerializeObject(other)
{
	m_numb_of_conf = other.m_numb_of_conf;
	m_index        = other.m_index;

	int i;
	for (i = 0; i < MAX_CONF_IN_LIST; i++)
	{
		if (other.m_pConf[i] == NULL)
			m_pConf[i] = NULL;
		else
			m_pConf[i] = new CCommConf(*other.m_pConf[i]);
	}

	m_dwSummaryUpdateCounter = other.m_dwSummaryUpdateCounter;
	m_dwFullUpdateCounter    = other.m_dwFullUpdateCounter;
	m_LastDeletedIndex       = other.m_LastDeletedIndex;
	for (i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = other.m_DeletedCounterHistory[i];
		m_DeletedIdHistory[i]      = other.m_DeletedIdHistory[i];
	}

	m_lastTerminatedConfId   = other.m_lastTerminatedConfId;  // ID of most recently terminated conference
	m_lastTerminationReason  = other.m_lastTerminationReason; // (1) End time over, (2) Operator Request
	m_TotalConferenceCounter = other.m_TotalConferenceCounter;
	m_bChanged               = other.m_bChanged;
}

//--------------------------------------------------------------------------
CCommConfDB& CCommConfDB::operator =(const CCommConfDB& other)
{
	int i;
	for (i = 0; i < MAX_CONF_IN_LIST; i++)
		POBJDELETE(m_pConf[i]);

	m_numb_of_conf = other.m_numb_of_conf;
	m_index        = other.m_index;

	for (i = 0; i < MAX_CONF_IN_LIST; i++)
	{
		if (other.m_pConf[i] == NULL)
			m_pConf[i] = NULL;
		else
			m_pConf[i] = new CCommConf(*other.m_pConf[i]);
	}

	m_dwSummaryUpdateCounter = other.m_dwSummaryUpdateCounter;
	m_dwFullUpdateCounter    = other.m_dwFullUpdateCounter;
	m_LastDeletedIndex       = other.m_LastDeletedIndex;
	for (i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = other.m_DeletedCounterHistory[i];
		m_DeletedIdHistory[i]      = other.m_DeletedIdHistory[i];
	}

	m_lastTerminatedConfId   = other.m_lastTerminatedConfId;
	m_lastTerminationReason  = other.m_lastTerminationReason;
	m_TotalConferenceCounter = other.m_TotalConferenceCounter;
	m_bChanged               = other.m_bChanged;

	return *this;
}

//--------------------------------------------------------------------------
CCommConfDB::~CCommConfDB()
{
	for (int i = 0; i < MAX_CONF_IN_LIST; i++)
		POBJDELETE(m_pConf[i]);
}

//--------------------------------------------------------------------------
void CCommConfDB::SerializeXml(CXMLDOMElement*& pActionNode) const
{
}

//--------------------------------------------------------------------------
void CCommConfDB::SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken)
{
	unsigned char   bChanged = FALSE;
	int             opcode;

	DWORD           ResSummeryCounter = 0xFFFFFFFF;
	CXMLDOMElement* pSumListNode;
	WORD            bSerialize = FALSE;
	// int showT1CasConf = ::GetpSystemCfg()->GetShowT1CasConf();

	pSumListNode = pActionNode->AddChildNode("CONF_SUMMARY_LS");
	pSumListNode->AddChildNode("OBJ_TOKEN", m_dwSummaryUpdateCounter);

	if (ObjToken == 0xFFFFFFFF)
		bChanged = TRUE;
	else  // to see how to get the m_updatecounter from skeleton - Request object
	{
		// in case the string can not be converted ResSummeryCounter=0
		// the conferences will be added as if the user sent -1 in the object token .
		ResSummeryCounter = ObjToken;
		if (m_dwSummaryUpdateCounter > ResSummeryCounter)
			bChanged = TRUE;
	}

	if (bChanged == TRUE)
	{
		pSumListNode->AddChildNode("CHANGED", TRUE, _BOOL);
		int i;
		for (i = 0; i < (int)m_numb_of_conf; i++)
		{
			if ((int)m_pConf[i]->GetSummaryCreationUpdateCounter() > (int)ResSummeryCounter)
				opcode = CONF_COMPLETE_INFO;
			else if ((int)m_pConf[i]->GetSummaryUpdateCounter() > (int)ResSummeryCounter)
				opcode = CONF_FAST_PLUS_SLOW_INFO;
			else
				opcode = CONF_NOT_CHANGED;

			m_pConf[i]->SerializeShortXml(pSumListNode, opcode);
		}

		CXMLDOMElement* pDeletedNode = pSumListNode->AddChildNode("DELETED_CONF_LIST");
		for (i = 0; i < 1000; i++)
		{
			if (m_DeletedCounterHistory[i] > ResSummeryCounter && ResSummeryCounter != 0xFFFFFFFF)
				pDeletedNode->AddChildNode("ID", m_DeletedIdHistory[i]);
		}
	}
	else
		pSumListNode->AddChildNode("CHANGED", FALSE, _BOOL);
}

// Hot Backup Yoella - Send IDs of conferences that made any change in amy field not just in SUMMERY fields
//--------------------------------------------------------------------------
void CCommConfDB::SerializeFullXml(CXMLDOMElement* pActionNode, DWORD ObjToken)
{
	unsigned char   bChanged = FALSE;
	int             opcode;

	DWORD           RequestFullCounter = 0xFFFFFFFF;
	CXMLDOMElement* pSumListNode;
	CXMLDOMElement* pChangedNode;

	pSumListNode = pActionNode->AddChildNode("FULL_CHANGE_LS");
	pSumListNode->AddChildNode("OBJ_TOKEN", m_dwFullUpdateCounter);


	if (ObjToken == 0xFFFFFFFF)
		bChanged = TRUE;
	else
	{
		RequestFullCounter = ObjToken;
		if (m_dwFullUpdateCounter > RequestFullCounter)
			bChanged = TRUE;
	}

	if (bChanged == TRUE)
	{
		pSumListNode->AddChildNode("CHANGED", TRUE, _BOOL);
		pChangedNode = pSumListNode->AddChildNode("CHANGED_LIST");

		int i;
		for (i = 0; i < (int)m_numb_of_conf; i++)
		{
			if ((int)m_pConf[i]->GetFullUpdateCounter() > (int)RequestFullCounter)
				pChangedNode->AddChildNode("ID", m_pConf[i]->GetMonitorConfId());
		}

		CXMLDOMElement* pDeletedNode = pSumListNode->AddChildNode("DELETED_LIST");
		for (i = 0; i < 1000; i++)
		{
			if ((m_DeletedCounterHistory[i] > RequestFullCounter) && (RequestFullCounter != 0xFFFFFFFF))
				pDeletedNode->AddChildNode("ID", m_DeletedIdHistory[i]);
		}
	}

	else
		pSumListNode->AddChildNode("CHANGED", FALSE, _BOOL);
}

//--------------------------------------------------------------------------//////////
int CCommConfDB::DeSerializeXml(CXMLDOMElement* pListNode, char* pszError, const char* action)
{
	int nStatus, nIndex = 0, i;
	m_bChanged = TRUE;

	GET_VALIDATE_CHILD(pListNode, "OBJ_TOKEN", &m_dwSummaryUpdateCounter, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pListNode, "CHANGED", &m_bChanged, _BOOL);

	if (m_bChanged)
	{
		CXMLDOMElement* pDelListNode, * pSummaryNode;

		for (i = 0; i < m_numb_of_conf; i++)
		{
			if (m_pConf[i])
				delete m_pConf[i];
		}

		m_numb_of_conf = 0;

		GET_FIRST_CHILD_NODE(pListNode, "CONF_SUMMARY", pSummaryNode);

		while (pSummaryNode && m_numb_of_conf < MAX_CONF_IN_LIST)
		{
			m_pConf[m_numb_of_conf] = new CCommConf;
			nStatus                 = m_pConf[m_numb_of_conf]->DeSerializeShortXml(pSummaryNode, pszError);

			if (nStatus != STATUS_OK)
			{
				POBJDELETE(m_pConf[m_numb_of_conf]);
				return nStatus;
			}

			m_numb_of_conf++;

			GET_NEXT_CHILD_NODE(pListNode, "CONF_SUMMARY", pSummaryNode);
		}

		GET_CHILD_NODE(pListNode, "DELETED_CONF_LIST", pDelListNode);

		if (pDelListNode)
		{
			CXMLDOMElement* pDelIDNode;

			for (i = 0; i < 1000; i++)
				m_DeletedIdHistory[i] = 0;

			GET_FIRST_CHILD_NODE(pDelListNode, "ID", pDelIDNode);
			nIndex = 0;
			while (pDelIDNode)
			{
				GET_VALIDATE(pDelIDNode, &(m_DeletedIdHistory[nIndex]), _0_TO_DWORD);

				GET_NEXT_CHILD_NODE(pDelListNode, "ID", pDelIDNode);
				nIndex++;
			}
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------////////
DWORD CCommConfDB::GetDeletedHistoryId(int index)
{
	return m_DeletedIdHistory[index];
}

//--------------------------------------------------------------------------
int CCommConfDB::Add(CCommConf& otherConf)
{
	int status = STATUS_OK;

	if (m_numb_of_conf >= MAX_CONF_IN_LIST)
		return STATUS_MAX_RESERVATIONS_EXCEEDED;

	if (FindName(otherConf) != NOT_FIND)
		return STATUS_CONF_NAME_EXISTS;

	m_pConf[m_numb_of_conf] = &otherConf;
	m_pConf[m_numb_of_conf]->SetConfFlags(m_pConf[m_numb_of_conf]->CalcRsrvFlags());
	m_pConf[m_numb_of_conf]->SetConfFlags2(m_pConf[m_numb_of_conf]->CalcRsrvFlags2());
	IncreaseSummaryUpdateCounter();
	IncreaseFullUpdateCounter();
	m_pConf[m_numb_of_conf]->SetSummeryCreationUpdateCounter();


	SetConfNumber(m_numb_of_conf+1);

	IncreaseConferenceCounter();

	return status;
}

//--------------------------------------------------------------------------
int CCommConfDB::Update(CCommConf& otherConf)
{
	int ind;
	ind = FindName(otherConf);
	if (ind == NOT_FIND)
		return STATUS_CONF_NOT_EXISTS;

	// Romem klocwork
	if (ind >= MAX_CONF_IN_LIST)
	{
		return STATUS_CONF_NOT_EXISTS;
	}

	POBJDELETE(m_pConf[ind]);

	m_pConf[ind] = &otherConf;

	IncreaseSummaryUpdateCounter();
	IncreaseFullUpdateCounter();

	int numbPart = m_pConf[ind]->GetNumParties();

	for (int j = 0; j < numbPart; j++)
	{
		CConfParty* confParty;
		if (0 == j)
			confParty = m_pConf[ind]->GetFirstParty();
		else
			confParty = m_pConf[ind]->GetNextParty();

		if (confParty != NULL)
			confParty->SetCreationUpdateCounter();
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommConfDB::Cancel(const ConfMonitorID confId)
{
	int ind = FindId(confId);
	if (ind == NOT_FIND) return STATUS_CONF_NOT_EXISTS;

	// Romem klocwork
	if (ind >= MAX_CONF_IN_LIST)
	{
		return STATUS_CONF_NOT_EXISTS;
	}

	if (m_numb_of_conf > MAX_CONF_IN_LIST)
		return STATUS_MAX_RESERVATIONS_EXCEEDED;

	WORD flag;
	flag = m_pConf[ind]->Is_TerminatingState();

	int isT1CasConf = 0;
	if (0 == strncmp(m_pConf[ind]->GetName(), "##T1CAS$$", 9))
		isT1CasConf = 1;

	POBJDELETE(m_pConf[ind]);

	int i;
	for (i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] == NULL)
			break;
	}

	for (int j = i; j < (int)m_numb_of_conf-1; j++)
	{
		m_pConf[j] = m_pConf[j+1];
	}

	m_pConf[m_numb_of_conf-1] = NULL;
	SetConfNumber(m_numb_of_conf-1);

	if (isT1CasConf)
	{
		int tCount = GetT1CasConfNumber();
		if (tCount > 0)
			SetT1CasConfNumber(tCount - 1);
	}

	int status = STATUS_OK;

	IncreaseSummaryUpdateCounter();
	IncreaseFullUpdateCounter();
	SetLastTerminatedConfId(confId);

	if (m_LastDeletedIndex >= 1000)
		m_LastDeletedIndex = 0;

	m_DeletedIdHistory[m_LastDeletedIndex] = confId;

	// Eitan V7 - Hot backup is not supported, this change cause the deleted conf list not to be updated ( we always send the full list) --> causes performence issues on DMA
	// For V7.1 need to think what is the best solution
	// From V4.6: MAX(m_dwFullUpdateCounter,m_dwSummaryUpdateCounter); //  YOELLA HotBackUp to send the deleted confs in GET_FULL_CONF_CHANGE_LS ,changed from m_dwSummaryUpdateCounter;based on the fact that m_dwFullUpdateCounter>m_dwSummaryUpdateCounter

	// VNGR-20464 - V7.2 MLA did NOT see DELETED conferences (May cause HotBackup problem on delete.
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (pProcess && pProcess->GetIsFailoverFeatureEnabled())                                              // if (HotBackUp)
		m_DeletedCounterHistory[m_LastDeletedIndex] = MAX(m_dwFullUpdateCounter, m_dwSummaryUpdateCounter); // YOELLA HotBackUp to send the deleted confs in GET_FULL_CONF_CHANGE_LS ,changed from m_dwSummaryUpdateCounter;based on the fact that m_dwFullUpdateCounter>m_dwSummaryUpdateCounter
	else                                                                                                  // MLA DMA CMA (all except HotBackup)
		m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwSummaryUpdateCounter;

	m_LastDeletedIndex++;

	return status;
}

//--------------------------------------------------------------------------
int CCommConfDB::Cancel(const char* confName)
{
	int ind = FindName(confName);
	if (ind == NOT_FIND) return STATUS_CONF_NOT_EXISTS;

	// Romem klocwork
	if (ind >= MAX_CONF_IN_LIST)
		return STATUS_CONF_NOT_EXISTS;

	if (m_numb_of_conf > MAX_CONF_IN_LIST)
		return STATUS_MAX_RESERVATIONS_EXCEEDED;

	WORD flag = m_pConf[ind]->Is_TerminatingState();

// CARMEL   WORD is_GW   = (m_pConf[ind]->GetisGateway());
	DWORD id = (m_pConf[ind]->GetMonitorConfId());

	int isT1CasConf = 0;
	if (0 == strncmp(m_pConf[ind]->GetName(), "##T1CAS$$", 9))
		isT1CasConf = 1;

	POBJDELETE(m_pConf[ind]);

	int i;
	for (i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] == NULL)
			break;
	}

	for (int j = i; j < (int)m_numb_of_conf-1; j++)
	{
		m_pConf[j] = m_pConf[j+1];
	}

	m_pConf[m_numb_of_conf-1] = NULL;
	SetConfNumber(m_numb_of_conf-1);
	if (isT1CasConf)
	{
		int tCount = GetT1CasConfNumber();
		if (tCount > 0)
			SetT1CasConfNumber(tCount - 1);
	}

	int status = STATUS_OK;

	IncreaseSummaryUpdateCounter();
	IncreaseFullUpdateCounter();
	SetLastTerminatedConfId(id);

	if (m_LastDeletedIndex >= 1000)
		m_LastDeletedIndex = 0;

	m_DeletedIdHistory[m_LastDeletedIndex]      = id;
	m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwSummaryUpdateCounter;
	m_LastDeletedIndex++;

	return status;
}
//--------------------------------------------------------------------------
int CCommConfDB::CancelRamOnly(const ConfMonitorID confId)
{
	int conf_index = FindId(confId);
	if (conf_index == NOT_FIND) return STATUS_CONF_NOT_EXISTS;

	// Romem klocwork
	if (conf_index >= MAX_CONF_IN_LIST)
		return STATUS_CONF_NOT_EXISTS;

	if (m_numb_of_conf > MAX_CONF_IN_LIST)
		return STATUS_MAX_RESERVATIONS_EXCEEDED;

	int isT1CasConf = 0;
	if (0 == strncmp(m_pConf[conf_index]->GetName(), "##T1CAS$$", 9))
		isT1CasConf = 1;

	POBJDELETE(m_pConf[conf_index]);

	for (int j = conf_index; j < (int)m_numb_of_conf-1; j++)
	{
		m_pConf[j] = m_pConf[j+1];
	}

	m_pConf[m_numb_of_conf-1] = NULL;
	SetConfNumber(m_numb_of_conf-1);

	if (isT1CasConf)   // for T1-CAS only
	{
		int count = 0;
		for (int n = 0; n < (int)m_numb_of_conf; n++)
			if (m_pConf[n])
				if (m_pConf[n]->GetName())
					if (0 == strncmp(m_pConf[n]->GetName(), "##T1CAS$$", 9))
						count++;

		SetT1CasConfNumber(count);
	}

	IncreaseSummaryUpdateCounter();
	IncreaseFullUpdateCounter();
	SetLastTerminatedConfId(confId);

	if (m_LastDeletedIndex >= 1000)
		m_LastDeletedIndex = 0;

	m_DeletedIdHistory[m_LastDeletedIndex]      = confId;
	m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwSummaryUpdateCounter;
	m_LastDeletedIndex++;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommConfDB::FindId(const ConfMonitorID confId)
{
	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (m_pConf[i]->GetMonitorConfId() == confId)
				return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCommConfDB::FindId(const CCommConf& otherConf)
{
	DWORD id  = otherConf.GetMonitorConfId();
	int   ret = FindId(id);
	return ret;
}

//--------------------------------------------------------------------------
int CCommConfDB::FindName(const char* name, BYTE is_display_name)
{
	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (is_display_name)
			{
				if (!strcmp(m_pConf[i]->GetDisplayName(), name))
					return i;
			}
			else
			{
				if (!strcmp(m_pConf[i]->GetName(), name))
					return i;
			}
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCommConfDB::FindName(const CCommConf& otherConf)
{
	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (!strcmp(m_pConf[i]->GetName(), otherConf.GetName()))
				return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyName(const char* confName, const char* partyName)
{
	CCommConf* pCommConf = GetCurrentConf(confName);
	if (!pCommConf)
		return STATUS_CONF_NOT_EXISTS;

	for (int i = 0; i < (int)pCommConf->GetNumParties(); ++i)
	{
		CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

		if (pConfParty && !strcmp(pConfParty->GetName(), partyName))
			return STATUS_OK;
	}

	return STATUS_PARTY_DOES_NOT_EXIST;
}

//--------------------------------------------------------------------------
BYTE CCommConfDB::SearchOperatorParty(COperatorConfInfo& partyOperatorConfInfo, CConfParty*& pFoundParty, CCommConf*& pFoundInConf)
{
	for (DWORD conf_index = 0; conf_index < (int)m_numb_of_conf; conf_index++)
	{
		if (m_pConf[conf_index] != NULL)
		{
			for (DWORD party_index = 0; party_index < (int)m_pConf[conf_index]->GetNumParties(); party_index++)
			{
				CConfParty* pCurrentParty = (party_index == 0) ? m_pConf[conf_index]->GetFirstParty() : m_pConf[conf_index]->GetNextParty();
				// klocwork Romem
				if (pCurrentParty)
				{
					COperatorConfInfo* pCurrentPartyOperatorConfInfo = pCurrentParty->GetOperatorConfInfo();
					if (pCurrentPartyOperatorConfInfo != NULL)
					{
						if (*pCurrentPartyOperatorConfInfo == partyOperatorConfInfo)
						{
							pFoundParty  = pCurrentParty;
							pFoundInConf = m_pConf[conf_index];
							return TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyName(const ConfMonitorID confId, const char* partyName)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (!pCommConf)
		return STATUS_CONF_NOT_EXISTS;

	for (int i = 0; i < (int)pCommConf->GetNumParties(); ++i)
	{
		CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

		if (pConfParty && !strcmp(pConfParty->GetName(), partyName))
			return STATUS_OK;
	}

	return STATUS_PARTY_DOES_NOT_EXIST;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyName(const ConfMonitorID confId, const PartyMonitorID partyId)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (!pCommConf)
		return STATUS_CONF_NOT_EXISTS;

	for (int i = 0; i < (int)pCommConf->GetNumParties(); ++i)
	{
		CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

		if (pConfParty && pConfParty->GetPartyId() == partyId)
				return STATUS_OK;
	}

	return STATUS_PARTY_DOES_NOT_EXIST;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyVisualName(const ConfMonitorID confId, const char* partyVisualName)
{
	if (!partyVisualName)
	{
		PTRACE(eLevelError, "CCommConfDB::SearchPartyVisualName. PartyVisualName is NULL pointer");
		return STATUS_OK; // The returned value STATUS_OK means visual name was found. In order to prevent set visual name after the search.
	}

	CCommConf* pCommConf = GetCurrentConf(confId);
	if (!pCommConf)
		return STATUS_CONF_NOT_EXISTS;

	for (int i = 0; i < (int)pCommConf->GetNumParties(); ++i)
	{
		CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

		if (pConfParty && !strcmp((pConfParty->GetVisualPartyName()), partyVisualName))
			return STATUS_OK;
	}

	return STATUS_PARTY_VISUAL_NAME_NOT_EXISTS;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyVisualName(const char* confName, const char* partyVisualName)
{
	if (!partyVisualName)
	{
		PTRACE(eLevelError, "CCommConfDB::SearchPartyVisualName. PartyVisualName is NULL pointer");
		return STATUS_OK; // The returned value STATUS_OK means visual name was found. In order to prevent set visual name after the search.
	}

	CCommConf* pCommConf = GetCurrentConf(confName);
	if (!pCommConf)
		return STATUS_CONF_NOT_EXISTS;

	for (int i = 0; i < (int)pCommConf->GetNumParties(); ++i)
	{
		CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

		if (pConfParty && !strcmp(pConfParty->GetVisualPartyName(), partyVisualName))
			return STATUS_OK;
	}

	return STATUS_PARTY_VISUAL_NAME_NOT_EXISTS;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyVisualNameByPartyId(const char* confName, const char* partyVisualName, const PartyMonitorID partyId)//N.A. BRIDGE-6030
{
	if (!partyVisualName)
	{
		PTRACE(eLevelError, "CCommConfDB::SearchPartyVisualName. PartyVisualName is NULL pointer");
		return STATUS_OK; // The returned value STATUS_OK means visual name was found. In order to prevent set visual name after the search.
	}

	CCommConf* pCommConf = GetCurrentConf(confName);
	if (!pCommConf)
		return STATUS_CONF_NOT_EXISTS;

	for (int i = 0; i < (int)pCommConf->GetNumParties(); i++)
	{
		CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

		if (pConfParty && !strcmp(pConfParty->GetVisualPartyName(), partyVisualName))
		{
			if (partyId != pConfParty->GetPartyId())
				return STATUS_OK;
		}
	}

	return STATUS_PARTY_VISUAL_NAME_NOT_EXISTS;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyNameInAllConferences(const char* partyName)
{
	for (int j = 0; j < (int)m_numb_of_conf; j++)
	{
		CCommConf* pCommConf = m_pConf[j];
		if (pCommConf)
		{
			for (int i = 0; i < (int)pCommConf->GetNumParties(); ++i)
			{
				CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

				if (pConfParty && !strcmp(pConfParty->GetName(), partyName))
					return STATUS_OK;
			}
		}
	}

	return STATUS_PARTY_DOES_NOT_EXIST;
}

//--------------------------------------------------------------------------
int CCommConfDB::SearchPartyVisualNameInAllConferences(const char* partyVisualName, const PartyMonitorID partyId)
{
	PTRACE(eLevelInfoNormal,"CCommConfDB::SearchPartyVisualNameInAllConferences");

	for (int j = 0; j < (int)m_numb_of_conf; j++)
	{
		CCommConf* pCommConf = m_pConf[j];
		if (pCommConf)
		{
			for (int i = 0; i < (int)m_pConf[j]->GetNumParties(); ++i)
			{
				CConfParty* pConfParty = (!i) ? pCommConf->GetFirstParty() : pCommConf->GetNextParty();

				if (pConfParty && !strcmp(pConfParty->GetVisualPartyName(), partyVisualName))
				{
					if (partyId == 0xFFFF || partyId != pConfParty->GetPartyId())
						return STATUS_OK;
					else
						return STATUS_PARTY_VISUAL_NAME_NOT_EXISTS;
				}
			}
		}
	}

	return STATUS_PARTY_VISUAL_NAME_NOT_EXISTS;
}

//--------------------------------------------------------------------------
WORD CCommConfDB::GetConfNumber() const
{
	return m_numb_of_conf;
}


//--------------------------------------------------------------------------
BYTE CCommConfDB::GetChanged() const
{
	return m_bChanged;
}

//--------------------------------------------------------------------------
void CCommConfDB::SetConfNumber(const WORD confNum)
{
	m_numb_of_conf = confNum;
}

//--------------------------------------------------------------------------
void CCommConfDB::SetT1CasConfNumber(const WORD confNum)
{
}

//--------------------------------------------------------------------------
WORD CCommConfDB::GetT1CasConfNumber()
{
	WORD t1CasConfs = 0;

	return t1CasConfs;
}

//--------------------------------------------------------------------------
void CCommConfDB::ResetAdHocProfileId(const DWORD profileId)
{
	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (m_pConf[i]->GetAdHocProfileId() == profileId)
			{
				m_pConf[i]->ResetAdHocProfileId();
			}
		}
	}
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetCurrentConf(const ConfMonitorID confId) const
{
	for (int i = 0; i < (int)m_numb_of_conf; ++i)
	{
		if (m_pConf[i] != NULL)
		{
			if (m_pConf[i]->GetMonitorConfId() == confId)
				return m_pConf[i];
		}
	}
	return NULL; // STATUS_RESERVATION_NOT_EXISTS
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetCurrentConf(const char* confName) const
{
	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (!strcmp(m_pConf[i]->GetName(), confName))
				return m_pConf[i];
		}
	}

	return NULL;      // STATUS_RESERVATION_NOT_EXISTS
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetCurrentConfByNameOrByNumericId(const char* confNameOrId) const
{
	CCommConf* pCommConf = GetCurrentConf(confNameOrId);
	if (pCommConf)
		return pCommConf;

	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (!strcmp(m_pConf[i]->GetNumericConfId(), confNameOrId))
				return m_pConf[i];
		}
	}

	return NULL;      // STATUS_RESERVATION_NOT_EXISTS
}

//--------------------------------------------------------------------------
BOOL CCommConfDB::IsNumericIDExistInEntryQueueConf(const char* confNumericId) const
{
	int  nameLen = strlen(confNumericId);
	int  i       = 0;
	BOOL isExist = FALSE;

	if (nameLen <= 0)
		isExist = FALSE;
	else
	{
		for (i = 0; i < (int)m_numb_of_conf; i++)
		{
			if ((m_pConf[i] != NULL) && (YES == m_pConf[i]->GetEntryQ()))
			{
				if (!strncmp(m_pConf[i]->GetNumericConfId(), confNumericId, nameLen))
				{
					int localNameLen = strlen(m_pConf[i]->GetNumericConfId());
					if (localNameLen != nameLen)
						PASSERT(1); // we have to chack if the phone was in the name too !!!

					isExist = TRUE;
					break;
				}
			}
		}
	}

	return isExist;
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetCurrentOnGoingEQ(const char* name_char) const
{
	int EQnameLen = strlen(name_char);

	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (!strncmp(m_pConf[i]->GetName(), name_char, EQnameLen))
			{
				int confIdLength;

				if (!(strncmp(m_pConf[i]->GetName()+(EQnameLen), "(", 1)))
				{
					const char* pFindChar;
					pFindChar = strchr(m_pConf[i]->GetName()+(EQnameLen+1), ')');

					if (pFindChar)
					{
						confIdLength = pFindChar - (m_pConf[i]->GetName() + (EQnameLen)) - 1;
					}
					else
					{
						continue;
					}
				}
				else
				{
					continue;
				}

				int onGoingEQNameLen = strlen(m_pConf[i]->GetName());

				if (onGoingEQNameLen == (EQnameLen + confIdLength + 2))
					return m_pConf[i];
			}
		}
	}

	return NULL;      // STATUS_RESERVATION_NOT_EXISTS
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetFirstCommConf()
{
	if (m_numb_of_conf > 0)
	{
		m_index = 1;
		return m_pConf[0];
	}
	else
		return NULL;
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetNextCommConf()
{
	if (m_index >= m_numb_of_conf)
		return NULL;
	else
		return m_pConf[m_index++];
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetFirstCommConf(int& nPos)
{
	CCommConf* pCommConf = GetFirstCommConf();
	nPos = m_index;
	return pCommConf;
}

//--------------------------------------------------------------------------
CCommConf* CCommConfDB::GetNextCommConf(int& nPos)
{
	m_index = nPos;
	CCommConf* pCommConf = GetNextCommConf();
	nPos = m_index;
	return pCommConf;
}

//--------------------------------------------------------------------------
int CCommConfDB::GetConfParty(const char* calledPhoneNumber, CCommConf** pConf)
{
// -------------------------------------
// Return value is one of the followings :
// 0  party and conference are found
// -1	 party and conference are not found
// -2	auto add conference is found
// -3	conference is locked
// -4	conference found but max conference party number has been reached
// -5  conference is secured
// -------------------------------------
	int  numbPart;
	WORD meet_per_conf = 0;
	WORD meet_per_mcu  = 0;

	if (!calledPhoneNumber)
	{
		PTRACE(eLevelError, "CCommConfDB::GetConfParty. Illegal called phone number.");
		return -1;
	}

	if (strlen(calledPhoneNumber) == 0)
	{
		PTRACE(eLevelError, "CCommConfDB::GetConfParty. Empty called phone number.");
		return -1;
	}

	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL && TRUE == m_pConf[i]->IsConfPhone(calledPhoneNumber))
		{
			*pConf = m_pConf[i];
			return -2;
		}

		// rons patch
		if (m_pConf[i] != NULL)
		{
			numbPart = m_pConf[i]->GetNumParties();
			for (int j = 0; j < numbPart; j++)
			{
				CConfParty* temp;
				if (!j)
				{
					temp = m_pConf[i]->GetFirstParty();
				}
				else
				{
					temp = m_pConf[i]->GetNextParty();
				}

				if (temp != NULL)
				{
					if (temp->IsIpNetInterfaceType() == NO)
					{
						if (temp->GetConnectionType() == DIAL_IN)
						{
							if (calledPhoneNumber != NULL && temp->GetBondingTmpNumber() != NULL)
							{
								char* tmpBnd         = (char*)temp->GetBondingTmpNumber();
								WORD  tmpBndLen      = strlen(tmpBnd);
								WORD  calledPhoneLen = strlen((char*)calledPhoneNumber);
								if (tmpBndLen > 0 && calledPhoneLen > 0)
								{
									WORD isEqual = 0;
									if (tmpBndLen == calledPhoneLen)
									{
										if (!strcmp((char*)calledPhoneNumber, (char*)temp->GetBondingTmpNumber()))
										{
											isEqual = 1;
										}
									}
									else if (tmpBndLen < calledPhoneLen)
									{
										WORD        prefixLen = calledPhoneLen - tmpBndLen;
										const char* suffix    = &calledPhoneNumber[prefixLen];
										if (strcmp(suffix, tmpBnd) == 0)
										{
											isEqual = 1;
										}
									}

									if (isEqual == 1)
									{
										*pConf = m_pConf[i];
										return -2;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return -1;
}

//--------------------------------------------------------------------------
// Function name: SearchForH323ConferenceMatch                    written by: Uri Avni
// Variables:     partyIPaddress: The IP address of the dial in setup message.
// pSrcH323AliasArray: The source aliases.
// wSrcNumAlias: Source number of aliases.
// pConf: Out value. The information on the selected conference at the DB.
// pParty: Out value. The information on the selected party at the DB.
// pDestH323AliasArray: The destination 'aliases' as they appear at the setup string.
// wDestNumAlias: destination number of 'aliases'.
// Description:	  Find the party and confernce, when searching as meet_me_per_conf, that match the dial in setup string and returned the party and conference that was found at DB.
// Return value:  0  - for found party and conference.
// -1 - for not found conference.
// -2 - for found unDefined party.
// -3 - for found locked conference.
// -4 - for found party and conference but max parties number in conference has been reached
// -7 - for found both party and conf, but the party is in 'connected' status. [happens in hot backup scenario], by Yiping
//--------------------------------------------------------------------------
int CCommConfDB::SearchForH323ConferenceMatch(const mcTransportAddress* pPartyIPaddress, CH323Alias* pSrcH323AliasArray, WORD wSrcNumAlias,
                                              CCommConf** pConf, CConfParty** pParty, CH323Alias* pDestH323AliasArray,
                                              WORD wDestNumAlias, WORD useTransitEQ, BYTE isH323)
{
	int numbPart;

	// Try to find reservationless conference
	if (pDestH323AliasArray)
	{
		// we need to look for parties in 3 different DialIn calls
		// 1. Prefix(Service Alias Name) + ConfName
		// 2. ConfName Only
		// 3. Service name only will be add to the global search
		const char* Conference_name = NULL;
		const char* TemporeryConf   = NULL;
		const char* aliasName       = NULL;
		WORD        aliasType;
		char        urlAlias[255] = {0};

		for (int j = 0; j < wDestNumAlias; j++)
		{
			if (CPObject::IsValidPObjectPtr(&pDestH323AliasArray[j]))
			{
				aliasName = pDestH323AliasArray[j].GetAliasName();
				aliasType = pDestH323AliasArray[j].GetAliasType();

				PTRACE2INT(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch - aliasType: ", aliasType);

				// bugs 21723, 21591
				if (aliasType == PARTY_H323_ALIAS_URL_ID_TYPE)
				{
					if (sscanf(aliasName, "h323:%254[^'@']", urlAlias) > 0)
						aliasName = urlAlias;
					else if (sscanf(aliasName, "%254[^'@']", urlAlias) > 0)
						aliasName = urlAlias;

					PTRACE2(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch - urlAlias: ", urlAlias);
				}

				if (aliasName && strlen(aliasName))
				{
					PTRACE2(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch - aliasName: ", aliasName);
					TemporeryConf = ::GetIpServiceListMngr()->FindServiceAndGetStringWithoutPrefix(aliasName, aliasType);
					if (TemporeryConf)
					{
						if (isH323 == TRUE)
							Conference_name = TemporeryConf;
						else
							// be notice: in sip call if Prefix=10, string=10650  ->  Conference_name=650
							// we need to search for both: string and Conference_name a conference match
							Conference_name = aliasName;
					}
					else
						Conference_name = aliasName;

					PTRACE2(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch - Conference_name: ", Conference_name);

					// Conference name without predefined ivr string
					char ConfName[H243_NAME_LEN];
					memset(&ConfName, '\0', H243_NAME_LEN);
					if (strstr((char*)Conference_name, "#"))
					{
						strncpy(ConfName, Conference_name, sizeof(ConfName) - 1);
						ConfName[sizeof(ConfName) - 1] = '\0';

						for (int i = 0; i < H243_NAME_LEN; i++)
						{
							if (ConfName[i] == '#')
							{
								ConfName[i] = '\0';
								break;
							}
						}

						Conference_name = ConfName;

						// VNGFE-4808
						BOOL isStarDelimiterAllowed = NO;
						CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_STAR_DELIMITER_ALLOWED, isStarDelimiterAllowed);
						if (isStarDelimiterAllowed && (strstr((char*)Conference_name, "*")))
						{
							strncpy(ConfName, Conference_name, sizeof(ConfName) - 1);
							ConfName[sizeof(ConfName) - 1] = '\0';
							for (int i = 0; i < H243_NAME_LEN; i++)
							{
								if (ConfName[i] == '*')
								{
									ConfName[i] = '\0';
									break;
								}
							}

							Conference_name = ConfName;
						}
					}
					else
					{
						if (strstr((char*)Conference_name, "("))
						{
							strncpy(ConfName, Conference_name, sizeof(ConfName) - 1);
							ConfName[sizeof(ConfName) - 1] = '\0';
							for (int i = 0; i < H243_NAME_LEN; i++)
							{
								if (ConfName[i] == '(')
								{
									ConfName[i] = '\0';
									break;
								}
							}

							Conference_name = ConfName;
						}

						else
						{
							BOOL isStarDelimiterAllowed = NO;
							CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_STAR_DELIMITER_ALLOWED, isStarDelimiterAllowed);
							if ((isStarDelimiterAllowed && (strstr((char*)Conference_name, "*"))) || (strstr((char*)Conference_name, "**"))) // _mccf_ "**" for DMA sends conf password in URI
							{
								strncpy(ConfName, Conference_name, sizeof(ConfName) - 1);
								ConfName[sizeof(ConfName) - 1] = '\0';
								for (int i = 0; i < H243_NAME_LEN; i++)
								{
									if (ConfName[i] == '*')
									{
										ConfName[i] = '\0';
										break;
									}
								}

								Conference_name = ConfName;
							}
						}
					}

					if ((isH323 == FALSE) && (TemporeryConf))
					{
						// Conference name without predefined ivr string
						char ConfName[H243_NAME_LEN];
						memset(&ConfName, '\0', H243_NAME_LEN);
						if (strstr((char*)TemporeryConf, "#"))
						{
							strncpy(ConfName, TemporeryConf, sizeof(ConfName) - 1);
							ConfName[sizeof(ConfName) - 1] = '\0';

							for (int i = 0; i < H243_NAME_LEN; i++)
							{
								if (ConfName[i] == '#')
								{
									ConfName[i] = '\0';
									break;
								}
							}

							TemporeryConf = ConfName;

							// VNGFE-4808
							BOOL isStarDelimiterAllowed = NO;
							CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_STAR_DELIMITER_ALLOWED, isStarDelimiterAllowed);
							if (isStarDelimiterAllowed && (strstr((char*)TemporeryConf, "*")))
							{
								strncpy(ConfName, TemporeryConf, sizeof(ConfName) - 1);
								ConfName[sizeof(ConfName) - 1] = '\0';
								for (int i = 0; i < H243_NAME_LEN; i++)
								{
									if (ConfName[i] == '*')
									{
										ConfName[i] = '\0';
										break;
									}
								}

								TemporeryConf = ConfName;
							}
						}
						else
						{
							if (strstr((char*)TemporeryConf, "("))
							{
								strncpy(ConfName, TemporeryConf, sizeof(ConfName) - 1);
								ConfName[sizeof(ConfName) - 1] = '\0';

								for (int i = 0; i < H243_NAME_LEN; i++)
								{
									if (ConfName[i] == '(')
									{
										ConfName[i] = '\0';
										break;
									}
								}
								TemporeryConf = ConfName;
							}
							else
							{
								BOOL isStarDelimiterAllowed = NO;
								CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_STAR_DELIMITER_ALLOWED, isStarDelimiterAllowed);
								if (isStarDelimiterAllowed && (strstr((char*)TemporeryConf, "*")))
								{
									strncpy(ConfName, TemporeryConf, sizeof(ConfName) - 1);
									ConfName[sizeof(ConfName) - 1] = '\0';
									for (int i = 0; i < H243_NAME_LEN; i++)
									{
										if (ConfName[i] == '*')
										{
											ConfName[i] = '\0';
											break;
										}
									}

									TemporeryConf = ConfName;
								}
							}
						}
					}

					// if the conf name arrives like Prefix+ConfName or only confName search for the particepent in that conference
					if (Conference_name && strlen(Conference_name))
					{
						// Trace of the string we look to match to a conference
						PTRACE2(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch - Looking for conference match with: ", Conference_name);

						for (int i = 0; i < (int)m_numb_of_conf; i++)
						{
							if (m_pConf[i])
							{
								const char* dbConfName      = m_pConf[i]->GetName();
								const char* dbConfNumericId = m_pConf[i]->GetNumericConfId();
								int         lengthToCompare = max(strlen(dbConfName), strlen(Conference_name));

								int         lengthToCompareForTemporeryConf;
								if ((isH323 == FALSE) && TemporeryConf && strlen(TemporeryConf))
									lengthToCompareForTemporeryConf = max(strlen(dbConfName), strlen(TemporeryConf));

								if (m_pConf[i]->GetEntryQ())
								{
									int curChar = 0;
									ALLOCBUFFER(ConfIdbuf, 16);
									sprintf(ConfIdbuf, "%d", m_pConf[i]->GetMonitorConfId());

									int iConfIdLen   = strlen(ConfIdbuf);
									int EqNameLength = strlen(dbConfName)-(iConfIdLen+2);
									lengthToCompare = max(EqNameLength, (int)strlen(Conference_name));

									if ((isH323 == FALSE) && TemporeryConf && strlen(TemporeryConf))
										lengthToCompareForTemporeryConf = max(EqNameLength, (int)strlen(TemporeryConf));

									DEALLOCBUFFER(ConfIdbuf);
									PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch : Checking Ongoing EQ");
								}

								if (!strncmp(dbConfName, Conference_name, lengthToCompare) ||
								    !strcmp(dbConfName, Conference_name) ||
								    !strncmp(dbConfNumericId, Conference_name, NUMERIC_CONFERENCE_ID_LEN) ||
								    ((isH323 == FALSE) && (!strncasecmp(dbConfName, Conference_name, lengthToCompare) ||
								                           !strcasecmp(dbConfName, Conference_name))))
								{
									// CONF was FOUND
									if (!useTransitEQ || (useTransitEQ && m_pConf[i]->GetEntryQ()))
									{
										// if the conference is locked we reject the incoming call
										if ((m_pConf[i])->GetConfLockFlag() == YES &&
										    !(m_pConf[i])->IsDefinedIVRService())
											return -3;  // conference is locked

										// same for secured conf
										if ((m_pConf[i])->IsConfSecured() == YES &&
										    !(m_pConf[i])->IsDefinedIVRService())
											return -5;    // conference is secured

										// find out if it is a Defined dial in call
										// we have 3 search type IP+Alias, Alias, IP
										numbPart = m_pConf[i]->GetNumParties();
										for (int k = 0; k < numbPart; k++)
										{
											CConfParty* tempParty;
											if (!k)
												tempParty = m_pConf[i]->GetFirstParty();
											else
												tempParty = m_pConf[i]->GetNextParty();

											if ((tempParty != NULL) && tempParty->IsIpNetInterfaceType())
											{
												BYTE connectionType = tempParty->GetConnectionType();
												WORD bIsUndefined   = tempParty->IsUndefinedParty();
												if ((connectionType == DIAL_IN) && (bIsUndefined == 0)) // if the party status is in a call, search for another party
												{
													DWORD party_status = tempParty->GetPartyState();
													if ((party_status == PARTY_WAITING_FOR_DIAL_IN) ||
													    (party_status == PARTY_DISCONNECTED)) // it is impossible to return party that is connected. this will block
													{ // undefined parties to receive in a conference that got defined parties
														// with the same parameters and are already connected.
														// all this in the case of meet_me_per_conf only
														if (CASCADE_MODE_SLAVE != tempParty->GetCascadeMode())
														{
															if (tempParty->IsH323paramInclude((mcTransportAddress*)pPartyIPaddress, pSrcH323AliasArray,
															                                  wSrcNumAlias))
															{
																*pConf  = m_pConf[i];
																*pParty = tempParty;
																return 0; // Conf and Party were found !!!
															}
														}
														// if it's from the cascading master, we should only check the Alias.
														else
														{
															if (tempParty->IsH323AliasInclude(pSrcH323AliasArray, wSrcNumAlias))
															{
																*pConf  = m_pConf[i];
																*pParty = tempParty;
																return 0; // Conf and Party were found !!!
															}
														}
													}
													// Yiping: if the new call with party connected/connecting/disconnecting/.. is from master MCU, we need to reject it.
													else if (((PARTY_CONNECTED == party_status)
													          || (PARTY_CONNECTING == party_status)
													          || (PARTY_DISCONNECTING == party_status)
													          || (PARTY_CONNECTED_PARTIALY == party_status)
													          || (PARTY_CONNECTED_WITH_PROBLEM == party_status))
													         && (CASCADE_MODE_SLAVE == tempParty->GetCascadeMode()))
													{
														if (tempParty->IsH323AliasInclude(pSrcH323AliasArray, wSrcNumAlias))
														{
															*pConf  = m_pConf[i];
															*pParty = tempParty;
															return -7; // Conf and Party were found with party connected !!!
														}
													}
													else
													{ }
												}
											}
										}

										// CONF was Found PARTY was NOT Found
										// if the dial in call is undefined, find her type
										*pConf = m_pConf[i];
										pParty = NULL;

										if ((*pConf)->GetMaxParties() <= (*pConf)->GetNumParties()
										    && (*pConf)->GetMaxParties() != 0xFFFF
										    && !(*pConf)->IsDefinedIVRService())
											return -4;  // max parties number in conference has been reached

										return -2;    // code for unDefined party
									}
									else
									{
										PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch, found a conference with transit EQ name but it's not EQ conference");
									}
								}

								if ((isH323 == FALSE) && TemporeryConf && strlen(TemporeryConf))
								{
									if (!strncmp(dbConfName, TemporeryConf, lengthToCompare) ||
									    !strcmp(dbConfName, TemporeryConf) ||
									    !strncmp(dbConfNumericId, TemporeryConf, NUMERIC_CONFERENCE_ID_LEN))
									{
										// CONF was FOUND
										if (!useTransitEQ || (useTransitEQ && m_pConf[i]->GetEntryQ()))
										{
											// if the conference is locked we reject the incoming call
											if ((m_pConf[i])->GetConfLockFlag() == YES &&
											    !(m_pConf[i])->IsDefinedIVRService())
												return -3;  // conference is locked

											// same for secured conf
											if ((m_pConf[i])->IsConfSecured() == YES &&
											    !(m_pConf[i])->IsDefinedIVRService())
												return -5;    // conference is secured

											// find out if it is a Defined dial in call
											// we have 3 search type IP+Alias, Alias, IP
											numbPart = m_pConf[i]->GetNumParties();
											for (int k = 0; k < numbPart; k++)
											{
												CConfParty* tempParty;
												if (!k)
													tempParty = m_pConf[i]->GetFirstParty();
												else
													tempParty = m_pConf[i]->GetNextParty();

												if ((tempParty != NULL) && tempParty->IsIpNetInterfaceType())
												{
													BYTE connectionType = tempParty->GetConnectionType();
													WORD bIsUndefined   = tempParty->IsUndefinedParty();
													if ((connectionType == DIAL_IN) && (bIsUndefined == 0)) // if the party status is in a call, search for another party
													{
														DWORD party_status = tempParty->GetPartyState();
														if ((party_status == PARTY_WAITING_FOR_DIAL_IN) ||
														    (party_status == PARTY_DISCONNECTED)) // it is impossible to return party that is connected. this will block
														{ // undefined parties to receive in a conference that got defined parties
															// with the same parameters and are already connected.
															// all this in the case of meet_me_per_conf only
															if (CASCADE_MODE_SLAVE != tempParty->GetCascadeMode())
															{
																if (tempParty->IsH323paramInclude((mcTransportAddress*)pPartyIPaddress, pSrcH323AliasArray,
																                                  wSrcNumAlias))
																{
																	*pConf  = m_pConf[i];
																	*pParty = tempParty;
																	return 0; // Conf and Party were found !!!
																}
															}
															// if it's from the cascading master, we should only check the Alias.
															else
															{
																if (tempParty->IsH323AliasInclude(pSrcH323AliasArray, wSrcNumAlias))
																{
																	*pConf  = m_pConf[i];
																	*pParty = tempParty;
																	return 0; // Conf and Party were found !!!
																}
															}
														}
														// Yiping: if the new call with party connected/connecting/disconnecting/.. is from master MCU, we need to reject it.
														else if (((PARTY_CONNECTED == party_status)
														          || (PARTY_CONNECTING == party_status)
														          || (PARTY_DISCONNECTING == party_status)
														          || (PARTY_CONNECTED_PARTIALY == party_status)
														          || (PARTY_CONNECTED_WITH_PROBLEM == party_status))
														         && (CASCADE_MODE_SLAVE == tempParty->GetCascadeMode()))
														{
															if (tempParty->IsH323AliasInclude(pSrcH323AliasArray, wSrcNumAlias))
															{
																*pConf  = m_pConf[i];
																*pParty = tempParty;
																return -7; // Conf and Party were found with party connected !!!
															}
														}
														else
														{ }
													}
												}
											}

											// CONF was Found PARTY was NOT Found
											// if the dial in call is undefined, find her type
											*pConf = m_pConf[i];
											pParty = NULL;

											if ((*pConf)->GetMaxParties() <= (*pConf)->GetNumParties()
											    && (*pConf)->GetMaxParties() != 0xFFFF
											    && !(*pConf)->IsDefinedIVRService())
												return -4;  // max parties number in conference has been reached

											return -2;    // code for unDefined party
										}
										else
										{
											PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323ConferenceMatch, found a conference with transit EQ name but it's not EQ conference");
										}
									}
								}
							}
						}
					}
				}
			}
			else
				break;
		}
	}

	return -1;
}

//--------------------------------------------------------------------------
// Function name: IsMeetingRoomSpecified
// Variables:     pDestH323AliasArray: The destination 'aliases' as they appear at the setup string.
// wDestNumAlias: destination number of 'aliases'.
// Description:	  Check if after stripping service name, there is a meeting room explicitly specified
// Return value:  YES if MR specified, NO otherwise.
//--------------------------------------------------------------------------
BOOL CCommConfDB::IsMeetingRoomSpecified(CH323Alias* pDestH323AliasArray, WORD wDestNumAlias, BYTE isH323)
{
	BOOL MeetingRoomSpecified = NO;

	// Feature intended for H323 only
	// =================================
	if (isH323 && pDestH323AliasArray)
	{
		const char* TemporaryConf = NULL;
		const char* aliasName     = NULL;
		WORD        aliasType;
		char        urlAlias[255] = {0};

		for (int j = 0; j < wDestNumAlias && CPObject::IsValidPObjectPtr(&pDestH323AliasArray[j]) && !MeetingRoomSpecified; j++)
		{
			aliasName = pDestH323AliasArray[j].GetAliasName();
			aliasType = pDestH323AliasArray[j].GetAliasType();

			PTRACE2INT(eLevelInfoNormal, "CCommConfDB::IsMeetingRoomSpecified - aliasType: ", aliasType);

			// bugs 21723, 21591
			if (aliasType == PARTY_H323_ALIAS_URL_ID_TYPE)
			{
				if (sscanf(aliasName, "h323:%254[^'@']", urlAlias) > 0)
					aliasName = urlAlias;
				else if (sscanf(aliasName, "%254[^'@']", urlAlias) > 0)
					aliasName = urlAlias;

				PTRACE2(eLevelInfoNormal, "CCommConfDB::IsMeetingRoomSpecified - urlAlias: ", urlAlias);
			}

			if (aliasType == PARTY_H323_ALIAS_TRANSPORT_ID_TYPE)  // added for BRIDGE-136
				continue;

			if (aliasName && strlen(aliasName))
			{
				BOOL ServiceMatched;
				PTRACE2(eLevelInfoNormal, "CCommConfDB::IsMeetingRoomSpecified - aliasName: ", aliasName);
				TemporaryConf = ::GetIpServiceListMngr()->FindServiceAndGetStringWithoutPrefix(aliasName, aliasType, &ServiceMatched);
				if (!ServiceMatched || (TemporaryConf && TemporaryConf[0]))
				{
					MeetingRoomSpecified = YES;
					if (ServiceMatched)
					{
						PTRACE2(eLevelInfoNormal, "CCommConfDB::IsMeetingRoomSpecified - Meeting room requested: ", TemporaryConf);
					}
					else
					{
						PTRACE(eLevelInfoNormal, "CCommConfDB::IsMeetingRoomSpecified - Alias exists and no matching service");
					}
				}
			}
		}
	}
	else if (!isH323)
	{
		// Forcing defined user search on non H323 parties
		// ==================================================
		MeetingRoomSpecified = NO;
	}

	return MeetingRoomSpecified;
}


//--------------------------------------------------------------------------
// Function name: SearchForH323DefinedMatch                    written by: Uri Avni
// Variables:     partyIPaddress: The IP address of the dial in setup message.
// pSrcH323AliasArray: The source aliases.
// wSrcNumAlias: Source number of aliases.
// pConf: Out value. The information on the selected conference at the DB.
// pParty: Out value. The information on the selected party at the DB.
// pDestH323AliasArray: The destination 'aliases' as they appear at the setup string.
// wDestNumAlias: destination number of 'aliases'.
// Description:	  Find the party, when searching as meet_me_per_MCU, that match the dial in setup string and returned the party and conference that was found at DB.
// Return value:  0  - in case party and conference was found.
// -1 - for not founding according to the dial in ruels.
//--------------------------------------------------------------------------
int CCommConfDB::SearchForH323DefinedMatch(mcTransportAddress partyIPaddress, CH323Alias* pSrcH323AliasArray, WORD wSrcNumAlias, CCommConf** pConf, CConfParty** pParty, CH323Alias* pDestH323AliasArray, WORD wDestNumAlias)
{
	int numbPart;

	// 0 - empty, 1 - FoundAlias, 2 - FoundIp, 3 - FoundIpAlias
	WORD FoundPartyArray[4] = {0, 0, 0, 0};

	WORD AtlistOneConference4IpAlias = 0;
	WORD AtlistOneConference4Alias   = 0;
	WORD AtlistOneConference4Ip      = 0;

	CCommConf*  CommConfArray[4]  = {NULL, NULL, NULL, NULL}; // 0 - pTempConf, 1 - pConfAlias, 2 - pConfIp, 3 - pConfIpAlias
	CConfParty* ConfPartyArray[4] = {NULL, NULL, NULL, NULL}; // 0 - pTempParty, 1 - pPartyAlias, 2 - pPartyIp, 3 - pPartyIpAlias

	WORD  MatchLevel           = 0;
	WORD  IsPartyConnected     = 0;
	WORD  IsOldPartyConnetcted = 0;
	DWORD party_status         = 0;

	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			numbPart = m_pConf[i]->GetNumParties();
			for (int j = 0; j < numbPart; j++)
			{
				CConfParty* temp;
				if (!j)
					temp = m_pConf[i]->GetFirstParty();
				else
					temp = m_pConf[i]->GetNextParty();

				if ((temp != NULL) && temp->IsIpNetInterfaceType())
				{
					if ((temp->GetConnectionType() == DIAL_IN) && !(temp->IsUndefinedParty()))
					{
						MatchLevel       = temp->IsH323paramInclude(&partyIPaddress, pSrcH323AliasArray, wSrcNumAlias);
						party_status     = temp->GetPartyState();
						IsPartyConnected = ((party_status != PARTY_WAITING_FOR_DIAL_IN) &&
						                    (party_status != PARTY_DISCONNECTED) &&
						                    (!temp->IsDefinedPartyAssigned()));

						if (MatchLevel) // 0 - pTempConf, 1 - pConfAlias, 2 - pConfIp, 3 - pConfIpAlias
						{
							if ((CommConfArray[MatchLevel] == NULL) && (ConfPartyArray[MatchLevel] == NULL))
							{
								CommConfArray[MatchLevel]  = m_pConf[i];
								ConfPartyArray[MatchLevel] = temp;
							}
							else if (IsPartyConnected == 0)
							{
								if (ConfPartyArray[MatchLevel])
								{
									party_status         = ConfPartyArray[MatchLevel]->GetPartyState();
									IsOldPartyConnetcted = ((party_status != PARTY_WAITING_FOR_DIAL_IN) &&
									                        (party_status != PARTY_DISCONNECTED));

									if (IsOldPartyConnetcted)
									{
										CommConfArray[MatchLevel]  = m_pConf[i];
										ConfPartyArray[MatchLevel] = temp;
									}
								}
							}

							FoundPartyArray[MatchLevel]++;


							if (ConfPartyArray[0])
							{
								party_status         = ConfPartyArray[0]->GetPartyState();
								IsOldPartyConnetcted = ((party_status != PARTY_WAITING_FOR_DIAL_IN) &&
								                        (party_status != PARTY_DISCONNECTED));

								if (IsOldPartyConnetcted == 0)
								{
									if ((MatchLevel == 3) && (IsPartyConnected == 0)) // found IpAlias not connected
									{
										CommConfArray[0]  = CommConfArray[MatchLevel];
										ConfPartyArray[0] = ConfPartyArray[MatchLevel];
									}
								}
								else
								{
									CommConfArray[0]  = m_pConf[i];
									ConfPartyArray[0] = temp;
								}
							}
							else
							{
								CommConfArray[0]  = m_pConf[i];
								ConfPartyArray[0] = temp;
							}
						}
						else
						{
							PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch : \'No Match Level \'");
						}
					}
				}
			}
		}                            // END of one conf

		// 0 - empty, 1 - FoundAlias, 2 - FoundIp, 3 - FoundIpAlias
		if (FoundPartyArray[3])      // Ip+Alias
			AtlistOneConference4IpAlias++;

		if (FoundPartyArray[1])      // Alias
			AtlistOneConference4Alias++;
		else if (FoundPartyArray[2]) // IP - if we find IP parties and Alias parties in the same conference
			// its like we found Alias because alias is stronger then Ip in the priority order of the search
			AtlistOneConference4Ip++;

		FoundPartyArray[3] = 0;      // IP+ALIAS
		FoundPartyArray[2] = 0;      // IP
		FoundPartyArray[1] = 0;      // ALIAS
	} // End All Confs

	if ((CommConfArray[0] != NULL) && (ConfPartyArray[0] != NULL))
	{
		*pConf  = CommConfArray[0];
		*pParty = ConfPartyArray[0];
		// if the conference is locked we reject the incoming call
		if ((*pConf)->GetConfLockFlag() == YES &&
		    !(*pConf)->IsDefinedIVRService())
		{
			PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch : \' Conference is locked\'");
			return -3;    // conference is locked
		}

		// same for secured conf
		if ((*pConf)->IsConfSecured() == YES &&
		    !(*pConf)->IsDefinedIVRService())
			return -5;  // conference is secured

		if (AtlistOneConference4IpAlias)
		{
			if (AtlistOneConference4IpAlias != 1)
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch : \' IP_ALIAS match found in more than one conference\'");
				return -1; // the party was found in more then one conference
			}

			return 0;
		}

		if (AtlistOneConference4Alias)
		{
			if ((AtlistOneConference4Alias != 1) || AtlistOneConference4Ip)
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch : \' ALIAS or IP match found in more than one conference\'");
				return -1; // the party was found in more then one conference or we found alias party in one conference
				// and IP party in another => we have no way to decide to which conference it was intend to enter
			}

			// if the conference is locked we reject the incoming call
			if ((*pConf)->GetConfLockFlag() == YES &&
			    !(*pConf)->IsDefinedIVRService())
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch (AtlistOneConference4Alias): \' Conference is locked\'");
				return -3;  // conference is locked
			}

			// same for secured conf
			if ((*pConf)->IsConfSecured() == YES &&
			    !(*pConf)->IsDefinedIVRService())
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch (AtlistOneConference4Alias): \' Conference is Securde and No IVR service is defined \'");
				return -5;  // conference is secured
			}

			return 0;
		}

		if (AtlistOneConference4Ip)
		{
			if (AtlistOneConference4Ip != 1)
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch : \' IP match found in more than one conference\'");
				return -1; // the party was found in more then one conference
			}

			// if the conference is locked we reject the incoming call
			if ((*pConf)->GetConfLockFlag() == YES &&
			    !(*pConf)->IsDefinedIVRService())
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch (AtlistOneConference4Ip): \' Conference is locked\'");
				return -3;  // conference is locked
			}

			// same for secured conf
			if ((*pConf)->IsConfSecured() == YES &&
			    !(*pConf)->IsDefinedIVRService())
			{
				PTRACE(eLevelInfoNormal, "CCommConfDB::SearchForH323DefinedMatch (AtlistOneConference4Ip): \' Conference is Securde and No IVR service is defined \'");
				return -5;  // conference is secured
			}

			return 0;
		}
	}

	return -1;
}

//--------------------------------------------------------------------------
ConfMonitorID CCommConfDB::GetConfId(const char* confName) const
{
	CCommConf* pCommConf = GetCurrentConf(confName);
	return pCommConf ? pCommConf->GetMonitorConfId() : 0xFFFFFFFF;
}

//--------------------------------------------------------------------------
const char* CCommConfDB::GetConName(const ConfMonitorID confId) const
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	return pCommConf ? pCommConf->GetName() : NULL;
}

//--------------------------------------------------------------------------
PartyMonitorID CCommConfDB::GetPartyId(const char* confName, const char* partyName) const
{
	CCommConf* pCommConf = GetCurrentConf(confName);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName);
		return pConfParty ? pConfParty->GetPartyId() : 0xFFFFFFFF;
	}
	return 0xFFFFFFFF;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
PartyMonitorID CCommConfDB::GetPartyId(const ConfMonitorID confId, const char* partyName) const
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName);
		return pConfParty ? pConfParty->GetPartyId() : 0xFFFFFFFF;
	}
	return 0xFFFFFFFF;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
PartyMonitorID CCommConfDB::GetRecordingLinkPartyId(const ConfMonitorID confId) const
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetRecordLinkCurrentParty();
		return pConfParty ? pConfParty->GetPartyId() : 0xFFFFFFFF;
	}
	return 0xFFFFFFFF;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
const char* CCommConfDB::GetRecordingLinkPartyName(const ConfMonitorID confId) const
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetRecordLinkCurrentParty();
		return pConfParty ? pConfParty->GetName() : NULL;
	}
	return NULL;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
const char* CCommConfDB::GetPartyName(const char* confName, const PartyMonitorID partyId) const
{
	CCommConf* pCommConf = GetCurrentConf(confName);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		return pConfParty ? pConfParty->GetName() : NULL;
	}
	return NULL;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
const char* CCommConfDB::GetPartyName(const ConfMonitorID confId, const PartyMonitorID partyId) const
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		return pConfParty ? pConfParty->GetName() : NULL;
	}
	return NULL;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
const CConfParty* CCommConfDB::GetCurrentParty(const ConfMonitorID confId, const PartyMonitorID partyId)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	return pCommConf ? pCommConf->GetCurrentParty(partyId) : NULL;
}

//--------------------------------------------------------------------------
DWORD CCommConfDB::GetSummaryUpdateCounter() const
{
	return m_dwSummaryUpdateCounter;
}

//--------------------------------------------------------------------------
void CCommConfDB::IncreaseSummaryUpdateCounter()
{
	m_dwSummaryUpdateCounter++;

	if (m_dwSummaryUpdateCounter == 0xFFFFFFFF)
		m_dwSummaryUpdateCounter = 0;
}

//--------------------------------------------------------------------------
DWORD CCommConfDB::GetFullUpdateCounter() const
{
	return m_dwFullUpdateCounter;
}

//--------------------------------------------------------------------------
void CCommConfDB::IncreaseFullUpdateCounter()
{
	m_dwFullUpdateCounter++;

	if (m_dwFullUpdateCounter == 0xFFFFFFFF)
		m_dwFullUpdateCounter = 0;
}

//--------------------------------------------------------------------------
WORD CCommConfDB::GetLastTerminatedConfReason()
{
	return m_lastTerminationReason;
}

//--------------------------------------------------------------------------
void CCommConfDB::SetLastTerminatedConfReason(WORD reason)
{
// Snmp defines 2 values for termination reasons:
// 1: normalTermination(1)
// 2: administrativelyTerminated(2)

	if (reason == TERMINATE_BY_OPERATOR)
		m_lastTerminationReason = 2;
	else
		m_lastTerminationReason = 1;
}

//--------------------------------------------------------------------------
void CCommConfDB::IncreaseConferenceCounter()
{
	m_TotalConferenceCounter++;
}

//--------------------------------------------------------------------------
DWORD CCommConfDB::GetConferenceCounter()
{
	return m_TotalConferenceCounter;
}

//--------------------------------------------------------------------------
BYTE CCommConfDB::IsConfSecured(ConfMonitorID confId)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	return pCommConf ? pCommConf->IsConfSecured() : NO;
}

//--------------------------------------------------------------------------
BYTE CCommConfDB::IsConfLocked(ConfMonitorID confId)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	return pCommConf ? pCommConf->GetConfLockFlag() : NO;
}

//--------------------------------------------------------------------------
BYTE CCommConfDB::IsLPRConf(ConfMonitorID confId)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	return pCommConf ? pCommConf->GetIsLPR() : NO;
}

//--------------------------------------------------------------------------
BYTE CCommConfDB::GetConnectionType(ConfMonitorID confId, PartyMonitorID partyId)
{
	CCommConf* pCommConf = GetCurrentConf(confId);
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		if (pConfParty)
			return pConfParty->GetConnectionType();
	}
	return 0;
}

//--------------------------------------------------------------------------
const char* CCommConfDB::GetConfByMsConversationId(char* msConvId) const
{
	for (int i = 0; i < (int)m_numb_of_conf; i++)
	{
		if (m_pConf[i] != NULL)
		{
			if (strcmp(m_pConf[i]->GetMsConversationId(), msConvId) == 0)
				return (m_pConf[i]->GetName());
		}
	}

	return NULL;      // STATUS_CONFERENCE_NOT_EXISTS
}

//--------------------------------------------------------------------------
void CCommConfDB::DeleteDB()
{
	if (m_numb_of_conf > 0)
		DBGPASSERT(m_numb_of_conf);

	m_index                  = 0;
	m_numb_of_conf           = 0;
	m_lastTerminationReason  = 1;
	m_TotalConferenceCounter = 0;
}

//--------------------------------------------------------------------------
CTaskApp* CCommConfDB::GetPartyTask(const char* tag)
{
	for (size_t j = 0; j < m_numb_of_conf; ++j)
	{
		if (m_pConf[j])
		{
			for (size_t i = 0; i < m_pConf[j]->GetNumParties(); ++i)
			{
				CConfParty* pParty = i ? m_pConf[j]->GetNextParty() : m_pConf[j]->GetFirstParty();

				if (pParty && pParty->GetPartyTag() == tag)
					return pParty->GetTask();
			}
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
DWORD CCommConfDB::GetPartyIDByTag(const char* tag)
{
	for (size_t j = 0; j < m_numb_of_conf; ++j)
	{
		if (m_pConf[j])
		{
			for (size_t i = 0; i < m_pConf[j]->GetNumParties(); ++i)
			{
				CConfParty* pParty = i ? m_pConf[j]->GetNextParty() : m_pConf[j]->GetFirstParty();

				if (pParty && pParty->GetPartyTag() == tag)
					return pParty->GetPartyId();
			}
		}
	}

	return 0xFFFFFFFF;
}

//--------------------------------------------------------------------------
DWORD CCommConfDB::GetConfIDByTag(const char* tag)
{
	for (size_t j = 0; j < m_numb_of_conf; ++j)
	{
		if (m_pConf[j])
		{
			for (size_t i = 0; i < m_pConf[j]->GetNumParties(); ++i)
			{
				CConfParty* pParty = i ? m_pConf[j]->GetNextParty() : m_pConf[j]->GetFirstParty();

				if (pParty && pParty->GetPartyTag() == tag)
					return m_pConf[j]->GetConfID();
			}
		}
	}

	return 0xFFFFFFFF;
}
//--------------------------------------------------------------------------
