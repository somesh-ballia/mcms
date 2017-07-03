// CapabilityResponse.h

#ifndef CAPABILITY_RESPONSE_H_
#define CAPABILITY_RESPONSE_H_

#include "FlcLicensing.h"
#include "FlcLicenseManager.h"
#include "FlcCapabilityResponse.h"

#include "PObject.h"


class CapabilityResponse : public CPObject
{
  CLASS_TYPE_1(CapabilityResponse , CPObject)

  public:
   CapabilityResponse(FlcLicensingRef lic);
   ~CapabilityResponse();
   virtual const char* NameOf() const { return "CapabilityResponse"; }
   FlcUInt8** getResponseMemory() { return &binaryResponse; }
   FlcSize*    getResponseMemorySize() { return &binaryResponseSize; }
   int     getRenewalInterval();
   bool    ProcessResponse();

  private:
   FlcErrorRef     error;
   FlcUInt8 *      binaryResponse;
   FlcSize         binaryResponseSize;
   FlcCapabilityResponseRef capabilityResponse;
   FlcLicensingRef licensing;
};


#endif

