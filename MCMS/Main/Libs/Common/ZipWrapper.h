// ZipWrapper.h

#ifndef ZIP_WRAPPER_H_
#define ZIP_WRAPPER_H_

#include <vector>
#include <string>
#include "zlib.h"

#define MAX_NON_ZIPPED 10000

class CZipWrapper
{
public:
	CZipWrapper(void);
	
	int Deflate(unsigned char* pInput, int nInputLen, unsigned char* pOutput, int nAllocatedOutputSize);
	int Inflate(unsigned char* pInput, int nInputLen, unsigned char* pOutput, int nOutputLen);
	
	static std::string StrError(int ret);
	static int InflateByChunk(const unsigned char* in,
                              unsigned int len,
                              std::vector<unsigned char>& out);

private:
	z_stream m_ZStream;
}; 

#endif  // ZIP_WRAPPER_H_
