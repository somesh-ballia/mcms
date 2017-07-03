// LicensingManager.cpp

#include "Trace.h"
#include "TraceStream.h"

#include "FlcLicensing.h"
#include "FlcLicenseManager.h"
#include "FlcWindbackDetection.h"

#include "IdentityClient.h"
#include "LicensingManager.h"
#include <sstream> 
#include <string.h>
#include "CapabilityRequest.h"
#include "CapabilityResponse.h"
#include "OsFileIF.h"

// TBD get from sys config
static const string serverDir="/fne/bin/capability"; 
static const string serverHttpDir="/fne/xml/reservations?detailed=true"; 
static const int    FneServerConnTimeout=5;
static const int    WindBackDetectInterval=86400;  //1 day in secs
static const string PlcmHostType="PLCM_VIRTUAL"; 
static const string VersionDate ="2014.1101";

// if license is not counted, will always acquire a single license (count=1)
// otherwise need to go to FNEServer and check for reservations
static const int NOTCOUNTED = 1;

// capability version is the same for all capabilities. Consider to remove this feild from struct and use some static string
//
static LicensedItem licensed_features2[] = { 
         { RPCS,                   "RPCS",                   VersionDate, NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPP_PKG,                "RPP_PKG",                "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { AVC_CIF_PLUS,           "AVC_CIF_PLUS",           "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { TIP,                    "TIP",                    "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPCS_MAX_PORTS,         "RPCS_MAX_PORTS",         "1.0",          0, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { MEDIA_ENCRYPTION,       "MEDIA_ENCRYPTION",       "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPCS_TELEPRESENCE,      "ITP",                    "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPCS_MULTIPLE_SERVICES, "RPCS_MULTIPLE_SERVICES", "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPCS_SVC,               "RPCS_SVC",               "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPCS_AVAYA,             "RPCS_AVAYA",             "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL},
         { RPCS_IBM,               "RPCS_IBM",               "1.0", NOTCOUNTED, 0, false, eAcqStatusUnknown, eAcqReasonNormal, NULL}
                                           };
static int nlicensed_features = sizeof(licensed_features2) / sizeof(licensed_features2[0]);


LicensingManager::LicensingManager(const string& ts_pathname)
{
   licensing = NULL;
   FlcErrorCreate(&error);
   commInterface = NULL;
   ts_path=ts_pathname;
   fneServerHttpParser=NULL;
   licInfoSender = new LicenseInfoSender(licensed_features2, nlicensed_features );
   isTsRead = false;

	m_licensingFeatures = new CLicensingFeatures(nlicensed_features,licensed_features2);

	m_pProcess = (CLicenseServerProcess*)CLicenseServerProcess::GetProcess();
}


void LicensingManager::setPrimaryServerUri(const string& server, const DWORD& port)
{
   stringstream ss;
   if( !server.empty() )
   {
       ss << (char *)"http://" << server <<  (char *)":"  << port;
   }

   primServerUri= ss.str() + serverDir;
   primHttpServerUri= ss.str() + serverHttpDir;

}

void LicensingManager::setCustHostId(const string& h_id)
{
   host_id = h_id;
}

void LicensingManager::init( )
{
	TRACEINTO << "\nLicensingManager::init START " ;

	if (m_pProcess->IsFlexeraSimulationMode())
	{
		TRACEINTO << "\nLicensingManager::init END " ;
		return;
	}


   reset();

   if(!FlcLicensingCreate( &licensing, identity_data, sizeof(identity_data),
            ts_path.c_str(), host_id.c_str(), error)  ) 
   {
       TRACEINTO << "Failed to create Licensing Object." << endl << "TS "<< ts_path << " host " << host_id
                 << ".Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
   }

   if(!FlcSetHostType(licensing, PlcmHostType.c_str(), error) ) 
   {
       TRACEINTO << "Failed to set Host Type." << endl << PlcmHostType
                 << ".Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
   }


   if(!FlcSetDefaultHostId(licensing, FLC_HOSTID_TYPE_VM_UUID, host_id.c_str() ,  error) )
    {
        TRACEINTO << "Failed to set Host Id " << endl << host_id
                  << ".Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
    }


   if(!FlcCommCreate(&commInterface, error) ||
          !FlcCommSetServer(commInterface, primServerUri.c_str(), error) )
   {
       TRACEINTO << "Fatal Error. Failed to Create Comm Object. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
       exit(1);
   }

   if(!FlcAddTrustedStorageLicenseSource(licensing, error))
   {

	   TRACEINTO << "Failed to add TrustedStorage LicenseSource. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));


	   STATUS statTmp;
	   std::string answerTmp;
	   std::string command = "rm -f "+(std::string)MCU_MCMS_DIR+"/TS/*";
	   statTmp = SystemPipedCommand(command.c_str(), answerTmp);


	   if(!FlcAddTrustedStorageLicenseSource(licensing, error))
		   TRACEINTO << "Failed to add TrustedStorage LicenseSource after clean TS directory. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));



   }

   if(!FlcClockWindbackDetectionEnable(licensing, WindBackDetectInterval, WindBackDetectInterval, error))
   {
       TRACEINTO << "Failed to set Windback Detection. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
   }

   if(!FlcCommSetConnectTimeout(commInterface, FneServerConnTimeout, error))
   {
       TRACEINTO << "Failed to set ConnectTimeout. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
   }

   if (NULL == fneServerHttpParser )
       fneServerHttpParser = new FneServerHttpRespParser( host_id );
        
   AcquireLicenseFromTrustedStore();
   TRACEINTO << "Flexera Licensing environment has been established. Server URL " << primServerUri;
}


// 1) sends HTTP query to get available reservations
// 2) for each feature look for a reservations. Note that it might be a number of reservations for a feature/version  
//    Note that the feature version might be <= than requested verision. 
// 3) Rebuild the license list for acquisition. This is needed because we need to have this list syncronized with reservation list
//    e.g. Initially we need to acquire license for feature A and version 2.0. In reservations we have entries for A/1.0, A/1.2, A/2.0
//    so when we try to acquire license we will acquire it for A/1.0, A/1.2, A/2.0
// 4) Prepares  capability request based on 3) 
// 5) Processes capability response. licensing variable holds all licensing information between the calls 
// 
int LicensingManager::UpdateLicenseInfo( list<Reservations>& lic_info )
{
	//TRACEINTO << "\nLicensingManager::UpdateLicenseInfo START " ;


	if (m_pProcess->IsFlexeraSimulationMode())
	{

		TRACEINTO << "\nLicensingManager::UpdateLicenseInfo END " ;
		return 0;
	}

	int ret_code=0;

    if( primServerUri.empty() )
       return 1;


    //EE-560
    SendConnectionStatus(eLicensingConnectionConnecting);

    string* http_resp = new string;

    // Rachel-flexera here its the rest api

    fneServerHttp.perform_request(primHttpServerUri, http_resp);

    fneServerHttpParser->parse_repsonse(*http_resp);

    PTRACE(eLevelInfoNormal, http_resp->c_str());

    for(int i=0; i < nlicensed_features ; ++i)
    {
    	//for now we are not supporting that capability
    	if (i == RPCS_MULTIPLE_SERVICES) continue;

        if(licensed_features2[i].req_count != NOTCOUNTED )
        {
            ReservationInfoCollection *col = fneServerHttpParser->get_reserv_for_feature(licensed_features2[i].capability, licensed_features2[i].version);

            //for each item in a collection create new license feature list
            //
            int reqCount;
            list<ReservationInfo>::iterator it= col->begin();
            for (it=col->begin(); it!=col->end(); ++it)
            {
                //limit port by MCU server capacity
                reqCount = ( (*it).second > maxPortCapacity ? maxPortCapacity : (*it).second );
                if ( (*it).second > maxPortCapacity )
                {
                    reqCount = maxPortCapacity;

                    licensed_features2[i].reason=eAcqReasonNormalExceedReservation;
                }
                else
                {
                    licensed_features2[i].reason=eAcqReasonNormal;
                    reqCount = (*it).second;
                }
     
                lic_info.push_back( Reservations( FeatureInfo(licensed_features2[i].capability, (*it).first), reqCount) );
            }
            delete col;
        }
        else
        {
            //add item to the list
            lic_info.push_back( Reservations( FeatureInfo( licensed_features2[i].capability, licensed_features2[i].version ), 1));
        }
    }
    delete http_resp;

    CapabilityRequest* capReq = new CapabilityRequest( licensing );
    
    CapabilityResponse *capResp;

    // Rachel-flexera here I want to get the GMT time for last request.
    CStructTm curTime;
    curTime.SetGmtTime();

    SendConnectionTime(eLicensingConnectionLastAttemptTime,curTime);


    if( !capReq->CreateCapRequest(lic_info) )
    {
       capResp = SendCapRequestToFneServer( capReq );
       if( capResp->ProcessResponse() )
           lic_renewal_intrvl = capResp->getRenewalInterval();
       else
           ret_code=1;

       delete capResp;
   }
   else 
       ret_code=1;

   delete capReq;

   return ret_code;
}

void LicensingManager::AcquireLicenseFromTrustedStore()
{
    FlcLicenseRef license = NULL;
    int availCnt;
    char exp_date_buf[20];

    CPrettyTable<const char*, const char*, int, const char*>
        tbl("Feature", "Version", "Count", "Exp Date");

    if( !isTsRead )
       isTsRead = true;
    else
       return;

    for(int i=0; i < nlicensed_features ; ++i)
    {
        availCnt=0;
        if (!FlcGetAvailableAcquisitionCount(licensing, licensed_features2[i].capability.c_str() , licensed_features2[i].version.c_str(), &availCnt, error))
        {
            TRACEINTO << "Failed to get available count for " << licensed_features2[i].capability.c_str() << " " << licensed_features2[i].version.c_str()
                      << ". Error " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
        }

        if(availCnt==0 || 
           !FlcAcquireLicenses(licensing, &license, licensed_features2[i].capability.c_str() , licensed_features2[i].version.c_str(), NOTCOUNTED, error))
        {
            availCnt=0;
            mapLicenseError( error, licensed_features2[i], NULL);
        }
        else
        {
            licensed_features2[i].status=eAcqStatusAcquired;
            licensed_features2[i].reason=eAcqReasonNormal;
        }
        licensed_features2[i].hasChanged=false;

        if(availCnt==0)
           continue;
        
        licensed_features2[i].count=availCnt;

        FillExpTime(licensed_features2[i], license);
        sprintf(exp_date_buf,"%02d.%02d.%04d %02d:%02d", licensed_features2[i].exp_date->tm_mday, (licensed_features2[i].exp_date->tm_mon+1), 
                             (1900 + licensed_features2[i].exp_date->tm_year), licensed_features2[i].exp_date->tm_min, licensed_features2[i].exp_date->tm_hour);
        tbl.Add(licensed_features2[i].capability.c_str(), licensed_features2[i].version.c_str(), availCnt, exp_date_buf);
    }

    //after we acquired license information we can release it
    if(!FlcReturnAllLicenses(licensing, error))
    {
       TRACEINTO << "Failed to Return License Information. Error: " << 
           (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
    }
    
    if( !tbl. IsEmpty() )
        TRACEINTO << "License information from the trusted store" << endl << tbl.Get();
    else
        TRACEINTO << "Trusted Store is empty.";

}

void LicensingManager::AcquireLicense()
{
	//TRACEINTO << "\nLicensingManager::AcquireLicense START " ;

    FlcLicenseRef license = NULL;
    int availCnt=0;

    list<Reservations> lic_info;

    if( 0 != UpdateLicenseInfo( lic_info ))
    {
        //no need to require license if cap request has failed
        return;
    }


    CPrettyTable<const char*, const char*, int, const char*>
        tbl("Feature", "Version", "Count", "Failed Reason");

    for(int i=0; i < nlicensed_features ; ++i)
    {
       if(licensed_features2[i].req_count == NOTCOUNTED)
       {


            if (!MyFlcAcquireLicenses(licensing, &license, licensed_features2[i].capability.c_str() , licensed_features2[i].version.c_str(), NOTCOUNTED, error,i))
            {
                availCnt=0;
                MyMapLicenseError( error, licensed_features2[i], &tbl,i);

            }
            else
            {
            	// Rachel-flexera here I want to get the GMT time for last successfull request.TBD - is it on RPCS only or per capability.
            	CStructTm curTime;
            	curTime.SetGmtTime();

            	SendConnectionTime(eLicensingConnectionSuccessTime,curTime);


            	licensed_features2[i].status=eAcqStatusAcquired;
            	licensed_features2[i].reason=eAcqReasonNormal;
            	availCnt=1;

            }

           // TRACEINTO << "\nLicensingManager::AcquireLicense NOTCOUNTED availCnt = " << availCnt;
       }
       else
       {
            availCnt=0;


            if (m_pProcess->IsFlexeraSimulationMode())
            {

            	availCnt = m_licensingFeatures->m_featureList[i].count;
            	TRACEINTO << "\nLicensingManager::AcquireLicense availCnt = " << availCnt;
            }
            else
            {
            	list<Reservations>::iterator it= lic_info.begin();
            	for (it=lic_info.begin(); it!=lic_info.end(); ++it)
            	{
            		if( !licensed_features2[i].capability.compare( (*it).first.first.c_str()) )
            		{
            			if (!FlcAcquireLicenses(licensing, &license, (*it).first.first.c_str(), (*it).first.second.c_str() , (*it).second, error))
            			{

            				TRACEINTO << "Failed to Acquire license for " << (*it).first.first.c_str() << " " << (*it).first.second.c_str()
            								<< " " << (*it).second << ". Error " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
            			}
            			else
            				availCnt+=(*it).second;

            		}
            	}
            }
            if( availCnt == 0)
            {
                MyMapLicenseError( error, licensed_features2[i], &tbl,i);
            }
            else
            {
            	if( licensed_features2[i].reason != eAcqReasonNormalExceedReservation )
            		licensed_features2[i].reason=eAcqReasonNormal;

            	licensed_features2[i].status=eAcqStatusAcquired;

            	//EE-560
            	CStructTm curTime;
            	curTime.SetGmtTime();

            	SendConnectionTime(eLicensingConnectionSuccessTime,curTime);

            }
       }

       if ( licensed_features2[i].count == availCnt )
       {
           licensed_features2[i].hasChanged = false;
       }
       else
       {
           licensed_features2[i].count=availCnt;
           licensed_features2[i].hasChanged = true;
       }

       if( availCnt == 0 )
    	   continue;


       FillExpTime(licensed_features2[i], license);
    }

    if (m_pProcess->IsFlexeraSimulationMode())
    {
    	if( !tbl. IsEmpty() )
    	{
			TRACEINTO << tbl.Get();
    	}

    	TRACEINTO << "\nLicensingManager::AcquireLicense END " ;

    	return;
    }

    //after we acquired license information we can release it
    if(!FlcReturnAllLicenses(licensing, error))
    {
       TRACEINTO << "Failed to Return License Information. Error: " << 
           (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
    }

    if( !tbl. IsEmpty() )
        TRACEINTO << tbl.Get();
}

CapabilityResponse*  LicensingManager::SendCapRequestToFneServer(CapabilityRequest* request) const
{
    CapabilityResponse* capResponce = new CapabilityResponse(licensing);
    
    if(!FlcCommSendBinaryMessage(commInterface, (void *)request->getRequestMemory(), request->getRequestMemorySize(),
                                (void **)capResponce->getResponseMemory(), capResponce->getResponseMemorySize(), error))
    {
        TRACEINTO << "Failed to send capability request. Error:" << 
            (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
    }
    return capResponce;
}

LicensingManager::~LicensingManager()
{
   reset();

   FlcErrorDelete(&error);

   if(fneServerHttpParser)
   {
       delete fneServerHttpParser;
       fneServerHttpParser=NULL;
   }

   if(licInfoSender)
        delete licInfoSender;

   if(m_licensingFeatures)
         delete m_licensingFeatures;

}

void LicensingManager::reset()
{
   if(licensing != NULL)
   {
       if(!FlcLicensingReset(licensing, error))
       {
           TRACEINTO << "Failed to Reset License Object. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
       }

       if(!FlcLicensingDelete(&licensing, error))
       {
           TRACEINTO << "Failed to Delete License Object. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
       }
       licensing=NULL;
   }

   if(commInterface != NULL) 
   {
      if (!FlcCommDelete(&commInterface, error))
      {
          TRACEINTO << "Failed to Delete Comm Object. Error: " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
      }
      commInterface=NULL;
   }

   FlcErrorReset(error);
}


void LicensingManager::mapLicenseError(const FlcErrorRef error, LicensedItem& lic_item, CPrettyTable<const char*, const char*, int, const char*>* tbl)
{
   lic_item.status=eAcqStatusFailed;
   
   FlcInt32 errCode = FlcErrorGetCode(error);

   switch(errCode)
   {
     case FLCERR_FEATURE_EXPIRED : 
         lic_item.reason=eAcqReasonExpired;
         break;

     case FLCERR_FEATURE_INSUFFICIENT_COUNT : 
         lic_item.reason=eAcqReasonWrongCount;
         break;

     case FLCERR_WINDBACK_DETECTED:
    	 lic_item.reason=eAcqReasonWindbackDetected;

     default:
         lic_item.reason=eAcqReasonGenError;
   }

   if(tbl != NULL)
       tbl->Add(lic_item.capability.c_str(), lic_item.version.c_str(), lic_item.count, (const char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error)));

   // if RPCS license failed, set interval to 0, so it will be taken from configuration
   if( !lic_item.capability.compare("RPCS") )
       lic_renewal_intrvl=0;
}

void LicensingManager::Send()
{
	//TRACEINTO << "\nLicensingManager::Send  " ;

   licInfoSender->Send();
}

void LicensingManager::SendConnectionStatus(eLicensingConnectionStatus type)
{
	TRACEINTO << "\nLicensingManager::SendConnectionStatus  " ;

   licInfoSender->SendConnectionStatus( type);
}

void LicensingManager::SendConnectionTime(eLicensingConnectionTime type,CStructTm curTime)
{
	TRACEINTO << "\nLicensingManager::SendConnectionTime  " ;

   licInfoSender->SendConnectionTime( type, curTime);
}

void LicensingManager::FillExpTime(LicensedItem& lic_item, FlcLicenseRef license)
{

	if (m_pProcess->IsFlexeraSimulationMode())
	{
		TRACEINTO << "\nLicensingManager::IsFlexeraSimulationMode END " ;
		return;
	}

   const struct tm* date = NULL;

   if(!FlcLicenseGetExpiration (license, &date, error))
   {
       TRACEINTO << "Failed to get expiration time. Error " << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
   }

   if( lic_item.exp_date == NULL )
   {
       lic_item.exp_date = new struct tm;
   }

   memcpy(lic_item.exp_date, date, sizeof( struct tm ));
}



////////////////////////////////////////////////////////////////////////
void LicensingManager::InitLicenseServerParamsFromFile()
{
	 TRACEINTO << "\nLicensingManager::InitLicenseServerParamsFromFile START" ;

	bool fileExist = IsFileExists(LICENSING_DEBUG_FEATURES_LIST.c_str());

	if (fileExist)
	{

		m_licensingFeatures->ReadXmlFile(LICENSING_DEBUG_FEATURES_LIST.c_str());


		TRACEINTO << "\nLicensingManager::InitLicenseServerParamsFromFile update licensed_features2 array " ;

		int prevCount=0;
		for(int i=0;i<nlicensed_features;i++)
		{
			//if (m_licensingFeatures->m_featureList[i].req_count == NOTCOUNTED)
			//{
				prevCount = licensed_features2[i].count;
			    memcpy((char *)&licensed_features2[i],(char *)&m_licensingFeatures->m_featureList[i],sizeof(LicensedItem));
			    licensed_features2[i].count = prevCount;
			//}
			//else
				 //  memcpy((char *)&licensed_features2[i],(char *)&m_licensingFeatures->m_featureList[i],sizeof(LicensedItem));


		}
		//memcpy((char *)&licensed_features2[0],(char *)&m_licensingFeatures->m_featureList[0],sizeof(LicensedItem)*nlicensed_features);



	}
	else
		m_licensingFeatures->WriteXmlFile(LICENSING_DEBUG_FEATURES_LIST.c_str());




}

//FlcBool LicensingManager::MyFlcAcquireLicenses(licensing, FlcLicenseRef *  license, licensed_features2[i].capability.c_str() , licensed_features2[i].version.c_str(), NOTCOUNTED, error,int idx)
FlcBool LicensingManager::MyFlcAcquireLicenses(FlcLicensingRef  licensing,
                            FlcLicenseRef *  license,
                            const FlcChar *  featureName,
                            const FlcChar *  featureVersion,
                            FlcUInt32        featureCount,
                            FlcErrorRef      error,
                            int idx)
{
	 //TRACEINTO << "\nLicensingManager::MyFlcAcquireLicenses START" ;


	 if (m_pProcess->IsFlexeraSimulationMode())
	{
            if (m_licensingFeatures->m_featureList[idx].status == eAcqStatusAcquired)
            {
            	 TRACEINTO << "\nLicensingManager::MyFlcAcquireLicenses idx " << idx << " status eAcqStatusAcquired" ;
            	return true;
            }
            else
            {
            	 TRACEINTO << "\nLicensingManager::MyFlcAcquireLicenses idx " << idx << " status NOT Acquired the status is"  << m_licensingFeatures->m_featureList[idx].status;
            	return false;
            }
	}
	else
		//return FlcAcquireLicenses(licensing, &license, licensed_features2[i].capability.c_str() , licensed_features2[i].version.c_str(), NOTCOUNTED, error);
	    return FlcAcquireLicenses(licensing, license, featureName , featureVersion, featureCount, error);
}

void LicensingManager::MyMapLicenseError(const FlcErrorRef error, LicensedItem& lic_item, CPrettyTable<const char*, const char*, int, const char*>* tbl,int idx)
 {
	 //TRACEINTO << "\nLicensingManager::MyMapLicenseError START" ;

	 if (m_pProcess->IsFlexeraSimulationMode())
	   {
		   lic_item.status = eAcqStatusFailed;
		   lic_item.reason = m_licensingFeatures->m_featureList[idx].reason ;

		   if(tbl != NULL)
		         tbl->Add(lic_item.capability.c_str(), lic_item.version.c_str(), lic_item.count, (const char *)LicAcqStatusReasonStr[lic_item.reason]);
	   }
	   else
		   mapLicenseError( error, lic_item, tbl);
}

void LicensingManager::GetLastAcquiredTabled(ostream& msg)
{
	if (m_licensingFeatures!=NULL)
		m_licensingFeatures->Dump(msg);
}








