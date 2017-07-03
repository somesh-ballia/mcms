/*
 * EventPackageImpl.h
 *
 *  Created on: Jan 22, 2014
 *      Author: Dmitry Krasnopolsky
 */

#ifndef EVENTPACKAGEIMPL_H_
#define EVENTPACKAGEIMPL_H_

//--------------------------------------------------------------------------
template<typename _Predicate>
bool Manager::GetUser(PartyRsrcID id, _Predicate predicate, UserInfo& info)
{
	const Conference* pConferenceInfo = GetConference(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConferenceInfo, "id:" << id, false);

	const Users::UsersContainer& users = pConferenceInfo->m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		if (predicate(*_iuser))
		{
			info.user = &*_iuser;
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------------
template<typename _Predicate>
bool Manager::GetUser(PartyRsrcID id, _Predicate predicate, std::list<UserInfo>& info)
{
	bool rc = false;

	const Conference* pConferenceInfo = GetConference(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConferenceInfo, "id:" << id, false);

	const Users::UsersContainer& users = pConferenceInfo->m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		if (predicate(*_iuser))
		{
			UserInfo temp;
			temp.user = &*_iuser;
			info.push_back(temp);
			rc = true;
		}
	}
	return rc;
}

//--------------------------------------------------------------------------
template<typename _Predicate>
bool Manager::GetEndpoint(PartyRsrcID id, _Predicate predicate, EndpointInfo& info)
{
	const Conference* pConferenceInfo = GetConference(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConferenceInfo, "id:" << id, false);

	const Users::UsersContainer& users = pConferenceInfo->m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		const User::EndpointsContainer& endpoints = _iuser->m_endpoints;
		for (User::EndpointsContainer::const_iterator _iendpoint = endpoints.begin(); _iendpoint != endpoints.end(); ++_iendpoint)
		{
			if (predicate(*_iendpoint))
			{
				info.user     = &*_iuser;
				info.endpoint = &*_iendpoint;
				return true;
			}
		}
	}
	return false;
}

//--------------------------------------------------------------------------
template<typename _Predicate>
bool Manager::GetEndpoint(PartyRsrcID id, _Predicate predicate, std::list<EndpointInfo>& info)
{
	bool rc = false;

	const Conference* pConferenceInfo = GetConference(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConferenceInfo, "id:" << id, false);

	const Users::UsersContainer& users = pConferenceInfo->m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		const User::EndpointsContainer& endpoints = _iuser->m_endpoints;
		for (User::EndpointsContainer::const_iterator _iendpoint = endpoints.begin(); _iendpoint != endpoints.end(); ++_iendpoint)
		{
			if (predicate(*_iendpoint))
			{
				EndpointInfo temp;
				temp.user     = &*_iuser;
				temp.endpoint = &*_iendpoint;
				info.push_back(temp);
				rc = true;
			}
		}
	}
	return rc;
}

//--------------------------------------------------------------------------
template<typename _Predicate>
bool Manager::GetMedia(PartyRsrcID id, _Predicate predicate, MediaInfo& info)
{
	const Conference* pConferenceInfo = GetConference(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConferenceInfo, "id:" << id, false);

	const Users::UsersContainer& users = pConferenceInfo->m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		const User::EndpointsContainer& endpoints = _iuser->m_endpoints;
		for (User::EndpointsContainer::const_iterator _iendpoint = endpoints.begin(); _iendpoint != endpoints.end(); ++_iendpoint)
		{
			const Endpoint::MediasContainer& medias = _iendpoint->m_medias;
			for (Endpoint::MediasContainer::const_iterator _imedia = medias.begin(); _imedia != medias.end(); ++_imedia)
			{
				if (predicate(*_imedia))
				{
					info.user     = &*_iuser;
					info.endpoint = &*_iendpoint;
					info.media    = &*_imedia;
					return true;
				}
			}
		}
	}
	return false;
}

//--------------------------------------------------------------------------
template<typename _Predicate>
bool Manager::GetMedia(PartyRsrcID id, _Predicate predicate, std::list<MediaInfo>& info)
{
	bool rc = false;

	const Conference* pConferenceInfo = GetConference(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConferenceInfo, "id:" << id, false);

	const Users::UsersContainer& users = pConferenceInfo->m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		const User::EndpointsContainer& endpoints = _iuser->m_endpoints;
		for (User::EndpointsContainer::const_iterator _iendpoint = endpoints.begin(); _iendpoint != endpoints.end(); ++_iendpoint)
		{
			const Endpoint::MediasContainer& medias = _iendpoint->m_medias;
			for (Endpoint::MediasContainer::const_iterator _imedia = medias.begin(); _imedia != medias.end(); ++_imedia)
			{
				if (predicate(*_imedia))
				{
					MediaInfo temp;
					temp.user     = &*_iuser;
					temp.endpoint = &*_iendpoint;
					temp.media    = &*_imedia;
					info.push_back(temp);
					rc = true;
				}
			}
		}
	}
	return rc;
}

#endif /* EVENTPACKAGEIMPL_H_ */
