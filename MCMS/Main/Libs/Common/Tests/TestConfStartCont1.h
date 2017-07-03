#ifndef TESTCONFSTARTCONT1_H_
#define TESTCONFSTARTCONT1_H_


#include <cppunit/extensions/HelperMacros.h>



class CTestConfStartCont1 : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestConfStartCont1 );
	CPPUNIT_TEST( testSerializeDeSerialize );
	CPPUNIT_TEST_SUITE_END();
public:
	CTestConfStartCont1();
	virtual ~CTestConfStartCont1();
	
	void setUp();
	void tearDown();
	
	void testSerializeDeSerialize();
};




#endif /*TESTCONFSTARTCONT1_H_*/
