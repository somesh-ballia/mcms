// PairOfSockets.cpp

#include "PairOfSockets.h"

#include "Segment.h"
#include "TaskApi.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "TraceStream.h"

CPairOfSockets::CPairOfSockets()
{
  m_ReciveSocketMailSlot   = new COsQueue;
  m_TransmitSocketMailSlot = new COsQueue;
  rx_pid = 0;
  tx_pid = 0;
}

CPairOfSockets::CPairOfSockets(const COsQueue& reciveSocketMailSlot,
                               const COsQueue& transmitSocketMailSlot,
                               int conId,
                               const COsSocketConnected& connected)
{
  m_ReciveSocketMailSlot = new COsQueue;
  *m_ReciveSocketMailSlot = reciveSocketMailSlot;

  m_TransmitSocketMailSlot = new COsQueue;
  *m_TransmitSocketMailSlot = transmitSocketMailSlot;

  m_conId = conId;
  m_connected = connected;

  rx_pid = 0;
  tx_pid = 0;
}

CPairOfSockets::CPairOfSockets(const CPairOfSockets& other)
  : CPObject(other)
{
  m_ReciveSocketMailSlot = new COsQueue;
  m_TransmitSocketMailSlot = new COsQueue;

  rx_pid = 0;
  tx_pid = 0;

  *this = other;
}

CPairOfSockets::~CPairOfSockets()
{
  POBJDELETE(m_ReciveSocketMailSlot);
  POBJDELETE(m_TransmitSocketMailSlot);
}

void CPairOfSockets::operator=(const CPairOfSockets& psocketTable)
{
  *m_ReciveSocketMailSlot = *psocketTable.m_ReciveSocketMailSlot;
  *m_TransmitSocketMailSlot = *psocketTable.m_TransmitSocketMailSlot;

  m_conId = psocketTable.m_conId;
  m_connected = psocketTable.m_connected;

  rx_pid = psocketTable.rx_pid;
  tx_pid = psocketTable.tx_pid;
}

void CPairOfSockets::Serialize(CSegment& seg)
{
  seg << m_conId;
  m_ReciveSocketMailSlot->Serialize(seg);
  m_TransmitSocketMailSlot->Serialize(seg);
}

void CPairOfSockets::DeSerialize(CSegment& seg)
{
  seg >> m_conId;
  m_ReciveSocketMailSlot->DeSerialize(seg);
  m_TransmitSocketMailSlot->DeSerialize(seg);
}

void CPairOfSockets::KillBoth()
{
  CProcessBase* proc = CProcessBase::GetProcess();

  CSysConfig* cfg = proc->GetSysConfig();

  BOOL flag;
  BOOL res = cfg->GetBOOLDataByKey(CFG_KEY_SYNC_DESTROY_SOCKET, flag);
  TRACEINTO << "get to KillBoth()";
  CTaskApi apiRx;
  CTaskApi apiTx;
  apiRx.CreateOnlyApi(*m_ReciveSocketMailSlot);
  apiTx.CreateOnlyApi(*m_TransmitSocketMailSlot);
  /****************************************************************************/
  /*VNGR-20510 - if it is process MplApi and flag is true           */
  /*Perform syncdestroy -kills in a synce way rx and tx tasks.          */
  /*Thus, only after destroying the tasks will be a remove from the vector    */
  /*which is also destroy the connection.Only then we create the new tasks    */
  /*from the new connection                         */
  /****************************************************************************/
  if (flag && proc->GetProcessType() == eProcessMplApi)
  {
    TRACEINTO << "SYNC_DESTROY_SOCKET flag is true";
    apiRx.SyncDestroyTaskID(rx_pid, TRUE);
    apiTx.SyncDestroyTaskID(tx_pid, TRUE);
  }
  else
  {
    TRACEINTO << "KillBoth() flag is false";
    apiRx.Destroy();
    apiTx.Destroy();
  }
}
