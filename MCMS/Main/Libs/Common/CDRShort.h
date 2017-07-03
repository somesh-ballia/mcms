// CDRShort.h

#ifndef CDR_SHORT_H_
#define CDR_SHORT_H_

#include <list>

#include "CDRDefines.h"
#include "ConfPartySharedDefines.h"
#include "StructTm.h"
#include "Macros.h"

#define MAX_CDR_FILE_NAME_LEN 9

class CStructTm;
class CXMLDOMElement;

class CCdrShort : public CPObject
{
  CLASS_TYPE_1(CCdrShort, CPObject)

 public:
  // Indicates undefined File Part Index
  static const DWORD kFilePartIndexUndefined;

  // Indicates first File Part Index and default value if absent
  static const DWORD kFilePartIndexFirst;

  // Checks appropriate system configuration flag
  static bool         IsEnableCDRFileParts();

                      CCdrShort();
  virtual const char* NameOf() const;
  virtual void        Dump(std::ostream& out) const;
  bool                operator==(const CCdrShort& rhs) const;

  void                Serialize(WORD format, std::ostream& ostr,
                                DWORD apiNum, bool isDisplayName = false);
  void                DeSerialize(WORD format, std::istream& m_istr,
                                  DWORD apiNum, bool isDisplayName = false);

  void                SerializeXml(CXMLDOMElement* head) const;
  void                SerializeXmlWithoutPartID(CXMLDOMElement* head) const;
  int                 DeSerializeXml(CXMLDOMElement* pActionNode,
                                     char* pszError);

  void                SetFileVersion(WORD file_version);
  WORD                GetFileVersion() const;
  void                SetH243ConfName(const char* h243confname);
  const char*         GetH243ConfName() const;
  void                SetConfId(DWORD Confid);
  DWORD               GetConfId() const;
  void                SetRsrvStrtTime(const CStructTm& other);
  const CStructTm*    GetRsrvStrtTime() const;
  void                SetRsrvDuration(const CStructTm& other);
  const CStructTm*    GetRsrvDuration() const;
  void                SetActualStrtTime(CStructTm& other);
  CStructTm*          GetActualStrtTime();
  void                SetActualDuration(CStructTm& other);
  CStructTm*          GetActualDuration();
  eConfCdrStatus      SetStatus(eConfCdrStatus status);
  eConfCdrStatus      GetStatus() const;
  void                SetFileName(const char* file_name);
  const char*         GetFileName() const;
  void                SetOffset(DWORD offset);
  DWORD               GetOffset() const;
  BYTE                GetGMTOffsetSign() const;
  void                SetGMTOffsetSign(BYTE GMT_offset_sign);
  BYTE                GetGMTOffset() const;
  void                SetGMTOffset(BYTE GMT_offset);
  void                GetGMTOffset(BYTE& GMT_offset_sign, // 0 for '-' 1 for '+'
                                   BYTE& GMT_offset_hours, // 0-15 hours
                                   BYTE& GMT_offset_minutes) const; // 0,5,10,15,...,55 in minutes
  void                SetGMTOffset(BYTE GMT_offset_sign, // 0 for '-' 1 for '+'
                                   BYTE GMT_offset_hours, // 0-15 hours
                                   BYTE GMT_offset_minutes); // 0,5,10,15,...,55 in minutes

  BYTE                GetFileMarked() const;
  void                SetFileMarked(const BYTE file_marked);

  void                SetDisplayName(const char* name);
  const char*         GetDisplayName() const;
  void                SetRsrvAudioPartiesNum(DWORD num_parties);
  DWORD               GetRsrvAudioPartiesNum() const;
  void                SetRsrvVideoPartiesNum(DWORD num_parties);
  DWORD               GetRsrvVideoPartiesNum(void) const;

  DWORD               IncrementFilePartIndex();
  DWORD               GetFilePartIndex() const;

 private:
  void                SerializeXmlCommon(CXMLDOMElement* head,
                                         bool part_enabled) const;

  WORD           m_file_version;
  char           m_h243conf_name[H243_NAME_LEN];
  DWORD          m_conf_Id;
  DWORD          m_offset;
  CStructTm      m_rsrv_strt_tm;
  CStructTm      m_rsrv_duration;
  CStructTm      m_actual_strt_tm;
  CStructTm      m_actual_duration;
  eConfCdrStatus m_status;
  char           m_filename[MAX_CDR_FILE_NAME_LEN];
  BYTE           m_GMT_offset_sign;
  BYTE           m_GMT_offset;
  BYTE           m_file_marked;
  char           m_DisplayName[H243_NAME_LEN];
  DWORD          m_rsrv_audio_parties;
  DWORD          m_rsrv_video_parties;
  DWORD          m_file_part_index;
};

class CCdrList : public CPObject, CNonCopyable
{
  CLASS_TYPE_1(CCdrList, CPObject)

 public:
                        CCdrList();
  virtual              ~CCdrList();
  virtual const char*   NameOf() const;

  void                  Serialize(WORD format, std::ostream& ostr,
                                  DWORD apiNum, bool isDiplayName = false);
  void                  DeSerialize(WORD format, std::istream& istr,
                                    DWORD apiNum, bool isDiplayName = false);

  void                  SerializeXml(CXMLDOMElement*& pFatherNode);
  int                   DeSerializeXml(CXMLDOMElement* pActionNode,
                                       char* pszError);

  BYTE                  IncrementFileNumber();
  DWORD                 GetFileNumber() const;
  void                  SetFileNumber(DWORD filenumber);
  int                   Add(const CCdrShort& other);
  void                  SetStatus(DWORD confID, eConfCdrStatus status);

  CCdrShort*            GetCurrentShort(DWORD confID) const;
  CCdrShort*            GetShort(DWORD confID, DWORD partID) const;

  int                   FindId(DWORD confID) const;
  int                   FindId(const char* fname) const;

  CCdrShort*            GetFirstShort() const;
  CCdrShort*            GetNextShort() const;
  void                  FileMarked(int index);
  DWORD                 FindBiggestConfId() const;
  BOOL                  TestForFileSystemWarning();
  void                  SetIndexShort(WORD index_short);

 private:
  size_t                GetArrayIndex(const CCdrShort* cdr) const;
  std::list<CCdrShort*> CDRConfID(DWORD confID) const;

  mutable WORD m_ind;
  WORD       m_index_short;
  DWORD      m_filenumber;
  WORD       m_numb_of_short_cdr;
  CCdrShort* m_pCdrShort[MAX_CDR_SHORT_IN_LIST_FOR_AMOS];
};

class CCdrListRequest : public CPObject, CNonCopyable
{
  CLASS_TYPE_1(CCdrListRequest, CPObject)

 public:
                   CCdrListRequest();
  char*            Serialize(WORD format);
  void             Serialize(WORD format, std::ostream& ostr);
  void             DeSerialize(WORD format, std::istream& istr);

  const char*      NameOf() const;
  void             SetKey(BYTE key);
  BYTE             GetKey() const;
  void             SetAmount(WORD amount);
  WORD             GetAmount() const;
  void             SetStartConfId(DWORD start_conf_id);
  DWORD            GetStartConfId() const;
  void             SetEndConfId(DWORD end_conf_id);
  DWORD            GetEndConfId() const;
  void             SetStartDateTime(const CStructTm& other);
  const CStructTm* GetStartDateTime() const;
  void             SetEndDateTime(const CStructTm& other);
  const CStructTm* GetREndDateTime() const;

 private:
  BYTE      m_key;
  WORD      m_amount;
  DWORD     m_start_conf_id;
  DWORD     m_end_conf_id;
  CStructTm m_start_datetime;
  CStructTm m_end_datetime;
};

#endif

