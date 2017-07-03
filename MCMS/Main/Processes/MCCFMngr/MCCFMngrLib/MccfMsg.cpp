#include "MccfMsg.h"
#include "ApiBaseObject.h"

#include "MccfPackage.h"
#include "MccfPackageFactory.h"

#include <sstream>

#include "TraceStream.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////
CMccfMsg::CMccfMsg(CMccfContext& context, const std::string& transactionID)
	: transactionID_(transactionID)
	, context_(context)
	, status_(mec_OK)
{
}

//////////////////////////////////////////////////////////////////////
bool CMccfMsg::DecodeHeader(CTokenizer& t)
{
	status_ = Decode(t);
	return mec_Syntax_Error != status_;
}

//////////////////////////////////////////////////////////////////////
CMccfBody::CMccfBody(CMccfContext& context, const std::string& transactionID)
	: CMccfMsg(context, transactionID)
	, contentLength_(0)
{
}

//////////////////////////////////////////////////////////////////////
void CMccfBody::ParseBody(const char* msg, size_t size)
{
	FTRACEINTO << "size:" << size << "\n" << msg;

	const CMccfPackageFactory& f = CMccfPackageFactory::const_instance();

	bodyObject_.reset(f.Create(controlPackage_));
	FPASSERT_AND_RETURN(!bodyObject_.get());

	const IMccfPackage* package = f.Lookup(controlPackage_);
	FPASSERT_AND_RETURN(!package); // actually, it may never be NULL here

	package->Decode(msg, size, *bodyObject_);
}

//////////////////////////////////////////////////////////////////////
bool CMccfBody::Dispatch(CSegment* seg, const COsQueue& queue) const
{
	FPASSERT_AND_RETURN_VALUE(!seg, false);

	const IMccfPackage* package = CMccfPackageFactory::const_instance().Lookup(controlPackage_);
	FPASSERT_AND_RETURN_VALUE(!package, false); // actually, it may never be NULL here

	const ApiBaseObject* body = BodyObject();
	FPASSERT_AND_RETURN_VALUE(!body, false); // actually, it may never be NULL here

	*seg << *body;

	return package->DispatchMessage(seg, queue);
}

//////////////////////////////////////////////////////////////////////
CMccfHeaderOnly::CMccfHeaderOnly(CMccfContext& context, const std::string& transactionID)
	: CMccfMsg(context, transactionID)
{
}

//////////////////////////////////////////////////////////////////////
