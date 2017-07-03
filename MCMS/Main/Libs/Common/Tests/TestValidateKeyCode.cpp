#include "TestValidateKeyCode.h"
#include "SystemFunctions.h"
#include "ValidateKeyCode.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestValidateKeyCode );

void TestValidateKeyCode::setUp()
{
}


void TestValidateKeyCode::tearDown()
{
}


void TestValidateKeyCode::testBitOperators()
{
	{
		unsigned long BIT_MASK = 0x02000000;
		CPPUNIT_ASSERT(IsBitOn(BIT_MASK, PORTS_UNIT_BIT));
		CPPUNIT_ASSERT(!IsBitOn(BIT_MASK, SVC_OPTION_BIT));
	}
	
	{
		unsigned long BIT_MASK = 0x01000000;
		CPPUNIT_ASSERT(!IsBitOn(BIT_MASK, PORTS_UNIT_BIT));
		CPPUNIT_ASSERT(IsBitOn(BIT_MASK, SVC_OPTION_BIT));
	}

	{
		unsigned long BIT_MASK = 0x03003456;
		CPPUNIT_ASSERT_EQUAL(0x34, BitRangeToNumber(BIT_MASK, SVC_PORT_BIT_BEGIN, SVC_PORT_BIT_END));
		CPPUNIT_ASSERT_EQUAL(0x56, BitRangeToNumber(BIT_MASK, AVC_PORT_BIT_BEGIN, AVC_PORT_BIT_END));
	}
}

void TestValidateKeyCode::testValidateGoodKeyCode()
{
#if 0
	eProductType const productType = eProductTypeGesher;
	
	{
		char const * KEY_CODE = "7803802828";
		CPPUNIT_ASSERT_EQUAL(STATUS_OK, ValidateKeyCode(productType, KEY_CODE));
	}

	/* productType = eProductTypeEdgeAxis;
	{
		char const * KEY_CODE = "D8.101810004";
		CPPUNIT_ASSERT_EQUAL(STATUS_OK, ValidateKeyCode(productType, KEY_CODE));
	}
	{
		char const * KEY_CODE = "D8.101010002";
		CPPUNIT_ASSERT_EQUAL(STATUS_FAIL, ValidateKeyCode(productType, KEY_CODE));
	}*/
#endif
}

void TestValidateKeyCode::testValidateBadKeyCode()
{
#if 0
	eProductType const productType = eProductTypeGesher;
	eProductType const productTypeIrrelevant = eProductTypeSoftMCU;

	{
		char const * KEY_CODE = "7803802728";
		CPPUNIT_ASSERT_EQUAL(STATUS_FAIL, ValidateKeyCode(productType, KEY_CODE));
		CPPUNIT_ASSERT_EQUAL(STATUS_OK, ValidateKeyCode(productTypeIrrelevant, KEY_CODE));
    }

    {
		char const * KEY_CODE = "XEAF-6967-5940-03FF-0404";
		CPPUNIT_ASSERT_EQUAL(STATUS_OK, ValidateKeyCode(productType, KEY_CODE));
	}

	{
		char const * KEY_CODE = "7802800128";
		CPPUNIT_ASSERT_EQUAL(STATUS_FAIL, ValidateKeyCode(productType, KEY_CODE));
		CPPUNIT_ASSERT_EQUAL(STATUS_OK, ValidateKeyCode(productTypeIrrelevant, KEY_CODE));
	}
#endif
}

