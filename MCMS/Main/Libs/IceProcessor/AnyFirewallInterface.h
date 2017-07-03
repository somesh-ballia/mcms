
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__
#ifndef ANYFIREWALL_INTERFACE_H
#define ANYFIREWALL_INTERFACE_H

// use date of interface change -- for version compatibility checks
#define AF_INTERFACE_VERSION 200802130
#define AF_DLL_VERSION	7514126

#include <string>
#include <string.h>
#include <vector>

class CAfStdString;

#define AF_CHANNEL_LOOPBACK  "loopback"
#define AF_CHANNEL_UDP       "udp"
#define AF_CHANNEL_RTP       "rtp"
#define AF_CHANNEL_RTCP      "rtcp"
#define AF_CHANNEL_TCP       "tcp"
#define AF_CHANNEL_TLS       "tls"
#define AF_CHANNEL_DATA      "data"
#define AF_CHANNEL_EVENT     "event"
#define AF_CHANNEL_DNS       "dns"
#define AF_CHANNEL_INVALID	-1

#define AF_HOST_PUBLIC       "public"
#define AF_HOST_LOCAL        "local"
#define AF_HOST_FIREWALL     "firewall"
#define AF_HOST_RELAY        "relay"
#define AF_HOST_UPNP		 "UPnP"
#define AF_HOST_DNS_SRV      "dns-srv"

#define AF_PROTOCOL_UDP		 AF_CHANNEL_UDP
#define AF_PROTOCOL_TCP		 AF_CHANNEL_TCP
#define AF_PROTOCOL_TLS		 AF_CHANNEL_TLS

#define AF_SELECT_NOEVENT	 0x0000
#define AF_SELECT_READ		 0x0001
#define AF_SELECT_WRITE		 0x0004
#define AF_SELECT_ERROR      0x8000

#define AF_TIMEOUT_INFINITE	-1
#define AF_BLOCKING         -1
#define AF_NON_BLOCKING      0

#define AF_MEDIA_STREAM_AUDIO       "audio"
#define AF_MEDIA_STREAM_VIDEO       "video"
#define AF_MEDIA_STREAM_DATA        "data"
#define AF_MEDIA_STREAM_APPLICATION "application"
#define AF_MEDIA_STREAM_CONTROL     "control"

#define AF_OPTION_TRUE         1
#define AF_OPTION_FALSE        0
#define AF_OPTION_INVALID	  -1

#define AF_ICE_COMPONENT_RTP  1
#define AF_ICE_COMPONENT_RTCP 2

#define AF_ICE_TRANSPORT_UDP                   "UDP"

#define AF_ICE_CANDIDATE_HOST  "host"
#define AF_ICE_CANDIDATE_SRFLX "srflx"
#define AF_ICE_CANDIDATE_PRFLX "prflx"
#define AF_ICE_CANDIDATE_RELAY "relay"

#define AF_MODE_STANDARD	   0
#define AF_MODE_AUTO           1
#define AF_MODE_MSOCS          2

#ifndef AFE_TYPES
#define AFE_TYPES

enum EAfErrorType
{
	EAfErrNone,
	EAfErrUnknown,
	EAfErrNotAValidChannel,	// A channel operation was performed on the session
	EAfErrNotAValidSession,	// A session operation was performed on the channel
	EAfErrNotAValidObject,	// Not a valid session or channel id
	
	EAfErrInvalidParameter,	// A parameter to the function was invalid
	EAfErrClosedChannel,	// Cannot perform the operation on a closed channel

	EAfChannelErrorTcpConnectionFailed,	
	EAfChannelErrorConnectionToServerLost
};


enum EAfEventType
{
	EAfEventFirewallTypeDetected = 120,

	EAfEventHTTPProxyDetected = 131,
	EAfEventHTTPProxyAuthenticationFailed = 407,
	EAfEventHTTPProxyConnectionFailed = 408,
	EAfEventHTTPProxyNTLMDomainEmpty = 409,
	EAfEventHTTPProxyAuthenticationFailureNTLM =410,
	
	EAfEventUPnPDeviceDiscovered = 141,
	EAfEventStunPassServerAuthenticationFailure = 401,
	EAfEventStunPassServerConnectionFailure = 402,

    EAfEventICECheckStarted = 100,
	EAfEventICECheckCompleted = 200,
	EAfEventICENeedUpdatedOffer = 151,
	EAfEventICECheckFailed = 404,
	EAfEventCallCompletionStatusAvailable = 171,

	EAfEventNoDataReceived = 480,
	EAfEventNoDataReceivedAfterFallbackToRelay = 481,
	EAfEventConnectionToServerLost = 503,
	EAfEventStunRelayBandwidthWarning = 507,

	EAfEventNetworkInterfacesChanged = 161,
	
	EAfEventOfferSDPAvailable = 201,
	EAfEventAnswerSDPAvailable = 202,

	EAfEventConnectSuccess = 203,
	EAfEventConnectFailure = 450,

	EAfEventLookupIPv4AddressSuccess = 204,
	EAfEventLookupIPv4AddressFailure = 444,
	
	EAfEventTurnServerDnsResolutionFailed = 451,
	EAfEventTurnServerConnectionFailed = 452,
	EAfEventTurnServerAuthenticationFailed = 453

};

struct AfEvent
{
	unsigned long uEvent;   //EAfEventType
	unsigned long uChannel; //0 means even is not specific to a channel
};


enum EAfSessionProperty
{
	EAfSessionNonICEPeer,
	EAfSessionCallCompletionType
};

enum EAfCallCompletionType
{
	// Initial state
	EAfCallCompletionUnknown,

	// Success cases

	EAfCallCompletionP2P,
	EAfCallCompletionUDPRelay,
	EAfCallCompletionTCPRelay,
	EAfCallCompletionHTTPRelay,

	// Failure cases 
	EAfCallCompletionFailed
	
};
		
// This struct can be used with SetChannelOption to set socket options for a channel.
struct AfSocketOptionParams
{
	int iLevel;
	int iOptName;
	const void* pOptVal;
	int iOptLen;
};

/*

Mode of Operation		----------------- Options ------------------
						STUN		Relay		HTTP Tunneling	UPnP
=================       ====		=====		==============  ====

Standard				TRUE		TRUE		FALSE			FALSE

Auto					TRUE		TRUE		TRUE			TRUE

Manual					TRUE/FALSE	TRUE/FALSE	TRUE/FALSE		TRUE/FALSE

*/

enum EAfOptionName
{
	EAfOptionStandardMode,
	EAfOptionAutoMode,
	EAfOptionManualMode,
	EAfOptionEnableStun,
	EAfOptionEnableRelay,
	EAfOptionEnableHTTPTunneling,
	EAfOptionEnableUPnP,
	EAfOptionKeepAlivePeriodSeconds,
	EAfOptionFullCone,
	EAfOptionBandwidthKbps,
	EAfOptionSocket
};

enum EAfDetectedFirewallType
{
	EAfFirewallTypeUnknown,
	EAfFirewallTypeNone,
	EAfFirewallTypeNAT,		// UDP is OK

	EAfFirewallTypeTCPOnly, // UDP blocked and TCP is enabled
	EAfFirewallTypeProxy,	// UDP blocked, TCP is possible only through proxy
	EAfFirewallTypeBlocked		// TCP and UDP both are blocked
};



	// structs containing SDP information

	// Details for one media
	struct AfMedia
	{
		// m-line. Example: "m=audio 49154 RTP/AVP 0"
		char *sMediaType;				// Example: "audio"
		int iPort;						// m-line default port. Example: 49154. Negative value means it should not be used in SDP.
		char *sTransport;				// Example: "RTP/AVP"
		
		int iRtcpPort;					// Default port for RTCP. Example: 49155. Negative value means it should not be used in SDP.

		// Media-level attributes (these override session-level counterparts)
		// c-line. Example: "c=IN IP4 202.53.169.195"
		char *sConnection;				// Media-level c-line. Example: "IN IP4 202.53.169.195"
		unsigned int uConnectionIp;		// c-line IP separately kept in this attribute so that it can be used 
										// without parsing. Example: 0xCA35A9C3 (202.53.169.195)
		
		// ICE attributes
		char *sUfrag;					// a=ice-ufrag:
		char *sUpass;					// a=ice-pwd:

		// One or more candidates
		int iNumCandidates;				// Number of candidates in this media
		char** psCandidates;			// Each candidate string can be accessed with: pcCandidates[index]
										// Example: printf("first candidate line:\n a=candidate:%s\r\n", pcCandidates[0])
		// a=remote-candidates line
		char *sRemoteCandidates;		// Example: "1 192.168.0.132 21439 2 192.168.0.132 13917"	
	};

	// SDP information is available through this struct
	struct AfSessionInfo
	{
		// Session-level attributes
		char *sVersion;					// v=
		char *sOwner;					// o=
		char *sSoftwareAttribute;		// s=
		char *sActiveTime;				// t=

		// c-line. Example: "c=IN IP4 202.53.169.195"
		char *sConnection;				// Session level c-line. Example: "IN IP4 202.53.169.195"
		unsigned int uConnectionIp;		// c-line IP separately kept in this attribute so that it can be used 
										// without parsing. Example: 0xCA35A9C3 (202.53.169.195)
		
		// ICE attributes
		char *sUfrag;					// a=ice-ufrag:
		char *sUpass;					// a=ice-pwd:
		
		// One or more media
		int iNumMedia;					// Number of media
		struct AfMedia **ppMedia;				// Each AfMedia instance can be accessed by: *(ppMedia[index])

		// The whole SDP as a string
		char *sSdp;
	};

#endif

typedef void (*send_cb_t)(int nChannel, int nBytesSent);

class IAfCallbackHandler
{
public:

	#if defined(WIN32) || defined(_WIN32_WCE)
		virtual void _cdecl HandleCallback(int iChannel, char *pData, int iLen, const CAfStdString& sSrcAddress, int iSrcPort, const void *pContext) = 0;
	#else
		virtual void HandleCallback(int iChannel, char *pData, int iLen, const CAfStdString& sSrcAddress, int iSrcPort, const void *pContext) = 0;
	#endif

	virtual ~IAfCallbackHandler() {}
};

#ifdef AF_BENCHMARK_ENABLED
class IAfBenchmarkHandler
{
public:
	virtual void AfHandleRecvCallback(int iChannel) = 0;
	virtual void AfHandleSendCallback(int iChannel) = 0;
	virtual void AfHandleSendCancel(int iChannel) = 0;

	virtual void AfHandleBenchmarkCallback(const std::string& sKey, bool bPush, unsigned long long, unsigned long) = 0;
protected:
	~IAfBenchmarkHandler() {}
};
#endif

class CAnyFirewallInterface
{
public:

	virtual ~CAnyFirewallInterface() {}

	virtual bool          Init(int iMode, bool bBlocking) = 0;
	virtual void          Release() = 0;

	virtual bool          SetSTUNUsernamePasswordRealm(const CAfStdString& sUsername, const CAfStdString& sPassword, const CAfStdString& sRealm) = 0;
	virtual CAfStdString* GetSTUNUsername() = 0; 
	virtual CAfStdString* GetSTUNPassword() = 0; 
	virtual CAfStdString* GetSTUNRealm() = 0; 

	virtual bool          SetSTUNPassServer(const CAfStdString& sHost) = 0;
	virtual CAfStdString* GetSTUNPassServer() = 0; 

	virtual bool          SetSTUNServer(const CAfStdString& sHost) = 0;
	virtual CAfStdString* GetSTUNServer() = 0; 

	virtual bool          SetSTUNRelayServer(const CAfStdString& sHost) = 0;
	virtual CAfStdString* GetSTUNRelayServer() = 0;

	virtual bool         DetectConnectivity() = 0;
	virtual int			 WaitForDetectConnectivity(int iTimeoutMillisec) = 0;
	
	virtual int          Create(const CAfStdString& channelType, int iUdpHostPort, int iUdpServerConnectionPort, int iTcpPort) = 0;

	virtual bool         Close(int iChannel) = 0;
	
	virtual bool         Connect(int iChannel, const CAfStdString& sHostList) = 0;
	virtual EAfErrorType GetLastError(int iChannel) = 0;
	
	virtual int          Send(int iChannel, const char pData[], int iLen, int iTimeoutMillisec) = 0;
	virtual int          SendTo(int iChannel, const char pData[], int iLen, const CAfStdString& sDestAddress, int iDestPort, int iTimeoutMillisec) = 0;

	virtual int          Recv(int iChannel, char pBuff[], int iLen, int iTimeoutMillisec) = 0;
	virtual int          RecvFrom(int iChannel, char pBuff[], int iLen, CAfStdString*& sSrcAddress, int& iSrcPort, int iTimeoutMillisec) = 0;

	// timeout can be 0 - 60000ms, or infinite (-1)
	virtual int          Select(int iNumChannels, int aChannels[], int aInputEvents[], 
	                            int aOutputEvents[], int iTimeoutMillisec) = 0;  

	virtual bool         IsClosed(int iChannel) = 0;
	virtual bool         ChannelExists(int iChannel) = 0;

	//Host Parsing Helper Functions
	virtual int          GetHostListSize(const CAfStdString& sHostList) = 0;
	virtual CAfStdString* GetHostFromList(const CAfStdString& sHostList, int iIndex) = 0;
	virtual CAfStdString* GetHostByType(const CAfStdString& sHostList, const CAfStdString& sType) = 0;
	virtual CAfStdString* AddHostToList(const CAfStdString& sHostList, const CAfStdString& sHost) = 0;

	virtual CAfStdString* CreateHost(const CAfStdString& sType, const CAfStdString& sAddress,
	                                int iPort, const CAfStdString& sProtocol) = 0;

	virtual CAfStdString* GetHostType(const CAfStdString& sHost) = 0;
	virtual CAfStdString* GetHostAddress(const CAfStdString& sHost) = 0;
	virtual int           GetHostPort(const CAfStdString& sHost) = 0;
	virtual CAfStdString* GetHostProtocol(const CAfStdString& sHosts) = 0;

	virtual CAfStdString* SetHostType(const CAfStdString& sHost, const CAfStdString& sType) = 0;
	virtual CAfStdString* SetHostAddress(const CAfStdString& sHost, const CAfStdString& sAddress) = 0;
	virtual CAfStdString* SetHostPort(const CAfStdString& sHost, int iPort) = 0;
	virtual CAfStdString* SetHostProtocol(const CAfStdString& sHost, const CAfStdString& sProtocol) = 0;

	virtual bool          SetHTTPProxy(const CAfStdString& sHost, const CAfStdString& sUsername, const CAfStdString& sPassword, const CAfStdString &sDomain) = 0;
	virtual CAfStdString* GetHTTPProxy() = 0;
	virtual CAfStdString* GetHTTPProxyUsername() = 0;
	virtual CAfStdString* GetHTTPProxyPassword() = 0;
	virtual CAfStdString* GetHTTPProxyDomain() = 0;

	virtual void          Log(const CAfStdString& sMessage) = 0;
	virtual bool		  SetPortRange(int iBottomRange, int iTopRange, const CAfStdString& socketType) = 0;

	virtual unsigned long DNS_LookupIPv4Address(int iDnsChannel, const CAfStdString& sHostName) = 0;
	virtual CAfStdString* DNS_LookupIPv4Addresses(int iDnsChannel, const CAfStdString& sHostName, int& iNumResults) = 0;
	virtual CAfStdString* DNS_SRV_Lookup(int iDnsChannel, const CAfStdString& sTarget, const CAfStdString& sProtocol) = 0;

	/****************************************************************************************************************************/

	virtual CAfStdString*  GetLocalAddress(int iChannel)= 0;
	virtual CAfStdString*  GetRemoteAddress(int iChannel) = 0;

	virtual int            CreateSession(const CAfStdString& sMediaDescription, const void *pContext) = 0;

	virtual bool           ModifySession(int iSessionID, const CAfStdString& sMediaDescription) = 0;
	virtual bool           CloseSession(int iSessionID) = 0;

	virtual bool           IsSession(int iSessionID) = 0;
	virtual CAfStdString*  GetSessionDescription(int iSessionID) = 0;

	virtual const struct AfSessionInfo *MakeOffer(int iSessionID) = 0;
	virtual const struct AfSessionInfo *MakeAnswer(int iSessionID, const CAfStdString& sOfferSDP) = 0;	
		
	virtual bool           ProcessAnswer(int iSessionID, const CAfStdString& sAnswerSDP) = 0;
	virtual int			   GetSessionProperty(int iSessionID, EAfSessionProperty eSessionProperty) = 0;

	virtual bool           SetChannelOption(int iChannel, EAfOptionName eOptionName, int iOptionValue) = 0;
	virtual int			   GetChannelOption(int iChannel, EAfOptionName eOptionName) = 0;
	
	virtual EAfDetectedFirewallType GetFirewallType() = 0;
	virtual CAfStdString* Bind(int iChannel) = 0;
	virtual bool          ClearLoopbackRecvBuffer (int iChannel) = 0;
	virtual bool          SetCallbackHandler(int iChannel, IAfCallbackHandler* pHandler) = 0;

	virtual CAfStdString* GetExternalIP() = 0;
	virtual CAfStdString* GetLocalInterfaceIPs() = 0;

	virtual void DeleteString(CAfStdString* pString) = 0;
	virtual void DeleteStringArray(CAfStdString* pStringArray) = 0;

#ifdef AF_BENCHMARK_ENABLED
	virtual bool          SetBenchmarkHandler(int iChannel, IAfBenchmarkHandler* pHandler) = 0;
#endif
	
	virtual const struct AfSessionInfo* GetSessionSDP(int iSessionID) = 0;
	virtual int SetSendCB(int iChannel, send_cb_t send_cb) = 0;
};


class CAfStdString
{
public:
	CAfStdString()
		: m_iSize(0), m_pBytes(NULL)
	{}

	CAfStdString(const std::string& s)
		: m_iSize((unsigned int)s.size()), m_pBytes(NULL)
	{
		if (m_iSize > 0)
		{
			m_pBytes = new char[m_iSize];
			memcpy(m_pBytes, s.data(), m_iSize);
		}
	}

	~CAfStdString()
	{
		if (m_iSize > 0)
		{
			delete [] m_pBytes;
		}
	}

	CAfStdString& operator=(const std::string& s)
	{
		if (m_iSize > 0)
		{
			delete [] m_pBytes;
		}

		m_iSize  = (unsigned int)s.size();
		m_pBytes = NULL;

		if (m_iSize > 0)
		{
			m_pBytes = new char[m_iSize];
			memcpy(m_pBytes, s.data(), m_iSize);
		}
		return (*this);
	}

	operator std::string() const
	{
		if (m_iSize > 0)
		{
			return std::string(m_pBytes, m_iSize);
		}
		else
		{
			return std::string();
		}
	}

private:
	unsigned long m_iSize;
	char*         m_pBytes;
};

#endif //ANYFIREWALL_INTERFACE_H
#endif	//__DISABLE_ICE__
