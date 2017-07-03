#include "MccfControlMsg.h"

#include "Tokenizer.h"

#include "MccfContext.h"

#include "ApiBaseObject.h"
#include "Segment.h"

#include "Trace.h"
#include "TraceStream.h"

#include <sstream>

//////////////////////////////////////////////////////////////////////
CMccfControlMsg::CMccfControlMsg(CMccfContext& context, const std::string& transactionID)
	: CMccfBody(context, transactionID)
{
}

//////////////////////////////////////////////////////////////////////
MccfErrorCodesEnum CMccfControlMsg::Decode(CTokenizer& t)
{
	for ( ; ; )
	{
		CLexeme key(t.tokenize(' '));
		CLexeme val(t.tokenize(0));

		if (!key && !val)
			break;

		if (key == "Control-Package:")
		{
			CTokenizer t(val);

			controlPackage_.name_ = t.tokenize('/');

			CLexeme vmajor(t.tokenize('.'));

			if (!vmajor.atoi(controlPackage_.version_.maj_))
				return mec_Syntax_Error;

			CLexeme vminor(t.tokenize(0));
			if (!vminor.atoi(controlPackage_.version_.min_))
				return mec_Syntax_Error;

			const IMccfPackage* package = CMccfPackageFactory::const_instance().Lookup(controlPackage_);

			if (!context_.isPackageSupported(controlPackage_))
			{
				FTRACEINTO << "requested package '" << controlPackage_ << "' was not negotiated via SYNC:\n" << context_;
				return mec_Invalid_Target_Package;
			}
		}
		else if (key == "Content-Type:")
		{
			contentType_ = val;
		}
		else if (key == "Content-Length:")
		{
			if (!val.atoi(contentLength_))
				return mec_Syntax_Error;
		}
		else
		{
			FTRACEINTO << "Unexpected Key:" << key;
			return mec_Syntax_Error; // key does not match the message
		}
	}

	FTRACEINTO << "Package:" << controlPackage_ << ", ContentType:" << contentType_ << ", Length:" << contentLength_;
	return t ? mec_Syntax_Error : mec_OK; // OK when get here on the End-of-Message
}

//////////////////////////////////////////////////////////////////////

