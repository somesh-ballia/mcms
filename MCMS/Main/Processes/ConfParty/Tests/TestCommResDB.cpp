
#include "TestCommResDB.h"
#include "CommResDB.h"
#include "CommResShort.h"
#include "StatusesGeneral.h"
#include "CommRes.h"
#include "psosxml.h"
#include <unistd.h>
      
CPPUNIT_TEST_SUITE_REGISTRATION( CTestCommResDB );

 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CTestCommResDB::setUp()
{
	m_testFolder=MCU_TMP_DIR+"/TestCommResDB/" ;
	CreateDirectory(m_testFolder.c_str());
	std::string cmd = "rm -Rf "+MCU_TMP_DIR+"/TestCommResDB/*";
	system(cmd.c_str());
	m_pCommResDB = new CCommResDB;	
	m_pCommResDB->SetFolderPath(m_testFolder.c_str());
    CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::setUp fail number of elements is not 0",m_pCommResDB->GetResNumber() == 0);  
}
//////////////////////////////////////////////CCommRes////////////////////////
void CTestCommResDB::tearDown()
{
	rmdir(m_testFolder.c_str());	
	POBJDELETE(m_pCommResDB);
	
}
//////////////////////////////////////////////////////////////////////
void CTestCommResDB::TestAddRes()
{
	CCommRes res1,res2;
	res1.SetName("res1");
	res2.SetName("res2");
	res1.SetDisplayName("res1");
	res2.SetDisplayName("res2");

	res1.SetMonitorConfId(111);
	res2.SetMonitorConfId(222);

    int add_status = m_pCommResDB->Add(res1);
    
	//Add first Res
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestAddRes fail at first Add status is not OK",add_status == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestAddRes fail number of elements is not 1",m_pCommResDB->GetResNumber() == 1);
	
	//make  sure we can not add it again
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestAddRes ",m_pCommResDB->Add(res1) != STATUS_OK);

	//make sure that res1 is in the vector
	CCommResShort * pResShort = m_pCommResDB->GetCurrentRsrvShort(res1.GetName());
	CPPUNIT_ASSERT(pResShort && pResShort->GetConferenceId() == 111 );
	if(pResShort){
		POBJDELETE(pResShort);
	}
	
	//Add first second
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestAddRes fail at second Add status is not OK",m_pCommResDB->Add(res2) == STATUS_OK);
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestAddRes fail at second Add",m_pCommResDB->GetResNumber() == 2);
	
	//make sure that res2 is in the vector
	CCommResShort * pResShort2 = m_pCommResDB->GetCurrentRsrvShort(res2.GetName());
	CPPUNIT_ASSERT(pResShort2 && pResShort2->GetConferenceId() == 222 );
	if(pResShort2){
		POBJDELETE(pResShort2);
	}
	
	//make  sure we can not add it again
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestAddSRes ",m_pCommResDB->Add(res2) != STATUS_OK);
	
	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res1") == STATUS_OK);
	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res2") == STATUS_OK);
}
   
void CTestCommResDB::TestUpdate()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
	res2.SetName("res2");
	res3.SetName("res3");
	res1.SetDisplayName("res1");
	res2.SetDisplayName("res2");
    res3.SetDisplayName("res3");
	res1.SetMonitorConfId(111);
	res2.SetMonitorConfId(222);
	res3.SetMonitorConfId(333);
	
	//make sure you can not update elements when list is empty
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestUpdate fail at updating in empty list",m_pCommResDB->Update(res1) != STATUS_OK);
	
	//Add 2 reservations
	m_pCommResDB->Add(res1);
	m_pCommResDB->Add(res2);
	
	res1.SetMeetMePerEntryQ(1);
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestUpdate fail at update an element",m_pCommResDB->Update(res1) == STATUS_OK);
	
	//get the res1 element and make sure that its field is updated
	CCommResShort * pResShort = m_pCommResDB->GetCurrentRsrvShort(res1.GetName());
	CPPUNIT_ASSERT(pResShort && pResShort->GetMeetMePerEntryQ() == 1);
	if(pResShort){
		POBJDELETE(pResShort);
	}
	
	//make sure you can not update element which is not in the list
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestUpdate fail in updating nonexisting element",m_pCommResDB->Update(res3) != STATUS_OK);
	
	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res1") == STATUS_OK);
	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res2") == STATUS_OK);
}

void CTestCommResDB::TestCancel()
{
	CCommRes res1,res2,res3;
	res1.SetName("res1");
	res2.SetName("res2");
    res1.SetDisplayName("res1");
    res2.SetDisplayName("res2");
	res1.SetMonitorConfId(111);
	res2.SetMonitorConfId(222);
	res3.SetName("res3");
    res3.SetDisplayName("res3");
	res3.SetMonitorConfId(333);
	
	//make sure you can not cancel elements when the list is empty
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestCancel fail at updating in empty list",m_pCommResDB->Cancel("res1") != STATUS_OK);
	
	//Add 2 reservations
	m_pCommResDB->Add(res1);
	m_pCommResDB->Add(res2);
	CPPUNIT_ASSERT(m_pCommResDB->GetResNumber() == 2);
	
	//make sure you can not cancel  an element which is not in the list
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestCancel fail in canceling nonexisting element",m_pCommResDB->Cancel("res3") != STATUS_OK);
	
	
	//Try to cacncel res1 frmo the vector
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestCancel fails in cancel element 1",m_pCommResDB->Cancel("res1") == STATUS_OK);
	//make sure that the vector size has been updated
	CPPUNIT_ASSERT(m_pCommResDB->GetResNumber() == 1);
	
	//make sure that res2 was left in the vector
	CCommResShort * pResShort = m_pCommResDB->GetCurrentRsrvShort(res2.GetName());
	CPPUNIT_ASSERT(pResShort && pResShort->GetConferenceId() == 222 );
	if(pResShort){
	    POBJDELETE(pResShort);
	}
	
	//remove the next Res
	CPPUNIT_ASSERT_MESSAGE( "CTestCommResDB::TestCancel fails in cancel element 2",m_pCommResDB->Cancel(222) == STATUS_OK);	
	//Vector must be empty
	CPPUNIT_ASSERT(m_pCommResDB->GetResNumber() == 0);
}  

void CTestCommResDB::TestSerializeXml()
{
	// CCommRes res1,res2,res3;
// 	res1.SetName("res1");	
// 	res1.SetH243Password("Test");
// 	res1.SetMonitorConfId(111);
	
// 	res2.SetName("res2");
// 	res2.SetMonitorConfId(222);
	
// 	//Add 2 reservations
// 	CPPUNIT_ASSERT(m_pCommResDB->Add(res1)== STATUS_OK);
// 	CPPUNIT_ASSERT(m_pCommResDB->Add(res2)== STATUS_OK);
	
// 	CXMLDOMElement domeElement;
// 	//serialize the DB
// 	m_pCommResDB->SerializeXml(&domeElement,0);
	
// 	CXMLDOMElement * pNode = 0;
// 	GET_CHILD_NODE((&domeElement),"PROFILE_SUMMARY_LS",pNode);
	
// 	CCommResDB oterResDB;
// 	oterResDB.DeSerializeXml(pNode,0,0);
	
// 	//Test if the new DB is equal to the Serilized one
// 	CPPUNIT_ASSERT(oterResDB.GetSummaryUpdateCounter() == m_pCommResDB->GetSummaryUpdateCounter());
	
	
// 	//Check if all the CCommResShort objects are properly initilized
// 	CPPUNIT_ASSERT(oterResDB.GetResNumber() == 2); //Test its size
	
// 	CCommResShort * pResShort = oterResDB.GetCurrentRsrvShort(res2.GetName());
// 	CPPUNIT_ASSERT(strcmp(pResShort->GetName(),"res2") == 0 );
// 	CPPUNIT_ASSERT(pResShort->GetConferenceId() == 222 );
// 	CPPUNIT_ASSERT(strncmp(pResShort->GetPassw(),"Test",4) != 0);
	
// 	pResShort = oterResDB.GetCurrentRsrvShort(res1.GetName());
// 	CPPUNIT_ASSERT(strcmp(pResShort->GetName(),"res1") == 0 );
// 	CPPUNIT_ASSERT(pResShort->GetConferenceId() == 111 );
// 	CPPUNIT_ASSERT(strncmp(pResShort->GetPassw(),"Test",4) == 0);
	
// 	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res1") == STATUS_OK);
// 	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res2") == STATUS_OK);
}
  

void CTestCommResDB::TestAssignOperator()
{
	//Testing the operator = on the CCommmresDB
	CCommRes res1,res2,res3;
	res1.SetName("res1");
    res1.SetDisplayName("res1");
	res1.SetH243Password("Test");
	res1.SetMonitorConfId(111);
	
	res2.SetName("res2");
    res2.SetDisplayName("res2");
	res2.SetMonitorConfId(222);
	
	res3.SetName("res3");
    res3.SetDisplayName("res3");
	res3.SetMonitorConfId(333);
	
	//Add 2 reservations
	m_pCommResDB->Add(res1);
	m_pCommResDB->Add(res2);
	m_pCommResDB->Add(res3);
	
	CCommResDB otherResDB;
	
	otherResDB = *m_pCommResDB;
	
	//Test the new DB content
	
	//Test if the new DB is equal to the Serilized one
	CPPUNIT_ASSERT(otherResDB.GetSummaryUpdateCounter() == m_pCommResDB->GetSummaryUpdateCounter());
	
	
	//Check if all the CCommResShort objects are properly initilized
	CPPUNIT_ASSERT(otherResDB.GetResNumber() == 3); //Test its size
	
	CCommResShort * pResShort = otherResDB.GetCurrentRsrvShort(res2.GetName());
	CPPUNIT_ASSERT(pResShort);
	if(pResShort){
		CPPUNIT_ASSERT(strcmp(pResShort->GetName(),"res2") == 0 );
		CPPUNIT_ASSERT(pResShort->GetConferenceId() == 222 );
		CPPUNIT_ASSERT(strncmp("Test",pResShort->GetPassw(),4) != 0);
		POBJDELETE(pResShort);
	}

	
	pResShort = otherResDB.GetCurrentRsrvShort(res1.GetName());
	CPPUNIT_ASSERT(pResShort);
	if(pResShort){
		CPPUNIT_ASSERT(strcmp(pResShort->GetName(),"res1") == 0 );
		CPPUNIT_ASSERT(pResShort->GetConferenceId() == 111 );
		CPPUNIT_ASSERT(strncmp("Test",pResShort->GetPassw(),4) == 0);
		POBJDELETE(pResShort);
	}

	
	pResShort = otherResDB.GetCurrentRsrvShort(res3.GetMonitorConfId());
	CPPUNIT_ASSERT(pResShort);
	if(pResShort){
		CPPUNIT_ASSERT(strcmp(pResShort->GetName(),"res3") == 0 );
		CPPUNIT_ASSERT(pResShort->GetConferenceId() == 333 );
		CPPUNIT_ASSERT(strncmp("Test",pResShort->GetPassw(),4) != 0);
		POBJDELETE(pResShort);
	}

	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res1") == STATUS_OK);
	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res2") == STATUS_OK);
	CPPUNIT_ASSERT(m_pCommResDB->Cancel("res3") == STATUS_OK);
}


void CTestCommResDB::TestGetEQOrigionResrvName()
{
  CCommRes res1;
  res1.SetName("Eq1");
  res1.SetDisplayName("Eq1");
  m_pCommResDB->Add(res1);
  
  ALLOCBUFFER(eqOnGoingName,H243_NAME_LEN);
  
  char * tmpName = 0;
  std::string s1="Eq1",s2;

  tmpName = "Eq1(123)";
  memset(eqOnGoingName,'\0',H243_NAME_LEN);
  strncpy(eqOnGoingName,tmpName,strlen(tmpName));
  s2 = (m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,123) ? m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,123) : "");
  CPPUNIT_ASSERT(s1 == s2);
  
  
  tmpName = "Eq1(44444)";
  memset(eqOnGoingName,'\0',H243_NAME_LEN);
  strncpy(eqOnGoingName,tmpName,strlen(tmpName));
  s2 = (m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,44444) ? m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,44444) : "") ;
  CPPUNIT_ASSERT(s1 == s2);

  tmpName = "Eq1(4546789)";
  memset(eqOnGoingName,'\0',H243_NAME_LEN);
  strncpy(eqOnGoingName,tmpName,strlen(tmpName));
  s2 = (m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,4546789) ? m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,4546789) : "");
  CPPUNIT_ASSERT(s1 == s2);

  tmpName = "Eq1(987656543)";
  memset(eqOnGoingName,'\0',H243_NAME_LEN);
  strncpy(eqOnGoingName,tmpName,strlen(tmpName));
  s2 =(m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,987656543) ? m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,987656543) : "");
  CPPUNIT_ASSERT(s1 == s2);

  tmpName = "Eq1(1)";
  memset(eqOnGoingName,'\0',H243_NAME_LEN);
  strncpy(eqOnGoingName,tmpName,strlen(tmpName));
  s2 = (m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,1) ? m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,1) : "");
  CPPUNIT_ASSERT(s1 == s2);
  
  tmpName = "Eq1(11)";
  memset(eqOnGoingName,'\0',H243_NAME_LEN);
  strncpy(eqOnGoingName,tmpName,strlen(tmpName));
  s2 = (m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,11) ? m_pCommResDB->GetOrigionEqReservationName(eqOnGoingName,11) : "");
  CPPUNIT_ASSERT(s1 == s2);

  CPPUNIT_ASSERT(m_pCommResDB->Cancel("Eq1") == STATUS_OK);
  DEALLOCBUFFER(eqOnGoingName);
  
}

