#ifndef __FILE_GUARD_H__
#define __FILE_GUARD_H__

#include <stdio.h>

class FCloseFile
{
    FILE * fp_;
public:
    FCloseFile(FILE * fp) : fp_(fp){}
    ~FCloseFile()
    {
        if (fp_)
        {
            fclose(fp_);
        }
    }
};

class PCloseFile
{
    FILE * fp_;
public:
    PCloseFile(FILE * fp) : fp_(fp){}
    ~PCloseFile()
    {
        if (fp_)
        {
            pclose(fp_);
        }
    }
};

#endif

