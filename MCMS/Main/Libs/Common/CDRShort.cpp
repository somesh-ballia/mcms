// CDRShort.cpp

#include "CDRShort.h"

#include <stdio.h>
#include <algorithm>
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "CDRDefines.h"
#include "ConfPartySharedDefines.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"

#define OPERATOR_TERMINATE_CONF	2
#define END_TIME_TERMINATE		3
#define AUTO_TERMINATION		4
#define NEVER_STARTED			252
#define CONF_PROBLEM			253
#define MCU_COMPLETED			254
#define OFFSET_NEGATIVE		    0
#define OFFSET_POSITIVE		    1
#define FILE_MARKED			    1
#define FILE_NOT_MARKED		    0

static bool FilePartIndexCmp(const CCdrShort* lhs, const CCdrShort* rhs)
{
  FPASSERT_AND_RETURN_VALUE(NULL == lhs || NULL == rhs, false);
  return lhs->GetFilePartIndex() < rhs->GetFilePartIndex();
}

// Helper to find CDR by File Part Index
class CDRFinder
{
public:
  CDRFinder(DWORD partID) :
    m_partID(partID)
  {}

  bool operator()(const CCdrShort* cur) const
  {
    FPASSERT_AND_RETURN_VALUE(NULL == cur, false);
    return m_partID == cur->GetFilePartIndex();
  }

private:
  DWORD m_partID;
};

// Static
const DWORD CCdrShort::kFilePartIndexUndefined = static_cast<DWORD>(-1);

// Static
const DWORD CCdrShort::kFilePartIndexFirst = 1;

// Static
bool CCdrShort::IsEnableCDRFileParts(void)
{
  CProcessBase* proc = CProcessBase::GetProcess();

  // In case of unit test TestCdrShortDrv::testSerializeDeserialize the pointer
  // is NULL and the mode is always enabled
  if (NULL == proc)
    return true;

  CSysConfig* cfg = proc->GetSysConfig();
  FPASSERT_AND_RETURN_VALUE(NULL == cfg, false);

  BOOL flag;
  BOOL res = cfg->GetBOOLDataByKey(CFG_KEY_ENABLE_MULTI_PART_CDR, flag);
  FPASSERT_AND_RETURN_VALUE(FALSE == res, false);

  return flag;
}

CCdrShort::CCdrShort(void)
{
  m_file_version = 0;
	memset(m_h243conf_name, 0, ARRAYSIZE(m_h243conf_name));
  m_conf_Id = 0xffffffff;
  m_offset = 0;
  m_status = DEFAULT_STATUS;
  memset(m_filename, 0, ARRAYSIZE(m_filename));
  m_GMT_offset_sign = 0;
  m_GMT_offset = 0;
  m_file_marked = NO;
  memset(m_DisplayName, 0, ARRAYSIZE(m_DisplayName));
	m_rsrv_audio_parties = 0;
	m_rsrv_video_parties = 0;
	m_file_part_index = kFilePartIndexFirst;
}

void CCdrShort::Dump(std::ostream& out) const
{
  out << "File Version: " << m_file_version<< '\n'
      << "Conf Name: " << m_h243conf_name << '\n'
      << "Conf ID: " << m_conf_Id << '\n'
      << "Offset: " << m_offset << '\n'
      << "Reserve Time: " << m_rsrv_strt_tm << '\n'
      << "Reserve Duration Time: " << m_rsrv_duration << '\n'
      << "Actual Time: " << m_actual_strt_tm << '\n'
      << "Actual Duration Time: " << m_actual_duration << '\n'
      << "Reserved audio participants: " << m_rsrv_audio_parties << '\n'
      << "Reserved video participants: " << m_rsrv_video_parties << '\n'
      << "Status: " << m_status << '\n'
      << "Filename: " << m_filename << '\n'
      << "GMT Offset Sign: " << (WORD)m_GMT_offset_sign << '\n'
      << "GMT Offset: " << (WORD)m_GMT_offset << '\n'
      << "File Marked: " << (WORD)m_file_marked << '\n'
      << "Display Name: " << m_DisplayName << '\n'
      << "File Part ID: " << m_file_part_index << '\n';
}

// Used by unit test TestCdrShortDrv::testSerializeDeserialize
bool CCdrShort::operator==(const CCdrShort& rhs) const
{
  if (m_file_version != rhs.m_file_version)
    return false;

  if (0 != strncmp(m_h243conf_name, rhs.m_h243conf_name,
      ARRAYSIZE(m_h243conf_name)))
    return false;

  if (m_conf_Id != rhs.m_conf_Id)
    return false;

  if (m_offset != rhs.m_offset)
    return false;

  if (m_status != rhs.m_status)
    return false;

  if (0 != strncmp(m_filename, rhs.m_filename, ARRAYSIZE(m_filename)))
    return false;

  if (m_GMT_offset_sign != rhs.m_GMT_offset_sign)
    return false;

  if (m_GMT_offset != rhs.m_GMT_offset)
    return false;

  if (m_file_marked != rhs.m_file_marked)
    return false;

  if (0 != strncmp(m_DisplayName, rhs.m_DisplayName, ARRAYSIZE(m_DisplayName)))
    return false;

  if (m_rsrv_audio_parties != rhs.m_rsrv_audio_parties)
    return false;

  if (m_rsrv_video_parties != rhs.m_rsrv_video_parties)
    return false;

  if (m_file_part_index != rhs.m_file_part_index)
    return false;

  if (m_rsrv_strt_tm != rhs.m_rsrv_strt_tm)
    return false;

  if (m_rsrv_duration != rhs.m_rsrv_duration)
    return false;

  if (m_actual_strt_tm != rhs.m_actual_strt_tm)
    return false;

  if (m_actual_duration != rhs.m_actual_duration)
    return false;

  return true;
}

void CCdrShort::Serialize(WORD format, std::ostream& ostr,
                          DWORD apiNum, bool isDisplayName)
{
  ostr << m_file_version << ",";

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
  {
    ostr << m_h243conf_name << ",";
  }
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    strncpy(tmp, m_h243conf_name, H243_NAME_LEN_OLD - 1);
    tmp[H243_NAME_LEN_OLD - 1] = '\0';

    ostr << tmp << ",";
  }

  ostr << m_conf_Id << ",";

  m_rsrv_strt_tm.SerializeBilling(ostr);
  m_rsrv_duration.SerializeCdr(ostr);
  m_actual_strt_tm.SerializeBilling(ostr);
  m_actual_duration.SerializeCdr(ostr);

  ostr << (WORD) m_status << ",";
  ostr << m_filename << ",";
  ostr << (WORD) m_GMT_offset_sign << ",";

  BYTE temp = m_GMT_offset;
  if (apiNum < API_NUM_GMT_OFFSET_MINUTES && format == OPERATOR_MCMS)
    temp = temp & 0x0f; // remove the minutes for old version, sagia

  ostr << (WORD) temp << ",";
  ostr << (WORD) m_file_marked << ",";

  if (isDisplayName)
    ostr << m_DisplayName << ",";

  ostr << m_rsrv_audio_parties << ",";
  ostr << m_rsrv_video_parties;

  if (IsEnableCDRFileParts())
    ostr << "," << m_file_part_index;

  ostr << ";\n";
}

void CCdrShort::DeSerialize(WORD format, std::istream& istr,
                            DWORD apiNum, bool isDisplayName)
{
  istr >> m_file_version;
  istr.ignore(1);

  if (apiNum == 0)
    apiNum = m_file_version;

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243conf_name, H243_NAME_LEN + 1, ',');
  else
    istr.getline(m_h243conf_name, H243_NAME_LEN_OLD + 1, ',');

  istr >> m_conf_Id;
  istr.ignore(1);

  m_rsrv_strt_tm.DeSerializeBilling(istr);
  m_rsrv_duration.DeSerializeCdr(istr);
  m_actual_strt_tm.DeSerializeBilling(istr);
  m_actual_duration.DeSerializeCdr(istr);

  WORD tmp;
  istr >> tmp;
  m_status = (eConfCdrStatus) tmp;
  istr.ignore(1);
  istr.getline(m_filename, MAX_CDR_FILE_NAME_LEN + 1, ',');
  istr >> tmp;
  m_GMT_offset_sign = (BYTE) tmp;
  istr.ignore(1);
  istr >> tmp;
  m_GMT_offset = (BYTE) tmp;
  istr.ignore(1);
  istr >> tmp;
  m_file_marked = (BYTE) tmp;
  istr.ignore(1);

  if (isDisplayName)
    istr.getline(m_DisplayName, H243_NAME_LEN, ',');

  // there is no display name in old CDR
  if ('\n' == m_DisplayName[0] || '\0' == m_DisplayName[0])
  {
    strncpy(m_DisplayName, m_h243conf_name, H243_NAME_LEN);
    m_DisplayName[H243_NAME_LEN - 1] = '\0';
  }

  if (!istr.eof())
  {
    istr >> m_rsrv_audio_parties;
    istr.ignore(1);
    istr >> m_rsrv_video_parties;
    istr.ignore(1);
  }

  if (!istr.eof() && IsEnableCDRFileParts())
  {
    istr >> m_file_part_index;
    istr.ignore(1);
  }
  else
    m_file_part_index = kFilePartIndexFirst;
}

// Virtual
const char* CCdrShort::NameOf(void) const
{
  return GetCompileType();
}

void CCdrShort::SetFileVersion(WORD file_version)
{
  m_file_version = file_version;
}

WORD CCdrShort::GetFileVersion(void) const
{
  return m_file_version;
}

void CCdrShort::SetH243ConfName(const char* h243confname)
{
  int len = strlen(h243confname);
  strncpy(m_h243conf_name, h243confname, H243_NAME_LEN - 1);

  if (len > H243_NAME_LEN - 1)
    m_h243conf_name[H243_NAME_LEN - 1] = '\0';
}

const char* CCdrShort::GetH243ConfName(void) const
{
  return m_h243conf_name;
}

void CCdrShort::SetConfId(DWORD confID)
{
  m_conf_Id = confID;
}

DWORD CCdrShort::GetConfId(void) const
{
  return m_conf_Id;
}

void CCdrShort::SetRsrvStrtTime(const CStructTm &other)
{
  m_rsrv_strt_tm = other;
}

const CStructTm* CCdrShort::GetRsrvStrtTime(void) const
{
  return &m_rsrv_strt_tm;
}

void CCdrShort::SetRsrvDuration(const CStructTm &other)
{
  m_rsrv_duration = other;
}

const CStructTm* CCdrShort::GetRsrvDuration(void) const
{
 return &m_rsrv_duration;
}

void CCdrShort::SetActualStrtTime(CStructTm &other)
{
  m_actual_strt_tm = other;
}

CStructTm* CCdrShort::GetActualStrtTime(void)
{
  return &m_actual_strt_tm;
}

void CCdrShort::SetActualDuration(CStructTm& other)
{
  m_actual_duration = other;
}

CStructTm* CCdrShort::GetActualDuration(void)
{
  return &m_actual_duration;
}

eConfCdrStatus CCdrShort::SetStatus(eConfCdrStatus status)
{
  eConfCdrStatus prev = m_status;
  m_status = status;
  return prev;
}

eConfCdrStatus CCdrShort::GetStatus(void) const
{
  return m_status;
}

void CCdrShort::SetFileName(const char* filename)
{
  int len = strlen(filename);
  strncpy(m_filename, filename, 9 -1);

  if (len > 9)
    m_filename[8]='\0';
}

const char* CCdrShort::GetFileName(void) const
{
  return m_filename;
}

void CCdrShort::SetOffset(DWORD offset)
{
  m_offset = offset;
}

DWORD CCdrShort::GetOffset(void) const
{
  return m_offset;
}

BYTE CCdrShort::GetGMTOffsetSign(void) const
{
  return m_GMT_offset_sign;
}

void CCdrShort::SetGMTOffsetSign(const BYTE GMT_offset_sign)
{
  m_GMT_offset_sign = GMT_offset_sign;
}

BYTE CCdrShort::GetGMTOffset(void) const
{
  return m_GMT_offset;
}

void CCdrShort::SetGMTOffset(BYTE GMT_offset)
{
  m_GMT_offset = GMT_offset;
}

void CCdrShort::GetGMTOffset(BYTE & GMT_offset_sign,   // 0 for '-' 1 for '+'
						     BYTE & GMT_offset_hours,  //  0-15 hours
							 BYTE & GMT_offset_minutes // 0,5,10,15,...,55 in minutes
							 ) const
{
	GMT_offset_sign = m_GMT_offset_sign;
	GMT_offset_hours = m_GMT_offset % 16;
	GMT_offset_minutes = (m_GMT_offset / 16) * 5;
}

void CCdrShort::SetGMTOffset(BYTE GMT_offset_sign, // 0 for '-' 1 for '+'
    BYTE GMT_offset_hours, // 0-15 hours
    BYTE GMT_offset_minutes) // 0,5,10,15,...,55 in minutes
{
  m_GMT_offset_sign = GMT_offset_sign;
  if ((GMT_offset_minutes != 0) &&
      (GMT_offset_minutes < 60) &&
      (GMT_offset_minutes % 5 == 0))
  {
    m_GMT_offset = GMT_offset_hours + (GMT_offset_minutes / 5) * 16;
    // Byte format is MMMMHHHH
  }
  else
  {
    m_GMT_offset = GMT_offset_hours;
    // Byte format is 0000HHHH
  }
}

BYTE CCdrShort::GetFileMarked(void) const
{
  return m_file_marked;
}

void CCdrShort::SetFileMarked(BYTE file_marked)
{
  m_file_marked = file_marked;
}

void CCdrShort::SetDisplayName(const char *name)
{
  strncpy(m_DisplayName, name, H243_NAME_LEN - 1);
  m_DisplayName[H243_NAME_LEN - 1] = '\0';
}

const char* CCdrShort::GetDisplayName(void) const
{
  return m_DisplayName;
}

void CCdrShort::SetRsrvAudioPartiesNum(DWORD num_parties)
{
  m_rsrv_audio_parties = num_parties;
}

DWORD CCdrShort::GetRsrvAudioPartiesNum(void) const
{
  return m_rsrv_audio_parties;
}

void CCdrShort::SetRsrvVideoPartiesNum(DWORD num_parties)
{
  m_rsrv_video_parties = num_parties;
}

DWORD CCdrShort::GetRsrvVideoPartiesNum(void) const
{
  return m_rsrv_video_parties;
}

// Private
void CCdrShort::SerializeXmlCommon(CXMLDOMElement* head, bool part_enabled) const
{
  CXMLDOMElement* summary = head->AddChildNode("CDR_SUMMARY");

  summary->AddChildNode("FILE_VERSION", m_file_version);
  summary->AddChildNode("NAME", m_h243conf_name);
  summary->AddChildNode("ID", m_conf_Id);
  summary->AddChildNode("STATUS_STR", m_status, CDR_STATUS_ENUM);
  summary->AddChildNode("STATUS", m_status);

  BYTE GMT_Offset_Sign, GMT_Offset_Hours, GMT_Offset_Minutes;
  GetGMTOffset(GMT_Offset_Sign, GMT_Offset_Hours, GMT_Offset_Minutes);

  int intGMT_Offset_Hours = GMT_Offset_Hours;
  if (!GMT_Offset_Sign)
    intGMT_Offset_Hours = -1 * GMT_Offset_Hours;

  summary->AddChildNode("GMT_OFFSET", intGMT_Offset_Hours);
  summary->AddChildNode("START_TIME", m_actual_strt_tm);

  CXMLDOMElement* duration = summary->AddChildNode("DURATION");
  duration->AddChildNode("HOUR", m_actual_duration.m_hour);
  duration->AddChildNode("MINUTE", m_actual_duration.m_min);
  duration->AddChildNode("SECOND", m_actual_duration.m_sec);

  summary->AddChildNode("RESERVE_START_TIME", m_rsrv_strt_tm);
  duration = summary->AddChildNode("RESERVE_DURATION");
  duration->AddChildNode("HOUR", m_rsrv_duration.m_hour);
  duration->AddChildNode("MINUTE", m_rsrv_duration.m_min);
  duration->AddChildNode("SECOND", m_rsrv_duration.m_sec);

  summary->AddChildNode("MCU_FILE_NAME", m_filename);
  summary->AddChildNode("FILE_SAVED", m_file_marked, _BOOL);
  summary->AddChildNode("GMT_OFFSET_MINUTE", GMT_Offset_Minutes);
  summary->AddChildNode("DISPLAY_NAME", m_DisplayName);
  summary->AddChildNode("RESERVED_AUDIO_PARTIES", m_rsrv_audio_parties);
  summary->AddChildNode("RESERVED_VIDEO_PARTIES", m_rsrv_video_parties);

  if (part_enabled)
    summary->AddChildNode("FILE_PART_INDEX", m_file_part_index);
}

// It is used by API that doesn't support File Part Index
void CCdrShort::SerializeXmlWithoutPartID(CXMLDOMElement* head) const
{
  SerializeXmlCommon(head, false);
}

// It is standard SerializeXml method, includes File Part Index
void CCdrShort::SerializeXml(CXMLDOMElement* head) const
{
  SerializeXmlCommon(head, IsEnableCDRFileParts());
}

// schema file name:  obj_cdr_summary_list.xsd
int CCdrShort::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int  nStatus = STATUS_OK;
	CXMLDOMElement* pChildNode = NULL;

	GET_VALIDATE_CHILD(pActionNode,"FILE_VERSION",&m_file_version,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243conf_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"ID",&m_conf_Id,_0_TO_DWORD);

	DWORD tmp = (DWORD)m_status;
	GET_VALIDATE_CHILD(pActionNode,"STATUS",&tmp,_0_TO_DWORD);

	int  intGMT_Offset_Hours = 0;
	BYTE GMT_Offset_Sign = 0, GMT_Offset_Hours = 0, GMT_Offset_Minutes = 0;

	GET_VALIDATE_CHILD(pActionNode,"GMT_OFFSET",&intGMT_Offset_Hours,_0_TO_WORD);

	if (intGMT_Offset_Hours < 0)
	{
		GMT_Offset_Hours = (BYTE)(-1 * intGMT_Offset_Hours);
	}
	else
	{
		GMT_Offset_Sign = 1;
		GMT_Offset_Hours = (BYTE)intGMT_Offset_Hours;
	}

	GET_VALIDATE_CHILD(pActionNode,"START_TIME",&m_actual_strt_tm,DATE_TIME);
	GET_CHILD_NODE(pActionNode, "DURATION", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode,"HOUR",&m_actual_duration.m_hour,_0_TO_99_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode,"MINUTE",&m_actual_duration.m_min,_0_TO_59_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode,"SECOND",&m_actual_duration.m_sec,_0_TO_59_DECIMAL);
	}

	GET_VALIDATE_CHILD(pActionNode,"RESERVE_START_TIME",&m_rsrv_strt_tm,DATE_TIME);

	GET_CHILD_NODE(pActionNode, "RESERVE_DURATION", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode,"HOUR",&m_rsrv_duration.m_hour,_0_TO_99_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode,"MINUTE",&m_rsrv_duration.m_min,_0_TO_59_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode,"SECOND",&m_rsrv_duration.m_sec,_0_TO_59_DECIMAL);
	}

	GET_VALIDATE_CHILD(pActionNode,"MCU_FILE_NAME",m_filename,_0_TO_MAX_DOS_FILE_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"FILE_SAVED",&m_file_marked,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"GMT_OFFSET_MINUTES",&GMT_Offset_Minutes,_0_TO_59_DECIMAL)

	SetGMTOffset(GMT_Offset_Sign,GMT_Offset_Hours,GMT_Offset_Minutes);

  GET_VALIDATE_CHILD(pActionNode,"DISPLAY_NAME",m_DisplayName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"RESERVED_AUDIO_PARTIES",&m_rsrv_audio_parties,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"RESERVED_VIDEO_PARTIES",&m_rsrv_video_parties,_0_TO_DWORD);

 	GET_VALIDATE_CHILD(pActionNode, "FILE_PART_INDEX", &m_file_part_index, _1_TO_DWORD);

 	// Comes API that doesn't support File Part Index
 	if (STATUS_NODE_MISSING == nStatus)
 	{
  	// API that doesn't support File Part Index always looks for first index
  	m_file_part_index = CCdrShort::kFilePartIndexFirst;

  	// Fixes status to OK, otherwise the error returns to a client
  	nStatus = STATUS_OK;
 	}

  return nStatus;
}

DWORD CCdrShort::IncrementFilePartIndex(void)
{
  bool part_enabled = IsEnableCDRFileParts();

  // Does not increase File Part Index if the mode is disabled
  if (!part_enabled && m_file_part_index <= kFilePartIndexFirst)
  {
    PASSERTSTREAM(true, "Illegal flow for CDR:\n" << *this);
    return kFilePartIndexFirst;
  }

  m_file_part_index++;

  TRACEINTOFUNC << CFG_KEY_ENABLE_MULTI_PART_CDR << "="
                << (part_enabled ? CFG_STR_YES : CFG_STR_NO)
                << ", new partID " << m_file_part_index
                << ", confID " << m_conf_Id
                << ", name " << m_DisplayName;

  return m_file_part_index;
}

DWORD CCdrShort::GetFilePartIndex(void) const
{
  return m_file_part_index;
}

CCdrList::CCdrList(void)
{
  m_filenumber = 0;
  m_ind = 0;
  m_index_short = 0;
  m_numb_of_short_cdr = 0;

  for (size_t i = 0; i < ARRAYSIZE(m_pCdrShort); i++)
    m_pCdrShort[i] = NULL;
}

CCdrList::~CCdrList(void)
{
  for (size_t i = 0; i < ARRAYSIZE(m_pCdrShort); i++)
    delete m_pCdrShort[i];
}

const char* CCdrList::NameOf() const
{
  return GetCompileType();
}

void CCdrList::Serialize(WORD format, std::ostream& ostr,
                         DWORD apiNum, bool isDiplayName)
{
  ostr << m_numb_of_short_cdr << ",";

  for (WORD i = 0; i < m_numb_of_short_cdr; i++)
  {
    if (m_pCdrShort[i] == NULL)
      continue;

    m_pCdrShort[i]->Serialize(format, ostr, apiNum, isDiplayName);
  }
}
///////////////////////////////////////////////////////////////////////////////
void CCdrList::SetIndexShort(WORD index_short)
{
	if((index_short > 0)&& (index_short <= m_numb_of_short_cdr))
	{
		m_index_short = index_short;
	}
	else
	{
		m_index_short = 0;
	}
}
///
void CCdrList::DeSerialize(WORD format, std::istream& istr,
                           DWORD apiNum, bool isDiplayName)
{
  // Cleans the list
  for (size_t i = 0; i < ARRAYSIZE(m_pCdrShort); i++)
    delete m_pCdrShort[i];

  istr.ignore(1);
  istr >> m_numb_of_short_cdr;
  istr.ignore(1);

  for (WORD i = 0; i < m_numb_of_short_cdr; i++)
  {
    m_pCdrShort[i] = new CCdrShort;
    m_pCdrShort[i]->DeSerialize(format, istr, apiNum, isDiplayName);
  }
}

std::list<CCdrShort*> CCdrList::CDRConfID(DWORD confID) const
{
  // Keeps CDR with same confID
  std::list<CCdrShort*> ret;

  for (CCdrShort* const * it = m_pCdrShort; it < ARRAYEND(m_pCdrShort); ++it)
  {
    if (NULL == *it)
      continue;

    if ((*it)->GetConfId() != confID)
      continue;

    // Puts together all CRDs with same confID
    ret.push_back(*it);
  }

  if (ret.empty())
    return ret;

  // Verifies the CDR list of a single conference. The verification can be
  // deleted if computation resources is a problem. Or if the code works
  // without issues.
  if (CCdrShort::IsEnableCDRFileParts())
  {
    ret.sort(FilePartIndexCmp);

    std::list<CCdrShort*>::iterator it;
    for (it = ret.begin(); it != ret.end(); ++it)
    {
      // The distance starts from 0
      DWORD dst = std::distance(ret.begin(), it);

      // The File Part Index starts from 1
      DWORD idx = (*it)->GetFilePartIndex();

      // Fixes the index to match the distance
      if ((idx - 1) < dst)
      {
        TRACEINTOFUNC << "WARNING: " << CFG_KEY_ENABLE_MULTI_PART_CDR << " "
                      << CFG_STR_YES << ": "
                      << "File Part Index (" << idx << " - 1) < "
                      << dst << " with same confID " << confID;
      }
    }
  }
  else
  {
    if (ret.size() == 1)
    {
      DWORD idx = ret.front()->GetFilePartIndex();
      if (CCdrShort::kFilePartIndexFirst != idx)
      {
        TRACEINTOFUNC << "WARNING: " << CFG_KEY_ENABLE_MULTI_PART_CDR << " "
                       << CFG_STR_NO << ": " << "Illegal PartID " << idx
                       << " for confID " << confID;
      }
    }
    else
    {
      TRACEINTOFUNC << "WARNING: " << CFG_KEY_ENABLE_MULTI_PART_CDR << " "
                    << CFG_STR_NO << ": " << ret.size()
                    << " CDR files with same confID " << confID;
    }
  }

  return ret;
}

size_t CCdrList::GetArrayIndex(const CCdrShort* cdr) const
{
  // Finds position of the CDR object in the array
  CCdrShort* const * ptr = std::find(m_pCdrShort, ARRAYEND(m_pCdrShort), cdr);

  PASSERT_AND_RETURN_VALUE(ptr == ARRAYEND(m_pCdrShort),
      static_cast<size_t>(NOT_FIND));

  // Returns index of the object
  return ptr - m_pCdrShort;
}

CCdrShort* CCdrList::GetCurrentShort(DWORD confID) const
{
  // Fills CDR list with same confID
  std::list<CCdrShort*> cdr = CDRConfID(confID);

  std::list<CCdrShort*>::const_iterator it = std::max_element(cdr.begin(),
      cdr.end(), FilePartIndexCmp);

  if (cdr.end() == it)
  {
    TRACEINTOFUNC << "There is no CDR for ConfID " << confID;
    return NULL;
  }

  return *it;
}

CCdrShort* CCdrList::GetShort(DWORD confID, DWORD partID) const
{
  // Fills list with CDR with same conference ID
  std::list<CCdrShort*> cdr = CDRConfID(confID);

  // Looks for CDR with the File Part Index only in the list
  std::list<CCdrShort*>::const_iterator it = std::find_if(cdr.begin(),
      cdr.end(), CDRFinder(partID));

  if (cdr.end() == it)
  {
//    TRACEINTOFUNC << "There is no CDR for ConfID "
//                  << confID << ", PartID " << partID;

    return NULL;
  }

  return *it;
}

int CCdrList::FindId(DWORD confID) const
{
  const CCdrShort* cdr = GetCurrentShort(confID);
  if (NULL == cdr)
  {
    PASSERTSTREAM(true, "There is no CDR for confID " << confID);
    return NOT_FIND;
  }

  size_t index = GetArrayIndex(cdr);
  TRACEINTOFUNC << "There is index " << index
                << " of CDR for ConfID " << confID;

  return index;
}

int CCdrList::FindId(const char* fname) const
{
  for (WORD i = 0; i < m_numb_of_short_cdr; i++)
  {
    if (m_pCdrShort[i] == NULL)
      continue;

    char real_name[64]={0};
    snprintf(real_name,sizeof(real_name)-1, "%s.cdr", m_pCdrShort[i]->GetFileName());

    if (strcmp(fname, real_name) == 0)
      return i;
  }

  return NOT_FIND;
}

int CCdrList::Add(const CCdrShort& cdr)
{
  // Checks existence of the CDR
  const CCdrShort* old = GetShort(cdr.GetConfId(), cdr.GetFilePartIndex());
  if (NULL != old)
  {
    PASSERTSTREAM(true,
        "CDR already exists for ConfID " << cdr.GetConfId()
        << ", PartID " << cdr.GetFilePartIndex()
        << ", Name " << cdr.GetH243ConfName());
    return STATUS_CONF_NAME_EXISTS;
  }

  int max_cdrs = MAX_CDR_SHORT_IN_LIST;
  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  if(eProductTypeRMX4000 == curProductType)
    max_cdrs = MAX_CDR_SHORT_IN_LIST_FOR_AMOS;

  if (m_index_short >= max_cdrs)
    m_index_short = 0;

  delete m_pCdrShort[m_index_short];

  m_pCdrShort[m_index_short] = new CCdrShort(cdr);
  m_index_short++;

  if (m_numb_of_short_cdr < max_cdrs)
    m_numb_of_short_cdr++;

  int index = m_index_short - 1;
//  TRACEINTOFUNC << "Added new CDR: "
//                << "Index: " << index
//                << ", ConfID " << m_pCdrShort[index]->GetConfId()
//                << ", PartID " << m_pCdrShort[index]->GetFilePartIndex()
//                << ", Name " << m_pCdrShort[index]->GetH243ConfName();

  return STATUS_OK;
}

// Updates statuses of sibling CDR files
void CCdrList::SetStatus(DWORD confID, eConfCdrStatus status)
{
  // Fills CDR list with same confID
  std::list<CCdrShort*> cdr = CDRConfID(confID);

  std::list<CCdrShort*>::iterator it = cdr.begin();
  for (it = cdr.begin(); it != cdr.end(); ++it)
  {
    eConfCdrStatus prev = (*it)->SetStatus(status);
    TRACEINTOFUNC << "CDR file status for ConfID "
                  << confID << ", PartID " << (*it)->GetFilePartIndex()
                  << " has been changed from " << GetConfCdrStatusName(prev) << " ("
                  << prev << ")" << " to " << GetConfCdrStatusName(status) << " ("
                  << status << ")";
  }
}

BYTE CCdrList::IncrementFileNumber(void)
{
  DWORD max_cdrs = MAX_CDR_SHORT_IN_LIST;
  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  if (eProductTypeRMX4000 == curProductType)
    max_cdrs = MAX_CDR_SHORT_IN_LIST_FOR_AMOS;

  if (m_filenumber < max_cdrs)
  {
    m_filenumber++;
    return 0;
  }

  m_filenumber = 1;
  return 1;
}

DWORD CCdrList::GetFileNumber(void) const
{
  return m_filenumber;
}

void CCdrList::SetFileNumber(DWORD filenumber)
{
  m_filenumber = filenumber;
}

CCdrShort* CCdrList::GetFirstShort(void) const
{
  m_ind = 1;
  return m_pCdrShort[0];
}

CCdrShort* CCdrList::GetNextShort(void) const
{
  if (m_ind >= m_numb_of_short_cdr)
    return NULL;

  return m_pCdrShort[m_ind++];
}

void CCdrList::FileMarked(int index)
{
  if (index < MAX_CDR_SHORT_IN_LIST_FOR_AMOS && m_pCdrShort[index] != NULL)
    m_pCdrShort[index]->SetFileMarked(YES);
}

DWORD CCdrList::FindBiggestConfId(void) const
{
	DWORD newconfid = 0;
	for (WORD i = 0; i < m_numb_of_short_cdr; i++)
	{
		if (m_pCdrShort[i] == NULL)
		    continue;

		DWORD dbconfid = m_pCdrShort[i]->GetConfId();
		if (( dbconfid > newconfid ) && ( dbconfid <= 0x7FFFFFFD ))
		    newconfid = dbconfid;
	}

	if (10 > newconfid)
		newconfid = 10;

	return newconfid;
}

void CCdrList::SerializeXml(CXMLDOMElement*& father)
{
	if (NULL == father)
	{
	    father = new CXMLDOMElement;
	    father->set_nodeName("CDR_SUMMARY_LS");
	}
	else
	{
	    father = father->AddChildNode("CDR_SUMMARY_LS");
	}

	bool part_enabled = CCdrShort::IsEnableCDRFileParts();

	unsigned int counter = 0;
	for (WORD i = 0; i < m_numb_of_short_cdr; i++)
	{
		if (m_pCdrShort[i] == NULL)
		    continue;

		const CCdrShort& cdr = *(m_pCdrShort[i]);
		if (!part_enabled)
		{
		    // Does not include File Part CDR if the System Flag is OFF
		    if (cdr.GetFilePartIndex() > CCdrShort::kFilePartIndexFirst)
		    {
		        TRACEINTOFUNC << CFG_KEY_ENABLE_MULTI_PART_CDR << " " << CFG_STR_NO
                          << ": Ignored CDR confID " << cdr.GetConfId()
                          << ", partID " << cdr.GetFilePartIndex()
                          << ", name " << cdr.GetDisplayName();
		        continue;
		    }
		}

		cdr.SerializeXml(father);
		counter++;
	}

	TRACEINTOFUNC << "Done with " << counter << " CDR files";
}


// schema file name:  obj_cdr_summary_list.xsd
int CCdrList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	for (WORD i = 0; i < m_numb_of_short_cdr; i++)
		POBJDELETE(m_pCdrShort[i]);

	m_numb_of_short_cdr = 0;

	int nStatus = STATUS_OK;
	CXMLDOMElement* pCdrShortNode = NULL;
	GET_FIRST_CHILD_NODE(pActionNode, "CDR_SUMMARY", pCdrShortNode);

	int max_cdrs = MAX_CDR_SHORT_IN_LIST;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if(eProductTypeRMX4000 == curProductType)
		max_cdrs = MAX_CDR_SHORT_IN_LIST_FOR_AMOS;

	while (pCdrShortNode  &&  m_numb_of_short_cdr < max_cdrs)
	{

		m_pCdrShort[m_numb_of_short_cdr] = new CCdrShort;
		nStatus = m_pCdrShort[m_numb_of_short_cdr]->DeSerializeXml(pCdrShortNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(m_pCdrShort[m_numb_of_short_cdr]);
			return nStatus;
		}

		m_numb_of_short_cdr++;
		GET_NEXT_CHILD_NODE(pActionNode, "CDR_SUMMARY", pCdrShortNode);
	}

	return STATUS_OK;
}


BOOL CCdrList::TestForFileSystemWarning()
{
    int i = m_index_short;
    int skip = 0;

    // VNGFE-3755: RMX encounter Process terminated McmsDaemonCDR
    // terminated error when it is idle. Break endless loop.
    do
    {
        i--;
        skip++;

        if (skip > MAX_CDR_SHORT_IN_LIST / 2)
        {
            if (m_pCdrShort[i])
            {
                if (m_pCdrShort[i]->GetFileMarked() == FALSE)
                {
                    return TRUE;
                }
            }
        }
    } while (i > 0);

    return FALSE;
}

CCdrListRequest::CCdrListRequest()
{
  m_key = 0;
  m_amount = 0;
  m_start_conf_id = 0;
  m_end_conf_id = 0;
}

void CCdrListRequest::Serialize(WORD format, std::ostream &m_ostr)
{
  m_ostr << (WORD)m_key << ",";
  m_ostr << m_amount   << ",";
  m_ostr << m_start_conf_id << ",";
  m_ostr << m_end_conf_id << ",";
  m_start_datetime.Serialize(m_ostr);
  m_end_datetime.Serialize(m_ostr);
}

void CCdrListRequest::DeSerialize(WORD format, std::istream &m_istr)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;

  m_istr >> tmp;
  m_key=(BYTE)tmp;
  m_istr >> m_amount;
  m_istr >> m_start_conf_id;
  m_istr >> m_end_conf_id;
  m_start_datetime.DeSerialize(m_istr);
  m_end_datetime.DeSerialize(m_istr);
}

// Virtual
const char* CCdrListRequest::NameOf(void) const
{
    return GetCompileType();
}

void CCdrListRequest::SetKey(BYTE key)
{
    m_key = key;
}

BYTE CCdrListRequest::GetKey(void) const
{
    return m_key;
}

void CCdrListRequest::SetAmount(WORD amount)
{
  m_amount = amount;
}

WORD CCdrListRequest::GetAmount(void) const
{
  return m_amount;
}

void CCdrListRequest::SetStartConfId(DWORD start_conf_id)
{
  m_start_conf_id = start_conf_id;
}

DWORD CCdrListRequest::GetStartConfId(void) const
{
  return m_start_conf_id;
}

void CCdrListRequest::SetEndConfId(DWORD end_conf_id)
{
  m_end_conf_id = end_conf_id;
}


DWORD CCdrListRequest::GetEndConfId(void) const
{
  return m_end_conf_id;
}

void CCdrListRequest::SetStartDateTime(const CStructTm& other)
{
  m_start_datetime = other;
}

const CStructTm* CCdrListRequest::GetStartDateTime(void) const
{
  return &m_start_datetime;
}

void CCdrListRequest::SetEndDateTime(const CStructTm& other)
{
  m_end_datetime = other;
}

const CStructTm* CCdrListRequest::GetREndDateTime(void) const
{
  return &m_end_datetime;
}
