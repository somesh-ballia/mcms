// DemoProcess.h

#ifndef DEMO_PROCESS_H_
#define DEMO_PROCESS_H_

#include "ProcessBase.h"
#include "Macros.h"

class CDemoProcess : public CProcessBase, CNonCopyable
{
  CLASS_TYPE_1(CDemoProcess, CProcessBase)
  friend class CTestDemoProcess;

 public:
                         CDemoProcess();
  virtual               ~CDemoProcess();
  virtual eProcessType   GetProcessType() {return eProcessDemo;}
  virtual BOOL           UsingSockets()   {return NO;}
  virtual TaskEntryPoint GetManagerEntryPoint();
};

#endif

