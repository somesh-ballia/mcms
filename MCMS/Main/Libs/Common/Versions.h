// Versions.h

#ifndef VERSIONS_H_
#define VERSIONS_H_

#include "SerializeObject.h"
#include "CommonStructs.h"
#include "StringsLen.h"

class CVersions : public CSerializeObject
{
  CLASS_TYPE_1(CVersions, CSerializeObject)

 public:
                            CVersions();
  virtual const char*       NameOf() const { return "CVersions"; }
  virtual CSerializeObject* Clone()        { return new CVersions; }

  virtual void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
  virtual int               DeSerializeXml(CXMLDOMElement* pActionNode,
                                           char*           pszError,
                                           const char*     action);

  const VERSION_S&          GetMcuVersion() const;
  const char*               GetMcuPrivateDescription() const;
  const char*               GetMcuDescription() const;
  const VERSION_S&          GetMcmsVersion() const;
  const char*               GetMcmsPrivateDescription() const;
  const char*               GetMcmsBaseline() const;
  const char*               GetMcuBaseline() const;
  const char*               GetMcuBuildDate() const { return m_mcuBuildDate; }

 private:
  VERSION_S m_mcuVersion;
  VERSION_S m_mcmsVersion;
  char      m_mcuPrivateDescription [PRIVATE_VERSION_DESC_LEN];
  char      m_mcmsPrivateDescription[PRIVATE_VERSION_DESC_LEN];
  char      m_mcmsVerBaseLine       [PRIVATE_VERSION_DESC_LEN];
  char      m_mcuVerBaseLine        [PRIVATE_VERSION_DESC_LEN];
  char      m_mcuDescription        [DESCRIPTION_LEN];
  char      m_mcuBuildDate          [14+1];  // Format is YYYYMMDDHHMMSS plus 1 for '\0'
};

#endif  // VERSIONS_H_
