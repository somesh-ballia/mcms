//+========================================================================+
//                            OsSocketConnected.h                          |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |

#ifndef __SECUREDSOCKETCONNECTED_H
#define __SECUREDSOCKETCONNECTED_H


#include <ostream>
#include <openssl/ssl.h>
#include "OsSocketConnected.h"


class CSegment;

class CSecuredSocketConnected: public COsSocketConnected
{

public:

	CSecuredSocketConnected(int size=128*1024-1,int threashold=-1);
    
	virtual ~CSecuredSocketConnected();
    
    virtual int Receive(char * buffer,int bytesToRead);
    
    virtual int Send(const char* buffer, int bytesToWrite);
    
    virtual void Serialize(CSegment& seg) const;
    virtual void DeSerialize(CSegment& seg);
    
    virtual void SetTlsParams(void* ssl);
    virtual BYTE IsSecured();

	SSL* m_pSsl;
};

	
std::ostream& operator<< (std::ostream& os,
                          const CSecuredSocketConnected& socket);


#endif 
