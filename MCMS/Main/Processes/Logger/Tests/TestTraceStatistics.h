#if !defined(TEST_TRACE_STAT_H)
#define TEST_TRACE_STAT_H


// part of official UnitTest library

#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestTraceStat   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestTraceStat );
	CPPUNIT_TEST( testKBSize );
	//...
	CPPUNIT_TEST_SUITE_END();

public:
	void testKBSize();
};

#endif // !defined(TEST_TRACE_STAT_H)

