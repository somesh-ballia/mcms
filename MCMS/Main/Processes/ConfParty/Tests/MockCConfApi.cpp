


#include "MockCConfApi.h"

////////////////////////////////////////////////////////////////////////
CMockConfApi::CMockConfApi()
{
	mm_reconnectParty = FALSE;
	mm_dropParty = FALSE;
	mm_discCause = 0;
	mm_observerUpdate_count = 0;
	m_connId = 0;
	m_expires = 0;
	mm_event = 0;
	m_type = 0;
	mm_val = 0;
	mm_observerInfo1 = 0;
	m_pState = new char[H243_NAME_LEN];
	mm_pUserName = new char[H243_NAME_LEN];
	mm_pQueue = NULL;
	
}

////////////////////////////////////////////////////////////////////////
CMockConfApi::~CMockConfApi()
{
	delete[] m_pState;
	delete[] mm_pUserName;
}



////////////////////////////////////////////////////////////////////////	
void CMockConfApi::ReconnectParty(const char* partyName, BYTE bUseDelay)
{
	mm_reconnectParty = TRUE;
	strncpy(mm_pUserName, partyName, H243_NAME_LEN);
}

/////////////////////////////////////////////////////////////////////////////
void  CMockConfApi::DropParty(const char* partyName,WORD mode,WORD discCause)  
{ 
	mm_dropParty = TRUE;
	strncpy(mm_pUserName, partyName, H243_NAME_LEN);
	mm_discCause = discCause;
}

////////////////////////////////////////////////////////////////////////	
void CMockConfApi::ObserverUpdate(WORD event, WORD type, DWORD observerInfo1, DWORD val, void* pPointer)
{
	mm_observerUpdate_count++;
	mm_event = event;
	mm_val = val;
	mm_type = type;
	mm_observerInfo1 = observerInfo1;
	mm_pQueue = (COsQueue*)pPointer;
}

////////////////////////////////////////////////////////////////////////	
void CMockConfApi::Verify_ReconnectParty_WasCalled(char* pUserName)
{
	CPPUNIT_ASSERT_MESSAGE("CMockConfApi::ReconnectParty has not been used", mm_reconnectParty );
	if(pUserName)
		strncpy(pUserName, mm_pUserName, H243_NAME_LEN);
	mm_reconnectParty = FALSE;
}

////////////////////////////////////////////////////////////////////////	
void CMockConfApi::Verify_DropParty_WasCalled(char* pUserName, WORD* discCause)
{
	CPPUNIT_ASSERT_MESSAGE("CMockConfApi::DropParty has not been used", mm_dropParty );
	if(pUserName)
		strncpy(pUserName, mm_pUserName, H243_NAME_LEN);
	mm_dropParty = FALSE;
	*discCause = mm_discCause;
}

////////////////////////////////////////////////////////////////////////	
void CMockConfApi::Verify_ObserverUpdate_WasCalled(COsQueue** pQueue, WORD* event, WORD* type, DWORD* observerInfo1, DWORD* val, WORD* count)
{
	CPPUNIT_ASSERT_MESSAGE("CMockConfApi::ObserverUpdate has not been used", mm_observerUpdate_count > 0);
	*count = mm_observerUpdate_count;
	mm_observerUpdate_count = 0;
	*pQueue = mm_pQueue;
	*event = mm_event;
	*val = mm_val;
	*type = mm_type;
	*observerInfo1 = mm_observerInfo1;
}

////////////////////////////////////////////////////////////////////////	
void CMockConfApi::Verify_ObserverUpdate_WasNotCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockConfApi::ObserverUpdate was used", mm_observerUpdate_count == 0);
}

