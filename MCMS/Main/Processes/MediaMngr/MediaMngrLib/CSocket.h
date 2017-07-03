#ifndef _MM_CSOCKET_H_
#define _MM_CSOCKET_H_
#include "PObject.h"

struct sockaddr;

class CSocket : public CPObject
{
CLASS_TYPE_1(CSocket,CPObject)

public:
	CSocket() {};
	virtual ~CSocket() {};
	
	virtual const char * NameOf() const {return "CSocket";}
	
	virtual int SendTo(const char *buffer, const sockaddr *dest, DWORD len)=0;
	virtual int RecvFrom(char *buffer, unsigned int buf_size, const sockaddr *src)=0;
	
	virtual int Select(int timeOut)=0;
	
};

#endif
