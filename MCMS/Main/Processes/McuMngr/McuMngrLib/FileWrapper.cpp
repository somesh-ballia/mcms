#include <string.h>
#include <stdarg.h>
#include "FileWrapper.h"

CFile::CFile(const char *filename,const char *mode)
{
    strncpy(m_filename,filename,sizeof(m_filename)-1);
    m_fp=fopen(filename,mode);
}

bool CFile::Open(const char *filename,const char *mode)
{
    strncpy(m_filename,filename,sizeof(m_filename)-1);
    m_fp=fopen(filename,mode);
    return m_fp!=NULL;
}

void CFile::Close()
{
    if (m_fp!=NULL)
    {
        fclose(m_fp);
        m_fp=NULL;
    }
}

int CFile::Print(const char *fmt,...)
{
    va_list vararg;
    va_start(vararg,fmt);
    int ret=vfprintf(m_fp,fmt,vararg);
    va_end(vararg);
    return ret;
}

