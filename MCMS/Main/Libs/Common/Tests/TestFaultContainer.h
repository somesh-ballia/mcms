#ifndef TESTFAULTCONTAINER_H_
#define TESTFAULTCONTAINER_H_


#include <cppunit/extensions/HelperMacros.h>



class CTestFaultContainer : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestFaultContainer );
	CPPUNIT_TEST( testSerializeDeSerialize );
	CPPUNIT_TEST_SUITE_END();
	
public:
	CTestFaultContainer();
	virtual ~CTestFaultContainer();
	
	void setUp(){}
	void tearDown(){}
	
	void testSerializeDeSerialize();
};

#endif /*TESTFAULTCONTAINER_H_*/
