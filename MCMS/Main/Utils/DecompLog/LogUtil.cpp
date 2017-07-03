#include <zlib.h>
#include <iostream>
#include <sys/stat.h>

#include <stdlib.h>
#include "LogUtil.h"


int use_stdout = 0;

void Exit(eStatus code)
{
	cout << "Exit with code " << StatusNames[code] << endl << endl << endl;
	exit(code); 
}

void FatalError(const char *ErrorMessage, const char *param, const DWORD ErrorCode)
{	
	const char *errorStr = zError(ErrorCode);
	printf("%s : %s Error Code: %s\r\n",ErrorMessage, param, errorStr);
	deflateEnd(&CompressionStream);

	if(NULL != ZipFileHandle)
	{
		fclose(ZipFileHandle);
	}
	if(NULL != UnZipFileHandle)
	{
		fclose(UnZipFileHandle);
	}
	
	Exit(eFatalError);
}


DWORD GetFileSize(const string &fileName)
{
	struct stat Buf;
    int res = stat (fileName.c_str(), &Buf);
    if (-1 == res)
    {
        FatalError("stat failed", fileName.c_str(), 0);
    }
    return Buf.st_size;
}

bool CreateUnZipFileName(const string &zipFileName, string &unZipFileName)
{
	const int zipFileNameLen = zipFileName.length();
	const string ext = zipFileName.substr(zipFileNameLen - 3, zipFileNameLen);
	if(ext != ZIP_FILE_EXT)
	{
		return false;
	}
	
	unZipFileName = zipFileName.substr(0, zipFileName.length() - 3);
	unZipFileName += UNZIP_FILE_EXT;
	
	return true;
}

bool TakeInput(int argc, char **argv, CFileNamesVector &names)
{
	bool isLegalFileFound = false;
	string zipFileName;
	string unZipFileName;
	for(int i = 1 ; i < argc ; i++)
	{
		zipFileName = argv[i];
        if (zipFileName == "--stdout")
        {
            use_stdout = 1;
            continue;
        }
        
		bool res = CreateUnZipFileName(zipFileName, unZipFileName);
		if(true == res)
		{
			isLegalFileFound = true;
			names.push_back(CFileNamePair(zipFileName, unZipFileName));
		}
		else
		{
			cout << "FAILED to convert file name from zip format to unzip format: " << zipFileName << endl; 
		}
	}
	return isLegalFileFound;
}

bool DecompressSingle(CFileNamePair &fileNamePair)
{    
	ZipFileHandle = fopen(fileNamePair.m_ZipFileName.c_str(), "r");
	if(NULL == ZipFileHandle)
	{
		FatalError("FAILED to open file", fileNamePair.m_ZipFileName.c_str(), 0);
	}

    if (use_stdout == 0)
    {        
        UnZipFileHandle = fopen(fileNamePair.m_UnZipFileName.c_str(), "w");
        if(NULL == UnZipFileHandle)
        {
            FatalError("FAILED to open file", fileNamePair.m_UnZipFileName.c_str(), 0);
        }
    }
    
	
	CompressionStream.avail_in	= 0;
	CompressionStream.avail_out	= 0;
	CompressionStream.next_in	= NULL;
	CompressionStream.next_out	= NULL;
	CompressionStream.total_in	= 0;
	CompressionStream.total_out	= 0;
	CompressionStream.zalloc 	= (alloc_func)0;
	CompressionStream.zfree 	= (free_func)0;
	CompressionStream.opaque 	= (voidpf)0;
	
	int lerror = inflateInit(&CompressionStream);
	if (lerror!=Z_OK)
	{
		FatalError("Cannot init zip deflate", "", lerror);
	}
	
	do {
        int readNum = fread(ZipBuffer, 1, CHUNK, ZipFileHandle);
        if(ferror(ZipFileHandle)) 
        {
            FatalError("fread FAILED", "", 0);
        }
        
        if (0 == readNum)
        {
            break;
        }
        
        CompressionStream.avail_in = readNum;    
        CompressionStream.next_in  = ZipBuffer;

        do {
            CompressionStream.avail_out = CHUNK;
            CompressionStream.next_out 	= UnZipBuffer;

            lerror = inflate(&CompressionStream, Z_NO_FLUSH);
            switch (lerror) 
			{
				case Z_STREAM_ERROR: 
		            FatalError("inflate finished with error", "Z_STREAM_ERROR", lerror);
		            break;
		        case Z_NEED_DICT:   
		            FatalError("inflate finished with error", "Z_NEED_DICT", lerror);
		            break;
		        case Z_DATA_ERROR:
		        	FatalError("inflate finished with error", "Z_DATA_ERROR", lerror);
		        	break;
		        case Z_MEM_ERROR:
		            FatalError("inflate finished with error", "Z_MEM_ERROR", lerror);
		            break;
		    }

            int have = CHUNK - CompressionStream.avail_out;
            if (use_stdout == 0)
            {                
                const int writenNum = fwrite(UnZipBuffer, 1, have, UnZipFileHandle);            
                if (writenNum != have || ferror(UnZipFileHandle)) 
                {
                    FatalError("fwrite  FAILED", "", 0);
                }
            }
            else
            {
                write(1,UnZipBuffer,have);
            }
            

        } while (CompressionStream.avail_out == 0);
    } while (lerror != Z_STREAM_END);

    inflateEnd(&CompressionStream);
    
    fclose(ZipFileHandle);
    if (use_stdout == 0)
        fclose(UnZipFileHandle);
		
    return true;
}

bool DecompressAll(CFileNamesVector &names)
{
	bool isDecompressSuccess = false;
	CFileNamesVector::iterator iTer = names.begin();
	CFileNamesVector::iterator iEnd = names.end();
	while(iTer != iEnd)
	{
		CFileNamePair &fileNamePair = *iTer; 
		bool res = DecompressSingle(fileNamePair);
		if(true == res)
		{
			isDecompressSuccess = true;
			cout << "Decompress: " << fileNamePair.m_ZipFileName << " -> " << fileNamePair.m_UnZipFileName << endl; 
		}		
		iTer++;
	}
	
	return isDecompressSuccess;
}


int main(int argc, char* argv[])
{
	cout << endl << endl;
	
	CFileNamesVector names;
	bool res = TakeInput(argc, argv, names);
	if(false == res)
	{
		cout << "FAILED to interpret input parameters" << endl;
		cout << "Usage: [filename.log]" << endl;
		return eIllegalInput;
	}

	res = DecompressAll(names);
	
	Exit(true == res ? eOk : eDecompressFailed); 
}




