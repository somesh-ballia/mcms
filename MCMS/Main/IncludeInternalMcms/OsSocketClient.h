// OsSocketClient.h

#ifndef OS_SOCKET_CLIENT_H_
#define OS_SOCKET_CLIENT_H_

#include "PObject.h"
#include "DataTypes.h"
#include "OsSocketConnected.h"
#include "IpAddressDefinitions.h"

class COsSocketClient : public CPObject
{
  CLASS_TYPE_1(COsSocketClient, CPObject)
public:
  COsSocketClient();
  virtual void   Close();
  virtual void 	 Drop();
  virtual STATUS ConfigureClientSocket();
  const char*    NameOf(void) const {return "COsSocketClient";}

  void           SetAddress(const mcTransportAddress ipAddr);
  virtual void   CreateSocketConnected(COsSocketConnected** pConnected);

  virtual STATUS Connect();
  BOOL           Select(TICKS timeout);

protected:
  mcTransportAddress m_serverIp;
  int                m_descriptor;

};

#endif
