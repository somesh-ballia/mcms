// SNMPAgentTask.h

#ifndef SNMP_AGENT_TASK_H_
#define SNMP_AGENT_TASK_H_

#include "SNMPTask.h"
#include "Macros.h"
#include "SNMPDefines.h"

class CSNMPAgentTask : public CSNMPTask, CNonCopyable
{
  CLASS_TYPE_1(CSNMPAgentTask, CSNMPTask)

 public:
  static unsigned char* ReadTelemetry(eTelemetryType type, size_t* var_len);

  CSNMPAgentTask();
  virtual ~CSNMPAgentTask();

  BOOL         IsSingleton() const {return YES;}
  const char*  GetTaskName() const {return "SNMPAgentTask";}
  void         BuildSnmp_conf_get_Gk_ip(std::string& Gk_ip) const;
  virtual void RegisterOID() const;

 protected:
  int m_ifNumber;

  PDECLAR_MESSAGE_MAP;
};

#endif
