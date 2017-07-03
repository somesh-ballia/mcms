// TestStructTm.cpp

#include "TestStructTm.h"
#include "StructTm.h"
#include "NStream.h"
#include "OperEvent.h"
#include "SystemFunctions.h"




// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestStructTm );


void TestStructTm::setUp()
{
}


void TestStructTm::tearDown()
{
}


void TestStructTm::testConstructor()
{
	CStructTm st;
	CPPUNIT_ASSERT_EQUAL( 0 , st.m_sec);
	CPPUNIT_ASSERT_EQUAL( 0 , st.m_min);
	CPPUNIT_ASSERT_EQUAL( 0 , st.m_hour);
	CPPUNIT_ASSERT_EQUAL( 1 , st.m_day);
	CPPUNIT_ASSERT_EQUAL( 0 , st.m_mon);
	CPPUNIT_ASSERT_EQUAL( 100 , st.m_year);

	COstrStream ostr;
	st.Serialize(ostr);
	CPPUNIT_ASSERT_EQUAL( std::string("1.0.100	0:0:0\n") , ostr.str());
}

void TestStructTm::testSerializeSnmp()
{
    CStructTm stSNMP(31,11,2001,12,30,5); // 31 dec 2001 12:30:05;
    COstrStream ostrSnmp;
    stSNMP.SerializeSNMP(ostrSnmp);

    const char *strTime = ostrSnmp.str().c_str();
    const char *strExpectedTime = "2001-11-31,12:30:5.0,0:0";
    int res = strcmp(strTime, strExpectedTime);

    CPPUNIT_ASSERT(res == 0);
}


void TestStructTm::testConstructor2()
{
	CStructTm st(31,11,101,12,30,0); // 31 dec 2001 12:30:00
	CPPUNIT_ASSERT_EQUAL( 0 , st.m_sec);
	CPPUNIT_ASSERT_EQUAL( 30 , st.m_min);
	CPPUNIT_ASSERT_EQUAL( 12 , st.m_hour);
	CPPUNIT_ASSERT_EQUAL( 31 , st.m_day);
	CPPUNIT_ASSERT_EQUAL( 11 , st.m_mon);
	CPPUNIT_ASSERT_EQUAL( 101 , st.m_year);

	COstrStream ostr;
	st.Serialize(ostr);
	CPPUNIT_ASSERT_EQUAL( std::string("31.11.101	12:30:0\n") , ostr.str());

}

void TestStructTm::testDeSerialze()
{
	CIstrStream istr("31.11.101   12:30:0\n");
	CStructTm st1(31,11,101,12,30,0); // 31 dec 2001 12:30:00
	CStructTm st2; // 31 dec 2001 12:30:00
	CStructTm st3;
	st3.DeSerialize(istr);

	CPPUNIT_ASSERT(st1 != st2);
	CPPUNIT_ASSERT(st1 == st3);
	CPPUNIT_ASSERT(st2 != st3);
}


void TestStructTm::testOrder()
{
	CStructTm st1(31,0,101,12,30,0);
	CStructTm st2(31,11,101,12,30,0);
	CStructTm st3(31,0,102,12,30,0);
	CStructTm st4(31,11,102,12,30,0);

	CPPUNIT_ASSERT(st1 < st2);
	CPPUNIT_ASSERT(st2 < st3);
	CPPUNIT_ASSERT(st3 < st4);

	CPPUNIT_ASSERT(st4 > st3);
	CPPUNIT_ASSERT(st3 > st2);
	CPPUNIT_ASSERT(st2 > st1);

	CPPUNIT_ASSERT(st1 <= st1);
	CPPUNIT_ASSERT(st1 <= st2);

	CPPUNIT_ASSERT(st2 >= st1);
	CPPUNIT_ASSERT(st2 >= st2);

	CPPUNIT_ASSERT(!(CStructTm::IsPeriodOverLapping(st1,st2,st3,st4)));
	CPPUNIT_ASSERT(!(CStructTm::IsPeriodOverLapping(st3,st4,st1,st2)));
	CPPUNIT_ASSERT((CStructTm::IsPeriodOverLapping(st1,st3,st2,st4)));
	CPPUNIT_ASSERT((CStructTm::IsPeriodOverLapping(st2,st4,st1,st3)));


}

void TestStructTm::testSerializeDeSerialze()
{
	CStructTm st1(31,11,101,12,30,0); // 31 dec 2001 12:30:00
	CStructTm st2(30,10,100,11,29,0); //

	COstrStream ostr;
	st1.Serialize(ostr);
	st2.Serialize(ostr);

	CStructTm st3;
	CStructTm st4;

	CIstrStream istr(ostr.str());
	st3.DeSerialize(istr);
	st4.DeSerialize(istr);

	CPPUNIT_ASSERT(st1 == st3 && st2 == st4);
}

void TestStructTm::testSerializeDeSerialzeCdr()
{
	CStructTm st1(12,30,0); // 31 dec 2001 12:30:00
	CStructTm st2(11,29,0); //

	COstrStream ostr;
	st1.SerializeCdr(ostr);
	st2.SerializeCdr(ostr);

	CStructTm st3;
	CStructTm st4;

	CIstrStream istr(ostr.str());
	st3.DeSerializeCdr(istr);
	st4.DeSerializeCdr(istr);

	CPPUNIT_ASSERT(st1 == st3 && st2 == st4);
}

void TestStructTm::testSerializeDeSerialzeBilling()
{
	CStructTm st1(31,11,2001,12,30,0); // 31 dec 2001 12:30:00
	CStructTm st2(30,10,2000,11,29,0); //

	COstrStream ostr;
	st1.SerializeBilling(ostr);
	st2.SerializeBilling(ostr);

	CStructTm st3;
	CStructTm st4;

	CIstrStream istr(ostr.str());
	st3.DeSerializeBilling(istr);
	st4.DeSerializeBilling(istr);

	CPPUNIT_ASSERT(st1 == st3 && st2 == st4);
}

void TestStructTm::testSerializeDeSerialzeOperAddParty()
{
	CStructTm 		st1(12,30,0); // 31 dec 2001 12:30:00
	COperAddParty 	rsrvStart1;

	rsrvStart1.SetPartyName("Name");
	rsrvStart1.SetPartyId(42);
	rsrvStart1.SetConnectionType(1);
//			rsrvStart1.SetBondingMode(m_pParty[i]->GetBondingMode1());
	rsrvStart1.SetNetNumberChannel(1);
//			rsrvStart1.SetNetChannelWidth(m_pParty[i]->GetNetChannelWidth());
	rsrvStart1.SetNetServiceName("Net Name");
//			rsrvStart1.SetRestrict(m_pParty[i]->GetRestrict());
	rsrvStart1.SetVoice(0);
//			rsrvStart1.SetNumType(m_pParty[i]->GetNumType());
	rsrvStart1.SetNetSubServiceName("Net Sub Name");
	rsrvStart1.SetIdentMethod(0);
	rsrvStart1.SetMeetMeMethod(0);

	COstrStream ostr;
	st1.SerializeCdr(ostr);
	rsrvStart1.Serialize(NATIVE, ostr, 0);

	CStructTm 		st2;
	COperAddParty 	rsrvStart2;

	CIstrStream istr(ostr.str());
	st2.DeSerializeCdr(istr);
	rsrvStart2.DeSerialize(NATIVE, istr, 0);

	bool res1 = (st1 == st2);
	bool res2 = (rsrvStart1 == rsrvStart2);
	CPPUNIT_ASSERT(res1 && res2);
}

void TestStructTm::testTimeDelta()
{
    CStructTm st1(12, 30, 30); // 31 dec 2001 12:30:30
    CStructTm st2(19, 0, 0);  // 31 dec 2001 19:00:00

    CStructTm expected1(0, 0, 0, 6, 29, 30);
    CStructTm dif1 = st2.GetTimeDelta(st1);

    CPPUNIT_ASSERT(expected1 == dif1);


    CStructTm st3(20, 0, 0); // 31 dec 2001 12:30:30
    CStructTm st4(23, 0, 0);  // 31 dec 2001 19:00:00

    CStructTm expected2(0, 0, 0, 3, 0, 0);
    CStructTm dif2 = st4.GetTimeDelta(st3);

    CPPUNIT_ASSERT(expected2 == dif2);

    CStructTm now;
    SystemGetTime(now);

    CStructTm midNight(now);
    midNight.m_hour = 24;
    midNight.m_min = 0;
    midNight.m_sec = 0;

    CStructTm dif3 = midNight.GetTimeDelta(now);

    bool res = expected2 == dif3;


    // does not work
//     DWORD absTime = st2 - st1;
//     CStructTm dif2;
//     dif2.SetAbsTime(absTime);


//    CPPUNIT_ASSERT(expected == dif2);
}

//Tests time of format:
//Year: regular, calenderial
//Months: 1 - 12  (12 == December)
void TestStructTm::testTimeAdd1()
{
    char test[200];

    CStructTm st1(31,12,2001,12,30,0); // 31 dec 2001 12:30:00
	CStructTm st2(0,0,0,100,0,0);      // 100 hours
    CStructTm st3 = st1 + st2;
    CStructTm expected1(4, 1, 2002 , 16, 30, 0);
    bool res1 = (expected1 == st3);
    st3.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res1);

    CStructTm st11(25,12,2004,18,30,0); // 25 dec 2004 18:30:00
	CStructTm st12(0,0,0,2030,0,0);     // 2030 = 84 * 24 + 14 hours
    CStructTm st13 = st11 + st12;
    CStructTm expected11(20, 3, 2005 , 8, 30, 0);
    bool res2 = (expected11 == st13);
    st13.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res2);

    CStructTm st21(9,5,2005,8,30,0);   // 9 may 2008 8:30:00
	CStructTm st22(0,0,0,0,0,1048576);  // 1048576 sec = 12 days 3 hours 16 minutes 16 seconds
    CStructTm st23 = st21 + st22;
    CStructTm expected21(21, 5, 2005 , 11, 46, 16);
    bool res3 = (expected21 == st23);
    st23.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res3);

    /*CStructTm st31(9,04,2005,8,30,0);      // 9 may 2008 8:30:00
	CStructTm st32(0,0,0,0,0,0x7fffffff);  // max positive int
    CStructTm st33 = st31 + st32;
    CStructTm expected31(13, 5, 2073 , 11, 44, 7);
    bool res4 = (expected31 == st33);
    st33.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res4);*/
}

//Tests time of format:
//Year: from 1900 (101 == 2001)
//Months: 0 - 11  (11 == December)
void TestStructTm::testTimeAdd2()
{
    char test[200];

    CStructTm st1(31,11,101,12,30,0); // 31 dec 2001 12:30:00
	CStructTm st2(0,0,0,100,0,0);      // 100 hours
    CStructTm st3 = st1 + st2;
    CStructTm expected1(4, 0, 102 , 16, 30, 0);
    bool res1 = (expected1 == st3);
    st3.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res1);

    CStructTm st11(25,11,104,18,30,0); // 25 dec 2004 18:30:00
	CStructTm st12(0,0,0,2030,0,0);     // 2030 = 84 * 24 + 14 hours
    CStructTm st13 = st11 + st12;
    CStructTm expected11(20, 2, 105 , 8, 30, 0);
    bool res2 = (expected11 == st13);
    st13.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res2);

    CStructTm st21(9,4,105,8,30,0);   // 9 may 2008 8:30:00
	CStructTm st22(0,0,0,0,0,1048576);  // 1048576 sec = 12 days 3 hours 16 minutes 16 seconds
    CStructTm st23 = st21 + st22;
    CStructTm expected21(21, 4, 105 , 11, 46, 16);
    bool res3 = (expected21 == st23);
    st23.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res3);

    /*CStructTm st31(9,04,105,8,30,0);      // 9 may 2008 8:30:00
	CStructTm st32(0,0,0,0,0,0x7fffffff);  // max positive int
    CStructTm st33 = st31 + st32;
    CStructTm expected31(13, 4, 173 , 11, 44, 7);
    bool res4 = (expected31 == st33);
    st33.DumpToBuffer(test);
    CPPUNIT_ASSERT_MESSAGE(test, res4);*/
}
