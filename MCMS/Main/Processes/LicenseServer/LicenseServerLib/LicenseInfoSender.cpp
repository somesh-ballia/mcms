// LicenseInfoSender.cpp

#include "LicenseInfoSender.h"
#include "LicenseDefs.h"
#include "Trace.h"
#include "ProcessBase.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "ConfigManagerApi.h"
#include "TraceStream.h"
#include "StructTm.h"

#include "LicensingManager.h"
#include "PrettyTable.h"


LicenseInfoSender::LicenseInfoSender(LicensedItem* ptr, int num_elem)
 : isFirstRefreshMsgSent(false), lic_feat(ptr), n_elem(num_elem)
{
}


void LicenseInfoSender::Send()
{
  OPCODE msg_id;

  CManagerApi api(eProcessMcuMngr);

  CSegment *pSeg = new CSegment;
  char exp_date_buff[58] = { 0 };

  FLEXERA_DATA_S lic_info;

  CPrettyTable<const char*, int, int, int, int, const char*>
    tbl("Feature", "IsEnabled", "Status", "HasChanged", "Count" , "Exp Date");

  char msg[200];

  for(int i=0; i < n_elem ; ++i)
  {
      lic_info.featuresArray[i].LicenseFeature = lic_feat[i].capability_id;
      lic_info.featuresArray[i].IsEnabled = (lic_feat[i].count > 0 ? true : false);
      lic_info.featuresArray[i].LicenseStatus = mapLicenseStatus(lic_feat[i]);
      (isFirstRefreshMsgSent == false ? lic_info.featuresArray[i].IsChanged  = 0 : lic_info.featuresArray[i].IsChanged  = lic_feat[i].hasChanged);
      lic_info.featuresArray[i].Counted    = lic_feat[i].count;
      //lic_info.featuresArray[i].version    =  lic_feat[i].version;
      //lic_info.featuresArray[i].capabilityStr = lic_feat[i].capability;
      if( lic_feat[i].exp_date != NULL)
      {
          lic_info.featuresArray[i].expirationDate  = CStructTm(*lic_feat[i].exp_date);
          lic_info.featuresArray[i].expirationDate.DumpToBuffer(exp_date_buff);
      }
      else
      {
          CStructTm t;
          lic_info.featuresArray[i].expirationDate = t;
      }

      tbl.Add(lic_feat[i].capability.c_str(), (lic_feat[i].count > 0 ? true : false), lic_info.featuresArray[i].LicenseStatus, 
              lic_feat[i].hasChanged, lic_feat[i].count, exp_date_buff);
  }
  PTRACE(eLevelInfoNormal, tbl.Get().c_str());

  pSeg->Put((BYTE*)&lic_info, sizeof(FLEXERA_DATA_S));

  if(isFirstRefreshMsgSent == false)
  {
      isFirstRefreshMsgSent=true;
      msg_id=LICENSE_SERVER_FIRST_UPDATE_PARAMS_REQ; 
  }
  else
  {
      msg_id=LICENSE_SERVER_UPDATE_PARAMS_REQ;
  }
  api.SendMsg(pSeg, msg_id);
}


E_FLEXERA_LICENSE_VALIDATION_STATUS LicenseInfoSender::mapLicenseStatus(const LicensedItem& lic_feat ) const
{
   switch (lic_feat.status)
   {
     case eAcqStatusAcquired:
             if(lic_feat.reason == eAcqReasonNormalExceedReservation)
                 return E_FLEXERA_LICENSE_INSUFFICIENT_RESOURCES;    //actually means that reserved in FNEserver more than machine capacity
             else
                 return E_FLEXERA_LICENSE_VALID;
             break;
     case eAcqStatusFailed:
             if(lic_feat.reason == eAcqReasonExpired)
                 return E_FLEXERA_LICENSE_TIME_EXPIRED;

             else if (lic_feat.reason == eAcqReasonWindbackDetected)
            	    return E_FLEXERA_LICENSE_WINDBACK_DETECTED;

             else
                 return E_FLEXERA_LICENSE_INVALID;
             break;
     default:
             return E_FLEXERA_LICENSE_INVALID;
   }
}

void LicenseInfoSender::SendConnectionStatus(eLicensingConnectionStatus type)
{


  CManagerApi api(eProcessMcuMngr);
  CSegment *pSeg = new CSegment;


  *pSeg << type;

  api.SendMsg(pSeg, LICENSE_SERVER_CONNECTION_STATUS_IND);
}

void LicenseInfoSender::SendConnectionTime(eLicensingConnectionTime type,CStructTm curTime)
{


  CManagerApi api(eProcessMcuMngr);
  CSegment *pSeg = new CSegment;


  *pSeg   << type
	      << curTime.m_year
	      << curTime.m_mon
	      << curTime.m_day
	      << curTime.m_hour
	      << curTime.m_min
	      << curTime.m_sec;

  api.SendMsg(pSeg, LICENSE_SERVER_CONNECTION_TIME_IND);
}
