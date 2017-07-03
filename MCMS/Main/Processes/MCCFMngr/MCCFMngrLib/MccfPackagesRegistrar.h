#ifndef MCCF_PACKAGES_REGISTRAR_H__
#define MCCF_PACKAGES_REGISTRAR_H__

/////////////////////////////////////////////////////////////////////////////
#include "MccfPackage.h"
#include "MccfCodec.h"

#include "OpcodesMcmsInternal.h"

/////////////////////////////////////////////////////////////////////////////
#include "MscIvr.h"
#include "MscPolycomMixer.h"

/////////////////////////////////////////////////////////////////////////////
// Note: to add support for a new MCCF package, add here the MACRO call:
// DECLARE_MCCF_PACKAGE(<wanted package name class>, <ApiBaseObject based class name>, <Codec class>, <Process to dispatch the package message>, <Opcode for SYNC>, <Opcode to send>)

DECLARE_MCCF_PACKAGE(CMccfIvrPackage, MscIvr,          CMccfXmlCodec, eProcessConfParty, 0, 0, MCCF_IVR_PACKAGE_REQUEST)
DECLARE_MCCF_PACKAGE(CMccfCdrPackage, MscPolycomMixer, CMccfXmlCodec, eProcessConfParty, MCCF_CHANNEL_SYNC, MCCF_CHANNEL_DROP, 0)

/////////////////////////////////////////////////////////////////////////////
class CMccfPackagesRegistrar
{
public:

	static void RegisterAllPackages();

private:

	CMccfPackagesRegistrar();
	~CMccfPackagesRegistrar();
};

/////////////////////////////////////////////////////////////////////////////
#endif // MCCF_PACKAGES_REGISTRAR_H__
