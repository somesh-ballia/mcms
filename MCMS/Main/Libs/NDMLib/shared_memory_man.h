#ifndef __SH_MEM_MAN_H__
#define __SH_MEM_MAN_H__


namespace NetraDspMonitor
{

struct SharedAddressFormat
{
    int offset;
    int maxSize;
    SharedAddressFormat()
        : offset(-1)
        , maxSize(-1)
    {
    }
    SharedAddressFormat(int os, int ms)
        : offset(os)
        , maxSize(ms)
    {
    }
};

const int    MAX_SHARED_MEN_CNT = 8;

class ShaMemMan
{
    struct SMMContent
    {
        char flag[16];
        struct
        {
            int shm_id;
        }shm[MAX_SHARED_MEN_CNT];
    } * c;
            
    int      m_nextPos[MAX_SHARED_MEN_CNT];
    char    *m_shm[MAX_SHARED_MEN_CNT];
    bool     m_creating[MAX_SHARED_MEN_CNT];

    void ManMemMap();
    void ManMemInit();
    void InitPrivateShM(int shm_index);
public:
    ShaMemMan();
    
    int Allocate(int max_size, const char *default_value, int shm_index);
    void AllocateComplete(int shm_index);
    char *Read(int offset, int shm_index);

    const char *WriteShmAddress(char *temp, int len, int offset, int max_size, int shm_index) const;
    void ReadShmAddress(const char *add, int & offset, int & max_size, int & shm_index) const;

    virtual ~ShaMemMan();

    
}; // ShaMemMan

extern ShaMemMan g_sha_mem_man;

}; // namespace NetraDspMonitor



#endif // __SH_MEM_MAN_H__

