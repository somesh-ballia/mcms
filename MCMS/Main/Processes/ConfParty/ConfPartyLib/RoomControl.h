/*
 * RoomControl.h
 *
 *  Created on: Jul 9, 2012
 *      Author: sshafrir
 */

#ifndef ROOMCONTROL_H_
#define ROOMCONTROL_H_

#include "ConfPartyApiDefines.h"

class  CConf;

/*typedef enum {
    eNonITP,     //like HDX - one link
    eCTS1000,    //one link
    eRPX2000,    //two  links
    eOTX,        //three links
    eCTS3000,    //three links
    eRPX4000,    //four links
} eITPEPType;*/

typedef struct
{
    char                    linkName[H243_NAME_LEN];
    DWORD                   linkMailbox;
    eTypeOfLinkParty        linkType;
} TLinkInfo;


class CRoomCntl : public CPObject

{ //abstract class

CLASS_TYPE_1(CRoomCntl,CPObject)

public:
    // Constructors
    CRoomCntl();
    CRoomCntl(DWORD numOfLinks,DWORD roomID, CConf* pConf);
    virtual ~CRoomCntl();
    virtual const char* NameOf() const { return "CRoomCntl";}

    // Initializations:
    CRoomCntl& operator=(const CRoomCntl& other);

    // Get/Set:
    void  SetRoomID (DWORD roomID) {m_roomID = roomID; /*DumpOfRoomControl();*/}
    DWORD GetRoomID () {return m_roomID;}

    void  SetNumOfLinks (DWORD numOfLinks) {m_numOfLinks = numOfLinks;}
    DWORD GetNumOfLinks () {return m_numOfLinks;}

    void  SetNumOfCurrentLinks (DWORD numOfCurrentLinks) {m_numOfCurrentLinks = numOfCurrentLinks;}
    DWORD GetNumOfCurrentLinks () {return m_numOfCurrentLinks;}

    void  SetCurrentTelepresenseType (eTelePresencePartyType currentTelepresenseType) {m_currentTelepresenseType = currentTelepresenseType;}
    eTelePresencePartyType GetCurrentTelepresenseType () {return m_currentTelepresenseType;}

    void  SetNumOfActiveLinks (DWORD numOfActiveLinks) {m_numOfActiveLinks = numOfActiveLinks;}
    DWORD GetNumOfActiveLinks () {return m_numOfActiveLinks;}

    void  SetMailBoxAccordingToIndex (DWORD indexOfLink, DWORD mailbox);


    BOOL  AddLink (const char* name, DWORD indexOfLink, eTypeOfLinkParty type, DWORD mailbox);
    void  RemoveLink(const char* name, DWORD indexOfLink);
    void  DumpOfRoomControl();
    BOOL  SetTelepresenceTypeAndIsActiveField(eTelePresencePartyType telePresenceLinkType, BYTE numOfActiveLinks);
    BOOL  IsSubPartyConnected(const char* name, DWORD indexOfLink);

protected:
    // Attributes
    DWORD                  m_roomID;                  //is according to mainLink..
    DWORD                  m_numOfLinks;
    DWORD                  m_numOfCurrentLinks;
    eTelePresencePartyType m_currentTelepresenseType; //Per Room
    DWORD                  m_numOfActiveLinks;
    TLinkInfo*             m_pPartiesInTheRoom;       //Pointer to array - size of m_numOfLinks
    CConf*                 m_pConf;                   //For using confParty of the all links (according to name).. CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

};


#endif /* ROOMCONTROL_H_ */
