// TestMediaIpConfigVector.cpp: implementation of the CTestMediaIpConfigVector class.
//
//////////////////////////////////////////////////////////////////////


#include "ProcessBase.h"
#include "TestMediaIpConfigVector.h"
#include "MediaIpConfigVector.h"
#include "Trace.h"
#include "SystemFunctions.h"



CPPUNIT_TEST_SUITE_REGISTRATION( CTestMediaIpConfigVector );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::tearDown()
{

}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMediaIpConfigVector::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testConstructor ",
		CProcessBase::GetProcess() != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testIsEmpty()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testIsEmpty ",
	                         0 == vec->Size() );

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testInsertFirst()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem = new CMediaIpConfig;
	vec->Insert(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertFirst ",
	                         1 == vec->Size() );

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testInsertAndRemoveByServiceId()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem = new CMediaIpConfig;
	firstItem->SetServiceId(11);
	vec->Insert(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByServiceId ",
	                         1 == vec->Size() );

	CMediaIpConfig *secondItem = new CMediaIpConfig;
	secondItem->SetServiceId(22);
	vec->Insert(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByServiceId ",
	                         2 == vec->Size() );

	vec->Remove(11);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByServiceId ",
	                         1 == vec->Size() );

	vec->Remove(22);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByServiceId ",
	                         0 == vec->Size() );

	POBJDELETE(secondItem);
	POBJDELETE(firstItem);
	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testInsertAndRemoveByObject()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem = new CMediaIpConfig;
	vec->Insert(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByObject ",
	                         1 == vec->Size() );

	CMediaIpConfig *secondItem = new CMediaIpConfig;
	vec->Insert(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByObject ",
	                         2 == vec->Size() );

	vec->Remove(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByObject ",
	                         1 == vec->Size() );

	vec->Remove(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testInsertAndDeleteByObject ",
	                         0 == vec->Size() );

	POBJDELETE(secondItem);
	POBJDELETE(firstItem);
	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testFind()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->Find(firstItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testFind ",
	                         found && 11 == found->GetServiceId() );
	                         
	found = vec->Find(secondItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testFind ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->Find(thirdItem);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testFind ",
	                         found && 33 == found->GetServiceId() );


	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testAt()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->At(1);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testAt ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->At(2);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testAt ",
	                         found && 33 == found->GetServiceId() );
	                         
	found = vec->At(0);
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testAt ",
	                         found && 11 == found->GetServiceId() );


	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testGetFirst()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetFirst ",
	                         found && 11 == found->GetServiceId() );

	                         
	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testGetNext()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNext ",
	                         found && 11 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNext ",
	                         found && 22 == found->GetServiceId() );
	                         

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testGetNextUntilEnd()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextUntilEnd ",
	                         found && 11 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextUntilEnd ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextUntilEnd ",
	                         found && 33 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextUntilEnd ",
	                         NULL == found );


	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testGetNextOverflow()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextOverflow ",
	                         found && 11 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextOverflow ",
	                         found && 22 == found->GetServiceId() );
	                         
	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextOverflow ",
	                         found && 33 == found->GetServiceId() );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextOverflow ",
	                         NULL == found );

	found = vec->GetNext();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextOverflow ",
	                         NULL == found );

	POBJDELETE(vec);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaIpConfigVector::testGetNextWithLoop()
{
	CMediaIpConfigVector *vec = new CMediaIpConfigVector;
	
	CMediaIpConfig *firstItem  = new CMediaIpConfig;
	CMediaIpConfig *secondItem = new CMediaIpConfig;
	CMediaIpConfig *thirdItem  = new CMediaIpConfig;

	firstItem->SetServiceId(11);
	secondItem->SetServiceId(22);
	thirdItem->SetServiceId(33);

	vec->Insert(firstItem);
	vec->Insert(secondItem);
	vec->Insert(thirdItem);
	                         
	                         
	CMediaIpConfig *found = vec->GetFirst();
	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextWithLoop ",
	                         found && 11 == found->GetServiceId() );

	while (NULL != found)
	{
		found = vec->GetNext();
	}

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaIpConfigVector::testGetNextOverflow ",
	                         NULL == found );


	POBJDELETE(vec);
}


