#if !defined(TEST_DEMO_H)
#define TEST_DEMO_H


#include <cppunit/extensions/HelperMacros.h>


class CTestEncryptionKeyServerProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestEncryptionKeyServerProcess );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testCreateEncryptionKey );
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testCreateEncryptionKey();
};

#endif // !defined(TEST_DEMO_H)
