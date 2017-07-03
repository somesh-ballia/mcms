#include "TestIfConfig.h"
#include "IfConfig.h"
#include "SystemFunctions.h"
#include "SystemFunctions.h"
#include "ConfigManagerApi.h"



extern int IsNiExist(const char *pIfName,	// IN 
			 	 	 bool * result);  		// OUT 

extern int GetNiIpV4Addr(const char *pIfName,   // IN 
					   DWORD *ppIpAddr);      // OUT

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CTestIfConfig );


void CTestIfConfig::setUp()
{
}

void CTestIfConfig::tearDown()
{
}

void CTestIfConfig::testConstructor()
{

}

void CTestIfConfig::testGetNotExistNiIpAddr()
{
	const char *pIfName = "cucu-lulu";//"eth0";
	DWORD ipAddr = 0xffffffff;
	int status = GetNiIpV4Addr(pIfName, &ipAddr);

    CPPUNIT_ASSERT(IFCONFIG_NI_NOTEXIST == status);
}

void CTestIfConfig::testGetExistNiIpAddr()
{
	std::string sMngmntNI = "eth0";
	// should be replaced with the actual NI, however
	//   the following code does not compile since 'ProcessBase' is not initiated
//	eConfigInterfaceNum mngmntIfNum = GetInterfaceNum(eManagmentNetwork);
//	sMngmntNI = GetConfigInterfaceNumName(mngmntIfNum);

	DWORD ipAddr = 0xffffffff;
	int status = GetNiIpV4Addr(sMngmntNI.c_str(), &ipAddr);

    CPPUNIT_ASSERT(IFCONFIG_OK == status);
}

void CTestIfConfig::testIsNiExist()
{
  std::string sMngmntNI = "eth1";
	// should be replaced with the actual NI, however
	//   the following code does not compile since 'ProcessBase' is not initiated
//	eConfigInterfaceNum mngmntIfNum = GetInterfaceNum(eManagmentNetwork);
//	string sMngmntNI = GetConfigInterfaceNumName(mngmntIfNum);

	bool resultExist = false;
	int status = IsNiExist(sMngmntNI.c_str(), &resultExist);
	
	sMngmntNI = "cucu-lulu";
	bool resultNotExist = true;
	status = IsNiExist(sMngmntNI.c_str(), &resultNotExist);
	
    CPPUNIT_ASSERT(true == resultExist);
    CPPUNIT_ASSERT(false == resultNotExist);
}
