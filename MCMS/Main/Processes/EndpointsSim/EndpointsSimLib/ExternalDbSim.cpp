//////////////////////////////////////////////////////////////////////
//
// ExternalDbSim.cpp: implementation of the external Database classes.
//
//////////////////////////////////////////////////////////////////////


#include "StatusesGeneral.h"
#include "OperCfg.h"
#include "Segment.h"
#include "CommEndpointsSimSet.h"
#include "ExternalDbSim.h"


const STATUS STATUS_EXT_DB_OK           = 0;
const STATUS STATUS_EXT_DB_ILL_USER_PWD = 1;
const STATUS STATUS_EXT_DB_ILL_NID      = 3;
const STATUS STATUS_EXT_DB_ILL_CLI      = 4;
const STATUS STATUS_EXT_DB_ILL_CLI_NID  = 5;
const STATUS STATUS_EXT_DB_INTERNAL_ERR = 6;

const STATUS STATUS_EXT_DB_ILL_USER_OR_PWD = 8;


/////////////////////////////////////////////////////////////////////////////
//    CExtDbSimulator
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CExtDbSimulator::CExtDbSimulator()
{
	m_connectionStatus = 1; // not connected
    
	m_records[0].Danny();
	m_records[1].Amir();
	m_records[2].Yuri();
	m_records[3].Anat();
	m_records[4].Vasily();
    m_records[5].Da_UTF8();
    m_records[6].ExAscii();
    m_records[7].Judith();
}

/////////////////////////////////////////////////////////////////////////////
CExtDbSimulator::~CExtDbSimulator()
{
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbSimulator::Serialize(CSegment& segment) const
{
	segment << (DWORD) MAX_ETX_DB_RECORDS;

	for( int i=0; i<MAX_ETX_DB_RECORDS; i++ )
		m_records[i].Serialize(segment);
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbSimulator::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
/*	// create <SYSTEM_DETAILS> section
	CXMLDOMElement* pSimDetailsNode = pFatherNode->AddChildNode("SYSTEM_DETAILS");
	if( NULL == pSimDetailsNode )
		return;

	// <SYSTEM_DETAILS> fields
	pSimDetailsNode->AddChildNode("CS_API_IP_ADDRESS",m_szCsApiIp);
	pSimDetailsNode->AddChildNode("CS_API_PORT", m_wCsApiPortNumber);
	pSimDetailsNode->AddChildNode("GUI_PORT", m_wGuiPortNumber);
	pSimDetailsNode->AddChildNode("ENCRYPT_DIAL_IN", m_bEncryptionDialIn,_BOOL);

	// create <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> section
	CXMLDOMElement* pRejectNode = pSimDetailsNode->AddChildNode("REJECT_DIAL_OUT");
	if( NULL != pRejectNode )
	{
		// <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> fields
		pRejectNode->AddChildNode("DIAL_OUT_STATUS", m_rejectStatus);
		pRejectNode->AddChildNode("REDIRECTION_SIP_ADDRESS",m_szRedirectionSipAddress);
	}
	pSimDetailsNode->AddChildNode("DELETE_DIAL_OUT_AFTER_DISCONNECT", m_bDeleteDialOut,_BOOL);*/
}

/////////////////////////////////////////////////////////////////////////////
int	CExtDbSimulator::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int		nStatus			= STATUS_OK;
/*	char*	pszChildName	= NULL;

	// get <SYSTEM_DETAILS> section
	CXMLDOMElement*		pSimDetailsNode = NULL;
	GET_CHILD_NODE(pActionNode,"SYSTEM_DETAILS",pSimDetailsNode);

	// get fields from <SYSTEM_DETAILS> section
	if( pSimDetailsNode ) {
		GET_VALIDATE_CHILD(pSimDetailsNode,"CS_API_IP_ADDRESS",m_szCsApiIp,_1_TO_IP_ADDRESS_LENGTH);
		GET_VALIDATE_CHILD(pSimDetailsNode,"CS_API_PORT",&m_wCsApiPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"GUI_PORT",&m_wGuiPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"ENCRYPT_DIAL_IN",&m_bEncryptionDialIn,_BOOL);

		// get <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> section
		CXMLDOMElement*		pRejectNode = NULL;
		GET_CHILD_NODE(pSimDetailsNode,"REJECT_DIAL_OUT",pRejectNode);

		if( pRejectNode )
		{
			GET_VALIDATE_CHILD(pRejectNode,"DIAL_OUT_STATUS",&m_rejectStatus,_0_TO_DWORD);
			GET_VALIDATE_CHILD(pRejectNode,"REDIRECTION_SIP_ADDRESS",m_szRedirectionSipAddress,_1_TO_H243_NAME_LENGTH);
		}

		GET_VALIDATE_CHILD(pSimDetailsNode,"DELETE_DIAL_OUT_AFTER_DISCONNECT",&m_bDeleteDialOut,_BOOL);
	}
*/
	return nStatus;	
}

/////////////////////////////////////////////////////////////////////////////
STATUS CExtDbSimulator::ProcessRequest(CCommSetExternalDbCreate* pReqCreate) const
{
	for( int i=0; i<MAX_ETX_DB_RECORDS; i++ )
	{
		if( 0 == strcmp(pReqCreate->GetNumericId(),m_records[i].GetNumericId()) )
		{
			pReqCreate->SetName(m_records[i].GetName());
			pReqCreate->SetMaxParties(m_records[i].GetMaxParties());
			pReqCreate->SetMinParties(m_records[i].GetMinParties());
			pReqCreate->SetPassword(m_records[i].GetChairPwd());
			pReqCreate->SetEntryPassword(m_records[i].GetEntryPwd());
			pReqCreate->SetBillingData(m_records[i].GetBillingData());
			pReqCreate->SetOwner(m_records[i].GetOwner());
			for( int j=0; j<3; j++ )
				pReqCreate->SetContactInfo(j,m_records[i].GetInfo(j));
			pReqCreate->SetDisplayName(m_records[i].GetDisplayName());

			return STATUS_OK;
		}
	}

	return STATUS_EXT_DB_ILL_NID;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CExtDbSimulator::ProcessRequest(CCommSetExternalDbAdd* pReqAdd) const
{
	for( int i=0; i<MAX_ETX_DB_RECORDS; i++ )
	{
		if( 0 == strcmp(pReqAdd->GetNumericId(),m_records[i].GetNumericId()) )
		{
			pReqAdd->SetName(m_records[i].GetName());
			pReqAdd->SetLeader(m_records[i].GetIsLeader());
			pReqAdd->SetVip(m_records[i].GetIsVip());
			for( int j=0; j<4; j++ )
				pReqAdd->SetUserInfo(j,m_records[i].GetInfo(j));

			return STATUS_OK;
		}
	}

	return STATUS_EXT_DB_ILL_NID;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CExtDbSimulator::ProcessRequest(CCommSetExternalDbUser* pReqUser) const
{
	struct {
		char name[H243_NAME_LEN];
		char pwd[H243_NAME_LEN];
		WORD group;
	}  atNightTestsUsers[] = 
	{
		{"Night_test_user" ,"Night_test_user" ,ORDINARY},
		{"Night_test_admin","Night_test_admin",SUPER},
		{"Night_test_prem" ,"Night_test_prem" ,SUPER},
	};

	for( int i=0; i<(int)(sizeof(atNightTestsUsers)/sizeof(atNightTestsUsers[0])); i++ )
	{
		if( 0 == strcmp(pReqUser->GetUserName(),atNightTestsUsers[i].name) && 
			0 == strcmp(pReqUser->GetUserPassword(),atNightTestsUsers[i].pwd) )
		{
			pReqUser->SetGroup(atNightTestsUsers[i].group);
			return STATUS_OK;
		}
	}

	return STATUS_EXT_DB_ILL_USER_OR_PWD;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbSimulator::KeepAlive()
{
	m_connectionStatus = 0; // connected
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbSimulator::KeepAliveTimer()
{
	m_connectionStatus = 1; // not connected
}


/////////////////////////////////////////////////////////////////////////////
//    CExtDbRecord
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CExtDbRecord::CExtDbRecord()
{
	memset(m_szNumericId,0,H243_NAME_LEN);
	memset(m_szName,0,H243_NAME_LEN);
	memset(m_szChairPassword,0,H243_NAME_LEN);
	memset(m_szEntryPassword,0,H243_NAME_LEN);
	memset(m_szBillingData,0,H243_NAME_LEN);
	memset(m_szOwner,0,H243_NAME_LEN);
	memset(m_szPhone1,0,H243_NAME_LEN);
	memset(m_szPhone2,0,H243_NAME_LEN);
	for( int i=0; i<4; i++ )
		memset(m_aszInfo[i],0,H243_NAME_LEN);
	memset(m_szDisplayName,0,H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 25;
	m_bIsLeader = false;
	m_bIsGuest  = false;
	m_bIsVip    = false;
}

/////////////////////////////////////////////////////////////////////////////
CExtDbRecord::~CExtDbRecord()
{
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Serialize(CSegment& segment) const
{
	segment	<< m_szNumericId
			<< m_szName
			<< m_wMinParties
			<< m_wMaxParties
			<< m_szChairPassword
			<< m_szEntryPassword
			<< (DWORD)m_bIsLeader
			<< (DWORD)m_bIsGuest
			<< (DWORD)m_bIsVip
			<< m_szBillingData
			<< m_szOwner
			<< m_szDisplayName;

//	segment	<< m_szPhone1
//			<< m_szPhone2;
//
//	for( int i=0; i<4; i++ )
//		segment	<< m_aszInfo[i];
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
/*	// create <SYSTEM_DETAILS> section
	CXMLDOMElement* pSimDetailsNode = pFatherNode->AddChildNode("SYSTEM_DETAILS");
	if( NULL == pSimDetailsNode )
		return;

	pSimDetailsNode->AddChildNode("DELETE_DIAL_OUT_AFTER_DISCONNECT", m_bDeleteDialOut,_BOOL);*/
}

/////////////////////////////////////////////////////////////////////////////
int	CExtDbRecord::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int		nStatus			= STATUS_OK;
/*	char*	pszChildName	= NULL;

	// get <SYSTEM_DETAILS> section
	CXMLDOMElement*		pSimDetailsNode = NULL;
	GET_CHILD_NODE(pActionNode,"SYSTEM_DETAILS",pSimDetailsNode);

	// get fields from <SYSTEM_DETAILS> section
	if( pSimDetailsNode ) {
		GET_VALIDATE_CHILD(pSimDetailsNode,"CS_API_IP_ADDRESS",m_szCsApiIp,_1_TO_IP_ADDRESS_LENGTH);

		// get <SYSTEM_DETAILS>::<REJECT_DIAL_OUT> section
		CXMLDOMElement*		pRejectNode = NULL;
		GET_CHILD_NODE(pSimDetailsNode,"REJECT_DIAL_OUT",pRejectNode);

		if( pRejectNode )
		{
			GET_VALIDATE_CHILD(pRejectNode,"REDIRECTION_SIP_ADDRESS",m_szRedirectionSipAddress,_1_TO_H243_NAME_LENGTH);
		}
	}
*/
	return nStatus;	
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Danny()
{
	strncpy(m_szNumericId,"91001",H243_NAME_LEN);
	strncpy(m_szName,"Danny",H243_NAME_LEN);
	strncpy(m_szDisplayName, "Danny",H243_NAME_LEN);
	strncpy(m_szChairPassword,"1111",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"9999",H243_NAME_LEN);
	strncpy(m_szBillingData,"Danny Bibi",H243_NAME_LEN);

	strncpy(m_aszInfo[0],"Info: Line # 1",H243_NAME_LEN);
	strncpy(m_aszInfo[1],"Info: Line # 2",H243_NAME_LEN);
	strncpy(m_aszInfo[2],"Info: Line # 3",H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 25;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = true;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Amir()
{ 
	strncpy(m_szNumericId,"91002",H243_NAME_LEN);
	strncpy(m_szName,"Amir",H243_NAME_LEN);
	strncpy(m_szDisplayName,"Amir",H243_NAME_LEN);
	strncpy(m_szChairPassword,"2222",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"8888",H243_NAME_LEN);
	strncpy(m_szBillingData,"Amir Kaplan",H243_NAME_LEN);

	m_wMinParties = 1;
	m_wMaxParties = 20;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = true;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Yuri()
{
	strncpy(m_szNumericId,"91003",H243_NAME_LEN);
	strncpy(m_szName,"Yuri",H243_NAME_LEN);
	strncpy(m_szDisplayName,"Yuri",H243_NAME_LEN);
	strncpy(m_szChairPassword,"3333",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"7777",H243_NAME_LEN);
	strncpy(m_szBillingData,"Yuri Wolberg",H243_NAME_LEN);
	strncpy(m_szOwner,"Danny Bibi",H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 25;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = false;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Anat()
{
	strncpy(m_szNumericId,"91004",H243_NAME_LEN);
	strncpy(m_szName,"Anat",H243_NAME_LEN);
	strncpy(m_szDisplayName,"Anat",H243_NAME_LEN);
	strncpy(m_szChairPassword,"4444",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"6666",H243_NAME_LEN);
	strncpy(m_szBillingData,"Anat Gavish",H243_NAME_LEN);
	strncpy(m_szOwner,"Amir Kaplan",H243_NAME_LEN);

	strncpy(m_aszInfo[0],"Info: Line # 1",H243_NAME_LEN);
	strncpy(m_aszInfo[1],"Info: Line # 2",H243_NAME_LEN);
	strncpy(m_aszInfo[2],"Info: Line # 3",H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 15;
	m_bIsLeader = false;
	m_bIsGuest  = true;
	m_bIsVip    = false;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Vasily()
{
	strncpy(m_szNumericId,"91005",H243_NAME_LEN);
	strncpy(m_szName,"Vasily",H243_NAME_LEN);
	strncpy(m_szDisplayName,"Vasily",H243_NAME_LEN);
	strncpy(m_szChairPassword,"5555",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"0000",H243_NAME_LEN);
	strncpy(m_szBillingData,"Vasily Bondarenko",H243_NAME_LEN);
	strncpy(m_szOwner,"Amir Kaplan",H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 10;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = true;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Da_UTF8()
{
	strncpy(m_szNumericId,"91006",H243_NAME_LEN);

    // it represents DA in russian UTF-8
    char buffer [H243_NAME_LEN];
    int index = 0;
    buffer[index++] = 0xd0;
    buffer[index++] = 0x94;  // D
    buffer[index++] = 0xd0;
    buffer[index++] = 0x90;  // A
    buffer[index++] = '\0';
    
    strncpy(m_szDisplayName, buffer,H243_NAME_LEN);	//conference name
    
    strncpy(m_szName, buffer,H243_NAME_LEN);		//participant name

    strncpy(m_szChairPassword,"5555",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"0000",H243_NAME_LEN);
	strncpy(m_szBillingData,"Cucu Lulu",H243_NAME_LEN);
	strncpy(m_szOwner,"Yuri Ratner",H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 10;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = true;
}

/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::ExAscii()
{
    strncpy(m_szNumericId,"91007",H243_NAME_LEN);

    // it represents DA in russian ISO/IEC 8859-5
    char buffer [H243_NAME_LEN];
    int index = 0;
    buffer[index++] = 0xb4;  // D
    buffer[index++] = 0xb0;  // A
    buffer[index++] = '\0';
    
    strncpy(m_szDisplayName, buffer,H243_NAME_LEN);	//conference name    
    
    strncpy(m_szName, buffer,H243_NAME_LEN);		//user name

    strncpy(m_szChairPassword,"5555",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"0000",H243_NAME_LEN);
	strncpy(m_szBillingData,"Cucu Lulu",H243_NAME_LEN);
	strncpy(m_szOwner,"Yuri Ratner",H243_NAME_LEN);

	m_wMinParties = 0;
	m_wMaxParties = 10;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = true;
}
/////////////////////////////////////////////////////////////////////////////
void CExtDbRecord::Judith()
{
    char buffer [H243_NAME_LEN];
    int index = 0;
    buffer[index++] = 0xd0;
    buffer[index++] = 0x94;  // D
    buffer[index++] = 0xd0;
    buffer[index++] = 0x90;  // A
    buffer[index++] = '\0';
    
    strncpy(m_szDisplayName, buffer,H243_NAME_LEN);
	strncpy(m_szNumericId,"91008",H243_NAME_LEN);
	strncpy(m_szName,"Judith",H243_NAME_LEN);
	strncpy(m_szChairPassword,"2525",H243_NAME_LEN);
	strncpy(m_szEntryPassword,"2626",H243_NAME_LEN);
	strncpy(m_szBillingData,"Judith Shuva",H243_NAME_LEN);

	m_wMinParties = 1;
	m_wMaxParties = 20;
	m_bIsLeader = true;
	m_bIsGuest  = false;
	m_bIsVip    = true;
}
/////////////////////////////////////////////////////////////////////////////

