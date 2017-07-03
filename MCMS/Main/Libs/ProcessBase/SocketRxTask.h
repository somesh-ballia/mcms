// SocketRxTask.h

#ifndef SOCKETRXTASK_H_
#define SOCKETRXTASK_H_

#include "SocketTask.h"

class COsSocketConnected;
class CMplMcmsProtocol;

class CSocketRxTask : public CSocketTask  
{
CLASS_TYPE_1(CSocketRxTask, CSocketTask)
public:
    CSocketRxTask(void);
	explicit CSocketRxTask(COsSocketConnected* pSocketDesc);

	STATUS Read(char* buffer, int len, int& sizeRead, BYTE partialRcv = FALSE);
	virtual const char* NameOf(void) const;

protected:
	bool ReadValidate_TPKT_Header(const char* bufHdr, DWORD& len) const;
	void OnCorruptedTPKT(const char* bufHdr, const char *msg = "TPKT header are corrupted");
    void OnFailReadFromSocket(const char* msg);
	
	virtual void ReceiveFromSocket(void) = 0;

    virtual int  GetPrivateFileDescriptor(void);
    virtual void HandlePrivateFileDescriptor(void);
};

#endif  // SOCKETRXTASK_H_
