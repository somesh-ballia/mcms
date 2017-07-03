
#ifndef TESTSTRUCTTM_H
#define TESTSTRUCTTM_H


#include <cppunit/extensions/HelperMacros.h>

class TestStructTm : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestStructTm );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testConstructor2 );
	CPPUNIT_TEST( testDeSerialze );
	CPPUNIT_TEST( testOrder );
	CPPUNIT_TEST( testSerializeDeSerialze );
	CPPUNIT_TEST( testSerializeDeSerialzeCdr );
	CPPUNIT_TEST( testSerializeDeSerialzeBilling );
	CPPUNIT_TEST( testSerializeDeSerialzeOperAddParty );
	CPPUNIT_TEST( testSerializeSnmp );
	CPPUNIT_TEST( testTimeDelta );
	CPPUNIT_TEST( testTimeAdd1 );
	CPPUNIT_TEST( testTimeAdd2 );
	
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testConstructor();
	void testConstructor2();
	void testDeSerialze();
	void testOrder();
	void testSerializeDeSerialze();
	void testSerializeDeSerialzeCdr();
	void testSerializeDeSerialzeBilling();

	void testSerializeDeSerialzeOperAddParty();

	void testSerializeSnmp();
	
	void testTimeDelta();
	
	void testTimeAdd1();
	void testTimeAdd2();
};

#endif  // TESTSTRUCTTM

