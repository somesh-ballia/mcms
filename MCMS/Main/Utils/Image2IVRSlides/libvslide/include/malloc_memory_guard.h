#ifndef __MALLOC_MEMORY_GUARD__
#define __MALLOC_MEMORY_GUARD__

#include <stdlib.h>

class MallocMemoryGuard
{
public:
    MallocMemoryGuard(void *pMem)
        : pMem_(pMem)
    {
    }
    ~MallocMemoryGuard()
    {
        if (pMem_)
        {
            free(pMem_);
            pMem_ = 0;
        }
    }

    void Detach()
    {
        pMem_ = 0;
    }
    
private:
    MallocMemoryGuard(MallocMemoryGuard const &);
    MallocMemoryGuard & operator=(MallocMemoryGuard const &);
    
    void * pMem_;
};

#endif

