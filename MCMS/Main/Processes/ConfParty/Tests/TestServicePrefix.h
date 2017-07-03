#ifndef TEST_PREFIX_SERVICE_H
#define TEST_PREFIX_SERVICE_H

#include <cppunit/extensions/HelperMacros.h>
#include "ServicePrefixStr.h"


class CTestServicePrefixStr   : public CPPUNIT_NS::TestFixture
{
    
	CPPUNIT_TEST_SUITE( CTestServicePrefixStr );
 	CPPUNIT_TEST(TestAdd);
	CPPUNIT_TEST(TestSerialixeXml);
	
	//...
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown(); 
	
	void TestAdd();
	void TestSerialixeXml();

private:
	WORD m_numServicePrefixStr;
	CServicePrefixStr* m_pServicePrefixStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
};

#endif
