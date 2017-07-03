
#ifndef TEST_MOCK_CCONF_H
#define  TEST_MOCK_CCONF_H

#include <cppunit/extensions/HelperMacros.h>

#include "ConfApi.h"

 
class CMockConfApi : public CConfApi
{
public:
	CMockConfApi();
	~CMockConfApi();


	virtual void  DropParty(const char* partyName,WORD mode = 0,WORD discCause = 0);
	virtual void  ReconnectParty(const char* partyName, BYTE bUseDelay = 1);
	
	
	virtual void  ObserverUpdate(WORD event, WORD type, DWORD observerInfo1, DWORD val, void* pPointer);

	/*virtual void SipSubscribeConfirm(int connId, WORD expires);
	virtual void SipSubscribeReject(int connId);
	virtual void SipNotify(int connId, char* state);
	
	void Verify_SipSubscribeConfirm_WasCalled(int *connId = NULL, WORD *expires = NULL);
	void Verify_SipSubscribeReject_WasCalled(int *connId = NULL);
	void Verify_SipNotify_WasCalled(int *connId, char* pState);
	void Verify_SipNotify_WasNotCalled(int *connId, char* pState);*/

	void Verify_ReconnectParty_WasCalled(char* pUserName);
	void Verify_DropParty_WasCalled(char* pUserName, WORD* discCause);
	void Verify_ObserverUpdate_WasCalled(COsQueue** pQueue, WORD* event, WORD* type, DWORD* observerInfo1, DWORD* val, WORD* count);
	void Verify_ObserverUpdate_WasNotCalled();

protected:
	
	BYTE mm_reconnectParty, mm_dropParty;
	WORD m_expires, mm_event, mm_type, mm_observerUpdate_count, mm_discCause;
	DWORD mm_val, mm_observerInfo1;
	int  m_connId;
	char *m_pState;
	char *mm_pUserName;
	COsQueue* mm_pQueue;
};



#endif //TEST_MOCK_CCONF_H
