#include "TestH221Str.h"
#include "H221Str.h"
#include "NStream.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestH221Str );


TestH221Str::TestH221Str()
{
}

TestH221Str::~TestH221Str()
{
}

void TestH221Str::setUp()
{
}

void TestH221Str::tearDown()
{
}

void TestH221Str::testSerializeDeSerialize()
{
	CH221Str h221_1;
	const char *str = "123";
	h221_1.SetH221FromString(strlen(str), str);
	
	COstrStream ostr;
    h221_1.Serialize(0, ostr);
    
    CIstrStream istr(ostr.str().c_str());
    
    CH221Str h221_2;
    h221_2.DeSerialize(0, istr);
    
    CPPUNIT_ASSERT(h221_1 == h221_2);
}

