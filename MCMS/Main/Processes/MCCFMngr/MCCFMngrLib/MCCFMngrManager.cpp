#include "MCCFMngrManager.h"

#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"

#include "ManagerApi.h"
#include "ListenSocketApi.h"

#include "SipMsg.h"
#include "MplMcmsProtocol.h"

#include "MccfRxSocket.h"
#include "MccfTxSocket.h"

#include "MccfPackageFactory.h"

#include "TerminalCommand.h"

#include "MccfPackagesRegistrar.h"

#include "FaultsDefines.h"

#include "ConfigHelper.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"

#include "Trace.h"
#include "TraceStream.h"

#include <arpa/inet.h> // ntohl, inet_addr
#include <memory> // auto_ptr
#include <ostream>

//////////////////////////////////////////////////////////////////////
enum MccfMngrStateEnum {
	STARTUP = 1,
	READY,
	DISABLED,
};

//////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMCCFMngrManager)
	ONEVENT(XML_REQUEST,                  ANYCASE,  CMCCFMngrManager::HandlePostRequest)
	ONEVENT(CSAPI_MSG,                    READY,    CMCCFMngrManager::OnCsApi) // Dispatches messages from CS
	ONEVENT(CSAPI_MSG,                    DISABLED, CMCCFMngrManager::OnCsApi)
	ONEVENT(MCCF_MNGMNT_INTERFACE_IP_IND, STARTUP,  CMCCFMngrManager::OnManagementInterfaceIND)
	ONEVENT(MCCF_CS_KEEP_ALIVE_TIMER,     READY,    CMCCFMngrManager::OnCsKeepAliveTimer)
	ONEVENT(MCCF_CHANNEL_OPENED,          READY,    CMCCFMngrManager::OnMccfChannelOpened) // notification on MCCF channel being opened
	ONEVENT(MCCF_CHANNEL_CLOSED,          READY,    CMCCFMngrManager::OnMccfChannelClosed) // notification on MCCF channel being closed
	ONEVENT(OPEN_SOCKET_CONNECTION,       READY,    CMCCFMngrManager::NullActionFunction)
	ONEVENT(CLOSE_SOCKET_CONNECTION,      READY,    CMCCFMngrManager::NullActionFunction)
PEND_MESSAGE_MAP(CMCCFMngrManager, CManagerTask);

//////////////////////////////////////////////////////////////////////
BEGIN_SET_TRANSACTION_FACTORY(CMCCFMngrManager)
END_TRANSACTION_FACTORY

//////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CMCCFMngrManager)
	ONCOMMAND("app_servers", CMCCFMngrManager::HandleAppServers, "Manage the over-MCCF Application Servers")
#ifdef USE_CUSTOM_MEM_TRACKER
	ONCOMMAND("mem_track",   CMCCFMngrManager::HandleMemTrack,   "Dump dynamic memory usage statistics")
#endif
END_TERMINAL_COMMANDS

//////////////////////////////////////////////////////////////////////
void MCCFMngrManagerEntryPoint(void* appParam)
{
	CMCCFMngrManager* mngr = new CMCCFMngrManager;
	mngr->Create(*reinterpret_cast<CSegment*>(appParam));
}

//////////////////////////////////////////////////////////////////////
AppServerContext::AppServerContext(AppServerID id, const mcTransportAddress& ip, const std::string& dialogID)
	: appServerID(id)
	, dialogID(dialogID)
{
	std::ostringstream ostr;
	ostr << ip;
	this->ip = ostr.str();
}

//////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const mcTransportAddress& obj)
{
	return seg << obj.ipVersion << obj.addr.v4.ip; // TODO: fix to support IPv6
}

//////////////////////////////////////////////////////////////////////
CSegment& operator >>(CSegment& seg, mcTransportAddress& obj)
{
	return seg >> obj.ipVersion >> obj.addr.v4.ip; // TODO: fix to support IPv6
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const mcTransportAddress& a)
{
	if (eIpVersion4 == a.ipVersion)
	{
		APIU32 address = htonl(a.addr.v4.ip); // translate to Network byte order to ensure the Big Endian
		const byte* ip = reinterpret_cast<const byte*>(&address);
		ostr << "IPv4: " << int(ip[0]) << '.' << int(ip[1]) << '.' << int(ip[2]) << '.' << int(ip[3]);

		if (a.port)
			ostr << ':' << a.port;
	}
	else if (eIpVersion6 == a.ipVersion)
	{
		ostr << "IPv6: ";
	}
	else
	{
		FPASSERT(a.ipVersion);
	}

	return ostr;
}

//////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const AppServerContext& obj)
{
	return seg << obj.callID << obj.appServerID << obj.ip << obj.dialogID;
}

//////////////////////////////////////////////////////////////////////
CSegment& operator >>(CSegment& seg, AppServerContext& obj)
{
	return seg >> obj.callID >> obj.appServerID >> obj.ip >> obj.dialogID;
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const SipCallID& obj)
{
	ostr << "SipCall { s:" << obj.serviceID << ", u:" << obj.unitID << ", c:" << obj.callID << " }";
	return ostr;
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const AppServerContext& obj)
{
	return ostr << "AppSrvCtx: { " << obj.callID << ", id:" << obj.appServerID << ", dlg:" << obj.dialogID << ", " << obj.ip << " }";
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CMCCFMngrManager::AppServersMap& obj)
{
	ostr << "AppServers: " << obj.size() << '\n';

	for (CMCCFMngrManager::AppServersMap::const_iterator it = obj.begin(); it != obj.end(); ++it)
		ostr << '\t' << *(it->second) << '\n';

	return ostr;
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CMCCFMngrManager::AlarmsMap& obj)
{
	ostr << "Alarms: " << obj.size() << '\n';

	for (CMCCFMngrManager::AlarmsMap::const_iterator it = obj.begin(); it != obj.end(); ++it)
		ostr << '\t' << "{ " << it->first << ", " << it->second << " }" << '\n';

	return ostr;
}

//////////////////////////////////////////////////////////////////////
CMCCFMngrManager::CMCCFMngrManager()
{
	m_state = STARTUP;
	memset(&managementIP_, 0, sizeof(managementIP_));
	managementIP_.port = MccfListenPort(); // listen on this port
	CMccfPackagesRegistrar::RegisterAllPackages();
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::ManagerPostInitActionsPoint()
{
	CProcessBase* proc = CProcessBase::GetProcess();


		proc->m_NetSettings.LoadFromFile();
		managementIP_.addr.v4.ip = proc->m_NetSettings.m_ipv4;
		OnManagementInterfaceIND(NULL);

	//UnlockRelevantSemaphore();
	//TestAndEnterFipsMode();
	//LockRelevantSemaphore();
}

//////////////////////////////////////////////////////////////////////
bool CMCCFMngrManager::ParseSipInviteMsg(const CMplMcmsProtocol& protocol, SipCallID& sipCallID)
{
	CFWID cfwID = {};
	mcTransportAddress appServerAddress;

	bool bOk = ParseCsInviteIndEvent(protocol, cfwID, appServerAddress, sipCallID);

	if (READY == m_state)
	{
		AppServerContext* ctx = new AppServerContext(0, appServerAddress, cfwID);
		ctx->callID = sipCallID;

		idxDlg_.insert(std::make_pair(cfwID, ctx));
		appServers_.insert(std::make_pair(sipCallID, ctx));

		TRACEINTO << appServers_;
	}

	PASSERT(!bOk);
	return bOk;
}

//////////////////////////////////////////////////////////////////////
bool CMCCFMngrManager::BuildSipInviteACK(const CMplMcmsProtocol& protocol, OPCODE& opcode, CSegment& segOut) const
{
	return BuildSipInviteResponseReq(protocol, (READY == m_state ? SipCodesOk : SipCodesUnsuppMediaType), managementIP_, segOut, opcode);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::SendToCS(const SipCallID& sipCallID, OPCODE opcode, CSegment& segOut) const
{
	SendSipMsgToCS(opcode, segOut, sipCallID);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::SendMccfDrop(const AppServerContext& ctx)
{
	TRACEINTO << ctx;

	CTaskApi api;
	api.CreateOnlyApi(ctx.mailbox);
	api.SendMsg(NULL, MCCF_CHANNEL_DROP);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::DestroyAppServerContext(AppServersMap::iterator it)
{
	delete it->second;
	appServers_.erase(it);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::AddAlarm(const AppServerContext& ctx, const std::string& description)
{
	AlarmID alarmID = AddActiveAlarm(
		FAULT_GENERAL_SUBJECT,
		AA_MCCF_CHANNEL_IS_NOT_CONNECTED,
		MAJOR_ERROR_LEVEL,
		description + " from " + ctx.ip,
		true,
		true);

	alarms_.insert(std::make_pair(ctx.ip, alarmID));
	TRACEINTO << ctx << '\n' << appServers_ << '\n' << alarms_;

	idxDlg_.erase(ctx.dialogID);
	idxAppSrvID_.erase(ctx.appServerID);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::AddAlarm(const SipCallID& callID, const std::string& description)
{
	AppServersMap::iterator it = appServers_.find(callID);
	FPASSERTSTREAM_AND_RETURN(it == appServers_.end(), callID << '\n' << appServers_);

	const AppServerContext& ctx = *it->second;

	AddAlarm(ctx, description);

	SendMccfDrop(ctx);

	DestroyAppServerContext(it);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::RemoveAlarm(const SipCallID& callID)
{
	AppServersMap::const_iterator it = appServers_.find(callID);
	FPASSERTSTREAM_AND_RETURN(it == appServers_.end(), "Unexpected " << callID << '\n' << appServers_);

	AlarmsMap::iterator alarm_it = alarms_.find(it->second->ip);

	if (alarm_it != alarms_.end())
	{
		TRACEINTO << *it->second;
		RemoveActiveAlarmById(alarm_it->second);
		alarms_.erase(alarm_it);
	}
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::StartKeepAlive()
{
	if (IsValidTimer(MCCF_CS_KEEP_ALIVE_TIMER))
		return;

	if (GetSystemCfgFlagInt<bool>(CFG_KEY_SIP_IS_KEEPALIVE_ENABLE))
		StartTimer(MCCF_CS_KEEP_ALIVE_TIMER, 0);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnCsApi(CSegment* pSeg)
{
	CMplMcmsProtocol protocol;
	protocol.DeSerialize(*pSeg, CS_API_TYPE);
	OPCODE opcode = protocol.getOpcode();

	ConnectionID connectionID = protocol.getPortDescriptionHeaderConnection_id();
	PASSERTSTREAM_AND_RETURN(MCCF_CONNECTION_ID != connectionID, "Unexpected connection ID:" << connectionID);

	if (opcode != SIP_CS_MCCF_SIG_SESSION_TIMER_EXPIRED_IND) {
		PASSERTSTREAM_AND_RETURN(!protocol.getpData(), "Missing protocol data for opcode:" << opcode);
	}

	switch (opcode)
	{
	case SIP_CS_MCCF_SIG_INVITE_IND:
		return OnSipInvite(protocol);

	case SIP_CS_MCCF_SIG_INVITE_ACK_IND:
		return OnSipInviteACK(protocol);

	case SIP_CS_MCCF_SIG_BYE_IND:
		return OnSipBye(protocol);

	case SIP_CS_MCCF_SIG_BYE_200_OK_IND:
		return OnSipByeACK(protocol);

	case SIP_CS_MCCF_SIG_SESSION_TIMER_EXPIRED_IND:
		return OnSipSessionTimer(protocol);

	case SIP_CS_MCCF_SIG_TRANSPORT_ERROR_IND:
		return OnSipTransportError(protocol);

	case SIP_CS_MCCF_SIG_BAD_STATUS_IND:
		return OnSipBadStatus(protocol);

	default:
		PASSERTSTREAM(true, "Unexpected opcode:" << opcode);
	}
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnManagementInterfaceIND(CSegment* pSeg)
{
	bool bIPv4 = true; // currently support IPv4 only

	managementIP_.ipVersion = bIPv4 ? eIpVersion4 : eIpVersion6;
	managementIP_.transportType = eTransportTypeTcp;
	managementIP_.distribution = eDistributionUnicast;

	if (!IsTarget())
	{
		CProcessBase* proc = CProcessBase::GetProcess();
		if (!bIPv4)
			managementIP_.addr.v6 = proc->m_NetSettings.m_ipv6_0.addr;

	}

	if (GetSystemCfgFlagInt<bool>(CFG_KEY_ENABLE_MCCF))
	{
		CListenSocketApi api(&MccfRxEntryPoint, &MccfTxEntryPoint, managementIP_);

		const size_t MAX_MCCF_CONNECTIONS = 10;
		api.SetMaxNumConnections(MAX_MCCF_CONNECTIONS);
		api.Create(*m_pRcvMbx);

		MccfHandleManager::instance().reserve(MAX_MCCF_CONNECTIONS * 10);

		m_state = READY;
	}
	else
		m_state = DISABLED;

	TRACEINTO << managementIP_ << ", State:" << m_state;
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnCsKeepAliveTimer(CSegment* /*pSeg*/)
{
	TRACEINTO << appServers_;

	if (appServers_.empty())
		return;

	CSegment seg;

	for (AppServersMap::const_iterator it = appServers_.begin(); it != appServers_.end(); ++it)
		SendToCS(it->first, SIP_CS_PARTY_KEEP_ALIVE_REQ, seg);

	DWORD timeout = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MSG_TIMEOUT) + 5;
	StartTimer(MCCF_CS_KEEP_ALIVE_TIMER, timeout * SECOND);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnMccfChannelOpened(CSegment* pSeg)
{
	COsQueue& rxQueue = *m_pClientRspMbx;

	AppServerContext ctx;
	*pSeg >> ctx;

	IndexByDialogID::const_iterator it = idxDlg_.find(ctx.dialogID);

	const bool ok = (it != idxDlg_.end());

	if (ok)
	{
		// update the AppServer record according to the context
		idxAppSrvID_.insert(std::make_pair(ctx.appServerID, it->second));
		it->second->appServerID = ctx.appServerID;
		it->second->mailbox = rxQueue;

		TRACEINTO << *it->second;
	}
	else
		TRACEINTO << "No AppServer found for DialogID:" << ctx.dialogID << '\n' << appServers_;

	CTaskApi api;
	api.CreateOnlyApi(rxQueue);
	api.SendMsg(NULL, ok ? MCCF_CHANNEL_SYNC : MCCF_CHANNEL_DROP);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnMccfChannelClosed(CSegment* pSeg)
{
	AppServerContext::AppServerID appServerID = 0;
	*pSeg >> appServerID;

	// find the AppServer according to the ID
	IndexByAppSrvID::const_iterator it = idxAppSrvID_.find(appServerID);
	TRACECOND_AND_RETURN(it == idxAppSrvID_.end(), "No AppServer found for ID:" << appServerID << '\n' << appServers_);

	const AppServerContext& ctx = *it->second;
	AppServersMap::iterator it_srv = appServers_.find(ctx.callID);

	if (it_srv != appServers_.end())
	{
		AddAlarm(ctx, "MCCF disconnected");

		OPCODE opcode;
		CSegment segOut;

		BuildSipByeReq(segOut, opcode);
		SendToCS(ctx.callID, opcode, segOut);

		DestroyAppServerContext(it_srv);
	}
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipInvite(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID;

	if (ParseSipInviteMsg(protocol, sipCallID))
	{
		OPCODE opcode;
		CSegment segOut;

		if (BuildSipInviteACK(protocol, opcode, segOut))
			SendToCS(sipCallID, opcode, segOut);
		else
			PASSERT(opcode);
	}
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipInviteACK(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID;
	DWORD outStatus;

	bool bOk = ParseCsInviteAckIndEvent(protocol, outStatus, sipCallID);
	TRACEINTO << "Status:" << outStatus << ", " << sipCallID;

	if (bOk && READY == m_state)
	{
		switch (outStatus)
		{
			case STATUS_OK:
			case SipCodesOk:
				RemoveAlarm(sipCallID);
				StartKeepAlive();
				break;
		}
	}

	PASSERT(!bOk);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipBye(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID;
	DWORD outStatus;

	bool bOk = ParseCsByeIndEvent(protocol, outStatus, sipCallID);
	TRACEINTO << "Status:" << outStatus << ", " << sipCallID;
	PASSERT_AND_RETURN(!bOk);

	OPCODE opcode;
	CSegment msgOut;

	if (BuildSipBye200OKReq(msgOut, opcode))
		SendToCS(sipCallID, opcode, msgOut);

	AddAlarm(sipCallID, "MCCF disconnected with SIP Bye");
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipByeACK(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID;
	DWORD outStatus;

	bool bOk = ParseCsBye200OKIndEvent(protocol, outStatus, sipCallID);
	TRACEINTO << "Status:" << outStatus << ", " << sipCallID;

	PASSERT(!bOk);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipSessionTimer(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID(protocol);

	TRACEINTO << sipCallID;
	AddAlarm(sipCallID, "MCCF disconnected on SIP Session Timer expired");
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipTransportError(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID;
	DWORD outStatus;
	const char* szError = NULL;

	bool bOk = ParseCsTransportErrorIndEvent(protocol, outStatus, szError, sipCallID);
	TRACEINTO << "Status:" << outStatus << ", " << sipCallID << ", Error:" << szError;
	AddAlarm(sipCallID, "MCCF disconnected on SIP Transport error");

	PASSERT(!bOk);
}

//////////////////////////////////////////////////////////////////////
void CMCCFMngrManager::OnSipBadStatus(const CMplMcmsProtocol& protocol)
{
	SipCallID sipCallID;
	DWORD opcode;
	const char* szError = NULL;

	bool bOk = ParseCsBadStatusIndEvent(protocol, opcode, szError, sipCallID);
	TRACEINTO << "Opcode:" << opcode << ", " << sipCallID << ", Error:" << szError;

	PASSERT(!bOk);
}

//////////////////////////////////////////////////////////////////////
WORD CMCCFMngrManager::MccfListenPort() const
{
	const WORD MccfListeningPort = 7563; // RFC 6230, Section 13.5
	return MccfListeningPort;
}

/////////////////////////////////////////////////////////////////////
STATUS CMCCFMngrManager::HandleAppServers(CTerminalCommand& command, std::ostream& answer)
{
	const DWORD count = command.GetNumOfParams();

	const string& cmd = command.GetToken(eCmdParam1);

	if (!count || cmd.empty() || "dump" == cmd)
		answer << appServers_;

	else if ("requests" == cmd)
		answer << MccfHandleManager::const_instance();

	else
	{
		answer << "Command '" << cmd << "' is not (yet) implemented.";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

#ifdef USE_CUSTOM_MEM_TRACKER
/////////////////////////////////////////////////////////////////////
STATUS CMCCFMngrManager::HandleMemTrack(CTerminalCommand& command, std::ostream& answer)
{
	answer << CustomMemTracker::MemoryManager::instance();

	return STATUS_OK;
}
#endif

/////////////////////////////////////////////////////////////////////
