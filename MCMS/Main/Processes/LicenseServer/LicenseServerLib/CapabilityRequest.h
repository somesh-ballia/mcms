// CapabilityRequest.h

#ifndef CAPABILITY_REQUEST_H_
#define CAPABILITY_REQUEST_H_

#include "FlcLicensing.h"
#include "FlcLicenseManager.h"
#include "FlcCapabilityRequest.h"

#include "PObject.h"

#include "LicenseDefs.h"

class CapabilityRequest : public CPObject
{
  CLASS_TYPE_1(CapabilityRequest , CPObject)

  public:
      CapabilityRequest(FlcLicensingRef lic);
      ~CapabilityRequest();
      virtual const char* NameOf() const { return "CapabilityRequest";}
      int CreateCapRequest(list<Reservations>& lic_info);
      FlcUInt8 * getRequestMemory() { return binaryRequest; }
      FlcSize    getRequestMemorySize() { return binaryRequestSize; }


  private:
      FlcLicensingRef licensing;
      FlcCapabilityRequestRef capabilityRequest;
      FlcUInt8 *  binaryRequest;
      FlcSize     binaryRequestSize;
      FlcErrorRef error;
};

#endif
