// RsrvPartyAction.cpp: implementation of the CRsrvPartyAction class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Party
//========   ==============   =====================================================================


#include "RsrvPartyAction.h"
#include "StatusesGeneral.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "ProcessBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRsrvPartyAction::CRsrvPartyAction()
{
	m_ConfID 	= 0xFFFFFFFF;
	m_PartyID 	= 0xFFFFFFFF;
	m_partyName[0] = '\0';	
	for (int i=0;i<MAX_USER_INFO_ITEMS;i++)
		m_contact_info_list[i][0]='\0';	
	m_AdditionalInfo[0] = '\0';	
	m_NumAction = UNKNOWN_ACTION;
	m_param1 	= 0;
	m_param2 	= 0;
}

//////////////////////////////////////////////////////////////////////////////
CRsrvPartyAction& CRsrvPartyAction::operator = (const CRsrvPartyAction &other)
{
	m_ConfID = other.m_ConfID;
	m_PartyID = other.m_PartyID;
	strncpy (m_partyName, other.m_partyName,H243_NAME_LEN);
    strncpy (m_AdditionalInfo, other.m_AdditionalInfo, H243_NAME_LEN);
	for (int i = 0; i<MAX_USER_INFO_ITEMS; i++)
	{		
		strncpy(m_contact_info_list[i],other.m_contact_info_list[i],H243_NAME_LEN);		
	}		
	
	m_NumAction = other.m_NumAction;
	m_param1	= other.m_param1;
	m_param2	= other.m_param2;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRsrvPartyAction::~CRsrvPartyAction()
{

}
/////////////////////////////////////////////////////////////////////////////
int CRsrvPartyAction::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;

	PASSERTSTREAM(pResNode == NULL, "Input pResNode shouldn't be NULL.");

	GET_VALIDATE_CHILD(pResNode,"ID",&m_ConfID,_0_TO_DWORD);

	if(!strncmp("DELETE_PARTY",strAction,12))
	{
		m_NumAction=DEL_PARTY;
	}
	else if(!strncmp("SET_CONNECT",strAction,11))
	{
		BYTE bConnect;
		GET_VALIDATE_MANDATORY_CHILD(pResNode,"CONNECT",&bConnect,_BOOL);
		
		if(bConnect)
			m_NumAction = RECONNECT_PARTY;
		else
			m_NumAction = DISCONNECT_PARTY;
	}
	else if(!strncmp("SET_PARTY_LAYOUT_TYPE",strAction,21))
	{
		m_NumAction = SET_PARTY_CONF_OR_PRIVATE_LAYOUT;
		
		int nVal;
		GET_VALIDATE_MANDATORY_CHILD(pResNode,"LAYOUT_TYPE",&nVal,LAYOUT_TYPE_ENUM);
		
		if(nVal) // Personal
			m_param1 = YES;
		else	 // Conference
			m_param1 = NO;
	}
	else if(!strncmp("SET_AUDIO_VOLUME",strAction,16))
	{
		m_NumAction = SET_AUDIO_VOLUME;
		
		BYTE volume;
		GET_VALIDATE_MANDATORY_CHILD(pResNode,"VOLUME",&volume,_1_TO_10_DECIMAL);
	
		m_param1 = volume;
	}
	else if(!strncmp("SET_LISTEN_AUDIO_VOLUME",strAction,23))
	{
		m_NumAction = SET_LISTENING_AUDIO_VOLUME;
		
		BYTE volume;
		GET_VALIDATE_MANDATORY_CHILD(pResNode,"LISTEN_VOLUME",&volume,_1_TO_10_DECIMAL);
	
		m_param1 = volume;
	}
	else if(!strncmp("SET_AUDIO_VIDEO_MUTE",strAction,20))
	{
		m_NumAction = BLOCK_PARTY;
		
		WORD bMute = 0;

		GET_VALIDATE_CHILD(pResNode,"AUDIO_MUTE",&bMute,_BOOL);

		m_param1 = bMute;	
		
		GET_VALIDATE_CHILD(pResNode,"VIDEO_MUTE",&bMute,_BOOL);

		m_param2 = bMute;	
	}
	else if(!strncmp("SET_AUDIO_BLOCK",strAction,15))
	{
		m_NumAction = BLOCK_NOT_MUTE_PARTY;
		
		WORD bBlock = 0;

		GET_VALIDATE_CHILD(pResNode,"AUDIO_BLOCK",&bBlock,_BOOL);

		m_param1 = bBlock;	
	}	
	else if(!strncmp("SET_AGC",strAction,7))
	{
		m_NumAction = SET_AGC;
		
		WORD bAGC = 0;

		GET_VALIDATE_CHILD(pResNode,"AGC",&bAGC,_BOOL);

		m_param1 = bAGC;	
	}	
	else if(!strncmp("SET_PARTY_VISUAL_NAME",strAction,21))
	{
		GET_VALIDATE_CHILD(pResNode,"NAME",m_partyName,_1_TO_H243_NAME_LENGTH);		
	}
	else if(!strncmp("SET_PARTY_CONTACT_INFO",strAction,22))
	{
		CXMLDOMElement *pChildNode;
		GET_CHILD_NODE(pResNode, "CONTACT_INFO_LIST", pChildNode);
		if(pChildNode)
		{
			
			//pChildNode->ResetChildList();	
			char    	contact_info_0[H243_NAME_LEN];
			memset(contact_info_0,'\0',H243_NAME_LEN);
			char    	contact_info_1[H243_NAME_LEN];
			memset(contact_info_1,'\0',H243_NAME_LEN);
			char    	contact_info_2[H243_NAME_LEN];
			memset(contact_info_2,'\0',H243_NAME_LEN);
			char    	contact_info_3[H243_NAME_LEN];
			memset(contact_info_3,'\0',H243_NAME_LEN);
            char    	additionalInfo[H243_NAME_LEN];
			memset(additionalInfo,'\0',H243_NAME_LEN);
			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO",contact_info_0,_0_TO_H243_NAME_LENGTH);
			//if (contact_info_0)
				strncpy(m_contact_info_list[0],contact_info_0,H243_NAME_LEN);

			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO_2",contact_info_1,_0_TO_H243_NAME_LENGTH);
			//if (contact_info_1)
				strncpy(m_contact_info_list[1],contact_info_1,H243_NAME_LEN);

			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO_3",contact_info_2,_0_TO_H243_NAME_LENGTH);
			//if (contact_info_2)
				strncpy(m_contact_info_list[2],contact_info_2,H243_NAME_LEN);		
			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO_4",contact_info_3,_0_TO_H243_NAME_LENGTH);
			//if (contact_info_3)
				strncpy(m_contact_info_list[3],contact_info_3,H243_NAME_LEN);
			GET_VALIDATE_CHILD(pChildNode,"ADDITIONAL_INFO", additionalInfo,_0_TO_H243_NAME_LENGTH);
			//if (contact_info_3)
				strncpy(m_AdditionalInfo, additionalInfo,H243_NAME_LEN);	
			
		}
	}
	else if(!strncmp("SET_LEADER",strAction,10))
	{
		m_NumAction = SET_PARTY_AS_LEADER;
		
		WORD bLeader = 0;
		GET_VALIDATE_CHILD(pResNode,"LEADER",&bLeader,_BOOL);
		m_param1 = bLeader;	
	}
	else if(!strncmp("REQUEST_INTRA",strAction,13))
	{
		WORD intraDirection = 0;
		GET_VALIDATE_CHILD(pResNode,"INTRA_DIRECTION",&intraDirection,INTRA_REQUEST_ENUM);
		m_NumAction = intraDirection;
	}
	else if((!strncmp("START_CONTENT",strAction,10)) &&
			(CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator))
	{
		m_NumAction = START_CONTENT;
			
	}
	else if((!strncmp("STOP_CONTENT",strAction,10)) &&
			(CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator))
	{
		m_NumAction = STOP_CONTENT;
				
	}
		
	
	
	if(!strncmp("STOP_PREVIEW",strAction,12) )
	{
		int direction;
	//	GET_VALIDATE_MANDATORY_CHILD(pResNode,"DIRECTION_TYPE",&direction,DIRECTION_TYPE_ENUM);
		GET_VALIDATE_MANDATORY_CHILD(pResNode,"VIDEO_DIRECTION",&direction,DIRECTION_TYPE_ENUM);

		m_param1 = direction;
	}

	if(!strncmp("REQUEST_INTRA",strAction,13))
	{
		int direction;
	//	GET_VALIDATE_MANDATORY_CHILD(pResNode,"DIRECTION_TYPE",&direction,DIRECTION_TYPE_ENUM);
		GET_VALIDATE_MANDATORY_CHILD(pResNode,"INTRA_DIRECTION",&direction,DIRECTION_TYPE_ENUM);
		m_param1 = direction;
	}

	GET_VALIDATE_MANDATORY_CHILD(pResNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);

	return nStatus;
}

/*
////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvPartyAction::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("DELETE_PARTY",strAction,12))
		numAction=DEL_PARTY;
	
	return numAction;
}


*/


///////////////////////////////////////////////////////////////////////////
void CRsrvPartyAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}


//////////////////////////////////////////////////////////////////////////
DWORD  CRsrvPartyAction::GetConfID()
{
	return m_ConfID;
}


//////////////////////////////////////////////////////////////////////////
DWORD  CRsrvPartyAction::GetPartyID()
{
	return m_PartyID;
}

//////////////////////////////////////////////////////////////////////////
void CRsrvPartyAction::SetConfID(DWORD confId)
{
	m_ConfID = confId;
}

//////////////////////////////////////////////////////////////////////////
void CRsrvPartyAction::SetPartyID(DWORD partyId)
{
	m_PartyID = partyId;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRsrvPartyAction::GetName()
{
    return m_partyName;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRsrvPartyAction::GetContactInfo(int contactNum)
{
    return m_contact_info_list[contactNum];
}

/////////////////////////////////////////////////////////////////////////////
const char* CRsrvPartyAction::GetAddionalInfo()
{
    return m_AdditionalInfo;
}
//////////////////////////////////////////////////////////////////////////
int  CRsrvPartyAction::GetNumAction()
{
	return m_NumAction;
}

//////////////////////////////////////////////////////////////////////////
int  CRsrvPartyAction::GetParam1()
{
	return m_param1;
}

//////////////////////////////////////////////////////////////////////////
int  CRsrvPartyAction::GetParam2()
{
	return m_param2;
}

//////////////////////////////////////////////////////////////////////////
void  CRsrvPartyAction::SetNumAction(int nAction)
{
	m_NumAction = nAction;
}
