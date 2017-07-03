#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _ICE_SOCKET_H_
#define _ICE_SOCKET_H_

#include "CSocket.h"
#include "AnyFirewallEngine_dll.h"


class IceSocket: public CSocket
{
CLASS_TYPE_1(IceSocket,CSocket)

public:
	IceSocket(int channelID);
	virtual ~IceSocket();
	
	virtual const char * NameOf() const {return "IceSocket";}
	
	
	virtual int SendTo(const char* buffer, const sockaddr* dest, DWORD len);
	virtual int RecvFrom(char* buffer, unsigned int buf_size, const sockaddr* destination);
	
	virtual int Select(int timeOut);
	
	
protected:
	
	int m_IceChannel;
	CAnyFirewallEngine *m_AFEngine;
	
};

#endif

#endif //__DISABLE_ICE__
