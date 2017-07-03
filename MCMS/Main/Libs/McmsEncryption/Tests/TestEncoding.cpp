#include "TestEncoding.h"


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestEncoding );



 


void TestEncoding::testUtf8StrSlicing()
{
    char utf8str [1024];
    int index = 0;
    char outBuffer[1024];
    DWORD limit = 0;
    char expected[1024];
    
// abDADADADA
    utf8str[index++] = 'a';
    utf8str[index++] = 'b';

    utf8str[index++] = 0xD0;
    utf8str[index++] = 0x94;
    utf8str[index++] = 0xD0;
    utf8str[index++] = 0x90;

    utf8str[index++] = 0xD0;
    utf8str[index++] = 0x94;
    utf8str[index++] = 0xD0;
    utf8str[index++] = 0x90;

    utf8str[index++] = '\0';

    // ab | DADADADA
    limit = 2;
    strcpy(expected, utf8str);
    expected[2] = '\0';
    bool res = CheckSlice(utf8str, outBuffer, limit, expected);

    
    // ab | DADADADA
    limit = 3;
    strcpy(expected, utf8str);
    expected[2] = '\0';
    res = CheckSlice(utf8str, outBuffer, limit, expected);
    

    // abD | ADADADA
    limit = 4;
    strcpy(expected, utf8str);
    expected[4] = '\0';
    res = CheckSlice(utf8str, outBuffer, limit, expected);

    
    // outBuffer > source
    limit = 20;
    strcpy(expected, utf8str);
    res = CheckSlice(utf8str, outBuffer, limit, expected);
    

    // another encodings
    index = 0;
    
    // ab[...][...][...][...]
    utf8str[index++] = 'a';
    utf8str[index++] = 'b';

    utf8str[index++] = 0xE8;
    utf8str[index++] = 0x94;
    utf8str[index++] = 0x90;

    utf8str[index++] = 0xE8;
    utf8str[index++] = 0x94;
    utf8str[index++] = 0x90;

    utf8str[index++] = 0xE8;
    utf8str[index++] = 0x94;
    utf8str[index++] = 0x90;

    utf8str[index++] = 0xE8;
    utf8str[index++] = 0x94;
    utf8str[index++] = 0x90;
    
    utf8str[index++] = '\0';

    
    // ab[...][...][...][...]
    limit = 3;
    strcpy(expected, utf8str);
    expected[2] = '\0';
    res = CheckSlice(utf8str, outBuffer, limit, expected);

    
    // ab[...][...][...][...]
    limit = 4;
    strcpy(expected, utf8str);
    expected[2] = '\0';
    res = CheckSlice(utf8str, outBuffer, limit, expected);

    
    // ab[...][...][...][...]
    limit = 5;
    strcpy(expected, utf8str);
    expected[5] = '\0';
    res = CheckSlice(utf8str, outBuffer, limit, expected);

    
    // ab[...][...][...][...]
    limit = 6;
    strcpy(expected, utf8str);
    expected[5] = '\0';
    res = CheckSlice(utf8str, outBuffer, limit, expected);
}

bool TestEncoding::CheckSlice(const char *utf8str, char *outBuffer, DWORD limit, char *expected)
{
    CEncodingConvertor::CutUtf8String(utf8str, outBuffer, limit);

    int resCmp = strcmp(expected, outBuffer);

    CPPUNIT_ASSERT(0 == resCmp);

    return 0 == resCmp;
}
