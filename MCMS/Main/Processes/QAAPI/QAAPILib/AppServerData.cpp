// AppServerData.cpp: implementation of the CAppServerData class.
//
//////////////////////////////////////////////////////////////////////


#include "AppServerData.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"

/*****************************************************
class   CAppServerData
******************************************************/

/////////////////////////////////////////////////////////////////
CAppServerData::CAppServerData()
{
    //m_login = new char[H243_NAME_LEN];
    //m_pwd = new char[H243_NAME_LEN];
    //m_directory = new char[NEW_FILE_NAME_LEN];
    memset(&m_serverIpAddr, 0, sizeof(mcTransportAddress));
    m_bEnableExternalDBAccess=FALSE;
    m_bExternalDbCnfgValid=FALSE;
    m_bConnectionlessTransport=FALSE;
}

/////////////////////////////////////////////////////////////////
CAppServerData::~CAppServerData()
{
    //PDELETE(m_login);
    //PDELETE(m_pwd);
    //PDELETE(m_directory);
}

/////////////////////////////////////////////////////////////////////////////
const char*   CAppServerData::NameOf()  const
{
        return "CAppServerData";
}

/////////////////////////////////////////////////////////////////
void CAppServerData::Create(char* path)
{

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string data;
    std::string key = CFG_KEY_JITC_MODE;
    BOOL bJITCMode;
    sysConfig->GetBOOLDataByKey(key, bJITCMode);
    if (bJITCMode) // QAAPI process should be disaled in JITC mode
        return;
    
    
	sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_EXTERNAL_DB_ACCESS, m_bEnableExternalDBAccess);

/*  =========== External DB configuratiom in Version 6 is taken from system.cfg ====*/
/*  ================================================================================*/

	if(m_bEnableExternalDBAccess)
	{
		
		char strPort[100];
		DWORD wTempPort=0;
		m_bExternalDbCnfgValid=TRUE;
		
		sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_IP, data);       
		
		mcTransportAddress ipAddress;
		memset(&ipAddress, 0, sizeof(mcTransportAddress));
		stringToIp(&ipAddress, (char*)data.c_str(), eNetwork);
		
		PTRACE2(eLevelError, "CAppServerData::Create string " , data.c_str());
		PTRACE2INT(eLevelError, "CAppServerData::Create ip=", (DWORD)ipAddress.addr.v4.ip);
		
		if(!isIpTaNonValid(&ipAddress))
			m_serverIpAddr= ipAddress;
		else
		{
			m_bExternalDbCnfgValid = FALSE;
			PASSERT(1);
		}
		sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_IP,m_strIP);
		
		
		sysConfig->GetDWORDDataByKey(CFG_KEY_EXTERNAL_DB_PORT,wTempPort);
		if(wTempPort != 0xFFFF)
			m_serverIpAddr.port = wTempPort;
		else
		{
			m_bExternalDbCnfgValid = FALSE;
			PASSERT(2);
		}
		
		sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_DIRECTORY,m_directory);
		
		sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_LOGIN,m_login);
		
		sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_EXTERNAL_DB_CONNECTIONLESS, m_bConnectionlessTransport);

		if(m_login.length() == 0)
		{
			m_bExternalDbCnfgValid = FALSE;
			PASSERT(4);
		}
		
		sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_PASSWORD,m_pwd);
			
		if(m_pwd.length() == 0)
		{
			m_bExternalDbCnfgValid = FALSE;
			PASSERT(5);
		}
		
		//if(!m_bIsExternalDbOn)
		//	m_bLoaded = TRUE;
		//else
		//{
			
			//::GetpHlogApi()->FileError(BAD_FILE,SYSTEM_CONFIGURATION,"system.cfg");
		//}
		
		sprintf(strPort,"%d" ,wTempPort);
		
		if (wTempPort==443)
			m_strUrl="https://"+m_strIP+":"+strPort+"/"+m_directory;
		else
			m_strUrl="http://"+m_strIP+":"+strPort+"/"+m_directory;
	}
}

/////////////////////////////////////////////////////////////////
const char* CAppServerData::Getlogin()
{
        return m_login.c_str();
}

/////////////////////////////////////////////////////////////////
const char* CAppServerData::GetPwd()
{
        return m_pwd.c_str();
}

/////////////////////////////////////////////////////////////////
const char* CAppServerData::GetDirectory()
{
        return m_directory.c_str();
}
/////////////////////////////////////////////////////////////////
const char* CAppServerData::GetStrIp()
{
        return m_strIP.c_str();
}

/////////////////////////////////////////////////////////////////
mcTransportAddress CAppServerData::GetServerAddr()
{
        return m_serverIpAddr;
}

/////////////////////////////////////////////////////////////////
WORD CAppServerData::GetPort()
{
	return m_serverIpAddr.port;
}

/////////////////////////////////////////////////////////////////
const char* CAppServerData::GetUrl()
{
        return m_strUrl.c_str();
}
/////////////////////////////////////////////////////////////////
BYTE CAppServerData::IsExternalDBAccessEnabled()
{
        return m_bEnableExternalDBAccess;
}

/////////////////////////////////////////////////////////////////
BYTE CAppServerData::IsConnectionlessTransport()
{
        return m_bConnectionlessTransport;
}

/////////////////////////////////////////////////////////////////
BYTE CAppServerData::IsExternalDBCnfgValid()
{
        return m_bExternalDbCnfgValid;
}


