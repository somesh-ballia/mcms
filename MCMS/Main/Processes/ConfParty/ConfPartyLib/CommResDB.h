#ifndef _COMM_RES_DB_H_
#define _COMM_RES_DB_H_

#include "SerializeObject.h"
#include "StatusesGeneral.h"
#include <vector>
#include "FileManager.h"
#include "ConfPartyDefines.h"
#include <map>
#include "AllocateStructs.h"

class CCommResShort;
class CCommRes;
class CH323Alias;

static DWORD       _dwProfileIndexCounter            = 0;
static DWORD       _dwTemplateIndexCounter           = 0;
static DWORD       _dwMRIndexCounter                 = 0;
static DWORD       _dwProfileHighestExistingCounter  = 0;
static DWORD       _dwTemplateHighestExistingCounter = 0;
static DWORD       _dwMRHighestExistingCounter       = 0;
static std::string _sTemplateUniqeRoutingName        = "tempate_uniqe_routing_name_";
static std::string _sProfileUniqeRoutingName         = "Profile_";
static std::string _sMRUniqeRoutingName              = "MR_routing_name_";


////////////////////////////////////////////////////////////////////////////
//                        CCommResDB
////////////////////////////////////////////////////////////////////////////
class CCommResDB : public CSerializeObject
{
  CLASS_TYPE_1(CCommResDB, CSerializeObject)

public:
  typedef std::vector<CCommResShort*> ReservArray;
  typedef std::map< WORD, std::string > PrefixMap;

  const static WORD         HISTORY_SIZE = 1000;
  static const char*        DEFAULT_COP_PROFILES_NAMES[6];

                            CCommResDB(WORD DBtype = PROFILES_DATABASE);
                            CCommResDB(const CCommResDB& other);
  virtual                  ~CCommResDB();
  CCommResDB&               operator=(const CCommResDB& other);

  virtual CSerializeObject* Clone()                                          { return new CCommResDB(); }

  virtual void              SerializeXml(CXMLDOMElement*& pFatherNode) const { }
  void                      SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken);
  virtual int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

  const char*               NameOf() const                                   { return "CCommResDB"; }

  int                       Add(CCommRes& other, bool isResFromProfilesFolder = false);
  int                       AddShortRes(const CCommResShort& otherResShort);
  int                       Update(CCommRes& other);
  int                       Cancel(const char* name);
  int                       Cancel(const DWORD confId);
  ReservArray::iterator     FindName(const char* name, BYTE is_display_name = 0);
  BYTE                      IsNameExist(const char* name, BYTE is_display_name = 0);
  BYTE                      IsConfIdExist(const DWORD confId);
  ReservArray::iterator     FindId(const DWORD confId = 0);
  BYTE                      TestUpdateDisplayName(const char* name, const char* display_name, const DWORD confId = 0);
  DWORD                     GetCurrentRsrvID(const char* name);
  CCommRes*                 GetCurrentRsrv(const DWORD confId);
  CCommResShort*            GetCurrentRsrvShort(const char* name);
  CCommResShort*            GetCurrentRsrvShort(const DWORD confId);
  CCommRes*                 GetCurrentRsrv(const char* name);
  CCommRes*                 GetDefaultProfile(const std::string& profileName, BYTE isGwProfile = FALSE, BYTE isAutoLayout = NO);

  CCommRes*                 GetDefaultRsrv(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId);
  CCommRes*                 GetDefaultEQ(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId);
  CCommRes*                 GetDefaultFact(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId);
  CCommRes*                 GetDefaultMR(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId);
  CCommRes*                 GetDefaultGW(const std::string& rsrvName, DWORD profileId, DWORD monitorID, DWORD numericId);
  bool                      IsFirstProfile(DWORD profileID) const;
  const char*               GetFirstEntryQueueName();
  const char*               GetFirstSIPFactoryName(BYTE bAutoConnectOnly = FALSE);
  void                      IncreaseSummaryUpdateCounter();
  int                       GetMeetingRoom(const char* calledPhoneNumber, const char* callingPhoneNumber, char* confName, DWORD& confId);
  int                       GetMeetingRoomForH323Call(CH323Alias* pDestH323AliasArray, WORD wDestNumAlias, char* confName, DWORD& confId, WORD useTransitEQ = FALSE, BYTE isH323 = TRUE);

  WORD                      GetResNumber() const                             { return m_reservArray.size(); }
  DWORD                     GetSummaryUpdateCounter() const                  { return m_dwSummaryUpdateCounter; }
  WORD                      GetDBtype() const                                { return m_DBtype; }
  void                      SetDBtype(const WORD DBtype)                     { m_DBtype = DBtype; }
  void                      SetDBCounter(DWORD counter)                      { *m_dwDBCounter = counter; }
  DWORD                     GetDBCounter() const                             { return *m_dwDBCounter; }
  void                      IncreaseDBCounter()                              { (*m_dwDBCounter)++; }
  void                      DecreaseDBCounter()                              { (*m_dwDBCounter)--; }
  void                      SetDBHighestIndexCounter(DWORD counter)          { *m_dwDBHighestInsertedCounter = counter; }
  DWORD                     GetDBHighestIndexCounter() const                 { return *m_dwDBHighestInsertedCounter; }

  STATUS                    SetFolderPath(std::string dirName); // Set the file Manager Path
  STATUS                    TestValidityOfDBElementsParms();

  // make sure that fileds which were changed in older execution will be default when loaded
  void                      InitDBSpecifiedFields(CCommRes* pCommRes);
  char*                     GetOrigionEqReservationName(const char* name, DWORD dConfId);
  void                      FillMRNumericIdList(MR_MONITOR_NUMERIC_ID_LIST_S& numericIdList) const;

  const ReservArray&        GetReservArray() const { return m_reservArray; }
  bool                      IsREservationUsingProfile(DWORD profileID) const;
  void                      UpdateAllRsrvServicePrefixes();

  char*                     GetTransitEQName()                               { return m_transitEQ; }
  DWORD                     GetTransitEQID();
  void                      SetTransitEQName(const char* EQName);
  void                      CancelTransitEQ();

  DWORD                     NextAdHocGwNidCounter();
  void                      SetAdHocGwNidCounter(DWORD adHocGwNidCounter);

  void                      ResetDB(std::string dirName);
  void                      ChkAllRsrvSysMode();                        // 2 modes cop/cp
  int                       UpdateForNewEntity(CCommRes& otherResConf); // sipProxySts

  void                      ForEachDisableEchoSuppression();
  void                      ForEachDisableKeyboardSuppression();

  void                      DeleteIPServiceFromProfileServiceList(const char* serviceName);
  void                      DeleteInexistentIPServiceFromServiceList();

  void                      ParseGivenRoutingNameForIndex(const char* valideASCIINameCandidate);
  DWORD                     CheckForDefaultRoutingNameFormat(const char* validASCIINameCandidate) const;
  BOOL                      CreateNewValidDefaultRoutingName(char* destH243NameBuff, DWORD useThisId = (DWORD)-1);

  DWORD                     GetMaxProfileIndexUsesInRoutingName() const;

protected:
  int                       Cancel(ReservArray::iterator it);
  void                      ClearVector();
  void                      ClearCommResVector(std::vector< CCommRes*>& vect);
  void                      InitMrSpecifiedFields(CCommRes* pCommRes);
  void                      InitTemplateSpecifiedFields(CCommRes* pCommRes);
  static void               DisableEchoSuppression(CCommResShort* pResShort);
  static void               DisableKeyboardSuppression(CCommResShort* pResShort);

  // Attributes
  DWORD*                    m_dwDBCounter;
  DWORD*                    m_dwDBHighestInsertedCounter;
  std::string const*        m_sUniqeRoutingName;
  DWORD                     m_dwSummaryUpdateCounter; // counts changes for operator updates
  DWORD                     m_dwFullUpdateCounter;    // counts changes for operator updates
  DWORD                     m_DeletedIdHistory[HISTORY_SIZE];
  DWORD                     m_DeletedCounterHistory[HISTORY_SIZE];
  DWORD                     m_LastDeletedIndex;

  DWORD                     m_adHocGwNidCounter;
  ReservArray               m_reservArray;
  WORD                      m_DBtype;                 // RESERVATION_DATABASE; MEETING_ROOM_DATABASE
  BYTE                      m_bChanged;
  char                      m_transitEQ[H243_NAME_LEN];
  CFileManager<CCommRes>*   m_pFileManager;           // The profiles file manager
  PrefixMap                 m_prefixMap;
};

#endif // ifndef _COMM_RES_DB_H_
