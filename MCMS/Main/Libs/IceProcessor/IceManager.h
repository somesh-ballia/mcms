#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _ICE_MANAGER_HXX_
#define _ICE_MANAGER_HXX_


#include <memory>
#include <map>
#include <sys/types.h>
//#include "SipUtil.h"
#include "AnyFirewallEngine_dll.h"
//#include "AnyFirewallEngine.h"
//#include "voipapp.h"
//#include "networkhandler.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "IceProcessor.h"

using std::string;


class IceManager;
class CMplMcmsProtocol;
class CSegment;
class IceSession;


class CEventCallback : public IAfCallbackHandler
{
public:
	CEventCallback(IceManager*);
	void HandleCallback(int, char*, int, const CAfStdString&, int, const void*);

private:
	IceManager	*m_iceManager;
};

class IceManager : public IceProcessor
{
public:
	// init ICE module
	bool 	Init(CMplMcmsProtocol &mplMsg);
	bool	MakeOffer(CMplMcmsProtocol &mplMsg, int &iceSessionID);
	bool	MakeAnswer(CMplMcmsProtocol &mplMsg, int &iceSessionID);
	bool MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg);
	bool MakeOfferNoCreate(CMplMcmsProtocol &mplMsg);
	bool 	CloseSession(CMplMcmsProtocol &mplMsg);
	//bool 	ModifySession(void*, int opcode);
	bool	ProcessAnswer(CMplMcmsProtocol &mplMsg);

	CAnyFirewallEngine* GetAFEngine() const {return m_pAFEngine;};
	void	SetFirewallTypeDetectionStatus(bool status) {m_isFirewallTypeDetected=status;};
	bool IsFirewallTypeDetected() const {return m_isFirewallTypeDetected;};

	void OnTimerCheckComplete (CSegment * pParam) {};

	bool SendIceInitInd(EAfDetectedFirewallType type);

	IceManager(IceMplMsgSender &mplSender);
	~IceManager();

	IceSession* GetIceSession(int iceSessionID);
	
protected:
	void	SetStunPassParams(ICE_SERVER_PARAMS_S*);
	void	SetUserPasswordRealParams(ICE_SERVER_TYPES_S*);
	void	SetStunUdpParams(ICE_SERVER_PARAMS_S*);
	void    SetStunTcpParams(ICE_SERVER_PARAMS_S*);
	void	SetRelayUdpParams(ICE_SERVER_PARAMS_S*);
	void	SetRelayTcpParams(ICE_SERVER_PARAMS_S*);
	BOOL	StartServersConfig();

	IceSession* 	NewSession();
	//void*	HandleEventCallback(int, char*, int, const char*, int);

	bool NewAnswerSession(CMplMcmsProtocol &mplMsg);
	bool NewOfferSession(CMplMcmsProtocol &mplMsg);

	//int		m_AudioChannel[MaxSessions];
	//int		m_VideoChannel[MaxSessions];
	//int		m_FeccChannel[MaxSessions];
	//int		m_ContentChannel[MaxSessions];

private:
	CAnyFirewallEngine 	*m_pAFEngine;	// The AFE class

	string m_sSTUNPassServerAddress;
	u_int16_t m_iSTUNPassServerPort;
	string	m_sSTUNPassServerUsername;
	string m_sSTUNPassServerPassword;
	string m_sSTUNPassServerRealm;

	string m_sSTUNUdpServerAddress;
	u_int16_t m_iSTUNUdpServerPort;
	string	m_sSTUNTcpServerAddress;
	u_int16_t m_iSTUNTcpServerPort;

	string m_sSTUNRelayUdpServerAddress;
	u_int16_t m_iSTUNRelayUdpServerPort;
	string m_sSTUNRelayTcpServerAddress;
	u_int16_t m_iSTUNRelayTcpServerPort;

	string m_sHttpProxyAddress;
	u_int16_t m_uHttpProxyPort;
	string m_sHttpProxyUsername;
	string m_sHttpProxyPassword;
	string m_sHttpProxyDomain;

	int 	m_iEventChannel;// The event channel


	//IceSession	*m_iceSession;

	bool m_bAFEngineInit;	// Is AFE initialized?
	bool m_bIsMSOCS;

	typedef std::map<int, IceSession*> ICE_SessionMap;
	ICE_SessionMap m_IceSessionTable;
	//indexTableStruct    m_sessionIndexTable;
	//indexTableEntry     m_sessionIndexTableArray[MaxSessions];

	bool	m_isFirewallTypeDetected;
	EAfDetectedFirewallType m_FireWallType;


	std::auto_ptr<CMplMcmsProtocol>	m_lastMplMcmsMsg;
	int	m_lastIceSessionID;
};

#endif

#endif	//__DISABLE_ICE__
