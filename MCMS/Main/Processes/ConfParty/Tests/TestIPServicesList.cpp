// TestIpServices.cpp: implementation of the CTestIpServicesList class.
//
//////////////////////////////////////////////////////////////////////

#include "TestIPServicesList.h"
#include "ConfStructs.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "cppunit/extensions/HelperMacros.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestIpServicesList);

 
//////////////////////////////////////////////////////////////////////
void CTestIpServicesList::setUp()
{
	m_pIpServiceListManager = new 	CIpServiceListManager;
}
//////////////////////////////////////////////////////////////////////
void CTestIpServicesList::tearDown()
{
	POBJDELETE(m_pIpServiceListManager);
	
}
//////////////////////////////////////////////////////////////////////
void CTestIpServicesList::CheckEmptyListOfIpServices()
{   
	WORD numberOfServices = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::CheckEmptyListOfIpServices", numberOfServices == 0);
}
//////////////////////////////////////////////////////////////////////
void CTestIpServicesList::InsertAndDeleteOneServiceToList()
{    
	CConfIpParameters* pConfIpParameters = new CConfIpParameters;
	CONF_IP_PARAMS_S* pConfIpParams = new CONF_IP_PARAMS_S;
	
	
	// Initialize parameters
	pConfIpParams->service_id = 6;
	char serviceName[20] = "Kuku Ruku";
	char aliasName[20] = "Poly Gavri";
	strncpy(pConfIpParams->service_name,serviceName, strlen(serviceName));
	//pConfIpParams->gatekeeper_Mode = 2;
	//pConfIpParams->alias_type = 7;
	//strncpy(pConfIpParams->alias_name,aliasName, strlen(aliasName));
	pConfIpParameters->SetData(pConfIpParams);
	
	
	/////////////////////////////////////////
	// Insert Item to List 
	/////////////////////////////////////////
	m_pIpServiceListManager->insertIpService(pConfIpParameters);
    WORD numberOfServicesInList = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteOneServiceToList", numberOfServicesInList == 1);
	CConfIpParameters* pConfIpParamsFromList = m_pIpServiceListManager->FindIpService(pConfIpParams->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteOneServiceToList", pConfIpParamsFromList != NULL);
    /////////////////////////////////////////
	// delete Item from List 
	/////////////////////////////////////////
    CConfIpParameters* pConfDeletedIpParamsFromList = m_pIpServiceListManager->removeIpService(pConfIpParams->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteOneServiceToList", pConfDeletedIpParamsFromList != NULL);
    
    
//    /////////////////////////////////////////
//	// clean up
//	/////////////////////////////////////////
//    POBJDELETE(pConfIpParameters);
	PDELETE(pConfIpParams);
}
//////////////////////////////////////////////////////////////////////
void CTestIpServicesList::InsertAndDeleteThreeServicesToList()
{  
//	CIpServiceListManager* pIpServiceListManager = new 	CIpServiceListManager;
	// Initialize 3 services
	CConfIpParameters* pConfIpService1 = new CConfIpParameters;
	CConfIpParameters* pConfIpService2 = new CConfIpParameters;
	CConfIpParameters* pConfIpService3 = new CConfIpParameters;
	// Initialize 3 structures of CONF_IP_PARAMS_S
	CONF_IP_PARAMS_S* pConfIpParam1 = new CONF_IP_PARAMS_S;
	CONF_IP_PARAMS_S* pConfIpParam2 = new CONF_IP_PARAMS_S;
	CONF_IP_PARAMS_S* pConfIpParam3 = new CONF_IP_PARAMS_S;


	// Initialize parameters

	// Initialize Service Id 
	pConfIpParam1->service_id = 6;
	pConfIpParam2->service_id = 3;
	pConfIpParam3->service_id = 5;

		// Initialize Service Name 
	char serviceName1[30] = "Kuku Ruku";
	char serviceName2[30] = "Roza";
	char serviceName3[30] = "Sara";
	strncpy(pConfIpParam1->service_name,serviceName1,NET_SERVICE_PROVIDER_NAME_LEN - 1);
	pConfIpParam1->service_name[NET_SERVICE_PROVIDER_NAME_LEN - 1] = '\0';
	strncpy(pConfIpParam2->service_name,serviceName2,NET_SERVICE_PROVIDER_NAME_LEN - 1);
	pConfIpParam2->service_name[NET_SERVICE_PROVIDER_NAME_LEN - 1] = '\0';
	strncpy(pConfIpParam3->service_name,serviceName3,NET_SERVICE_PROVIDER_NAME_LEN - 1);
	pConfIpParam3->service_name[NET_SERVICE_PROVIDER_NAME_LEN - 1] = '\0';

    // Initialize Alias  Name 
	char aliasName1[30] = "Poly Gavri";
	char aliasName2[30] = "Shuki Zikri";
	char aliasName3[30] = "Chezi";
	
	//strncpy(pConfIpParam1->alias_name,aliasName1,strlen(aliasName1));
	//strncpy(pConfIpParam2->alias_name,aliasName2,strlen(aliasName2));
	//strncpy(pConfIpParam3->alias_name,aliasName3,strlen(aliasName3));
	// Initialize GK mode

	//pConfIpParam1->gatekeeper_Mode = 2;
	//pConfIpParam2->gatekeeper_Mode = 3;
	//pConfIpParam3->gatekeeper_Mode = 4;
	// Initialize Alias  Type 
	//pConfIpParam1->alias_type = 5;
	//pConfIpParam2->alias_type = 7;
	//pConfIpParam3->alias_type = 8;
	// Set Data
	pConfIpService1->SetData(pConfIpParam1);
	pConfIpService2->SetData(pConfIpParam2);
	pConfIpService3->SetData(pConfIpParam3);
	/////////////////////////////////////////
	// Insert Items to List 
	/////////////////////////////////////////
	m_pIpServiceListManager->insertIpService(pConfIpService1);
	m_pIpServiceListManager->insertIpService(pConfIpService2);
	m_pIpServiceListManager->insertIpService(pConfIpService3);

    WORD numberOfServicesInList = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", numberOfServicesInList == 3);
	CConfIpParameters* pConfIpParamsFromList1 = m_pIpServiceListManager->FindIpService(pConfIpParam1->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", pConfIpParamsFromList1 != NULL);
	CConfIpParameters* pConfIpParamsFromList2 = m_pIpServiceListManager->FindIpService(pConfIpParam2->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", pConfIpParamsFromList2 != NULL);
	CConfIpParameters* pConfIpParamsFromList3 = m_pIpServiceListManager->FindIpService(pConfIpParam3->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", pConfIpParamsFromList3 != NULL);
    /////////////////////////////////////////
	// delete Items from List 
	/////////////////////////////////////////
	// delete 1st item
    /////////////////////////////////////////
    CConfIpParameters* pConfDeletedIpParamsFromList1 = m_pIpServiceListManager->removeIpService(pConfIpParam1->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", pConfDeletedIpParamsFromList1 != NULL);
	numberOfServicesInList = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", numberOfServicesInList == 2);
	/////////////////////////////////////////
	// delete 2nd item
    /////////////////////////////////////////
	 CConfIpParameters* pConfDeletedIpParamsFromList2 = m_pIpServiceListManager->removeIpService(pConfIpParam2->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", pConfDeletedIpParamsFromList2 != NULL);
	numberOfServicesInList = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", numberOfServicesInList == 1);
	/////////////////////////////////////////
	// delete 2nd item
    /////////////////////////////////////////
	 CConfIpParameters* pConfDeletedIpParamsFromList3 = m_pIpServiceListManager->removeIpService(pConfIpParam3->service_id);
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", pConfDeletedIpParamsFromList3 != NULL);
	numberOfServicesInList = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::InsertAndDeleteThreeServicesToList", numberOfServicesInList == 0);
//    /////////////////////////////////////////
//	// clean up
//	/////////////////////////////////////////
//    POBJDELETE(pConfIpService1);
//	POBJDELETE(pConfIpService2);
//	POBJDELETE(pConfIpService3);
//	/////////////////////////////////////////
	PDELETE(pConfIpParam1);
	PDELETE(pConfIpParam2);
	PDELETE(pConfIpParam3);
}
//////////////////////////////////////////////////////////////////////
void CTestIpServicesList::FindServiceByName()
{
//	CIpServiceListManager* pIpServiceListManager = new 	CIpServiceListManager;
	CConfIpParameters* pConfIpParameters = new CConfIpParameters;
	CONF_IP_PARAMS_S* pConfIpParams = new CONF_IP_PARAMS_S;
	// Initialize parameters
	pConfIpParams->service_id = 6;
	char serviceName[20] = "Kuku Ruku";
	char aliasName[20] = "Poly Gavri";
	strncpy(pConfIpParams->service_name,serviceName, strlen(serviceName));
	//pConfIpParams->gatekeeper_Mode = 2;
	//pConfIpParams->alias_type = 7;
	//WORD sizeOfAlias = sizeof(pConfIpParams->alias_name);
	//memset(pConfIpParams->alias_name,'\0',sizeOfAlias);
	//strncpy(pConfIpParams->alias_name,aliasName, strlen(aliasName));
	pConfIpParameters->SetData(pConfIpParams); 
	/////////////////////////////////////////
	// Insert Item to List 
	/////////////////////////////////////////
	m_pIpServiceListManager->insertIpService(pConfIpParameters);
    WORD numberOfServicesInList = m_pIpServiceListManager->numberOfIpServices();
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::FindServiceByName", numberOfServicesInList == 1);
	/////////////////////////////////////////
	// Find IP service according to Name
	/////////////////////////////////////////
    //WORD LengthOfAlias = strlen( (const char*)pConfIpParameters->GetAliasName() );
	//char* aliasName1 = (char*)(pConfIpParameters->GetAliasName());
	CConfIpParameters* pConfIpParamsFromList = m_pIpServiceListManager->FindServiceByName(serviceName);/*pConfIpParams->alias_name*/;
	CPPUNIT_ASSERT_MESSAGE("CTestIpServicesList::FindServiceByName", pConfIpParamsFromList != NULL);
//    /////////////////////////////////////////
//	// clean up
//	/////////////////////////////////////////
//    POBJDELETE(pConfIpParameters);
	PDELETE(pConfIpParams);	
}


   
