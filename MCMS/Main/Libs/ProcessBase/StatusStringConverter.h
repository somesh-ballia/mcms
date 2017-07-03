// StatusStringConverter.h

#ifndef STATUS_STRING_CONVERTER_H_
#define STATUS_STRING_CONVERTER_H_

#include <string>
#include "ConvertorBase.h"

class CXMLDOMElement;

class CStatusStringConverter : public CConvertorBase<STATUS, std::string>
{
 public:
                      CStatusStringConverter();

  virtual const char* NameOf() const { return "CStatusStringConverter"; }
  const std::string&  GetStringByStatus(STATUS status);
  void                AddStatusString(STATUS status, const std::string& str);

  void                SerializeApiStatuses(CXMLDOMElement* pLanguageNode);
  void                AddStatusNode(CXMLDOMElement* pLanguageNode,
                                    char strKey[7],
                                    const std::string& value);

 private:
  void                InitAllStatusString();
};

#endif
