#ifndef MCCFCONTROLMSG_H__
#define MCCFCONTROLMSG_H__

//////////////////////////////////////////////////////////////////////
#include "MccfMsg.h"

//////////////////////////////////////////////////////////////////////
class CMccfControlMsg : public CMccfBody
{
public:

	CMccfControlMsg(CMccfContext& context, const std::string& transactionID);

	virtual ~CMccfControlMsg()
	{}

	virtual MccfMethodEnum MsgType() const
	{ return eMCCF_METHOD_CONTROL; }

private:

	virtual MccfErrorCodesEnum Decode(CTokenizer& t);
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFSYNCMSG_H__
