#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GetMacAddr.h"
#include "tools.h"

static void ExtractStringByDelim(char const * head, char const * trail, char const * inStr, char * buf, size_t bufLen)
{
    char * pBeg = strstr(inStr, head);
    if (!pBeg)
    {
        buf[0] = '\0';
        return;
    }
    pBeg += strlen(head);

    char * pEnd = strstr(pBeg, trail);
    if (!pEnd)
    {
        pEnd = pBeg + strlen(pBeg) - 1;
    }

    char * trimStr = StringTrim(pBeg, pEnd);

    if(trimStr)
    {
    	  strncpy(buf, trimStr, bufLen);
        buf[bufLen -1] = '\0';
	  free(trimStr);
    }
    else
    {
    	  buf[0] = '\0';
    }
	
    return;
}

/*
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
*/
char const * GetMacAddrWithSemiColon(char const * devName, char * buf, size_t bufLen)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "/sbin/ifconfig %s", devName);
    FILE *fp = popen(bufCmdLine, "r");
    if (NULL==fp)
    {
        return "";
    }

    char const * anchor = "HWaddr";

    char line[128];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, anchor))
        {
            ExtractStringByDelim(anchor, "\n", line, buf, bufLen);
            break;
        }
    }

done:

    if(fp) pclose(fp);
    return buf;
}
