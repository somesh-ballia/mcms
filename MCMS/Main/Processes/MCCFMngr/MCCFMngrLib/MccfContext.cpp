#include "MccfContext.h"

#include "MccfPackageFactory.h"

#include "MccfRxSocket.h"

#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CMccfContext& obj)
{
	return ostr << "MccfContext: { " << obj.packages_ << " }";
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const MccfPackages& obj)
{
	bool first = true;
	ostr << "Packages: { ";

	for (MccfPackages::const_iterator it = obj.begin(); it != obj.end(); ++it)
	{
		if (!first)
			ostr << ',';

		ostr << (*it)->staticTraits();

		first = false;
	}

	ostr << " }";
	return ostr;
}

//////////////////////////////////////////////////////////////////////
void CMccfContext::OnSync() const
{
	owner_.OnSync();
}

//////////////////////////////////////////////////////////////////////
CMccfContext::~CMccfContext()
{
	CSegment* pSeg = new CSegment;
	*pSeg << owner_.AppServerID();

	for (MccfPackages::const_iterator it = packages_.begin(); it != packages_.end(); ++it)
		if ((*it)->DispatchMessage(pSeg, owner_.twinMailbox(), IMccfPackage::os_Drop))
			FTRACEINTO << "Done deinitializing package:" << (*it)->staticTraits();
}

//////////////////////////////////////////////////////////////////////
bool CMccfContext::isPackageSupported(const Package& controlPackage) const
{
	const IMccfPackage* package = CMccfPackageFactory::const_instance().Lookup(controlPackage);
	return packages_.find(package) != packages_.end();
}

//////////////////////////////////////////////////////////////////////
