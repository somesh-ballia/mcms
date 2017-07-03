//+========================================================================+
//                     TokenMsgMngr.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TokenMsgMngr.cpp	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2006  |                                                      |
//+========================================================================+

#include "TokenMsgMngr.h"

/////////////////////////////////////////////////////////////////////////////
CTokenMsgMngr::CTokenMsgMngr()
{
	m_state=DISABLE;
}
/////////////////////////////////////////////////////////////////////////////
CTokenMsgMngr::~CTokenMsgMngr()
{
	ClearAndDestroy();
}
/////////////////////////////////////////////////////////////////////////////
CTokenMsgMngr::CTokenMsgMngr (const CTokenMsgMngr& rOtherTokenMsgMngr)  
    :CPObject(rOtherTokenMsgMngr)
{
	//Copy all Msgs
	TOKEN_MSG_LIST List = rOtherTokenMsgMngr.m_tokenMsgList;
	TOKEN_MSG_LIST::iterator itr;
	
	for (itr = List.begin(); itr != List.end();++itr)	
	{
		CTokenMsg* pTMsg = new CTokenMsg();
		if (!CPObject::IsValidPObjectPtr(*itr))
		{
			PASSERT(1);
			POBJDELETE(pTMsg);
		}
		else
		{
			*pTMsg = *(*itr);
 			m_tokenMsgList.push_back(pTMsg);
		}
	}
	m_state=rOtherTokenMsgMngr.m_state;
}

/////////////////////////////////////////////////////////////////////////////
CTokenMsgMngr&	CTokenMsgMngr::operator= (const CTokenMsgMngr& rOtherTokenMsgMngr)
{
	if (&rOtherTokenMsgMngr == this ) 
		return *this;

	ClearAndDestroy();

	//Copy all Msgs
	TOKEN_MSG_LIST List = rOtherTokenMsgMngr.m_tokenMsgList;
	TOKEN_MSG_LIST::iterator itr;
	
	for (itr = List.begin(); itr != List.end();++itr)	
	{
		CTokenMsg* pTMsg = new CTokenMsg();
		if (!CPObject::IsValidPObjectPtr(*itr))
		{
			PASSERT(1);
			POBJDELETE(pTMsg);
			return *this;
		}
		
		*pTMsg = *(*itr);
 		m_tokenMsgList.push_back(pTMsg);
	}
	m_state=rOtherTokenMsgMngr.m_state;
	
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EMsgStatus CTokenMsgMngr::NewTokenMsg(CTokenMsg* pTokenMsg, EHwStreamState eHwStreamState)
{
	EMsgStatus Status = eMsgFree; 

	//Check if all msg come from the right direction
	if(m_state == ENABLE)
	{
		switch(pTokenMsg->GetMsgOpcode())
		{	//from Brdg to party
			case CONTENT_ROLE_TOKEN_WITHDRAW:
			case CONTENT_ROLE_TOKEN_RELEASE_ACK:
			case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
			case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
			case CONTENT_MEDIA_PRODUCER_STATUS:
			case CONTENT_ROLE_PROVIDER_IDENTITY: 
			case CONTENT_RATE_CHANGE:
	        case CONTENT_ROLE_TOKEN_ACQUIRE:
	        case CONTENT_ROLE_TOKEN_RELEASE:
	        case CONTENT_ROLE_TOKEN_WITHDRAW_ACK:
	        case CONTENT_NO_ROLE_PROVIDER:
			{
				if(pTokenMsg->GetMsgDirection() != eMsgOut)
				{
					DBGPASSERT(1);
	                PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = eMsgInvalid");
					return Status = eMsgInvalid;
				}
				break;
			}
			
			// from Party to Brdg
		//	case FLOW_CONTROL_RELEASE_REQ:
	        case PARTY_TOKEN_ACQUIRE:
			case PARTY_TOKEN_WITHDRAW_ACK:
			case PARTY_TOKEN_RELEASE:
			case ROLE_PROVIDER_IDENTITY:
			case MEDIA_PRODUCER_STATUS:
			case PARTY_CONTENT_RATE_CHANGED:
	        case PARTY_TOKEN_ACQUIRE_ACK:
	        case PARTY_TOKEN_ACQUIRE_NAK:
	        case PARTY_TOKEN_WITHDRAW:
	        case PARTY_BFCP_TOKEN_QUERY:
			{
				if(pTokenMsg->GetMsgDirection() != eMsgIn)
				{
					DBGPASSERT(2);
	                PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = eMsgInvalid");
					return Status = eMsgInvalid;
				}
				break;
			}
	        case CONTENT_ROLE_BFCP_HELLO_ACK:
	        {
	            return Status = eMsgInvalid;
	        }
			default	:
			{
				if(pTokenMsg->GetMsgOpcode())
					DBGPASSERT(pTokenMsg->GetMsgOpcode());
				else DBGPASSERT(3);
                PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = eMsgInvalid");
				return Status = eMsgInvalid;			
			}
		} //End of switch

		switch(eHwStreamState)  //Switch on stream stat
		{
			//If Strem is None
		  	case eHwStreamStateNone:
		  	{
				if(!IsListEmpty())
				{
					DBGPASSERT(4);
					ClearAndDestroy();
				}
				break;
		  	}
		  	//If Stream OFF
			case eHwStreamStateOff:  
			{ 			
			  	switch(pTokenMsg->GetMsgDirection())   // Switch on	Msg direction	
			  	{   //If Msg came from Party
			  		case(eMsgIn):
					{	 
						if(!IsListEmpty())
						{
							DBGPASSERT(5);
							ClearAndDestroy();
						}
						break;
					}
					case(eMsgOut):
					{	//If Msg came from ContentBridge
						if(pTokenMsg->GetMsgDirection() == eMsgOut)
						{
							if(pTokenMsg->GetMsgOpcode() == CONTENT_ROLE_TOKEN_ACQUIRE_ACK)
							{
								Status = AddMsgToListAndRetDelayed(pTokenMsg);
		                        PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = ", Status);
							}
							else
							{
								if(!IsListEmpty())
								{
									DBGPASSERT(6);
									ClearAndDestroy();
								}
							}
						}
						break;
					}
					default:
					{
						if(pTokenMsg->GetMsgDirection())
							DBGPASSERT(pTokenMsg->GetMsgDirection());
						else DBGPASSERT(3);
		                PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = eMsgInvalid");
						return Status = eMsgInvalid;
					}
			  	}
				break;
			}
			//If Stream ON
			case eHwStreamStateOn:
			{
				switch(pTokenMsg->GetMsgDirection())   // Switch on	Msg direction	
			  	{   //If Msg came from Party
			  		case(eMsgIn):
			  		{   //If Token_Release from Party 
			  			if(pTokenMsg->GetMsgOpcode() == PARTY_TOKEN_RELEASE)
			  			{
			  				Status = AddMsgToListAndRetDelayed(pTokenMsg);
                            PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = ", Status);
			  			}
			  			else
			  			{
			  				if(!IsListEmpty())
			  				{
								Status = AddMsgToListAndRetDelayed(pTokenMsg);
                                PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = ", Status);
			  				}
			  			}
			  			break;		  				
			  		}
			  		case(eMsgOut):
			  		{   //If Token_Withdraw form ContentBridge 
			  			if(pTokenMsg->GetMsgOpcode() == CONTENT_ROLE_TOKEN_WITHDRAW ||
	                       pTokenMsg->GetMsgOpcode() == CONTENT_ROLE_TOKEN_WITHDRAW_ACK)
			  			{
			  				Status = AddMsgToListAndRetDelayed(pTokenMsg);
                            PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = ", Status);
			  			}
			  			else	
			  			{
			  				if(!IsListEmpty())
			  				{
								Status = AddMsgToListAndRetDelayed(pTokenMsg);
                                PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = ", Status);
			  				}
			  				else
			  				{
                                PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : list is empty");
			  				}
			  			}	
			  			break;
			  		}
			  		default:
					{
						if(pTokenMsg->GetMsgDirection())
							DBGPASSERT(pTokenMsg->GetMsgDirection());
						else DBGPASSERT(3);
		                PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = eMsgInvalid");
						return Status = eMsgInvalid;
					}
			  		
			  	}
				break;	
			}
			default:
			{
				if(pTokenMsg->GetMsgDirection())
					DBGPASSERT(pTokenMsg->GetMsgDirection());
				else DBGPASSERT(3);
			    PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = eMsgInvalid");
				return Status = eMsgInvalid;
			}
			
		 }
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : m_state == DISABLE, SubOpcode - ", pTokenMsg->GetMsgOpcode());

		if(pTokenMsg->GetMsgOpcode()==PARTY_TOKEN_ACQUIRE && pTokenMsg->GetMsgDirection()== eMsgIn)
		{
			Status = AddMsgToListAndRetDelayed(pTokenMsg);
	        PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : m_state == DISABLE in IF");
		}
		else
		{
	        PTRACE(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : m_state == DISABLE NOT in IF");
		}
	}

	if (Status != eMsgFree)
	{
		PTRACE2INT(eLevelInfoNormal,"CTokenMsgMngr::NewTokenMsg : return Status = ",Status);
	}

	return Status;
}

////////////////////////////////////////////////////////////////////////////////
EMsgStatus CTokenMsgMngr::AddMsgToListAndRetDelayed(CTokenMsg* pMsg)
{
	PTRACE(eLevelInfoNormal,"CTokenMsgMngr::AddMsgToListAndRetDelayed ");
	
	EStat Status = AddMsg(pMsg);
	if(Status == statOK)
    	return eMsgDelayed;  
    else
     	return eMsgInvalid;
}

////////////////////////////////////////////////////////////////////////////////
void CTokenMsgMngr::StreamUpdate(CTokenMsgMngr* pPartyList)
{
	PTRACE(eLevelInfoNormal,"CTokenMsgMngr::StreamUpdate ");
	//If m_tokenMsgList is not Empty we will copy this list to list 
	//we received from party and delete m_tokenMsgList!!
	if(!IsListEmpty())
	{
		PTRACE(eLevelInfoNormal,"CTokenMsgMngr::StreamUpdate TMM List not empty! ");

		//Copy one list to another	
		*pPartyList = *this;

		ClearAndDestroy();	
	}

}
///////////////////////////////////////////////////////////////////////////////
BOOL CTokenMsgMngr::IsListEmpty()
{
	if(Size()==0)
		return TRUE;
	else 
		return FALSE;	
}
//////////////////////////////////////////////////////////////////////////////
EStat CTokenMsgMngr::AddMsg(CTokenMsg* pTokenMsg)
{
	if(!CPObject::IsValidPObjectPtr(pTokenMsg))
		return statInconsistent;
	
	CTokenMsg* pMsg = new CTokenMsg;
	*pMsg = *pTokenMsg;  

	m_tokenMsgList.push_back(pMsg);

	return statOK;
}	
///////////////////////////////////////////////////////////////////////////
DWORD CTokenMsgMngr::Size()
{
	return m_tokenMsgList.size();
}

//////////////////////////////////////////////////////////////////////////
void CTokenMsgMngr::ClearAndDestroy()  
{
	PTRACE(eLevelInfoNormal,"CTokenMsgMngr::ClearAndDestroy Celar TokenMsgs ");
	
	CTokenMsg*  pRemovedTokenMsg = NULL;
	
	//Define pointer that points to the begining of the list 
	TOKEN_MSG_LIST::iterator itr =  m_tokenMsgList.begin();

	while (itr != m_tokenMsgList.end())
	{
		pRemovedTokenMsg = (*itr);
		m_tokenMsgList.erase(itr);
		POBJDELETE(pRemovedTokenMsg);
		itr =  m_tokenMsgList.begin();
	}
	
}
/////////////////////////////////////////////////////////////////////////
TOKEN_MSG_LIST::iterator CTokenMsgMngr::Begin()
{
	TOKEN_MSG_LIST::iterator itr = m_tokenMsgList.begin();
	return itr;
}
/////////////////////////////////////////////////////////////////////////
TOKEN_MSG_LIST::iterator CTokenMsgMngr::End()
{
	TOKEN_MSG_LIST::iterator itr = m_tokenMsgList.end();
	return itr;
}
/////////////////////////////////////////////////////////////////////////
void CTokenMsgMngr::Clear()
{
	m_tokenMsgList.clear();
}
