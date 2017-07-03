#include "TestFaultContainer.h"
#include "FaultsContainer.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestFaultContainer );


CTestFaultContainer::CTestFaultContainer()
{
}

CTestFaultContainer::~CTestFaultContainer()
{
}


void CTestFaultContainer::testSerializeDeSerialize()
{
	CFaultList *list = new CFaultList;
	CTaskStateFaultList taskStateFault1(	"Cucu-Lulu", 
											eTaskMinor, 
											eTaskStateStartup, 
											list);
	CSegment seg;
	taskStateFault1.Serialize(seg);																
	
	CTaskStateFaultList taskStateFault2;
	taskStateFault2.DeSerialize(seg);
		
	CPPUNIT_ASSERT(taskStateFault1 == taskStateFault2);
}


