#include "GetNetraBoardGeneralInfo.h"
#include "dsp_monitor_getter.h"
#include "copy_string.h"
void GetNetraBoardGeneralInfo(NetraBoardGeneralInfo& info, int boadid)
{
 // get dsp card general infomation from share memery
  DSPMonitorGeneralList lst;
  GetDspMonitorStatus(lst);
  int ilen = lst.len;
  for(int i=0; i< ilen;i++)
  {
    if(lst.status[i].boardId == boadid)
    {
      CopyString(info.hwver,lst.status[i].hardwareVersion);
      CopyString(info.swver,lst.status[i].softwareVersion);
      CopyString(info.riserCardCpldVersion,lst.status[i].riserCardCpldVersion);
      break;
    }
  }
}

