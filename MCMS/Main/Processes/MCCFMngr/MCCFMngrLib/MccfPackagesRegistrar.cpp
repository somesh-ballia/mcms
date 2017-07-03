#include "MccfPackagesRegistrar.h"
#include "MccfPackageFactory.h"

/////////////////////////////////////////////////////////////////////////////
// Note: to add support for a new MCCF package, add here the MACRO call:
// BIND_MCCF_PACKAGE(<ApiBaseObject based class name>, "package name from RFC", major version, minor version)

BIND_MCCF_PACKAGE(MscIvr,          "msc-ivr",           1, 0)
BIND_MCCF_PACKAGE(MscPolycomMixer, "msc-polycom-mixer", 1, 0)

/////////////////////////////////////////////////////////////////////////////
void CMccfPackagesRegistrar::RegisterAllPackages()
{
	ApiObjectsFactoriesRegistrar& r(ApiObjectsFactoriesRegistrar::instance());

	r.registerFactory(MscIvrMccfPackageApiFactory::const_instance());
	r.registerFactory(CdrCommonApiFactory::const_instance());

	// NOTE: Add here more ApiObjectFactory-s as needed

	CMccfPackageFactory& f(CMccfPackageFactory::instance());

	f.Register(CMccfIvrPackage::instance());
	f.Register(CMccfCdrPackage::instance());

	// NOTE: Add here more packages as needed
}

/////////////////////////////////////////////////////////////////////////////
