// SNMPProcessProcess.h

#ifndef SNMP_PROCESS_H_
#define SNMP_PROCESS_H_

#include "ProcessBase.h"
#include "SnmpData.h"

class CSNMPProcessProcess : public CProcessBase
{
  friend class CTestSNMPProcessProcess;
  CLASS_TYPE_1(CSNMPProcessProcess, CProcessBase)

public:
  CSNMPProcessProcess();
  virtual ~CSNMPProcessProcess();
  virtual TaskEntryPoint GetManagerEntryPoint();
  virtual void           AddExtraStringsToMap();

  virtual eProcessType  GetProcessType()        { return eProcessSNMPProcess; }
  virtual BOOL          UsingSockets()          { return NO; }
  CSnmpData&      GetSnmpData()      { return m_snmpConfiguration; }
  void                  SetSnmpData(const CSnmpData& data);
  void SetJitcMode(BOOL jitc);
  BOOL GetJitcMode() const;


private:
  CSnmpData m_snmpConfiguration;

  BOOL 	m_jitc;

};

#endif  // SNMP_PROCESS_H_

