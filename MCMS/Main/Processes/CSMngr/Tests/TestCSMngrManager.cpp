#include "TestCSMngrManager.h"
#include "CSMngrManager.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "Request.h"
#include "CIPServiceDel.h"
#include "CSMngrProcess.h"
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION( CTestCSMngrManager );



static CCSMngrManager	*pCSMngrManager = NULL;
static CIPService       *pIpService 	= NULL;
static CIPServiceList   *pServiceList	= NULL;

CTestCSMngrManager::CTestCSMngrManager()
{
	
}

CTestCSMngrManager::~CTestCSMngrManager()
{

}


//////////////////////////////////////////////////////////////////////
void CTestCSMngrManager::setUp()
{
	pCSMngrManager 	= new CCSMngrManager;
	pServiceList 	= ((CCSMngrProcess*)CProcessBase::GetProcess())->GetIpServiceListDynamic();
}

//////////////////////////////////////////////////////////////////////
void CTestCSMngrManager::tearDown()
{
	POBJDELETE(pCSMngrManager);	
}

//////////////////////////////////////////////////////////////////////
void CTestCSMngrManager::testConstructor()
{
	FPTRACE(eLevelInfoNormal,"CTestCSMngrManager::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testConstructor ",
		pCSMngrManager != NULL );
}


void CTestCSMngrManager::testAddIpService()
{
	FPTRACE(eLevelInfoNormal,"CTestCSMngrManager::testAddIpService");
 
 	pIpService = new CIPService;
	CRequest request;
	request.SetRequestObject(pIpService);
	
	DWORD oldNumOfServices = pServiceList->GetServiceNumber();
	
	STATUS status = pCSMngrManager->AddIpService(&request);
	
	WORD newNumOfServices = pServiceList->GetServiceNumber();
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testAddIpService ",
		newNumOfServices == oldNumOfServices + 1);
	
	CIPService *service = pServiceList->GetService(pIpService->GetId());
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testAddIpService ",
		NULL != service);
	
	request.SetRequestObject(NULL);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testAddIpService ",
		status == STATUS_OK);
}


void CTestCSMngrManager::testUpdateIpService()
{
	FPTRACE(eLevelInfoNormal,"CTestCSMngrManager::testUpdateIpService");
	
	DWORD newIpAddrr = SystemIpStringToDWORD("1.2.3.4");
	pIpService->SetNetIPaddress(newIpAddrr);
	
	CRequest request;
	request.SetRequestObject(pIpService);
	
	DWORD oldNumOfServices = pServiceList->GetServiceNumber();
	
	STATUS status = pCSMngrManager->UpdateIpService(&request);
	
	WORD newNumOfServices = pServiceList->GetServiceNumber();
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testAddIpService ",
		newNumOfServices == oldNumOfServices);
	
	request.SetRequestObject(NULL);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testUpdateIpService ",
		status == STATUS_OK);
}


void CTestCSMngrManager::testDeleteIpService()
{
	FPTRACE(eLevelInfoNormal,"CTestCSMngrManager::testDeleteIpService");

	CIPServiceDel del;
	del.SetIPServiceName(pIpService->GetName());
	CRequest request;
	request.SetRequestObject(&del);
	
	DWORD oldNumOfServices = pServiceList->GetServiceNumber();
	
	STATUS status = pCSMngrManager->DeleteIpService(&request);
	
	WORD newNumOfServices = pServiceList->GetServiceNumber();
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testAddIpService ",
		newNumOfServices == oldNumOfServices - 1);
		
	request.SetRequestObject(NULL);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrManager::testDeleteIpService ",
		status == STATUS_OK);
}


















