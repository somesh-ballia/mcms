#ifndef TESTTERMINALCOMMAND_H_
#define TESTTERMINALCOMMAND_H_



#include <cppunit/extensions/HelperMacros.h>



class TestTerminalCommand : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestTerminalCommand );
//	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testSerializeDeserializeSegment );
	CPPUNIT_TEST( testSerializeDeserializeStream );
	CPPUNIT_TEST_SUITE_END();

public:
	TestTerminalCommand();
	virtual ~TestTerminalCommand();
	
	void testSerializeDeserializeSegment();
	void testSerializeDeserializeStream();	
};

#endif /*TESTTERMINALCOMMAND_H_*/
