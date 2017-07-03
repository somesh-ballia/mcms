#include "MccfMsgFactory.h"

#include "MccfSyncMsg.h"
#include "MccfControlMsg.h"
#include "MccfKeepAliveMsg.h"

#include "ApiBaseObject.h"

#include "Tokenizer.h"

#include "TraceStream.h"

#include <memory> // auto_ptr

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, MccfMethodEnum obj)
{
	return ostr << "m:" << static_cast<size_t>(obj);
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const IMccfPackage* obj)
{
	if (obj)
		ostr << " p:" << *obj;

	return ostr;
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const IMccfMessage* obj)
{
	if (obj)
		ostr << (void*)obj << " " << obj->MsgType() << obj->ControlPackage() << " t:" << obj->transaction();

	return ostr;
}

/////////////////////////////////////////////////////////////////////////////
CMccfMsgFactory::CMccfMsgFactory()
	: signature_("CFW")
	, transactions_(0)
{
	AddToMap<CMccfSyncMsg>("SYNC");
	AddToMap<CMccfControlMsg>("CONTROL");
	AddToMap<CMccfKeepAliveMsg>("K-ALIVE");
}

/////////////////////////////////////////////////////////////////////////////
IMccfMessage* CMccfMsgFactory::DecodeBasicParams(CMccfContext& context, CTokenizer& t, MccfErrorCodesEnum& status) const
{
	FTRACEINTO << t;

	CLexeme signature(t.tokenize(' '));

	if (signature != signature_)
	{
		FTRACEINTO << "Bad Signature:" << signature;
		status = mec_Syntax_Error; // really, we should NEVER get here
		return NULL;
	}

	MessageParams params;

	CLexeme transactionID(t.tokenize(' '));
	params.transactionID = transactionID;

	CLexeme method(t.tokenize(0));

	MethodsMap::const_iterator it = map_.find(method);

	if (it != map_.end())
		return (*it->second)(context, params);

	int code = 0;

	if (!method.atoi(code) || code < mec_FIRST || code >= mec_AFTER_LAST)
	{
		FTRACEINTO << "Unexpected Method or Code: '" << method << '\'';
		status = mec_Unclear_Request;
	}
	else
	{
		// Response got here, currently do nothing with it
		params.code = static_cast<MccfErrorCodesEnum>(code);
		FTRACEINTO << "RESPONSE Code:" << method;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
HMccfMessage CMccfMsgFactory::CreateHeader(CMccfContext& context, const char* pBuffer, size_t size, MccfErrorCodesEnum& status) const
{
	CTokenizer t(pBuffer, size);

	std::auto_ptr<IMccfMessage> header(DecodeBasicParams(context, t, status));

	if (header.get())
	{
		bool ok = header->DecodeHeader(t);
		status = header->status();

		if (!ok)
		{
			FTRACEINTO << "DecodeMessageHeader failed, End-of-Message:" << BOOL2STR(!t) << ", status:" << status;
			return HMccfMessage();
		}
	}

	return HMccfMessage(header.release());
}

/////////////////////////////////////////////////////////////////////////////
void CMccfMsgFactory::EncodeACK(HMccfMessage hrequest, std::string& out, size_t timeout/* = 0*/) const
{
	const IMccfMessage& request = **hrequest;

	MccfErrorCodesEnum code = request.status();
	if (mec_OK == code && request.BodyLength())
		code = mec_Processing;

	std::ostringstream ostr;

	ostr << signature_ << ' ' << request.transaction() << ' ' << code << '\n';

	if (code == mec_Processing)
		ostr << "Timeout: " << timeout << '\n';

	request.EncodeACK(ostr);
	ostr << '\n';

	out = ostr.str();
}

/////////////////////////////////////////////////////////////////////////////
void CMccfMsgFactory::EncodeResponse(
	std::string& out,
	const IMccfPackage& package,
	const ApiBaseObject& response,
	size_t timeout/* = 0*/,
	HMccfMessage request/* = HMccfMessage()*/,
	bool isUpdateReport,
	DWORD seqNum) const
{
	std::string body;
	package.Encode(response, body);

	std::ostringstream ostr;

	ostr << signature_ << ' ';

	if (request)
		ostr << request->transaction();
	else
		ostr << "rmx%" << ++transactions_;

	const char* method = request ? "REPORT" : "CONTROL";

	ostr << ' ' << method << '\n';

	if (request)
	{
		ostr <<
			"Seq: "<<seqNum<<"\n";
		
		if(isUpdateReport)
		{
			ostr <<	"Status: update\n"
			"Timeout: " << timeout << '\n';
			out.reserve(static_cast<size_t>(ostr.tellp()));
			out = ostr.str();
			return;
		}
		else	
		{
			ostr <<	"Status: terminate\n"			
				"Timeout: " << timeout << '\n';
		}
	}

	ostr <<
		"Control-Package: " << package << "\n"
		"Content-Type: " << package.mimeType() << '/' << package.staticTraits().name_ << '+' << package.mimeSubtype() << "\n"
		"Content-Length: " << body.size() << "\n\n";

	out.reserve(static_cast<size_t>(ostr.tellp()) + body.size());
	out = ostr.str();
	out += body;
}

/////////////////////////////////////////////////////////////////////////////
