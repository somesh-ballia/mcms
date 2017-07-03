/*
 * RoomControl.cpp
 *
 *  Created on: Jul 9, 2012
 *      Author: sshafrir
 */

#include <string>
#include <stdlib.h>
#include <sstream>
#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "RoomControl.h"
#include "Conf.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRoomCntl::CRoomCntl()
{
    m_roomID                  = 0;
    m_numOfLinks              = 0;
    m_numOfCurrentLinks       = 0;
    m_numOfActiveLinks        = 0;
    m_currentTelepresenseType = eTelePresencePartyNone;
    m_pConf                   = NULL;
    m_pPartiesInTheRoom       = NULL;

 }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRoomCntl::CRoomCntl(DWORD numOfLinks,DWORD roomID, CConf* pConf): m_roomID(roomID),
		                        m_numOfLinks(numOfLinks)//CPartyConnection
{
    m_numOfCurrentLinks       = 0;
    m_numOfActiveLinks        = 0;
    m_currentTelepresenseType = eTelePresencePartyNone;
    m_pConf                   = pConf;

    m_pPartiesInTheRoom = new TLinkInfo[m_numOfLinks];

    for(DWORD i = 0; i < m_numOfLinks; i++)
    {
        memset(m_pPartiesInTheRoom[i].linkName, '\0', H243_NAME_LEN);

        m_pPartiesInTheRoom[i].linkMailbox   = 0;
        m_pPartiesInTheRoom[i].linkType      = eRegularParty;
    }
}
/////////////////////////////////////////////////////////////////////////////
CRoomCntl& CRoomCntl::operator=(const CRoomCntl& other)
{
  if (&other == this)
    return *this;


  m_roomID                  = other.m_roomID;;
  m_numOfLinks              = other.m_numOfLinks;
  m_numOfActiveLinks        = other.m_numOfActiveLinks;
  m_numOfCurrentLinks       = other.m_numOfCurrentLinks;
  m_currentTelepresenseType = other.m_currentTelepresenseType;
  m_pConf                   = other.m_pConf;

  m_pPartiesInTheRoom = new TLinkInfo[m_numOfLinks];


  for(DWORD i = 0; i < m_numOfLinks; i++)
  {
      memset(m_pPartiesInTheRoom[i].linkName, '\0', H243_NAME_LEN);
      strncpy(m_pPartiesInTheRoom[i].linkName, other.m_pPartiesInTheRoom[i].linkName, strlen(other.m_pPartiesInTheRoom[i].linkName) );
      m_pPartiesInTheRoom[i].linkName[strlen(other.m_pPartiesInTheRoom[i].linkName)] = '\0';

      m_pPartiesInTheRoom[i].linkMailbox   = other.m_pPartiesInTheRoom[i].linkMailbox;

      m_pPartiesInTheRoom[i].linkType      = other.m_pPartiesInTheRoom[i].linkType;
  }
  return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CRoomCntl::AddLink (const char* name, DWORD indexOfLink, eTypeOfLinkParty type, DWORD mailbox)
{

    if ( indexOfLink >= m_numOfLinks )
    {
        PASSERTMSG(indexOfLink,"ITP_CASCADE: CRoomCntl::AddLink - ERROR - indexOfLink >= m_numOfLinks - index:");
        return FALSE;
    }

    //name:
    strcpy_safe(m_pPartiesInTheRoom[indexOfLink].linkName, H243_NAME_LEN, name);
    PTRACE2(eLevelInfoNormal, "CRoomCntl::AddLink name2: ", m_pPartiesInTheRoom[indexOfLink].linkName);

    //mailBox:
    m_pPartiesInTheRoom[indexOfLink].linkMailbox   = mailbox;

    //type;
    m_pPartiesInTheRoom[indexOfLink].linkType      = type;

    m_numOfCurrentLinks = m_numOfCurrentLinks + 1;

    DumpOfRoomControl();

    if (m_numOfCurrentLinks == m_numOfLinks)
    {
        PTRACE(eLevelInfoNormal, "ITP_CASCADE: CRoomCntl::AddLink - Room Fully Connected!");
        return TRUE;
    }

    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CRoomCntl::RemoveLink(const char* name, DWORD indexOfLink)
{
    PTRACE2INT(eLevelInfoNormal, "ITP_CASCADE: CRoomCntl::RemoveLink -index:",indexOfLink);

    memset(m_pPartiesInTheRoom[indexOfLink].linkName, '\0', H243_NAME_LEN);

    m_pPartiesInTheRoom[indexOfLink].linkMailbox   = 0;
    m_pPartiesInTheRoom[indexOfLink].linkType      = eRegularParty;

    m_numOfCurrentLinks = m_numOfCurrentLinks - 1;

    DumpOfRoomControl();

}
/////////////////////////////////////////////////////////////////////////////
void  CRoomCntl::SetMailBoxAccordingToIndex (DWORD indexOfLink, DWORD mailbox)
{
    m_pPartiesInTheRoom[indexOfLink].linkMailbox = mailbox;

    DumpOfRoomControl();
}
/////////////////////////////////////////////////////////////////////////////
BOOL  CRoomCntl::SetTelepresenceTypeAndIsActiveField(eTelePresencePartyType telePresenceLinkType, BYTE numOfActiveLinks)
{
    BOOL finishedToSet = TRUE;

    PTRACE2INT(eLevelInfoNormal, "ITP_CASCADE: CRoomCntl::SetTelepresenceTypeAndIsActiveField:",telePresenceLinkType);
    m_currentTelepresenseType = telePresenceLinkType;
    if (m_pConf)
    {

        for (BYTE i = 0; i < numOfActiveLinks; i++)
        {
        	CRsrvParty* pConfParty = (CRsrvParty*)m_pConf->GetCommConf()->GetCurrentParty(m_pPartiesInTheRoom[i].linkName);
            if (pConfParty)
            {
            	pConfParty->SetIsActive(TRUE);
                pConfParty->SetTelePresenceMode((BYTE)telePresenceLinkType);
            }
            else
            	TRACEINTO << "index:" << i << ", LinkName:" << m_pPartiesInTheRoom[i].linkName << " - ITP_CASCADE: ERROR1, pConfParty is NULL";
        }

        for (BYTE i = numOfActiveLinks; i < m_numOfLinks; i++)
        {
        	CRsrvParty* pConfParty = (CRsrvParty*)m_pConf->GetCommConf()->GetCurrentParty(m_pPartiesInTheRoom[i].linkName);
            if (pConfParty)
            {
                pConfParty->SetIsActive(FALSE);
                pConfParty->SetTelePresenceMode(eTelePresencePartyInactive);
            }
            else
            	TRACEINTO << "index:" << i << ", LinkName:" << m_pPartiesInTheRoom[i].linkName << " - ITP_CASCADE: ERROR2, pConfParty is NULL";
        }
    }
    else
    {
        PASSERTMSG(1,"ITP_CASCADE: CRoomCntl::SetTelepresenceTypeAndIsActiveField ERROR - m_pConf is NULL");
        finishedToSet = FALSE;
    }

    return finishedToSet;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CRoomCntl::IsSubPartyConnected(const char* name, DWORD indexOfLink)
{
    if (!m_pPartiesInTheRoom[indexOfLink].linkName)
        PTRACE(eLevelInfoNormal, "ITP_CASCADE: CRoomCntl::IsSubPartyConnected1 FALSE");
    else
        PTRACE(eLevelInfoNormal, "CRoomCntl::IsSubPartyConnected1 TRUE");

    if (m_pPartiesInTheRoom[indexOfLink].linkName[0] == '\0')
    {
        PTRACE(eLevelInfoNormal, "ITP_CASCADE: CRoomCntl::IsSubPartyConnected2 FALSE");
        return FALSE;
    }
    else
        return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CRoomCntl::DumpOfRoomControl()
{
    TRACESTR(eLevelError) << "ITP_CASCADE: CRoomCntl::DumpOfRoomControl - "
              << "roomID:" << m_roomID
              << " numOfLinks:" << m_numOfLinks
              << " numOfCurrentLinks:" << m_numOfCurrentLinks
              << " numOfActiveLinks:" << m_numOfActiveLinks
              << " currentTelepresenseType:" << m_currentTelepresenseType;

    for(DWORD i = 0; i < m_numOfLinks; i++)
    {
        TRACESTR(eLevelError) << " linkNumber:" << i
                  << " linkName:" << m_pPartiesInTheRoom[i].linkName
                  << " linkMailBox:" << m_pPartiesInTheRoom[i].linkMailbox
                  << " linkType:" << m_pPartiesInTheRoom[i].linkType;
    }
}
/////////////////////////////////////////////////////////////////////////////
CRoomCntl::~CRoomCntl() // destructor
{
     PDELETEA(m_pPartiesInTheRoom);
     m_pConf = NULL;
}
