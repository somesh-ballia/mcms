#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _ICE_MM_TRANSFER_PROC_H_
#define _ICE_MM_TRANSFER_PROC_H_


#include <memory>
#include <map>
#include "auto_array.h"
#include "IceProcessor.h"



class CMplMcmsProtocol;
class CSegment;



class IceMMTransferProcessor : public IceProcessor
{
public:
	bool Init(CMplMcmsProtocol &mplMsg);

	bool MakeOffer(CMplMcmsProtocol &mplMsg, int &sessID);
	bool MakeAnswer(CMplMcmsProtocol &mplMsg, int &sessID);
	bool MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg);
	bool MakeOfferNoCreate(CMplMcmsProtocol &mplMsg);
	bool CloseSession(CMplMcmsProtocol &mplMsg);
	//bool ModifySession(void*, int opcode);
	bool ProcessAnswer(CMplMcmsProtocol &mplMsg);
	void OnTimerCheckComplete (CSegment * pParam) {};

	IceMMTransferProcessor(IceMplMsgSender &mplSender):IceProcessor(mplSender) {};
	~IceMMTransferProcessor() {};
};


#endif

#endif	//__DISABLE_ICE__
