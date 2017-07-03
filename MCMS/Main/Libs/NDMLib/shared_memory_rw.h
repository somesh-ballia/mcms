#ifndef __SH_MEM_RW_H__
#define __SH_MEM_RW_H__

#include <list>

namespace NetraDspMonitor
{
struct SharedMemAttached
{
    SharedMemAttached(int id_, char *ad_)
        : id(id_)
        , ad(ad_)
    {
    }
    int id;
    char *ad;
};

typedef SharedMemAttached * SharedMemAttached_t;

static const int max_shm_size = 1024;

class ShMemRW
{
    char *m_dataPointer;

    int m_dataLen;

    bool GetSharedMem(int key, int flag);
    bool GetFromAttachedList(int shm_id);

public:
    ShMemRW(const char *addr);
    virtual ~ShMemRW();
    
    virtual bool Write(const char *message, int actual_len);
    virtual bool Read(char *message, int large_len);

    const char *Debug(char *debug_str);
    
}; // SharedMem


}; // namespace NetraDspMonitor



#endif // __SH_MEM_RW_H__

