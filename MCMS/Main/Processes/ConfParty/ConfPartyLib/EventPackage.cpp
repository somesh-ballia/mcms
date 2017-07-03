/*
 * EventPackage::Manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: Dmitry Krasnopolsky
 */

#include <memory>
#include "PrettyTable.h"
#include "StatusesGeneral.h"
#include "ConfPartyOpcodes.h"
#include "EventPackage.h"

using namespace EventPackage;

std::map<EventType, EventAbstactFactory::EventCreatePtr> EventAbstactFactory::_classFactory;

//--------------------------------------------------------------------------
std::ostream& operator<< (std::ostream& os, const Subscribers& in)
{
	CPrettyTable<EventType, COsQueue*, OPCODE> tbl("Event", "Observer", "Opcode");

	for (Subscribers::const_iterator _isubscriber = in.begin(); _isubscriber != in.end(); ++_isubscriber)
		tbl.Add(_isubscriber->first, _isubscriber->second.first, _isubscriber->second.second);
	os << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
//                        Manager
////////////////////////////////////////////////////////////////////////////
namespace EventPackage
{

template<>
void Manager::MergeEntity<ApiBaseObjectPtr>(ApiBaseObjectPtr& dst, const ApiBaseObjectPtr& src);

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Execution>(Execution& dst, const Execution& src)
{
	MergeEntity(dst.m_when, src.m_when);
	MergeEntity(dst.m_reason, src.m_reason);
	MergeEntity(dst.m_by, src.m_by);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Uri>(Uri& dst, const Uri& src)
{
	MergeEntity(dst.m_entity, src.m_entity);
	MergeEntity(dst.m_purpose, src.m_purpose);
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_modified, src.m_modified);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Uris>(Uris& dst, const Uris& src)
{
	if (!src.IsAssigned())
		return;

	if (src.m_state == eStateType_Full || src.m_state == eStateType_Deleted)
		dst.Clear();

	if (src.m_state == eStateType_Deleted)
		return;

	dst.m_state = eStateType_Full;

	for (Uris::UrisContainer::const_iterator _isrc = src.m_uris.begin(); _isrc != src.m_uris.end(); ++_isrc)
	{
		Uris::UrisContainer::iterator _idst = std::find_if(dst.m_uris.begin(), dst.m_uris.end(), std::bind2nd(Predicate::Uri_Uri(), _isrc->m_entity));
		if (_idst != dst.m_uris.end())
			MergeEntity(*_idst, *_isrc);
		else
			dst.m_uris.push_back(*_isrc);
	}
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Host>(Host& dst, const Host& src)
{
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_webPage, src.m_webPage);
	MergeEntity(dst.m_uris, src.m_uris);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<ConferenceState>(ConferenceState& dst, const ConferenceState& src)
{
	MergeEntity(dst.m_userCount, src.m_userCount);
	MergeEntity(dst.m_active, src.m_active);
	MergeEntity(dst.m_locked, src.m_locked);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<VideoParameters>(VideoParameters& dst, const VideoParameters& src)
{
	MergeEntity(dst.m_videoMode, src.m_videoMode);
	MergeEntity(dst.m_intendedPrimaryPresenterSource, src.m_intendedPrimaryPresenterSource);
}

//--------------------------------------------------------------------------
template<>
inline void Manager::MergeEntity<ModalParameters>(ModalParameters& dst, const ModalParameters& src)
{
	MergeEntity(dst.m_videoParameters, src.m_videoParameters);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<ConferenceMedium>(ConferenceMedium& dst, const ConferenceMedium& src)
{
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_type, src.m_type);
	MergeEntity(dst.m_status, src.m_status);
	MergeEntity(dst.m_label, src.m_label);
	MergeEntity(dst.m_modalParameters, src.m_modalParameters);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<ConferenceMedia>(ConferenceMedia& dst, const ConferenceMedia& src)
{
	for (ConferenceMedia::MediumsContainer::const_iterator _isrc = src.m_mediums.begin(); _isrc != src.m_mediums.end(); ++_isrc)
	{
		ConferenceMedia::MediumsContainer::iterator _idst = std::find_if(dst.m_mediums.begin(), dst.m_mediums.end(), std::bind2nd(Predicate::Medium_Type(), _isrc->m_type));
		if (_idst != dst.m_mediums.end())
			MergeEntity(*_idst, *_isrc);
		else
			dst.m_mediums.push_back(*_isrc);
	}
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<ConferenceDescription>(ConferenceDescription& dst, const ConferenceDescription& src)
{
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_subject, src.m_subject);
	MergeEntity(dst.m_freeText, src.m_freeText);
	MergeEntity(dst.m_keywords, src.m_keywords);
	MergeEntity(dst.m_maximumUserCount, src.m_maximumUserCount);
	MergeEntity(dst.m_confUris, src.m_confUris);
	MergeEntity(dst.m_serviceUris, src.m_serviceUris);
	MergeEntity(dst.m_availableMedia, src.m_availableMedia);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Media>(Media& dst, const Media& src)
{
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_label, src.m_label);
	MergeEntity(dst.m_sourceName, src.m_sourceName);
	MergeEntity(dst.m_type, src.m_type);
	MergeEntity(dst.m_status, src.m_status);
	MergeEntity(dst.m_id, src.m_id);
	MergeEntity(dst.m_ssrcId, src.m_ssrcId);
	MergeEntity(dst.m_msi, src.m_msi);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Call>(Call& dst, const Call& src)
{
	if (!src.m_pCallType.IsAssigned())
		return;

	if (src.m_pCallType.Contains(SipDialogId::classType()))
	{
		SipDialogId& srcSipDialogId = (SipDialogId&)*(src.m_pCallType);

		if (!dst.m_pCallType.IsAssigned())
			dst.m_pCallType.create(srcSipDialogId.objectTag(), srcSipDialogId.objectNsUrn());

		SipDialogId& dstSipDialogId = (SipDialogId&)*(dst.m_pCallType);

		MergeEntity(dstSipDialogId.m_displayText, srcSipDialogId.m_displayText);
		MergeEntity(dstSipDialogId.m_callId, srcSipDialogId.m_callId);
		MergeEntity(dstSipDialogId.m_fromTag, srcSipDialogId.m_fromTag);
		MergeEntity(dstSipDialogId.m_toTag, srcSipDialogId.m_toTag);
	}
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Endpoint>(Endpoint& dst, const Endpoint& src)
{
	dst.m_state = eStateType_Full;

	MergeEntity(dst.m_status, src.m_status);
	MergeEntity(dst.m_joiningMethod, src.m_joiningMethod);
	MergeEntity(dst.m_disconnectionMethod, src.m_disconnectionMethod);
	MergeEntity(dst.m_entity, src.m_entity);
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_referred, src.m_referred);
	MergeEntity(dst.m_joiningInfo, src.m_joiningInfo);
	MergeEntity(dst.m_disconnectionInfo, src.m_disconnectionInfo);
	MergeEntity(dst.m_callInfo, src.m_callInfo);

	for (Endpoint::MediasContainer::const_iterator _isrc = src.m_medias.begin(); _isrc != src.m_medias.end(); ++_isrc)
	{
		Endpoint::MediasContainer::iterator _idst = std::find_if(dst.m_medias.begin(), dst.m_medias.end(), std::bind2nd(Predicate::Media_Id(), _isrc->m_id));
		if (_idst != dst.m_medias.end())
			MergeEntity(*_idst, *_isrc);
		else
			dst.m_medias.push_back(*_isrc);
	}
}

//--------------------------------------------------------------------------
template<>
inline void Manager::MergeEntity<UserRoles>(UserRoles& dst, const UserRoles& src)
{
	MergeEntity(dst.m_roles, src.m_roles);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<User>(User& dst, const User& src)
{
	if (!src.IsAssigned())
		return;

	if (src.m_state == eStateType_Full || src.m_state == eStateType_Deleted)
		dst.Clear();

	if (src.m_state == eStateType_Deleted)
		return;

	dst.m_state = eStateType_Full;

	MergeEntity(dst.m_entity, src.m_entity);
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_languages, src.m_languages);
	MergeEntity(dst.m_cascadedFocus, src.m_cascadedFocus);
	MergeEntity(dst.m_associatedAors, src.m_associatedAors);
	MergeEntity(dst.m_roles, src.m_roles);

	for (User::EndpointsContainer::const_iterator _isrc = src.m_endpoints.begin(); _isrc != src.m_endpoints.end(); ++_isrc)
	{
		User::EndpointsContainer::iterator _idst = std::find_if(dst.m_endpoints.begin(), dst.m_endpoints.end(), std::bind2nd(Predicate::Endpoint_Entity(), _isrc->m_entity));

		// All entities with deleted state should be removed from 'actual' document
		if (_isrc->m_state == eStateType_Deleted)
		{
			if (_idst != dst.m_endpoints.end())
				dst.m_endpoints.erase(_idst);
			continue;
		}

		if (_idst != dst.m_endpoints.end())
			MergeEntity(*_idst, *_isrc);        // Entity already exist, update it
		else
			dst.m_endpoints.push_back(*_isrc);  // Entity is not exist, insert it as is
	}
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Users>(Users& dst, const Users& src)
{
	if (!src.IsAssigned())
		return;

	if (src.m_state == eStateType_Full || src.m_state == eStateType_Deleted)
		dst.Clear();

	if (src.m_state == eStateType_Deleted)
		return;

	dst.m_state = eStateType_Full;

	for (Users::UsersContainer::const_iterator _isrc = src.m_users.begin(); _isrc != src.m_users.end(); ++_isrc)
	{
		Users::UsersContainer::iterator _idst = std::find_if(dst.m_users.begin(), dst.m_users.end(), std::bind2nd(Predicate::User_Entity(), _isrc->m_entity));

		// All entities with deleted state should be removed from 'actual' document
		if (_isrc->m_state == eStateType_Deleted)
		{
			if (_idst != dst.m_users.end())
				dst.m_users.erase(_idst);
			continue;
		}

		if (_idst != dst.m_users.end())
			MergeEntity(*_idst, *_isrc);    // Entity already exist, update it
		else
			dst.m_users.push_back(*_isrc);  // Entity is not exist, insert it as is
	}
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<EntityState>(EntityState& dst, const EntityState& src)
{
	MergeEntity(dst.m_displayText, src.m_displayText);
	MergeEntity(dst.m_active, src.m_active);
	MergeEntity(dst.m_locked, src.m_locked);
	MergeEntity(dst.m_presentationModeCapable, src.m_presentationModeCapable);
	MergeEntity(dst.m_multiViewCapable, src.m_multiViewCapable);
	MergeEntity(dst.m_videoPresentationModeCapable, src.m_videoPresentationModeCapable);
	MergeEntity(dst.m_media, src.m_media);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<EntityView>(EntityView& dst, const EntityView& src)
{
	dst.m_state = eStateType_Full;

	MergeEntity(dst.m_entity, src.m_entity);
	MergeEntity(dst.m_entityState, src.m_entityState);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<ConferenceView>(ConferenceView& dst, const ConferenceView& src)
{
	if (!src.IsAssigned())
		return;

	if (src.m_state == eStateType_Full || src.m_state == eStateType_Deleted)
		dst.Clear();

	if (src.m_state == eStateType_Deleted)
		return;

	dst.m_state = eStateType_Full;

	for (ConferenceView::EntityViewsContainer::const_iterator _isrc = src.m_entityViews.begin(); _isrc != src.m_entityViews.end(); ++_isrc)
	{
		ConferenceView::EntityViewsContainer::iterator _idst = std::find_if(dst.m_entityViews.begin(), dst.m_entityViews.end(), std::bind2nd(Predicate::EntityView_Entity(), _isrc->m_entity));

		// All entities with deleted state should be removed from 'actual' document
		if (_isrc->m_state == eStateType_Deleted)
		{
			if (_idst != dst.m_entityViews.end())
				dst.m_entityViews.erase(_idst);
			continue;
		}

		if (_idst != dst.m_entityViews.end())
			MergeEntity(*_idst, *_isrc);          // Entity already exist, update it
		else
			dst.m_entityViews.push_back(*_isrc);  // Entity is not exist, insert it as is
	}
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<Conference>(Conference& dst, const Conference& src)
{
	if (!src.IsAssigned())
		return;

	if (src.m_state == eStateType_Full || src.m_state == eStateType_Deleted)
		dst.Clear();

	if (src.m_state == eStateType_Deleted)
		return;

	dst.m_state = eStateType_Full;

	MergeEntity(dst.m_entity, src.m_entity);
	MergeEntity(dst.m_version, src.m_version);
	MergeEntity(dst.m_conferenceDescription, src.m_conferenceDescription);
	MergeEntity(dst.m_hostInfo, src.m_hostInfo);
	MergeEntity(dst.m_conferenceState, src.m_conferenceState);
	MergeEntity(dst.m_users, src.m_users);
	MergeEntity(dst.m_sidebarsByRef, src.m_sidebarsByRef);
	MergeEntity(dst.m_conferenceView, src.m_conferenceView);
	MergeEntity(dst.m_pSidebarsByVal, src.m_pSidebarsByVal);
}

//--------------------------------------------------------------------------
template<>
void Manager::MergeEntity<ApiBaseObjectPtr>(ApiBaseObjectPtr& dst, const ApiBaseObjectPtr& src)
{
	if (!src.IsAssigned())
		return;

	SidebarsByVal& srcSidebarsByVal = (SidebarsByVal&)*(src);

	if (srcSidebarsByVal.m_state == eStateType_Full || srcSidebarsByVal.m_state == eStateType_Deleted)
	{
		if (dst.IsAssigned())
			((SidebarsByVal&)*(dst)).Clear();

		if (srcSidebarsByVal.m_state == eStateType_Deleted)
			return;
	}
	if (!dst.IsAssigned())
		dst.create(srcSidebarsByVal.objectTag(), srcSidebarsByVal.objectNsUrn());

	SidebarsByVal& dstSidebarsByVal = (SidebarsByVal&)*(dst);

	for (SidebarsByVal::ConferencesInfoContainer::const_iterator _isrc = srcSidebarsByVal.m_conferencesInfo.begin(); _isrc != srcSidebarsByVal.m_conferencesInfo.end(); ++_isrc)
	{
		SidebarsByVal::ConferencesInfoContainer::iterator _idst = std::find_if(dstSidebarsByVal.m_conferencesInfo.begin(), dstSidebarsByVal.m_conferencesInfo.end(), std::bind2nd(Predicate::Conference_Entity(), _isrc->m_entity));

		// All entities with deleted state should be removed from 'actual' document
		if (_isrc->m_state == eStateType_Deleted)
		{
			if (_idst != dstSidebarsByVal.m_conferencesInfo.end())
				dstSidebarsByVal.m_conferencesInfo.erase(_idst);
			continue;
		}

		if (_idst != dstSidebarsByVal.m_conferencesInfo.end())
			MergeEntity(*_idst, *_isrc);                          // Entity already exist, srcSidebarsByVal it
		else
			dstSidebarsByVal.m_conferencesInfo.push_back(*_isrc); // Entity is not exist, insert it as is
	}
	dstSidebarsByVal.m_state = eStateType_Full;
}

//--------------------------------------------------------------------------
template<typename _T>
inline void Manager::MergeEntity(ApiType<_T>& dst, const ApiType<_T>& src)
{
	if (src.IsAssigned())
		dst = src;
}

//--------------------------------------------------------------------------
template<typename _T>
inline void Manager::MergeEntity(_T& dst, const _T& src)
{
		dst = src;
}

}

//--------------------------------------------------------------------------
STATUS Manager::AddConference(PartyRsrcID id, const char* callId, const char* xmlBuffer, size_t xmlLen)
{
	#undef PARAMS
	#define PARAMS "Id:" << id << ", CallId:" << callId

	FPASSERTSTREAM_AND_RETURN_VALUE(!callId, PARAMS, STATUS_INCONSISTENT_PARAMETERS);
	FPASSERTSTREAM_AND_RETURN_VALUE(!xmlBuffer, PARAMS, STATUS_INCONSISTENT_PARAMETERS);

	EP_TRACE << PARAMS;

	// Create a new entry or retrieve the existed
	ConfInfo* pConfInfo = (id != INVALID) ? AddConfInfo(id, callId) : GetConfInfo(callId);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConfInfo, PARAMS, STATUS_FAIL);

	Conference& curConf = pConfInfo->conference;

	std::auto_ptr<Conference> oldConf(curConf.NewCopy());
	std::auto_ptr<Conference> newConf(new Conference);

	FPASSERTSTREAM_AND_RETURN_VALUE(!newConf->ReadFromXmlStream(xmlBuffer, xmlLen), PARAMS, STATUS_FAIL);

	//EPTRACE << PARAMS << ", newXml:\n" << *newInfo.get();

	MergeEntity(curConf, *newConf.get());

	EP_TRACE << PARAMS << ", curXml:\n" << curConf;

	BuildEvents(pConfInfo, *oldConf.get(), curConf);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
bool Manager::DelConference(PartyRsrcID id)
{
	Conferences::iterator _iconference = m_conferences.find(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(_iconference == m_conferences.end(), "Id:" << id << " - Failed, Does not exist", false);

	delete _iconference->second;
	m_conferences.erase(_iconference);
	return true;
}

//--------------------------------------------------------------------------
const Conference* Manager::GetConference(PartyRsrcID id)
{
	ConfInfo* pConfInfo = GetConfInfo(id);
	return (pConfInfo) ? &pConfInfo->conference : NULL;
}

//--------------------------------------------------------------------------
STATUS Manager::AddSubscriber(PartyRsrcID id, EventType event, const std::pair<COsQueue*, OPCODE>& subscriber)
{
	#undef PARAMS
	#define PARAMS "Id:" << id << ", Event:" << event << ", Observer:" << subscriber.first << ", Opcode:" << subscriber.second

	FPASSERTSTREAM_AND_RETURN_VALUE(!subscriber.first, PARAMS << " - Failed, Invalid queue", STATUS_INCONSISTENT_PARAMETERS);
	FPASSERTSTREAM_AND_RETURN_VALUE(!subscriber.first->IsValid(), PARAMS << " - Failed, Invalid queue", STATUS_INCONSISTENT_PARAMETERS);

	ConfInfo* pConfInfo = AddConfInfo(id, NULL); // Should be AddConference here, because subscription can be before BENOTIFY
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConfInfo, PARAMS << " - Failed, Invalid id", STATUS_INCONSISTENT_PARAMETERS);

	Subscribers& subscribers = pConfInfo->subscribers;
	std::pair<Subscribers::iterator, Subscribers::iterator> subscribers_range = subscribers.equal_range(event);

	Subscribers::iterator _isubscriber = std::find_if(subscribers_range.first, subscribers_range.second, std::bind2nd(Predicate::Subscriber(), subscriber));
	FPASSERTSTREAM_AND_RETURN_VALUE(_isubscriber != subscribers_range.second, PARAMS << " - Failed, Already subscribed", STATUS_OK);

	subscribers.insert(std::make_pair(event, subscriber));

	EP_TRACE << PARAMS;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS Manager::DelSubscriber(PartyRsrcID id, EventType event, const std::pair<COsQueue*, OPCODE>& subscriber)
{
	#undef PARAMS
	#define PARAMS "Id:" << id << ", Event:" << event << ", Observer:" << subscriber.first << ", Opcode:" << subscriber.second

	FPASSERTSTREAM_AND_RETURN_VALUE(!subscriber.first, PARAMS << " - Failed, Invalid queue", STATUS_INCONSISTENT_PARAMETERS);

	ConfInfo* pConfInfo = GetConfInfo(id);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pConfInfo, PARAMS << " - Failed, Invalid id", STATUS_INCONSISTENT_PARAMETERS);

	Subscribers& subscribers = pConfInfo->subscribers;
	std::pair<Subscribers::iterator, Subscribers::iterator> subscribers_range = subscribers.equal_range(event);

	Subscribers::iterator _isubscriber = std::find_if(subscribers_range.first, subscribers_range.second, std::bind2nd(Predicate::Subscriber(), subscriber));
	FPASSERTSTREAM_AND_RETURN_VALUE(_isubscriber == subscribers_range.second, PARAMS << " - Failed, not subscribed", STATUS_FAIL);

	subscribers.erase(_isubscriber);

	EP_TRACE << PARAMS;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
template<typename _T>
_T* Manager::CreateEvent(const User& user)
{
	_T* pEvent = new _T;

	pEvent->value.user = user.m_entity;

	return pEvent;
}

//--------------------------------------------------------------------------
template<typename _T>
_T* Manager::CreateEvent(const User& user, const Endpoint& endpoint)
{
	_T* pEvent = CreateEvent<_T>(user);

	pEvent->value.endpoint = endpoint.m_entity;

	return pEvent;
}

//--------------------------------------------------------------------------
template<typename _T>
_T* Manager::CreateEvent(const User& user, const Endpoint& endpoint, const Media& media)
{
	_T* pEvent = CreateEvent<_T>(user, endpoint);

	pEvent->value.media = media.m_id;
	pEvent->value.type  = media.m_type;
	pEvent->value.msi   = media.m_msi;

	return pEvent;
}

//--------------------------------------------------------------------------
void Manager::BuildEvents(ConfInfo* pConfInfo, const Conference& former, const Conference& update)
{
	Subscribers& subscribers = pConfInfo->subscribers;
	if (subscribers.empty())
	{
		EP_TRACE << "Id:" << pConfInfo->id << " - No subscriptions for this conference";
		return;
	}

	EP_TRACE << "Id:" << pConfInfo->id;

	Events events(pConfInfo->id);
	BuildEvents(events, former.m_users.m_users, update.m_users.m_users);
	BuildEvents(events, former.m_conferenceView.m_entityViews, update.m_conferenceView.m_entityViews);

	EP_TRACE << "Subscribers:" << subscribers;
	EP_TRACE << "All events:" << events;

	Events dummyMap(pConfInfo->id);

	Notifications notifications;
	for (int event = eEventType_DEFAULT+1; event < eEventType_LAST; ++event)
	{
		std::pair<Events::iterator, Events::iterator> event_range = events.equal_range(static_cast<EventType>(event));
		if (event_range.first == event_range.second)
			continue; // There were no events of this type, skip processing

		std::pair<Subscribers::iterator, Subscribers::iterator> subscribers_range = subscribers.equal_range(static_cast<EventType>(event));
		if (subscribers_range.first == subscribers_range.second)
			continue; // No subscribers for this event, skip processing

		for (Subscribers::iterator _isubscriber = subscribers_range.first; _isubscriber != subscribers_range.second; ++_isubscriber)
		{
			std::pair<Notifications::iterator, bool> ret = notifications.insert(std::make_pair(_isubscriber->second, dummyMap));
			if (ret.first != notifications.end())
				ret.first->second.insert(event_range.first, event_range.second);
		}
	}

	for (Notifications::iterator _inotification = notifications.begin(); _inotification != notifications.end(); ++_inotification)
	{
		EP_TRACE << "Subscriber:" << _inotification->first.first << ", Opcode:" << _inotification->first.second << _inotification->second << "\n";

		CSegment* pSeg = new CSegment;
		*pSeg << (OPCODE)LYNC_CONF_INFO_UPDATED << _inotification->second;

		_inotification->second.clear();

		_inotification->first.first->Send(pSeg, _inotification->first.second);
	}
}

//--------------------------------------------------------------------------
void Manager::BuildEvents(Events& events, const Users::UsersContainer& former, const Users::UsersContainer& update)
{
	User::EndpointsContainer empty;

	for (Users::UsersContainer::const_iterator _iformer = former.begin(); _iformer != former.end(); ++_iformer)
	{
		Users::UsersContainer::const_iterator _iupdate = std::find_if(update.begin(), update.end(), std::bind2nd(Predicate::User_Entity(), _iformer->m_entity));
		if (_iupdate != update.end())
		{
			if (_iformer->m_displayText != _iupdate->m_displayText)
			{
				EventUserDisplayTextUpdated* pEvent = CreateEvent<EventUserDisplayTextUpdated>(*_iupdate);
				pEvent->value.old_val = _iformer->m_displayText.value();
				pEvent->value.new_val = _iupdate->m_displayText.value();
				events.insert(std::make_pair(pEvent->Type(), pEvent));
			}
			BuildEvents(events, _iformer->m_endpoints, _iupdate->m_endpoints, *_iformer);
		}
		else
		{
			EventUserDeleted* pEvent = CreateEvent<EventUserDeleted>(*_iformer);
			events.insert(std::make_pair(pEvent->Type(), pEvent));

			BuildEvents(events, _iformer->m_endpoints, empty, *_iformer);
		}
	}

	for (Users::UsersContainer::const_iterator _iupdate = update.begin(); _iupdate != update.end(); ++_iupdate)
	{
		Users::UsersContainer::const_iterator _iformer = std::find_if(former.begin(), former.end(), std::bind2nd(Predicate::User_Entity(), _iupdate->m_entity));
		if (_iformer == former.end())
		{
			EventUserAdded* pEvent = CreateEvent<EventUserAdded>(*_iupdate);
			events.insert(std::make_pair(pEvent->Type(), pEvent));

			BuildEvents(events, empty, _iupdate->m_endpoints, *_iupdate);
		}
	}
}

//--------------------------------------------------------------------------
void Manager::BuildEvents(Events& events, const User::EndpointsContainer& former, const User::EndpointsContainer& update, const User& user)
{
	Endpoint::MediasContainer empty;

	for (User::EndpointsContainer::const_iterator _iformer = former.begin(); _iformer != former.end(); ++_iformer)
	{
		User::EndpointsContainer::const_iterator _iupdate = std::find_if(update.begin(), update.end(), std::bind2nd(Predicate::Endpoint_Entity(), _iformer->m_entity));
		if (_iupdate != update.end())
		{
			if (_iformer->m_displayText != _iupdate->m_displayText)
			{
				EventEndpointDisplayTextUpdated* pEvent = CreateEvent<EventEndpointDisplayTextUpdated>(user, *_iformer);
				pEvent->value.old_val = _iformer->m_displayText.value();
				pEvent->value.new_val = _iupdate->m_displayText.value();
				events.insert(std::make_pair(pEvent->Type(), pEvent));
			}
			if (_iformer->m_status != _iupdate->m_status)
			{
				EventEndpointStatusUpdated* pEvent = CreateEvent<EventEndpointStatusUpdated>(user, *_iformer);
				pEvent->value.old_val = _iformer->m_status;
				pEvent->value.new_val = _iupdate->m_status;
				events.insert(std::make_pair(pEvent->Type(), pEvent));
			}
			BuildEvents(events, _iformer->m_medias, _iupdate->m_medias, user, *_iformer);
		}
		else
		{
			EventEndpointDeleted* pEvent = CreateEvent<EventEndpointDeleted>(user, *_iformer);
			events.insert(std::make_pair(pEvent->Type(), pEvent));

			BuildEvents(events, _iformer->m_medias, empty, user, *_iformer);
		}
	}

	for (User::EndpointsContainer::const_iterator _iupdate = update.begin(); _iupdate != update.end(); ++_iupdate)
	{
		User::EndpointsContainer::const_iterator _iformer = std::find_if(former.begin(), former.end(), std::bind2nd(Predicate::Endpoint_Entity(), _iupdate->m_entity));
		if (_iformer == former.end())
		{
			EventEndpointAdded* pEvent = CreateEvent<EventEndpointAdded>(user, *_iupdate);
			events.insert(std::make_pair(pEvent->Type(), pEvent));

			BuildEvents(events, empty, _iupdate->m_medias, user, *_iupdate);
		}
	}
}

//--------------------------------------------------------------------------
void Manager::BuildEvents(Events& events, const Endpoint::MediasContainer& former, const Endpoint::MediasContainer& update, const User& user, const Endpoint& endpoint)
{
	for (Endpoint::MediasContainer::const_iterator _iformer = former.begin(); _iformer != former.end(); ++_iformer)
	{
		Endpoint::MediasContainer::const_iterator _iupdate = std::find_if(update.begin(), update.end(), std::bind2nd(Predicate::Media_Id(), _iformer->m_id));
		if (_iupdate != update.end())
		{
			if (_iformer->m_displayText != _iupdate->m_displayText)
			{
				EventMediaDisplayTextUpdated* pEvent = CreateEvent<EventMediaDisplayTextUpdated>(user, endpoint, *_iformer);
				pEvent->value.old_val = _iformer->m_displayText.value();
				pEvent->value.new_val = _iupdate->m_displayText.value();
				events.insert(std::make_pair(pEvent->Type(), pEvent));
			}
			if (_iformer->m_status != _iupdate->m_status)
			{
				EventMediaStatusUpdated* pEvent = CreateEvent<EventMediaStatusUpdated>(user, endpoint, *_iformer);
				pEvent->value.old_val = _iformer->m_status;
				pEvent->value.new_val = _iupdate->m_status;
				events.insert(std::make_pair(pEvent->Type(), pEvent));
			}
		}
		else
		{
			EventMediaDeleted* pEvent = CreateEvent<EventMediaDeleted>(user, endpoint, *_iformer);
			events.insert(std::make_pair(pEvent->Type(), pEvent));
		}
	}

	for (Endpoint::MediasContainer::const_iterator _iupdate = update.begin(); _iupdate != update.end(); ++_iupdate)
	{
		Endpoint::MediasContainer::const_iterator _iformer = std::find_if(former.begin(), former.end(), std::bind2nd(Predicate::Media_Id(), _iupdate->m_id));
		if (_iformer == former.end())
		{
			EventMediaAdded* pEvent = CreateEvent<EventMediaAdded>(user, endpoint, *_iupdate);
			events.insert(std::make_pair(pEvent->Type(), pEvent));
		}
	}
}

//--------------------------------------------------------------------------
void Manager::BuildEvents(Events& events, const ConferenceView::EntityViewsContainer& former, const ConferenceView::EntityViewsContainer& update)
{
	for (ConferenceView::EntityViewsContainer::const_iterator _iformer = former.begin(); _iformer != former.end(); ++_iformer)
	{
		std::size_t found = _iformer->m_entity.find("opaque=app:conf:audio-video");
		if (found == std::string::npos)
			continue;

		ConferenceView::EntityViewsContainer::const_iterator _iupdate = std::find_if(update.begin(), update.end(), std::bind2nd(Predicate::EntityView_Entity(), _iformer->m_entity));
		if (_iupdate != update.end())
		{
			const ConferenceMedia::MediumsContainer& former_mediums = _iformer->m_entityState.m_media.m_mediums;
			const ConferenceMedia::MediumsContainer& update_mediums = _iupdate->m_entityState.m_media.m_mediums;

			ConferenceMedia::MediumsContainer::const_iterator _former_video = std::find_if(former_mediums.begin(), former_mediums.end(), std::bind2nd(Predicate::Medium_Type(), eMediumType_Video));
			if (_former_video != former_mediums.end())
			{
				ConferenceMedia::MediumsContainer::const_iterator _update_video = std::find_if(update_mediums.begin(), update_mediums.end(), std::bind2nd(Predicate::Medium_Type(), eMediumType_Video));
				FPASSERT_AND_RETURN(_update_video == update_mediums.end());

				const VideoParameters& former_video_param = _former_video->m_modalParameters.m_videoParameters;
				const VideoParameters& update_video_param = _update_video->m_modalParameters.m_videoParameters;

				if (former_video_param.m_videoMode != update_video_param.m_videoMode || !former_video_param.m_intendedPrimaryPresenterSource.m_entry.IsTheSame(update_video_param.m_intendedPrimaryPresenterSource.m_entry))
				{
					if (update_video_param.m_videoMode == eVideoSwitchingModeType_ManualSwitched)
					{
						if (update_video_param.m_intendedPrimaryPresenterSource.m_entry.IsAssigned())
						{
							const std::string& user = update_video_param.m_intendedPrimaryPresenterSource.m_entry.value();

							UserInfo info;
							GetUser(events.m_id, std::bind2nd(Predicate::User_Entity(), user), info);
							FPASSERTSTREAM_AND_RETURN(!info.user, "User:" << user);

							const User::EndpointsContainer& endpoints = info.user->m_endpoints;
							User::EndpointsContainer::const_iterator _iendpoint = std::find_if(endpoints.begin(), endpoints.end(), std::bind2nd(Predicate::Endpoint_SessionType(), eSessionType_AudioVideo));
							FPASSERTSTREAM_AND_RETURN(_iendpoint == endpoints.end(), "User:" << user);

							const Endpoint::MediasContainer& medias = _iendpoint->m_medias;
							Endpoint::MediasContainer::const_iterator _imedia = std::find_if(medias.begin(), medias.end(), std::bind2nd(Predicate::Media_Type(), eMediaType_Video));
							FPASSERTSTREAM_AND_RETURN(_imedia == medias.end(), "User:" << user);

							EventVideoSwitchingModeUpdated* pEvent = CreateEvent<EventVideoSwitchingModeUpdated>(*info.user, *_iendpoint, *_imedia);
							pEvent->value.old_val = former_video_param.m_videoMode;
							pEvent->value.new_val = update_video_param.m_videoMode;
							events.insert(std::make_pair(pEvent->Type(), pEvent));
						}
						return;
					}

					if (update_video_param.m_videoMode == eVideoSwitchingModeType_DominantSpeakerSwitched)
					{
						EventVideoSwitchingModeUpdated* pEvent = CreateEvent<EventVideoSwitchingModeUpdated>(User(), Endpoint(), Media());
						pEvent->value.old_val = former_video_param.m_videoMode;
						pEvent->value.new_val = update_video_param.m_videoMode;
						events.insert(std::make_pair(pEvent->Type(), pEvent));
						return;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
ConfInfo* Manager::AddConfInfo(PartyRsrcID id, const char* callId)
{
	FPASSERT_AND_RETURN_VALUE(id == INVALID, NULL);

	ConfInfo* pConfInfo = GetConfInfo(id);
	if (!pConfInfo)
	{
		pConfInfo = new ConfInfo;
		pConfInfo->id = id;
		m_conferences.insert(std::pair<PartyRsrcID, ConfInfo*>(id, pConfInfo));
	}
	if (callId)
		pConfInfo->callId = callId;
	return pConfInfo;
}

//--------------------------------------------------------------------------
ConfInfo* Manager::GetConfInfo(PartyRsrcID id)
{
	Conferences::iterator _iconference = m_conferences.find(id);
	return (_iconference != m_conferences.end()) ? _iconference->second : NULL;
}

//--------------------------------------------------------------------------
ConfInfo* Manager::GetConfInfo(const char* callId)
{
	Conferences::const_iterator _iconference = std::find_if(m_conferences.begin(), m_conferences.end(), std::bind2nd(Predicate::Conference_CallId(), callId));
	return (_iconference != m_conferences.end()) ? _iconference->second : NULL;
}

