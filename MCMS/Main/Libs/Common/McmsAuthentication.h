
#ifndef MCMS_AUTHENTICATION_H__
#define MCMS_AUTHENTICATION_H__

#include <ostream>
#include "DataTypes.h"
#include "CommonStructs.h"
#include "ObjString.h"

typedef struct McmsAuthenticationS
{
	DWORD	productType;
	BYTE	switchBoardId;
	BYTE	switchSubBoardId;
    DWORD	rmxSystemCardsMode;
    VERSION_S	chassisVersion;
    BYTE    isCtrlNewGeneration;
} MCMS_AUTHENTICATION_S;


typedef struct McmsInfoS
{
	char	serialNumber[30];
	char	ipv4[30];
	char	ipv6[65];
	VERSION_S	chassisVersion;
//	char   	hostName[30];
} MCMS_INFO_S;


std::ostream& operator<< (std::ostream& os, const McmsAuthenticationS& obj );

class CMcmsAuthentication
{

public:

    static BOOL	IsForceStrongPassword();
    static STATUS IsLegalStrongPassword(const std::string lopgin, const std::string password, CObjString &description);
    static STATUS VerifyStrongPassword(char * password,CObjString & description);


};

#endif

