// TestAuthentication.cpp: implementation of the CTestAuthentication class.
//
//////////////////////////////////////////////////////////////////////



#include "TestAuthentication.h"
#include "Authentication.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestAuthentication );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestAuthentication::setUp()
{
	m_pAuthentication = new CAuthentication;
}

//////////////////////////////////////////////////////////////////////
void CTestAuthentication::tearDown()
{
	POBJDELETE(m_pAuthentication);
}

//////////////////////////////////////////////////////////////////////
void CTestAuthentication::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestAuthentication::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testConstructor ",
		m_pAuthentication != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestAuthentication::testSetSerialNumber()
{

	FPTRACE(eLevelInfoNormal,"CTestAuthentication::testSetSerialNumber");


	CAuthentication *authent = new CAuthentication;

	char tmp1[MPL_SERIAL_NUM_LEN] = "1234";
	authent->SetSerialNumber(tmp1);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetSerialNumber ",
		!strcmp("1234", authent->GetSerialNumber()) );


	char tmp2[MPL_SERIAL_NUM_LEN] = "5645";
	authent->SetSerialNumber(tmp2);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetSerialNumber ",
		!strcmp("5645", authent->GetSerialNumber()) );

	
	POBJDELETE(authent);
} 

//////////////////////////////////////////////////////////////////////
void CTestAuthentication::testSetPlatformType()
{

	FPTRACE(eLevelInfoNormal,"CTestAuthentication::testSetPlatformType");


	CAuthentication *authent = new CAuthentication;

	authent->SetPlatformType(eGideonLite);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetPlatformType ",
		eGideonLite == authent->GetPlatformType() );  


	authent->SetPlatformType(eGideon5);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetPlatformType ",
		eGideon5 == authent->GetPlatformType() );  

	
	authent->SetPlatformType(eGideon14);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetPlatformType ",
		eGideon14 == authent->GetPlatformType() );  

	
	authent->SetPlatformType(eAmos);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetPlatformType ",
		eAmos == authent->GetPlatformType() );  

	
	POBJDELETE(authent);
} 

//////////////////////////////////////////////////////////////////////
void CTestAuthentication::testSetMcuVersion()
{

	FPTRACE(eLevelInfoNormal,"CTestAuthentication::testSetMcuVersion");

	CAuthentication *authent = new CAuthentication;


	VERSION_S vInput1, vInput2, vRes1, vRes2;
	vInput1.ver_major    = vRes1.ver_major    = 1;
	vInput1.ver_minor    = vRes1.ver_minor    = 2;
	vInput1.ver_internal = vRes1.ver_internal = 3;
	vInput1.ver_release  = vRes1.ver_release  = 4;

	vInput2.ver_major    = vRes2.ver_major    = 5;
	vInput2.ver_minor    = vRes2.ver_minor    = 6;
	vInput2.ver_internal = vRes2.ver_internal = 7;
	vInput2.ver_release  = vRes2.ver_release  = 8;


	authent->SetMcuVersionFromMpl(vInput1);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetMcuVersion ",
		(vRes1.ver_major    == authent->GetMcuVersionFromMpl().ver_major)    &&
		(vRes1.ver_minor    == authent->GetMcuVersionFromMpl().ver_minor)    &&
		(vRes1.ver_internal == authent->GetMcuVersionFromMpl().ver_internal) &&
		(vRes1.ver_release  == authent->GetMcuVersionFromMpl().ver_release) );


	authent->SetMcuVersionFromMpl(vInput2);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetMcuVersion ",
		(vRes2.ver_major    == authent->GetMcuVersionFromMpl().ver_major)    &&
		(vRes2.ver_minor    == authent->GetMcuVersionFromMpl().ver_minor)    &&
		(vRes2.ver_internal == authent->GetMcuVersionFromMpl().ver_internal) &&
		(vRes2.ver_release  == authent->GetMcuVersionFromMpl().ver_release) );

	authent->SetMcuChassisVersionFromMpl(vInput1);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetChassisVersion ",
		(vRes1.ver_major    == authent->GetMcuChassisVersionFromMpl().ver_major)    &&
		(vRes1.ver_minor    == authent->GetMcuChassisVersionFromMpl().ver_minor)    &&
		(vRes1.ver_internal == authent->GetMcuChassisVersionFromMpl().ver_internal) &&
		(vRes1.ver_release  == authent->GetMcuChassisVersionFromMpl().ver_release) );


	authent->SetMcuChassisVersionFromMpl(vInput2);
	CPPUNIT_ASSERT_MESSAGE( "CTestAuthentication::testSetChassisVersion ",
		(vRes2.ver_major    == authent->GetMcuChassisVersionFromMpl().ver_major)    &&
		(vRes2.ver_minor    == authent->GetMcuChassisVersionFromMpl().ver_minor)    &&
		(vRes2.ver_internal == authent->GetMcuChassisVersionFromMpl().ver_internal) &&
		(vRes2.ver_release  == authent->GetMcuChassisVersionFromMpl().ver_release) );
	
	

	POBJDELETE(authent);
} 


