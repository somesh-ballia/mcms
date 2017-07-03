// IConv.cpp

#include "IConv.h"

#include <errno.h>
#include <ostream>
#include <string.h>
#include <stdlib.h>

#include "Macros.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"

bool CIConv::IConvOpenDescriptor(const char *codeFrom,
                                 const char *codeTo,
                                 iconv_t & outConvDesc,
                                 std::ostream & outErrorStr)
{
    outConvDesc = iconv_open(codeTo, codeFrom);
    bool retStatus = ((iconv_t)-1 != outConvDesc);

    if (false == retStatus)
    {
        int errCode = errno;
        switch(errCode)
        {
            case EMFILE:
                outErrorStr <<
                    "file descriptors are currently open in the calling process.";
                break;

            case ENFILE:
                outErrorStr <<
                    "Too many files are currently open in the system.";
                break;

            case ENOMEM:
                outErrorStr <<
                    "Insufficient storage space is available.";
                break;

            case EINVAL:
                outErrorStr <<
                    "The conversion specified by fromcode and tocode is not supported by the implementation.";
                break;

            default:
                outErrorStr << errCode << " : Unknown error during iconv_open.";
                break;
        }
    }
    return retStatus;
}

bool CIConv::IConvConvertStringEncoding(iconv_t cd,
                                        const char* input,
                                        char* output,
                                        size_t outbytesleft,
                                        std::ostream& out)
{
  size_t inbytesleft = strlen(input);
  size_t res = iconv(cd, (char**)&input, &inbytesleft, &output, &outbytesleft);

  if ((size_t) -1 == res)
  {
    out << strerror(errno) << " (" << errno << ")";
    return false;
  }

  // Writes out the byte sequence to get into the
  // initial state if this is necessary, and terminate the outBuff
  res = iconv(cd, NULL, NULL, &output, (size_t*) &outbytesleft);
  if ((size_t) -1 == res)
  {
    out << strerror(errno) << " (" << errno << ")";
    return false;
  }

  *output = '\0';

  return true;
}

bool CIConv::IConvCloseDescriptor(iconv_t convDesc, std::ostream & outErrorStr)
{
  int res = iconv_close(convDesc);
  bool retStatus = (0 == res);

  if (false == retStatus)
    outErrorStr << "iconv_close failed";

  return retStatus;
}

bool CIConv::ConvertStringEncoding(const char *buffFrom,
                                   char *buffTo,
                                   const DWORD bufToMaxLen,
                                   const std::string &codeFrom,
                                   const std::string &codeTo,
                                   std::ostream &outErrorStr)
{
     iconv_t convDesc = (iconv_t)-1;
     bool resOpen = CIConv::IConvOpenDescriptor(codeFrom.c_str(),
                                                codeTo.c_str(),
                                                convDesc,
                                                outErrorStr);
     if(false == resOpen)
     {
         return false;
     }

     bool resConv = CIConv::IConvConvertStringEncoding(convDesc,
                                                       buffFrom,
                                                       buffTo,
                                                       bufToMaxLen,
                                                       outErrorStr);
     if(false == resConv)
     {
         // no return because the descriptor is still open, so it should be close.
     }

     bool resClose = CIConv::IConvCloseDescriptor(convDesc, outErrorStr);

     bool retStatus = resOpen & resConv & resClose;
     return retStatus;
}

// Static
bool CIConv::ValidateStringEncoding(const char* src,
                                    const char* code,
                                    std::ostream& out)
{
  FPASSERT_AND_RETURN_VALUE(NULL == src, false);

  iconv_t cd;
  bool res = CIConv::IConvOpenDescriptor(code, code, cd, out);
  if (!res)
    return false;

  size_t len = strlen(src);
  char* buf = (char*)malloc(len + 1);
  res = CIConv::IConvConvertStringEncoding(cd, src, buf, len + 1, out);
  free(buf);

  bool res2 = CIConv::IConvCloseDescriptor(cd, out);

  return res && res2;
}
