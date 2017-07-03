#include "MccfSyncMsg.h"
#include "Tokenizer.h"

#include "MccfContext.h"

#include "Trace.h"
#include "TraceStream.h"

#include <ostream>

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& out, const PackageVersionMap& obj)
{
	bool first = true;

	for (PackageVersionMap::const_iterator it = obj.begin(); it != obj.end(); ++it)
	{
		if (!first)
			out << ',';

		out << it->first << '/' << it->second;

		first = false;
	}

	return out;
}

//////////////////////////////////////////////////////////////////////
CMccfSyncMsg::CMccfSyncMsg(CMccfContext& context, const std::string& transactionID)
	: CMccfHeaderOnly(context, transactionID)
	, keepAlive_(0)
{
}

//////////////////////////////////////////////////////////////////////
MccfErrorCodesEnum CMccfSyncMsg::Decode(CTokenizer& t)
{
	for ( ; ; )
	{
		CLexeme key(t.tokenize(' '));
		CLexeme val(t.tokenize(0));

		if (!key && !val)
			break;

		if (key == "Packages:")
		{
			CTokenizer t(val);

			for ( ; ; )
			{
				CLexeme package(t.tokenize(','));

				if (!package)
					break;

				Package p;

				CTokenizer t(package);
				p.name_ = t.tokenize('/');

				CLexeme vmaj(t.tokenize('.'));

				if (!vmaj.atoi(p.version_.maj_))
					return mec_Syntax_Error;

				CLexeme vmin = t.tokenize(0);
				if (!vmin.atoi(p.version_.min_))
					return mec_Syntax_Error;

				// TODO: version check support!!
				if (CMccfPackageFactory::const_instance().IsSupported(p))
					packages_.insert(std::make_pair(p.name_, p.version_));
				else
					FTRACEINTO << "Unsupported Package:" << p;
			}
		}
		else if (key == "Dialog-id:")
		{
			dialogID_ = val;
		}
		else if (key == "Keep-alive:")
		{
			if (!val.atoi(keepAlive_))
				return mec_Syntax_Error;
		}
		else
		{
			FTRACEINTO << "Key:" << key;
			return mec_Syntax_Error; // key does not match the message
		}
	}

	FTRACEINTO << "DialogID:" << dialogID_ << ", Keep-Alive:" << keepAlive_; // TODO: remove
	return t ? mec_Syntax_Error : (packages_.empty() ? mec_No_Packages_Supported : mec_OK); // OK when get here on the End-of-Message
}

//////////////////////////////////////////////////////////////////////
void CMccfSyncMsg::EncodeACK(std::ostream& out) const
{
	out << "Keep-alive: " << keepAlive_ << '\n';

	if (!packages_.empty())
		out << "Packages: " << packages_ << '\n';

	const std::string supported(CMccfPackageFactory::const_instance().SupportedPackages(packages_));

	if (!supported.empty())
		out << "Supported: " << supported << '\n';
}

//////////////////////////////////////////////////////////////////////
bool CMccfSyncMsg::Dispatch(CSegment* seg, const COsQueue& queue) const
{
	for (MccfPackages::const_iterator it = context_.packages_.begin(); it != context_.packages_.end(); ++it)
		(*it)->DispatchMessage(seg, queue, IMccfPackage::os_Drop);

	context_.packages_.clear();

	context_.dialogID_ = dialogID_;

	const CMccfPackageFactory& f = CMccfPackageFactory::const_instance();

	for (PackageVersionMap::const_iterator it = packages_.begin(); it != packages_.end(); ++it)
	{
		const IMccfPackage* package = f.Lookup(Package(it->first, it->second));
		FPASSERT_AND_RETURN_VALUE(!package, false); // actually, it may never be NULL here

		context_.packages_.insert(package);
		package->DispatchMessage(seg, queue, IMccfPackage::os_Sync);
	}

	context_.OnSync();

	return true;
}

//////////////////////////////////////////////////////////////////////
