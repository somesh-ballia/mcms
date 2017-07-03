#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

class CFile
{
public:
    CFile() : m_fp(NULL)
    {
        memset(m_filename,0,PATH_MAX);
    }
    CFile(const char *filename,const char *mode);
    virtual ~CFile()
    {
        Close();
    }

    bool IsValid()
    {
        return m_fp!=NULL;
    }

    bool Open(const char *filename,const char *mode);
    void Close();

    int Read(char *buf,int len)
    {
        return fread(buf,1,len,m_fp);
    }
    bool ReadInt(int *buf)
    {
        return Read((char*)buf,4)==4;
    }
    bool ReadShort(short *buf)
    {
        return Read((char*)buf,2)==2;
    }
    bool ReadChar(char *buf)
    {
        return Read(buf,1)==1;
    }
    bool ReadUInt(uint32_t *buf)
    {
        return Read((char*)buf,4)==4;
    }
    bool ReadUShort(uint16_t *buf)
    {
        return Read((char*)buf,2)==2;
    }
    bool ReadUChar(uint8_t *buf)
    {
        return Read((char*)buf,1)==1;
    }
    int Write(const char *buf,int len)
    {
        return fwrite(buf,1,len,m_fp);
    }
    bool WriteInt(int val)
    {
        return Write((char*)&val,4)==4;
    }
    bool WriteShort(short val)
    {
        return Write((char*)&val,2)==2;
    }
    bool WriteChar(char val)
    {
        return Write(&val,1)==1;
    }
    bool WriteUInt(uint32_t buf)
    {
        return Write((char*)&buf,4)==4;
    }
    bool WriteUShort(uint16_t buf)
    {
        return Write((char*)&buf,2)==2;
    }
    bool WriteUChar(uint8_t buf)
    {
        return Write((char*)&buf,1)==1;
    }
    void Flush()
    {
        fflush(m_fp);
    }
    bool EndOfFile()
    {
        return feof(m_fp)!=0;
    }

    bool ReadString(char *buf,int size)
    {
        return fgets(buf,size,m_fp)!=NULL;
    }
    bool WriteString(const char *buf)
    {
        return fputs(buf,m_fp)!=EOF;
    }

    int Print(const char *fmt,...);

    int Seek(long offset,int whence)
    {
        return fseek(m_fp,offset,whence);
    }
    int SeekToBegin()
    {
        return Seek(0,SEEK_SET);
    }
    int SeekToEnd()
    {
        return Seek(0,SEEK_END);
    }
    int Tell()
    {
        return ftell(m_fp);
    }

    const char *GetFilename()
    {
        return m_filename;
    }
    int GetStatus(struct stat *buf)
    {
        return fstat(fileno(m_fp),buf);
    }
    static int GetStatus(const char *filename,struct stat *buf);

protected:
    char	m_filename[PATH_MAX];
    FILE*	m_fp;
};

inline int CFile::GetStatus(const char *filename,struct stat *buf)
{
    return stat(filename,buf);
}

#endif
