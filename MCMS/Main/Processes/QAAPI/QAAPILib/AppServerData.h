#ifndef EXTERNALSERVERDATA_H_
#define EXTERNALSERVERDATA_H_


#include "PObject.h"
#include "IpAddressDefinitions.h"

class CAppServerData:public CPObject
{
CLASS_TYPE_1(CAppServerData, CPObject)

public:

    CAppServerData();
    ~CAppServerData();

    virtual const char*  NameOf() const;
    void    Create(char* path = NULL);

    const char*     Getlogin();
    const char*     GetPwd();
    const char*     GetDirectory();
    const char*     GetStrIp();
    const char*		GetUrl();
	mcTransportAddress	GetServerAddr();
	WORD			GetPort();
	BYTE		IsExternalDBAccessEnabled();
	BYTE 		IsExternalDBCnfgValid();
	BYTE		IsConnectionlessTransport();
	
			

protected:
    std::string    m_login, m_pwd, m_directory,m_strIP,m_strUrl;
    mcTransportAddress   m_serverIpAddr;
	
	BYTE 	m_bEnableExternalDBAccess;
	BYTE	m_bExternalDbCnfgValid;
	BYTE    m_bConnectionlessTransport;
};




#endif /*EXTERNALSERVERDATA_H_*/
