#include "GetIFList.h"
#include "LineTokenizer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

namespace
{
    bool CompareIfInfo(IFInfo const & lhs, IFInfo const & rhs)
    {
        return lhs.index < rhs.index;
    }
}

void GetIFList(vector<IFInfo> &ifs)
{
    ifs.clear();
    
    char const * const s_procDevFile = "/proc/net/dev";

    std::ifstream netDevFile(s_procDevFile);
    if (!netDevFile) return;

    std::string line;
    while (std::getline(netDevFile, line))
    {
        //std::cout << line << "********" << std::endl << std::endl;
        std::string::size_type idx = line.find("eth");
        if (std::string::npos==idx || line.length()<(idx+4)) continue;

        IFInfo ifInfo;
        bzero(&ifInfo, sizeof(ifInfo));


        std::string tmpName = line.substr(idx, 4);
        ifInfo.index = tmpName[3]-'0';

        strncpy(ifInfo.name, tmpName.c_str(),sizeof(ifInfo.name)-1);

        ifs.push_back(ifInfo);
    }

    std::sort(ifs.begin(), ifs.end(), CompareIfInfo);
}

