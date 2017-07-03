

#include "MockConfPartyManagerLocalApi.h"
#include "InterfaceType.h"

//////////////////////////////////////////////////////////////////////////
CMockConfPartyManagerLocalApi::CMockConfPartyManagerLocalApi()
{
	mm_createSipDialOutParty = FALSE;
	mm_createDialOutParty = FALSE;
	mm_interfaceType = SIP_INTERFACE_TYPE;
	mm_pPartyUri = new char[H243_NAME_LEN];
	mm_pPartyData = new char[H243_NAME_LEN];
}

//////////////////////////////////////////////////////////////////////////
CMockConfPartyManagerLocalApi::~CMockConfPartyManagerLocalApi()
{
	delete[] mm_pPartyUri;
	delete[] mm_pPartyData;
}

//////////////////////////////////////////////////////////////////////////
/*void CMockConfPartyManagerLocalApi::CreateSipDialOutParty(DWORD wMonitorConfID, WORD listId, char* pPartyURI, WORD boardId, const char* pRefferedBy, char* pPartyName, const char* pMSAssociatedStr)
{
	mm_createSipDialOutParty = TRUE;
	strncpy(mm_pPartyUri, pPartyURI, H243_NAME_LEN );
}
*/
//////////////////////////////////////////////////////////////////////////
void CMockConfPartyManagerLocalApi::CreateDialOutParty(DWORD wMonitorConfID, WORD listId, BYTE interfaceType, char* pPartyURI, BYTE isTel, const char* pRefferedBy, const char* pReferUri, const char* pMSAssociatedStr)
{
	mm_interfaceType = interfaceType;
	mm_createDialOutParty = TRUE;
	strncpy(mm_pPartyData, pPartyURI, H243_NAME_LEN );
}

//////////////////////////////////////////////////////////////////////////
/*void CMockConfPartyManagerLocalApi::Verify_CreateSipDialOutParty_WasCalled(char* pPartyUri)
{
	CPPUNIT_ASSERT_MESSAGE("CMockConfPartyManagerLocalApi, CreateSipDialOutParty has not been used", mm_createSipDialOutParty );

	mm_createSipDialOutParty = FALSE;
	if(pPartyUri!=NULL)
		strncpy(pPartyUri, mm_pPartyUri, H243_NAME_LEN);
}*/
	
//////////////////////////////////////////////////////////////////////////
void CMockConfPartyManagerLocalApi::Verify_CreateDialOutParty_WasCalled(char* pPartyUri, BYTE &interfaceType)
{
	CPPUNIT_ASSERT_MESSAGE("CMockConfPartyManagerLocalApi, Verify_CreateDialOutParty_WasCalled has not been used", mm_createDialOutParty );

	mm_createSipDialOutParty = FALSE;
	if(pPartyUri!=NULL)
		strncpy(pPartyUri, mm_pPartyData, H243_NAME_LEN);
	interfaceType = mm_interfaceType;
}
