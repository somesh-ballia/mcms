#ifndef __AUDITOR_RXSOCKET_H__
#define __AUDITOR_RXSOCKET_H__

#include "SocketRxTask.h"
#include "MplMcmsStructs.h"



class CMplMcmsProtocol;


// should be more then 2^16 - max len of a message,
// there is only 2 bytes for len in TPKT
const DWORD BufferLen = 1024 * 100;

extern "C" void AuditorSocketRxEntryPoint(void* appParam);



class CAuditorRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CAuditorRxSocket,CSocketRxTask )	
public:
	CAuditorRxSocket();
	virtual ~CAuditorRxSocket();
	virtual const char*  NameOf() const {return "CAuditorRxSocket";}
	virtual const char * GetTaskName()const{return "CAuditorRxSocket";}
  
	void InitTask();
	BOOL IsSingleton() const {return NO;}

	void ReceiveFromSocket();
    
    
private:
    bool ValidateHeaders(const CMplMcmsProtocol & prot);
    void PrintCorruptedEvent(CMplMcmsProtocol &prot, const string & errorMessage);
    


    
	BYTE m_SocketBuffer[BufferLen + 1];
};

#endif /*__AUDITOR_RXSOCKET_H__*/
