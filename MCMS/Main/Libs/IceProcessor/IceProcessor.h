#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _ICE_PROCESSOR_H_
#define _ICE_PROCESSOR_H_


#include <string>
#include "IceMplMsgSender.h"
#include "IceCmReq.h"
#include "auto_array.h"


class CMplMcmsProtocol;
class CSegment;

class IceProcessor
{
public:
	bool HandleIceMplMsg(CMplMcmsProtocol &mplMsg);
	virtual bool Init(CMplMcmsProtocol &mplMsg)=0;

	virtual bool MakeOffer(CMplMcmsProtocol &mplMsg, int &iceSessionID)=0;
	virtual bool MakeAnswer(CMplMcmsProtocol &mplMsg, int &iceSessionID)=0;
	virtual bool MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg)=0;
	virtual bool MakeOfferNoCreate(CMplMcmsProtocol &mplMsg)=0;
	virtual bool CloseSession(CMplMcmsProtocol &mplMsg)=0;
	//virtual bool ModifySession(void*, int opcode)=0;
	virtual bool ProcessAnswer(CMplMcmsProtocol &mplMsg)=0;

	void SendForMplApi(CMplMcmsProtocol & rMplProtocol) const;

    static auto_array<char> BuildIceSdpIndByReq(const ICE_GENERAL_REQ_S &ICEreq, const std::string &localSdp, int &size);
	void SendCloseSessionInd(CMplMcmsProtocol &mplMsg) const;

	virtual void OnTimerCheckComplete (CSegment * pParam)=0;

	IceProcessor(IceMplMsgSender &mplSender):m_MplMsgSender(mplSender) {};
	virtual ~IceProcessor() { delete &m_MplMsgSender; };

protected:
	IceMplMsgSender &m_MplMsgSender;
};

#endif /*_ICE_PROCESSOR_H_*/

#endif //__DISABLE_ICE__
