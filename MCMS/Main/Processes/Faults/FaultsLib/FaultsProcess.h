// FaultsProcess.h

#ifndef FAULTS_PROCESS_H_
#define FAULTS_PROCESS_H_

#include "Macros.h"
#include "ProcessBase.h"

class CHlogList;

class CFaultsProcess: public CProcessBase
{
  CLASS_TYPE_1(CFaultsProcess, CProcessBase)
public:
  friend class CTestFaultsProcess;

  CFaultsProcess();
  virtual ~CFaultsProcess();
  virtual eProcessType GetProcessType();
  virtual BOOL UsingSockets();
  virtual TaskEntryPoint GetManagerEntryPoint();
  virtual DWORD GetMaxTimeForIdle() const;
  virtual int GetProcessAddressSpace();

  CHlogList* GetFaultsListDB() const;
  CHlogList* GetFaultsShortListDB() const;

  int SetUp();
  void SetFaultList(CHlogList *pFaultsList);
  void SetShortFaultList(CHlogList *pFaultsList);

  void SetIsHardDiskOk(BOOL isOk);
  BOOL GetIsHardDiskOk();

protected:
  CHlogList* m_pFaultsList;
  CHlogList* m_pFaultsShortList;
  BOOL m_IsHardDiskOk;

  DISALLOW_COPY_AND_ASSIGN( CFaultsProcess);
};

#endif  // FAULTS_PROCESS_H_
