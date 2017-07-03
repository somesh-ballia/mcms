#include "OsQueue.h"
#include "Segment.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"


static COsQueue queueCS;

extern "C"
{
  void  InitMcmsWatchdog()
  {
      queueCS.CreateWrite(eProcessMcmsDaemon,"Manager");
  }



  void SendMcmsWatchdog()
  {
    CSegment *pSeg = new CSegment;
    *pSeg << (DWORD) eProcessCsModule;
    queueCS.Send(pSeg, WD_KEEP_ALIVE);    
  }


  void DownMcmsWatchdog()
  {
      queueCS.Delete();
      
  }

}



void UnitTestAssert(char const*)
{
    
}


