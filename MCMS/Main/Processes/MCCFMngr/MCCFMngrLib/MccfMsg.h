#ifndef MCCFMSG_H__
#define MCCFMSG_H__

//////////////////////////////////////////////////////////////////////
#include "MccfMsgInterface.h"

#include <string>
#include <memory>

#include "MccfPackage.h"
#include "MccfPackageFactory.h"

//////////////////////////////////////////////////////////////////////
class ApiBaseObject;
class CMccfContext;

//////////////////////////////////////////////////////////////////////
class CMccfMsg : public IMccfMessage
{
public:

	CMccfMsg(CMccfContext& context, const std::string& transactionID);

	virtual MccfErrorCodesEnum status() const
	{ return status_; }

	virtual const std::string& transaction() const
	{ return transactionID_; }

private:

	virtual MccfErrorCodesEnum Decode(CTokenizer& t) = 0;

	virtual bool DecodeHeader(CTokenizer& t); // calls the Decode above, translates the status to bool

protected:

	const std::string  transactionID_;

	CMccfContext&      context_;

	MccfErrorCodesEnum status_;
};

//////////////////////////////////////////////////////////////////////
class CMccfBody : public CMccfMsg
{
public:

	CMccfBody(CMccfContext& context, const std::string& transactionID);

	virtual size_t BodyLength() const
	{ return contentLength_; }

	virtual void ParseBody(const char* msg, size_t size);

	virtual const IMccfPackage* ControlPackage() const
	{ return CMccfPackageFactory::const_instance().Lookup(controlPackage_); }

protected:

	virtual void EncodeACK(std::ostream& out) const
	{}

	virtual const ApiBaseObject* BodyObject() const
	{ return bodyObject_.get(); }

private:

	virtual bool Dispatch(CSegment* seg, const COsQueue& queue) const;

protected:

	std::auto_ptr<ApiBaseObject> bodyObject_;
	size_t                       contentLength_;

	Package                      controlPackage_;
	std::string                  contentType_;
};

//////////////////////////////////////////////////////////////////////
class CMccfHeaderOnly : public CMccfMsg
{
public:

	CMccfHeaderOnly(CMccfContext& context, const std::string& transactionID);

	virtual size_t BodyLength() const
	{ return 0; }

	virtual void ParseBody(const char* msg, size_t size)
	{}

	virtual const IMccfPackage* ControlPackage() const
	{ return NULL; }

 private:

	virtual const ApiBaseObject* BodyObject() const
	{ return NULL; }

	virtual MccfErrorCodesEnum Decode(CTokenizer& t)
	{ return mec_OK; }

	virtual bool Dispatch(CSegment* seg, const COsQueue& queue) const
	{ return false; }
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFMSG_H__
