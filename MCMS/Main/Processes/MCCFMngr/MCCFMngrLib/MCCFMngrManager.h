#ifndef MCCF_MNGR_MANAGER_H__
#define MCCF_MNGR_MANAGER_H__

//////////////////////////////////////////////////////////////////////
#include "ManagerTask.h"
#include "Macros.h"

#include "Segment.h"
#include "MplMcmsProtocol.h"

#include "OsQueue.h"

#include "SipMsg.h"

#include <string>
#include <iosfwd>
#include <map>
#include <set>

//////////////////////////////////////////////////////////////////////
struct SipCallID;

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const mcTransportAddress& a);

//////////////////////////////////////////////////////////////////////
struct AppServerContext
{
	typedef std::string IpAddress;
	typedef HANDLE      AppServerID;
	typedef std::string DialogID;

	SipCallID   callID;      // unique
	AppServerID appServerID; // unique
	IpAddress   ip;          // unique
	DialogID    dialogID;    // unique

	COsQueue    mailbox;     // MccfRxSocket's mailbox for incoming messages

	AppServerContext()
		: appServerID(0)
	{}

	AppServerContext(AppServerID id, const mcTransportAddress& ip, const std::string& dialogID);

	AppServerContext(AppServerID id, const IpAddress& ip, const std::string& dialogID)
		: appServerID(id)
		, ip(ip)
		, dialogID(dialogID)
	{}
};

//////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const AppServerContext& obj);
CSegment& operator >>(CSegment& seg, AppServerContext& obj);

//////////////////////////////////////////////////////////////////////
void MCCFMngrManagerEntryPoint(void* appParam);
void MCCFMngrMonitorEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
class CMCCFMngrManager : public CManagerTask
{
	CLASS_TYPE_1(CMCCFMngrManager, CManagerTask)

public:

	typedef std::map<SipCallID, AppServerContext*> AppServersMap;

	typedef DWORD AlarmID;

	typedef std::map<AppServerContext::IpAddress, AlarmID> AlarmsMap;

public:

	CMCCFMngrManager();
	virtual ~CMCCFMngrManager() {}

private: // helpers

	WORD MccfListenPort() const;

	bool ParseSipInviteMsg(const CMplMcmsProtocol& protocol, SipCallID& sipCallID);
	bool BuildSipInviteACK(const CMplMcmsProtocol& protocol, OPCODE& opcode, CSegment& segOut) const;
	void SendToCS(const SipCallID& sipCallID, OPCODE opcode, CSegment& segOut) const;

	void DestroyAppServerContext(AppServersMap::iterator it);

	void AddAlarm(const AppServerContext& ctx, const std::string& description);
	void AddAlarm(const SipCallID& callID, const std::string& description);
	void RemoveAlarm(const SipCallID& callID);

	void SendMccfDrop(const AppServerContext& ctx);

	void StartKeepAlive();

private: // overrides from base classes

	virtual TaskEntryPoint GetMonitorEntryPoint()
	{ return MCCFMngrMonitorEntryPoint; }

	virtual void ManagerPostInitActionsPoint();

private: // Actions

	void OnCsApi(CSegment* pSeg);
	void OnManagementInterfaceIND(CSegment* pSeg);

	void OnCsKeepAliveTimer(CSegment* pSeg);

	void OnMccfChannelOpened(CSegment* pSeg);
	void OnMccfChannelClosed(CSegment* pSeg);

private: // protocol handlers

	void OnSipInvite(const CMplMcmsProtocol& protocol);
	void OnSipInviteACK(const CMplMcmsProtocol& protocol);

	void OnSipBye(const CMplMcmsProtocol& protocol);
	void OnSipByeACK(const CMplMcmsProtocol& protocol);

	void OnSipSessionTimer(const CMplMcmsProtocol& protocol);
	void OnSipTransportError(const CMplMcmsProtocol& protocol);
	void OnSipBadStatus(const CMplMcmsProtocol& protocol);

private: // Terminal commands

	STATUS HandleAppServers(CTerminalCommand& command, std::ostream& answer);

#ifdef USE_CUSTOM_MEM_TRACKER
	STATUS HandleMemTrack(CTerminalCommand& command, std::ostream& answer);
#endif

private: // maps and miscellaneous

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

	// disallow copying
	CMCCFMngrManager(const CMCCFMngrManager&);
	void operator=(const CMCCFMngrManager&);

private:

	typedef std::map<AppServerContext::DialogID, AppServerContext*> IndexByDialogID;
	typedef std::map<AppServerContext::AppServerID, AppServerContext*> IndexByAppSrvID;

private: // data members

	mcTransportAddress managementIP_; // in Host byte order

	AppServersMap      appServers_;
	IndexByDialogID    idxDlg_;
	IndexByAppSrvID    idxAppSrvID_;

	AlarmsMap          alarms_;

};

//////////////////////////////////////////////////////////////////////
#endif // MCCF_MNGR_MANAGER_H__
