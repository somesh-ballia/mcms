#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "ele_proxy.h"

#define MAX_MESSAGE_SIZE 1024

namespace NetraDspMonitor
{

EleProxy::EleProxy(ShMemRW *shm_rw)
    : m_shmRW(shm_rw)
{
}

EleProxy::~EleProxy()
{
}

bool EleProxy::Download(ParcelData *parcel)
{
    char message[MAX_MESSAGE_SIZE];

    m_shmRW->Read(message, MAX_MESSAGE_SIZE);
    assert((int)strlen(message) < MAX_MESSAGE_SIZE);

    parcel->ClearWrite();
    parcel->Put(message);
    
    return true;
}

bool EleProxy::Upload(const ParcelData *parcel)
{
    assert(parcel->Len() > 0);
    
    char message[MAX_MESSAGE_SIZE];
    
    memcpy(message, parcel->Data(), parcel->Len());
    message[parcel->Len()] = 0; // For It's must be a string

    m_shmRW->Write(message, parcel->Len() + 1);
    
    return true;
}


}; // namespace NetraDspMonitory

