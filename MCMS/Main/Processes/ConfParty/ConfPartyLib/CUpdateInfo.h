#ifndef CUPDATEINFO_H_
  #define CUPDATEINFO_H_

#include "PObject.h"
#include "RsrvParty.h"
#include "XmlApi.h"
#include "PlcmCdrEventConfOperatorUpdateParty.h"

class CUpdateInfo : public CPObject
{
CLASS_TYPE_1(CUpdateInfo, CPObject )
public:
              CUpdateInfo();
            ~CUpdateInfo();

  virtual const char* NameOf() const { return "CUpdateInfo"; }
  friend WORD operator==(const CUpdateInfo& first, const CUpdateInfo& second);

  int SendDropParty();
  void SendUpdatePartyEventToCdr();

  CRsrvParty* m_pRsrvParty;
  DWORD       m_conferId;
  DWORD       m_partyId;

  // Fields for giving response to OperatorWS
  WORD        m_connectId;
  char        m_ActionName[H243_NAME_LEN]; // name of the action

  static void DeleteObject(CUpdateInfo* pObject) { POBJDELETE(pObject); }
};

#endif /*CUPDATEINFO_H_*/
