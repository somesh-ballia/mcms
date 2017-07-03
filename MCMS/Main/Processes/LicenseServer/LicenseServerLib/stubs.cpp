#include "FlcLicensing.h"
#include "FlcLicenseManager.h"
#include "FlcMachineType.h"
#include "FlcCapabilityRequest.h"
#include "FlcCapabilityResponse.h"
#include "FlcComm.h"
#include "FlcMessageStatus.h"

#include <stdlib.h>
#include <time.h>

#define NO_FLEXERA_64BIT_LIB
#ifdef FLEXERA_64BIT_LIB
extern "C" {


FlcBool FlcAddTrustedStorageLicenseSource( FlcLicensingRef licensing, FlcErrorRef     error) { return true; }
FlcBool FlcCapabilityRequestAddDesiredFeature( FlcLicensingRef licensing, FlcCapabilityRequestRef cap, const FlcChar* ver, const FlcChar* c, FlcInt32 cnt, FlcErrorRef error) { return true; }
FlcBool FlcCapabilityRequestCreate( FlcLicensingRef licensing, FlcCapabilityRequestRef* req, FlcErrorRef     error) { return true; }
FlcBool FlcCapabilityRequestDelete( FlcLicensingRef licensing, FlcCapabilityRequestRef* req, FlcErrorRef error) { return true; }
FlcBool FlcCapabilityRequestGenerate( FlcLicensingRef licensing, FlcCapabilityRequestRef req, FlcUInt8 **      request, FlcSize * requestSize, FlcErrorRef     error) { return true; }
FlcBool FlcCapabilityResponseDelete( FlcLicensingRef licensing, FlcCapabilityResponseRef*    capabilityResponse, FlcErrorRef     error) { return true; }
FlcBool FlcCommCreate( FlcCommRef* commInterface, FlcErrorRef  error) { return true; }
FlcBool FlcCommSetConnectTimeout( FlcCommRef commInterface, FlcUInt32 timeout,  FlcErrorRef     error) { return true; }
FlcBool FlcCommSetServer( FlcCommRef commInterface, const FlcChar* str, FlcErrorRef error) { return true; }
FlcBool FlcErrorCreate( FlcErrorRef*     error) { return true; }
FlcBool FlcErrorDelete( FlcErrorRef*     error) { return true; }
FlcBool FlcErrorGetCode( FlcErrorRef     error) { return true; }
FlcBool FlcFeatureCollectionDelete( FlcFeatureCollectionRef* features, FlcErrorRef     error) { return true; }
FlcBool FlcFeatureCollectionSize( FlcFeatureCollectionRef features, FlcSize* i, FlcErrorRef     error) { return true; }
FlcBool FlcGetTrustedStorageFeatureCollection( FlcLicensingRef licensing, FlcFeatureCollectionRef* features, FlcBool b, FlcErrorRef error) { return true; }
FlcBool FlcLicensingCreate(FlcLicensingRef* licensing, const FlcUInt8 *arr, FlcSize size, const FlcChar *str, const FlcChar * str2, FlcErrorRef     error) { return true; }
FlcBool FlcLicensingDelete(FlcLicensingRef* licensing, FlcErrorRef error) { return true; }
FlcBool FlcLicensingReset( FlcLicensingRef ref, FlcErrorRef     error) { return true; }
FlcBool FlcProcessCapabilityResponseData( FlcLicensingRef licensing, FlcCapabilityResponseRef* resp, const FlcUInt8* is, FlcSize si, FlcErrorRef     error) { return true; }
FlcBool FlcSetHostType( FlcLicensingRef licensing, const FlcChar* str, FlcErrorRef     error) { return true; }

FlcBool FlcGetAvailableAcquisitionCount(FlcLicensingRef licensing, const FlcChar * featureName, const FlcChar * featureVersion, FlcInt32 * count, FlcErrorRef error) { return true; }
FlcBool FlcAcquireLicenses(FlcLicensingRef  licensing,
                           FlcLicenseRef *  license,
                           const FlcChar *  featureName,
                           const FlcChar *  featureVersion,
                           FlcUInt32        featureCount,
                           FlcErrorRef      error) { return false; }

FlcBool FlcLicenseGetExpiration(FlcLicenseRef       license,
                                const struct tm **  expiration,
                                FlcErrorRef         error) 
{ 
    time_t ctime;

    ctime = time(NULL);
    
    struct tm* stm;
    stm = localtime( &ctime );
    //expiration = (const struct tm **)&stm;
    *expiration = stm;
return true; }

FlcBool FlcLicenseIsPerpetual(FlcLicenseRef       license,
                              FlcBool *           isPerpetual,
                              FlcErrorRef         error) { return true; }

const FlcChar * FlcErrorCodeGetDescription(FlcInt32 code) { return "this is FlcErrorCodeGetDescription"; }
const FlcChar * FlcErrorGetMessage(FlcErrorRef error)  { return "this is FlcErrorGetMessage"; }
FlcBool FlcDateIsPerpetual(const struct tm * date,
                           FlcBool *         isPerpetual,
                           FlcErrorRef       error) { return true; }

FlcBool FlcCommSendBinaryMessage(FlcCommRef    comm,
                                 const void *  sendData,
                                 FlcSize       sendDataLength,
                                 void **       recvData,
                                 FlcSize *     recvDataLength,
                                 FlcErrorRef   error) { return true; }

FlcBool FlcClockWindbackDetectionEnable(FlcLicensingRef licensing, FlcUInt32 i, FlcUInt32 ii, FlcErrorRef       error) { return true; }

FlcBool FlcCapabilityResponseGetRenewInterval(FlcCapabilityResponseRef    capabilityResponse, FlcUInt32* i,  FlcErrorRef  error) { *i=100; return true; }

FlcBool FlcCommDelete(FlcCommRef* comm,FlcErrorRef   error) { return true; }
FlcBool FlcErrorReset(FlcErrorRef   error) { return true; }

};
#endif
