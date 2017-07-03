#ifndef MCCFSYNCMSG_H__
#define MCCFSYNCMSG_H__

//////////////////////////////////////////////////////////////////////
#include "MccfMsg.h"
#include "MccfPackageFactory.h"

#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////
class CMccfSyncMsg : public CMccfHeaderOnly
{
public:
	CMccfSyncMsg(CMccfContext& context, const std::string& transactionID);

	virtual ~CMccfSyncMsg() {}

	virtual MccfMethodEnum MsgType() const
	{ return eMCCF_METHOD_SYNC; }

 private:

	virtual MccfErrorCodesEnum Decode(CTokenizer& t);

	virtual void EncodeACK(std::ostream& out) const;
	virtual bool Dispatch(CSegment* seg, const COsQueue& queue) const;

private:

	PackageVersionMap packages_;

	std::string dialogID_;
	unsigned int keepAlive_;
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFSYNCMSG_H__
