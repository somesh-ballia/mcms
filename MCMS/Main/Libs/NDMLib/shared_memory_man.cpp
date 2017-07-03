#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <string>

using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h> 
#include <sys/shm.h>

#include "shared_memory_man.h"

#define GET_MCU_HOME_DIR (getenv("MCU_HOME_DIR") == NULL ? "" : getenv("MCU_HOME_DIR"))

namespace NetraDspMonitor
{

const int    INVALID_SHARED_MEM_ID = -1;

const int    MAX_SHARED_MEM_SIZ = (1024 * 1024);

const char   SHA_MEM_MAN_FLAG[16] = "polycompctc2013";

ShaMemMan g_sha_mem_man;

static int Shared_Memory_Get(int key, int max_size)
{
    int id;
    if ((id = shmget(key, max_size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return INVALID_SHARED_MEM_ID;
    }

    return id;
}

static char *Shared_Memory_Att(int id)
{
    char *shm;
    if ((shm = (char *)shmat(id, NULL, 0)) == (char *) -1)
    {
        perror("shmat");
        return NULL;
    }
    return shm;
}

ShaMemMan::ShaMemMan()
{
    for (int i = 0; i < MAX_SHARED_MEN_CNT; i++)
    {
        m_nextPos[i] = -1;
        m_shm[i] = 0;
    }
    ManMemMap();
    ManMemInit();
   
}

void ShaMemMan::ManMemMap()
{
	std::string SHA_MEM_MAN_ID_TOKEN = (std::string)GET_MCU_HOME_DIR+"/tmp/.ninja_vmp_sha_mem_man";
    mkdir(SHA_MEM_MAN_ID_TOKEN.c_str(), 0777);
    int shmm_id = Shared_Memory_Get(ftok(SHA_MEM_MAN_ID_TOKEN.c_str(), 'v'), sizeof(SMMContent));
    printf("shmm_id = %d\n", shmm_id);
    this->c = (SMMContent *)Shared_Memory_Att(shmm_id);
    //shmctl(shmm_id, IPC_RMID, NULL);
}

void ShaMemMan::InitPrivateShM(int shm_index)
{
    if (INVALID_SHARED_MEM_ID == this->c->shm[shm_index].shm_id)
    {
        this->c->shm[shm_index].shm_id = Shared_Memory_Get(IPC_PRIVATE, MAX_SHARED_MEM_SIZ);
        m_shm[shm_index] = Shared_Memory_Att(this->c->shm[shm_index].shm_id);
        memset(m_shm[shm_index], 0, MAX_SHARED_MEM_SIZ);
        m_creating[shm_index] = true;
    }
    else
    {
        m_shm[shm_index] = Shared_Memory_Att(this->c->shm[shm_index].shm_id);
        m_creating[shm_index] = false;
    }
    m_nextPos[shm_index] = 0;
}

void ShaMemMan::ManMemInit()
{
    if (!strcmp(SHA_MEM_MAN_FLAG, this->c->flag))
    {
        printf("Shared Memory Has been Created Before\n");
    }
    else
    {
        printf("First Create Here, So init it\n");
        
        strcpy(this->c->flag, SHA_MEM_MAN_FLAG);
        for (int i = 0; i < MAX_SHARED_MEN_CNT; i++)
        {
            this->c->shm[i].shm_id = INVALID_SHARED_MEM_ID;
        }
    }

    // Create one shared memory for storing data
    // for dsp monitor
#define DSP_MONITOR_SHM_INDEX 0
    InitPrivateShM(DSP_MONITOR_SHM_INDEX);

    // Create one for other
#define VIDEO_FLOW_SHM_INDEX 1
    InitPrivateShM(VIDEO_FLOW_SHM_INDEX);

    // Continue to add module here
    
}

int ShaMemMan::Allocate(int max_size, const char *default_value, int shm_index)
{
    // FIXME: How to sync with EleProxy::SetParcel2Message
    assert((int)strlen(default_value) < max_size);
    
    if (m_creating[shm_index])
    {
        memcpy(m_shm[shm_index] + m_nextPos[shm_index], default_value, 
            (int)strlen(default_value) < max_size - 1 ? strlen(default_value) : max_size - 1);
    }
    
    int start_index = m_nextPos[shm_index];
    m_nextPos[shm_index] += max_size;

    return start_index;
}

void ShaMemMan::AllocateComplete(int shm_index)
{
    m_creating[shm_index] = false;
}

char *ShaMemMan::Read(int offset, int shm_index)
{
    return m_shm[shm_index] + offset;
}

const char *ShaMemMan::WriteShmAddress(char *temp, int len, int offset, int max_size, int shm_index) const
{
    snprintf(temp, len, "%d %d %d", offset, max_size, shm_index);
    return temp;
}

void ShaMemMan::ReadShmAddress(const char *add, int & offset, int & max_size, int & shm_index) const
{
    assert((int)strlen(add) >= 5);
    sscanf(add, "%d %d %d", &offset, &max_size, &shm_index);
}

ShaMemMan::~ShaMemMan()
{
}

}; // namespace NetraDspMonitor

