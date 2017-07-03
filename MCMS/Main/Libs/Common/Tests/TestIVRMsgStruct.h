#ifndef TESTIVRMSGSTRUCT_H_
#define TESTIVRMSGSTRUCT_H_



#include <cppunit/extensions/HelperMacros.h>



class CTestIVRMsgStruct : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestIVRMsgStruct );
	CPPUNIT_TEST( testSerializeDeSerialize );
	CPPUNIT_TEST_SUITE_END();
public:
	CTestIVRMsgStruct();
	virtual ~CTestIVRMsgStruct();
	
	void setUp();
	void tearDown();
	
	void testSerializeDeSerialize();
};



#endif /*TESTIVRMSGSTRUCT_H_*/
