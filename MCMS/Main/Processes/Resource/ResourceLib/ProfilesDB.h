#ifndef PROFILESDB_H_
#define PROFILESDB_H_

#include "PObject.h"
#include "AllocateStructs.h"
#include <map>
#include "StatusesGeneral.h"

#define ProfileIdType DWORD

////////////////////////////////////////////////////////////////////////////
//                        CProfilesDB
////////////////////////////////////////////////////////////////////////////
class CProfilesDB : public CPObject
{
	CLASS_TYPE_1(CProfilesDB, CPObject)

	typedef std::map<ProfileIdType, eVideoPartyType> Profiles;

public:
	                CProfilesDB();
	virtual        ~CProfilesDB();
	const char*     NameOf() const { return "CProfilesDB"; }

	STATUS          AddProfile(ProfileIdType profileId, eVideoPartyType maxVideoPartyType);
	STATUS          UpdateProfile(ProfileIdType profileId, eVideoPartyType maxVideoPartyType);
	STATUS          RemoveProfile(ProfileIdType profileId);
	eVideoPartyType GetMaxVideoPartyTypeByProfileId(ProfileIdType profileId) const;
	void            InitProfileList(PROFILE_IND_LIST_S* profileList);
	size_t          GetNumOfProfiles() const;

	friend std::ostream& operator<<(std::ostream& os, CProfilesDB& obj);

private:
	Profiles        m_profiles;
};

#endif /*PROFILESDB_H_*/
