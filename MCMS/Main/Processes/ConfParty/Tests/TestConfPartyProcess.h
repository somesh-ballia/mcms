#if !defined(TEST_ConfParty_H)
#define TEST_ConfParty_H

#include <cppunit/extensions/HelperMacros.h>
#include "ConfPartyProcess.h"

// private tests (example)

class CTestConfPartyProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestConfPartyProcess );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testTemp );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();

	void testTemp();

	CProcessBase * m_pConfPartyProcess;

};

#endif // !defined(TEST_ConfParty_H)

