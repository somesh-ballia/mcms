#include "TextOnScreenMngr.h"
#include "ObjString.h"
#include "Conf.h"

////////////////////////////////////////////////////////////////////////////
//                        CTextOnScreenMngr
////////////////////////////////////////////////////////////////////////////
CTextOnScreenMngr::CTextOnScreenMngr()
{
}

//--------------------------------------------------------------------------
CTextOnScreenMngr::~CTextOnScreenMngr()
{
  ClearAndDestroy();
}

//--------------------------------------------------------------------------
CTextOnScreenMngr::CTextOnScreenMngr(const CTextOnScreenMngr& rOtherTextMsgMngr)
                  :CPObject(rOtherTextMsgMngr)
{
  // Copy all Msgs
  TEXT_MSG_LIST List = rOtherTextMsgMngr.m_TextMsgList;

  for (TEXT_MSG_LIST::iterator itr = List.begin(); itr != List.end(); ++itr)
  {
    if (!CPObject::IsValidPObjectPtr(*itr))
    {
      PASSERT(1);
    }
    else
    {
      CTextOnScreenMsg* pTMsg = new CTextOnScreenMsg();
      *pTMsg = *(*itr);
      m_TextMsgList.push_back(pTMsg);
    }
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngr::AddMsg(CTextOnScreenMsg* pTextLine)
{
  DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pTextLine));

  CTextOnScreenMsg* pMsg = new CTextOnScreenMsg;
  *pMsg = *pTextLine;

  m_TextMsgList.push_back(pMsg);
}

//--------------------------------------------------------------------------
void CTextOnScreenMngr::Dump()
{
  std::ostringstream msg;
  msg.precision(0);
  msg << "CTextOnScreenMngr::Dump() - Size:" << Size();

  for (TEXT_MSG_LIST::iterator itr = m_TextMsgList.begin(); itr != m_TextMsgList.end(); ++itr)
  {
    CTextOnScreenMsg* pMsg = (*itr);
    if (pMsg != NULL)
      msg << "\n'" << pMsg->GetTextLine() << "'";
  }

  PTRACE(eLevelInfoNormal, msg.str().c_str());
}

//--------------------------------------------------------------------------
void CTextOnScreenMngr::Serialize(WORD format, CSegment& seg)
{
  if (format == NATIVE && Size())
  {
    seg << Size();
    for (TEXT_MSG_LIST::iterator itr = m_TextMsgList.begin(); itr != m_TextMsgList.end(); ++itr)
    {
      CTextOnScreenMsg* pTextLine = (*itr);
      if (pTextLine != NULL)
        seg.Put((BYTE*) pTextLine, sizeof(CTextOnScreenMsg));
    }
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngr::DeSerialize(WORD format, CSegment& seg)
{
  DWORD numberOfLines = 0;
  if (format == NATIVE)
  {
    seg >> numberOfLines;
    for (DWORD i = 0; i < numberOfLines; i++)
    {
      CTextOnScreenMsg* pTextLine = new CTextOnScreenMsg;
      if (pTextLine)
      {
        seg.Get((BYTE*) pTextLine, sizeof(CTextOnScreenMsg));
        m_TextMsgList.push_back(pTextLine);
      }
    }
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngr::ClearAndDestroy()
{
  TRACEINTO << "CTextOnScreenMngr::ClearAndDestroy";

  // Define pointer that points to the beginning of the list
  TEXT_MSG_LIST::iterator itr =  m_TextMsgList.begin();

  while (itr != m_TextMsgList.end())
  {
    CTextOnScreenMsg* pRemovedTextMsg = (*itr);
    m_TextMsgList.erase(itr);
    POBJDELETE(pRemovedTextMsg);
    itr =  m_TextMsgList.begin();
  }
}

//--------------------------------------------------------------------------
CTextOnScreenMsg* CTextOnScreenMngr::GetFirst()
{
  m_TextMsgListIterator = m_TextMsgList.begin();

  if (m_TextMsgListIterator != m_TextMsgList.end())
    return (*m_TextMsgListIterator);
  else
    return NULL;
}

//--------------------------------------------------------------------------
CTextOnScreenMsg* CTextOnScreenMngr::GetNext()
{
  // make sure not to increase iterator if it's already at end
  if (m_TextMsgListIterator == m_TextMsgList.end())
    return NULL;

  m_TextMsgListIterator++;

  if (m_TextMsgListIterator != m_TextMsgList.end())
    return (*m_TextMsgListIterator);
  else
    return NULL;
}


////////////////////////////////////////////////////////////////////////////
//                        CExtendedTextOnScreenMsg
////////////////////////////////////////////////////////////////////////////
CExtendedTextOnScreenMsg::CExtendedTextOnScreenMsg()
{
  m_pTextOnScreenMsg = NULL;
}

//--------------------------------------------------------------------------
CExtendedTextOnScreenMsg::CExtendedTextOnScreenMsg(CTextOnScreenMsg* pMsg, DWORD timeout, BYTE allowOverride)
{
  m_pTextOnScreenMsg  = new CTextOnScreenMsg();
  *m_pTextOnScreenMsg = *pMsg;
  m_timeout           = timeout;
  m_allowOverride     = allowOverride;
}

//--------------------------------------------------------------------------
CExtendedTextOnScreenMsg::CExtendedTextOnScreenMsg(const CExtendedTextOnScreenMsg& other)
                         :CPObject(other)
{
  if (other.m_pTextOnScreenMsg)
  {
    m_pTextOnScreenMsg  = new CTextOnScreenMsg();
    *m_pTextOnScreenMsg = *other.m_pTextOnScreenMsg;
  }
  else
    m_pTextOnScreenMsg = NULL;	

  m_timeout       = other.m_timeout;
  m_allowOverride = other.m_allowOverride;
}

//--------------------------------------------------------------------------
CExtendedTextOnScreenMsg::~CExtendedTextOnScreenMsg()
{
  POBJDELETE(m_pTextOnScreenMsg);
}

//--------------------------------------------------------------------------
CMedString CExtendedTextOnScreenMsg::Dump()
{
  CMedString cstr;
  cstr << "msg: ";
  if (m_pTextOnScreenMsg)
    cstr << m_pTextOnScreenMsg->GetTextLine() << "\n";
  else
    cstr <<"**m_pTextOnScreenMsg is INVALID**" << "\n";

  cstr << "timeout: " << m_timeout << "\n";
  cstr << "allowOverride: " << m_allowOverride << "\n";

  return cstr;
}


////////////////////////////////////////////////////////////////////////////
//                        CDataForLine
////////////////////////////////////////////////////////////////////////////
CDataForLine::CDataForLine(DWORD partyId)
{
  m_partyId          = partyId;
  m_isActiveTimer    = NO;
  m_allowOverride    = YES;
  m_pTimerForMessage = NULL;
  nextMessage        = new MESSAGES_QUEUE;
}

//--------------------------------------------------------------------------
CDataForLine::CDataForLine(CDataForLine& other) : CPObject(other)
{
  m_partyId       = other.m_partyId;
  m_isActiveTimer = other.m_isActiveTimer;
  m_allowOverride = other.m_allowOverride;
  if (other.m_pTimerForMessage)
    m_pTimerForMessage = new CTimerForMessage(*other.m_pTimerForMessage);
  else
    m_pTimerForMessage = NULL;

  nextMessage = new MESSAGES_QUEUE;
  MESSAGES_QUEUE* otherQ = other.nextMessage;
  for (MESSAGES_QUEUE::iterator it = otherQ->begin(); it != otherQ->end(); ++it)
  {
    CExtendedTextOnScreenMsg* pMsg = *it;
    nextMessage->push_back(new CExtendedTextOnScreenMsg(*pMsg));
  }
}

//--------------------------------------------------------------------------
CDataForLine::~CDataForLine()
{
  POBJDELETE(m_pTimerForMessage);

  MESSAGES_QUEUE::iterator it;
  for (it = nextMessage->begin(); it != nextMessage->end(); ++it)
    POBJDELETE(*it);

  nextMessage->clear();
  POBJDELETE(nextMessage);
}

//--------------------------------------------------------------------------
void CDataForLine::StartTimerForParty(CTextOnScreenMngrForGwSession* pTextOnScreenManager, DWORD timeout)
{
  if (!m_pTimerForMessage)
  {
    // new timer message
    m_pTimerForMessage = new CTimerForMessage(pTextOnScreenManager, m_partyId, timeout);
    m_pTimerForMessage->StartLocalTimer();
  }
  else
  {
    // there is already a timer message on screen and new timer message should overide it
    m_pTimerForMessage->RestartLocalTimer(timeout);
  }
}

//--------------------------------------------------------------------------
void CDataForLine::DeleteTimerForParty()
{
  POBJDELETE(m_pTimerForMessage);
}

//--------------------------------------------------------------------------
void CDataForLine::AddMessageToQ(CExtendedTextOnScreenMsg* pExtendedMsg)
{
  CMedString cstr;
  cstr << "CDataForLine::AddMessageToQ Timer is active and allow override = false --> push message to Q\n";

  if (!nextMessage->empty())
  {
    cstr << "Queue is not empty ";
    CExtendedTextOnScreenMsg* pLastMsg = nextMessage->back();
    if (pLastMsg->GetAllowOverride() == TRUE)
    {
      cstr << " override last message";

      // remove previous message
      nextMessage->pop_back();
      // Eitan(test) - make sure the element destructor is called
      PASSERT(CPObject::IsValidPObjectPtr(pLastMsg));
    }
  }

  // add the new message at end of Q
  nextMessage->push_back(pExtendedMsg);

  PTRACE(eLevelInfoNormal, cstr.GetString());
}

//--------------------------------------------------------------------------
BYTE CDataForLine::IsQEmpty()
{
  return nextMessage->empty();
}

//--------------------------------------------------------------------------
CExtendedTextOnScreenMsg* CDataForLine::GetNextMessage()
{
  CExtendedTextOnScreenMsg* pExtendedMsg = nextMessage->front();
  nextMessage->pop_front();
  return pExtendedMsg;
}

//--------------------------------------------------------------------------
void CDataForLine::DumpMessageQ(CLargeString& cstr)
{
  if (nextMessage->size() > 0)
  {
    cstr << "message queue: \n";
    cstr << "=============  \n";

    MESSAGES_QUEUE::iterator it;
    for (it = nextMessage->begin(); it != nextMessage->end(); ++it)
    {
      cstr << (*it)->Dump();
      cstr << "\t";
    }
  }
  else
  {
    cstr << "message queue is empty!";
  }
}


////////////////////////////////////////////////////////////////////////////
//                        CTextOnScreenMngrForGwSession
////////////////////////////////////////////////////////////////////////////
CTextOnScreenMngrForGwSession::CTextOnScreenMngrForGwSession(CConf* pConf)
{
  m_pConf = pConf;
}

//--------------------------------------------------------------------------
CTextOnScreenMngrForGwSession::CTextOnScreenMngrForGwSession(const CTextOnScreenMngrForGwSession& other) : CTextOnScreenMngr(other)
{
}

//--------------------------------------------------------------------------
CTextOnScreenMngrForGwSession::~CTextOnScreenMngrForGwSession()
{
  ClearVector();
  m_LineInfo.clear();
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::PrepareMessage(DWORD partyId, const char* msgStr, DWORD timeout, BYTE allowOverride, BYTE displayImmediately)
{
  DWORD line = GetPartyLine(partyId);
  PASSERT_AND_RETURN(line == INVALID_LINE);

  CTextOnScreenMsg* pMsg = new CTextOnScreenMsg;
  pMsg->SetTextLine(msgStr);
  if (displayImmediately || m_LineInfo[line]->GetIsActiveTimer() != TRUE || m_LineInfo[line]->GetIsAllowOverride() == TRUE)
  {
    if (displayImmediately)
      ClearMessageQ(m_LineInfo[line]->GetMessageQ());

    AddMsgToLine(pMsg, line, timeout, allowOverride, partyId);
  }
  else
  {
    // add Message to queue
    CExtendedTextOnScreenMsg* pExtendedMsg = new CExtendedTextOnScreenMsg(pMsg, timeout, allowOverride);
    m_LineInfo[line]->AddMessageToQ(pExtendedMsg);
  }
  POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::AddMsgToLine(CTextOnScreenMsg* pTextLine, DWORD line, DWORD timeout, BYTE allowOverride, DWORD partyId)
{
  CMedString cstr;
  cstr << "CTextOnScreenMngrForGwSession::AddMsgToLine  line: " << line << ", msg: " <<  pTextLine->GetTextLine() << ", timeout = " << timeout;
  TEXT_MSG_LIST::iterator it;
  if (line >= Size())
  {
    cstr << "line > Size ( line=" << line <<" , size=" << Size() << " )\n";
    int i = 0;
    for (it = m_TextMsgList.end(); it != m_TextMsgList.begin()+line; ++it)
    {
      cstr << "insert dummy message [" << ++i << "]\n";
      CTextOnScreenMsg dummyMsg;
      AddMsg(&dummyMsg);
    }

    cstr << "insert original message";
    AddMsg(pTextLine);
  }
  else
  {
    cstr << "updating line message\n";
    it = Begin()+line;
    if (CPObject::IsValidPObjectPtr(*it))
      **it = *pTextLine;
    else
      PASSERT_AND_RETURN(line);
  }

  m_pConf->SendPartyMsgOnScreenForGWConf();
  PTRACE(eLevelInfoNormal, cstr.GetString());
  DumpVector();
  Dump();

  if (timeout)
  {
    m_LineInfo[line]->StartTimerForParty(this, timeout); // CreateTimerForParty(m_LineInfo[line]->GetPartyId(),timeout);
    m_LineInfo[line]->SetIsActiveTimer(TRUE);
    m_LineInfo[line]->SetIsAllowOverride(allowOverride);
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::CreateTimerForParty(DWORD partyId, DWORD timeout)
{
  TRACEINTO << "CTextOnScreenMngrForGwSession::CreateTimerForParty - PartyId:" << partyId << ", timeout:" << timeout << ", this:" << (DWORD)this;
  CTimerForMessage* pTimerForMessage = new CTimerForMessage(this, partyId, timeout); AUTO_DELETE(pTimerForMessage);
  pTimerForMessage->StartLocalTimer();
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::OnTimerForParty(DWORD partyId)
{
  // party line could have been deleted/moved while the timer was active
  DWORD line = GetPartyLine(partyId);
  PASSERT_AND_RETURN(line == INVALID_LINE);

  TRACEINTO << "CTextOnScreenMngrForGwSession::OnTimerForParty - PartyId:" << partyId << ", Line:" << line;

  m_LineInfo[line]->SetIsActiveTimer(FALSE);
  m_LineInfo[line]->DeleteTimerForParty();

  // check if there are pending messages in Q
  if (!m_LineInfo[line]->IsQEmpty())
  {
    CExtendedTextOnScreenMsg* pExtendedMsg = m_LineInfo[line]->GetNextMessage();
    PrepareMessage(m_LineInfo[line]->GetPartyId(), pExtendedMsg->GetTextMessage()->GetTextLine(), pExtendedMsg->GetTimeOut(), pExtendedMsg->GetAllowOverride());
  }
  else
    RemoveLine(partyId, line);
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::RemoveLine(DWORD partyId, DWORD line)
{
  TRACEINTO << "CTextOnScreenMngrForGwSession::RemoveLine - PartyId:" << partyId << ", Line:" << line;

  PASSERT_AND_RETURN(line >= m_LineInfo.size());

  TEXT_LINE_INFO::iterator it = m_LineInfo.begin()+line;
  CDataForLine* pData = *it;
  m_LineInfo.erase(it);
  POBJDELETE(pData);

  PASSERT_AND_RETURN(line >= Size());

  TEXT_MSG_LIST::iterator itr = m_TextMsgList.begin()+line;
  CTextOnScreenMsg* pRemovedTextMsg = (*itr);
  m_TextMsgList.erase(itr);
  POBJDELETE(pRemovedTextMsg);

  m_pConf->SendPartyMsgOnScreenForGWConf();
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::ClearMessageQ(MESSAGES_QUEUE* q)
{
  PASSERT_AND_RETURN(!q);

  TRACEINTO << "CTextOnScreenMngrForGwSession::ClearMessageQ - Size:" << q->size();

  if (q->size())
  {
    for (MESSAGES_QUEUE::iterator qIt = q->begin(); qIt != q->end(); ++qIt)
      POBJDELETE(*qIt);

    q->clear();
  }
}

//--------------------------------------------------------------------------
DWORD CTextOnScreenMngrForGwSession::GetPartyLine(DWORD partyId)
{
  DWORD line  = INVALID_LINE;
  DWORD index = 0;
  for (TEXT_LINE_INFO::iterator it = m_LineInfo.begin(); it != m_LineInfo.end(); ++it)
  {
    if ((*it)->GetPartyId() == partyId)
    {
      line = index;
      break;
    }
    else
      index++;
  }

  return line;
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::RemoveParty(DWORD partyId)
{
  DWORD line = GetPartyLine(partyId);

  TRACEINTO << "CTextOnScreenMngrForGwSession::RemoveParty - PartyId:" << partyId << ", Line:" << line;

  if (line != INVALID_LINE)
    RemoveLine(partyId, line);
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::AddParty(DWORD partyId)
{
  DWORD line = GetPartyLine(partyId);

  TRACEINTO << "CTextOnScreenMngrForGwSession::AddParty - PartyId:" << partyId << ", Line:" << line;

  if (line == INVALID_LINE)
  {
    CDataForLine* newLine = new CDataForLine(partyId);
    m_LineInfo.push_back(newLine);

    CTextOnScreenMsg pDummyMsg;
    AddMsgToLine(&pDummyMsg, GetPartyLine(partyId), 0, TRUE, partyId);
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::DumpVector()
{
  CLargeString cstr;
  int index = 0;
  for (TEXT_LINE_INFO::iterator it = m_LineInfo.begin(); it != m_LineInfo.end(); ++it)
  {
    CDataForLine* currentLine = *it;
    cstr << "line number: " << index << " party id: " << currentLine->GetPartyId();
    cstr << " isActiveTimer: " << (currentLine->GetIsActiveTimer() ? "YES" : "NO ") << " ";
    cstr << " allowOverride: " << (currentLine->GetIsAllowOverride() ? "YES" : "NO ") << " ";
    currentLine->DumpMessageQ(cstr);
    cstr << "\n";
    index++;
  }

  PTRACE(eLevelInfoNormal, cstr.GetString());
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::ClearVector()
{
  TRACEINTO << "CTextOnScreenMngrForGwSession::ClearVector";
  DumpVector();

  TEXT_LINE_INFO::iterator it = m_LineInfo.begin();
  while (it != m_LineInfo.end())
  {
    CDataForLine* pCurrentLine = *it;
    it = m_LineInfo.erase(it);
    POBJDELETE(pCurrentLine);
    it = m_LineInfo.begin();
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForGwSession::TextMsgListToCharArray(char** displayStringArr)
{
  int i = 0;
  for (TEXT_MSG_LIST::iterator it = m_TextMsgList.begin(); it != m_TextMsgList.end() && i < MAX_TEXT_LEN; ++it)
  {
    displayStringArr[i] = new char[MAX_SITE_NAME_ARR_SIZE];
    strncpy(displayStringArr[i], (*it)->GetTextLine(), MAX_SITE_NAME_ARR_SIZE-1);
    displayStringArr[i][MAX_SITE_NAME_ARR_SIZE-1] = '\0';
    i++;
  }
}

PBEGIN_MESSAGE_MAP(CTimerForMessage)
  ONEVENT(LOCAL_MSG_TIMER, ANYCASE, CTimerForMessage::OnLocalMsgTimer)
PEND_MESSAGE_MAP(CTimerForMessage, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CTimerForMessage
////////////////////////////////////////////////////////////////////////////
void* CTimerForMessage::GetMessageMap()
{
  return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CTimerForMessage::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
  DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
CTimerForMessage::CTimerForMessage()
{
  m_pTextOnScreenManager = NULL;
}

//--------------------------------------------------------------------------
CTimerForMessage::CTimerForMessage(CTextOnScreenMngrForGwSession* pTextOnScreenManager, DWORD partyId, DWORD timeout)
{
  m_pTextOnScreenManager = pTextOnScreenManager;
  m_partyId              = partyId;
  m_tout                 = timeout;
}

//--------------------------------------------------------------------------
CTimerForMessage::CTimerForMessage(CTimerForMessage& other) : CStateMachine(other)
{
  m_pTextOnScreenManager = other.m_pTextOnScreenManager;

  m_partyId              = other.m_partyId;
  m_tout                 = other.m_tout;
}

//--------------------------------------------------------------------------
CTimerForMessage::~CTimerForMessage()
{
}

//--------------------------------------------------------------------------
void CTimerForMessage::OnLocalMsgTimer(CSegment* pParam)
{
  TRACEINTO << "CTimerForMessage::OnLocalMsgTimer - PartyId:" << m_partyId;
  m_pTextOnScreenManager->OnTimerForParty(m_partyId);
}

//--------------------------------------------------------------------------
void CTimerForMessage::StartLocalTimer()
{
  StartTimer(LOCAL_MSG_TIMER, m_tout);
}

//--------------------------------------------------------------------------
void CTimerForMessage::RestartLocalTimer(DWORD timeout)
{
  m_tout = timeout;
  StartLocalTimer();
}


////////////////////////////////////////////////////////////////////////////
//                        CTextOnScreenMngrForInvitedSession
////////////////////////////////////////////////////////////////////////////
CTextOnScreenMngrForInvitedSession::CTextOnScreenMngrForInvitedSession(CConf* pConf)
                                   :CTextOnScreenMngrForGwSession(pConf)
{
}

//--------------------------------------------------------------------------
CTextOnScreenMngrForInvitedSession::CTextOnScreenMngrForInvitedSession(const CTextOnScreenMngrForInvitedSession& other)
                                   :CTextOnScreenMngrForGwSession(other)
{
}

//--------------------------------------------------------------------------
CTextOnScreenMngrForInvitedSession::~CTextOnScreenMngrForInvitedSession(void)
{
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForInvitedSession::AddMsgToLine(CTextOnScreenMsg* pTextLine, DWORD line, DWORD timeout, BYTE allowOverride, DWORD partyId)
{
  CMedString              cstr;
  cstr << "CTextOnScreenMngrForInvitedSession::AddMsgToLine  line: " << line << ", msg: " <<  pTextLine->GetTextLine() << ", timeout = " << timeout;
  TEXT_MSG_LIST::iterator it;
  if (line >= Size())
  {
    cstr << "line > Size ( line=" << line <<" , size=" << Size() << " )\n";
    int i = 0;
    for (it = m_TextMsgList.end(); it != m_TextMsgList.begin()+line; ++it)
    {
      cstr << "insert dummy message [" << ++i << "]\n";
      CTextOnScreenMsg dummyMsg;
      AddMsg(&dummyMsg);
    }

    cstr << "insert original message";
    AddMsg(pTextLine);
  }
  else
  {
    cstr << "updating line message\n";
    it = Begin()+line;
    if (CPObject::IsValidPObjectPtr(*it))
      **it = *pTextLine;
    else
      PASSERT_AND_RETURN(line);
  }

  m_pConf->SendPartyMsgOnScreenForDtmfInvitePartyConf(partyId);
  PTRACE(eLevelInfoNormal, cstr.GetString());
  DumpVector();
  Dump();

  if (timeout)
  {
    m_LineInfo[line]->StartTimerForParty(this, timeout);
    m_LineInfo[line]->SetIsActiveTimer(TRUE);
    m_LineInfo[line]->SetIsAllowOverride(allowOverride);
  }
}

//--------------------------------------------------------------------------
void CTextOnScreenMngrForInvitedSession::RemoveLine(DWORD partyId, DWORD line)
{
  TRACEINTO << "CTextOnScreenMngrForInvitedSession::RemoveLine - PartyId:" << partyId << ", Line:" << line;

  PASSERT_AND_RETURN(line >= m_LineInfo.size());

  TEXT_LINE_INFO::iterator it = m_LineInfo.begin()+line;
  CDataForLine* pData = *it;
  m_LineInfo.erase(it);
  POBJDELETE(pData);

  PASSERT_AND_RETURN(line >= Size());

  TEXT_MSG_LIST::iterator itr = m_TextMsgList.begin()+line;
  CTextOnScreenMsg* pRemovedTextMsg = (*itr);
  m_TextMsgList.erase(itr);
  POBJDELETE(pRemovedTextMsg);

  m_pConf->SendPartyMsgOnScreenForDtmfInvitePartyConf(partyId);
}
