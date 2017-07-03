// CapabilityRequest.cpp

#include "Trace.h"
#include "TraceStream.h"

#include "CapabilityRequest.h"

#include "stubs.h"


CapabilityRequest::CapabilityRequest(FlcLicensingRef lic)
{
   binaryRequest     = NULL;
   binaryRequestSize = 0;
   capabilityRequest = NULL;
   licensing =  lic;
   FlcErrorCreate(&error);
}

CapabilityRequest::~CapabilityRequest()
{
   if( capabilityRequest )
   {
       (void)FlcCapabilityRequestDelete(licensing, &capabilityRequest, NULL);
   }

   if( binaryRequest ) 
   {
       FlcMemoryFree(binaryRequest); 
   }

   FlcErrorDelete(&error);
}

int CapabilityRequest::CreateCapRequest(list<Reservations>& lic_info)
{

   if(!FlcCapabilityRequestCreate(licensing, &capabilityRequest, error))
   {
       TRACEINTO << "Failed to create capability request. Error: " 
                                   << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));

       return 1;
   }

   if(!FlcCapabilityRequestSetForceResponse(licensing, capabilityRequest, FLC_TRUE, error))
   {
       TRACEINTO << "Failed to set ForceResponse in capability request. Error: " 
                                   << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));

   }

   list<Reservations>::iterator it=lic_info.begin();

   for (it=lic_info.begin(); it!=lic_info.end(); ++it)
   {
       if (!FlcCapabilityRequestAddDesiredFeature(licensing, capabilityRequest,
                                                   (*it).first.first.c_str(),
                                                   (*it).first.second.c_str(),
                                                   (*it).second, error))
       {
            (void)FlcCapabilityRequestDelete(licensing, &capabilityRequest, NULL);
            TRACEINTO << "Failed to add feature " << (*it).first.first << " to the capability request. Error:" 
                                   << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
       }
   }

   if (!FlcCapabilityRequestGenerate(licensing, capabilityRequest, &binaryRequest, &binaryRequestSize, error))
   {
       (void)FlcCapabilityRequestDelete(licensing, &capabilityRequest, NULL);
       stringstream buf;
       buf << "Failed to generate capability request. Error [" << (char *)FlcErrorGetMessage(error) << "]" 
                                   << " Description [" << (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error)) << "]";

       TRACESTR(eLevelInfoNormal) << buf;
       return 1;
   }
   return 0;
}
