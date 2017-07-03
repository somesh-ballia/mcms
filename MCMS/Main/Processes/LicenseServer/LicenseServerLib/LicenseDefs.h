// LicenseDefs.h
//
#ifndef LICENSE_DEFS_H_
#define LICENSE_DEFS_H_

#include <string>
#include <list>

#include "FlcTypes.h"

#include "LicensingServerStructs.h"


using namespace std;

// Represents feature name and version
typedef pair<string, string> FeatureInfo;

//Represents feature version and count
typedef pair<string, int> ReservationInfo;

//Keeps all reservations that fits FeatureInfo (that might include different versions for a feature)
//Reservations that might be used for license aquisitions
typedef list<ReservationInfo> ReservationInfoCollection;

//Reservations as it stored in FNEServer
typedef pair<FeatureInfo, int> Reservations;

enum LicAcqStatus 
{ 
  eAcqStatusUnknown,
  eAcqStatusAcquired,
  eAcqStatusFailed
};

enum LicAcqStatusReason
{ 
  eAcqReasonNormal,
  eAcqReasonNormalExceedReservation,
  eAcqReasonExpired,
  eAcqReasonWrongCount,
  eAcqReasonWindbackDetected,
  eAcqReasonGenError
};

static const char * LicAcqStatusReasonStr[] =
{
	"Normal",
	"NormalExceedReservation",
	"Expired",
	"WrongCount",
    "WindbackDetected",
	"GenError"

};

typedef struct {
   E_FLEXERA_LICENSE_FEATURES capability_id;
   string capability;
   string version;
   int    req_count;
   int    count;
   bool   hasChanged;
   LicAcqStatus    status;
   LicAcqStatusReason    reason;
   struct tm*   exp_date;
} LicensedItem;

#endif

