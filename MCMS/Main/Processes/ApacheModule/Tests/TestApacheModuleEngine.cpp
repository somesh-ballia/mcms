// TestApacheModuleEngine.cpp: implementation of the CTestApacheModuleEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "TestApacheModuleEngine.h"
#include "ConnectionList.h"
#include "XmlMiniParser.h"
#include "ApacheIncludes.h"
#include "ApacheDefines.h"
#include "ApacheModuleEngine.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestApacheModuleEngine);

CConnectionList* pConnList = NULL;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestApacheModuleEngine::setUp()
{
	pConnList = new CConnectionList;
}

//////////////////////////////////////////////////////////////////////
void CTestApacheModuleEngine::tearDown()
{
	delete pConnList;
}

//////////////////////////////////////////////////////////////////////
void CTestApacheModuleEngine::testConnListConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testConnListConstructor",
		pConnList != NULL );  

	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testConnListConstructor",
		pConnList->GetCount() == 0 );  
} 

void CTestApacheModuleEngine::testAddRemoveConnection()
{
	CConnection connection;

	connection.SetLogin("UserLogin");
	connection.SetStationName("UserStationName");
	connection.SetAuthorization(GUEST);

	int nConnId = pConnList->AddConnection(connection);

	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testAddConnectionToConnList",
							pConnList->IsValidConnection(nConnId) == true );

	pConnList->RemoveConnection(nConnId);

	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testAddConnectionToConnList",
							pConnList->IsValidConnection(nConnId) == false ) ;
}

void CTestApacheModuleEngine::testFindActionName()
{
	CXmlMiniParser XmlMiniParser;
	char *pszStartActionName, *pszEndActionName;

	const char* pszTransaction = "<TRANS_MCU><TRANS_COMMON_PARAMS><MCU_TOKEN>2</MCU_TOKEN><MCU_USER_TOKEN>2</MCU_USER_TOKEN></TRANS_COMMON_PARAMS><ACTION><LOGIN><MCU_IP>127.0.0.1</MCU_IP><USER_NAME>TestUser</USER_NAME><PASSWORD>TestPassword</PASSWORD></LOGIN></ACTION></TRANS_MCU>";

	pszStartActionName = XmlMiniParser.GetActionName(pszTransaction,&pszEndActionName);

	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testFindActionName",
							pszStartActionName != NULL );

	int nStrSize = pszEndActionName - pszStartActionName;
	
	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testFindActionName",
							!strncmp(pszStartActionName,"LOGIN",nStrSize) );
}

void CTestApacheModuleEngine::testFindTransName()
{
	CXmlMiniParser XmlMiniParser;
	char *pszStartTransName, *pszEndTransName;

	const char* pszTransaction = "<TRANS_MCU><TRANS_COMMON_PARAMS><MCU_TOKEN>2</MCU_TOKEN><MCU_USER_TOKEN>2</MCU_USER_TOKEN></TRANS_COMMON_PARAMS><ACTION><LOGIN><MCU_IP>127.0.0.1</MCU_IP><USER_NAME>TestUser</USER_NAME><PASSWORD>TestPassword</PASSWORD></LOGIN></ACTION></TRANS_MCU>";

	pszStartTransName = XmlMiniParser.GetTransName(pszTransaction,&pszEndTransName);

	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testFindTransName",
							pszStartTransName != NULL );

	int nStrSize = pszEndTransName - pszStartTransName;
	
	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::testFindTransName",
							!strncmp(pszStartTransName,"TRANS_MCU",nStrSize) );
}

//////////////////////////////////////////////////////////////////////
void CTestApacheModuleEngine::TestVirtualDirectoryList()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestApacheModuleEngine::TestVirtualDirectoryList",
		pConnList != NULL );

	// the data from real S-MCU
	struct { std::string pszPhyz; char* pszVirt; char* pszAcces; char* pszCreator; char* pszAudit; } data[] = {
				{ (MCU_MCMS_DIR+"/EMA/")               , ""                                      , "anonymous"           , "none"          , "NO" },
				{ (MCU_MCMS_DIR+"/Backup/")            , "Backup"                                , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/Install/")           , "Install"                               , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/KeysForCS/")         , "OCS"                                   , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/Audit/")             , "Audit"                                 , "administrator_readonly" , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/Restore/")           , "Restore"                               , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/LogFiles/")          , "LogFiles"                              , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/Cfg/IVR/")           , "Cfg/IVR"                               , "administrator"       , "administrator" , "YES" },
				{ (MCU_MCMS_DIR+"/Cfg/AudibleAlarms/") , "Cfg/AudibleAlarms"                     , "authenticated_user"  , "administrator" , "YES" },
				{ (MCU_MCMS_DIR+"/CdrFiles/")          , "CdrFiles"                              , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/EMACfg/")            , "EMACfg"                                , "authenticated_user"  , "all"           , "NO" },
				{ (MCU_MCMS_DIR+"/Diagnostics/")       , "Diagnostics"                           , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/MediaRecording/")    , "MediaRecording"                        , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/MIBS/")              , "Mibs"                                  , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/CRL/")               , "CRL"                                   , "administrator"       , "none"          , "YES" },
				{ (MCU_MCMS_DIR+"/CACert/")            , "CA"                                    , "administrator"       , "none"          , "YES" },
				{ (MCU_TMP_DIR+"/EMAProductType.txt")  , "EMA.UI/ProductType/EMAProductType.txt" , "nobody"              , "administrator" , "YES" },
				{ (MCU_TMP_DIR+"/JITC_MODE.txt")       , "EMA.UI/ProductType/JITC_MODE.txt"      , "nobody"              , "administrator" , "YES" },
				{ (MCU_CONFIG_DIR+"/ema/InternalConfigSet_Customized.xml") , "EMA.UI/Configuration/InternalConfigSet_Customized.xml", "nobody" , "administrator", "YES" },
				{ (MCU_TMP_DIR+"/Versions.xml")        , "Versions.xml"                          , "nobody"              , "administrator" , "YES" },
	};

	for( int i=0; i<ARRAYEND(data); i++ )
	{
		CApacheModuleEngine::SetPhysicalPath(data[i].pszPhyz, data[i].pszVirt);
		CApacheModuleEngine::SetDirectoryAccess(data[i].pszPhyz, data[i].pszAcces);
		CApacheModuleEngine::SetCreateDirectory(data[i].pszPhyz, data[i].pszCreator);
		CApacheModuleEngine::SetAuditabilityOfDirectory(data[i].pszPhyz, data[i].pszAudit);
	}

	CPPUNIT_ASSERT_MESSAGE("CTestApacheModuleEngine::TestVirtualDirectoryList",
			CApacheModuleEngine::GetVirtualDirectoryPos(MCU_TMP_DIR+"/Versions.xml") == ARRAYEND(data)-1);
	CPPUNIT_ASSERT_MESSAGE();
}

/*
 *
***** ApacheModule *****
Dump of virtual directory's table (i < m_nLastVD)
--------------------------------------------------------------------
MCU_MCMS_DIR/EMA/          ,           , 101 , 2, Not Auditable
MCU_MCMS_DIR/Backup/       , Backup    , 0   , 2, Auditable
MCU_MCMS_DIR/Install/      , Install   , 0   , 2, Auditable
MCU_MCMS_DIR/KeysForCS/    , OCS       , 0   , 2, Auditable
MCU_MCMS_DIR/Audit/        , Audit     , 6   , 2, Auditable
MCU_MCMS_DIR/Restore/      , Restore   , 0   , 2, Auditable
MCU_MCMS_DIR/LogFiles/     , LogFiles  , 0   , 2, Auditable
MCU_MCMS_DIR/Cfg/IVR/      , Cfg/IVR   , 0   , 0, Auditable
MCU_MCMS_DIR/Cfg/AudibleAlarms/, Cfg/AudibleAlarms, 100 , 0, Auditable
MCU_MCMS_DIR/CdrFiles/     , CdrFiles  , 0   , 2, Auditable
MCU_MCMS_DIR/EMACfg/       , EMACfg    , 100 , 1, Not Auditable
MCU_MCMS_DIR/Diagnostics/  , Diagnostics, 0   , 2, Auditable
MCU_MCMS_DIR/MediaRecording/, MediaRecording, 0   , 2, Auditable
MCU_MCMS_DIR/MIBS/         , Mibs      , 0   , 2, Auditable
MCU_MCMS_DIR/CRL/          , CRL       , 0   , 2, Auditable
MCU_MCMS_DIR/CACert/       , CA        , 0   , 2, Auditable
CU_TMP_DIR/EMAProductType.txt, EMA.UI/ProductType/EMAProductType.txt, -1  , 0, Auditable
MCU_TMP_DIR/JITC_MODE.txt  , EMA.UI/ProductType/JITC_MODE.txt, -1  , 0, Auditable
MCU_CONFIG_DIR/ema/InternalConfigSet_Customized.xml, EMA.UI/Configuration/InternalConfigSet_Customized.xml, -1  , 0, Auditable
MCU_TMP_DIR/Versions.xml   , Versions.xml, -1  , 0, Auditable



 *
 */


