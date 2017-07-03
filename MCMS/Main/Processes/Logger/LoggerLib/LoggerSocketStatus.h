// LoggerSocketStatus.h

#ifndef LOGGER_SOCKET_STATUS_H_
#define LOGGER_SOCKET_STATUS_H_

#include <ostream>

#include "Macros.h"

enum eSocketStatus
{
  eSocketStatusInvalid = -1,
  eSocketStatusNormal,
  eSocketStatusDrop,
  eSocketStatusDead,

  NumOfSocketStatuses
};

class SocketStatusUtils
{
public:
  static const char*   GetSocketStatusName(eSocketStatus index);
  static eSocketStatus GetSocketStatusByName(const char* name);
  static void          DumpSocketStatus(std::ostream& answer);

private:
  SocketStatusUtils();
  DISALLOW_COPY_AND_ASSIGN(SocketStatusUtils);
};

#endif
