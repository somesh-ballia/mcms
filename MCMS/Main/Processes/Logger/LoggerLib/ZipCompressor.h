#ifndef ZIPCOMPRESSOR_H_
#define ZIPCOMPRESSOR_H_

#include "PObject.h"
#include "zlib.h"


class CZipCompressor : public CPObject
{
CLASS_TYPE_1(CZipCompressor, CPObject)	
public:
	CZipCompressor();
	virtual ~CZipCompressor();
	    
	virtual const char* NameOf() const { return "CZipCompressor";}
	bool IsInit();    
	int Init(BYTE *outBuffer, DWORD length);
	void SetNextIn(BYTE *inBuffer, DWORD length);
	void SetNextOut(BYTE *outBuffer, DWORD length);
	ULONG GetAvailIn();
	ULONG GetTotalIn();
	ULONG GetTotalOut();
	int Deflate(int flush);
	int DeflateReset(BYTE *outBuffer, DWORD length);
	int DeflateEnd();
	
private:
	// disabled
	CZipCompressor(const CZipCompressor&);
	CZipCompressor&operator=(const CZipCompressor&);

	z_stream m_CompressionStream;	
};

#endif /*ZIPCOMPRESSOR_H_*/
