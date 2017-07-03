// AlarmStrTable.cpp

#include "AlarmStrTable.h"

#include <stdio.h>

#include "Trace.h"
#include "psosxml.h"
#include "FaultDesc.h"
#include "ObjString.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "FaultsDefines.h"
#include "AlarmParamValidator.h"
#include "AlarmStrDeclaration.h"
#include "ConvertorBase.h"
#include "PrettyTable.h"

// Static, public.
CAlarmStringConverter& CAlarmStringConverter::Instance()
{
  static CAlarmStringConverter inst;
  return inst;
}

// Private.
CAlarmStringConverter::CAlarmStringConverter()
{}

void CAlarmStringConverter::AddAlarm(int         code,
                                     const char* name,
                                     const char* desc,
                                     bool        is_alarm)
{
  PASSERT_AND_RETURN(NULL == name);
  PASSERT_AND_RETURN(NULL == desc);
  PASSERTSTREAM_AND_RETURN(is_alarm && !CAlarmParamValidator::IsInAlarmRange(code),
    "Alarm code " << code << " (" << name << ", " << desc << ") is not in range.");

  const char aa[] = "AA_";
  if (0 == strncmp(aa, name, ARRAYSIZE(aa)-1))
    name += ARRAYSIZE(aa)-1;

	Add(code, std::make_pair(name, RebrandString(desc)));
}

const std::string& CAlarmStringConverter::GetName(int code) const
{
  const AlarmValue& val = Get(code);
  FTRACECOND_AND_RETURN_VALUE(val.first.empty(),
    "Code " << code << " is not registered in alarm table",
    Unknown());

  return val.first;
}

const std::string& CAlarmStringConverter::GetDesc(int code) const
{
  const AlarmValue& val = Get(code);
  FTRACECOND_AND_RETURN_VALUE(val.second.empty(),
    "Code " << code << " is not registered in alarm table",
    Unknown());

  return val.second;
}

void CAlarmStringConverter::Dump(std::ostream& out) const
{
  CPrettyTable<int, std::string, std::string> tbl("key", "name", "description");

  std::map<int, AlarmValue>::const_iterator it;
  for (it = begin(); it != end(); ++it)
    tbl.Add(it->first, it->second.first, it->second.second);

  tbl.SetCaption("active alarm db");
  tbl.Sort(0);
  tbl.Dump(out);
}

void CAlarmStringConverter::SerializeFaultsAndAlarms(CXMLDOMElement* alarms,
                                                     CXMLDOMElement* faults) const
{
  for (std::map<int, AlarmValue>::const_iterator ii = begin(); end() != ii; ++ii)
  {
    std::ostringstream buf;
    buf << ii->first;

    if (CAlarmParamValidator::IsInAlarmRange(ii->first))
      AddNode(alarms, buf.str().c_str(), ii->second.second.c_str());
    else
      AddNode(faults, buf.str().c_str(), ii->second.second.c_str());
  }
}

// Private, static.
void CAlarmStringConverter::AddNode(CXMLDOMElement* node,
                                    const char*     code,
                                    const char*     val)
{
  CXMLDOMElement* pStringNode = node->AddChildNode("String");

  CXMLDOMAttribute* pKey_Attribute;
  CXMLDOMAttribute* pValue_Attribute;

  pKey_Attribute = new CXMLDOMAttribute;

  pKey_Attribute->set_nodeName("Key");
  pKey_Attribute->SetValueForElement(code);
  pStringNode->AddAttribute(pKey_Attribute);

  pValue_Attribute = new CXMLDOMAttribute;
  pValue_Attribute->set_nodeName("Value");
  pValue_Attribute->SetValueForElement(val);
  pStringNode->AddAttribute(pValue_Attribute);
}

const char* GetAlarmName(int code)
{
  return CAlarmStringConverter::Instance().GetName(code).c_str();
}

const char* GetAlarmDescription(int code)
{
  return CAlarmStringConverter::Instance().GetDesc(code).c_str();
}

void SerializeFaultsAndActiveAlarms(CXMLDOMElement* alarms,
                                    CXMLDOMElement* faults)
{
  CAlarmStringConverter::Instance().SerializeFaultsAndAlarms(alarms, faults);
}
