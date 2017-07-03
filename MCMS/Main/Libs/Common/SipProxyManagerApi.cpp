#include <stdio.h>
#include "SipProxyManagerApi.h"
#include "OsQueue.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "StructTm.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::AddConference(DWORD serviceId, const char* pName, DWORD confId, BYTE isEQ, DWORD duration) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId << ", Duration:" << duration << ", IsEQ:" << (int)isEQ;

	CSegment* pRetParam = new CSegment;
	*pRetParam
		<< confId
		<< pName
		<< (BYTE)TRUE  //on-going
		<< (BYTE)FALSE //not MR
		<< (BYTE)isEQ  //not EQ
		<< (BYTE)FALSE //not Factory
		<< (BYTE)FALSE //not GW
		<< duration;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::AddMR(DWORD serviceId, const char* pName, DWORD confId, DWORD duration) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId << ", Duration:" << duration;

	CSegment* pRetParam = new CSegment();
	*pRetParam
		<< confId
		<< pName
		<< (BYTE)FALSE //not on-going
		<< (BYTE)TRUE  //MR
		<< (BYTE)FALSE //not EQ
		<< (BYTE)FALSE //not Factory
		<< (BYTE)FALSE //not GW
		<< duration;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::AddEQ(DWORD serviceId, const char* pName, DWORD confId, DWORD duration) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId << ", Duration:" << duration;

	CSegment* pRetParam = new CSegment();
	*pRetParam
		<< confId
		<< pName
		<< (BYTE)FALSE //not on-going
		<< (BYTE)FALSE //not MR
		<< (BYTE)TRUE  //EQ
		<< (BYTE)FALSE //not Factory
		<< (BYTE)FALSE //not GW
		<< duration;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::AddFactory(DWORD serviceId, const char* pName, DWORD confId, DWORD duration) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId << ", Duration:" << duration;

	CSegment* pRetParam = new CSegment();
	*pRetParam
		<< confId
		<< pName
		<< (BYTE)FALSE //not on-going
		<< (BYTE)FALSE //not MR
		<< (BYTE)FALSE //not EQ
		<< (BYTE)TRUE  //Factory
		<< (BYTE)FALSE //not GW
		<< duration;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::AddGW(DWORD serviceId, const char* pName, DWORD confId, DWORD duration) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId << ", Duration:" << duration;

	CSegment* pRetParam = new CSegment();
	*pRetParam
		<< confId
		<< pName
		<< (BYTE)FALSE //not on-going
		<< (BYTE)FALSE //not MR
		<< (BYTE)FALSE //not EQ
		<< (BYTE)FALSE //not Factory
		<< (BYTE)TRUE  //GW
		<< duration;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::DelConference(DWORD serviceId, const char* pName, DWORD confId) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId;

	CSegment* pRetParam = new CSegment();
	*pRetParam
		<< confId
		<< pName;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, TERMINATE_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSipProxyManagerApi::ChangePresenceStatus(DWORD serviceId, const char* pName, DWORD confId, BYTE presenceState) const
{
	TRACEINTO  << "ServiceId:" << serviceId << ", ConfName:" << DUMPSTR(pName) << ", ConfMonitorId:" << confId << ", Status:" << (int)presenceState;

	CSegment* pRetParam = new CSegment;
	*pRetParam
		<< confId
		<< pName
		<< presenceState;

	CSipProxyTaskApi api(serviceId);
	return api.SendMsg(pRetParam, SIP_PROXY_CHANGE_PRESENCE_STATE);
}

