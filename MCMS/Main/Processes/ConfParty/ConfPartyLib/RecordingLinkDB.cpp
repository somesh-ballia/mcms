#include "RecordingLinkDB.h"
#include "ConfPartyApiDefines.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "ConfPartyDefines.h"
#include "ConfPartyProcess.h"
#include <fcntl.h>

struct SAscendingPartyIdSort
{
     bool operator()(CRsrvRecordLinkPartyAdd* const& rpStart, CRsrvRecordLinkPartyAdd* const& rpEnd)
     {
          return rpStart->GetRsrvParty()->GetPartyId() < rpEnd->GetRsrvParty()->GetPartyId();
     }
};

//////////////////////////////////////////////////////////////////////////////////
CRecordingLinkDB::CRecordingLinkDB() : m_pFileManager(0)
{
  m_default[0]      = '\0';
  m_partyIdCounter  = 0;
  m_updateCounter   = 0;
  m_bChanged        = FALSE;
  m_prefix          = "recordlink_ser_";
}
//////////////////////////////////////////////////////////////////////////////////
CRecordingLinkDB::CRecordingLinkDB(CRecordingLinkDB& other)
                 :CSerializeObject(other), m_pFileManager(new CFileManager<CRsrvRecordLinkPartyAdd>(*other.m_pFileManager))
{
  *this = other;
}
//////////////////////////////////////////////////////////////////////////////////
CRecordingLinkDB& CRecordingLinkDB::operator=(const CRecordingLinkDB &other)
{
  if(this == &other){
	  return *this;
  }

  //Clear the parties vector
  for (RsrvPartyList::iterator itr = m_partyList.begin(); itr != m_partyList.end(); ++itr)
    POBJDELETE(*itr);

  m_partyList.clear();

  for (RsrvPartyList::const_iterator it = other.m_partyList.begin(); it != other.m_partyList.end(); ++it)
    m_partyList.push_back(new CRsrvParty(*(*it)));

  POBJDELETE(m_pFileManager);

  if (other.m_pFileManager != NULL)
    m_pFileManager = new CFileManager<CRsrvRecordLinkPartyAdd>(*other.m_pFileManager);

  strncpy(m_default, other.m_default, H243_NAME_LEN);
  m_default[H243_NAME_LEN - 1] = '\0';

  m_partyIdCounter = other.m_partyIdCounter;
  m_updateCounter = other.m_updateCounter;
  m_bChanged = other.m_bChanged;

  return *this;
}
//////////////////////////////////////////////////////////////////////////////////
CRecordingLinkDB::~CRecordingLinkDB()
{
  POBJDELETE(m_pFileManager);

  for (RsrvPartyList::iterator itr = m_partyList.begin(); itr != m_partyList.end(); ++itr)
    POBJDELETE(*itr);
  m_partyList.clear();
}
//////////////////////////////////////////////////////////////////////////////////
CSerializeObject* CRecordingLinkDB::Clone()
{
  return new CRecordingLinkDB();
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  //!!!! in order to cancel the const on the method
  CRecordingLinkDB* pNode = ((CRecordingLinkDB*)this);
  if (!pFatherNode)
  {
    pFatherNode = new CXMLDOMElement;
    pFatherNode->set_nodeName("RECORDING_LINKS_LIST");
  }
  pNode->SerializeXml(pFatherNode, 0xFFFFFFFF);
}
//////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDB::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const char* action)
{
  CXMLDOMElement *pChildNode, *pVisualEffectsNode, *pServiceNode;
  DWORD nVal;
  BYTE nByteVal;
  int nStatus = STATUS_OK;

  m_bChanged = TRUE;
  GET_VALIDATE_CHILD(pResNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pResNode, "CHANGED", &m_bChanged, _BOOL);
  GET_VALIDATE_CHILD(pResNode, "REC_LINK_DEFAULT_NAME", m_default, _0_TO_H243_NAME_LENGTH);

  if (m_bChanged)
  {
    CXMLDOMElement* pChildNode = NULL;
    GET_CHILD_NODE(pResNode, "PARTY_LIST", pChildNode);
    if (pChildNode)
    {
      nStatus = DeSerializePartyListXml(pChildNode, pszError, UPDATE_PARTY);
      if (nStatus != STATUS_OK)
        return nStatus;
    }
  }
  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken)
{
  CXMLDOMElement *pTokenNode, *pResNode;
  unsigned char bChanged = FALSE;
  DWORD CurrentCounter = 0;

  char* nodeName;
  pActionNode->get_nodeName(&nodeName);
  pResNode = pActionNode->AddChildNode("RECORDING_LINKS_LIST");

  if (ObjToken == 0xFFFFFFFF || m_updateCounter > ObjToken)
    bChanged = TRUE;

  pTokenNode = pResNode->AddChildNode("OBJ_TOKEN", m_updateCounter);
  pResNode->AddChildNode("CHANGED", bChanged, _BOOL);
  pResNode->AddChildNode("REC_LINK_DEFAULT_NAME", m_default);

  if (bChanged)
    AddPartiesXmlToResponse(pResNode);
}
//////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDB::DeSerializePartyListXml(CXMLDOMElement* pListNode, char* pszError, int nAction)
{
  CXMLDOMElement* pPartyNode = NULL;

  GET_FIRST_CHILD_NODE(pListNode, "PARTY", pPartyNode);
  DWORD maxPartyId = 0;

  while (pPartyNode)
  {
    CRsrvParty* pParty = new CRsrvParty;

    int nStatus = pParty->DeSerializeXml(pPartyNode, pszError, nAction);
    if (nStatus != STATUS_OK)
    {
      POBJDELETE(pParty);
      return nStatus;
    }

    Add(*pParty);

    maxPartyId = std::max(maxPartyId, pParty->GetPartyId());
    POBJDELETE(pParty);

    GET_NEXT_CHILD_NODE(pListNode, "PARTY", pPartyNode);
  }

  //Setting the party Id counter if we find a party
  WORD numParties = m_partyList.size();
  if (numParties > 0)
    m_partyIdCounter = ++maxPartyId;

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::AddPartiesXmlToResponse(CXMLDOMElement* pResNode)
{
  CXMLDOMElement *pPartyListNode = pResNode->AddChildNode("PARTY_LIST");
  for (RsrvPartyList::iterator itr = m_partyList.begin(); itr != m_partyList.end(); ++itr)
    (*itr)->SerializeXml(pPartyListNode, FULL_DATA);
}
//////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDB::Add(const CRsrvParty& other, BOOL isResFromProfilesFolder)
{
  WORD numParties = m_partyList.size();

  if (numParties >= MAX_RECORDING_LINKS_IN_LIST)
  {
    PTRACE(eLevelInfoNormal, "CRecordingLinkDB::Add - Failed, cannot add RecordingLink (MAX_RECORDING_LINKS_IN_LIST reached)");
    return STATUS_MAX_RECORDING_LINKS_REACHED;
  }

  if (IsNameExist(other.GetName()) == YES)
  {
    PTRACE2(eLevelInfoNormal, "CRecordingLinkDB::Add - Failed, cannot add RecordingLink (name already exist), partyName:", other.GetName());
    return STATUS_PARTY_NAME_EXISTS;
  }

  CRsrvRecordLinkPartyAdd* pRecordLinkPartyAdd = new CRsrvRecordLinkPartyAdd(new CRsrvParty(other));

  if (!isResFromProfilesFolder && (m_pFileManager->AddFileData(*pRecordLinkPartyAdd, GetFileUniqueName(m_prefix, other.GetPartyId())) != STATUS_OK))
  {
    POBJDELETE(pRecordLinkPartyAdd);
    PASSERTMSG_AND_RETURN_VALUE(1, "CRecordingLinkDB::Add - Failed, cannot add RecordingLink (file operation error)", STATUS_FAIL);
  }

  //In order to remove the leak
  CRsrvParty* RsrvParty = new CRsrvParty(*(pRecordLinkPartyAdd->GetRsrvParty()));
  POBJDELETE(pRecordLinkPartyAdd);
  m_partyList.push_back(RsrvParty);

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDB::Update(CRsrvParty& other)
{
  RsrvPartyList::iterator itr = FindName(other.GetName());
  if (itr == m_partyList.end())
  {
    PTRACE2(eLevelInfoNormal, "CRecordingLinkDB::Update - Failed, cannot find RecordingLink, partyName:", other.GetName());
    return STATUS_PARTY_DOES_NOT_EXIST;
  }

  CRsrvParty* pRsrvParty = new CRsrvParty(other);
  pRsrvParty->SetPartyId((*itr)->GetPartyId()); //return the correct party id
  *(*itr) = (*pRsrvParty);

  //First delete the old file from the folder
  if (m_pFileManager->DeleteFileData(GetFileUniqueName(m_prefix, other.GetPartyId())) != STATUS_OK) {
    PASSERTMSG_AND_RETURN_VALUE(1, "CRecordingLinkDB::Update - Failed, cannot delete RecordingLink (file operation error)", STATUS_FAIL);
  }

  CRsrvRecordLinkPartyAdd pRecordLinkPartyAdd;
  pRecordLinkPartyAdd.SetRsrvParty(&other);

  //Second Add the new file to the folder
  if (m_pFileManager->AddFileData(pRecordLinkPartyAdd, GetFileUniqueName(m_prefix, other.GetPartyId())) != STATUS_OK) {
    PASSERTMSG_AND_RETURN_VALUE(2, "CRecordingLinkDB::Update - Failed, cannot add RecordingLink (file operation error)", STATUS_FAIL);
  }

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDB::Cancel(const char* name)
{
  RsrvPartyList::iterator itr = FindName(name);
  if (itr == m_partyList.end())
  {
    PTRACE2(eLevelInfoNormal, "CRecordingLinkDB::Cancel - Failed, cannot find RecordingLink, partyName:", name);
    return STATUS_PARTY_DOES_NOT_EXIST;
  }

  CRsrvParty* pRsrvParty = *itr;
  m_partyList.erase(itr);

  //remove the Res from Dir
  if (m_pFileManager->DeleteFileData(GetFileUniqueName(m_prefix, pRsrvParty->GetPartyId())) != STATUS_OK)
  {
    POBJDELETE(pRsrvParty);
    PASSERTMSG_AND_RETURN_VALUE(1, "CRecordingLinkDB::Cancel - Failed, cannot delete RecordingLink (file operation error)", STATUS_FAIL);
  }
  POBJDELETE(pRsrvParty);
  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDB::Cancel(const DWORD partyId)
{
  RsrvPartyList::iterator itr = FindId(partyId);
  if (itr == m_partyList.end())
  {
    PTRACE2INT(eLevelInfoNormal, "CRecordingLinkDB::Cancel - Failed, cannot find RecordingLink, partyId:", partyId);
    return STATUS_PARTY_DOES_NOT_EXIST;
  }

  CRsrvParty* pRsrvParty = *itr;
  m_partyList.erase(itr);

  //remove the Res from Dir
  if (m_pFileManager->DeleteFileData(GetFileUniqueName(m_prefix, pRsrvParty->GetPartyId())) != STATUS_OK)
  {
    POBJDELETE(pRsrvParty);
    PASSERTMSG_AND_RETURN_VALUE(1, "CRecordingLinkDB::Cancel - Failed, cannot delete RecordingLink (file operation error)", STATUS_FAIL);
  }
  POBJDELETE(pRsrvParty);
  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
WORD CRecordingLinkDB::GetNumParties()
{
  return m_partyList.size();
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CRecordingLinkDB::TestPartyRsrvValidity(CRsrvParty* pRsrvParty)
{
  PTRACE2(eLevelInfoNormal, "CRecordingLinkDB::TestPartyRsrvValidity - RsrvPartyName:", pRsrvParty->GetName());
  BYTE errorCode = 0;
  STATUS status = STATUS_OK;

  pRsrvParty->SetPartyDefaultParams();
  status = pRsrvParty->CheckReservRangeValidity(errorCode);

  if (status == STATUS_OK)
    status = TestRecordingLinkPartyValidity(pRsrvParty);

  if (status != STATUS_OK)
    PTRACE2(eLevelInfoNormal, "CRecordingLinkDB::TestPartyRsrvValidity - Failed, Status:", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
  return status;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CRecordingLinkDB::TestRecordingLinkPartyValidity(CRsrvParty* pRsrvParty)
{
  STATUS status;
  CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

  const char* partyName = pRsrvParty->GetName();
  if (partyName == NULL)
    return STATUS_ILLEGAL_PARTY_NAME;

  if (!strcmp(partyName, ""))
    return STATUS_ILLEGAL_PARTY_NAME;

  //Check party name for invalid characters
  char* illigalChar = NULL;
  illigalChar = (char*)strrchr(partyName, ',');
  if (illigalChar != NULL)
    return STATUS_INVALID_CHARACTER_IN_PARTY_NAME;
  illigalChar = NULL;
  illigalChar = (char*)strrchr(partyName, ';');
  if (illigalChar != NULL)
    return STATUS_INVALID_CHARACTER_IN_PARTY_NAME;

  // system.cfg flags tests will be tested only after adding the recording party to conf

  // NET service - in this version we support only H323 recording links
  // Other NET Service validity tests will be tested only after adding the recording party to conf
  const char* serv = pRsrvParty->GetServiceProviderName();
  BYTE interfaceType = pRsrvParty->GetNetInterfaceType();

  //In V7.6 we add the option for SIP recorder
  if (interfaceType != H323_INTERFACE_TYPE && interfaceType != SIP_INTERFACE_TYPE)
  {
    return STATUS_INVALID_RECORDING_LINK_NETWORK_SETTINGS;
  }
  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
CRsrvParty* CRecordingLinkDB::GetParty(const char* name)
{
  RsrvPartyList::iterator itr = FindName(name);
  return (itr == m_partyList.end()) ? NULL : (*itr);
}
//////////////////////////////////////////////////////////////////////////////////
CRsrvParty* CRecordingLinkDB::GetFirstParty()
{
  if (m_partyList.size() == 0)
    return NULL;
  RsrvPartyList::iterator itr = m_partyList.begin();
  return (*itr);
}
//////////////////////////////////////////////////////////////////////////////////
CRsrvParty* CRecordingLinkDB::GetPartyById(DWORD partyId)
{
  RsrvPartyList::iterator itr = FindId(partyId);
  return (itr == m_partyList.end()) ? NULL : (*itr);
}
//////////////////////////////////////////////////////////////////////////////////
const char* CRecordingLinkDB::GetDefaultRecordingLinkName() const
{
  return m_default;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::SetDefaultRecordingLinkName(const char* defaultName)
{
  if (defaultName)
  {
    strncpy(m_default, defaultName, H243_NAME_LEN - 1);  //For KlockWork
    m_default[H243_NAME_LEN - 1] = '\0';
  }
}
//////////////////////////////////////////////////////////////////////////////////
DWORD CRecordingLinkDB::GetUpdateCounter() const
{
  return m_updateCounter;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::SetUpdateCounter(DWORD updateCounter)
{
  m_updateCounter = updateCounter;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::IncreaseUpdateCounter()
{
  m_updateCounter++;
  if (m_updateCounter == 0xFFFFFFFF)
    m_updateCounter = 0;
}
//////////////////////////////////////////////////////////////////////////////////
RsrvPartyList::iterator CRecordingLinkDB::FindName(const char* name)
{
  for (RsrvPartyList::iterator itr = m_partyList.begin(); itr != m_partyList.end(); ++itr)
  {
    if (CPObject::IsValidPObjectPtr(*itr))
      if (!strncmp((*itr)->GetName(), name, H243_NAME_LEN))
        return itr;
  }
  return m_partyList.end();
}
//////////////////////////////////////////////////////////////////////////////////
RsrvPartyList::iterator CRecordingLinkDB::FindId(DWORD partyID)
{
  for (RsrvPartyList::iterator itr = m_partyList.begin(); itr != m_partyList.end(); ++itr)
  {
    if (CPObject::IsValidPObjectPtr(*itr))
      if ((*itr)->GetPartyId() == partyID)
        return itr;
  }
  return m_partyList.end();
}
//////////////////////////////////////////////////////////////////////////////////
BYTE CRecordingLinkDB::IsNameExist(const char* name)
{
  RsrvPartyList::iterator itr = FindName(name);
  return (itr == m_partyList.end()) ? NO : YES;
}
//////////////////////////////////////////////////////////////////////////////////
DWORD CRecordingLinkDB::NextPartyId()
{
  DWORD lastId = m_partyIdCounter;

  if (m_partyIdCounter < HALF_MAX_DWORD)
    m_partyIdCounter++;
  else
    m_partyIdCounter = 0;

  return lastId;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::SetNextPartyId(DWORD nextPartyId)
{
  m_partyIdCounter = nextPartyId;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CRecordingLinkDB::SetFolderPath(std::string dirName)
{
  mode_t mode = 0755;

  POBJDELETE(m_pFileManager);
  PTRACE2(eLevelInfoNormal, "CRecordingLinkDB::SetFolderPath: ", dirName.c_str());

  if (open(dirName.c_str(), O_DIRECTORY) == -1) {//Create the Folder only if needed
    if (CreateDirectory(dirName.c_str(), mode) == FALSE) {
      PASSERTMSG_AND_RETURN_VALUE(1, "CRecordingLinkDB::SetFolderPath - Failed, cannot create RecordingLink Service directory", STATUS_FAIL);
    }
  }

  m_pFileManager = new CFileManager<CRsrvRecordLinkPartyAdd> (dirName);
  std::vector<CRsrvRecordLinkPartyAdd *> vect;

  // Update the ShortRes Array with the loaded array
  if ((m_pFileManager->LoadDataToVect(vect)) != STATUS_OK)
  {
    ClearRLVector(vect);
    PASSERTMSG_AND_RETURN_VALUE(2, "CRecordingLinkDB::SetFolderPath - Failed, cannot load all the profiles from FileManager", STATUS_FAIL);
  }

  if (vect.size() == 0) // This maybe the first time;
    return STATUS_OK;

  // Sort the vector according to partyid
  std::sort(vect.begin(), vect.end(), SAscendingPartyIdSort());

  // First set the Items Counter to the max counter
  WORD i = 0;
  for (std::vector<CRsrvRecordLinkPartyAdd *>::iterator it = vect.begin(); it != vect.end() && i < MAX_RECORDING_LINKS_IN_LIST; ++it, ++i)
  {
    CRsrvParty* pRsrvParty = (*it)->GetRsrvParty();
    STATUS status = TestRLServiceValidity(pRsrvParty);
    if (STATUS_OK == status)
    {
      if (Add(*pRsrvParty) == STATUS_OK) //Add each one to the DB
      {
        if (1 == GetNumParties())
          SetDefaultRecordingLinkName(pRsrvParty->GetName());
        IncreaseUpdateCounter();
        continue;
      }
      PASSERTMSG(3, "CRecordingLinkDB::SetFolderPath - Failed to add Recording Link to reservation party list");
    }
  }
  ClearRLVector(vect);
  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CRecordingLinkDB::TestRLServiceValidity(CRsrvParty* pRsrvParty)
{
  std::ostringstream msg;
  msg << "CRecordingLinkDB::TestRLServiceValidity - RsrvPartyName:" << pRsrvParty->GetName() << ", RsrvPartyId:" << pRsrvParty->GetPartyId();
  PTRACE(eLevelInfoNormal, msg.str().c_str());

  if (GetNumParties() >= MAX_RECORDING_LINKS_IN_LIST) {
    PASSERTMSG_AND_RETURN_VALUE(1, "CRecordingLinkDB::TestRLServiceValidity - Failed, maximum Recording Links is reached", STATUS_MAX_RECORDING_LINKS_REACHED);
  }

  if (GetParty(pRsrvParty->GetName()) != NULL) {
    PASSERTMSG_AND_RETURN_VALUE(2, "CRecordingLinkDB::TestRLServiceValidity - Failed, Recording Link already exist in reservation party list", STATUS_PARTY_NAME_EXISTS);
  }

  if (pRsrvParty->GetPartyId() <= HALF_MAX_DWORD || pRsrvParty->GetPartyId() == 0xFFFFFFFF)
  {
    DWORD nextPartyId = NextPartyId();

    // Remove the resource from folder
    if (m_pFileManager->DeleteFileData(GetFileUniqueName(m_prefix, pRsrvParty->GetPartyId())) != STATUS_OK) {
      PASSERTMSG_AND_RETURN_VALUE(3, "CRecordingLinkDB::TestRLServiceValidity - Failed, cannot delete the Recording Link file", STATUS_FAIL);
    }

    ((CRsrvParty*)pRsrvParty)->SetPartyId(nextPartyId);
  }

  return TestPartyRsrvValidity(pRsrvParty);
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::ClearRLVector(std::vector<CRsrvRecordLinkPartyAdd*>& vect)
{
  PTRACE(eLevelInfoNormal, "CRecordingLinkDB::ClearRLVector");

  for (std::vector<CRsrvRecordLinkPartyAdd *>::iterator it = vect.begin(); it != vect.end(); ++it)
    POBJDELETE(*it);

  vect.clear();
}
//////////////////////////////////////////////////////////////////////////////////
std::string CRecordingLinkDB::GetFileUniqueName(const std::string& dbPrefix, int PartyId)const
{
  char PartyName[H243_NAME_LEN+5+sizeof(".xml")+1];
  snprintf(PartyName, H243_NAME_LEN+5+sizeof(".xml"), "%s%05d.xml", dbPrefix.c_str(), PartyId);
  return PartyName;
}
//////////////////////////////////////////////////////////////////////////////////
void CRecordingLinkDB::ResetDB()
{
  IncreaseUpdateCounter();

  m_pFileManager->DeleteVectData();

  m_partyIdCounter = 0;
  m_partyList.clear();
  m_default[0] = '\0';
  POBJDELETE(m_pFileManager);

  m_pFileManager = new CFileManager<CRsrvRecordLinkPartyAdd> (FILE_RECORDLINK_SRV_DB);
}

