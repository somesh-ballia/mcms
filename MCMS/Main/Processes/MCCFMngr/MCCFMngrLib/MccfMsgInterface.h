#ifndef MCCFMSGINTERFACE_H__
#define MCCFMSGINTERFACE_H__

/////////////////////////////////////////////////////////////////////////////
#include <iosfwd>

#include "MccfPackage.h"

/////////////////////////////////////////////////////////////////////////////
enum MccfMethodEnum {
	eMCCF_METHOD_NULL = 0,
	eMCCF_METHOD_SYNC,
	eMCCF_METHOD_CONTROL,
	eMCCF_METHOD_REPORT,
	eMCCF_METHOD_KALIVE,
	eMCCF_METHOD_RESPONSE
};

/////////////////////////////////////////////////////////////////////////////
class CMccfMsgFactory;
class CSegment;
class ApiBaseObject;
class CTokenizer;

/////////////////////////////////////////////////////////////////////////////
class IMccfMessage
{
	friend class CMccfMsgFactory;

public:

	virtual ~IMccfMessage() {}

	virtual MccfMethodEnum MsgType() const = 0;

	virtual void   ParseBody(const char* pBuffer, size_t size) = 0;
	virtual size_t BodyLength() const = 0;

	virtual MccfErrorCodesEnum status() const = 0;
	virtual const std::string& transaction() const = 0;

	virtual const ApiBaseObject* BodyObject() const = 0;
	virtual const IMccfPackage*  ControlPackage() const = 0;

	virtual bool Dispatch(CSegment* seg, const COsQueue& queue) const = 0;

private:

	virtual bool DecodeHeader(CTokenizer& t) = 0;
	virtual void EncodeACK(std::ostream& out) const = 0;
};

/////////////////////////////////////////////////////////////////////////////
#endif // MCCFMSGINTERFACE_H__
