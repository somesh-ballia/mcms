// LicenseInfoSender.h

#ifndef LIC_INFO_SENDER_H
#define LIC_INFO_SENDER_H

#include "PObject.h"
#include "LicenseDefs.h"
#include "LicensingServerStructs.h"

using namespace std;

class LicenseInfoSender : public CPObject
{
 CLASS_TYPE_1(LicenseInfoSender, CPObject)

 public:
  LicenseInfoSender(LicensedItem* lic_feat, int num_elem);
  void Send();
  const char* NameOf() const { return "LicenseInfoSender"; }
  E_FLEXERA_LICENSE_VALIDATION_STATUS mapLicenseStatus(const LicensedItem& lic_feat ) const;
  void SendConnectionStatus(eLicensingConnectionStatus type);
  void SendConnectionTime(eLicensingConnectionTime type,CStructTm curTime);
 private:
  bool isFirstRefreshMsgSent;
  LicensedItem*  lic_feat;
  int            n_elem;
};

#endif

