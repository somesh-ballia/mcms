// CdrSettingsGet.h

#ifndef CDR_SETTINGS_GET_H_
#define CDR_SETTINGS_GET_H_

#include "Macros.h"
#include "SerializeObject.h"
#include "CDRProcess.h"


class CCdrSettingsGet : public CSerializeObject
{
  CLASS_TYPE_1(CCdrSettingsGet, CSerializeObject)

 public:
  	  	  	  	  	  	   CCdrSettingsGet();
  virtual                  ~CCdrSettingsGet();
  CCdrSettingsGet& operator = (const CCdrSettingsGet &other);
  void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int               DeSerializeXml(CXMLDOMElement* pActionNode,
                                           char* pszError,
                                           const char* action);
  CSerializeObject* Clone() {return new CCdrSettingsGet();}

 protected:
  CCDRProcess* m_pProcess;
};

#endif

