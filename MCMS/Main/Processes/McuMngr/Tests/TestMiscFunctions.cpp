#include "TestMiscFunctions.h"
#include "strip_string.h"
#include "Trace.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMiscFunctions );

void CTestMiscFunctions::setUp()
{
}

void CTestMiscFunctions::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestMiscFunctions::testStripLeft()
{
    char bufSrc[128];
    strcpy(bufSrc, " \t \n Abc");
    inplace_trim(bufSrc);
    CPPUNIT_ASSERT_EQUAL(0, strcmp(bufSrc, "Abc"));
} 

void CTestMiscFunctions::testStripRight()
{
    char bufSrc[128];
    strcpy(bufSrc, "Abc \t\n ");
    inplace_trim(bufSrc);
    CPPUNIT_ASSERT_EQUAL(0, strcmp(bufSrc, "Abc"));
} 

void CTestMiscFunctions::testStripBoth()
{
    char bufSrc[128];
    strcpy(bufSrc, " \t \n A b\tc\n ");
    inplace_trim(bufSrc);
    CPPUNIT_ASSERT_EQUAL(0, strcmp(bufSrc, "A b\tc"));
} 

