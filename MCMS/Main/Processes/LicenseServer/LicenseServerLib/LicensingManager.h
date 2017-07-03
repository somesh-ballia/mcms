// LicensingManager.h

#ifndef LICENSING_MANAGER_H_
#define LICENSING_MANAGER_H_

#include "PObject.h"


#include "FlcLicensing.h"
#include "FlcLicenseManager.h"
#include "FlcMachineType.h"
#include "FlcCapabilityRequest.h"
#include "FlcCapabilityResponse.h"
#include "FlcComm.h"
#include "FlcMessageStatus.h"


#include <string>

#include "time.h"

#include "LicenseDefs.h"
#include "LicenseInfoSender.h"
#include "HttpSAP.h"
#include "FneServerHttpRespParser.h"
#include "PrettyTable.h"

#include "stubs.h"

#include "LicensingFeatures.h"
#include "LicenseServerProcess.h"

#define LICENSING_DEBUG_FEATURES_LIST         ((std::string)(MCU_MCMS_DIR+"/Cfg/LicensingDebugFeatureList.xml"))


class CapabilityRequest;
class CapabilityResponse;

class LicensingManager : public CPObject
{
   CLASS_TYPE_1(LicensingManager , CPObject)

  public:
   LicensingManager(const string& path);
   ~LicensingManager();

   virtual const char* NameOf() const { return "LicensingManager";}
   void setPrimaryServerUri(const string& server, const DWORD& port);
   void setCustHostId( const string& hostid);
   void setLicenseVersion( string& version );
   void setMaxPortCapacity( int capacity ) { maxPortCapacity = capacity; }
   int  getMaxPortCapacity( ) { return maxPortCapacity; }

   void AcquireLicense();
   void AcquireLicenseFromTrustedStore();
   void InitLicenseServerParamsFromFile();

   CapabilityResponse* SendCapRequestToFneServer(CapabilityRequest* request) const;
   void init( );

   int getRenewalInterval() { return lic_renewal_intrvl; }

   void Send();

   void GetLastAcquiredTabled(ostream& msg);


  private:
   void reset( );
   void mapLicenseError(const FlcErrorRef error , LicensedItem& lic, CPrettyTable<const char*, const char*, int, const char*>* tbl);
   void FillExpTime(LicensedItem& lic, FlcLicenseRef license);
   int UpdateLicenseInfo(list<Reservations>& lic_info );
   void SendConnectionStatus(eLicensingConnectionStatus type);
   void SendConnectionTime(eLicensingConnectionTime type,CStructTm curTime);





   FlcBool MyFlcAcquireLicenses(FlcLicensingRef  licensing,
                              FlcLicenseRef *  license,
                              const FlcChar *  featureName,
                              const FlcChar *  featureVersion,
                              FlcUInt32        featureCount,
                              FlcErrorRef      error,
                              int idx);

   void MyMapLicenseError(const FlcErrorRef error, LicensedItem& lic_item, CPrettyTable<const char*, const char*, int, const char*>* tbl,int idx);

   FlcLicensingRef licensing;
   string ts_path;
   string primServerUri;
   string primHttpServerUri;
   string host_id;
   int    maxPortCapacity;
   FlcErrorRef error;
   FlcCommRef commInterface;
   FneServerHttpRespParser* fneServerHttpParser;
   HttpSAP fneServerHttp;
   LicenseInfoSender* licInfoSender;
   int lic_renewal_intrvl;
   bool isTsRead;

   //for debugging only
   CLicensingFeatures*  m_licensingFeatures;  //for debug only with export SIMULATE_FNO=YES
   //end only for debug section

   CLicenseServerProcess*   m_pProcess ;
};

#endif
