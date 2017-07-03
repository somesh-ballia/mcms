// TestMediaIpParamsVector.cpp: implementation of the CTestMediaIpParamsVector class.
//
//////////////////////////////////////////////////////////////////////


#include "ProcessBase.h"
#include "TestMediaIpParamsVector.h"
#include "MediaIpParamsVector.h"
#include "Trace.h"
#include "SystemFunctions.h"



CPPUNIT_TEST_SUITE_REGISTRATION( CTestMediaIpParamsVector );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::tearDown()
{

}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMediaIpParamsVector::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testConstructor ",
		CProcessBase::GetProcess() != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testIsEmpty()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testIsEmpty ",
	                         0 == vec->Size() );

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testInsertFirst()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem = new CMediaIpParameters;
	vec->Insert(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertFirst ",
	                         1 == vec->Size() );

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testInsertAndRemoveByServiceId()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem = new CMediaIpParameters;
	firstItem->SetServiceId(11);
	vec->Insert(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByServiceId ",
	                         1 == vec->Size() );

	CMediaIpParameters *secondItem = new CMediaIpParameters;
	secondItem->SetServiceId(22);
	vec->Insert(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByServiceId ",
	                         2 == vec->Size() );

	vec->Remove(11);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByServiceId ",
	                         1 == vec->Size() );

	vec->Remove(22);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByServiceId ",
	                         0 == vec->Size() );

	POBJDELETE(secondItem);
	POBJDELETE(firstItem);
	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testInsertAndRemoveByObject()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem = new CMediaIpParameters;
	vec->Insert(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByObject ",
	                         1 == vec->Size() );

	CMediaIpParameters *secondItem = new CMediaIpParameters;
	vec->Insert(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByObject ",
	                         2 == vec->Size() );

	firstItem = vec->Remove(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByObject ",
	                         1 == vec->Size() );

	secondItem = vec->Remove(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testInsertAndDeleteByObject ",
	                         0 == vec->Size() );

	POBJDELETE(secondItem);
	POBJDELETE(firstItem);
	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testFind()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->Find(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testFind ",
	                         found && 11 == found->GetServiceId() );
	                         
	found = vec->Find(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testFind ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->Find(thirdItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testFind ",
	                         found && 33 == found->GetServiceId() );


	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testAt()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->At(1);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testAt ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->At(2);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testAt ",
	                         found && 33 == found->GetServiceId() );
	                         
	found = vec->At(0);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testAt ",
	                         found && 11 == found->GetServiceId() );


	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testGetFirst()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetFirst ",
	                         found && 11 == found->GetServiceId() );

	                         
	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testGetNext()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNext ",
	                         found && 11 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNext ",
	                         found && 22 == found->GetServiceId() );
	                         

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testGetNextUntilEnd()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextUntilEnd ",
	                         found && 11 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextUntilEnd ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextUntilEnd ",
	                         found && 33 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextUntilEnd ",
	                         NULL == found );


	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testGetNextOverflow()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextOverflow ",
	                         found && 11 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextOverflow ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextOverflow ",
	                         found && 33 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextOverflow ",
	                         NULL == found );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextOverflow ",
	                         NULL == found );

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpParamsVector::testGetNextWithLoop()
{
	CMediaIpParamsVector *vec = new CMediaIpParamsVector;
	
	CMediaIpParameters *firstItem  = new CMediaIpParameters;
	CMediaIpParameters *secondItem = new CMediaIpParameters;
	CMediaIpParameters *thirdItem  = new CMediaIpParameters;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpParameters *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextWithLoop ",
	                         found && 11 == found->GetServiceId() );

	while (NULL != found)
	{
		found = vec->GetNext();
	}

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpParamsVector::testGetNextOverflow ",
	                         NULL == found );


	POBJDELETE(vec);
}


