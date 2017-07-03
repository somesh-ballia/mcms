// RsrvPartyAction.cpp: implementation of the CRsrvPartyAction class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for conf general actions
//========   ==============   =====================================================================


#include "ConfAction.h"
#include "ConfPartyDefines.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CConfAction::CConfAction()
{
  m_ConfID = 0xFFFFFFFF;
  m_NumAction = UNKNOWN_ACTION;
  m_NumAction1 = UNKNOWN_ACTION;
  m_NumAction2 = UNKNOWN_ACTION;
  m_billing_data[0] = '\0';
  for (int i=0;i<MAX_CONF_INFO_ITEMS;i++)
		m_contact_info_list[i][0]='\0';
  m_entry_password[0] = '\0';
  memset( m_chairPersonPassword, '\0', H243_NAME_LEN);
  m_recordingCommand = (DWORD)(-1);
}

/////////////////////////////////////////////////////////////////////////////
CConfAction& CConfAction::operator = (const CConfAction &other)
{
	m_ConfID = other.m_ConfID;
	m_NumAction = other.m_NumAction;
	m_NumAction1 = other.m_NumAction1;
	m_NumAction2 = other.m_NumAction2;
	strncpy(m_billing_data,other.m_billing_data,H243_NAME_LEN);
	for (int i = 0; i<MAX_CONF_INFO_ITEMS; i++)
	{
		strncpy(m_contact_info_list[i],other.m_contact_info_list[i],H243_NAME_LEN);
	}
	strncpy(m_entry_password,other.m_entry_password,CONFERENCE_ENTRY_PASSWORD_LEN);
	strncpy(m_chairPersonPassword,other.m_chairPersonPassword,H243_NAME_LEN);
	m_recordingCommand = other.m_recordingCommand;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CConfAction::~CConfAction()
{

}
/////////////////////////////////////////////////////////////////////////////
int CConfAction::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"ID",&m_ConfID,_0_TO_DWORD);

	std::string stringAction(strAction);

	if(!strncmp("SET_AUTO_LAYOUT",strAction,15))
	{
		BYTE bAutoLayout;
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"AUTO_LAYOUT",&bAutoLayout,_BOOL);

		if(bAutoLayout)
			m_NumAction = YES;
		else
			m_NumAction = NO;
	}
	else if(!strncmp("SET_VIDEO_CLARITY",strAction,17))
	{
			BYTE bVideoClarity;
			GET_VALIDATE_MANDATORY_CHILD(pActionNode,"VIDEO_CLARITY",&bVideoClarity,_BOOL);
			if(bVideoClarity)
				m_NumAction = YES;
			else
				m_NumAction = NO;
	}
    else if(!strncmp("SET_AUTO_REDIAL",strAction,15))
    {
        BYTE bAutoRedial;
        GET_VALIDATE_MANDATORY_CHILD(pActionNode,"AUTO_REDIAL",&bAutoRedial,_BOOL);
        if(bAutoRedial)
            m_NumAction = YES;
        else
            m_NumAction = NO;
    }
    else if(!strncmp("SET_AUTO_SCAN_INTERVAL",strAction,22))
    {
    	GET_VALIDATE_CHILD(pActionNode,"AUTO_SCAN_INTERVAL",&m_NumAction,AUTO_SCAN_INTERVAL_ENUM);
    }
	else if(!strncmp("SET_BILLING_DATA",strAction,16))
	{
	    CSmallString rBillingInfo;
		GET_VALIDATE_CHILD(pActionNode,"BILLING_DATA",rBillingInfo,_0_TO_H243_NAME_LENGTH);
		//Make sure Billing Data length is not over limit
		if (  MAX_BILLING_INFO_SIZE < rBillingInfo.GetStringLength() )
		         return STATUS_ILLEGAL_BILLING_DATA_LENGTH;

		strncpy(m_billing_data,rBillingInfo.GetString(),sizeof(m_billing_data) - 1);
		m_billing_data[sizeof(m_billing_data) - 1] ='\0';

	}
	else if(!strncmp("SET_CONF_CONTACT_INFO",strAction,21))
	{
		CXMLDOMElement *pChildNode;
		GET_CHILD_NODE(pActionNode, "CONTACT_INFO_LIST", pChildNode);
		if(pChildNode)
		{

			//pChildNode->ResetChildList();
			std::string contact_info_0;
			std::string contact_info_1;
			std::string contact_info_2;

			/*char    	m_contact_info_0[H243_NAME_LEN];
			char    	m_contact_info_1[H243_NAME_LEN];
			char    	m_contact_info_2[H243_NAME_LEN];*/
			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO",contact_info_0,_0_TO_H243_NAME_LENGTH);
			if (contact_info_0.empty() == false)
				strncpy(m_contact_info_list[0],contact_info_0.c_str(),H243_NAME_LEN);

			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO_2",contact_info_1,_0_TO_H243_NAME_LENGTH);
			if (contact_info_1.empty() == false)
				strncpy(m_contact_info_list[1],contact_info_1.c_str(),H243_NAME_LEN);

			GET_VALIDATE_CHILD(pChildNode,"CONTACT_INFO_3",contact_info_2,_0_TO_H243_NAME_LENGTH);
			if (contact_info_2.empty() == false)
				strncpy(m_contact_info_list[2],contact_info_2.c_str(),H243_NAME_LEN);

		}

	}
	else if(!strncmp("SET_ENTRY_PASSWORD",strAction,18))
	{
		GET_VALIDATE_CHILD(pActionNode,"ENTRY_PASSWORD",m_entry_password,_0_TO_CONFERENCE_ENTRY_PASSWORD_LENGTH);
	}

	else if ( stringAction == "SET_PASSWORD")
	  {
	    GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_chairPersonPassword,_0_TO_H243_NAME_LENGTH);
	  }
	else if(!strncmp("WITHDRAW_CONTENT_TOKEN",strAction,22))
	{
		m_NumAction = YES;//WITHDRAW_EPC_CONTENT_TOKEN;
	}

	else if ( stringAction == "START_RECORDING" )
	{
		m_recordingCommand = SET_START_RECORDING;
	}

	else if ( stringAction == "STOP_RECORDING" )
	{
		m_recordingCommand = SET_STOP_RECORDING;
	}

	else if ( stringAction == "PAUSE_RECORDING" )
	{
		m_recordingCommand = SET_PAUSE_RECORDING;
	}

	else if ( stringAction == "RESUME_RECORDING" )
	{
		m_recordingCommand = SET_RESUME_RECORDING;
	}
	else if ( stringAction == "SET_EXCLUSIVE_CONTENT_MODE" )
	{
		BYTE bIsExclusiveContentMode;
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"EXCLUSIVE_CONTENT_MODE",&bIsExclusiveContentMode,_BOOL);
		if(bIsExclusiveContentMode)
			m_NumAction = YES;
		else
			m_NumAction = NO;
	}
	else if ( stringAction == "SET_MUTE_PARTIES_IN_LECTURE" )
	{
		BYTE bIsMuteIncomingLectureMode;
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"MUTE_PARTIES_IN_LECTURE",&bIsMuteIncomingLectureMode,_BOOL);
		if(bIsMuteIncomingLectureMode)
			m_NumAction = YES;
		else
			m_NumAction = NO;
	}
	else if(stringAction == "SET_AUDIO_VIDEO_MUTE_PARTIES_EXCEPT_LEADER")
	{
		BYTE bMute = 0;
		BYTE bMuteUsers = 0;
		GET_VALIDATE_CHILD(pActionNode,"AUDIO_MUTE",&bMute,_BOOL);
		if (nStatus == STATUS_NODE_MISSING)
		{
			m_NumAction = -1;
			nStatus = STATUS_OK;
		}
		else
		{
			m_NumAction = bMute;
		}
		GET_VALIDATE_CHILD(pActionNode,"VIDEO_MUTE",&bMute,_BOOL);
		if (nStatus == STATUS_NODE_MISSING)
		{
			m_NumAction1 = -1;
			nStatus = STATUS_OK;
		}
		else
		{
			m_NumAction1 = bMute;
		}
		GET_VALIDATE_CHILD(pActionNode,"MUTE_EXISTING_USERS",&bMuteUsers,_BOOL);
		if (nStatus == STATUS_NODE_MISSING)
		{
			m_NumAction2 = -1;
			nStatus = STATUS_OK;
		}
		else
		{
			m_NumAction2 = bMuteUsers;
		}
	}

	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CConfAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
DWORD  CConfAction::GetConfID()
{
	return m_ConfID;
}

//////////////////////////////////////////////////////////////////////////
int  CConfAction::GetNumAction()
{
	return m_NumAction;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfAction::GetBillingData()
{
    return m_billing_data;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfAction::GetContactInfo(int contactNum)
{
    return m_contact_info_list[contactNum];
}

/*
/////////////////////////////////////////////////////////////////////////////
const char*  CConfAction::GetContactInfo1()
{
    return m_contact_info_0;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfAction::GetContactInfo2()
{
    return m_contact_info_1;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfAction::GetContactInfo3()
{
    return m_contact_info_2;
}
*/

//////////////////////////////////////////////////////////////////////////
void  CConfAction::SetNumAction(int nAction)
{
	m_NumAction = nAction;
}

//////////////////////////////////////////////////////////////////////////
const char*	CConfAction::GetEntryPassword()
{
	return m_entry_password;
}


//////////////////////////////////////////////////////////////////////////
DWORD CConfAction::GetRecordingCommand()
{
	return m_recordingCommand;
}
int  CConfAction::GetNumAction1()
{
	return m_NumAction1;
}
int  CConfAction::GetNumAction2()
{
	return m_NumAction2;
}


