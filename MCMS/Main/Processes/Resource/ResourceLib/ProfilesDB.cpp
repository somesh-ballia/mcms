#include "ProfilesDB.h"
#include "AllocateStructs.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CProfilesDB
////////////////////////////////////////////////////////////////////////////
CProfilesDB::CProfilesDB()
{
}

////////////////////////////////////////////////////////////////////////////
CProfilesDB::~CProfilesDB()
{
}

////////////////////////////////////////////////////////////////////////////
STATUS CProfilesDB::AddProfile(ProfileIdType profileId, eVideoPartyType maxVideoPartyType)
{
	Profiles::iterator _ii = m_profiles.find(profileId);
	if (_ii != m_profiles.end())
		return STATUS_FAIL;

	m_profiles.insert(std::make_pair(profileId, maxVideoPartyType));
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CProfilesDB::UpdateProfile(ProfileIdType profileId, eVideoPartyType maxVideoPartyType)
{
	Profiles::iterator _ii = m_profiles.find(profileId);
	if (_ii == m_profiles.end())
		return STATUS_FAIL;

	_ii->second = maxVideoPartyType;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CProfilesDB::RemoveProfile(ProfileIdType profileId)
{
	Profiles::iterator _ii = m_profiles.find(profileId);
	if (_ii == m_profiles.end())
		return STATUS_FAIL;

	m_profiles.erase(_ii);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
eVideoPartyType CProfilesDB::GetMaxVideoPartyTypeByProfileId(ProfileIdType profileId) const
{
	Profiles::const_iterator _ii = m_profiles.find(profileId);

	PASSERTSTREAM_AND_RETURN_VALUE(_ii == m_profiles.end(), "ProfileId:" << profileId, eVideo_party_type_none)

	return _ii->second;
}

////////////////////////////////////////////////////////////////////////////
void CProfilesDB::InitProfileList(PROFILE_IND_LIST_S* pProfileList)
{
	for (DWORD i = 0; i < pProfileList->list_size; ++i)
		AddProfile((pProfileList->profile_list[i]).profile_Id, (pProfileList->profile_list[i]).maxVideoPartyType);
}

////////////////////////////////////////////////////////////////////////////
size_t CProfilesDB::GetNumOfProfiles() const
{
	return (m_profiles.size());
}

#undef min
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CProfilesDB& obj)
{
	CPrettyTable<ProfileIdType, const char*> tbl("ProfileId", "MaxVideoPartyType");

	CProfilesDB::Profiles::iterator _iiEnd = obj.m_profiles.end();
	for (CProfilesDB::Profiles::iterator _ii = obj.m_profiles.begin(); _ii != _iiEnd; ++_ii)
		tbl.Add(_ii->first, eVideoPartyTypeNames[_ii->second]);

	os << tbl.Get();
	return os;
}

#ifndef min
	#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
