#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "parcel_data.h"

#define NDM_MIN(x, y)  \
        ((x) < (y) ? (x) : (y))

namespace NetraDspMonitor
{

ParcelData::ParcelData()
{
    ClearRead();
    ClearWrite();
}

ParcelData::ParcelData(const char * data, int len)
{
    assert(len < max_data_len - 1);
    ClearRead();
    if (len > 0)
        memcpy(m_data, data, len);
    m_wPointer = len == 0 ? 0 : len + 1;
    m_data[len] = 0;
    m_data[m_wPointer] = 0;
}

ParcelData::ParcelData(const ParcelData &parcel_data)
    : m_rPointer(0)
    , m_wPointer(0)
{
    CopyFrom(parcel_data);
}

ParcelData::~ParcelData()
{
}

bool ParcelData::Empty() const
{
    return m_wPointer == 0;
}

void ParcelData::CopyFrom(const ParcelData &parcel_data)
{
    if (parcel_data.m_wPointer == 0)
    {
        m_wPointer = parcel_data.m_wPointer;
        return;
    }

    assert(parcel_data.m_wPointer > 1);

    memset(m_data, 0, max_data_len);
    int len2cop = NDM_MIN(NDM_MIN((int)strlen(parcel_data.m_data), parcel_data.m_wPointer + 1), max_data_len - 1);
    memcpy(m_data, parcel_data.m_data, len2cop);
    
    ClearRead();
    m_wPointer = parcel_data.m_wPointer;
}

void ParcelData::ClearWrite()
{
    m_wPointer = 0;
    m_data[0] = 0;
}

void ParcelData::ClearRead()
{
    m_rPointer = 0;
}

const char *ParcelData::Data() const
{
    return m_data;
}

int ParcelData::Len() const
{
    return m_wPointer > 0 ? m_wPointer - 1 : 0;
}

void ParcelData::PutStr(const char *str)
{
    assert(str);
    assert(strlen(str));
    assert(m_wPointer + strlen(str) < max_data_len - 1);
    strcpy(m_data + m_wPointer, str);
    m_wPointer += strlen(str);
    m_data[m_wPointer++] = 0;
}

void ParcelData::GetStrCopy(char **str)
{
    if (!m_data[m_rPointer])
    {
        *str = 0;
        return;
    }
    char *rStr = new char[strlen(m_data + m_rPointer) + 1];
    
    strcpy(rStr, m_data + m_rPointer);
    m_rPointer += strlen(rStr) + 1;
    *str = rStr;
}

void ParcelData::Put(int data)
{
    char int_str[max_int_len] = {0};

    snprintf(int_str, sizeof(int_str), "%d", data);
    PutStr(int_str);
}

void ParcelData::Put(const char *data)
{
    PutStr(data);
}

bool ParcelData::Get(int * data)
{
    char *str;
    GetStrCopy(&str);
    if (!str)
    {
        ClearRead();
        return false;
    }
    
    *data = atoi(str);
    delete str;
    return true;
}
bool ParcelData::Get(char ** data)
{
    GetStrCopy(data);
    if (!*data)
    {
        ClearRead();
        return false;
    }
    return true;
}

ParcelData &ParcelData::operator=(const ParcelData &parcel_data)
{
    CopyFrom(parcel_data);
    return *this;
}  

}; // namespace NetraDspMonitor

