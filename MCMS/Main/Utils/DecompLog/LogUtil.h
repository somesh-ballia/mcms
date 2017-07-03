#ifndef LOGUTIL_H_
#define LOGUTIL_H_

#include <string>
#include <vector>
using namespace std;

#include "logdefs.h"



enum eStatus
{
	eOk,
	eIllegalInput,
	eDecompressFailed,
	eFatalError
};

static const char *StatusNames [] =
{
	"Ok",
	"Illegal Input",
	"Decompress Failed",
	"Fatal Error"
};  


static const char *ZIP_FILE_EXT = "log";
static const char *UNZIP_FILE_EXT = "txt";



// see zlib library for more details
static z_stream CompressionStream; 



// buffers
static const DWORD CHUNK 	= 16384;
static BYTE ZipBuffer		[CHUNK];
static BYTE UnZipBuffer		[CHUNK];



// files handlers
static FILE *ZipFileHandle 		= NULL; 
static FILE *UnZipFileHandle 	= NULL; 




// internal data structures for input file names
struct CFileNamePair
{
	string m_ZipFileName;
	string m_UnZipFileName;
public:	
	CFileNamePair(const string &zipFileName, const string &unZipFileName)
	{
		m_ZipFileName = zipFileName;
		m_UnZipFileName = unZipFileName;
	}
};
typedef vector<CFileNamePair> CFileNamesVector;



#endif /*LOGUTIL_H_*/
