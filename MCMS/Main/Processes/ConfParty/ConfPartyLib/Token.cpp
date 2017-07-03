#include "Token.h"

////////////////////////////////////////////////////////////////////////////
//                        CTokenUser
////////////////////////////////////////////////////////////////////////////
CTokenUser::CTokenUser()
{
  SetToNone();
}

////////////////////////////////////////////////////////////////////////////
CTokenUser::CTokenUser(PartyRsrcID partyId, BYTE mcuNumber, BYTE terminalnumber, BYTE randomNumber)
{
  SetToNone();

	Set(partyId, mcuNumber, terminalnumber, randomNumber);
}

////////////////////////////////////////////////////////////////////////////
CTokenUser::~CTokenUser(void)
{
}

////////////////////////////////////////////////////////////////////////////
void CTokenUser::Set(PartyRsrcID partyId, BYTE mcuNumber, BYTE terminalnumber, BYTE randomNumber)
{
	m_partyId        = partyId;
  m_mcuNumber       = mcuNumber;
  m_terminalNumber  = terminalnumber;
  m_randomNumber    = randomNumber;
}

////////////////////////////////////////////////////////////////////////////
const CTaskApp* CTokenUser::GetParty(void)
{
	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyId);
	if (!pParty)
	{
		SetToNone();
		return NULL;
	}
	return pParty;
}

////////////////////////////////////////////////////////////////////////////
void CTokenUser::SetToNone(void)
{
	m_partyId         = 0;
  m_mcuNumber       = 0;
  m_terminalNumber  = 0;
  m_randomNumber    = 0;
}

////////////////////////////////////////////////////////////////////////////
BYTE CTokenUser::operator!(void)
{
	return (m_partyId) ? NO : YES;
}
