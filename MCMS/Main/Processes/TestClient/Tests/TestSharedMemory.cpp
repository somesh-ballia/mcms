// TestSharedMemory.cpp: implementation of the CTestSharedMemory class.
//
//////////////////////////////////////////////////////////////////////
#include <string>
#include "TestTestClientProcess.h"
#include "TestClientProcess.h"
#include "DataTypes.h"
#include "OsQueue.h"
#include "Segment.h"
#include "ManagerApi.h"
#include "SystemFunctions.h"
#include "TestServerOpcodes.h"
#include "Trace.h"
#include "SingleToneApi.h"
#include "TraceStream.h"



#include "NStream.h"
#include "SharedMemoryMap.h"
#include "SharedMemoryArray.h"
#include "TestSharedMemory.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestSharedMemory );

struct CTestEntry
{
	DWORD m_id; // this is a mandatory field for SharedMemoryMap
	DWORD m_cardId;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::tearDown()
{
}


//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::testConstructor()
{
//	SystemSleep(10,FALSE);
	//PTRACE(eLevelInfoNormal,"CTestSharedMemory::testConstructor");
	//CTestEntry entry;
	CSharedMemoryMap<CTestEntry> table("Hello",READ_WRITE,1000);
//    table.Clean();
//	string name = table.m_name;
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed status ", table.m_status == STATUS_OK );
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed first ", table.m_first != FALSE );
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed header", table.m_pHeader != NULL );
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed enteris", table.m_pEntries != NULL );
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed max enteries ", table.m_pHeader->m_maxEntries == 1000);
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed num enteries ", table.m_pHeader->m_numEntries == 0);
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed file map ", table.m_fileMap != 0);
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed view ", table.m_pView != NULL);
//	CPPUNIT_ASSERT_MESSAGE( "constructor failed name ", name == "Hello");
} 

//:m_table("ServerTable",READ_WRITE,1000),
// m_array("ServerArray",READ_WRITE,100)

/* in server
CTestEntry entry1 = {1,10};
CTestEntry entry5 = {5,56};
CTestEntry entry6 = {6,87};
CTestEntry entry7 = {7,101};

m_table.Add(entry1);
m_table.Add(entry5);
m_table.Add(entry6);
m_table.Add(entry7);
*/

//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::test2Tables()
{
	//PTRACE(eLevelInfoNormal,"CTestSharedMemory::test2Tables");
	//CTestEntry entry;
	CSharedMemoryMap<CTestEntry> table("ServerTable",READ_WRITE,500);
	CPPUNIT_ASSERT_MESSAGE( "test2Tables failed ", table.m_status == STATUS_OK );  	
	CPPUNIT_ASSERT_MESSAGE( "test2Tables failed ", table.m_first == FALSE );  
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", table.m_pHeader->m_maxEntries == 1000);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", table.m_pHeader->m_numEntries == 4);


	CTestEntry temp;
	temp.m_id = -1;
	temp.m_cardId = -1;
	STATUS stat = table.Get(1,temp);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", stat == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", temp.m_id == 1);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", temp.m_cardId == 10);
} 

//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::testTableOverFlow()
{
	//PTRACE(eLevelInfoNormal,"CTestSharedMemory::testTableOverFlow");
	CSharedMemoryMap<CTestEntry> table("HelloOverFlow",READ_WRITE,20);
	for (DWORD i= 0; i<20; i++)
	{
		CTestEntry entry = {i,i+100};
		STATUS res = table.Add(entry);
		CPPUNIT_ASSERT_MESSAGE(  "Add failed", res == STATUS_OK);	
	}
	CPPUNIT_ASSERT_MESSAGE( "overflow failed", table.m_pHeader->m_numEntries == 20);
	CTestEntry overFlowEntry = {100,300};
	CPPUNIT_ASSERT_MESSAGE( "overflow failed", table.Add(overFlowEntry) != STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "overflow failed", table.m_pHeader->m_numEntries == 20);
	CPPUNIT_ASSERT_MESSAGE( "remove failed", table.Remove(0) == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "remove failed", table.Remove(434) != STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "remove failed", table.m_pHeader->m_numEntries == 19);
	CPPUNIT_ASSERT_MESSAGE( "overflow failed", table.Add(overFlowEntry) == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "remove failed", table.m_pHeader->m_numEntries == 20);

	CTestEntry temp;
	temp.m_id = -1;
	temp.m_cardId = -1;
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Get(3253,temp) != STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Get(5,temp) == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_id == 5);
	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_cardId == 105);
	
	temp.m_cardId = 800;
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Update(temp) == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Get(5,temp) == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_id == 5);
	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_cardId == 800);

	temp.m_cardId = 5676;
	temp.m_id = 5676;
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Update(temp) != STATUS_OK);

	table.Clean();

	CPPUNIT_ASSERT_MESSAGE( "remove failed", table.m_pHeader->m_numEntries == 0);
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Get(3253,temp) != STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "find failed", table.Get(5,temp) != STATUS_OK);

}

void CTestSharedMemory::testArrayConstructor()
{
	//PTRACE(eLevelInfoNormal,"CTestSharedMemory::testArrayConstructor");
	CSharedMemoryArray<CTestEntry> table("TestTable",READ_WRITE,40000);
    table.Clean();
    
	string name = table.m_name;
	CPPUNIT_ASSERT_MESSAGE( "constructor failed status ", table.m_status == STATUS_OK );  	
	CPPUNIT_ASSERT_MESSAGE( "constructor failed first ", table.m_first != FALSE );  
	CPPUNIT_ASSERT_MESSAGE( "constructor failed header ", table.m_pHeader != NULL ); 
	CPPUNIT_ASSERT_MESSAGE( "constructor failed enteries ", table.m_pEntries != NULL ); 
	CPPUNIT_ASSERT_MESSAGE( "constructor failed max ", table.m_pHeader->m_maxEntries == 40000);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed num ", table.m_pHeader->m_numEntries == 0);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed file ", table.m_fileMap != 0);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed view ", table.m_pView != NULL);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed name ", name == "TestTable");
}

//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::test2Arrays()
{
	//PTRACE(eLevelInfoNormal,"CTestSharedMemory::test2Arrays");
	//CTestEntry entry;
	CSharedMemoryArray<CTestEntry> table("ServerArray",READ_WRITE,500);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", table.m_status == STATUS_OK );  	
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", table.m_first == FALSE );  
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", table.m_pHeader->m_maxEntries == 100);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", table.m_pHeader->m_numEntries == 4);

	CTestEntry temp;
	temp.m_id = -1;
	temp.m_cardId = -1;
	STATUS stat = table.Get(1,temp);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", stat == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", temp.m_id == 1);
	CPPUNIT_ASSERT_MESSAGE( "constructor failed ", temp.m_cardId == 10);
} 


//////////////////////////////////////////////////////////////////////
void CTestSharedMemory::testArrayOverFlow()
{	
	//PTRACE(eLevelInfoNormal,"CTestSharedMemory::testArrayOverFlow");
//	CSharedMemoryArray<CTestEntry> array("HelloOverFlowArray",READ_WRITE,20);
//	for (DWORD i= 0; i<20; i++)
//	{
//		CTestEntry entry = {i,i+100};
//		STATUS res = array.Add(entry);
//		CPPUNIT_ASSERT_MESSAGE(  "Add failed", res == STATUS_OK);
//	}
//	CPPUNIT_ASSERT_MESSAGE( "overflow failed", array.m_pHeader->m_numEntries == 20);
//	CTestEntry overFlowEntry = {20,300};
//	CPPUNIT_ASSERT_MESSAGE( "overflow failed", array.Add(overFlowEntry) != STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "overflow failed", array.m_pHeader->m_numEntries == 20);
//	CPPUNIT_ASSERT_MESSAGE( "remove failed", array.Remove(0) == STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "remove failed", array.Remove(434) != STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "remove failed", array.m_pHeader->m_numEntries == 20);
//	CPPUNIT_ASSERT_MESSAGE( "overflow failed", array.Add(overFlowEntry) != STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "remove failed", array.m_pHeader->m_numEntries == 20);
//
//	CTestEntry temp;
//	temp.m_id = -1;
//	temp.m_cardId = -1;
//
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Get(3253,temp) != STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Get(5,temp) == STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_id == 5);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_cardId == 105);
//
//	temp.m_cardId = 800;
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Update(temp) == STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Get(5,temp) == STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_id == 5);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", temp.m_cardId == 800);
//
//	temp.m_cardId = 5676;
//	temp.m_id = 5676;
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Update(temp) != STATUS_OK);
//
//	array.Clean();
//
//	CPPUNIT_ASSERT_MESSAGE( "remove failed", array.m_pHeader->m_numEntries == 0);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Get(3253,temp) != STATUS_OK);
//	CPPUNIT_ASSERT_MESSAGE( "find failed", array.Get(5,temp) != STATUS_OK);

}

