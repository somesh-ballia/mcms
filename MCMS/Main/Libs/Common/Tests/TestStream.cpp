// TestStream.cpp

#include "TestStream.h"
#include "NStream.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestStream );


void TestStream::setUp()
{
}


void TestStream::tearDown()
{
}


void TestStream::testConstructor()
{
	COstrStream stream;
	stream << "Hello World";
	std::string hello = "Hello World";
	CPPUNIT_ASSERT_EQUAL( hello , stream.str());
}

void TestStream::testConstructorString()
{
//	char * temp = new char[1000];
//	COstrStream stream(temp);
//	stream << "Hello World";
//	stream.flush();
//	std::string hello = "Hello World";
//	std::string hello2 = stream.str();
//	CPPUNIT_ASSERT_EQUAL( hello , hello2);
//	delete [] temp;
}

void TestStream::testRead()
{
	CIstrStream stream("Hello World");
	std::string hello,world;
	stream >> hello;
	stream >> world;
	CPPUNIT_ASSERT_EQUAL(std::string("Hello") , hello);
	CPPUNIT_ASSERT_EQUAL(std::string("World") , world);
}

void TestStream::testReadFromStdString()
{
	std::string hello_world = "hello world";
	CIstrStream stream(hello_world);
	std::string hello,world;
	stream >> hello;
	stream >> world;
	CPPUNIT_ASSERT_EQUAL(std::string("hello") , hello);
	CPPUNIT_ASSERT_EQUAL(std::string("world") , world);
}




