// TestGlobalVideoHWUtils.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestGlobalVideoHWUtils.h"
#include "ConfPartyGlobals.h"
#include "H221.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "cppunit/extensions/HelperMacros.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestGlobalVideoHWUtils);

 
//////////////////////////////////////////////////////////////////////
void CTestGlobalVideoHWUtils::setUp()
{

}
//////////////////////////////////////////////////////////////////////
void CTestGlobalVideoHWUtils::tearDown()
{

	
}
//////////////////////////////////////////////////////////////////////
void CTestGlobalVideoHWUtils::CheckH263GlobalVideoHWUtils()
{   
    BYTE H263Arr[6];
	BYTE localH263[6] = {2,2,2,2};
	
	::Get263VideoCardMPI(128,H263Arr); 

	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode:::CheckH263GlobalVideoHWUtils", H263Arr[0] == localH263[0]);
	
}
/////////////////////////////////////////////////////////////////////
void CTestGlobalVideoHWUtils::CheckH264GlobalVideoHWUtils()
{   
  	
	//CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestConfType", isLocalVideoProtocol == isAutoProtocol);

}


