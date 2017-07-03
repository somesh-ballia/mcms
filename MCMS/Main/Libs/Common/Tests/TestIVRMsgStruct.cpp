#include "TestIVRMsgStruct.h"
#include "IVRAvMsgStruct.h"
#include "NStream.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestIVRMsgStruct );


CTestIVRMsgStruct::CTestIVRMsgStruct()
{
}

CTestIVRMsgStruct::~CTestIVRMsgStruct()
{
}


void CTestIVRMsgStruct::setUp()
{
}

void CTestIVRMsgStruct::tearDown()
{
}

void CTestIVRMsgStruct::testSerializeDeSerialize()
{
	CAvMsgStruct IVRstruct1;
	
	IVRstruct1.SetAttendedWelcome(3);
	IVRstruct1.SetAvMsgServiceName("RNX IVR Service");

	COstrStream ostr;
    IVRstruct1.CdrSerialize(0, ostr);
    
    const string tmpStr = ostr.str();
    
    CIstrStream istr(ostr.str());
	CAvMsgStruct IVRstruct2;
	IVRstruct2.CdrDeSerialize(0, istr);

	bool res = (IVRstruct1 == IVRstruct2);
    CPPUNIT_ASSERT(res);
}

