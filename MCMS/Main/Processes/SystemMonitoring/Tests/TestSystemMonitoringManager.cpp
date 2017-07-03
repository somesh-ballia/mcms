/*
 * TestSystemMonitoringManager.cpp
 *
 *  Created on: Aug 1, 2014
 *      Author: penrod
 */

#include "TestSystemMonitoringManager.h"

#include "Trace.h"
#include "SystemFunctions.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestSystemMonitoringManager );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void TestSystemMonitoringManager::setUp()
{
	pSystemMonitoringManager = new CTestedTmp;
//	pSystemMonitoringProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void TestSystemMonitoringManager::tearDown()
{
//	SystemSleep(10);
//	pSystemMonitoringProcess->TearDown();
	delete pSystemMonitoringManager;
}

//////////////////////////////////////////////////////////////////////
void TestSystemMonitoringManager::testNinjaCoreManager()
{

	FPTRACE(eLevelInfoNormal,"TestSystemMonitoringManager::testNinjaCoreManager");

	CPPUNIT_ASSERT_MESSAGE( "TestSystemMonitoringManager::testNinjaCoreManager ",
		pSystemMonitoringManager != NULL );
	CPPUNIT_ASSERT_MESSAGE( "TestSystemMonitoringManager::testNinjaCoreManager ",
			pSystemMonitoringManager->m_pCurrentCoreDumpsMap == NULL);
	pSystemMonitoringManager->m_fCoreDumpSizeLimitation = 1;   // suppose the size limitation of core dump files is 1MB
	//create fake core dump list and test function ManageNinjaCoreDump()
	if(NULL == pSystemMonitoringManager->m_pCurrentCoreDumpsMap)
		pSystemMonitoringManager->m_pCurrentCoreDumpsMap = new CORE_MAP;
	CORE_MAP *pCurrentCoreMap = pSystemMonitoringManager->m_pCurrentCoreDumpsMap;
	createCoredumpMap("CSMngr", 6);
	createCoredumpMap("McuMngr", 6);
	createCoredumpMap("Audio", 8);
	DumpCores();
	pSystemMonitoringManager->ManageNinjaCoreDump();
	DumpCores();
	//verify
	CPPUNIT_ASSERT_MESSAGE("TestSystemMonitoringManager::testNinjaCoreManager ",
			3 == (*pCurrentCoreMap)["CSMngr"].size());
	CPPUNIT_ASSERT_MESSAGE("TestSystemMonitoringManager::testNinjaCoreManager ",
				3 == (*pCurrentCoreMap)["McuMngr"].size());
	CPPUNIT_ASSERT_MESSAGE("TestSystemMonitoringManager::testNinjaCoreManager ",
				4 == (*pCurrentCoreMap)["Audio"].size());

	//delete rest temporary files.
	CORE_MAP::iterator itMap;
	for(itMap= pCurrentCoreMap->begin(); itMap != pCurrentCoreMap->end(); ++itMap)
	{
		CORE_LIST::iterator itList;
		for( itList = itMap->second.begin(); itList != itMap->second.end(); ++itList)
			deleteFile(itList->m_fileName.c_str());
	}

}

void TestSystemMonitoringManager::DumpCores()
{
	CORE_MAP::iterator itMap;
	printf("Dump all core dump information\n");
	printf("process\t filename\t\t modification\t size\n");
	for(itMap= pSystemMonitoringManager->m_pCurrentCoreDumpsMap->begin(); itMap != pSystemMonitoringManager->m_pCurrentCoreDumpsMap->end(); ++itMap)
	{
		CORE_LIST::iterator itList;
		for( itList = itMap->second.begin(); itList != itMap->second.end(); ++itList)
			printf("%s\t %s\t %ld\t %lld\n", itMap->first.c_str(),
					itList->m_fileName.c_str(),
					itList->m_modified, itList->m_size);
	}
}

void TestSystemMonitoringManager::createCoredumpMap(std::string strProcessName, int unCount)
{
	CORE_MAP *pCurrentCoreMap = pSystemMonitoringManager->m_pCurrentCoreDumpsMap;
	off_t nCoreSize = 1024*100; // suppose all core dump is 100K
	for(int i=0; i<unCount; i++)
	{
		char caFileName[1024];
		snprintf(caFileName, sizeof(caFileName), "%s_RMX_8.5.0.213_%d", strProcessName.c_str(), i);
		createEmptyFile(caFileName);
		std::string strFileName = caFileName;
		(*pCurrentCoreMap)[strProcessName].push_back(CoreFileDescriptor(strFileName, (time_t)i, nCoreSize));
	}
}

void TestSystemMonitoringManager::createEmptyFile(const char *pFileName)
{
	FILE *pf = fopen(pFileName, "wb");
	if( NULL == pf)
	{
		printf("[ERROR] can't create a file %s error(%d): %s\n", pFileName, errno, strerror(errno));
		CPPUNIT_ASSERT_MESSAGE("TestSystemMonitoringManager::createEmptyFile ", pf != NULL);
	}
	fclose(pf);
}

void TestSystemMonitoringManager::deleteFile(const char *pFileName)
{
	int res = unlink(pFileName);
	if(res)
	{
		printf("[ERROR] can't delete a file %s error(%d): %s\n", pFileName, errno, strerror(errno));
		CPPUNIT_ASSERT_MESSAGE("TestSystemMonitoringManager::deleteFile ", res == 0);
	}
}
