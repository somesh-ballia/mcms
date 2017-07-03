#if !defined(CTESTGETREQUEST_H__)
#define CTESTGETREQUEST_H__

#include <cppunit/extensions/HelperMacros.h>

class CTestGetRequest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestGetRequest );
	CPPUNIT_TEST( testGetManagerTransectionsQueue );
	CPPUNIT_TEST( testSendMessage );
	CPPUNIT_TEST_SUITE_END();

public:

	void setUp();
	void tearDown(); 
	
	void testGetManagerTransectionsQueue();
	void testSendMessage();

};

#endif // !defined(CTESTGETREQUEST_H__)
