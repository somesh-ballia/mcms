#ifndef TEXTONSCREENMNGR_H_
#define TEXTONSCREENMNGR_H_

#define INVALID_LINE (DWORD)-1
#include <vector>
#include <list>
#include "TextOnScreenMsg.h"
#include "TaskApp.h"
#include "StateMachine.h"
#include "ObjString.h"

class CConf;

typedef std::vector< CTextOnScreenMsg*>TEXT_MSG_LIST;

////////////////////////////////////////////////////////////////////////////
//                        CTextOnScreenMngr
////////////////////////////////////////////////////////////////////////////
class CTextOnScreenMngr : public CPObject
{
  CLASS_TYPE_1(CTextOnScreenMngr, CPObject)

public:
                                 CTextOnScreenMngr();
                                 CTextOnScreenMngr(const CTextOnScreenMngr& rOtherTextMsgMngr);
  virtual                       ~CTextOnScreenMngr(void);

  virtual const char*            NameOf() const         { return "CTextOnScreenMngr";}

  DWORD                          Size()                 { return m_TextMsgList.size(); }
  TEXT_MSG_LIST::iterator        Begin()                { return m_TextMsgList.begin(); }
  TEXT_MSG_LIST::iterator        End()                  { return m_TextMsgList.end(); }
  void                           Clear()                { m_TextMsgList.clear(); }
  DWORD                          GetNumOfTextLines()    { return Size(); }
  void                           ClearAndDestroy();
  void                           Dump();

  void                           Serialize(WORD format, CSegment& seg);
  void                           DeSerialize(WORD format, CSegment& seg);
  void                           AddMsg(CTextOnScreenMsg* pTextLine);
  CTextOnScreenMsg*              GetFirst();
  CTextOnScreenMsg*              GetNext();

protected:
  TEXT_MSG_LIST                  m_TextMsgList;
  TEXT_MSG_LIST::iterator        m_TextMsgListIterator; // for GetFirst and GetNext functions

private:
  BOOL                           IsListEmpty()          { return (Size() == 0) ? TRUE : FALSE; }
};


////////////////////////////////////////////////////////////////////////////
//                        CExtendedTextOnScreenMsg
////////////////////////////////////////////////////////////////////////////
class CExtendedTextOnScreenMsg : public CPObject
{
  CLASS_TYPE_1(CExtendedTextOnScreenMsg, CPObject)

public:
                                 CExtendedTextOnScreenMsg();
                                 CExtendedTextOnScreenMsg(CTextOnScreenMsg* pMsg, DWORD timeout, BYTE allowOverride);
                                 CExtendedTextOnScreenMsg(const CExtendedTextOnScreenMsg& other);
  virtual                       ~CExtendedTextOnScreenMsg(void);

  virtual const char*            NameOf() const     { return "CExtendedTextOnScreenMsg";}

  CTextOnScreenMsg*              GetTextMessage()   { return m_pTextOnScreenMsg;}
  BYTE                           GetAllowOverride() { return m_allowOverride;   }
  DWORD                          GetTimeOut()       { return m_timeout;     }
  CMedString                     Dump();

private:
  CTextOnScreenMsg*              m_pTextOnScreenMsg;
  DWORD                          m_timeout;
  BYTE                           m_allowOverride;
};

typedef std::list<CExtendedTextOnScreenMsg*> MESSAGES_QUEUE;
class CTimerForMessage;
class CTextOnScreenMngrForGwSession;

////////////////////////////////////////////////////////////////////////////
//                        CDataForLine
////////////////////////////////////////////////////////////////////////////
class CDataForLine : public CPObject
{
  CLASS_TYPE_1(CDataForLine, CPObject)

public:
                                 CDataForLine(DWORD partyId);
                                 CDataForLine(CDataForLine& other);
  virtual                       ~CDataForLine();
  virtual const char*            NameOf() const                                 { return "CDataForLine"; }

  void                           SetPartyId(DWORD partyId)                      { m_partyId = partyId; }
  void                           SetIsActiveTimer(BYTE isActiveVal)             { m_isActiveTimer = isActiveVal; }
  void                           SetIsAllowOverride(BYTE isAllowOverrideVal)    { m_allowOverride = isAllowOverrideVal; }

  DWORD                          GetPartyId()                                   { return m_partyId; }
  BYTE                           GetIsActiveTimer()                             { return m_isActiveTimer; }
  BYTE                           GetIsAllowOverride()                           { return m_allowOverride; }
  MESSAGES_QUEUE*                GetMessageQ()                                  { return nextMessage; }

  void                           StartTimerForParty(CTextOnScreenMngrForGwSession* pTextOnScreenManager, DWORD timeout);
  void                           DeleteTimerForParty();
  void                           AddMessageToQ(CExtendedTextOnScreenMsg* pMsg);
  BYTE                           IsQEmpty();
  CExtendedTextOnScreenMsg*      GetNextMessage();
  void                           DumpMessageQ(CLargeString& cstr);

private:
  DWORD                          m_partyId;
  BYTE                           m_isActiveTimer;
  BYTE                           m_allowOverride;
  CTimerForMessage*              m_pTimerForMessage;
  MESSAGES_QUEUE*                nextMessage;
};

typedef std::vector<CDataForLine*> TEXT_LINE_INFO;

////////////////////////////////////////////////////////////////////////////
//                        CTextOnScreenMngrForGwSession
////////////////////////////////////////////////////////////////////////////
class CTextOnScreenMngrForGwSession : public CTextOnScreenMngr
{
  CLASS_TYPE_1(CTextOnScreenMngrForGwSession, CTextOnScreenMngr)

public:
                                 CTextOnScreenMngrForGwSession(CConf* pConf);
                                 CTextOnScreenMngrForGwSession(const CTextOnScreenMngrForGwSession& other);
  virtual                       ~CTextOnScreenMngrForGwSession(void);

  virtual const char*            NameOf() const { return "CTextOnScreenMngrForGwSession";}

  void                           PrepareMessage(DWORD partyId, const char* msgStr, DWORD timeout = 0, BYTE allowOverride = TRUE, BYTE displayImmediately = FALSE);
  virtual void                   AddMsgToLine(CTextOnScreenMsg* pTextLine, DWORD line, DWORD timeout = 0, BYTE allowOverride = TRUE, DWORD partyId = -1);
  virtual void                   RemoveLine(DWORD partyId, DWORD line);

  DWORD                          GetPartyLine(DWORD partyId);
  void                           RemoveParty(DWORD partyId);
  void                           AddParty(DWORD partyId);

  void                           CreateTimerForParty(DWORD partyId, DWORD timeout);
  void                           OnTimerForParty(DWORD line);

  void                           DumpVector();
  void                           TextMsgListToCharArray(char** displayStringArr);
  void                           ClearMessageQ(MESSAGES_QUEUE* q);
  void                           ClearVector();

protected:
  TEXT_LINE_INFO                 m_LineInfo;
  CConf*                         m_pConf;
};


const WORD LOCAL_MSG_TIMER = 1;

////////////////////////////////////////////////////////////////////////////
//                        CTimerForMessage
////////////////////////////////////////////////////////////////////////////
class CTimerForMessage : public CStateMachine
{
  CLASS_TYPE_1(CTimerForMessage, CStateMachine)

public:
                                 CTimerForMessage();
                                 CTimerForMessage(CTextOnScreenMngrForGwSession* pTextOnScreenManager, DWORD partyId, DWORD tout);
                                 CTimerForMessage(CTimerForMessage& other);
  virtual                       ~CTimerForMessage();

  // CPObject and CStateMachine pure virtual
  virtual void*                  GetMessageMap();
  virtual void                   HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
  virtual const char*            NameOf() const {return "CTimerForMessage";}

  void                           StartLocalTimer();
  void                           RestartLocalTimer(DWORD timeout);
  void                           OnLocalMsgTimer(CSegment* pParam);

private:

  CTextOnScreenMngrForGwSession* m_pTextOnScreenManager;
  DWORD                          m_partyId;
  DWORD                          m_tout;

  PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CTextOnScreenMngrForInvitedSession
////////////////////////////////////////////////////////////////////////////
class CTextOnScreenMngrForInvitedSession : public CTextOnScreenMngrForGwSession
{
  CLASS_TYPE_1(CTextOnScreenMngrForInvitedSession, CTextOnScreenMngrForGwSession)

public:
                                 CTextOnScreenMngrForInvitedSession(CConf* pConf);
                                 CTextOnScreenMngrForInvitedSession(const CTextOnScreenMngrForInvitedSession& other);
  virtual                       ~CTextOnScreenMngrForInvitedSession(void);

  virtual const char*            NameOf() const { return "CTextOnScreenMngrForInvitedSession";}

  virtual void                   AddMsgToLine(CTextOnScreenMsg* pTextLine, DWORD line, DWORD timeout = 0, BYTE allowOverride = TRUE, DWORD partyId = -1);
  virtual void                   RemoveLine(DWORD partyId, DWORD line);
};

#endif /*TEXTONSCREENMNGR_H_*/

