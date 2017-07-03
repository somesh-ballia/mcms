#ifndef __ELE_PROXYE_H__
#define __ELE_PROXYE_H__

#include "parcel_data.h"

#include "shared_memory_rw.h"

namespace NetraDspMonitor
{

class EleProxy
{
    static const int s_max_name_len = 128;

    char m_eleName[s_max_name_len];

    ShMemRW *m_shmRW;

public:
    EleProxy(ShMemRW *shm_rw);
    virtual ~EleProxy();
    
    virtual bool Download(ParcelData *);
    virtual bool Upload(const ParcelData *);

}; // EleProxy


}; // namespace NetraDspMonitor



#endif // __ELE_PROXYE_H__

