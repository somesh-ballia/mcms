#include "GetMacAddr.h"
#include "FileGuard.h"
#include "LineTokenizer.h"
#include "copy_string.h"
#include "strip_string.h"
#include <stdio.h>
#include <string.h>

namespace
{
    string ExtractStringByDelim(char const * head, char const * trail, char const * inStr)
    {
        char const * pBeg = strstr(inStr, head);
        if (!pBeg)
        {
            return "";
        }

        char const * pEnd = strstr(pBeg, trail);
        if (!pEnd)
        {
            return pBeg+strlen(head);
        }

        return string(pBeg+strlen(head), pEnd-pBeg-strlen(head));
    }
}

string GetMacAddrWithoutSemiColon(char const * devName)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "/sbin/ifconfig %s", devName);
    FILE *fp = popen(bufCmdLine, "r");
    if (NULL==fp)
    {
        return "";
    }

    PCloseFile cp(fp);

    char const * anchor = "HWaddr";

    char line[128];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, anchor))
        {
            std::string macStr = ExtractStringByDelim(anchor, "\n", line);
            inplace_trim(macStr);
            LineTokenizer lt(macStr, " \n:\t", LineTokenizer::STRIP_SPACE_NO);
            string result(macStr.size(), '\0');
            size_t len = 0;
            for (int i=0; i<lt.GetFieldNum(); ++i)
            {
                std::string const & field = lt.GetField(i);
                std::copy(field.begin(), field.end(), result.begin()+len);
                len += field.size();
            }
            result.resize(len);

            return result;
        }
    }

    return "";
}

char const * GetMacAddrWithSemiColon(char const * devName, char * buf, size_t bufLen)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "/sbin/ifconfig %s", devName);
    FILE *fp = popen(bufCmdLine, "r");
    if (NULL==fp)
    {
        return "";
    }

    PCloseFile cp(fp);

    char const * anchor = "HWaddr";

    char line[128];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, anchor))
        {
            std::string macStr = ExtractStringByDelim(anchor, "\n", line);
            inplace_trim(macStr);
            CopyString(buf, bufLen, macStr.c_str());
            return buf;
        }
    }

    return "";
}
