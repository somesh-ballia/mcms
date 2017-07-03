#ifndef IPSERVICEVALIDATOR_H_
#define IPSERVICEVALIDATOR_H_

#include "PObject.h"

class CIPService;
class CIPServiceList;
class CIpServiceValidator;
class CObjString;

typedef STATUS (CIpServiceValidator::*ValidateMethodType)(CObjString &errorMsg);


class CIpServiceValidator : public CPObject
{
CLASS_TYPE_1(CIpServiceValidator, CPObject)
public:
	CIpServiceValidator(CIPService & service);
	virtual ~CIpServiceValidator();
	virtual const char* NameOf() const { return "CIpServiceValidator";}

	STATUS ValidateBase(CObjString &errorMsg);
	STATUS ValidateFullCS(CObjString &errorMsg);
    STATUS ValidateFullMNGMNT(CObjString &errorMsg);

//	STATUS ValidateDuplicateIpAddrCS(CObjString &errorMsg); // moved to CCSMngr process
	STATUS ValidateGK(CObjString &errorMsg);
	STATUS ValidateSpanList(CObjString &errorMsg);
	STATUS ValidateIpAddressWithMask(CObjString &errorMsg);
    STATUS ValidateTCPPortRange(CObjString &errorMsg);
    STATUS ValidateIpAddressLocality(CObjString &errorMsg);
    STATUS ValidateIpAddress(CObjString &errorMsg);
    STATUS ValidateSipServers(CObjString &errorMsg);
    STATUS ValidateDnsServers(CObjString &errorMsg);
    
private:
	// disabled
	CIpServiceValidator(const CIpServiceValidator&);
	CIpServiceValidator&operator=(const CIpServiceValidator&);

	STATUS ValidateGKPrefixName(const char *prefixName)const;

	CIPService & m_Service;
};








class CIpServiceListValidator : public CPObject
{
CLASS_TYPE_1(CIpServiceListValidator, CPObject)
public:
	CIpServiceListValidator(CIPServiceList & serviceList);
	virtual ~CIpServiceListValidator();
	virtual const char* NameOf() const { return "CIpServiceListValidator";}

	STATUS ValidateSingleCS(CIPService & service, bool isMustExist, bool isMustNotExist, CObjString &errorMsg)const;
	STATUS ValidateBase(CObjString &errorMsg);
	STATUS ValidateFullCS(CObjString &errorMsg);
    STATUS ValidateFullMNGMNT(CObjString &errorMsg);
//	STATUS ValidateDuplicateIpAddrCS(CObjString &errorMsg);
	STATUS ValidateTCPPortRange(CObjString &errorMsg);
    STATUS ValidateIpAddressLocality(CObjString &errorMsg);
    STATUS ValidateIpAddress(CObjString &errorMsg);
    int    RemoveSipTls(CObjString &errorMsg);


private:
	// disabled
	CIpServiceListValidator(const CIpServiceListValidator&);
	CIpServiceListValidator&operator=(const CIpServiceListValidator&);

	STATUS ApplyValidateMethod(ValidateMethodType method, CObjString &errorMsg);


	CIPServiceList & m_ServiceList;
};




#endif /*IPSERVICEVALIDATOR_H_*/
