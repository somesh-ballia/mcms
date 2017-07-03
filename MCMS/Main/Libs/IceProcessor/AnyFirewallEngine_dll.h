#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

// included by projects using the AnyFirewall Engine DLL
#ifndef ANYFIREWALLENGINE_DLL_H
#define ANYFIREWALLENGINE_DLL_H

#include "AnyFirewallInterface.h"
#ifdef _WIN32_WCE
#include "tstring.h"
#endif

#ifdef AFE_STATIC
#include "AnyFirewallDll.h"
#endif //AFE_STATIC

// Platform-define checks
#if defined(WIN32) || defined(_WIN32_WCE)
  #define AFE_WIN32  // Windows version
#else
  #define AFE_LINUX  // Linux version
#endif

// Note: it's possible to over-ride (pre-define) this DLL file-name
#ifndef AF_DLL_FILE_NAME
  #ifdef AFE_WIN32
#ifndef _WIN32_WCE
    #define AF_DLL_FILE_NAME "AnyFirewall.DLL"
#else
    #define AF_DLL_FILE_NAME "AnyFirewallDllCE.dll"
#endif
  #endif // AFE_WIN32

  #ifdef AFE_LINUX
    #ifdef __APPLE__
    #define AF_DLL_FILE_NAME "libAFE.dylib"
    #else
    #define AF_DLL_FILE_NAME "libAFE.so"
    #endif
  #endif // AFE_LINUX
#endif //AF_DLL_FILE_NAME

#define AF_DLL_MODULE_FUNCTION_NAME   "EyeballGetModule_AnyFirewall"
#define AF_DLL_VERSION_FUNCTION_NAME  "EyeballGetVersion_AnyFirewall"
#define AF_DLL_GETDLL_VERSION_FUNCTION_NAME  "EyeballGetDLLVersion_AnyFirewall"

typedef int (* af_func_ptr)(void);
typedef int (* af_GetAFDLLVersion_func_ptr)(void);

//{6d5e294b-339c-4588-8446-42db1fa7f9f9}
#define DEF_CLSID_ANYFIREWALL { 0x6d5e294b, 0x339c, 0x4588, { 0x84, 0x46, 0x42, 0xdb, 0x1f, 0xa7, 0xf9, 0xf9 } }

#ifdef AFE_LINUX
#include <dlfcn.h>
#include <stdio.h>
#endif //AFE_LINUX

// C++ DLL loader/wrapper class
// All methods are in-line so no static lib is needed
class CAnyFirewallEngine
{
public:
	CAnyFirewallEngine()
	:	m_hDll(NULL),
		m_pAf(NULL),
		m_bLoadLibraryError(false),
		m_bVersionCheckFailed(false),
		m_iAFDLLVersionNumber(-1)
	{
	}

	~CAnyFirewallEngine()
	{
	}

	bool Init(int iMode, bool iBlocking)
	{
		if (!Load())
		{
			Unload();
			return false;
		}

		if (!m_pAf->Init(iMode, iBlocking))
		{
			return false;
		}

		return true;
	}

#ifdef AFE_WIN32
	// Version of Init which takes a handle to an existing DLL instance
	// This DLL should be in the same directory as the AFE DLL.
	// Overloaded function for Polycom MS-OCS mode
	bool InitDll(HINSTANCE hInst, int iMode, bool iBlocking, std::string sPath = "")
	{
		m_sDllPath = GetPathName(GetModuleDir(hInst), AF_DLL_FILE_NAME);

		m_sDllRegPath = sPath;

		return Init(iMode, iBlocking);
	}
#endif //AFE_WIN32
	int GetAFDLLVersionNumber()
	{
		return m_iAFDLLVersionNumber;
	}

	void Release()
	{
		if (NULL == m_pAf)
		{
			return;
		}

		m_pAf->Release();
		delete m_pAf;
		m_pAf = NULL;

		Unload();
	}
	
	bool SetSTUNUsernamePasswordRealm(const std::string& sUsername, const std::string& sPassword, const std::string& sRealm)
	{
		return (m_pAf) ? m_pAf->SetSTUNUsernamePasswordRealm(sUsername, sPassword, sRealm) : false;
	}

	std::string GetSTUNUsername()
	{
		return (m_pAf) ? CopyString(m_pAf->GetSTUNUsername()) : "";
	}

	std::string GetSTUNPassword()
	{
		return (m_pAf) ? CopyString(m_pAf->GetSTUNPassword()) : "";
	}

	std::string GetSTUNRealm()
	{
		return (m_pAf) ? CopyString(m_pAf->GetSTUNRealm()) : "";
	}

	bool SetSTUNPassServer(const std::string& sHost)
	{
		return (m_pAf) ? m_pAf->SetSTUNPassServer(sHost) : false;
	}

	std::string GetSTUNPassServer()
	{
		return (m_pAf) ? CopyString(m_pAf->GetSTUNPassServer()) : "";
	}

	bool SetSTUNServer(const std::string& sHost)
	{
		return (m_pAf) ? m_pAf->SetSTUNServer(sHost) : false;
	}

	std::string GetSTUNServer()
	{
		return (m_pAf) ? CopyString(m_pAf->GetSTUNServer()) : "";
	}

	bool SetSTUNRelayServer(const std::string& sHost)
	{
		return (m_pAf) ? m_pAf->SetSTUNRelayServer(sHost) : false;
	}

	std::string GetSTUNRelayServer()
	{
		return (m_pAf) ? CopyString(m_pAf->GetSTUNRelayServer()) : "";
	}

	bool DetectConnectivity()
	{
		return ((m_pAf) ? m_pAf->DetectConnectivity() : false);
	}

	int WaitForDetectConnectivity(int iTimeoutMillisec)
	{
		return ((m_pAf) ? m_pAf->WaitForDetectConnectivity(iTimeoutMillisec) : -1);
	}

	int Create(const std::string& channelType, int iUdpHostPort, int iUdpServerConnectionPort, int iTcpPort)
	{
		return (m_pAf) ? m_pAf->Create(channelType, iUdpHostPort, iUdpServerConnectionPort, iTcpPort) : -1;
	}

	bool Close(int iChannel)
	{
		return (m_pAf) ? m_pAf->Close(iChannel) : false;
	}

	std::string Bind(int iChannel)
	{
		return (m_pAf) ? CopyString(m_pAf->Bind(iChannel)) : "";
	}

	int Send(int iChannel, const char pData[], int iLen, int iTimeoutMillisec)
	{
		return (m_pAf) ? m_pAf->Send(iChannel, pData, iLen, iTimeoutMillisec) : -1;
	}

	int SendTo(int iChannel, const char pData[], int iLen, const std::string& sDestAddress, int iDestPort, int iTimeoutMillisec)
	{
		return (m_pAf) ? m_pAf->SendTo(iChannel, pData, iLen, sDestAddress, iDestPort, iTimeoutMillisec) : -1;
	}

	int Select(int iNumChannels, int aChannels[], int aInputEvents[], int aOutputEvents[], int iTimeoutMillisec)
	{
		return (m_pAf) ? m_pAf->Select(iNumChannels, aChannels, aInputEvents, aOutputEvents, iTimeoutMillisec) : -1;
	}

	int Recv(int iChannel, char pBuff[], int iLen, int iTimeoutMillisec)
	{
		return (m_pAf) ? m_pAf->Recv(iChannel, pBuff, iLen, iTimeoutMillisec) : -1;
	}

	int RecvFrom(int iChannel, char pBuff[], int iLen, std::string& sSrcAddress, int& iSrcPort, int iTimeoutMillisec)
	{
		int iRet = -1;
		sSrcAddress = "0.0.0.0";
		iSrcPort = 0;

		if (m_pAf)
		{
			CAfStdString* pAddress = NULL;
			iRet = m_pAf->RecvFrom(iChannel, pBuff, iLen, pAddress, iSrcPort, iTimeoutMillisec);
			sSrcAddress = CopyString(pAddress);
		}
		return iRet;
	}

	bool Connect(int iChannel, const std::string& sHostList)
	{
		return (m_pAf) ? m_pAf->Connect(iChannel, sHostList) : false;
	}

	EAfErrorType GetLastError(int iChannel)
	{
		return (m_pAf) ? m_pAf->GetLastError(iChannel) : EAfErrNone;
	}

	bool IsClosed(int iChannel)
	{
		return (m_pAf) ? m_pAf->IsClosed(iChannel) : true;
	}

	bool ChannelExists(int iChannel)
	{
		return (m_pAf) ? m_pAf->ChannelExists(iChannel) : false;
	}

	//Host Parsing Helper Functions
	int GetHostListSize(const std::string& sHostList)
	{
		return (m_pAf) ? m_pAf->GetHostListSize(sHostList) : 0;
	}

	std::string GetHostFromList(const std::string& sHostList, int iIndex)
	{
		return (m_pAf) ? CopyString(m_pAf->GetHostFromList(sHostList, iIndex)) : "";
	}

	std::string GetHostByType(const std::string& sHostList, const std::string& sType)
	{
		return (m_pAf) ? CopyString(m_pAf->GetHostByType(sHostList, sType)) : "";
	}

	std::string AddHostToList(const std::string& sHostList, const std::string& sHost)
	{
		return (m_pAf) ? CopyString(m_pAf->AddHostToList(sHostList, sHost)) : "";
	}

	std::string CreateHost(const std::string& sType, const std::string& sAddress,
					 	   int iPort, const std::string& sProtocol)
	{
		return (m_pAf) ? CopyString(m_pAf->CreateHost(sType, sAddress, iPort, sProtocol)) : "";
	}

	std::string GetHostType(const std::string& sHost)
	{
		return (m_pAf) ? CopyString(m_pAf->GetHostType(sHost)) : "";
	}

	std::string GetHostAddress(const std::string& sHost)
	{
		return (m_pAf) ? CopyString(m_pAf->GetHostAddress(sHost)) : "";
	}

	int GetHostPort(const std::string& sHost)
	{
		return (m_pAf) ? m_pAf->GetHostPort(sHost) : 0;
	}

	std::string GetHostProtocol(const std::string& sHost)
	{
		return (m_pAf) ? CopyString(m_pAf->GetHostProtocol(sHost)) : "";
	}

	std::string SetHostType(const std::string& sHost, const std::string& sType)
	{
		return (m_pAf) ? CopyString(m_pAf->SetHostType(sHost, sType)) : "";
	}

	std::string SetHostAddress(const std::string& sHost, const std::string& sAddress)
	{
		return (m_pAf) ? CopyString(m_pAf->SetHostAddress(sHost, sAddress)) : "";
	}

	std::string SetHostPort(const std::string& sHost, int iPort)
	{
		return (m_pAf) ? CopyString(m_pAf->SetHostPort(sHost, iPort)) : "";
	}

	std::string SetHostProtocol(const std::string& sHost, const std::string& sProtocol)
	{
		return (m_pAf) ? CopyString(m_pAf->SetHostProtocol(sHost, sProtocol)) : "";
	}

	bool SetHTTPProxy(const std::string& sHost, const std::string& sUsername, const std::string& sPassword, const std::string &sProxyDomain)
	{
		return (m_pAf) ? m_pAf->SetHTTPProxy(sHost, sUsername, sPassword, sProxyDomain) : false;
	}

	std::string GetHTTPProxy()
	{
		return (m_pAf) ? CopyString(m_pAf->GetHTTPProxy()) : "";
	}

	std::string GetHTTPProxyUsername()
	{
		return (m_pAf) ? CopyString(m_pAf->GetHTTPProxyUsername()) : "";
	}

	std::string GetHTTPProxyPassword()
	{
		return (m_pAf) ? CopyString(m_pAf->GetHTTPProxyPassword()) : "";
	}

	std::string GetHTTPProxyDomain()
	{
		return (m_pAf) ? CopyString(m_pAf->GetHTTPProxyDomain()) : "";
	}


	void Log(const std::string& sMessage)
	{
		if (m_pAf) m_pAf->Log(sMessage);
	}

	bool SetPortRange(int iBottomRange, int iTopRange, const std::string& socketType)
	{
		return (m_pAf) ? m_pAf->SetPortRange(iBottomRange, iTopRange, socketType) : false;
	}

	unsigned long DNS_LookupIPv4Address(int iDnsChannel, const std::string& sHostName)
	{
		return (m_pAf) ? m_pAf->DNS_LookupIPv4Address(iDnsChannel, sHostName) : 0;
	}

	std::string* DNS_LookupIPv4Addresses(int iDnsChannel, const std::string& sHostName, int& iNumResults)
	{
		if (m_pAf)
		{
			CAfStdString* pStringArray = m_pAf->DNS_LookupIPv4Addresses(iDnsChannel, sHostName, iNumResults);
			return CopyStringArray(pStringArray, iNumResults);
		}
		iNumResults = 0;
		return NULL;	
	}

	std::string DNS_SRV_Lookup(int iDnsChannel, const std::string& sTarget, const std::string& sProtocol)
	{
		return (m_pAf) ? CopyString(m_pAf->DNS_SRV_Lookup(iDnsChannel, sTarget, sProtocol)) : "";
	}

	
	int CreateSession(const std::string& sMediaDescription, const void *pContext)
	{
		return (m_pAf) ? m_pAf->CreateSession(sMediaDescription, pContext) : -1;
	}

	bool ModifySession(int iSessionID, const std::string& sMediaDescription)
	{
		return (m_pAf) ? m_pAf->ModifySession(iSessionID, sMediaDescription) : false;
	}

	bool CloseSession(int iSessionID)
	{
		return (m_pAf) ? m_pAf->CloseSession(iSessionID) : false;
	}

	bool IsSession(int iSessionID)
	{
		return (m_pAf) ? m_pAf->IsSession(iSessionID) : false;
	}

	std::string GetSessionDescription(int iSessionID)
	{
		return (m_pAf) ? CopyString(m_pAf->GetSessionDescription(iSessionID)) : "";
	}

	const struct AfSessionInfo *MakeOffer(int iSessionID)
	{
		return (m_pAf) ? m_pAf->MakeOffer(iSessionID) : NULL;
	}
	


	std::string GetLocalAddress(int iChannel)
	{
		return (m_pAf) ? CopyString(m_pAf->GetLocalAddress(iChannel)) : "";
	}

	std::string GetLocalInterfaceIPs()
	{
		return (m_pAf) ? CopyString(m_pAf->GetLocalInterfaceIPs()) : "";
	}

	std::string GetRemoteAddress(int iChannel)
	{
		return (m_pAf) ? CopyString(m_pAf->GetRemoteAddress(iChannel)) : "";
	}

	const struct AfSessionInfo *MakeAnswer(int iSessionID, const std::string& sOfferSDP)
	{
		return (m_pAf) ? m_pAf->MakeAnswer(iSessionID, sOfferSDP) : NULL;
	}	

	bool ProcessAnswer(int iSessionID, const std::string& sAnswerSDP)
	{
		return (m_pAf) ? m_pAf->ProcessAnswer(iSessionID, sAnswerSDP) : false;
	}

	int GetSessionProperty(int iSessionID, EAfSessionProperty eSessionProperty)
	{
		return (m_pAf) ? m_pAf->GetSessionProperty(iSessionID, eSessionProperty) : -1;
	}

	bool SetChannelOption(int iChannel, EAfOptionName eOptionName, int iOptionValue)
	{
		return (m_pAf) ? m_pAf->SetChannelOption(iChannel, eOptionName, iOptionValue) : false;
	}

	int GetChannelOption(int iChannel, EAfOptionName eOptionName)
	{
		return (m_pAf) ? m_pAf->GetChannelOption(iChannel, eOptionName) : -1;
	}

	EAfDetectedFirewallType	GetFirewallType()
	{
		return (m_pAf) ? m_pAf->GetFirewallType() : EAfFirewallTypeUnknown;
	}

	bool ClearLoopbackRecvBuffer(int iChannel)
	{
		return (m_pAf) ? m_pAf->ClearLoopbackRecvBuffer(iChannel) : false;
	}

	bool SetCallbackHandler(int iChannel, IAfCallbackHandler* pHandler)
	{
		return (m_pAf) ? m_pAf->SetCallbackHandler(iChannel, pHandler) : false;
	}

	const struct AfSessionInfo *GetSessionSDP(int iSessionID)
	{
		return (m_pAf) ? m_pAf->GetSessionSDP(iSessionID) : NULL;
	}	

	//[TEST]
	int SetSendCB(int iChannel, send_cb_t send_cb)
	{
		return (m_pAf) ? m_pAf->SetSendCB(iChannel, send_cb) : -1;
	}

#ifdef AF_BENCHMARK_ENABLED
	bool SetBenchmarkHandler(int iChannel, IAfBenchmarkHandler* pHandler)
	{
		return (m_pAf) ? m_pAf->SetBenchmarkHandler(iChannel, pHandler) : false;
	}
#endif //AF_BENCHMARK_ENABLED

	bool LoadLibraryFailed()
	{
		return m_bLoadLibraryError;
	}

	bool VersionCheckFailed()
	{
		return m_bVersionCheckFailed;
	}

private:
	/**
	 * Checks the DLL version
	 *
	 * Calling this function is optional, but may be useful for checking for
	 * and handling multiple DLL versions.
	 *
	 * Function fails if DLL is not loaded. (e.g. Init failed or not yet called)
	 *
	 *@return true  if DLL version check function returns true, false otherwise
	 */

	bool CheckVersion()
	{
		if (NULL == m_hDll)
			return false;

#ifdef AFE_WIN32
#ifndef _WIN32_WCE
		af_func_ptr pFn = (af_func_ptr)GetProcAddress(m_hDll, AF_DLL_VERSION_FUNCTION_NAME);
		af_GetAFDLLVersion_func_ptr pFnToGetAFDLLVersion = (af_GetAFDLLVersion_func_ptr)GetProcAddress(m_hDll, AF_DLL_GETDLL_VERSION_FUNCTION_NAME);
#else
				af_func_ptr pFn = (af_func_ptr)GetProcAddress(m_hDll, _T(AF_DLL_VERSION_FUNCTION_NAME));
		af_GetAFDLLVersion_func_ptr pFnToGetAFDLLVersion = (af_GetAFDLLVersion_func_ptr)GetProcAddress(m_hDll, _T(AF_DLL_GETDLL_VERSION_FUNCTION_NAME));
#endif
#else
		af_func_ptr pFn = (af_func_ptr) dlsym(m_hDll, AF_DLL_VERSION_FUNCTION_NAME);
		af_GetAFDLLVersion_func_ptr pFnToGetAFDLLVersion = (af_GetAFDLLVersion_func_ptr)dlsym(m_hDll, AF_DLL_GETDLL_VERSION_FUNCTION_NAME);
#endif

		if (NULL == pFn)
			return false;

		if (NULL == pFnToGetAFDLLVersion)
			return false;

		int iRet = (pFn)();
		m_iAFDLLVersionNumber = (pFnToGetAFDLLVersion)();

		return (AF_INTERFACE_VERSION == iRet);
	}

	bool Load()
	{

#ifndef AFE_STATIC

		if (NULL != m_hDll)
		{
			return true;  // already loaded
		}

#ifdef AFE_WIN32
		if (m_sDllPath.size() > 0)
		{

#ifndef _WIN32_WCE
			m_hDll = LoadLibraryA(m_sDllPath.c_str());
#else
			TCHAR szDLLName[40]; 
			lstrcpy(szDLLName, _T("AnyFirewallDllCE.dll")); 
			m_hDll = LoadLibrary(szDLLName);
#endif
			if (m_hDll && (!CheckVersion()))
			{
				Unload();
				m_bVersionCheckFailed = true;
			}
		}

		if (NULL == m_hDll)
		{
#ifndef _WIN32_WCE
			m_hDll = LoadLibraryA(m_sDllRegPath.c_str());
#else
			tstring ts = tstring(m_sDllRegPath);
			m_hDll = LoadLibraryW(ts.c_str());
#endif
			if (m_hDll && (!CheckVersion()))
			{
				Unload();
				m_bVersionCheckFailed = true;
			}
		}
#endif //AFE_WIN32

		if (NULL == m_hDll)
		{
#ifdef AFE_WIN32
#ifndef _WIN32_WCE
			m_hDll = LoadLibraryA(AF_DLL_FILE_NAME);
#else
			tstring ts = tstring(AF_DLL_FILE_NAME);
			m_hDll = LoadLibrary(ts.c_str());
#endif
#else
			m_hDll = dlopen(AF_DLL_FILE_NAME, RTLD_LAZY);
			if (NULL == m_hDll)
				fprintf(stderr, "dlopen failed: %s\n", dlerror());
#endif //AFE_WIN32

			if (NULL == m_hDll)
			{
				m_bLoadLibraryError = true;
				return false;  // could not load DLL
			}
		}

		m_bLoadLibraryError = false;

		if (!CheckVersion())
		{
			m_bVersionCheckFailed = true;
			return false;
		}

#else //AFE_STATIC

		m_bLoadLibraryError = false;
		m_bVersionCheckFailed = false;

#endif //AFE_STATIC


#ifndef AFE_STATIC

#ifdef AFE_WIN32

#ifndef _WIN32_WCE
		af_func_ptr pFn = (af_func_ptr)GetProcAddress(m_hDll, AF_DLL_MODULE_FUNCTION_NAME);
#else
		af_func_ptr pFn = (af_func_ptr)GetProcAddress(m_hDll, _T(AF_DLL_MODULE_FUNCTION_NAME));
#endif

#else //AFE_WIN32

		af_func_ptr pFn = (af_func_ptr)dlsym(m_hDll, AF_DLL_MODULE_FUNCTION_NAME);

#endif //AFE_WIN32

#else //AFE_STATIC
		af_func_ptr pFn = (af_func_ptr) EyeballGetModule_AnyFirewall;
#endif //AFE_STATIC

		if (NULL == pFn)
		{
			return false;  // could not load function from DLL
		}

		m_pAf = (CAnyFirewallInterface*)(pFn)();

		if (NULL == m_pAf)
		{
			return false;  // could not execute function
		}

		return true;
	}

	void Unload()
	{
		if (NULL != m_hDll)
		{
#ifdef AFE_WIN32
			FreeLibrary(m_hDll);
#else

#ifndef AFE_STATIC
			dlclose(m_hDll);
#endif //AFE_STATIC

#endif //AFE_WIN32
			m_hDll = NULL;
		}

		if (NULL != m_pAf)
		{
			// do not delete m_pAf -- the object is static
			m_pAf = NULL;
		}
	}

#ifdef AFE_WIN32
	// helper functions
	std::string GetModuleDir(HMODULE hInst)
	{
		char szFileName[MAX_PATH];
#ifndef _WIN32_WCE
		bool bRet = (GetModuleFileNameA(
			hInst, 
			szFileName, 
			sizeof(szFileName) / sizeof(char)) != 0);
#else
		tstring filename;
		filename.resize(MAX_PATH);

		bool bRet = (GetModuleFileName(
			hInst, 
			&filename[0], 
			sizeof(szFileName) / sizeof(char)) != 0);

		std::string s(filename.begin(), filename.end());
		strcpy(szFileName, s.c_str());
#endif

		if (!bRet)
		{
			return "";
		}

		std::string sPathName = szFileName;
		std::string::size_type idx = sPathName.find_last_of('\\');
		if (idx == std::string::npos)
		{
			return "";
		}

		return sPathName.substr(0, idx);
	}

	std::string GetPathName(const std::string& sDirName, const std::string& sFileName)
	{
		if (sFileName.find_first_of('\\') != std::string::npos)
		{
			//filename contains backslash;
			return "";
		}

		if (sDirName.empty())
		{
			return sFileName;
		}

		if (sFileName.empty())
		{
			return sDirName;
		}

		std::string sRet = sDirName;
		if (sDirName[sDirName.length() - 1] != '\\')
		{
			sRet += '\\';
		}
		sRet += sFileName;

		return sRet;
	}
#endif //AFE_WIN32

	std::string CopyString(CAfStdString* pString)
	{
		if (pString != NULL)
		{
			std::string s2(*pString);
			m_pAf->DeleteString(pString);
			return s2;
		}
		return "";
	}

	std::string* CopyStringArray(CAfStdString* pStringArray, int iArraySize)
	{
		if (iArraySize > 0)
		{
			std::string* pNewArray = new std::string[iArraySize];
			for (int i = 0; i < iArraySize; i++)
			{
				pNewArray[i] = pStringArray[i];
			}
			m_pAf->DeleteStringArray(pStringArray);
			return pNewArray;
		}
		return NULL;
	}

private:

#ifdef AFE_WIN32
	HINSTANCE m_hDll;
#else
	void* m_hDll;
#endif //AFE_WIN32

	CAnyFirewallInterface* m_pAf;

	bool m_bLoadLibraryError;
	bool m_bVersionCheckFailed;
	int m_iAFDLLVersionNumber;

#ifdef AFE_WIN32
	std::string m_sDllPath;  // path to the AnyFirewall DLL
	std::string m_sDllRegPath;
#endif //AFE_WIN32
};

#endif //ANYFIREWALLENGINE_DLL_H

#endif	//__DISABLE_ICE__
