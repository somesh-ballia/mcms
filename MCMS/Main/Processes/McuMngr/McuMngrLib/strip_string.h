#ifndef __STRIP_STRING_H__
#define __STRIP_STRING_H__

#include <ctype.h>
#include <string.h>
#include <string>
#include <algorithm>

inline void inplace_trim(char* s)
{
    int start;
    for (start = 0; s[start] && isspace(s[start]); ++start)
    {
    }

    int end = strlen(s);
    if (s[start])
    {
        while (end > 0 && isspace(s[end-1]))
        {
            --end;
        }
    }
    memmove(s, &s[start], end - start);
    s[end - start] = '\0';
}

inline std::string & inplace_ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

inline std::string & inplace_rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

inline std::string & inplace_trim(std::string &s)
{
    return inplace_ltrim(inplace_rtrim(s));
}

#endif

