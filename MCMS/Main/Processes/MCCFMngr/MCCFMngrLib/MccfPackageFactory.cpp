#include "MccfPackageFactory.h"
#include "ApiBaseObject.h"

#include <sstream>

#include "TraceStream.h"
#include "Macros.h"

/////////////////////////////////////////////////////////////////////////////
void CMccfPackageFactory::Register(const IMccfPackage& package)
{
	std::pair<PackagesMap::iterator, bool> res = packages_.insert(std::make_pair(package.Traits().name_, &package));

	FTRACEINTO << "Package:" << package << ", Succeeded:" << BOOL2STR(res.second);
}

/////////////////////////////////////////////////////////////////////////////
bool CMccfPackageFactory::IsSupported(const Package& package) const
{
	// TODO: add version comparison check
	PackagesMap::const_iterator it = packages_.find(package.name_);

	return (it != packages_.end());
}

/////////////////////////////////////////////////////////////////////////////
ApiBaseObject* CMccfPackageFactory::Create(const Package& package) const
{
	// TODO: add version comparison check
	PackagesMap::const_iterator it = packages_.find(package.name_);

	return (it != packages_.end()) ? it->second->Create() : NULL;
}

/////////////////////////////////////////////////////////////////////////////
const IMccfPackage* CMccfPackageFactory::Lookup(const Package& package) const
{
	PackagesMap::const_iterator it = packages_.find(package.name_);

	return (it != packages_.end()) ? it->second : NULL;
}

/////////////////////////////////////////////////////////////////////////////
const std::string CMccfPackageFactory::SupportedPackages(const PackageVersionMap& exceptPackages) const
{
	std::ostringstream out;
	bool first = true;

	const PackageVersionMap::const_iterator NOT_FOUND = exceptPackages.end();

	for (PackagesMap::const_iterator it = packages_.begin(); it != packages_.end(); ++it)
	{
		if (exceptPackages.find(it->first) != NOT_FOUND)
			continue;

		if (!first)
			out << ',';

		out << *it->second;

		first = false;
	}

	return out.str();
}

/////////////////////////////////////////////////////////////////////////////
