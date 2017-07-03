// CapabilityResponse.cpp

#include "CapabilityResponse.h"
#include "stubs.h"
#include "Trace.h"
#include "TraceStream.h"


CapabilityResponse::CapabilityResponse(FlcLicensingRef lic)
{
    binaryResponse        = NULL;
    binaryResponseSize    = 0;
    FlcCapabilityResponseRef capabilityResponse;
    licensing =  lic;
    if (!FlcErrorCreate(&error))
    {
       TRACEINTO << "Failed to create Error Object";
    }
}

int CapabilityResponse::getRenewalInterval()
{
    FlcUInt32 renewInterval=0;

    if (!FlcCapabilityResponseGetRenewInterval(capabilityResponse, &renewInterval, error))
    {
       TRACEINTO << "Failed to get RenewInterval. Error: " << 
           (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
    }
    return renewInterval;
}

bool CapabilityResponse::ProcessResponse()
{
    if (binaryResponseSize == 0)
    {
        TRACEINTO << "Capability Response is empty. Error: "; 
               (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
        return FLC_FALSE;
    }
    if (!FlcProcessCapabilityResponseData( licensing, &capabilityResponse, binaryResponse, binaryResponseSize, error))
    {
        TRACEINTO << "Failed to ProcessCapabilityResponse. Error: " << 
                       (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error)) << " code " << FlcErrorGetCode(error) ;
        // patch some Flexera issue and prevent seg fault in dtor
        binaryResponseSize=0;

        return FLC_FALSE;
    }
    return FLC_TRUE;
}

CapabilityResponse::~CapabilityResponse()
{
   if(binaryResponseSize > 0)
   {
       if(!FlcCapabilityResponseDelete(licensing, &capabilityResponse, error))
       {
          TRACEINTO << "Failed to Delete Capability Response. Error: " << 
                              (char *)FlcErrorCodeGetDescription(FlcErrorGetCode(error));
       }
   }

   FlcMemoryFree(binaryResponse);

   FlcErrorDelete(&error);
}
