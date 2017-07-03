#include "ZipCompressor.h"

CZipCompressor::CZipCompressor()
{
	memset(&m_CompressionStream,0,sizeof(z_stream));
}

CZipCompressor::~CZipCompressor()
{
}

bool CZipCompressor::IsInit()
{
	return m_CompressionStream.state != NULL;
}

int CZipCompressor::Init(BYTE *outBuffer, DWORD length)
{
	m_CompressionStream.zalloc = (alloc_func)0;
	m_CompressionStream.zfree = (free_func)0;
	m_CompressionStream.opaque = (voidpf)0;
	SetNextOut(outBuffer, length);
	
	int lErr = deflateInit(&m_CompressionStream, Z_BEST_SPEED);
	
	return lErr;
}

void CZipCompressor::SetNextIn(BYTE *inBuffer, DWORD length)
{
	m_CompressionStream.next_in		= inBuffer;
	m_CompressionStream.avail_in	= length;
}

void CZipCompressor::SetNextOut(BYTE *outBuffer, DWORD length)
{
	m_CompressionStream.next_out = outBuffer;
	m_CompressionStream.avail_out= length;
}

ULONG CZipCompressor::GetAvailIn()
{
	return m_CompressionStream.avail_in;
}

ULONG CZipCompressor::GetTotalIn()
{
	return m_CompressionStream.total_in;
}

ULONG CZipCompressor::GetTotalOut()
{
	return m_CompressionStream.total_out;
}

int CZipCompressor::Deflate(int flush)
{
	int lErr = deflate(&m_CompressionStream, flush);
	
	return lErr;
}

int CZipCompressor::DeflateReset(BYTE *outBuffer, DWORD length)
{
	int lErr = deflateReset (&m_CompressionStream);
	SetNextOut(outBuffer, length);
	return lErr;
}

int CZipCompressor::DeflateEnd()
{
	int lErr = deflateEnd(&m_CompressionStream); // release compression stream memory
    return lErr;
}	
