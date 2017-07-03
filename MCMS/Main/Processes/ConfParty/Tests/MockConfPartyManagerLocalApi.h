
#ifndef TEST_MOCK_CMCUAPI_H
#define  TEST_MOCK_CMCUAPI_H

#include <cppunit/extensions/HelperMacros.h>

#include "ConfPartyManagerLocalApi.h"

 
class CMockConfPartyManagerLocalApi : public CConfPartyManagerLocalApi
{
public:
	CMockConfPartyManagerLocalApi();
	~CMockConfPartyManagerLocalApi();

	//virtual void CreateSipDialOutParty(DWORD wMonitorConfID, WORD listId, char* pPartyURI, WORD boardId, const char* pRefferedBy=NULL, char* pPartyName = NULL, const char* pMSAssociatedStr=NULL);
	virtual void CreateDialOutParty(DWORD wMonitorConfID, WORD listId, BYTE interfaceType, char* pPartyURI, BYTE isTel=FALSE, const char* pRefferedBy=NULL, const char* pReferUri=NULL,const char* pMSAssociatedStr=NULL);

	//void Verify_CreateSipDialOutParty_WasCalled(char* pPartyUri);
	void Verify_CreateDialOutParty_WasCalled(char* pPartyUri, BYTE &interfaceType);

protected:
	
	BYTE	mm_createSipDialOutParty, mm_createDialOutParty;
	BYTE	mm_interfaceType;
	char	*mm_pPartyUri, *mm_pPartyData;
	

};

#endif


