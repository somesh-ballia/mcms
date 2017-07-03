// LicenseServerProcess.h

#ifndef LICENSE_SERVER_PROCESS_H_
#define LICENSE_SERVER_PROCESS_H_

#include "ProcessBase.h"
#include "Macros.h"

class CLicenseServerProcess : public CProcessBase, CNonCopyable
{
  CLASS_TYPE_1(CLicenseServerProcess, CProcessBase)
  friend class CTestLicenseServerProcess;

 public:
                         CLicenseServerProcess();
  virtual               ~CLicenseServerProcess();
  virtual eProcessType   GetProcessType() {return eProcessLicenseServer;}
  virtual BOOL           UsingSockets()   {return NO;}
  virtual TaskEntryPoint GetManagerEntryPoint();

  BOOL      IsFlexeraSimulationMode();

 private:
  BOOL     m_isFlexeraSimulationMode;
};

#endif

