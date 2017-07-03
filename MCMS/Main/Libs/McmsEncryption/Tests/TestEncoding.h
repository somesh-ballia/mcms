
#ifndef TESTENCODING_H
#define TESTENCODING_H



#include <cppunit/extensions/HelperMacros.h>
#include "DataTypes.h"
#include "EncodingConvertor.h"


class TestEncoding : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestEncoding );

	CPPUNIT_TEST( testUtf8StrSlicing );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void testUtf8StrSlicing();

private:
    bool CheckSlice(const char *utf8str, char *outBuffer, DWORD limit, char *expected);
    
    
};

#endif 
