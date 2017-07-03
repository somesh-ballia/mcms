#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "ele_parcel.h"

#include "shared_memory_rw.h"

extern char * NDMStrSafeCopy(char *dst, const char *src, int max_dst_len);
namespace NetraDspMonitor
{

EleParcel::EleParcel(const char *parcelName)
    : m_parcel(0)
    , m_shmRW(0)
    , m_proxy(0)
{
    if (!strncmp(parcelName, INVALID_NAME, strlen(INVALID_NAME)))
        return;
    NDMStrSafeCopy(m_parName, parcelName, sizeof(m_parName));
    Allocate();
}

void EleParcel::Allocate()
{
    m_parcel = new ParcelData();
    m_shmRW  = new ShMemRW(m_parName);
    m_proxy  = new EleProxy(m_shmRW);
}

EleParcel::EleParcel(const EleParcel & ele_parcel)
{
    NDMStrSafeCopy(this->m_parName, ele_parcel.m_parName, sizeof(this->m_parName));
    Allocate();
    SetParcel(ele_parcel.GetParcel());
}

EleParcel & EleParcel::operator=(const EleParcel & ele_parcel)
{
    NDMStrSafeCopy(this->m_parName, ele_parcel.m_parName, sizeof(this->m_parName));
    Allocate();
    SetParcel(ele_parcel.GetParcel());
    return *this;
}

EleParcel::~EleParcel()
{
    delete m_proxy;
    delete m_shmRW;
    delete m_parcel;

    m_proxy = 0;
    m_shmRW = 0;
    m_parcel = 0;
}

const ParcelData *EleParcel::GetParcel() const
{
    return m_parcel;
}

void EleParcel::SetParcel(const ParcelData *one_parcel) 
{
    if (m_parcel && one_parcel)
    {
        *m_parcel = *one_parcel;
    }
}

ParcelData * EleParcel::ParcelRef() 
{
    return m_parcel;
}

void EleParcel::Download()
{
    if (m_proxy && m_proxy->Download(m_parcel))
        Unmarshing();
}

void EleParcel::Upload()
{
    if (m_proxy)
    {
        Marshing();
        m_proxy->Upload(m_parcel);
    }
}

}; // namespace NetraDspMonitor

