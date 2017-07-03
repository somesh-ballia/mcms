#ifndef MCCFPACKAGEFACTORY_H__
#define MCCFPACKAGEFACTORY_H__

//////////////////////////////////////////////////////////////////////
#include <map>
#include <string>

#include "MccfPackage.h"

//////////////////////////////////////////////////////////////////////
class ApiBaseObject;

//////////////////////////////////////////////////////////////////////
typedef std::map<std::string, Version> PackageVersionMap;

//////////////////////////////////////////////////////////////////////
class CMccfPackageFactory : public SingletonHolder<CMccfPackageFactory>
{
public:

	void Register(const IMccfPackage& package);
	bool IsSupported(const Package& package) const;

	const IMccfPackage* Lookup(const Package& package) const;
	ApiBaseObject*      Create(const Package& package) const;

	const std::string   SupportedPackages(const PackageVersionMap& exceptPackages) const;

private:

	// TODO: add package version support??
	typedef std::map<std::string, const IMccfPackage*> PackagesMap;

	PackagesMap packages_;
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFPACKAGEFACTORY_H__
