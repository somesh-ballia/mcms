// ConvertorBase.cpp

#include "ConvertorBase.h"
#include "ProcessBase.h"

//#define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
#include "OsFileIF.h"
#endif

// Replace brand names in a strins, depend on a product type.
std::string RebrandString(const std::string& str)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, str);

#ifndef DEBUG_OUTPUT
  if (eProductTypeSoftMCUMfw != proc->GetProductType())
    return str;
#endif

  static std::pair<std::string, std::string> brands[] = {
    std::make_pair("Polycom",  ""),
    std::make_pair("MCU",      "VMCU"),  // Should be prior to other MCU substitution.
    std::make_pair("SVMCU",    "VMCU"),  // Converts SMCU to VMCU.
    std::make_pair("RMX 4000", "VMCU"),
    std::make_pair("RMX 2000", "VMCU"),
    std::make_pair("RMX 1500", "VMCU"),
    std::make_pair("RMX",      "VMCU")   // Should be after specific RMX products.
  };

  std::string ret(str);
  const std::pair<std::string, std::string>* it;
  for (it = brands; it != ARRAYEND(brands); ++it)
  {
    size_t idx = 0;

    // Locates substring to replace.
    while (std::string::npos != (idx = ret.find(it->first, idx)))
    {
      // Makes the replacement.
      ret.replace(idx, it->first.length(), it->second);

      // Advances index forward so the next iteration doesn't pick up.
      idx += it->second.length();

#ifdef DEBUG_OUTPUT
      WriteLocalLog(str.c_str());
      WriteLocalLog(ret.c_str());
#endif
    }
  }

  return ret;
}

