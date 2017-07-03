// LoggerSocketStatus.cpp

#include "LoggerSocketStatus.h"

#include <string.h>
#include "DataTypes.h"

// eSocketStatus
static const char* SocketStatusNames[] =
{
  "normal",
  "drop",
  "dead"
};

const char* SocketStatusUtils::GetSocketStatusName(eSocketStatus index)
{
  return ((size_t)index < ARRAYSIZE(SocketStatusNames) ?
      SocketStatusNames[index] : "Invalid");
}

eSocketStatus SocketStatusUtils::GetSocketStatusByName(const char* name)
{
  for (DWORD i = 0; i < ARRAYSIZE(SocketStatusNames); i++)
  {
    if (0 == strcmp(SocketStatusNames[i], name))
      return (eSocketStatus)i;
  }

  return eSocketStatusInvalid;
}

void SocketStatusUtils::DumpSocketStatus(std::ostream& answer)
{
  for (DWORD i = 0; i < ARRAYSIZE(SocketStatusNames); i++)
    answer << GetSocketStatusName((eSocketStatus)i) << ",";
}
