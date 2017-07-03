#include "TestGetMacAddr.h"
#include "GetMacAddr.h"
#include "Trace.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestGetMacAddr );

void CTestGetMacAddr::setUp()
{
}

void CTestGetMacAddr::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestGetMacAddr::testGetMacWithoutSemiColon()
{
    std::string mac = GetMacAddrWithoutSemiColon("eth0");
    
    CPPUNIT_ASSERT_MESSAGE( "CTestGetMacAddr::testGetMacWithoutColon should return non-empty mac", !mac.empty());
} 

void CTestGetMacAddr::testGetMacWithSemiColon()
{
    char bufMac[64];
    char const * mac = GetMacAddrWithSemiColon("eth0", bufMac, sizeof(bufMac));
    
    CPPUNIT_ASSERT_MESSAGE( "CTestGetMacAddr::testGetMacWithoutColon should return non-empty mac", (0!=mac[0]));
    fprintf(stderr, "%s:%s:%d: mac |%s|\n", __FILE__, __func__, __LINE__, mac);
} 


