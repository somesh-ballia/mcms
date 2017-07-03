#ifndef _FILE_HANDLE_GUARD_HEADER_
#define _FILE_HANDLE_GUARD_HEADER_

#include <cstdio>

class FileHandleGuard
{
public:
    FileHandleGuard(FILE * fp) : fp_(fp) {}
    ~FileHandleGuard()
    {
        if (fp_)
        {
            fclose(fp_);
        }
    }
    
private:
    FILE * const fp_;
};

#endif

