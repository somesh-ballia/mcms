#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h> 
#include <sys/shm.h>

#include "shared_memory_man.h"

#include "shared_memory_rw.h"

namespace NetraDspMonitor
{

#define SHM_INVALID_ADDR "0"
#define SHM_ADDR_PART_SEPRATE ' '

ShMemRW::ShMemRW(const char *addr)
    : m_dataPointer(0)
    , m_dataLen(0)
{
    int offset, max_size, shm_index;
    g_sha_mem_man.ReadShmAddress(addr, offset, max_size, shm_index);
    m_dataLen = max_size;
    m_dataPointer = g_sha_mem_man.Read(offset, shm_index);
}

ShMemRW::~ShMemRW()
{
}

bool ShMemRW::Write(const char *message, int actual_len)
{
    assert(m_dataLen >= actual_len);
    memcpy(m_dataPointer, message, actual_len);
    return true;
}

bool ShMemRW::Read(char *message, int large_len)
{
    assert(large_len > m_dataLen);
    if (!message)
    	return false;
    if (!m_dataPointer)
    	return false;
    if (m_dataLen <= 0)
    	return false;
    memcpy(message, m_dataPointer, m_dataLen);
    return true;
}

}; // namespace NetraDspMonitor

