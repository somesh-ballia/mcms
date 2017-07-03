

#include "TestObjString.h"
#include "ObjString.h"




// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CTestObjString );


// CTestObjString::CTestObjString()
// {
// }

// CTestObjString::~CTestObjString()
// {
// }


bool Predicat(char ch)
{
    return ch == '_';
}



void CTestObjString::testReplaceChar()
{
	CSmallString str = "cucu_lulu_cucu_lulu";
	str.ReplaceChar('c', 'l');
	const char *strExpected = "lulu_lulu_lulu_lulu";

	CPPUNIT_ASSERT(strcmp(str.GetString(), strExpected) == 0);

	char sstr[64];
	strcpy(sstr,"cucu_lulu_cucu_lulu");
	CObjString::ReplaceChar(sstr,'c','l');

	CPPUNIT_ASSERT(strcmp(sstr, strExpected) == 0);

	str = "cucu_lulu_cucu_lulu";
	str.ReplaceChar(Predicat, '-');
	strExpected = "cucu-lulu-cucu-lulu";

	CPPUNIT_ASSERT(strcmp(str.GetString(), strExpected) == 0);
}


void CTestObjString::testRemoveChar()
{
    CSmallString str = "cucu_lulu_cucu_lulu";
    str.RemoveChars("lulu");
    const char *strExpected = "cucu__cucu_";

    CPPUNIT_ASSERT(strcmp(str.GetString(), strExpected) == 0);
}


void CTestObjString::testReverse()
{
    CSmallString str = "123456789";
    str.Reverse();
    const char *strExpected = "987654321";

    CPPUNIT_ASSERT(strcmp(str.GetString(), strExpected) == 0);
}

void CTestObjString::testCopyStr()
{
    CSmallString str;
    const char * cStr = "123456789";
    str = cStr;

    CPPUNIT_ASSERT(strcmp(str.GetString(), cStr) == 0);
}

void CTestObjString::testSpecialChar()
{
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars("1234567890") == false);
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars("!@#$%^*()_+-*/={}[]|\\~`:;?") == false);
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars(">1234567890") == true);
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars("1234<567890") == true);
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars("1\'234567890") == true);
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars("12345\"67890") == true);
    CPPUNIT_ASSERT(CObjString::IsContainsSpecialChars("12345&67890") == true);
}



