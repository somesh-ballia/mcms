#ifndef _RSRV_DB_H_
#define _RSRV_DB_H_

#include "SerializeObject.h"
#include "StatusesGeneral.h"
#include <vector>
#include "FileRsrvManager.h"
#include "FaultsDefines.h"
#include <map>
#include "AllocateStructs.h"
#include "Trace.h"
#include "PObject.h"

#define RESERVATION_DB_DIR "Cfg/Reservations/"

class CCommResRsrvShort;
class CCommResApi;
class CReservator;

typedef std::vector<CCommResRsrvShort*> ReservArray;
typedef std::map< WORD, std::string > PrefixMap;

////////////////////////////////////////////////////////////////////////////
//                        CRsrvDB
////////////////////////////////////////////////////////////////////////////
class CRsrvDB : public CSerializeObject
{
  CLASS_TYPE_1(CRsrvDB, CSerializeObject)

public:
                                 CRsrvDB(WORD DBtype = PROFILES_DATABASE);
  virtual                       ~CRsrvDB();

  const char*                    NameOf() const { return "CRsrvDB"; }

  friend std::ostream&           operator<<(std::ostream& os, CRsrvDB& obj);
  void                           Dump(std::ostream& os);

  STATUS                         InitRsrvDB(std::string dirName, CReservator* pReservator);

  CCommResApi*                   GetRsrv(const DWORD confId);
  CCommResRsrvShort*             GetShortRsrv(const DWORD confId);
  CCommResRsrvShort*             GetNextReservation();
  WORD                           GetResNumber() const;

  STATUS                         Add(CCommResApi& other, bool isResFromProfilesFolder = false);
  STATUS                         Update(CCommResApi& other);
  STATUS                         Cancel(const DWORD confId, BOOL bForUpdate = FALSE);

  BOOL                           NameExists(const char* name);
  BOOL                           DisplayNameExists(const char* name);

  // overrides
  virtual CSerializeObject*      Clone()                                            {return new CRsrvDB();}
  virtual void                   SerializeXml(CXMLDOMElement*& pFatherNode) const   { }
  virtual int                    DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
  void                           SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken);
  bool                           IsRsrvUsingProfile(DWORD profileID) const;

  DWORD                          GetSummaryUpdateCounter() const                    { return m_dwSummaryUpdateCounter; }
  void                           IncreaseSummaryUpdateCounter();
  STATUS                         CheckAllReservationsSysMode(); // 2 modes cop/cp

private:
  ReservArray::iterator          FindId(const DWORD confId);
  ReservArray::iterator          FindName(const char* name);
  ReservArray::iterator          FindDisplayName(const char* name);

  void                           UpdateSchortResWithLongResData(CCommResRsrvShort* shortRes, CCommResApi& otherResConf);

  void                           ClearCommResVector(std::vector< CCommResApi*>& vect);
  void                           ClearVector();

  // make sure that fields which were changed in older execution will be default when loaded
  void                           InitDBSpecifiedFields(CCommResApi* pCommRes);

  WORD                           GetDBtype() const                                  { return m_DBtype; }
  void                           SetDBtype(const WORD DBtype)                       { m_DBtype = DBtype; }

  DWORD                          GetDBCounter() const                               { return m_DBCounter; }
  void                           SetDBCounter(DWORD counter)                        { m_DBCounter = counter; }

  WORD                           m_DBtype;                             // RESERVATION_DATABASE;
  DWORD                          m_DBCounter;
  DWORD                          m_dwSummaryUpdateCounter;             // counts changes for operator updates
  const static WORD              HISTORY_SIZE = MAX_RSRV_IN_LIST_AMOS; // we choose here MAX_RSRV_IN_LIST_AMOS, because it's the biggest
  DWORD                          m_DeletedIdHistory[HISTORY_SIZE];
  DWORD                          m_DeletedCounterHistory[HISTORY_SIZE];
  DWORD                          m_LastDeletedIndex;
  CFileRsrvManager<CCommResApi>* m_pFileManager;                       // The profiles file manager
  ReservArray                    m_reservArray;
  PrefixMap                      m_prefixMap;

  friend class CSelfConsistency;
};

#endif // _RSRV_DB_H_
