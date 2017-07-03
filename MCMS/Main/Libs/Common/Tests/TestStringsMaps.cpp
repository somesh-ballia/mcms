// TestStringsMaps.cpp
#include <string.h>
#include "TestStringsMaps.h"
#include "StringsMaps.h"
#include "SharedDefines.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestStringsMaps );


void TestStringsMaps::setUp()
{
	CStringsMaps::Build();
}


void TestStringsMaps::tearDown()
{
	CStringsMaps::CleanUp();
}


void TestStringsMaps::testConstructor()
{
	const char *c = NULL;
	BYTE res = CStringsMaps::GetDescription(100,1000,&c);

	CPPUNIT_ASSERT(res == FALSE);
	CPPUNIT_ASSERT(c == NULL);

	CStringsMaps::AddItem(100,1000,"desc1");
	CStringsMaps::AddItem(100,1001,"desc2");
	CStringsMaps::AddItem(100,1003,"desc3");

	c = NULL;
	res = CStringsMaps::GetDescription(100,1000,&c);

	CPPUNIT_ASSERT(res != FALSE);
	CPPUNIT_ASSERT(strcmp(c,"desc1") == 0);
	
	int x = 0;
	res = CStringsMaps::GetValue(100,x,"desc2");
	CPPUNIT_ASSERT(res != FALSE);
	CPPUNIT_ASSERT_EQUAL(1001,x);
}



