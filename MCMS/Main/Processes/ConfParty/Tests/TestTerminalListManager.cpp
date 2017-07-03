#include "TestTerminalListManager.h"
#include "StatusesGeneral.h"
#include "Party.h"
#include "TerminalListManager.h"
#include <iostream>
#include <algorithm>

CPPUNIT_TEST_SUITE_REGISTRATION(CTestTerminalListManager);
 

void  CTestTerminalListManager::setUp()
{
  m_pTerminalListManager = new CTerminalNumberingManager();
}

void CTestTerminalListManager::tearDown()
{
  POBJDELETE(m_pTerminalListManager);
}

void CTestTerminalListManager::TestCtor()
{
  CPPUNIT_ASSERT_MESSAGE("CTestTerminalListManager::TestCtor",m_pTerminalListManager->length() == 0);
}

void CTestTerminalListManager::TestListAdd()
{
  WORD mcuNumber=0,partyTerminalNumber=0;
  CParty party1,party2,party3,party4;
  party1.SetFullName("Party1","Conf1");
  party2.SetFullName("Party2","Conf1");
  party3.SetFullName("Party3","Conf1");
  party4.SetFullName("Party4","Conf1");

  PartiesVector vect1;
  vect1.push_back(&party1);
  vect1.push_back(&party2);
  vect1.push_back(&party3);
  vect1.push_back(&party4);

  TERMINALLIST * tetminalList = m_pTerminalListManager->getPartyListPtr();
  
  for ( PartiesVector::iterator it = vect1.begin(); it != vect1.end() ; ++it){
    m_pTerminalListManager->allocatePartyNumber(*it,mcuNumber,partyTerminalNumber);
  }
  
  CPPUNIT_ASSERT_MESSAGE("CTestTerminalListManager::TestListAdd",m_pTerminalListManager->length() == 4);
  
  TERMINALLIST::iterator itr = tetminalList->begin();
  PartiesVector::iterator it = vect1.begin();
  
  for( int i=0; itr != tetminalList->end();++itr,++i,++it)
    {
      CPPUNIT_ASSERT_MESSAGE("CTestTerminalListManager::TestListAdd compare party id",(*itr)->getPartyID() == (*it) );
      CPPUNIT_ASSERT_MESSAGE("CTestTerminalListManager::TestListAdd",(*itr)->getPartyNumber() == (i+1) );
    }
}

void CTestTerminalListManager::TestListRemove()
{
  CTerminalNumberingItem* pTmpTermNumber=0;
  WORD mcuNumber=0,partyTerminalNumber=0;
  CParty party1,party2,party3,party4;
  party1.SetFullName("Party1","Conf1");
  party2.SetFullName("Party2","Conf1");
  party3.SetFullName("Party3","Conf1");
  party4.SetFullName("Party4","Conf1");
  
  PartiesVector vect1;
  vect1.push_back(&party1);
  vect1.push_back(&party2);
  vect1.push_back(&party3);
  vect1.push_back(&party4);

  for ( PartiesVector::iterator it = vect1.begin(); it != vect1.end() ; ++it){
    m_pTerminalListManager->allocatePartyNumber(*it,mcuNumber,partyTerminalNumber);
  }

  //Remove Party2 and Party4
  m_pTerminalListManager->Remove(&party2);
  m_pTerminalListManager->Remove(&party4);
  CPPUNIT_ASSERT_MESSAGE("CTestTerminalListManager::TestListRemove check lenth after Remove",m_pTerminalListManager->length() == 2);

  CTerminalNumberingItem termNumberItem;
  termNumberItem.Create(1,1,&party1);
  pTmpTermNumber = m_pTerminalListManager->Find(&termNumberItem);
  CPPUNIT_ASSERT(pTmpTermNumber != 0);

  //Make sure second element was deleted
  termNumberItem.Create(mcuNumber,2,&party2);
  pTmpTermNumber= m_pTerminalListManager->Find(&termNumberItem);
  CPPUNIT_ASSERT(pTmpTermNumber == 0);

  //Make sure third element was deletedis in the list
  termNumberItem.Create(mcuNumber,3,&party3);
  pTmpTermNumber = m_pTerminalListManager->Find(&termNumberItem);
  CPPUNIT_ASSERT( pTmpTermNumber != 0 );

  //Make sure 4th element was deleted
  termNumberItem.Create(mcuNumber,4,&party4);
  pTmpTermNumber= m_pTerminalListManager->Find(&termNumberItem);
  CPPUNIT_ASSERT(pTmpTermNumber == 0);

  //Remove Party1 anf PArty3
  m_pTerminalListManager->Remove(&party3);
  m_pTerminalListManager->Remove(&party1);
  CPPUNIT_ASSERT_MESSAGE("CTestTerminalListManager::TestListRemove check lenth after Remove",m_pTerminalListManager->length() == 0);
  
}


void CTestTerminalListManager::TestAddRemove()
{
  CTerminalNumberingItem* pTmpTermNumber=0;
  WORD mcuNumber=0,partyTerminalNumber=0;
  
  PartiesVector vect1;
  CParty * pParty=0;
  std::string name;

  //Create 30 parties
  int numOfParties = 30;
  ALLOCBUFFER(partyName,H243_NAME_LEN);

  for (int i=0;i<numOfParties;++i)
    {
      memset(partyName,'\0',H243_NAME_LEN);
      sprintf(partyName,"_(%d)",i+1);
      pParty = new CParty;
      name="Party";
      name+=partyName;
      pParty->SetFullName(name.c_str(),"Conf1");
      vect1.push_back(pParty);
    }
  
  DEALLOCBUFFER(partyName);
  
  for ( PartiesVector::iterator it = vect1.begin(); it != vect1.end() ; ++it){
    m_pTerminalListManager->allocatePartyNumber(*it,mcuNumber,partyTerminalNumber);
  }

  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 30);

  //Remove Party 10 untill 20
  for (int i=10; i<20;++i)
    m_pTerminalListManager->Remove(vect1[i]);
  
  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 20);

  //Make sure all specified parties were deleted
  CTerminalNumberingItem termNumberItem;
  TERMINALLIST::iterator itr;
  for (int i=10; i<20;++i){
    termNumberItem.Create(mcuNumber,i+1,vect1[i]);
    pTmpTermNumber = m_pTerminalListManager->Find(&termNumberItem);
    CPPUNIT_ASSERT( pTmpTermNumber == 0 );
  }

  //Add the new parties make sure they get the former terminal numbers
  for (int i=10; i<20;++i){
    m_pTerminalListManager->allocatePartyNumber(vect1[i],mcuNumber,partyTerminalNumber);
    CPPUNIT_ASSERT(partyTerminalNumber == i+1);
  }

  //Size of the list is 30 again
  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 30);

   //Remove Party 0 untill 10
  for (int i=0; i<10;++i)
    m_pTerminalListManager->Remove(vect1[i]);
  
  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 20);

  //Make sure all specified parties were deleted
  for (int i=0; i<10;++i){
    termNumberItem.Create(mcuNumber,i+1,vect1[i]);
    pTmpTermNumber = m_pTerminalListManager->Find(&termNumberItem);
    CPPUNIT_ASSERT( pTmpTermNumber == 0 );
  }

  //Add the new parties make sure they get the former terminal numbers
  for (int i=0; i<10;++i){
    m_pTerminalListManager->allocatePartyNumber(vect1[i],mcuNumber,partyTerminalNumber);
    CPPUNIT_ASSERT(partyTerminalNumber == i+1);
  }

  //Size of the list is 30 again
  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 30);

  //Remove Party 20 untill 30
  for (int i=20; i<30;++i)
    m_pTerminalListManager->Remove(vect1[i]);
  
  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 20);

  //Make sure all specified parties were deleted
  for (int i=20; i<30;++i){
    termNumberItem.Create(mcuNumber,i+1,vect1[i]);
    pTmpTermNumber = m_pTerminalListManager->Find(&termNumberItem);
    CPPUNIT_ASSERT(pTmpTermNumber == 0);
  }

  //Add the new parties make sure they get the former terminal numbers
  for (int i=20; i<30;++i){
    m_pTerminalListManager->allocatePartyNumber(vect1[i],mcuNumber,partyTerminalNumber);
    CPPUNIT_ASSERT(partyTerminalNumber == i+1);
  }

  //Size of the list is 30 again
  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 30);

  //Clear all allocations
  for ( PartiesVector::iterator it = vect1.begin(); it != vect1.end() ; ++it)
    POBJDELETE(*it);
  
  vect1.clear();
}

void CTestTerminalListManager::TestAddRemoveRandom()
{
  CTerminalNumberingItem* pTmpTermNumber=0;
  WORD mcuNumber=0,partyTerminalNumber=0;
  
  PartiesVector vect1;
  CParty * pParty=0;
  std::string name;
  
  //Create 30 parties
  int numOfParties = 30;
  ALLOCBUFFER(partyName,H243_NAME_LEN);
  
  for (int i=0;i<numOfParties;++i)
    {
      memset(partyName,'\0',H243_NAME_LEN);
      sprintf(partyName,"_(%d)",i+1);
      pParty = new CParty;
      name="Party";
      name+=partyName;
      pParty->SetFullName(name.c_str(),"Conf1");
      vect1.push_back(pParty);
    }
  
  DEALLOCBUFFER(partyName);
  
  for ( PartiesVector::iterator it = vect1.begin(); it != vect1.end() ; ++it){
    m_pTerminalListManager->allocatePartyNumber(*it,mcuNumber,partyTerminalNumber);
  }

  CPPUNIT_ASSERT(m_pTerminalListManager->length() == 30);

  //Remove the following 7 parties, 3,17,9,21,11,6,8
  std::vector<int> partiesIndexVect;
  partiesIndexVect.push_back(3);// terminal id=4
  partiesIndexVect.push_back(17);// terminal id=18
  partiesIndexVect.push_back(9);// terminal id=10
  partiesIndexVect.push_back(21);// terminal id=22
  partiesIndexVect.push_back(11);// terminal id=12
  partiesIndexVect.push_back(6);// terminal id=7
  partiesIndexVect.push_back(8);// terminal id=9

  //Remove the parties in the above order
  for (std::vector<int>::iterator vectIt = partiesIndexVect.begin() ;vectIt != partiesIndexVect.end(); ++vectIt )
    m_pTerminalListManager->Remove(vect1[*vectIt]);

  std::sort(partiesIndexVect.begin(),partiesIndexVect.end());

  //Add the partiesin sorted mode in order to caheck later the terminal number of each one
  for (std::vector<int>::iterator vectIt = partiesIndexVect.begin() ; vectIt != partiesIndexVect.end(); ++vectIt )
    m_pTerminalListManager->allocatePartyNumber(vect1[*vectIt],mcuNumber,partyTerminalNumber);

  //Make sure all the parties got the right Terminal id
  for (int i=0;i<numOfParties;++i)
    { 
      CPPUNIT_ASSERT(m_pTerminalListManager->GetPartyTerminalNumber(vect1[i],mcuNumber,partyTerminalNumber) == STATUS_OK);
      CPPUNIT_ASSERT(1 == mcuNumber);
      CPPUNIT_ASSERT(i+1 == partyTerminalNumber);
    }
}
