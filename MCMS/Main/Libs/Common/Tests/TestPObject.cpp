// TestPObject.cpp

#include "TestPObject.h"
#include "PObject.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestPObject );


class CMyObject : public CPObject
{
CLASS_TYPE_1(CMyObject,CPObject )
public:
	virtual const char* NameOf() const {return "CMyObject";}
};

void TestPObject::setUp()
{
}


void TestPObject::tearDown()
{
}


void TestPObject::testConstructor()
{
	CMyObject A;
	std::string name1 = A.NameOf();
	std::string name2 = "CMyObject";
	CPPUNIT_ASSERT_EQUAL( name2,name1);
	CPPUNIT_ASSERT( A.m_type != NULL);
}

void TestPObject::testNewDelete()
{
	CMyObject *obj = new CMyObject;
	POBJDELETE(obj);
	CPPUNIT_ASSERT_EQUAL((int)obj,0);
}

void TestPObject::testValidFlag()
{
	CMyObject A;
	CPPUNIT_ASSERT( A.m_validFlag == OBJ_STAMP );
}
