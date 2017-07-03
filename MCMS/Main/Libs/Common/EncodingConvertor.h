#if !defined(_ENCODING_CONVERTOR_H__)
#define _ENCODING_CONVERTOR_H__

#include <string>
using namespace std;

#include "DataTypes.h"



class CEncodingConvertor
{
public:
    static STATUS Convert(const string & encodingTo,
                          string & outEncodingTypeFrom,
                          char *& outContent,
                          ostream &outErrorString);
    static STATUS ConvertValidate(const string & encodingTo,
                                  string & outEncodingFrom,
                                  char *& outContent,
                                  ostream & outErrorString);

    static void CutUtf8String(const char *buffer,    // should be NULL terminated string
                              char *outBuffer,       
                              const DWORD limitLength);

    static bool IsKnownEncoding(const string &encoding);
    
    static bool GetEncodingType(const char * buffer,
                                string & outEncodingType,
                                ostream & outErrorStr);

    static STATUS ValidateString(const string & encoding,
                                 const char *pContent,
                                 ostream & outErrorString);
    static STATUS ValidateFile(const string & encoding,
                               const string & fileName,
                               ostream & outErrorString);
    
private:
    // disabled
    CEncodingConvertor();

    static DWORD GetUtf8NextCharLength(const BYTE currentByte);
    static bool IsFirstByte(const BYTE currentByte);
    
};



#endif 
