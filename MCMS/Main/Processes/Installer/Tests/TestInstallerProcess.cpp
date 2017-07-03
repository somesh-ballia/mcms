// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestInstallerProcess.h"
#include "InstallerProcess.h"
#include "InstallerManager.h"
#include "Trace.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "InstallerManager.h"
#include <ostream>
#include "TerminalCommand.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestInstallerProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestInstallerProcess::setUp()
{

}

//////////////////////////////////////////////////////////////////////
void CTestInstallerProcess::tearDown()
{

}

//////////////////////////////////////////////////////////////////////
void CTestInstallerProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestInstallerProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

//////////////////////////////////////////////////////////////////////
void CTestInstallerProcess::testConverteString2Version()
{
    std::string mystring = "RMX_103.0045.222.1984";
    
    VERSION_S ver = CInstallerManager::ConvertStringToVersionStruct(mystring);
    
//	FPTRACE(eLevelInfoNormal,"CTestInstallerProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_major == 103 );
	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_minor == 45 );
	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_release == 222 );
	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_internal == 1984 );
} 

//////////////////////////////////////////////////////////////////////
void CTestInstallerProcess::testConverteString2VersionPrivateVersion()
{
    std::string mystring = "RMX_1.0.0.044_haggai_2006-04-27_15:28:38";
    
    VERSION_S ver = CInstallerManager::ConvertStringToVersionStruct(mystring);
    
//	FPTRACE(eLevelInfoNormal,"CTestInstallerProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_major == 1 );
	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_minor == 0 );
	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_release == 0 );
	CPPUNIT_ASSERT_MESSAGE( "CTestInstallerProcess::testConverteString2Version ",
		ver.ver_internal == 44 );
} 

void CTestInstallerProcess::testCheckNewVersionValidity()
{
	CTerminalCommand command;
	std::ostringstream answer;
	std::string str;

	CInstallerManager mngr;
	const char* terminal_file_name = ttyname(0);
	if (NULL == terminal_file_name)
	{
	    terminal_file_name = "/dev/null";
	}
	command.AddToken(terminal_file_name);
	command.AddToken("Installer");
	command.AddToken("test_new_ver_validation");
	const char* rmx_bin_path = getenv("RMX_BIN_FILE_PATH");
	if (rmx_bin_path == NULL)
	{
		return;
	}
	command.AddToken(rmx_bin_path);
	command.AddToken("YES");

	//	command.AddToken("/Carmel-Versions/NonStableBuild/RMX_100.0/RMX_8.1.6.24/RMX_8.1.6.24.bin");
	STATUS stat = mngr.HandleTerminalCheckNewVersionValidity(command,answer);
	CPPUNIT_ASSERT_MESSAGE( "CTestCheckNewVersionValidity::testCheckNewVersionValidity Faield", STATUS_OK == stat);
}
