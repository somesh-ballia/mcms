// CdrFullGet.h

#ifndef CDR_FULL_GET_H_
#define CDR_FULL_GET_H_

#include "Macros.h"
#include "SerializeObject.h"

class CCdrLongStruct;

class CCdrFullGet : public CSerializeObject, CNonCopyable
{
  CLASS_TYPE_1(CCdrFullGet, CSerializeObject)

 public:
                            CCdrFullGet();
  virtual                  ~CCdrFullGet();

  virtual const char*       NameOf() const;
  virtual CSerializeObject* Clone();
  virtual void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
  virtual int               DeSerializeXml(CXMLDOMElement* pActionNode,
                                           char* pszError,
                                           const char* action);

  DWORD                     GetConfId() const;
  DWORD                     GetFilePartIndex(void) const;
  void                      SetCDRLong(CCdrLongStruct* cdr_long);

 private:
  DWORD           m_confID;
  DWORD           m_partID;
  bool            m_no_partID;
  CCdrLongStruct* m_cdr_long;
};

#endif

