#include "TestCdrShortDrv.h"

#include "Native.h"
#include "NStream.h"
#include "Segment.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestCdrShortDrv );

void TestCdrShortDrv::setUp()
{}

void TestCdrShortDrv::tearDown()
{}

void TestCdrShortDrv::testConstructor()
{}

void TestCdrShortDrv::testSerializeDeserialize()
{
    CCdrShortDrv obj1;

    obj1.SetFileVersion(1);
    obj1.SetH243ConfName("Cucu-Lulu");

    // Set the same name
    obj1.SetDisplayName(obj1.GetH243ConfName());

    obj1.SetConfId(42);

    CStructTm rsrvStart(1,2,2003, 4, 5, 6);
    obj1.SetRsrvStrtTime(rsrvStart);
    
    CStructTm rsrvDuration(110, 6, 7);
    obj1.SetRsrvDuration(rsrvDuration);

    CStructTm actualStart(20,3,2007, 3, 4, 8);
    obj1.SetActualStrtTime(actualStart);

    CStructTm actualDuration(1, 2, 3);
    obj1.SetActualDuration(actualDuration);

    obj1.SetStatus(ONGOING_CONFERENCE);
    obj1.SetFileName("name");
    obj1.SetOffset(42);
    obj1.SetGMTOffsetSign(1);
    obj1.SetGMTOffset(1);
    obj1.SetFileMarked(1);

    CSegment seg;
    obj1.Serialize(NATIVE,seg);

    CCdrShortDrv obj2;
    obj2.DeSerialize(NATIVE,seg);

    // Offset value is not part of Serialize/Deserialize. Sets the value manually.
    obj2.SetOffset(obj1.GetOffset());

    CPPUNIT_ASSERT(obj1 == obj2);
}



