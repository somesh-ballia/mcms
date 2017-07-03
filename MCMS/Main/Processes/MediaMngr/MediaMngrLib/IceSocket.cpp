#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include <cerrno>
#include "IceSocket.h"
#include "Trace.h"
#include "TraceStream.h"
#include "IceManager.h"


extern IceManager *g_pIceMngr;


IceSocket::IceSocket(int channelID)
	:m_IceChannel(channelID),
	m_AFEngine(0)
{
	if(g_pIceMngr)
		m_AFEngine=g_pIceMngr->GetAFEngine();
	else
		TRACEINTO << "IceSocket:: Init IceSocket failed. The IceManager is not started yet!(g_pIceMngr==0)";
}

IceSocket::~IceSocket()
{
}	
	
int IceSocket::SendTo(const char* buffer, const sockaddr* dest, DWORD len)
{
	if(!m_AFEngine)
		return 0;

	return m_AFEngine->Send(m_IceChannel, buffer, len, AF_BLOCKING);
}

int IceSocket::RecvFrom(char* buffer, unsigned int buf_size, const sockaddr* destination)
{
	if(!m_AFEngine)
		return 0;

	return m_AFEngine->Recv(m_IceChannel, buffer, buf_size, 1000);
}
	
int IceSocket::Select(int timeOut)
{
	if(!m_AFEngine)
		return 0;

	int aChannels[]={m_IceChannel};
	int aInputEvents[]={AF_SELECT_READ};
	int aOutputEvents[]={AF_SELECT_NOEVENT};
	
	int selectResult = m_AFEngine->Select(1, aChannels, aInputEvents, aOutputEvents, timeOut*1000);

	if (selectResult < 0) //Process the error condition
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CIceSocket::Select select() errno=" << myerrno << " description: " << myerrnostr.c_str();
	}
	else if (selectResult == 0) //Process the select timeout condition
	{
		//TRACEINTO << "CUdpSocket::Select select() TIMEOUT";
	}

	return selectResult;	
}
	
#endif //__DISABLE_ICE__
