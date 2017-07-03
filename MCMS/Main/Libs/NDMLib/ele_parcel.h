#ifndef __ELE_PARCEL_H__
#define __ELE_PARCEL_H__

#include "parcel_data.h"
#include "ele_proxy.h"

#define INVALID_NAME "null_name"

namespace NetraDspMonitor
{

class EleParcel
{
    ParcelData  * m_parcel;
    ShMemRW     * m_shmRW;
    EleProxy    * m_proxy;

    char          m_parName[1024];

public:
    EleParcel(const char *name);
    EleParcel(const EleParcel & ele_parcel);
    EleParcel & operator=(const EleParcel & ele_parcel);
private:
    void Allocate();
public:
    virtual ~EleParcel();
    
    const ParcelData * GetParcel() const;
    void SetParcel(const ParcelData *one_parcel);
    
    virtual void Marshing() = 0;
    virtual void Unmarshing() = 0;

protected:
    ParcelData * ParcelRef();

    virtual void Download();
    virtual void Upload();
    
}; // EleParcel


}; // namespace NetraDspMonitor


#endif // __ELE_PARCEL_H__

