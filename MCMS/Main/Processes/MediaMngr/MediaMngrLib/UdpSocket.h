#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_


#include "CSocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>


class CUdpSocket: public CSocket// : public CStateMachine
{
CLASS_TYPE_1(CUdpSocket, CSocket)
public:
	CUdpSocket(DWORD address=INADDR_ANY, DWORD port=INADDR_ANY);
	virtual ~CUdpSocket();
	
	//virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode) {};
	virtual const char * NameOf() const {return "CUdpSocket";}
	
	
	//int SendTo(const char* buffer, const sockaddr* dest);
	virtual int SendTo(const char* buffer, const sockaddr* dest, DWORD len);
	
	virtual int RecvFrom(char* buffer, unsigned int buf_size, const sockaddr* destination);
	//int RecvFrom(BYTE* buffer, const sockaddr* destination);
	
	virtual int Select(int timeOut);
	
	
protected:
	//int InitUdpSocket(); //Tx	
	int InitUdpSocket(DWORD address, DWORD port);
	
	int CreateUdpSocket();
	int CloseUdpSocket();
	
	int m_descriptor;
	struct sockaddr_in m_client;
	
//private:
		
	//PDECLAR_MESSAGE_MAP
};

#endif /*UDPSOCKET_H_*/
