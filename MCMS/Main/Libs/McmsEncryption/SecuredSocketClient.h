

#ifndef __SECUREDSOCKETCLIENT_H
#define __SECUREDSOCKETCLIENT_H

#include <openssl/ssl.h>
#include "OsSocketClient.h"


class CSecuredSocketConnected;


class CSecuredSocketClient : public COsSocketClient
{
CLASS_TYPE_1(CSecuredSocketClient,COsSocketClient)	
public:
	CSecuredSocketClient();
	virtual ~CSecuredSocketClient();
    virtual void Close();
	virtual const char* NameOf() const { return "CSecuredSocketClient";}
    
    virtual STATUS ConfigureClientSocket();
    virtual STATUS Connect();
    

    virtual void CreateSocketConnected(COsSocketConnected** pConnected);
    void SetRequestPeerCertificate(BOOL bisRequestPeerCertificate);
    BOOL GetRequestPeerCertificate();
    
protected:
	SSL* m_pSsl;
	SSL_CTX* m_pCtx;	
	BOOL m_isRequestPeerCertificate;

	
};


#endif //__SECUREDSOCKETCLIENT_H
