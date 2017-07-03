/*
 * EventPackage::Manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: Dmitry Krasnopolsky
 */

#ifndef EVENT_PACKAGE_H_
#define EVENT_PACKAGE_H_

#include "ConfPartySharedDefines.h"
#include "EventPackageInterfaceApiDefines.h"
#include "EventPackageInterfaceApiClasses.h"
#include "EventPackageInterfaceApiEnums.h"
#include "EventPackageApi.h"
#include "Trace.h"
#include "TraceStream.h"
#include "OsQueue.h"
#include "Singleton.h"

#ifndef EP_TRACE
	//#define EV_TERMINAL
	#ifdef EV_TERMINAL
		#define EP_TRACE std::cerr << "\n" << __FUNCTION__ << " - "
	#else
		#define EP_TRACE FTRACEINTO
	#endif
#endif

namespace EventPackage
{
template <typename _T> void delete_second(_T& T) { delete T.second; }

namespace EventHelper
{
	struct User { std::string user; };

	inline std::ostream& operator<<(std::ostream& os, const User& obj)     { return os << "user:" << obj.user; }
	inline CSegment&     operator<<(CSegment& os, const User& obj)         { return os << obj.user; }
	inline CSegment&     operator>>(CSegment& os, User& obj)               { return os >> obj.user; }

	struct Endpoint : public User { std::string endpoint; };

	inline std::ostream& operator<<(std::ostream& os, const Endpoint& obj) { return os << (User&)obj << ", endpoint:" << obj.endpoint; }
	inline CSegment&     operator<<(CSegment& os, const Endpoint& obj)     { return os << (User&)obj << obj.endpoint; }
	inline CSegment&     operator>>(CSegment& os, Endpoint& obj)           { return os >> (User&)obj >> obj.endpoint; }

	struct Media : public Endpoint { std::string media; MediaType type; LyncMsi msi; };

	inline std::ostream& operator<<(std::ostream& os, const Media& obj)    { return os << (Endpoint&)obj << ", media:" << obj.media << ", type:" << obj.type << ", msi:" << obj.msi; }
	inline CSegment&     operator<<(CSegment& os, const Media& obj)        { return os << (Endpoint&)obj << obj.media << obj.type << obj.msi; }
	inline CSegment&     operator>>(CSegment& os, Media& obj)              { return os >> (Endpoint&)obj >> obj.media >> obj.type >> obj.msi; }

	template<typename _TBase, class _TVal>
	struct Value : public _TBase
	{
		_TVal old_val;
		_TVal new_val;
	};

	template<typename _TBase, class _TVal>
	inline std::ostream& operator<<(std::ostream& os, const Value<_TBase, _TVal>& obj)
	{
		return os << (const _TBase&)obj <<", old_val:" << obj.old_val << ", new_val:" << obj.new_val;
	}

	template<typename _TBase, class _TVal>
	inline CSegment& operator<<(CSegment& os, const Value<_TBase, _TVal>& obj)
	{
		return os << (const _TBase&)obj << obj.old_val << obj.new_val;
	}

	template<typename _TBase, class _TVal>
	inline CSegment& operator>>(CSegment& os, Value<_TBase, _TVal>& obj)
	{
		return os >> (_TBase&)obj >> obj.old_val >> obj.new_val;
	}
} /* namespace EventHelper */

////////////////////////////////////////////////////////////////////////////
//                        EventAbstract
////////////////////////////////////////////////////////////////////////////
class EventAbstract
{
public:
	virtual ~EventAbstract() { }

	virtual CSegment& Put(CSegment& seg) = 0;
	virtual CSegment& Get(CSegment& seg) = 0;

	virtual void      Dump(std::ostream& os) const = 0;
	virtual EventType Type() const = 0;
};

////////////////////////////////////////////////////////////////////////////
//                        EventAbstactFactory
////////////////////////////////////////////////////////////////////////////
class EventAbstactFactory
{
public:
	typedef void* (*EventCreatePtr)();

	static void RegisterEvent(EventType event, EventCreatePtr funcPtr)
	{
		_classFactory.insert(std::pair<EventType, EventCreatePtr>(event, funcPtr));
	}

	static void* GenerateEvent(EventType event)
	{
		std::map<EventType, EventCreatePtr>::iterator _itr = _classFactory.find(event);
		return (_itr != _classFactory.end()) ? (_itr->second)() : NULL;
	}

private:
	static std::map<EventType, EventCreatePtr> _classFactory;
};

////////////////////////////////////////////////////////////////////////////
//                        Event
////////////////////////////////////////////////////////////////////////////
template<EventType _TType, class _TValue>
struct Event : public EventAbstract
{
	Event()                                   { EventAbstactFactory::RegisterEvent(_TType, &Create); }

	CSegment&    Put(CSegment& seg)           { return seg << value; }
	CSegment&    Get(CSegment& seg)           { return seg >> value; }

	void         Dump(std::ostream& os) const { os << _TType << " {" << value << "}"; }

	EventType    Type() const                 { return _TType; }
	static void* Create()                     { return new Event<_TType, _TValue>; }

	_TValue      value;
};

inline std::ostream& operator<<(std::ostream& os, const EventAbstract& obj) { obj.Dump(os); return os; }

// User related events
typedef Event<eEventType_UserAdded, EventHelper::User> EventUserAdded;
typedef Event<eEventType_UserDeleted, EventHelper::User> EventUserDeleted;
typedef Event<eEventType_UserDisplayTextUpdated, EventHelper::Value<EventHelper::User, std::string> > EventUserDisplayTextUpdated;

// Endpoint related events
typedef Event<eEventType_EndpointAdded, EventHelper::Endpoint> EventEndpointAdded;
typedef Event<eEventType_EndpointDeleted, EventHelper::Endpoint> EventEndpointDeleted;
typedef Event<eEventType_EndpointDisplayTextUpdated, EventHelper::Value<EventHelper::Endpoint, std::string> > EventEndpointDisplayTextUpdated;
typedef Event<eEventType_EndpointStatusUpdated, EventHelper::Value<EventHelper::Endpoint, EndpointStatusType> > EventEndpointStatusUpdated;

// Media related events
typedef Event<eEventType_MediaAdded, EventHelper::Media> EventMediaAdded;
typedef Event<eEventType_MediaDeleted, EventHelper::Media> EventMediaDeleted;
typedef Event<eEventType_MediaDisplayTextUpdated, EventHelper::Value<EventHelper::Media, std::string> > EventMediaDisplayTextUpdated;
typedef Event<eEventType_MediaStatusUpdated, EventHelper::Value<EventHelper::Media, MediaStatusType> > EventMediaStatusUpdated;

typedef Event<eEventType_VideoSwitchingModeUpdated, EventHelper::Value<EventHelper::Media, VideoSwitchingModeType> > EventVideoSwitchingModeUpdated;

////////////////////////////////////////////////////////////////////////////
//                        Events
////////////////////////////////////////////////////////////////////////////
class Events : public std::multimap<EventType, EventAbstract*>
{
public:
	 Events() : m_id(INVALID) {}
	 Events(PartyRsrcID id) : m_id(id) {}
	~Events() { std::for_each(begin(), end(), &delete_second<Events::value_type>); }

	PartyRsrcID m_id;
};

inline CSegment& operator<<(CSegment& seg, const Events& in)
{
	seg << in.m_id << in.size();
	for (Events::const_iterator _itr = in.begin(); _itr != in.end(); ++_itr)
	{
		seg << _itr->second->Type();
		_itr->second->Put(seg);
	}
	return seg;
}

inline CSegment& operator>>(CSegment& seg, Events& in)
{
	Events::size_type size; seg >> in.m_id >> size;
	for (Events::size_type i = 0; i < size; ++i)
	{
		EventType event; seg >> event;
		EventAbstract* typeExemplar = (EventAbstract*)EventAbstactFactory::GenerateEvent(event);
		FPASSERTSTREAM_AND_RETURN_VALUE(!typeExemplar, "type:" << event, seg);

		seg = typeExemplar->Get(seg);
		in.insert(std::pair<EventType, EventAbstract*>(event, typeExemplar));
	}
	return seg;
}

inline std::ostream& operator<<(std::ostream& os, const Events& in)
{
	os << "\nId:" << in.m_id << ", Size:" << in.size();
	for (Events::const_iterator _itr = in.begin(); _itr != in.end(); ++_itr)
		os << "\n" << *(_itr->second);
	return os;
}

typedef std::multimap<EventType, std::pair<COsQueue*, OPCODE> > Subscribers;
typedef std::map<std::pair<COsQueue*, OPCODE>, Events> Notifications;

struct UserInfo                           { const User* user; };
struct EndpointInfo : public UserInfo     { const Endpoint* endpoint; };
struct MediaInfo    : public EndpointInfo { const Media* media; };

struct ConfInfo
{
	PartyRsrcID id;
	std::string callId;
	Conference  conference;
	Subscribers subscribers;
	LyncDshList dsh; // parsed dsh
	LyncMsiList msi; // unparsed dsh
};

typedef std::map<PartyRsrcID, ConfInfo*> Conferences;

struct Predicate
{
	struct Conference_Entity : public std::binary_function<Conference, std::string, bool>
	{
		bool operator()(const Conference& in, const std::string& key) const { return in.m_entity == key; }
	};
	struct User_Entity : public std::binary_function<User, std::string, bool>
	{
		bool operator()(const User& in, const std::string& key) const { return in.m_entity == key; }
	};
	struct Endpoint_Entity : public std::binary_function<Endpoint, std::string, bool>
	{
		bool operator()(const Endpoint& in, const std::string& key) const { return in.m_entity == key; }
	};
	struct Endpoint_SessionType : public std::binary_function<Endpoint, SessionType, bool>
	{
		bool operator()(const Endpoint& in, const SessionType& key) const { return in.m_sessionType == key; }
	};
	struct Uri_Uri : public std::binary_function<Uri, std::string, bool>
	{
		bool operator()(const Uri& in, const std::string& key) const { return in.m_entity == key; }
	};
	struct Uri_Purpose : public std::binary_function<Uri, std::string, bool>
	{
		bool operator()(const Uri& in, const std::string& key) const { return in.m_purpose.value() == key; }
	};
	struct Media_Msi : public std::binary_function<Media, LyncMsi, bool>
	{
		bool operator()(const Media& in, const LyncMsi& key) const { return in.m_msi == key; }
	};
	struct Media_Id : public std::binary_function<Media, std::string, bool>
	{
		bool operator()(const Media& in, const std::string& key) const { return in.m_id == key; }
	};
	struct Media_Type : public std::binary_function<Media, MediaType, bool>
	{
		bool operator()(const Media& in, const MediaType& key) const { return in.m_type == key; }
	};
	struct Medium_Type : public std::binary_function<ConferenceMedium, MediumType, bool>
	{
		bool operator()(const ConferenceMedium& in, const MediumType& key) const { return in.m_type == key; }
	};
	struct Subscriber : public std::binary_function<Subscribers::iterator::value_type, std::pair<COsQueue*, OPCODE>, bool>
	{
		bool operator()(const Subscribers::iterator::value_type& in, const std::pair<COsQueue*, OPCODE>& key) const { return in.second.first == key.first && in.second.second == key.second; }
	};
	struct Conference_CallId : public std::binary_function<Conferences::iterator::value_type, std::string, bool>
	{
		bool operator()(const Conferences::iterator::value_type& in, const std::string& key) const { return in.second->callId == key; }
	};
	struct DSH_Msi : public std::binary_function<LyncDsh, LyncMsi, bool>
	{
		bool operator()(const LyncDsh& in, const LyncMsi& key) const { return (in.audio.msi == key || in.video.msi == key); }
	};
	struct EntityView_Entity : public std::binary_function<EntityView, std::string, bool>
	{
		bool operator()(const EntityView& in, const std::string& key) const { return in.m_entity == key; }
	};
};

////////////////////////////////////////////////////////////////////////////
//                        Manager
////////////////////////////////////////////////////////////////////////////
class Manager : public SingletonHolder<Manager>
{
	friend class      ApiLync;
	friend class      ApiPlcm;

public:
	static Manager&   Instance() { return SingletonHolder<Manager>::instance(); }

	~Manager()        { std::for_each(m_conferences.begin(), m_conferences.end(), &delete_second<Conferences::value_type>); }

	STATUS            AddConference(PartyRsrcID id, const char* callId, const char* xmlBuffer, size_t xmlLen);
	bool              DelConference(PartyRsrcID id);
	const Conference* GetConference(PartyRsrcID id);

	STATUS            AddSubscriber(PartyRsrcID id, EventType event, const std::pair<COsQueue*, OPCODE>& subscriber);
	STATUS            DelSubscriber(PartyRsrcID id, EventType event, const std::pair<COsQueue*, OPCODE>& subscriber);

	template<typename _Predicate>
	bool              GetUser(PartyRsrcID id, _Predicate predicate, UserInfo& info);

	template<typename _Predicate>
	bool              GetUser(PartyRsrcID id, _Predicate predicate, std::list<UserInfo>& info);

	template<typename _Predicate>
	bool              GetEndpoint(PartyRsrcID id, _Predicate predicate, EndpointInfo& info);

	template<typename _Predicate>
	bool              GetEndpoint(PartyRsrcID id, _Predicate predicate, std::list<EndpointInfo>& info);

	template<typename _Predicate>
	bool              GetMedia(PartyRsrcID id, _Predicate predicate, MediaInfo& info);

	template<typename _Predicate>
	bool              GetMedia(PartyRsrcID id, _Predicate predicate, std::list<MediaInfo>& info);

private:
	ConfInfo*         AddConfInfo(PartyRsrcID id, const char* callId = NULL);
	ConfInfo*         GetConfInfo(PartyRsrcID id);
	ConfInfo*         GetConfInfo(const char* callId);

	template<typename _T>
	void              MergeEntity(_T& dst, const _T& src);

	template<typename _T>
	void              MergeEntity(ApiType<_T>& dst, const ApiType<_T>& src);

	template<typename _T>
	_T*               CreateEvent(const User& user);
	template<typename _T>
	_T*               CreateEvent(const User& user, const Endpoint& endpoint);
	template<typename _T>
	_T*               CreateEvent(const User& user, const Endpoint& endpoint, const Media& media);

	void              BuildEvents(ConfInfo* pConfInfo, const Conference& former, const Conference& update);
	void              BuildEvents(Events& events, const Users::UsersContainer& former, const Users::UsersContainer& update);
	void              BuildEvents(Events& events, const User::EndpointsContainer& former, const User::EndpointsContainer& update, const User& user);
	void              BuildEvents(Events& events, const Endpoint::MediasContainer& former, const Endpoint::MediasContainer& update, const User& user, const Endpoint& endpoint);
	void              BuildEvents(Events& events, const ConferenceView::EntityViewsContainer& former, const ConferenceView::EntityViewsContainer& update);

private:
	Conferences       m_conferences;
};

#include "EventPackageImpl.h" // Should be the last here because of template instantiation

}  /* namespace EventPackage */

#endif /* EVENT_PACKAGE_H_ */
