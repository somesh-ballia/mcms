#include "SelfConsistencyWhatToCheck.h"
#include "SharedDefines.h"

CSelfConsistencyWhatToCheck::CSelfConsistencyWhatToCheck()
{
	SetAll(FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////
CSelfConsistencyWhatToCheck::~CSelfConsistencyWhatToCheck()
{
}
///////////////////////////////////////////////////////////////////////////////////////////
void CSelfConsistencyWhatToCheck::CheckEverything()
{
	SetAll(TRUE);
}
///////////////////////////////////////////////////////////////////////////////////////////
void CSelfConsistencyWhatToCheck::SetAll(BOOL bset)
{
	m_bCheckNoMoreConferences = bset;
	m_bCheckNumOfConferences = bset;
	m_bCheckAudioVideoConfiguration = bset;
	m_bCheckNIDs = bset;
	m_bCheckPhones = bset;
	m_bCheckParties = bset;
	m_bCheckPartiesResourceDescriptors = bset;
	m_bCheckPartiesResourceDescriptorsCheckSharedMemory = bset;
	m_bCheckPartiesUDPDescriptors = bset;
	m_bCheckPartiesPorts = bset;
}
///////////////////////////////////////////////////////////////////////////////////////////
