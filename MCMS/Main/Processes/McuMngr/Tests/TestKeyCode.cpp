#include "TestKeyCode.h"
#include "KeyCode.h"
#include "ValidateKeyCode.h"
#include "ApiStatuses.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestKeyCode );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestKeyCode::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestKeyCode::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestKeyCode::testKeyCodeGeneration()
{
    CKeyCode keycode("");
    char szCreatedKeyCode[128] = {0};
    char option[128];
    strcpy(option, "D7.803FF0404");
    memset(szCreatedKeyCode, 0, sizeof(szCreatedKeyCode));
    keycode.GenerateKeyCode(szCreatedKeyCode, (char *)"9251aBc471", option, 'X');
    CPPUNIT_ASSERT_EQUAL(std::string("X9C8-E79F-9AC0-03FF-0404"), std::string(szCreatedKeyCode));
} 

void CTestKeyCode::testValidateKeyCodeOption()
{
	int failReason = 0;
	//we check for gesher product type for other Tests below change teh productType at before new cpp unit
	eProductType productType = eProductTypeGesher;
/////// Gesher productType
	std::string keyCode = "X0E3-598F-E950-01FF-0004";
	CPPUNIT_ASSERT_EQUAL(STATUS_CFS_MSG_1, ValidateKeyCode(productType, keyCode.c_str(), failReason));

	keyCode = "XEAF-6967-5940-003F-0004";
	CPPUNIT_ASSERT_EQUAL(STATUS_CFS_MSG_1, ValidateKeyCode(productType, keyCode.c_str(), failReason));

	keyCode = "XEAF-6967-5940-03FF-0004";
	CPPUNIT_ASSERT_EQUAL(STATUS_OK, ValidateKeyCode(productType, keyCode.c_str(), failReason));


	keyCode = "XEAF-6967-5940-11FF-0004";
	CPPUNIT_ASSERT_EQUAL(STATUS_CFS_MSG_1, ValidateKeyCode(productType, keyCode.c_str(), failReason));

	keyCode = "XEAF-6967-5940-01FF-0504";
	CPPUNIT_ASSERT_EQUAL(STATUS_CFS_MSG_1, ValidateKeyCode(productType, keyCode.c_str(), failReason));

	keyCode = "XEAF-6967-5940-01FF-0505";
	CPPUNIT_ASSERT_EQUAL(STATUS_CFS_MSG_1, ValidateKeyCode(productType, keyCode.c_str(), failReason));

//end Gesher Product Type

}
