#include "NStream.h"
#include "PartyList.h"
#include "Trace.h"
#include "StatusesGeneral.h"

/////////////////////////////////////////////////////////////////////////////
CPartyList::CPartyList()
{
}

/////////////////////////////////////////////////////////////////////////////
CPartyList::~CPartyList()
{
	PARTYLIST::iterator _end = m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_PartyList.begin(); _itr != _end; ++_itr)
	{
		_itr->second->Destroy();
		delete _itr->second;
	}
	m_PartyList.clear();
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CPartyList::find(const char* name)
{
	if (!name)
		return NULL;

	PARTYLIST::iterator _itr = m_PartyList.find(name);
	return (_itr != m_PartyList.end()) ? _itr->second : NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CPartyList::find(const CTaskApp* pTask)
{
	PARTYLIST::iterator _end = m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_PartyList.begin(); _itr != _end; ++_itr)
		if (_itr->second && _itr->second->GetPartyTaskApp() == pTask)
			return _itr->second;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CPartyList::find(const PartyRsrcID partyId)
{
	PARTYLIST::iterator _end = m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_PartyList.begin(); _itr != _end; ++_itr)
		if (_itr->second && _itr->second->GetPartyRsrcId() == partyId)
			return _itr->second;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CPartyList::remove(const CTaskApp* pTask)
{
	PARTYLIST::iterator _end = m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_PartyList.begin(); _itr != _end; ++_itr)
	{
		if (_itr->second && _itr->second->GetPartyTaskApp() == pTask)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			m_PartyList.erase(_itr);
			Dump("CPartyList::remove - ", pPartyConnection);
			return pPartyConnection;
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
int CPartyList::insert(CPartyConnection* pPartyConnection)
{
	if (!pPartyConnection)
		return STATUS_FAIL;

	PASSERT_AND_RETURN_VALUE(!pPartyConnection->GetName(), STATUS_FAIL);
	m_PartyList.insert(std::make_pair(pPartyConnection->GetName(), pPartyConnection));

	Dump("CPartyList::insert - ", pPartyConnection);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CPartyList::entries()
{
	return(m_PartyList.size());
}

/////////////////////////////////////////////////////////////////////////////
void CPartyList::Dump(const char* prefixString, CPartyConnection* pPartyConnection)
{
	if (pPartyConnection)
	{
		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (pPartyCntl)
		{
			std::ostringstream msg;
			msg << prefixString
					<< "PartyName:" << pPartyCntl->GetName()
					<< ", PartyId:" << pPartyCntl->GetPartyRsrcId()
					<< ", MonitorPartyId:" << pPartyCntl->GetMonitorPartyId()
					<< ", MonitorConfId:" << pPartyCntl->GetMonitorConfId();

			PTRACE(eLevelInfoNormal, msg.str().c_str());
		}
	}
}
