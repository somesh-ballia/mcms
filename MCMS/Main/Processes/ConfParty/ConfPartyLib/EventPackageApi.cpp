/*
 * EventPackageApi.cpp
 *
 *  Created on: Jan 23, 2014
 *      Author: Dmitry Krasnopolsky
 */

#include "StatusesGeneral.h"
#include "EventPackageApi.h"
#include "EventPackage.h"
#include "PrettyTable.h"

namespace EventPackage
{

////////////////////////////////////////////////////////////////////////////
//                        ApiLync
////////////////////////////////////////////////////////////////////////////
STATUS ApiLync::AddDSH(PartyRsrcID id, const LyncMsi* dshBuffer, size_t dshLen)
{
	#undef PARAMS
	#define PARAMS "Id:" << id

	FPASSERTSTREAM_AND_RETURN_VALUE(!dshBuffer, PARAMS << " - Failed, Invalid DSH buffer", STATUS_INCONSISTENT_PARAMETERS);

	ConfInfo* pConfInfo = Manager::Instance().GetConfInfo(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConfInfo, PARAMS << " - Failed, Invalid id", STATUS_INCONSISTENT_PARAMETERS);

	LyncMsiList& msi = pConfInfo->msi;

	msi.clear();
	msi.insert(msi.begin(), dshBuffer, dshBuffer+dshLen);

	std::ostringstream msg;
	msg << ", DSH:{";
	for (LyncMsiList::const_iterator _msi = msi.begin(); _msi != msi.end(); ++_msi)
	{
		if (_msi == msi.begin())
			msg << *_msi;
		else
			msg << "," << *_msi;
	}
	msg << "}";
	EP_TRACE << PARAMS << msg.str().c_str();
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void ApiLync::GetDSH(PartyRsrcID id, LyncDshList& dshList)
{
	#undef PARAMS
	#define PARAMS "Id:" << id

	dshList.clear();

	ConfInfo* pConfInfo = Manager::Instance().GetConfInfo(id);
	FPASSERTSTREAM_AND_RETURN(!pConfInfo, PARAMS << " - Failed, Invalid id");

	CPrettyTable<const char*, const char*, LyncMsi, const char*, const char*, LyncMsi, const char*> tbl("Endpoint", "AudId", "AudMsi", "AudStatus", "VidId", "VidMsi", "VidStatus");

	for (LyncMsiList::const_iterator _msi = pConfInfo->msi.begin(); _msi != pConfInfo->msi.end(); ++_msi)
	{
		if (*_msi == 0)
			continue;

		MediaInfo info;
		bool RC = Manager::Instance().GetMedia(id, std::bind2nd(Predicate::Media_Msi(), *_msi), info);
		if (!RC)
		{
			FPASSERTSTREAM(!RC, PARAMS << ", Msi:" << *_msi);
			continue;
		}

		LyncDsh dsh;
		dsh.user.id              = info.user->m_entity;
		dsh.user.displayText     = info.user->m_displayText.value();
		dsh.endpoint.id          = info.endpoint->m_entity;
		dsh.endpoint.displayText = info.endpoint->m_displayText.value();

		const Endpoint::MediasContainer& medias = info.endpoint->m_medias;
		for (Endpoint::MediasContainer::const_iterator _imedia = medias.begin(); _imedia != medias.end(); ++_imedia)
		{
			if (_imedia->m_type == eMediaType_Video)
			{
				dsh.video.id     = _imedia->m_id;
				dsh.video.msi    = _imedia->m_msi;
				dsh.video.status = _imedia->m_status;
			}
			if (_imedia->m_type == eMediaType_Audio)
			{
				dsh.audio.id     = _imedia->m_id;
				dsh.audio.msi    = _imedia->m_msi;
				dsh.audio.status = _imedia->m_status;
			}
		}
		tbl.Add(dsh.endpoint.id.c_str(),
				dsh.audio.id.c_str(), dsh.audio.msi, to_string(dsh.audio.status),
				dsh.video.id.c_str(), dsh.video.msi, to_string(dsh.video.status));
		dshList.push_back(dsh);
	}
	EP_TRACE << PARAMS << tbl.Get();
}

//--------------------------------------------------------------------------
LyncMsi ApiLync::GetCorrelativeMSI(PartyRsrcID id, LyncMsi msi, MediaType mediaType)
{
	#undef PARAMS
	#define PARAMS "Id:" << id << ", Msi:" << msi << ", MediaType:" << mediaType

	MediaInfo info;
	bool RC = Manager::Instance().GetMedia(id, std::bind2nd(Predicate::Media_Msi(), msi), info);
	FPASSERTSTREAM_AND_RETURN_VALUE(!RC, PARAMS, 0);

	const Endpoint::MediasContainer& medias = info.endpoint->m_medias;
	Endpoint::MediasContainer::const_iterator _imedia = std::find_if(medias.begin(), medias.end(), std::bind2nd(Predicate::Media_Type(), mediaType));
	FTRACECOND_AND_RETURN_VALUE(_imedia == medias.end(), PARAMS, 0);

	return _imedia->m_msi;
}

//--------------------------------------------------------------------------
void ApiLync::GetMsiList(PartyRsrcID id, LyncMsiList& msiList, MediaType mediaType, bool activeOnly)
{
	ConfInfo* pConfInfo = Manager::Instance().GetConfInfo(id);
	FPASSERTSTREAM_AND_RETURN(!pConfInfo, "id:" << id);

	const Users::UsersContainer& users = pConfInfo->conference.m_users.m_users;
	for (Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
	{
		const User::EndpointsContainer& endpoints = _iuser->m_endpoints;
		for (User::EndpointsContainer::const_iterator _iendpoint = endpoints.begin(); _iendpoint != endpoints.end(); ++_iendpoint)
		{
			const Endpoint::MediasContainer& medias = _iendpoint->m_medias;
			for (Endpoint::MediasContainer::const_iterator _imedia = medias.begin(); _imedia != medias.end(); ++_imedia)
			{
				if (_imedia->m_type == mediaType)
				{
					if (activeOnly)
					{
						if (_imedia->m_status == eMediaStatusType_SendOnly || _imedia->m_status == eMediaStatusType_SendRecv)
							msiList.push_back(_imedia->m_msi);
					}
					else
						msiList.push_back(_imedia->m_msi);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
std::string ApiLync::GetSpotlightUserId(PartyRsrcID id)
{
	std::string userId;

	ConfInfo* pConfInfo = Manager::Instance().GetConfInfo(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConfInfo, "id:" << id, userId);

	const ConferenceView::EntityViewsContainer& viewsContainer = pConfInfo->conference.m_conferenceView.m_entityViews;
	for (ConferenceView::EntityViewsContainer::const_iterator _iview = viewsContainer.begin(); _iview != viewsContainer.end(); ++_iview)
	{
		const ConferenceMedia::MediumsContainer& mediums = _iview->m_entityState.m_media.m_mediums;

		ConferenceMedia::MediumsContainer::const_iterator _imedium = std::find_if(mediums.begin(), mediums.end(), std::bind2nd(Predicate::Medium_Type(), eMediumType_Video));
		if (_imedium != mediums.end())
		{
			VideoParameters videoParameters = _imedium->m_modalParameters.m_videoParameters;

			if (videoParameters.m_videoMode == eVideoSwitchingModeType_ManualSwitched)
			{
				if (videoParameters.m_intendedPrimaryPresenterSource.m_entry.IsAssigned())
					return videoParameters.m_intendedPrimaryPresenterSource.m_entry.value();
			}
		}
	}

	return userId;
}


//--------------------------------------------------------------------------
LyncMsi ApiLync::GetUserMSI(PartyRsrcID id, const std::string& userId, MediaType mediaType, bool activeOnly)
{
	std::list<EventPackage::MediaInfo> info;
	Manager::Instance().GetMedia(id, std::bind2nd(EventPackage::Predicate::Media_Type(), mediaType), info);

	std::list<EventPackage::MediaInfo>::iterator _end = info.end();
	for (std::list<EventPackage::MediaInfo>::iterator _ii = info.begin(); _ii != _end; ++_ii)
	{
		if (activeOnly)
		{
			if (_ii->media->m_status == eMediaStatusType_SendOnly || _ii->media->m_status == eMediaStatusType_SendRecv)
				if (_ii->user->m_entity == userId)
					return _ii->media->m_msi;
		}
		else
		{
			if (_ii->user->m_entity == userId)
				return _ii->media->m_msi;
		}
	}
	FTRACEINTO << "Id:" << id << ", User:" << userId << ", MediaType:" << mediaType << ", ActiveOnly:" << (int)activeOnly << " - Failed, Media does not found";
	return 0;
}


} /* namespace EventPackage */
