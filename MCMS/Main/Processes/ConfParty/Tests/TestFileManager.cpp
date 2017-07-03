#include "TestFileManager.h"
#include "CommRes.h"
#include "StatusesGeneral.h"
#include "CommRes.h"
#include "psosxml.h"
       
CPPUNIT_TEST_SUITE_REGISTRATION( CTestFileManager );

 
/******************************************************************************
 * This functor holds the profile name and the profile file name prefix,
 *  when it reaches a file name it is extracting the profile name and comparing
 *  it to the m_profileName
 ******************************************************************************/
struct CProfilesFunctor
{
  CProfilesFunctor(const string & profileID):m_profileId(profileID),m_suffix(".xml"){}
  
  CProfilesFunctor(const CProfilesFunctor & other)
    :m_profileId(other.m_profileId),m_suffix(".xml"){}
  
  bool operator()(const string & str)const{
  	
    if ( str.find(m_suffix) == string::npos)
      return false;    
      
	unsigned int startId,idLength;
	startId=str.rfind("_") + 1                ; //begine of the profile id string
	idLength = (str.find(m_suffix) -  startId);//length of the conf id   
    string profileId= str.substr(startId,idLength);
    
    return m_profileId == profileId;
  }
  
private:
  string m_profileId; //Holds the profile name
  string m_suffix     ;// hold the specified profile file suffix
};
//------------------------------------------------------------------------------------


/******************************************************************************
 * This functor is extracting the profile id and returning it back
 ******************************************************************************/
struct CProfilesIDFunctor
{
  CProfilesIDFunctor():m_suffix(".xml"){}
  
  CProfilesIDFunctor(const CProfilesIDFunctor & other)
    :m_suffix(".xml"){}
  
  int operator()(const string & str)const{	
    if ( str.find(m_suffix) == string::npos)
      return false;    
      
	unsigned int startId,idLength;
	startId=str.rfind("_") + 1                ; //begine of the profile id string
	idLength = (str.find(m_suffix) -  startId);//length of the conf id   
    string profileId= str.substr(startId,idLength);
    return atoi(profileId.c_str());
  }
private:
  string m_suffix     ;// hold the specified profile file suffix
};
//-------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CTestFileManager::setUp()
{ 
	m_testFolder=MCU_TMP_DIR+"/profilesTest/";
	CreateDirectory(m_testFolder.c_str());
	m_pFileManager = new CFileManagerMock(m_testFolder.c_str());
}
//////////////////////////////////////////////CCommRes////////////////////////
void CTestFileManager::tearDown()
{
	rmdir(m_testFolder.c_str());
	POBJDELETE(m_pFileManager);
}
//////////////////////////////////////////////////////////////////////
void CTestFileManager::TestAdd()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
//	res1.SetDisplayName("res1");
	res1.SetMonitorConfId(111);
	     
	res2.SetName("res2");
//	res2.SetDisplayName("res2");
	res2.SetMonitorConfId(222);
	
	res3.SetName("res3");
//	res3.SetDisplayName("res3");
	res3.SetMonitorConfId(333);
	  
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res1,"res1.xml") == STATUS_OK) ;
	//Try to add it again
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res1,"res1.xml") != STATUS_OK) ;
	
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res2,"res2.xml") == STATUS_OK) ;
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res3,"res3.xml") == STATUS_OK) ;
	
	m_pFileManager->DeleteFileData("res1.xml");
	m_pFileManager->DeleteFileData("res2.xml");
	m_pFileManager->DeleteFileData("res3.xml");
}

void CTestFileManager::TestGetFileData()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
//	res1.SetDisplayName("res1");
	res1.SetSIPFactory(TRUE);
	res1.SetMonitorConfId(111);
	res2.SetName("res2");
//	res2.SetDisplayName("res2");
	res2.SetMonitorConfId(222);
	res3.SetName("res3");
//	res3.SetDisplayName("res3");
	res3.SetMonitorConfId(333);
	
// 	//Try to get data from empty array
 	CCommRes * pCommRes = m_pFileManager->GetFileData("res1.xml");
 	CPPUNIT_ASSERT( pCommRes == NULL );
	
 	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res1,"res1.xml") == STATUS_OK) ;
 	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res2,"res2.xml") == STATUS_OK) ;
 	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res3,"res3.xml") == STATUS_OK) ;
	
	pCommRes = m_pFileManager->GetFileData("res1.xml");
	CPPUNIT_ASSERT( pCommRes != NULL );
	CPPUNIT_ASSERT(strcmp(pCommRes->GetName(),"res1") == 0 );
	CPPUNIT_ASSERT(pCommRes->GetMonitorConfId() == 111 );
	CPPUNIT_ASSERT(pCommRes->IsSIPFactory()  == TRUE);
	POBJDELETE(pCommRes);
	
	//Get res1 again
	pCommRes = m_pFileManager->GetFileData("res1.xml");
	CPPUNIT_ASSERT( pCommRes != NULL );
	CPPUNIT_ASSERT(strcmp(pCommRes->GetName(),"res1") == 0 );
	CPPUNIT_ASSERT(pCommRes->GetMonitorConfId() == 111 );
	CPPUNIT_ASSERT(pCommRes->IsSIPFactory()  == TRUE);
	POBJDELETE(pCommRes);
	
	pCommRes = m_pFileManager->GetFileData("res2.xml");
	CPPUNIT_ASSERT( pCommRes != NULL );
	CPPUNIT_ASSERT(strcmp(pCommRes->GetName(),"res2") == 0 );
	CPPUNIT_ASSERT(pCommRes->GetMonitorConfId() == 222 );
	CPPUNIT_ASSERT(pCommRes->IsSIPFactory()  == FALSE);
	POBJDELETE(pCommRes);
	
	 pCommRes = m_pFileManager->GetFileData("res3.xml");
	 CPPUNIT_ASSERT( pCommRes != NULL );
 	CPPUNIT_ASSERT(strcmp(pCommRes->GetName(),"res3") == 0 );
 	CPPUNIT_ASSERT(pCommRes->GetMonitorConfId() == 333 );
 	//CPPUNIT_ASSERT(pCommRes->IsSIPFactory()  == TRUE);
 	POBJDELETE(pCommRes); 
	
 	m_pFileManager->DeleteFileData("res1.xml");
 	m_pFileManager->DeleteFileData("res2.xml");
 	m_pFileManager->DeleteFileData("res3.xml");
	 
}
  
void CTestFileManager::TestDeleteData()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
//	res1.SetDisplayName("res1");
	res1.SetMonitorConfId(111);
	res2.SetName("res2");
//	res2.SetDisplayName("res2");/view/udib_MCMS-V2.0/vob/MCMS/Main
	res2.SetMonitorConfId(222);
	res3.SetName("res3");
//	res3.SetDisplayName("res3");
	res3.SetMonitorConfId(333);
	
	//delete from empty list
	CPPUNIT_ASSERT( m_pFileManager->DeleteFileData("res1.xml") !=  STATUS_OK);
	
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res1,"res1.xml") == STATUS_OK) ;
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res2,"res2.xml") == STATUS_OK) ;
	CPPUNIT_ASSERT( m_pFileManager->AddFileData(res3,"res3.xml") == STATUS_OK) ;
	
	//delete res1
	CPPUNIT_ASSERT( m_pFileManager->DeleteFileData("res1.xml") ==  STATUS_OK);
	
	//try to delete again
	CPPUNIT_ASSERT( m_pFileManager->DeleteFileData("res1.xml") !=  STATUS_OK);
	
	CPPUNIT_ASSERT( m_pFileManager->DeleteFileData("res2.xml") ==  STATUS_OK);
	CPPUNIT_ASSERT( m_pFileManager->DeleteFileData("res3.xml") ==  STATUS_OK);
}

void CTestFileManager::TestCopyCtorAssignOperator()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
	res1.SetMonitorConfId(111);
	res2.SetName("res2");
	res2.SetMonitorConfId(222);
	res3.SetName("res3");
	res3.SetMonitorConfId(333);
	
	m_pFileManager->AddFileData(res1,"res1.xml") ;
	m_pFileManager->AddFileData(res2,"res2.xml") ;
	m_pFileManager->AddFileData(res3,"res3.xml") ;
	
	//Copy Ctor instance
	CFileManagerMock fileManager1(*m_pFileManager);
	
	//Testing Assignment operator
	CFileManagerMock fileManager2("/dirname/");
	fileManager2 = *m_pFileManager;
	
	//Test if the object are equal
	CPPUNIT_ASSERT(fileManager2 == (*m_pFileManager) );
	CPPUNIT_ASSERT(fileManager1 == (*m_pFileManager) );
	
	m_pFileManager->DeleteFileData("res1.xml");
	m_pFileManager->DeleteFileData("res2.xml");
	m_pFileManager->DeleteFileData("res3.xml");
}
 
void CTestFileManager::TestLoadDataToVect()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
	res1.SetMonitorConfId(111);
	res2.SetName("res2");
	res2.SetMonitorConfId(222);
	res3.SetName("res3");
	res3.SetMonitorConfId(333);
	
	//Create 3 xml files in the folder
	m_pFileManager->AddFileData(res1,"res1.xml") ;
	m_pFileManager->AddFileData(res2,"res2.xml") ;
	m_pFileManager->AddFileData(res3,"res3.xml") ;
	
	//Create another instance which will be load this 3 files 
	CFileManagerMock fileManager1(m_testFolder);
	
	std::vector< CCommRes *> vect;	
	CPPUNIT_ASSERT(fileManager1.LoadDataToVect(vect) == STATUS_OK);
	
	
	CPPUNIT_ASSERT(vect.size() == 3);
	
	std::vector< CCommRes * >::iterator it;
	
	bool flag=false;
	for ( it = vect.begin(); it != vect.end() ; ++it )
		if ( (*it)->GetMonitorConfId() == 111 ){
			flag = CompareCommRes(*(*it),res1) ;
			break;
		}		
	CPPUNIT_ASSERT(flag);
	
	flag=false;
	for ( it = vect.begin(); it != vect.end() ; ++it )
		if ( (*it)->GetMonitorConfId() == 222 ){
			flag = CompareCommRes(*(*it),res2) ;
			break;
		}		
	CPPUNIT_ASSERT(flag);
	
	flag=false;
	for ( it = vect.begin(); it != vect.end() ; ++it )
		if ( (*it)->GetMonitorConfId() == 333 ){
			flag = CompareCommRes(*(*it),res3) ;
			break;
		}		
	CPPUNIT_ASSERT(flag);
	


//Clean  the test
	for ( it = vect.begin(); it != vect.end() ; ++it )
		POBJDELETE(*it);	
	
	m_pFileManager->DeleteFileData("res1.xml");
	m_pFileManager->DeleteFileData("res2.xml");
	m_pFileManager->DeleteFileData("res3.xml");
}
  

bool CTestFileManager::CompareCommRes(const CCommRes & commRes,const CCommRes & otherRes)const
{
	return ( (strcmp(commRes.GetName(),otherRes.GetName()) == 0 ) &&
			commRes.GetMonitorConfId() == otherRes.GetMonitorConfId());
}

void CTestFileManager::TestGetFileName()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
	res1.SetMonitorConfId(111);
	res2.SetName("res2");
	res2.SetMonitorConfId(222);
	res3.SetName("res3");
	res3.SetMonitorConfId(333);
	
	//Create 3 xml files in the folder
	m_pFileManager->AddFileData(res1,res1.GetFileUniqueName("profile")) ;
	m_pFileManager->AddFileData(res2,res2.GetFileUniqueName("profile")) ;
	m_pFileManager->AddFileData(res3,res3.GetFileUniqueName("profile")) ;
	
	     
	CProfilesFunctor functor1("111");
	std::string str = m_pFileManager->GetFileName(functor1);
	CPPUNIT_ASSERT( str == res1.GetFileUniqueName("profile") );
	 
	CProfilesFunctor functor2("222");
	str = m_pFileManager->GetFileName(functor2);
	CPPUNIT_ASSERT( str == res2.GetFileUniqueName("profile"));
	
	CProfilesFunctor functor3("333");
	str = m_pFileManager->GetFileName(functor3);
	CPPUNIT_ASSERT( str == res3.GetFileUniqueName("profile"));
	
	
	m_pFileManager->DeleteFileData(res1.GetFileUniqueName("profile"));
	m_pFileManager->DeleteFileData(res2.GetFileUniqueName("profile"));
	m_pFileManager->DeleteFileData(res3.GetFileUniqueName("profile"));
}


 
