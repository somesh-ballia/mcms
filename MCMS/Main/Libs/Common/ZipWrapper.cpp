// ZipWrapper.cpp

#include "ZipWrapper.h"
#include <stdio.h>
#include "Trace.h"
#include "TraceStream.h"
#include "Macros.h"

static void DumpZStreamS(const char *title,
                         int err,
                         const z_stream& zStreamS,
                         char* outMsg,
                         int msgLen)
{
  snprintf(
      outMsg,
      msgLen,
      "%s err = %d \navail_in = %u, avail_out = %u, total_in = %lu\n"
      "total_out(%lu) + avail_out(%u) = buf_size(%lu) = allocated(%u)",
      title,
      err,
      zStreamS.avail_in,
      zStreamS.avail_out,
      zStreamS.total_in,
      zStreamS.total_out,
      zStreamS.avail_out,
      zStreamS.total_out + zStreamS.avail_out,
      MAX_NON_ZIPPED * 100);
}

// Static
int CZipWrapper::InflateByChunk(const unsigned char* in,
                                unsigned int len,
                                std::vector<unsigned char>& out)
{
    if (0 == len)
        return Z_OK;

    // Allocates inflate state
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    strm.avail_in = len;
    strm.next_in = const_cast<unsigned char*>(in);

    unsigned int have;
    unsigned char buf[256 * 1024];

    // Runs inflate() on input until output buffer not full
    do
    {
        strm.avail_out = ARRAYSIZE(buf);
        strm.next_out = buf;

        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_STREAM_END && ret != Z_OK)
            break;  // error

        have = ARRAYSIZE(buf) - strm.avail_out;
        out.insert(out.end(), buf, buf + have);

    } while (ret != Z_STREAM_END);  // Done when inflate() says it's done

    // Cleans up
    inflateEnd(&strm);

    return (ret == Z_STREAM_END) ? Z_OK : ret;
}

// Statis
// report a zlib or i/o error
std::string CZipWrapper::StrError(int err)
{
    switch (err)
    {
    case Z_STREAM_ERROR:
        return "Invalid compression level";

    case Z_DATA_ERROR:
    case Z_NEED_DICT:
        return "Invalid or incomplete deflate data";

    case Z_MEM_ERROR:
        return "Out of memory";

    case Z_VERSION_ERROR:
        return "Zlib version mismatch";

    default:
        {
            std::ostringstream buf;
            buf << "Unknown error code " << err;
            return buf.str();
        }
    }

    return std::string();
}

CZipWrapper::CZipWrapper(void)
{
	memset(&m_ZStream, 0, sizeof (z_stream));
}

int CZipWrapper::Deflate(unsigned char* pInput,
                         int nInputLen,
                         unsigned char* pOutput,
                         int nAllocatedOutputSize)
{
	int lastCharIdx = strlen(reinterpret_cast<char*>(pInput)) - 1;

	// remove unnecessary spaces (before zipping)
	char lastChar = pInput[lastCharIdx];
	if (' ' == lastChar)
	{
		while (lastChar)
		{
			lastCharIdx--;
			lastChar = pInput[lastCharIdx];

			if (' ' != lastChar)
				break;
		}
		
		pInput[lastCharIdx+1] = 0;
	}

	int retValue = deflateInit(&m_ZStream,Z_BEST_SPEED);	//should be 0 (=Z_OK)

	int nZippedLength = -1;

	retValue = deflateReset(&m_ZStream);
	
	m_ZStream.next_in = pInput;
	m_ZStream.avail_in = nInputLen;
	m_ZStream.next_out = pOutput;
	m_ZStream.avail_out = nAllocatedOutputSize;
	
	int err = deflate(&m_ZStream,Z_FINISH);
	
	retValue = deflateEnd(&m_ZStream);
	
	if(err == Z_STREAM_END)
		nZippedLength = m_ZStream.total_out;
		
	return nZippedLength;
}

int CZipWrapper::Inflate(unsigned char* pInput, int nInputLen, unsigned char* pOutput, int nOutputLen)
{
	int nUnZippedLength = -1;	
	
	m_ZStream.avail_in = 0;
	m_ZStream.next_in = Z_NULL;
	inflateInit(&m_ZStream);
	
	m_ZStream.next_in = pInput;
	m_ZStream.avail_in = nInputLen;
	m_ZStream.next_out = pOutput;
	m_ZStream.avail_out = nOutputLen;
	
	int err = inflate( &m_ZStream, Z_FINISH );
	
	inflateEnd(&m_ZStream);
	
	if(err == Z_STREAM_END)
		nUnZippedLength = m_ZStream.total_out;
	else
	{
        char msg[512];
        snprintf(msg, sizeof(msg)-1,
                 "CZipWrapper::Inflate - fail to decompress the string (string is too big? = %d)",
                 nInputLen);
		FPASSERTMSG(1, msg);
        
        DumpZStreamS("CZipWrapper::Inflate - failed", err, m_ZStream, msg, sizeof(msg) / sizeof(msg[0]));
        FPTRACE(eLevelInfoNormal,msg);	
	}
	return nUnZippedLength;	
}
