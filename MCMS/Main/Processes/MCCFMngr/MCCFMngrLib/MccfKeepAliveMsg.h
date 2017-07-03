#ifndef MCCFKEEPALIVEMSG_H__
#define MCCFKEEPALIVEMSG_H__

//////////////////////////////////////////////////////////////////////
#include "MccfMsg.h"

//////////////////////////////////////////////////////////////////////
class CMccfKeepAliveMsg : public CMccfHeaderOnly
{
public:
	CMccfKeepAliveMsg(CMccfContext& context, const std::string& transactionID) : CMccfHeaderOnly(context, transactionID) {}

	virtual ~CMccfKeepAliveMsg() {}

	virtual MccfMethodEnum MsgType() const { return eMCCF_METHOD_KALIVE; }

private:

	virtual void EncodeACK(std::ostream& out) const {}
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFKEEPALIVEMSG_H__
