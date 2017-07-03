#include "CommResShort.h"
#include "CommRes.h"
#include "psosxml.h"
#include "H323Alias.h"
#include <utility>
#include "ConfPartyStatuses.h"
#include "TraceStream.h"
#include "IpServiceListManager.h"
#include "ConfPartyGlobals.h"
#include "IVRServiceList.h"
#include "ProcessSettings.h"
#include "HlogApi.h"
#include "FaultsDefines.h"
#include "CdrApiClasses.h"
#include "ApiStatuses.h"
#include <algorithm>
#include <memory>

extern CIpServiceListManager* GetIpServiceListMngr();
extern WORD GetMaxConfTemplates();

////////////////////////////////////////////////////////////////////////////
//                        CCommResDB
////////////////////////////////////////////////////////////////////////////
CCommResDB::CCommResDB(WORD DBtype) : CSerializeObject(),
  m_dwDBCounter(NULL),
  m_dwDBHighestInsertedCounter(NULL),
  m_dwSummaryUpdateCounter(0),
  m_dwFullUpdateCounter(0),
  m_LastDeletedIndex(0),
  m_adHocGwNidCounter(0),
  m_DBtype(DBtype),
  m_bChanged(FALSE),
  m_pFileManager(0)
{
  m_prefixMap[PROFILES_DATABASE]       = "profile_";
  m_prefixMap[MEETING_ROOM_DATABASE]   = "meeting_room_";
  m_prefixMap[CONF_TEMPLATES_DATABASE] = "template_";

  memset(m_DeletedCounterHistory, 0, sizeof(m_DeletedCounterHistory));
  memset(m_DeletedIdHistory, 0, sizeof(m_DeletedIdHistory));

  m_transitEQ[0] = '\0';

  switch (m_DBtype)
  {
    case PROFILES_DATABASE:
      m_dwDBCounter                = &::_dwProfileIndexCounter;
      m_dwDBHighestInsertedCounter = &::_dwProfileHighestExistingCounter;
      m_sUniqeRoutingName          = &_sProfileUniqeRoutingName;
      break;

    case CONF_TEMPLATES_DATABASE:
      m_dwDBCounter                = &_dwTemplateIndexCounter;
      m_dwDBHighestInsertedCounter = &_dwTemplateHighestExistingCounter;
      m_sUniqeRoutingName          = &_sTemplateUniqeRoutingName;
      break;

    default:
      m_dwDBCounter                = &_dwMRIndexCounter;
      m_dwDBHighestInsertedCounter = &_dwMRHighestExistingCounter;
      m_sUniqeRoutingName          = &_sMRUniqeRoutingName;
      break;
  }
}

//--------------------------------------------------------------------------
CCommResDB::CCommResDB(const CCommResDB& other) : CSerializeObject(other),
  m_dwDBCounter(other.m_dwDBCounter),
  m_dwDBHighestInsertedCounter(other.m_dwDBHighestInsertedCounter),
  m_dwSummaryUpdateCounter(other.m_dwSummaryUpdateCounter),
  m_dwFullUpdateCounter(other.m_dwFullUpdateCounter),
  m_LastDeletedIndex(other.m_LastDeletedIndex),
  m_adHocGwNidCounter(other.m_adHocGwNidCounter),
  m_DBtype(other.m_DBtype),
  m_bChanged(other.m_bChanged),
  m_pFileManager(new CFileManager<CCommRes>(*other.m_pFileManager))
{
  m_prefixMap[PROFILES_DATABASE]       = "profile_";
  m_prefixMap[MEETING_ROOM_DATABASE]   = "meeting_room_";
  m_prefixMap[CONF_TEMPLATES_DATABASE] = "template_";
  m_transitEQ[0] = '\0';
  SAFE_COPY(m_transitEQ, other.m_transitEQ);

  for (ReservArray::const_iterator it = other.m_reservArray.begin(); it != other.m_reservArray.end(); ++it)
  {
    CCommResShort* pShort = new CCommResShort();
    *pShort = *(*it);
    m_reservArray.push_back(pShort);
  }

  memcpy(m_DeletedCounterHistory, other.m_DeletedCounterHistory, sizeof(m_DeletedCounterHistory));
  memcpy(m_DeletedIdHistory, other.m_DeletedIdHistory, sizeof(m_DeletedIdHistory));
}

//--------------------------------------------------------------------------
CCommResDB& CCommResDB::operator=(const CCommResDB& other)
{
  if (&other == this)
    return *this;

  // Clean the rsrvArray
  ClearVector();

  // Copy all rsrv
  for (ReservArray::const_iterator it = other.m_reservArray.begin(); it != other.m_reservArray.end(); ++it)
  {
    CCommResShort* pShort = new CCommResShort();
    *pShort = *(*it);
    m_reservArray.push_back(pShort);
  }

  memcpy(m_DeletedCounterHistory, other.m_DeletedCounterHistory, sizeof(m_DeletedCounterHistory));
  memcpy(m_DeletedIdHistory, other.m_DeletedIdHistory, sizeof(m_DeletedIdHistory));

  m_dwSummaryUpdateCounter     = other.m_dwSummaryUpdateCounter;
  m_DBtype                     = other.m_DBtype;
  m_LastDeletedIndex           = other.m_LastDeletedIndex;
  m_bChanged                   = other.m_bChanged;
  m_dwDBCounter                = other.m_dwDBCounter;
  m_dwDBHighestInsertedCounter = other.m_dwDBHighestInsertedCounter;
  m_sUniqeRoutingName          = other.m_sUniqeRoutingName;
  m_adHocGwNidCounter          = other.m_adHocGwNidCounter;

  POBJDELETE(m_pFileManager);

  if (other.m_pFileManager != NULL)
    m_pFileManager = new CFileManager<CCommRes>(*other.m_pFileManager);

  m_transitEQ[0] = '\0';
  SAFE_COPY(m_transitEQ, other.m_transitEQ);

  return *this;
}

//--------------------------------------------------------------------------
CCommResDB::~CCommResDB()
{
  ClearVector();

  POBJDELETE(m_pFileManager);
}

//--------------------------------------------------------------------------
void CCommResDB::SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken)
{
  bool            bChanged          = false;
  int             opcode            = CONF_NOT_CHANGED;
  DWORD           ResSummeryCounter = 0xFFFFFFFF;
  CXMLDOMElement* pSumListNode      = NULL;

  switch (m_DBtype)
  {
    case MEETING_ROOM_DATABASE  : pSumListNode = pActionNode->AddChildNode("MEETING_ROOM_SUMMARY_LS"); break;
    case PROFILES_DATABASE      : pSumListNode = pActionNode->AddChildNode("PROFILE_SUMMARY_LS"); break;
    case CONF_TEMPLATES_DATABASE: pSumListNode = pActionNode->AddChildNode("CONFERENCE_TEMPLATE_SUMMARY_LS"); break;
    default                     : PASSERTMSG_AND_RETURN(1, "CCommResDB::SerializeXml - Failed, invalid DB type");
  }

  pSumListNode->AddChildNode("OBJ_TOKEN", m_dwSummaryUpdateCounter);

  if (ObjToken == 0xFFFFFFFF)
    bChanged = true;
  else
  {
    // in case the string can not be converted ResSummeryCounter=0 the conferences will be added
    // as if the user sent -1 in the object token .
    ResSummeryCounter = ObjToken;
    if (m_dwSummaryUpdateCounter > ResSummeryCounter)
      bChanged = true;
  }

  if (!bChanged)
  {
    pSumListNode->AddChildNode("CHANGED", FALSE, _BOOL);
    return;
  }

  CXMLDOMElement* pChangedNode = pSumListNode->AddChildNode("CHANGED", TRUE, _BOOL);
  CXMLDOMElement* pDeletedNode = pSumListNode->AddChildNode("DELETED_RES_LIST");
  for (int i = 0; i < HISTORY_SIZE; i++)
    if (m_DeletedCounterHistory[i] > ResSummeryCounter && ResSummeryCounter != 0xFFFFFFFF)
      pDeletedNode->AddChildNode("ID", m_DeletedIdHistory[i]);

  if (m_DBtype == MEETING_ROOM_DATABASE)
    pSumListNode->AddChildNode("DEFAULT_EQ_NAME", m_transitEQ);

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    PASSERTMSG_AND_RETURN(!CPObject::IsValidPObjectPtr(*it), "CCommResDB::SerializeXml - Failed, object not valid in vector");

    // VNGR-23571 - GW IVR Service and Gateway Profile should be unavailable in COP mode.
    if (::GetIsCOPdongleSysMode() && strcmp((*it)->GetName(), "Factory_GW_Profile") == 0)
      continue;

    if ((int)(*it)->GetSummeryCreationUpdateCounter() > (int)ResSummeryCounter)
      opcode = CONF_COMPLETE_INFO;
    else if ((int)(*it)->GetSummeryUpdateCounter() > (int)ResSummeryCounter)
      opcode = CONF_FAST_PLUS_SLOW_INFO;
    else
      opcode = CONF_NOT_CHANGED;

    (*it)->SerializeXml(pSumListNode, opcode);
  }
}

//--------------------------------------------------------------------------
int CCommResDB::DeSerializeXml(CXMLDOMElement *pListNode, char *pszError, const char* action)
{
  PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::DeSerializeXml - This Code Should not be invoked", STATUS_FAIL);
}

//--------------------------------------------------------------------------
int CCommResDB::Add(CCommRes& otherResConf, bool isResFromProfilesFolder)
{
  if (isResFromProfilesFolder)
	TRACEINTO << "CCommResDB::Add - From Profile Folder";

  PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(&otherResConf), STATUS_FAIL);
  PASSERT_AND_RETURN_VALUE(!m_pFileManager, STATUS_FAIL);

  if(m_DBtype == CONF_TEMPLATES_DATABASE && m_reservArray.size() >= GetMaxConfTemplates())
  {
    TRACEINTO << "CCommResDB::Add - The new template cound NOT be added due to exceed the max number";
    return STATUS_MAX_CONF_TEMPLATES_EXCEEDED;
  }

  if(m_DBtype == PROFILES_DATABASE && m_reservArray.size() >= MAX_TEMPLATES_IN_LIST)
  {
    TRACEINTO << "CCommResDB::Add - The new profile could NOT be added due to exceed the max number";
    return STATUS_MAX_PROFILES_EXCEEDED;
  }

  if (strlen(otherResConf.GetName()) == 0 && strlen(otherResConf.GetDisplayName()) == 0)
  {
    TRACEINTO << " Profile/MR/EQ xml are bad because the routing and display name are empty - will not be added to DB";
    return STATUS_ILLEGAL_RESERVATION_NAME;
  }

  CCommResShort* shortRes = new CCommResShort;
  shortRes->SetFileUniqueName(otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]));
  shortRes->SetName(otherResConf.GetName());
  shortRes->SetDisplayName(otherResConf.GetDisplayName());
  shortRes->SetPassw(otherResConf.GetH243Password());
  shortRes->SetConferenceId(otherResConf.GetMonitorConfId());
  shortRes->SetStartTime(*otherResConf.GetStartTime());
  shortRes->SetDurationTime(*otherResConf.GetDurationTime());
  shortRes->SetPermanent(otherResConf.IsPermanent());
  shortRes->SetOperatorConf(otherResConf.GetOperatorConf());
  shortRes->SetEntryPassword(otherResConf.GetEntryPassword());
  shortRes->SetMeetMePerEntryQ(otherResConf.GetMeetMePerEntryQ());
  shortRes->SetNumericConfId(otherResConf.GetNumericConfId());
  shortRes->SetNumParties(otherResConf.GetNumParties());
  shortRes->SetNumUndefParties(otherResConf.GetNumUndefParties());
  shortRes->SetConfTransferRate(otherResConf.GetConfTransferRate());
  shortRes->SetNetwork(otherResConf.GetNetwork());
  shortRes->SetAdHocProfileId(otherResConf.GetAdHocProfileId());
  shortRes->SetContinuousPresenceScreenNumber(otherResConf.GetContinuousPresenceScreenNumber());
  shortRes->SetAutoLayoutFlag(otherResConf.GetIsAutoLayout());
  shortRes->SetIsTelePresenceMode(otherResConf.GetIsTelePresenceMode());
  shortRes->SetHDVSW(otherResConf.GetIsHDVSW());
  shortRes->SetIsTipCompatible(otherResConf.GetIsTipCompatible());
  shortRes->SetNatKAPeriod(otherResConf.GetNatKAPeriod());
  shortRes->SetconfMediaType( otherResConf.GetConfMediaType());

  for (int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
  {
    const char* pContactInfo = otherResConf.GetConfContactInfo(i);
    if (pContactInfo)
      shortRes->SetConfContactInfo(pContactInfo, i);
  }

  // Every new reservation enters with operator mask = TRUE
  COPERMASKONINRSRVSHORT(shortRes);

  // Meeting room parameters
  shortRes->SetMeetingRoomState(otherResConf.GetMeetingRoomState());
  otherResConf.SetServicePhoneToShort(*shortRes);
  otherResConf.CopyToShortAllPrefixStr(*shortRes);

  shortRes->SetRsrvFlags(otherResConf.CalcRsrvFlags());
  shortRes->SetRsrvFlags2(otherResConf.CalcRsrvFlags2());
  shortRes->SetRsrvEncryptionType(otherResConf.GetEncryptionType());
  DWORD sts = otherResConf.CheckCurrReservSysMode();                                          // Status Field
  if (sts != STATUS_OK)
    shortRes->SetResSts(eWrongSysMode);                                                       // Status Field
  else
    shortRes->SetResSts(eStsOK);

  shortRes->SetSipRegistrationTotalSts(otherResConf.GetSipRegistrationTotalSts());            // sipProxySts
  for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
  {
    shortRes->SetServiceRegistrationContentServiceNm(i, otherResConf.GetServiceRegistrationContentServiceName(i));
    shortRes->SetServiceRegistrationContentReg(i, otherResConf.GetServiceRegistrationContentRegister(i));
    shortRes->SetServiceRegistrationContentAccept_Call(i, otherResConf.GetServiceRegistrationContentAcceptCall(i));
    shortRes->SetSipRegistrationSts(i, otherResConf.GetServiceRegistrationContentStatus(i));  // sipProxySts1
  }

  if (FindName(otherResConf.GetName()) != m_reservArray.end() || FindName(otherResConf.GetDisplayName(), 1) != m_reservArray.end())
  {
    POBJDELETE(shortRes);
    switch (m_DBtype)
    {
      case PROFILES_DATABASE      : return STATUS_PROFILE_NAME_ALREADY_EXISTS;
      case RESERVATION_DATABASE   : return STATUS_RESERVATION_NAME_EXISTS;
      case MEETING_ROOM_DATABASE  : return STATUS_RESERVATION_NAME_EXISTS;
      case CONF_TEMPLATES_DATABASE: return STATUS_CONF_TEMPLATE_NAME_ALREADY_EXISTS;
    }
    PASSERT(m_DBtype+1);
    return STATUS_RESERVATION_NAME_EXISTS;
  }

  /* Write to file only if the file comes from the operator
  * (a file can come in the init stage when reading all files from folder,
  * in this case no need to write to the disk again */



  if (!isResFromProfilesFolder)
  {
	  int addFileDataStatus = STATUS_OK;
	  addFileDataStatus = m_pFileManager->AddFileData(otherResConf, otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]));


	  //VNGFE-7071 if file already exists then we look in directory of profiles search for the maximum file number and increment
	  if (addFileDataStatus == STATUS_FILE_ALREADY_EXISTS)
	  {
		  int maxConfereneMonitorNum = m_pFileManager->GetMaxFileNumAccordingToPrefix(m_prefixMap[m_DBtype]);
		  if (maxConfereneMonitorNum == -1)
		  {
			  POBJDELETE(shortRes);
			  PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::Add - Failed to add the CcommRes file", STATUS_FAIL);
		  }

		  otherResConf.SetMonitorConfId(maxConfereneMonitorNum+1);
		  addFileDataStatus = m_pFileManager->AddFileData(otherResConf, otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]));

		  if (addFileDataStatus !=STATUS_OK)
		  {
			  POBJDELETE(shortRes);
			  PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::Add - Failed to add the CcommRes file", STATUS_FAIL);
		  }

		  SetDBCounter(maxConfereneMonitorNum);
	  }
	  else if (addFileDataStatus !=STATUS_OK)
	  {
	  		POBJDELETE(shortRes);
	  		PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::Add - Failed to add the CcommRes file", STATUS_FAIL);
	  }

  }

  m_reservArray.push_back(shortRes);
  IncreaseSummaryUpdateCounter();
  shortRes->SetSummeryCreationUpdateCounter(GetSummaryUpdateCounter());
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResDB::Update(CCommRes& otherResConf)
{
  PASSERTMSG_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(&otherResConf), "CCommResDB::Update - Failed, CCommRes is not valid", STATUS_FAIL);

  CCommResShort* shortRes = new CCommResShort;
  shortRes->SetFileUniqueName(otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]));
  shortRes->SetName(otherResConf.GetName());
  shortRes->SetDisplayName(otherResConf.GetDisplayName());
  shortRes->SetPassw(otherResConf.GetH243Password());
  shortRes->SetConferenceId(otherResConf.GetMonitorConfId());
  shortRes->SetStartTime(*otherResConf.GetStartTime());
  shortRes->SetDurationTime(*otherResConf.GetDurationTime());
  shortRes->SetPermanent(otherResConf.IsPermanent());
  shortRes->SetOperatorConf(otherResConf.GetOperatorConf());
  shortRes->SetEntryPassword(otherResConf.GetEntryPassword());
  shortRes->SetMeetMePerEntryQ(otherResConf.GetMeetMePerEntryQ());
  shortRes->SetNumericConfId(otherResConf.GetNumericConfId());
  shortRes->SetNumParties(otherResConf.GetNumParties());
  shortRes->SetNumUndefParties(otherResConf.GetNumUndefParties());
  shortRes->SetConfTransferRate(otherResConf.GetConfTransferRate());
  shortRes->SetNetwork(otherResConf.GetNetwork());
  shortRes->SetAdHocProfileId(otherResConf.GetAdHocProfileId());
  shortRes->SetContinuousPresenceScreenNumber(otherResConf.GetContinuousPresenceScreenNumber());
  shortRes->SetAutoLayoutFlag(otherResConf.GetIsAutoLayout());
  shortRes->SetIsTelePresenceMode(otherResConf.GetIsTelePresenceMode());
  shortRes->SetHDVSW(otherResConf.GetIsHDVSW());
  shortRes->SetIsTipCompatible(otherResConf.GetIsTipCompatible());
  shortRes->SetNatKAPeriod(otherResConf.GetNatKAPeriod());
  shortRes->SetconfMediaType( otherResConf.GetConfMediaType());

  DWORD sts = otherResConf.CheckCurrReservSysMode();                                          // Status Field
  if (sts != STATUS_OK)
    shortRes->SetResSts(eWrongSysMode);                                                       // Status Field
  else
    shortRes->SetResSts(eStsOK);

  shortRes->SetSipRegistrationTotalSts(otherResConf.GetSipRegistrationTotalSts());            // sipProxySts
  for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
  {
    shortRes->SetServiceRegistrationContentServiceNm(i, otherResConf.GetServiceRegistrationContentServiceName(i));
    shortRes->SetServiceRegistrationContentReg(i, otherResConf.GetServiceRegistrationContentRegister(i));
    shortRes->SetServiceRegistrationContentAccept_Call(i, otherResConf.GetServiceRegistrationContentAcceptCall(i));
    shortRes->SetSipRegistrationSts(i, otherResConf.GetServiceRegistrationContentStatus(i));  // sipProxySts1
  }

  for (int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
  {
    const char* pContactInfo = otherResConf.GetConfContactInfo(i);
    if (pContactInfo)
      shortRes->SetConfContactInfo(pContactInfo, i);
  }

 // meeting room parameters
  shortRes->SetMeetingRoomState(otherResConf.GetMeetingRoomState());
  otherResConf.SetServicePhoneToShort(*shortRes);

  // Copy all prefixes to short res
  otherResConf.CopyToShortAllPrefixStr(*shortRes);

  shortRes->SetRsrvFlags(otherResConf.CalcRsrvFlags());
  shortRes->SetRsrvFlags2(otherResConf.CalcRsrvFlags2());
  shortRes->SetRsrvEncryptionType(otherResConf.GetEncryptionType());

  // Every new reservation enters with operator mask = TRUE
  COPERMASKONINRSRVSHORT(shortRes);

  ReservArray::iterator it = FindName(otherResConf.GetName());
  if (it == m_reservArray.end())
    it = FindId(otherResConf.GetMonitorConfId());

  if (it == m_reservArray.end())
  {
    POBJDELETE(shortRes);
    TRACEINTO << "CCommResDB::Update - Failed, Reservation does not exist";
    return (m_DBtype == PROFILES_DATABASE) ? STATUS_PROFILE_NOT_FOUND : STATUS_RESERVATION_NOT_EXISTS;
  }

  // First delete the old file from the folder
  if (m_pFileManager->DeleteFileData((*it)->GetFileUniqueName()) != STATUS_OK)
  {
    POBJDELETE(shortRes);
    PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::Update - Failed, Can not delete the CCommRes file", STATUS_FAIL);
  }

  // write the new one to the disk
  if (m_pFileManager->AddFileData(otherResConf, otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype])) != STATUS_OK)
  {
    POBJDELETE(shortRes);
    PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::Update - Failed, Can not write the CCommRes file", STATUS_FAIL);
  }

  *(*it) = (*shortRes);
  POBJDELETE(shortRes);

  IncreaseSummaryUpdateCounter();

  (*it)->SetSummeryUpdateCounter(GetSummaryUpdateCounter());

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResDB::UpdateForNewEntity(CCommRes& otherResConf) // sipProxySts
{
  PASSERTMSG_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(&otherResConf), "CCommResDB::UpdateForNewEntity - Failed, CCommRes is not valid", STATUS_FAIL);

  CCommResShort* shortRes = new CCommResShort;
  shortRes->SetFileUniqueName(otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]));
  shortRes->SetName(otherResConf.GetName());
  shortRes->SetDisplayName(otherResConf.GetDisplayName());
  shortRes->SetPassw(otherResConf.GetH243Password());
  shortRes->SetConferenceId(otherResConf.GetMonitorConfId());
  shortRes->SetStartTime(*otherResConf.GetStartTime());
  shortRes->SetDurationTime(*otherResConf.GetDurationTime());
  shortRes->SetPermanent(otherResConf.IsPermanent());
  shortRes->SetOperatorConf(otherResConf.GetOperatorConf());
  shortRes->SetEntryPassword(otherResConf.GetEntryPassword());
  shortRes->SetMeetMePerEntryQ(otherResConf.GetMeetMePerEntryQ());
  shortRes->SetNumericConfId(otherResConf.GetNumericConfId());
  shortRes->SetNumParties(otherResConf.GetNumParties());
  shortRes->SetNumUndefParties(otherResConf.GetNumUndefParties());
  shortRes->SetConfTransferRate(otherResConf.GetConfTransferRate());
  shortRes->SetNetwork(otherResConf.GetNetwork());
  shortRes->SetAdHocProfileId(otherResConf.GetAdHocProfileId());
  shortRes->SetContinuousPresenceScreenNumber(otherResConf.GetContinuousPresenceScreenNumber());
  shortRes->SetAutoLayoutFlag(otherResConf.GetIsAutoLayout());
  shortRes->SetIsTelePresenceMode(otherResConf.GetIsTelePresenceMode());
  shortRes->SetHDVSW(otherResConf.GetIsHDVSW());
  shortRes->SetIsTipCompatible(otherResConf.GetIsTipCompatible());
  shortRes->SetNatKAPeriod(otherResConf.GetNatKAPeriod());
  shortRes->SetconfMediaType( otherResConf.GetConfMediaType());

  DWORD sts = otherResConf.CheckCurrReservSysMode();                                          // Status Field
  if (sts != STATUS_OK)
    shortRes->SetResSts(eWrongSysMode);                                                       // Status Field
  else
    shortRes->SetResSts(eStsOK);

  shortRes->SetSipRegistrationTotalSts(otherResConf.GetSipRegistrationTotalSts()); // sipProxySts
  for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
  {
    const char* Service_nm = otherResConf.GetServiceRegistrationContentServiceName(i);
    if (Service_nm[0] != '\0')
    {
      shortRes->SetServiceRegistrationContentServiceNm(i, Service_nm);
      shortRes->SetServiceRegistrationContentReg(i, otherResConf.GetServiceRegistrationContentRegister(i));
      shortRes->SetServiceRegistrationContentAccept_Call(i, otherResConf.GetServiceRegistrationContentAcceptCall(i));
      shortRes->SetSipRegistrationSts(i, otherResConf.GetServiceRegistrationContentStatus(i)); // sipProxySts1
    }
  }

  for (int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
  {
    const char* pContactInfo = otherResConf.GetConfContactInfo(i);
    if (pContactInfo)
      shortRes->SetConfContactInfo(pContactInfo, i);
  }

  // meeting room parameters
  shortRes->SetMeetingRoomState(otherResConf.GetMeetingRoomState());
  otherResConf.SetServicePhoneToShort(*shortRes);

  // Copy all prefixes to short res
  otherResConf.CopyToShortAllPrefixStr(*shortRes);

  shortRes->SetRsrvFlags(otherResConf.CalcRsrvFlags());
  shortRes->SetRsrvFlags2(otherResConf.CalcRsrvFlags2());

  // Every new reservation enters with operator mask = TRUE
  COPERMASKONINRSRVSHORT(shortRes);

  ReservArray::iterator it = FindName(otherResConf.GetName());
  if (it == m_reservArray.end())  // new Entity
    it = FindId(otherResConf.GetMonitorConfId());

  if (it == m_reservArray.end())
  {
    // write the new one to the disk
    TRACEINTO << "CCommResDB::UpdateForNewEntity - Create the Entity";
    if (m_pFileManager->AddFileData(otherResConf, otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype])) != STATUS_OK)
    {
      POBJDELETE(shortRes);
      PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::UpdateForNewEntity - Failed, Can not write the CCommRes file", STATUS_FAIL);
    }

    m_reservArray.push_back(shortRes);
    IncreaseSummaryUpdateCounter();
    shortRes->SetSummeryCreationUpdateCounter(GetSummaryUpdateCounter());
  }
  else                            // update
  {
    TRACEINTO << "CCommResDB::UpdateForNewEntity - Update the Entity";
    // First delete the old file from the folder
    if (m_pFileManager->DeleteFileData((*it)->GetFileUniqueName()) != STATUS_OK)
    {
      POBJDELETE(shortRes);
      PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::UpdateForNewEntity - Failed, Can not delete the CCommRes file", STATUS_FAIL);
    }

    // write the new one to the disk
    if (m_pFileManager->AddFileData(otherResConf, otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype])) != STATUS_OK)
    {
      POBJDELETE(shortRes);
      PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::UpdateForNewEntity - Failed, Can not write the CCommRes file", STATUS_FAIL);
    }

    *(*it) = (*shortRes);
    POBJDELETE(shortRes);

    IncreaseSummaryUpdateCounter();
    (*it)->SetSummeryCreationUpdateCounter(GetSummaryUpdateCounter());
  }

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResDB::Cancel(const char* name)
{
  ReservArray::iterator it = FindName(name);
  if (it == m_reservArray.end())
    return STATUS_RESERVATION_NOT_EXISTS;

  return Cancel(it);
}

//--------------------------------------------------------------------------
int CCommResDB::Cancel(const DWORD confId)
{
  ReservArray::iterator it = FindId(confId);
  if (it == m_reservArray.end())
    return STATUS_RESERVATION_NOT_EXISTS;

  return Cancel(it);
}

//--------------------------------------------------------------------------
int CCommResDB::Cancel(ReservArray::iterator it)
{
  PASSERTMSG_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(*it), "CCommResDB::Cancel - Failed, Iterator is not valid", STATUS_FAIL);

  DWORD confId = (*it)->GetConferenceId();

  // remove the Res from Dir
  if (m_pFileManager->DeleteFileData((*it)->GetFileUniqueName()) != STATUS_OK)
    PASSERTMSG(1, "CCommResDB::Cancel - Failed, Can not delete the CCommRes file");

  TRACEINTO << "CCommResDB::Cancel - Removing an element from DB, ConfId:" << confId;

  POBJDELETE(*it);         // Erase the element
  m_reservArray.erase(it); // remove the elemnet from the vector

  IncreaseSummaryUpdateCounter();

  if (m_LastDeletedIndex >= HISTORY_SIZE)
    m_LastDeletedIndex = 0;

  m_DeletedIdHistory[m_LastDeletedIndex]      = confId;
  m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwSummaryUpdateCounter;
  m_LastDeletedIndex++;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
CCommResDB::ReservArray::iterator CCommResDB::FindName(const char* name, BYTE is_display_name)
{
  PASSERT_AND_RETURN_VALUE(!name, m_reservArray.end());

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    PASSERTMSG_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(*it), "CCommResDB::FindName - Failed, Iterator is not valid", m_reservArray.end());

    if (is_display_name)    // utf-8 name
    {
      if (!strncmp((*it)->GetDisplayName(), name, H243_NAME_LEN))
        return it;
    }
    else                    // ascii name
    {
      if (!strncmp((*it)->GetName(), name, H243_NAME_LEN))
        return it;
    }
  }

  return m_reservArray.end();
}

//--------------------------------------------------------------------------
BYTE CCommResDB::TestUpdateDisplayName(const char* name, const char* display_name, const DWORD confId)
{
  PASSERT_AND_RETURN_VALUE(!name, YES);

  ReservArray::iterator it_source_rsrv = FindName(name);
  if (m_DBtype == CONF_TEMPLATES_DATABASE && it_source_rsrv == m_reservArray.end())
    it_source_rsrv = FindId(confId);

  if (it_source_rsrv == m_reservArray.end()) {
    PASSERTMSG_AND_RETURN_VALUE(1, "CCommResDB::TestUpdateDisplayName - Failed, Updaded reservation not found", YES);
  }

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    if (CPObject::IsValidPObjectPtr(*it) && it != it_source_rsrv)
    {
      if (!strncmp((*it)->GetDisplayName(), display_name, H243_NAME_LEN))
      {
        TRACEINTO << "CCommResDB::TestUpdateDisplayName - Failed, Display name already exist:\n" << name << ", " << display_name << "\n" << (*it)->GetName() << ", " << (*it)->GetDisplayName();
        return YES;
      }
    }
  }
  return NO;
}

//--------------------------------------------------------------------------
BYTE CCommResDB::IsNameExist(const char* name, BYTE is_display_name)
{
  CCommResDB::ReservArray::iterator it = FindName(name, is_display_name);
  return (it == m_reservArray.end()) ? NO : YES;
}

//--------------------------------------------------------------------------
BYTE CCommResDB::IsConfIdExist(const DWORD confId)
{
  CCommResDB::ReservArray::iterator it = FindId(confId);
  return (it == m_reservArray.end()) ? NO : YES;
}

//--------------------------------------------------------------------------
CCommResDB::ReservArray::iterator CCommResDB::FindId(const DWORD confId)
{
  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    PASSERTMSG_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(*it), "CCommResDB::FindId - Failed, Iterator is not valid", m_reservArray.end());

    if ((*it)->GetConferenceId() == confId)
      return it;
  }
  return m_reservArray.end();
}

//--------------------------------------------------------------------------
DWORD CCommResDB::GetCurrentRsrvID(const char* name_char)
{
  CCommResShort* pResShort = GetCurrentRsrvShort(name_char);
  DWORD rsrvID = 0xFFFFFFFF;

  if (pResShort)
    rsrvID = pResShort->GetConferenceId();

  POBJDELETE(pResShort);
  return rsrvID;
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetCurrentRsrv(const DWORD confId)
{
  //Search in file manager the file name with the match id
  ReservArray::iterator it = FindId(confId);
  if (it == m_reservArray.end())
  {
    TRACEINTO << "CCommResDB::GetCurrentRsrv - Failed, Can not find id in the CommResDB, ConfId:" << confId;
    return NULL;
  }

  return m_pFileManager->GetFileData((*it)->GetFileUniqueName());
}

//--------------------------------------------------------------------------
const char* CCommResDB::GetFirstEntryQueueName()
{
  const char* BAD_CHARACTERS_FOR_SIP_URI = " \"<>@:\\"; //from SipUtils.h
  const char* SPECIAL_ENTRY_QUEUE        = "##T1CAS$$";

  int iNumberOfReserv = m_reservArray.size();
  for (int i = 0; i < iNumberOfReserv; i++)
  {
    if (m_reservArray[i] != NULL)
    {
      std::auto_ptr<CCommRes> pCurrentRsrv(GetCurrentRsrv(m_reservArray[i]->GetName()));
      if (pCurrentRsrv.get() && pCurrentRsrv->GetEntryQ())
      {
        const char* pConfName = m_reservArray[i]->GetName();
        if ((strncmp(pConfName, SPECIAL_ENTRY_QUEUE, 9) == 0) || (strcspn(pConfName, BAD_CHARACTERS_FOR_SIP_URI) != strlen(pConfName)))
          continue;

        return m_reservArray[i]->GetName();
      }
    }
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* CCommResDB::GetFirstSIPFactoryName(BYTE bAutoConnectOnly)
{
  int iNumberOfReserv = m_reservArray.size();
  for (int i = 0; i < iNumberOfReserv; i++)
  {
    if (m_reservArray[i] != NULL)
    {
      std::auto_ptr<CCommRes> pCurrentRsrv(GetCurrentRsrv(m_reservArray[i]->GetName()));
      if (pCurrentRsrv.get() && pCurrentRsrv->IsSIPFactory() && (pCurrentRsrv->IsAutoConnectFactory() || !bAutoConnectOnly))
        return m_reservArray[i]->GetName();
    }
  }
  return NULL;
}

//--------------------------------------------------------------------------
CCommResShort* CCommResDB::GetCurrentRsrvShort(const char* name_char)
{
  ReservArray::iterator it = FindName(name_char);
  if (it == m_reservArray.end())
    return NULL;

  //find the specified element
  return new CCommResShort(*(*it));
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetCurrentRsrv(const char* name_char)
{
  std::auto_ptr<CCommResShort> pResShort(GetCurrentRsrvShort(name_char));
  if (pResShort.get())
    return GetCurrentRsrv(pResShort->GetConferenceId());
  return NULL;
}

//--------------------------------------------------------------------------
CCommResShort* CCommResDB::GetCurrentRsrvShort(const DWORD confId)
{
  ReservArray::iterator it = FindId(confId);
  if (it == m_reservArray.end())
    return NULL;

  //find the specified element
  return new CCommResShort(*(*it));
}

//--------------------------------------------------------------------------
void CCommResDB::IncreaseSummaryUpdateCounter()
{
  m_dwSummaryUpdateCounter++;

  if (m_dwSummaryUpdateCounter == 0xFFFFFFFF)
    m_dwSummaryUpdateCounter = 0;
}

//--------------------------------------------------------------------------
int CCommResDB::GetMeetingRoom(const char* calledPhoneNumber, const char* callingPhoneNumber, char* confName, DWORD& confId)
{
  // ---------------------------------------
  //   Return value is one of the followings
  //    0   meeting room is found
  //   -1   meeting room is not found
  // ---------------------------------------

  TRACECOND_AND_RETURN_VALUE(!calledPhoneNumber, "CCommResDB::GetMeetingRoom - Failed, Illegal called phone number", -1);
  TRACECOND_AND_RETURN_VALUE(!strlen(calledPhoneNumber), "CCommResDB::GetMeetingRoom - Failed, Empty called phone number", -1);

  for (ReservArray::const_iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    if ((*it)->IsRoomPhone((char*)calledPhoneNumber))
    {
      if (confName)
      {
        strncpy(confName, (*it)->GetName(), H243_NAME_LEN-1);
        confName[H243_NAME_LEN-1] = '\0';
      }
      confId = (*it)->GetConferenceId();
      return 0;
    }
  }
  return -1;
}

// --------------------------------------------------------------------------
int CCommResDB::GetMeetingRoomForH323Call(CH323Alias* pDestH323AliasArray, WORD wDestNumAlias, char* confName, DWORD& confId, WORD useTransitEQ, BYTE isH323)
{
  // -------------------------------------
  // Return value is one of the followings
  //  0   meeting room is found
  // -1   meeting room is not found
  // -------------------------------------
  TRACEINTO << "CCommResDB::GetMeetingRoomForH323Call - Searching for MR with " << wDestNumAlias << " destination H323 aliases";

  PASSERTMSG_AND_RETURN_VALUE(!pDestH323AliasArray, "CCommResDB::GetMeetingRoomForH323Call - Failed, Illegal called parameter: pDestH323AliasArray", -1);

  const char* coference_name = NULL;
  const char* TemporeryConf  = NULL;
  const char* aliasName      = NULL;
  WORD        aliasType;
  char        urlAlias[255]  = { 0 };

  for (int j = 0; j < wDestNumAlias; j++)
  {
    if (CPObject::IsValidPObjectPtr(&pDestH323AliasArray[j]))
    {
      aliasName = pDestH323AliasArray[j].GetAliasName();
      aliasType = pDestH323AliasArray[j].GetAliasType();

      TRACEINTO << "CCommResDB::GetMeetingRoomForH323Call - aliasType:" << aliasType;
      // bugs 21723, 21591
      if (aliasType == PARTY_H323_ALIAS_URL_ID_TYPE)
      {
        if (sscanf(aliasName, "h323:%254[^'@']", urlAlias) > 0)
          aliasName = urlAlias;
        else if (sscanf(aliasName, "%254[^'@']", urlAlias) > 0)
          aliasName = urlAlias;

        TRACEINTO << "CCommResDB::GetMeetingRoomForH323Call - urlAlias:" << urlAlias;
      }

      if (aliasName && strlen(aliasName))
      {
        TemporeryConf =  ::GetIpServiceListMngr()->FindServiceAndGetStringWithoutPrefix(aliasName, aliasType);

        if (TemporeryConf && isH323)
          coference_name = TemporeryConf;
        else
          coference_name = aliasName;

        // Conference name without predefined ivr string
        char ConfName[H243_NAME_LEN];
        memset(ConfName, 0, H243_NAME_LEN);
        if (strstr((char*)coference_name, "#"))
        {
          SAFE_COPY(ConfName, coference_name);
          for (int i = 0; i < H243_NAME_LEN; i++)
          {
            if (ConfName[i] == '#')
            {
              ConfName[i] = '\0';
              break;
            }
          }

          coference_name = ConfName;

          // VNGFE-4808
          if (strstr((char*)coference_name, "*"))
          {
            SAFE_COPY(ConfName, coference_name);
            for (int i = 0; i < H243_NAME_LEN; i++)
            {
              if (ConfName[i] == '*')
              {
                ConfName[i] = '\0';
                break;
              }
            }

            coference_name = ConfName;
          } // if string contains * - delimiter for GW

        }   // if string contains #
        else if (strstr((char*)coference_name, "*"))
        {
          SAFE_COPY(ConfName, coference_name);
          for (int i = 0; i < H243_NAME_LEN; i++)
          {
            if (ConfName[i] == '*')
            {
              ConfName[i] = '\0';
              break;
            }
          }

          coference_name = ConfName;
        }   // if string contains * - delimiter for GW
        else if (strstr((char*)coference_name, "("))
        {
          SAFE_COPY(ConfName, coference_name);
          for (int i = 0; i < H243_NAME_LEN; i++)
          {
            if (ConfName[i] == '(')
            {
              ConfName[i] = '\0';
              break;
            }
          }

          coference_name = ConfName;
        }   // if string contains (

        TRACEINTO << "CCommResDB::GetMeetingRoomForH323Call - Checking confernence name, ConfName:" <<  coference_name;
        if (coference_name && strlen(coference_name))
        {
          for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
          {
            if (CPObject::IsValidPObjectPtr(*it))
            {
              // Verify it's not a SIP factory
              DWORD flags = (*it)->GetRsrvFlags();
              if (!(flags & SIP_FACTORY))
              {
                const char* dbConfNumericId = (*it)->GetNumericConfId();
                if (!strncmp((*it)->GetName(), coference_name, H243_NAME_LEN) ||
                    (dbConfNumericId && !strncmp(dbConfNumericId, coference_name, NUMERIC_CONFERENCE_ID_LEN))
                    || ((isH323 == FALSE) && !strncasecmp((*it)->GetName(), coference_name, H243_NAME_LEN)))
                {
                  if (!useTransitEQ || (useTransitEQ && (*it)->IsEntryQ()))
                  {
                    if (confName) // Romeme klocwork
                    {
                      strncpy(confName, (*it)->GetName(), H243_NAME_LEN-1);
                      confName[H243_NAME_LEN-1] = '\0';
                    }

                    confId = (*it)->GetConferenceId();
                    TRACEINTO << "CCommResDB::GetMeetingRoomForH323Call - Found the MR in the DB, ConfId:" << confId << ", ConfName:"<< confName;
                    return 0;
                  }
                  else
                  {
                    if (useTransitEQ)
                    {
                      TRACEINTO << "CCommResDB::GetMeetingRoomForH323Call - A MR with the Transit EQ name was found, but it's not an EQ";
                    }
                  }
                }
              }
            }
          }
        }        // closing if (coference_name && ....
      }
    }
    else
      break;
  }  // for

  return -1;
}

//--------------------------------------------------------------------------
void CCommResDB::ClearVector()
{
  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
    POBJDELETE(*it);
  m_reservArray.clear();
}

//--------------------------------------------------------------------------
STATUS CCommResDB::TestValidityOfDBElementsParms()
{
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Romem - 14.2.07 - This function is used only to test validity of MR/EQ/Profiles and  not of onging conference
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  TRACEINTO << "CCommResDB::TestValidityOfDBElementsParms - Test Validity of all DB elements";

  STATUS validityParamStatus = STATUS_OK;
  std::string badDescription;

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    std::auto_ptr<CCommRes> pCurrRes(m_pFileManager->GetFileData((*it)->GetFileUniqueName()));
    if (!CPObject::IsValidPObjectPtr(pCurrRes.get()))
    {
      badDescription = "EQ/MR/Profile does found on disk, Conference Name: ";
      badDescription += (*it)->GetName();
      CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, FILE_NOT_EXIST, MAJOR_ERROR_LEVEL, badDescription.c_str(), FALSE);
      PTRACE2(eLevelError, "CCommResDB::TestValidityOfDBElementsParms - Failed, ", badDescription.c_str());
      continue;
    }

    STATUS validityOfDBItem       = pCurrRes->TestValidity();
    STATUS internalStatusOfDBItem = pCurrRes->GetInternalConfStatus();

    if (validityOfDBItem != STATUS_OK || internalStatusOfDBItem != STATUS_OK)
    {
      if (pCurrRes->GetEntryQ())
        badDescription = "Wrong parameters in EQ ";
      else if (pCurrRes->GetIsGateway())
        badDescription = "Wrong parameters in GW profile ";
      else if (pCurrRes->IsMeetingRoom())
        badDescription = "Wrong parameters in MR ";
      else if (pCurrRes->IsConfTemplate())
        badDescription = "Wrong parameters in Conf Template ";
      else
        badDescription = "Wrong parameters in Profile ";

      badDescription += pCurrRes->GetName();
      CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ILLEGAL_CONFERENCE_PARAMETERS, MAJOR_ERROR_LEVEL, badDescription.c_str(), FALSE);
    }
    if (validityOfDBItem != STATUS_OK)
    {
      badDescription += ", Status:";
      badDescription += (CProcessBase::GetProcess()->GetStatusAsString(validityOfDBItem).c_str());
      PTRACE2(eLevelError, "CCommResDB::TestValidityOfDBElementsParms - Failed, ", badDescription.c_str());

      validityParamStatus = STATUS_FAIL;
    }
    else if (internalStatusOfDBItem != STATUS_OK)
    {
      badDescription += ", Status:";
      badDescription += (CProcessBase::GetProcess()->GetStatusAsString(internalStatusOfDBItem).c_str());
      PTRACE2(eLevelError, "CCommResDB::TestValidityOfDBElementsParms - Warning, ", badDescription.c_str());
    }
  }
  return validityParamStatus;
}

//--------------------------------------------------------------------------
STATUS CCommResDB::SetFolderPath(std::string dirName)
{
  POBJDELETE(m_pFileManager);
  m_pFileManager = new CFileManager<CCommRes> (dirName);

  std::vector<CCommRes *> vect;

  //Update the ShortRes Array with the loaded array
  if ((m_pFileManager->LoadDataToVect(vect)) == STATUS_FAIL)
  {
    // VNGR-9280 - if there is problem loading some of the files we won't clear all the vector and just upload the once we can
    PASSERTMSG(1, "CCommResDB::SetFolderPath - Failed uploading some of the elements");
  }

  // First set the Items Counter to the max counter
  int maxItemID = 0;
  for (std::vector<CCommRes*>::iterator _ii = vect.begin(); _ii != vect.end(); ++_ii) // run over all items
  {
    if (!CPObject::IsValidPObjectPtr(*_ii))
    {
      PASSERT(1);
      continue; // VNGFE-5431, DK - Continue loading even found defective
    }

    int addStatus = Add(**_ii, true);
    if (addStatus != STATUS_OK)
    {
      TRACEINTO << "CCommResDB::SetFolderPath - Failed"
                << ", name:"      << (*_ii)->GetName()
                << ", addStatus:" << CProcessBase::GetProcess()->GetStatusAsString(addStatus).c_str()
                << ", maxItemID:" << maxItemID;
      PASSERT(1);
      continue; // VNGFE-5431, DK - Continue loading even found defective
    }

    maxItemID = std::max(maxItemID, int((*_ii)->GetMonitorConfId())); // Calculate max id
    ParseGivenRoutingNameForIndex((*_ii)->GetName());
    InitDBSpecifiedFields(*_ii); // Set relvant fields to default
  }
  //VNGFE-7071 We check if maxItemID is okay by searching maximum file number
  int MaxFileName = m_pFileManager->GetMaxFileNumAccordingToPrefix(m_prefixMap[m_DBtype]);
  if (MaxFileName > maxItemID)
  {
    maxItemID = MaxFileName;
  }

  SetDBCounter(maxItemID); // Set the profiles Counter
  ClearCommResVector(vect);
  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommResDB::ClearCommResVector(std::vector<CCommRes*>& vect)
{
  for (std::vector<CCommRes *>::iterator it = vect.begin(); it != vect.end(); ++it)
    POBJDELETE(*it);

  vect.clear();
}

//--------------------------------------------------------------------------
void CCommResDB::InitDBSpecifiedFields(CCommRes* pCommRes)
{
  //Update only MR DB
  switch (m_DBtype)
  {
    case (MEETING_ROOM_DATABASE):
      InitMrSpecifiedFields(pCommRes);
      break;

    case (PROFILES_DATABASE):
      InitTemplateSpecifiedFields(pCommRes);
      break;

    case (CONF_TEMPLATES_DATABASE):
      break;

    default:
      PTRACE2INT(eLevelError,"CCommResDB::InitDBSpecifiedFields - Failed, Invalid DB type, DBType:", m_DBtype);
  }
  return;
}

//--------------------------------------------------------------------------
void CCommResDB::InitMrSpecifiedFields(CCommRes* pCommRes)
{
  TRACECOND_AND_RETURN(m_DBtype != MEETING_ROOM_DATABASE, "CCommResDB::InitMrSpecifiedFields - Failed, Invalid DB type, DBType:" << m_DBtype);

  // VNGFE-2011 update only when needed
  bool bIsUpdateNeeded = false;

  // first read of MR from the file, set it to passive mode
  if (pCommRes->GetMeetingRoomState() != MEETING_ROOM_PASSIVE_STATE)
  {
    pCommRes->SetMeetingRoomState(MEETING_ROOM_PASSIVE_STATE);
    bIsUpdateNeeded = true;
  }

  // set display name / name if empty
  // on upgraded system display name will be empty
  if (pCommRes->IsEmptyDiplayNameOrName())
  {
    bIsUpdateNeeded = true;
    pCommRes->FillEmptyDiplayNameOrName();
  }

  if (::IsValidASCII(pCommRes->GetName(), H243_NAME_LEN, "CONF_NAME") == NO)
  {
    const char* strNumericID = pCommRes->GetNumericConfId();
    if (strNumericID[0] != '\0')
    {
      TRACEINTO << "CCommResDB::InitMrSpecifiedFields - Failed, Conference name has invalid ASCII symbols, try to generate name from conference numeric id, NumericConfId:" << strNumericID;
      bIsUpdateNeeded = true;
      pCommRes->SetName(strNumericID);
    }
    else
      TRACEINTO << "CCommResDB::InitMrSpecifiedFields - Failed, Conference name has invalid ASCII symbols, cannot to generate name from conference numeric id";
  }

  if (!pCommRes->isValidStartTime())
  {
    TRACEINTO << "CCommResDB::InitMrSpecifiedFields - Failed, Start time is invalid, correct it, ConfName:" << pCommRes->GetName();
    bIsUpdateNeeded = true;
    pCommRes->ResetStartTime();
  }

  // /VNGFE-2011 avoid writing all the meeting rooms again to the filesystem
  if (bIsUpdateNeeded)
  {
    if (Update(*pCommRes) != STATUS_OK)
      PTRACE(eLevelError, "CCommResDB::InitMrSpecifiedFields - Failed, Cannot update the fields on disk");
  }
}

//--------------------------------------------------------------------------
void CCommResDB::InitTemplateSpecifiedFields(CCommRes* pCommRes)
{
  TRACECOND_AND_RETURN(m_DBtype != PROFILES_DATABASE, "CCommResDB::InitTemplateSpecifiedFields - Failed, Invalid DB type, DBType:" << m_DBtype);

  // /VNGFE-2011 update only when needed
  bool bIsUpdateNeeded = false;

  // If not using YUV, translate RGB -> YUV
  if (!pCommRes->GetUseYUVcolor())
  {
    pCommRes->TranslateRGBColorToYUV();
    pCommRes->SetUseYUVcolor(TRUE);
    bIsUpdateNeeded = true;
  }

  // set display name / name if empty
  // on upgraded system display name will be empty
  if (pCommRes->IsEmptyDiplayNameOrName())
  {
    bIsUpdateNeeded = true;
    pCommRes->FillEmptyDiplayNameOrName();
  }

  if (::IsValidASCII(pCommRes->GetName(), H243_NAME_LEN, "CONF_NAME") == NO)
  {
    const char* strNumericID = pCommRes->GetNumericConfId();
    if (strNumericID[0] != '\0')
    {
      TRACEINTO << "CCommResDB::InitTemplateSpecifiedFields - Failed, Conference name has invalid ASCII symbols, try to generate name from conference numeric id, NumericConfId:" << strNumericID;
      bIsUpdateNeeded = true;
      pCommRes->SetName(strNumericID);
    }
    else
      TRACEINTO << "CCommResDB::InitTemplateSpecifiedFields - Failed, Conference name has invalid ASCII symbols, cannot to generate name from conference numeric id";
  }

  if (!pCommRes->isValidStartTime())
  {
    TRACEINTO << "CCommResDB::InitTemplateSpecifiedFields - Failed, Start time is invalid, correct it, ConfName:" << pCommRes->GetName();
    bIsUpdateNeeded = true;
    pCommRes->ResetStartTime();
  }

  // /VNGFE-2011 avoid writing all the meeting rooms again to the filesystem
  if (bIsUpdateNeeded)
  {
    if (Update(*pCommRes) != STATUS_OK)
      PTRACE(eLevelError, "CCommResDB::InitTemplateSpecifiedFields - Failed, Cannot update the fields on disk");
  }
}

//--------------------------------------------------------------------------
void CCommResDB::FillMRNumericIdList(MR_MONITOR_NUMERIC_ID_LIST_S& numericIdList) const
{
  eRsrcConfType confType;
  DWORD         sipFactCounter = 0;
  DWORD         eqCounter      = 0;
  DWORD         mrCounter      = 0;

  // Set the size and allocate the list array
  numericIdList.list_size            = m_reservArray.size();
  numericIdList.monitor_numeric_list = new MR_MONITOR_NUMERIC_ID_S[numericIdList.list_size];
  memset(numericIdList.monitor_numeric_list, 0, numericIdList.list_size*sizeof(MR_MONITOR_NUMERIC_ID_S));

  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

  ReservArray::const_iterator cIt =  m_reservArray.begin();
  for (int i = 0; cIt != m_reservArray.end(); ++cIt, ++i)
  {
    if ((*cIt)->IsEntryQ())             // EQ reservation
    {
      DWORD max_files = MAX_NUMBER_OF_EQ_RMX2000;
      if (eProductTypeRMX4000 == curProductType)
        max_files = MAX_NUMBER_OF_EQ_AMOS;

      PASSERTSTREAM(max_files == eqCounter, "CCommResDB::FillMRNumericIdList - Numer of EQ exceeded the maximum limit of:" << max_files);

      // Get The EQ phones list
      (*cIt)->FillStructWithPhones(numericIdList.monitor_numeric_list[i]);

      confType = eEQ_type;
      eqCounter++;
    }
    else if ((*cIt)->IsSIPFactory())    // Sip Factory Reservation
    {
      DWORD max_files = MAX_NUMBER_OF_SIP_FACTORY_RMX2000;
      if (eProductTypeRMX4000 == curProductType)
        max_files = MAX_NUMBER_OF_SIP_FACTORY_AMOS;

      PASSERTSTREAM(max_files == sipFactCounter, "CCommResDB::FillMRNumericIdList - Numer of SipFactory exceeded the maximum limit of:" << max_files);

      confType = eSipFact_type;
      sipFactCounter++;
    }
    else                                // MR
    {
      DWORD max_files = MAX_NUMBER_OF_MEETING_ROOM_RMX2000;
      if (eProductTypeRMX4000 == curProductType)
        max_files = MAX_NUMBER_OF_MEETING_ROOM_AMOS;

      PASSERTSTREAM(max_files == sipFactCounter, "CCommResDB::FillMRNumericIdList - Numer of MR exceeded the maximum limit of:" << max_files);

      // Get The EQ phones list
      (*cIt)->FillStructWithPhones(numericIdList.monitor_numeric_list[i]); // olga

      confType = eMR_type;
      mrCounter++;
    }

    numericIdList.monitor_numeric_list[i].conf_type = confType;
    numericIdList.monitor_numeric_list[i].meeting_room_monitor_Id = (*cIt)->GetConferenceId();
    SAFE_COPY(numericIdList.monitor_numeric_list[i].numeric_Id, (*cIt)->GetNumericConfId());
  }
}

//--------------------------------------------------------------------------
bool CCommResDB::IsREservationUsingProfile(DWORD profileID) const
{
  bool rc = false;
  for (ReservArray::const_iterator cIt = m_reservArray.begin(); cIt != m_reservArray.end(); ++cIt)
  {
    if ((*cIt)->GetAdHocProfileId() == profileID)
    {
      rc = true;
      break;
    }
  }
  TRACEINTO << "CCommResDB::IsREservationUsingProfile - ProfileId:" << profileID << ", IsREservationUsingProfile:" << (int)rc;
  return rc;
}

//--------------------------------------------------------------------------
char* CCommResDB::GetOrigionEqReservationName(const char* name, DWORD dConfId)
{
  PASSERT_AND_RETURN_VALUE(!name, NULL);

  // this is a special treatment for the EQ.(different name at on going and in the MR)
  char originalMR_name[H243_NAME_LEN] = { 0 };
  SAFE_COPY(originalMR_name, name);

  int originalMR_nameLength = strlen(originalMR_name);

  if (originalMR_nameLength > 0 && !(strncmp(originalMR_name+(originalMR_nameLength-1), ")", 1)))
  {
    char ConfIdbuf[16];
    sprintf(ConfIdbuf, "%d", dConfId);
    int iConfIdLen = strlen(ConfIdbuf);

    if (!(strncmp(originalMR_name+(originalMR_nameLength-(iConfIdLen+2)), "(", 1)))
    {
      originalMR_name[originalMR_nameLength-(iConfIdLen+2)] = '\0';
      ReservArray::iterator it = FindName(originalMR_name);
      PASSERTSTREAM_AND_RETURN_VALUE(it == m_reservArray.end(), "Failed, Cannot find Reservation in MR DB, originalMR_name:" << originalMR_name, 0);

      TRACEINTO << "CCommResDB::GetOrigionEqReservationName - EQ ongoing name is:" << name << ", originalMR_name:" << originalMR_name;
      return (char*)(*it)->GetName();
    }
  }
  return NULL;
}

//--------------------------------------------------------------------------
DWORD CCommResDB::GetTransitEQID()
{
  if (m_transitEQ != NULL && strcmp(m_transitEQ, ""))
  {
    ReservArray::iterator it = FindName(m_transitEQ);
    if (it != m_reservArray.end())
      return (*it)->GetConferenceId();
  }
  return 0xFFFFFFFF;
}

//--------------------------------------------------------------------------
void CCommResDB::SetTransitEQName(const char* EQName)
{
  PASSERT_AND_RETURN(!EQName);

  CProcessSettings *pProcessSettings = CProcessBase::GetProcess()->GetProcessSettings();
  pProcessSettings->SetSetting("TRANSIT_EQ_NAME", EQName);

  SAFE_COPY(m_transitEQ, EQName);
  TRACEINTO << "CCommResDB::SetTransitEQName - EQName:" << m_transitEQ;

  IncreaseSummaryUpdateCounter();
}

//--------------------------------------------------------------------------
void CCommResDB::CancelTransitEQ()
{
  m_transitEQ[0] = '\0';

  CProcessSettings *pProcessSettings = CProcessBase::GetProcess()->GetProcessSettings();
  pProcessSettings->RemoveSetting("TRANSIT_EQ_NAME");

  IncreaseSummaryUpdateCounter();
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetDefaultProfile(const std::string& profileName, BYTE isGwProfile,BYTE isAutoLayout)
{
  CCommRes* pProfileRsrv = new CCommRes();

  char name[H243_NAME_LEN] = { 0 };
  SAFE_COPY(name, profileName.c_str());

  TRACEINTO << "CCommResDB::GetDefaultProfile - Creating a new default profile, ProfileName: " << name << ", isGwProfile:" << (int)isGwProfile;

  // Set all other default profile params
  pProfileRsrv->SetName(name);
  pProfileRsrv->SetTemplate(TRUE);
  pProfileRsrv->SetCascadeMode(CASCADE_MODE_NONE);
  pProfileRsrv->SetConfTransferRate(Xfer_384);            // 384 rate
  pProfileRsrv->SetAutomaticTermination(TRUE);
  pProfileRsrv->SetTimeBeforeFirstJoin(10);               // Auto terminate
  pProfileRsrv->SetNumUndefParties(3);                    // Min parties
  pProfileRsrv->SetIsAutoLayout(FALSE);

  // Set Default Duration
  CStructTm tmpTime(0, 0, 0, 1, 0, 0);
  pProfileRsrv->SetDurationTime(tmpTime);

  CLectureModeParams* pLectuteModeParam = pProfileRsrv->GetLectureMode();
  if (pLectuteModeParam)
    pLectuteModeParam->SetLectureModeType(3);             // Presentaion mode

  // Set the default IVR service
  char defaultIVRService[AV_MSG_SERVICE_NAME_LEN] = { 0 };
  CAvMsgStruct* pMsgStruct = pProfileRsrv->GetpAvMsgStruct();
  if (isGwProfile)
  {
    // GW IVR service with all features disabled and new msgs /slides
    SAFE_COPY(defaultIVRService, "GW IVR Service");

    // GW Terminate immediatly when last remains
    pProfileRsrv->SetLastQuitType(eTerminateWithLastRemains);
    pProfileRsrv->SetTimeAfterLastQuit(0);

    // for GW default profile we will use classic skin
    pProfileRsrv->SetUseYUVcolor(YES);                    // UseYUV color
    pProfileRsrv->SetBackgroundImageID(0);                // Default GW Skin is skin1 where ImageID is 0 (S1-0,S2-0,S3-1,S4-2)
    pProfileRsrv->SetBckgColorYUV(81, 144, 119);
    pProfileRsrv->SetLayoutBorderColorYUV(133, 149, 117);
    pProfileRsrv->SetSpeakerNotColorYUV(161, 44, 160);
  }
  else
  {
    SAFE_COPY(defaultIVRService, ::GetpAVmsgServList()->GetDefaultIVRName());
    pProfileRsrv->SetLastQuitType(eTerminateAfterLastLeaves);
    pProfileRsrv->SetTimeAfterLastQuit(1);

    // Set Default Visual Effects (Skins)
    pProfileRsrv->SetUseYUVcolor(YES);                    // UseYUV color
    pProfileRsrv->SetBackgroundImageID(1);                // Default Skin is skin3 where ImageID is 1 (S1-0,S2-0,S3-1,S4-2)
    pProfileRsrv->SetBckgColorYUV(0, 0, 0);
    pProfileRsrv->SetLayoutBorderColorYUV(48, 159, 121);  // (0x30,0x9f,0x79)
    pProfileRsrv->SetSpeakerNotColorYUV(125, 83, 196);    // (0x7d,0x83,0x169)
  }

  pMsgStruct->SetAvMsgServiceName(defaultIVRService);
  pProfileRsrv->SetIsTelePresenceMode(NO);

  if (eProductTypeSoftMCUMfw ==  CProcessBase::GetProcess()->GetProductType())
   {
 	  pProfileRsrv->SetConfMediaType(eMixAvcSvcVsw);
 	  pProfileRsrv->SetConfTransferRate(Xfer_1920); //1920 rate
   }

  pProfileRsrv->SetManageTelepresenceLayoutInternaly(NO);
  if (eProductFamilySoftMcu ==  CProcessBase::GetProcess()->GetProductFamily())
  {
	  pProfileRsrv->SetDefaultParamsAccordingToProductType();
	  //pProfileRsrv->SetConfTransferRate(Xfer_768); //768 rate
	  pProfileRsrv->SetIsAutoLayout(TRUE);
//	  pProfileRsrv->SetIsLPR(FALSE);
	  pProfileRsrv->SetEncryptionParameters(NO, eEncryptNone);
	  pProfileRsrv->SetIsTelePresenceMode(NO);
  }
  else
  {
	  pProfileRsrv->SetConfTransferRate(Xfer_384); //384 rate
	  pProfileRsrv->SetIsAutoLayout(isAutoLayout);
  }

  return pProfileRsrv;
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetDefaultRsrv(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId)
{
  char pName[H243_NAME_LEN] = {0};
  char confNid[NUMERIC_CONFERENCE_ID_LEN] = {0};

  sprintf(confNid, "%d", numericId);
  SAFE_COPY(pName, rsrvName.c_str());

  CCommRes * pRsrv = new CCommRes();

  //Set Resrv Params
  pRsrv->SetName(pName);
  pRsrv->SetMonitorConfId(monitorID);
  pRsrv->SetNumericConfId(confNid);
  pRsrv->SetAdHocProfileId(profileId);
  pRsrv->SetCascadeMode(CASCADE_MODE_NONE);
  pRsrv->SetMeetingRoom(TRUE);

  //Make sure password is empty
  char emptyPassStr[H243_NAME_LEN];
  memset(emptyPassStr, 0, H243_NAME_LEN);
  pRsrv->SetH243Password(emptyPassStr);

  //Set Default Duration
  CStructTm tmpTime(0, 0, 0, 1, 0, 0);
  pRsrv->SetDurationTime(tmpTime);

  return pRsrv;
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetDefaultEQ(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId)
{
  CCommRes* pEq = GetDefaultRsrv(rsrvName,profileId,monitorID,numericId);

  pEq->SetEntryQ(YES);
  pEq->SetAdHoc(TRUE);

  //Set the EQ default IVR-service
  char defaultIVRService[AV_MSG_SERVICE_NAME_LEN] = { 0 };
  CAvMsgStruct* pMsgStruct = pEq->GetpAvMsgStruct();
  SAFE_COPY(defaultIVRService, ::GetpAVmsgServList()->GetDefaultEQName());
  pMsgStruct->SetAvMsgServiceName(defaultIVRService);

  return pEq;
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetDefaultFact(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId)
{
  CCommRes* pFact = GetDefaultRsrv(rsrvName,profileId,monitorID,numericId);

  pFact->SetSIPFactory(TRUE);
  pFact->SetAutoConnectFactory(TRUE);

  return pFact;
}

//--------------------------------------------------------------------------
bool CCommResDB::IsFirstProfile(DWORD profileID) const
{
  CCommResShort *pFirstProfileInList = *(m_reservArray.begin());
  if (m_reservArray.size())
  {
    DWORD firstProfileID = pFirstProfileInList->GetConferenceId();
    if (profileID == firstProfileID)
      return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetDefaultMR(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId)
{
  return GetDefaultRsrv(rsrvName, profileId, monitorID, numericId);
}

//--------------------------------------------------------------------------
CCommRes* CCommResDB::GetDefaultGW(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId)
{
  CCommRes * pRsrv = GetDefaultRsrv(rsrvName, profileId, monitorID, numericId);
  pRsrv->SetIsGateway(YES);
  //pRsrv->SetGWDialOutProtocols(GW_H323_OUT | GW_SIP_OUT | GW_H320_OUT | GW_PSTN_OUT);

  if (eProductFamilySoftMcu ==  CProcessBase::GetProcess()->GetProductFamily())	// BRIDGE-10707
  {
	  pRsrv->SetGWDialOutProtocols(GW_H323_OUT | GW_SIP_OUT);
	  TRACEINTO << " SoftMCU Family, ISDN and PSTN default GW will not be created";
  }
  else
	  pRsrv->SetGWDialOutProtocols(GW_H323_OUT | GW_SIP_OUT | GW_H320_OUT | GW_PSTN_OUT);


  return pRsrv;
}

//--------------------------------------------------------------------------
void CCommResDB::UpdateAllRsrvServicePrefixes()
{
  std::ostringstream msg;
  msg.precision(0);

  msg << "CCommResDB::UpdateAllRsrvServicePrefixes - DB size is:" << m_reservArray.size() << "\n";

  STATUS status = STATUS_OK;
  APIU32 ticks0 = SystemGetTickCount().GetIntegerPartForTrace();

  int i = 0;
  for (ReservArray::iterator it =  m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    i++;
    if (i%50 == 0)
      SystemSleep(20);

    APIU32 ticks1      = SystemGetTickCount().GetIntegerPartForTrace();
    CCommRes* pCurrRes = m_pFileManager->GetFileData((*it)->GetFileUniqueName());
    APIU32 ticks2      = SystemGetTickCount().GetIntegerPartForTrace();

    if (CPObject::IsValidPObjectPtr(pCurrRes))
    {
      msg << "ConfName:" << pCurrRes->GetName();
      APIU32 ticks3 = SystemGetTickCount().GetIntegerPartForTrace();
      pCurrRes->SetConfDialinPrefix();
      APIU32 ticks4 = SystemGetTickCount().GetIntegerPartForTrace();

      if ((status = Update(*pCurrRes)) != STATUS_OK)
      {
        PASSERT(status);
        PASSERTSTREAM(status, "CCommResDB::UpdateAllRsrvServicePrefixes -  Failed, Cannot update reservation:" << (*it)->GetName());
      }

      POBJDELETE(pCurrRes);
      APIU32 ticks5 = SystemGetTickCount().GetIntegerPartForTrace();
      msg << ", Reading:"         << ticks2-ticks1;
      msg << ", SetDialinPrefix:" << ticks4-ticks3;
      msg << ", Update:"          << ticks5-ticks4;
      msg << "\n";
    }
    else
    {
      PASSERTSTREAM(1, "CCommResDB::UpdateAllRsrvServicePrefixes -  Reservation is not found on disk: " <<(*it)->GetName());
      continue;
    }
  }

  APIU32 ticks6 = SystemGetTickCount().GetIntegerPartForTrace();
  msg << "TotalTime:" << ticks6-ticks0;
  PTRACE(eLevelInfoNormal, msg.str().c_str());
}

//--------------------------------------------------------------------------
DWORD CCommResDB::NextAdHocGwNidCounter()
{
  m_adHocGwNidCounter++;
  return (m_adHocGwNidCounter - 1);
}

//--------------------------------------------------------------------------
void CCommResDB::SetAdHocGwNidCounter(DWORD adHocGwNidCounter)
{
  m_adHocGwNidCounter = adHocGwNidCounter;
}

//--------------------------------------------------------------------------
void CCommResDB::ResetDB(std::string dirName)
{
  IncreaseSummaryUpdateCounter();

  if (m_LastDeletedIndex >= HISTORY_SIZE)
    m_LastDeletedIndex = 0;

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    if (CPObject::IsValidPObjectPtr(*it))
    {
      DWORD confId = (*it)->GetConferenceId();
      m_DeletedIdHistory[m_LastDeletedIndex]      = confId;
      m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwSummaryUpdateCounter;
      m_LastDeletedIndex++;
    }
  }

  m_pFileManager->DeleteVectData();

  ClearVector();

  POBJDELETE(m_pFileManager);

  m_prefixMap[PROFILES_DATABASE]       = "profile_";
  m_prefixMap[MEETING_ROOM_DATABASE]   = "meeting_room_";
  m_prefixMap[CONF_TEMPLATES_DATABASE] = "template_";

  *m_dwDBCounter                       = 0;
  *m_dwDBHighestInsertedCounter        = 0;
  m_adHocGwNidCounter                  = 0;
  m_transitEQ[0]                       = '\0';
  m_pFileManager                       = new CFileManager<CCommRes>(dirName);
}

//--------------------------------------------------------------------------
void CCommResDB::ChkAllRsrvSysMode() // 2 modes cop/cp
{
  TRACEINTO << "CCommResDB::ChkAllRsrvSysMode";

  int i = 0;
  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    i++;
    if (i%50 == 0)
      SystemSleep(20);

    if (CPObject::IsValidPObjectPtr(*it))
    {
      CCommRes* pCommRes = GetCurrentRsrv((*it)->GetConferenceId());
      if (pCommRes)
      {
        DWORD sts_chkSysModes = pCommRes->CheckCurrReservSysMode();
        if (sts_chkSysModes != STATUS_OK) // Status Field
          pCommRes->SetResSts(eWrongSysMode);
        else
          pCommRes->SetResSts(eStsOK);

        if (Update(*pCommRes) != STATUS_OK)
          PTRACE(eLevelError, "CCommResDB::ChkAllRsrvSysMode - Failed, Cannot update CommRes fields on disk");
      }

      POBJDELETE(pCommRes);
    }
  }
}

//--------------------------------------------------------------------------
void CCommResDB::DisableEchoSuppression(CCommResShort* pResShort)
{
  CCommRes* pProfile = ::GetpProfilesDB()->GetCurrentRsrv(pResShort->GetName());
  if (pProfile)
  {
    if (pProfile->GetEchoSuppression() != NO)
    {
      pProfile->SetEchoSuppression(FALSE);
      ::GetpProfilesDB()->Update(*pProfile);
    }
    POBJDELETE(pProfile);
  }
}

//--------------------------------------------------------------------------
void CCommResDB::DisableKeyboardSuppression(CCommResShort* pResShort)
{
  CCommRes* pProfile = ::GetpProfilesDB()->GetCurrentRsrv(pResShort->GetName());
  if (pProfile)
  {
    if (pProfile->GetKeyboardSuppression() != NO)
    {
      pProfile->SetKeyboardSuppression(FALSE);
      ::GetpProfilesDB()->Update(*pProfile);
    }
    POBJDELETE(pProfile);
  }
}

//--------------------------------------------------------------------------
void CCommResDB::ForEachDisableEchoSuppression()
{
  std::for_each(m_reservArray.begin(), m_reservArray.end(), CCommResDB::DisableEchoSuppression);
}

//--------------------------------------------------------------------------
void CCommResDB::ForEachDisableKeyboardSuppression()
{
  std::for_each(m_reservArray.begin(), m_reservArray.end(), CCommResDB::DisableKeyboardSuppression);
}

//--------------------------------------------------------------------------
/* VNGR-20807: Remove service from all profiles' service list */
void CCommResDB::DeleteIPServiceFromProfileServiceList(const char* serviceName)
{
  PASSERT_AND_RETURN(!serviceName);
  PASSERTSTREAM_AND_RETURN(m_DBtype != PROFILES_DATABASE, "Invalid DB type, DBType:" << m_DBtype);

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    CCommRes* pCommRes = GetCurrentRsrv((*it)->GetConferenceId());
    if (CPObject::IsValidPObjectPtr(pCommRes))
    {
      int serviceIndex = pCommRes->GetServiceRegistrationContentServiceIndexByName(serviceName);
      if (serviceIndex != -1)
      {
        TRACEINTO << "CCommResDB::DeleteIPServiceFromProfileServiceList "
                  << "- ProfileId:"   << (*it)->GetConferenceId()
                  << ", ProfileName:" << pCommRes->GetName()
                  << ", ServiceId:"   << serviceIndex
                  << ", ServiceName:" << serviceName;

        pCommRes->SetServiceRegistrationContentServiceName(serviceIndex, "");
        pCommRes->SetServiceRegistrationContentRegister(serviceIndex, FALSE);
        pCommRes->SetServiceRegistrationContentAcceptCall(serviceIndex, FALSE);
        pCommRes->SetServiceRegistrationContentStatus(serviceIndex, 0);

        Update(*pCommRes);
      }
    }
    else
      PTRACE(eLevelError, "CCommResDB::DeleteIPServiceFromProfileServiceList - Can not find the id in the CommResDB!!!");

    POBJDELETE(pCommRes);
  }
}

//--------------------------------------------------------------------------
/* VNGR-20807: Remove inexistent service from service list */
void CCommResDB::DeleteInexistentIPServiceFromServiceList()
{
  switch (m_DBtype)
  {
    case PROFILES_DATABASE    : TRACEINTO << "CCommResDB::DeleteInexistentIPServiceFromServiceList - PROFILES_DATABASE"; break;
    case MEETING_ROOM_DATABASE: TRACEINTO << "CCommResDB::DeleteInexistentIPServiceFromServiceList - MEETING_ROOM_DATABASE"; break;
    default                   : TRACEINTO << "CCommResDB::DeleteInexistentIPServiceFromServiceList - DBtype:" << m_DBtype; break;
  }

  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();

  for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
  {
    CCommRes* pCommRes = m_pFileManager->GetFileData((*it)->GetFileUniqueName());
    if (!CPObject::IsValidPObjectPtr(pCommRes))
      continue;

    for (int serviceId = 0; serviceId < NUM_OF_IP_SERVICES; serviceId++)
    {
      const char* serviceName = pCommRes->GetServiceRegistrationContentServiceName(serviceId);
      if (serviceName[0] != '\0')
      {
        CConfIpParameters* pServiceParams = pIpServiceListManager->FindServiceByName(serviceName);
        if (pServiceParams == NULL)
        {
          TRACEINTO << "CCommResDB::DeleteInexistentIPServiceFromServiceList "
                    << "- ResId:"       << (*it)->GetConferenceId()
                    << ", ResName:"     << pCommRes->GetName()
                    << ", ServiceId:"   << serviceId
                    << ", ServiceName:" << serviceName;

          pCommRes->SetServiceRegistrationContentServiceName(serviceId, "");
          pCommRes->SetServiceRegistrationContentRegister(serviceId, FALSE);
          pCommRes->SetServiceRegistrationContentAcceptCall(serviceId, FALSE);
          pCommRes->SetServiceRegistrationContentStatus(serviceId, 0);

          Update(*pCommRes);
        }
      }
    }

    POBJDELETE(pCommRes);
  }
}

//--------------------------------------------------------------------------
void CCommResDB::ParseGivenRoutingNameForIndex(const char* valideASCIINameCandidate)
{
  DWORD indexUsedInName = CheckForDefaultRoutingNameFormat(valideASCIINameCandidate);
  // input is a "default" name for this DB ("Profile_X", "conf_template_X")
  if ((indexUsedInName != (DWORD)-1) && (GetDBHighestIndexCounter() < indexUsedInName))
    SetDBHighestIndexCounter(indexUsedInName);
}

//--------------------------------------------------------------------------
// BOOL return value is for valid candidate string or not.
// The argument by-reference will differ from -1 only if the string matches the format.
DWORD CCommResDB::CheckForDefaultRoutingNameFormat(const char* validASCIINameCandidate) const
{
	WORD  candidateLen  = strlen(validASCIINameCandidate);
	WORD  prefixLen     = m_sUniqeRoutingName->length();

	// check that it begins with the prefix
	if (strstr(validASCIINameCandidate, m_sUniqeRoutingName->c_str()) != validASCIINameCandidate)
		return (DWORD)-1;

	// no negatives-treated as a string
	if (validASCIINameCandidate[prefixLen] == '-')
		return (DWORD)-1;

	for (int i = prefixLen; i < candidateLen; ++i)
	{
		// if it's not a number string, that's ok
		if ((validASCIINameCandidate[i] < '0') || (validASCIINameCandidate[i] > '9'))
			return (DWORD)-1;
	}

	// ok so it's a number string. checking the number...
	// check that the number string is not too long
	if (candidateLen - prefixLen > NUMERIC_CONFERENCE_ID_LEN)
		return (DWORD)-1;

	ULONGLONG tempID = atoll(&validASCIINameCandidate[prefixLen]);
	// check that the number string is not too large
	if (tempID >= (DWORD)-1)
		return (DWORD)-1;

	return (DWORD)tempID;
}

//--------------------------------------------------------------------------
BOOL CCommResDB::CreateNewValidDefaultRoutingName(char* destH243NameBuff, DWORD useThisId)
{
  std::string sRoutingName = *m_sUniqeRoutingName;
  DWORD       highestId    = (useThisId != (DWORD)-1) ? useThisId : ++(*m_dwDBHighestInsertedCounter);

  char buff[20]     = "\0";
  sprintf(buff, "%d", highestId);
  sRoutingName += buff;

  // VNGR-24845 - after reset m_dwDBHighestInsertedCounter set to 0
  // fix - check that created name is unique - otherwise re-calculate m_dwDBHighestInsertedCounter
  if ( FindName(sRoutingName.c_str()) != m_reservArray.end() ){
    TRACEINTO << "CCommResDB::CreateNewValidDefaultRoutingName generate wrong name: "  <<  sRoutingName.c_str()  << " - creating new one";
    DWORD max_index_uses =  GetMaxProfileIndexUsesInRoutingName();
    
    char newbuff[20] = "\0";
   sprintf(newbuff, "%d", max_index_uses+1 );
   sRoutingName = *m_sUniqeRoutingName;
   sRoutingName += newbuff;
   PASSERT(sRoutingName.length() > H243_NAME_LEN);
   *m_dwDBHighestInsertedCounter = max_index_uses+1;
  }

  strncpy(destH243NameBuff, sRoutingName.c_str(), H243_NAME_LEN - 1);
  destH243NameBuff[H243_NAME_LEN - 1] = 0;

  TRACEINTO << "CCommResDB::CreateNewValidDefaultRoutingName - ConfName:" << destH243NameBuff;
  return *destH243NameBuff != 0;
}

///////////////////////////////////////////////////////////////////////////

DWORD CCommResDB::GetMaxProfileIndexUsesInRoutingName()const
{
  DWORD max_index_uses = 0;

  // just a print
   switch (m_DBtype)
   {
   case PROFILES_DATABASE:
     TRACEINTO << "CCommResDB::GetMaxProfileIndexUsesInRoutingName: checking profiles";
     break;
   case CONF_TEMPLATES_DATABASE:
     TRACEINTO << "CCommResDB::GetMaxProfileIndexUsesInRoutingName: checking templates";
     break;
   default:
     TRACEINTO << "CCommResDB::GetMaxProfileIndexUsesInRoutingName: checking meeting rooms";
     break;
   }

   // calculate max_index_uses from existing names
   for (ReservArray::const_iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it){

    DWORD indexUsedInName =  CheckForDefaultRoutingNameFormat( (*it)->GetName() );
    if ((indexUsedInName != (DWORD)-1) && (indexUsedInName > max_index_uses)){
      max_index_uses = indexUsedInName;
    }

  }

  TRACEINTO << "CCommResDB::GetMaxProfileIndexUsesInRoutingName: max_index_uses = " << max_index_uses; 
  return max_index_uses;
}

///////////////////////////////////////////////////////////////////////////

