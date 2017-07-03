#ifndef __COPY_STRING_H__
#define __COPY_STRING_H__

#include <string.h>
#include <string>

static inline char * CopyString(char * dest, size_t destBufSize, char const * src)
{
    size_t srcLen = strlen(src);
    int const copyLen = (destBufSize-1>srcLen) ? srcLen : destBufSize-1;
    memcpy(dest, src, copyLen);
    dest[copyLen] = 0;
    return (char *)dest;
}

template <int N>
char * CopyString(char (&dest)[N], char const * src)
{
    return CopyString((char *)dest, N, src);
}

template <int N>
char * CopyString(char (&dest)[N], std::string const & src)
{
    return CopyString((char *)dest, N, src.c_str());
}

#endif

