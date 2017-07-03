// IConv.h

#ifndef ICONV_H_
#define ICONV_H_

#include <ostream>
#include <iconv.h>

#include "DataTypes.h"

class CIConv
{
public:
  static bool ConvertStringEncoding(const char* buffFrom,
                                    char* buffTo,
                                    const DWORD bufToMaxLen,
                                    const std::string& codeFrom,
                                    const std::string& codeTo,
                                    std::ostream& outErrorStr);

  static bool ValidateStringEncoding(const char *buffToTest,
                                     const char *encodingCode,
                                     std::ostream& outErrorStr);

private:
  CIConv(void);  // disable creation of instance

  static bool IConvOpenDescriptor(const char* codeFrom,
                                  const char* codeTo,
                                  iconv_t& outConvDesc,
                                  std::ostream& outErrorStr);

  static bool IConvConvertStringEncoding(iconv_t cd,
                                         const char* input,
                                         char* output,
                                         size_t outbytesleft,
                                         std::ostream& out);

  static bool IConvCloseDescriptor(iconv_t convDesc,
                                   std::ostream& outErrorStr);
};

#endif
