
#ifndef TEST_FILE_NAME_PARSER_H_
#define TEST_FILE_NAME_PARSER_H_



#include <cppunit/extensions/HelperMacros.h>
#include "DataTypes.h"


class TestFileNameParser : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestFileNameParser );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void testParser();

private:
    
    
};

#endif 
