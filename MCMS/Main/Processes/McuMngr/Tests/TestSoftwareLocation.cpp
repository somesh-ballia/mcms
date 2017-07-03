// TestSoftwareLocation.cpp: implementation of the CTestSoftwareLocation class.
//
//////////////////////////////////////////////////////////////////////



#include "TestSoftwareLocation.h"
#include "SoftwareLocation.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestSoftwareLocation );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::setUp()
{
	m_pSoftwareLocation = new CSoftwareLocation;
}

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::tearDown()
{
	POBJDELETE(m_pSoftwareLocation);
}

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testConstructor ",
		m_pSoftwareLocation != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetHostName()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetHostName");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	char hName1[NAME_LEN] = "first host";
	char hName2[NAME_LEN] = "second host";

	swLocation->SetHostName((BYTE*)hName1);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetHostName ",
		!memcmp("first host", swLocation->GetHostName(), strlen("first host")) );  

	swLocation->SetHostName((BYTE*)hName2);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetHostName ",
		!memcmp(hName2, swLocation->GetHostName(), strlen(hName2)) );  

	
	POBJDELETE(swLocation);
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetHostIp()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetHostIp");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	DWORD ipAdd1 = 1234, ipAdd2 = 5678;

	swLocation->SetHostIp(ipAdd1);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetHostIp ",
		              1234 == swLocation->GetHostIp() );  

	swLocation->SetHostIp(ipAdd2);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetHostIp ",
		              5678 == swLocation->GetHostIp() );  

	
	POBJDELETE(swLocation);
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetLocation()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetLocation");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	char location1[NAME_LEN] = "../Libs/Common";
	char location2[NAME_LEN] = "../Include";

	swLocation->SetHostName((BYTE*)location1);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetLocation ",
		!memcmp("../Libs/Common", swLocation->GetHostName(), strlen("../Libs/Common")) );  

	swLocation->SetHostName((BYTE*)location2);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetLocation ",
		!memcmp(location2, swLocation->GetHostName(), strlen(location2)) );  


	
	POBJDELETE(swLocation);
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetUrlType()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetUrlType");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	swLocation->SetUrlType(eFtp);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetUrlType ",
		eFtp == swLocation->GetUrlType() );  


	swLocation->SetUrlType(eNfs);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetUrlType ",
		eNfs == swLocation->GetUrlType() );  

	
	POBJDELETE(swLocation);
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetUserName()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetUserName");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	char uName1[NAME_LEN] = "me";
	char uName2[NAME_LEN] = "you";

	swLocation->SetUserName((BYTE*)uName1);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetUserName ",
		!memcmp("me", swLocation->GetUserName(), strlen("me")) );  

	swLocation->SetUserName((BYTE*)uName2);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetUserName ",
		!memcmp(uName2, swLocation->GetUserName(), strlen(uName2)) );  

	
	POBJDELETE(swLocation);
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetPassword()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetPassword");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	char pwd1[NAME_LEN] = "so secret";
	char pwd2[NAME_LEN] = "very secret";

	swLocation->SetPassword((BYTE*)pwd1);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetPassword ",
		!memcmp("so secret", swLocation->GetPassword(), strlen("so secret")) );  

	swLocation->SetPassword((BYTE*)pwd2);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetPassword ",
		!memcmp(pwd2, swLocation->GetPassword(), strlen(pwd2)) );  

	
	POBJDELETE(swLocation);
} 

//////////////////////////////////////////////////////////////////////
void CTestSoftwareLocation::testSetVLanId()
{

	FPTRACE(eLevelInfoNormal,"CTestSoftwareLocation::testSetVLanId");


	CSoftwareLocation *swLocation = new CSoftwareLocation;

	DWORD vLanId1 = 12, vLanId2 = 34;

	swLocation->SetVLanId(vLanId1);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetVLanId ",
		              12 == swLocation->GetVLanId() );  

	swLocation->SetVLanId(vLanId2);
	CPPUNIT_ASSERT_MESSAGE( "CTestSoftwareLocation::testSetVLanId ",
		              34 == swLocation->GetVLanId() );  

	
	POBJDELETE(swLocation);
} 

