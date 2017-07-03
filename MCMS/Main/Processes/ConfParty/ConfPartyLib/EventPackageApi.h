/*
 * EventPackageApi.h
 *
 *  Created on: Jan 23, 2014
 *      Author: Dmitry Krasnopolsky
 */

#ifndef EVENTPACKAGEAPI_H_
#define EVENTPACKAGEAPI_H_

#include "ConfPartySharedDefines.h"
#include "EventPackageInterfaceApiDefines.h"
#include "EventPackageInterfaceApiClasses.h"
#include "EventPackageInterfaceApiEnums.h"
#include "Trace.h"
#include "TraceStream.h"
#include "Singleton.h"

namespace EventPackage
{

////////////////////////////////////////////////////////////////////////////
//                        LyncDsh
////////////////////////////////////////////////////////////////////////////
struct LyncDsh
{
	struct User     { std::string id; std::string displayText; };
	struct Endpoint { std::string id; std::string displayText; };
	struct Media    { std::string id; LyncMsi msi; MediaStatusType status; };

	User     user;
	Endpoint endpoint;
	Media    audio;
	Media    video;
};

typedef std::vector<LyncDsh> LyncDshList;
typedef std::vector<LyncMsi> LyncMsiList;

////////////////////////////////////////////////////////////////////////////
//                        Api
////////////////////////////////////////////////////////////////////////////
class Api
{
};

////////////////////////////////////////////////////////////////////////////
//                        ApiLync
////////////////////////////////////////////////////////////////////////////
class ApiLync : public Api, public SingletonHolder<ApiLync>
{
public:
	static ApiLync& Instance() { return SingletonHolder<ApiLync>::instance(); }

	STATUS          AddDSH(PartyRsrcID id, const LyncMsi* dshBuffer, size_t dshLen);
	void            GetDSH(PartyRsrcID id, LyncDshList& dshList);

	LyncMsi         GetCorrelativeMSI(PartyRsrcID id, LyncMsi audioMsi, MediaType mediaType);
	LyncMsi         GetUserMSI(PartyRsrcID id, const std::string& userId, MediaType mediaType, bool activeOnly = true);
	void            GetMsiList(PartyRsrcID id, LyncMsiList& msiList, MediaType mediaType, bool activeOnly = true);
	std::string     GetSpotlightUserId(PartyRsrcID id);
};

////////////////////////////////////////////////////////////////////////////
//                        ApiPlcm
////////////////////////////////////////////////////////////////////////////
class ApiPlcm : public Api, public SingletonHolder<ApiPlcm>
{
public:
	static ApiPlcm& Instance() { return SingletonHolder<ApiPlcm>::instance(); }
};

} /* namespace EventPackage */

#endif /* EVENTPACKAGEAPI_H_ */
