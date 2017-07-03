// TestSegment.cpp

#include <iostream>
#include "TestSegment.h"
#include "Segment.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestSegment );


void TestSegment::setUp()
{
}


void TestSegment::tearDown()
{
}


void TestSegment::testConstructor()
{
	CSegment seg;
	seg << "Hello world";
	char tmp[100];
	seg >> tmp;
	CPPUNIT_ASSERT(strcmp(tmp,"Hello world") == 0);
	//CPPUNIT_ASSERT_EQUAL(1,1);
}

void TestSegment::test2Strings()
{
	CSegment seg;
	seg << "Hello world" << "123";
	char tmp[100];
	char tmp2[100];
	seg >> tmp >> tmp2;
	CPPUNIT_ASSERT(strcmp(tmp,"Hello world") == 0);
	CPPUNIT_ASSERT(strcmp(tmp2,"123") == 0);
	CPPUNIT_ASSERT_EQUAL(1,1);
}

void TestSegment::testStringNumStringNum()
{
	CSegment seg;
	seg << "Hello world" << (DWORD)123 << "123" << (DWORD) 456;
	char tmp[100];
	char tmp2[100];
	DWORD x,y;
	seg >> tmp >> x >> tmp2 >> y;
	CPPUNIT_ASSERT(strcmp(tmp,"Hello world") == 0);
	CPPUNIT_ASSERT(strcmp(tmp2,"123") == 0);
	CPPUNIT_ASSERT_EQUAL((DWORD)123,x);
	CPPUNIT_ASSERT_EQUAL((DWORD)456,y);
}

void TestSegment::testPutDynamicSize()
{
	CSegment seg;
	DWORD dwordVar = 42;
	char buffer [128] = {"Cucu-Lulu"};
	seg << dwordVar;
	seg.Put((BYTE*)buffer, strlen(buffer));
	
	DWORD dwordVarAfter = 0;
	char bufferAfter[128] = {'\0'}; 
	seg >> dwordVarAfter;
	DWORD strLen = seg.GetWrtOffset() - sizeof(dwordVarAfter);
	seg.Get((BYTE*)bufferAfter, strLen);
	bufferAfter[strLen] = '\0';
	
	bool dwordCmp = (dwordVar == dwordVarAfter);
	bool strCmp = (0 == strcmp(buffer, bufferAfter));
	
	CPPUNIT_ASSERT(dwordCmp);
	CPPUNIT_ASSERT(strCmp);
}





