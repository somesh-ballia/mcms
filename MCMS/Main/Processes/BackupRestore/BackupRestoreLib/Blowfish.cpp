#include <memory.h>
#include "Blowfish.h"


// Initialization of key, for blowfish de/encryption
void BlowFish::InitKey(/*BF_KEY &key*/)
{
    char * bufForKey = "-zman_i_s_money-";
    //BF_set_key(&key, strlen(bufForKey), (unsigned char*)bufForKey);
}

////////////////////////////////////////////////////////////////////////////////
bool BlowFish::CopyFromBuffer(unsigned char * from, int fromOffset, unsigned char * to, int toOffset, int size)
{
    if(0 > toOffset || fromOffset < 0 || size < 0 || !to || !from)
    {
        cout << "Error: one of the arguments are missing or corrupted!" << endl;
        return false;
    }

    int j = toOffset;
    int max = fromOffset+size;
    for(int i = fromOffset; i < max; ++i)
    {
    	to[j] = from[i]; 
    	++j;
    }	
 
    //to[j] = '\0';

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool BlowFish::Encrypt(const char * srcFile, const char * destFile)
{
    //BF_KEY key;
    //InitKey(key);

    //ofstream ofs;

    FILE* ifs = fopen(srcFile, "r");
    if (NULL == ifs)
    {
        cout << "BlowFish::Encrypt() Failed to open file " << srcFile << endl; 
        return false;
    }

    //ofs.open(destFile);
    FILE* ofs = fopen(destFile, "r+");
    if (NULL == ofs)
    {
        cout << "BlowFish::Encrypt() Failed to open file " << destFile << endl; 
        return false;
    }
    
    unsigned char buf[BUFFER_SIZE],
               outBuf[BUFFER_SIZE],
              tmpBuf[ENCRYPT_SIZE],
              encBuf[ENCRYPT_SIZE];

    int bytesRead = 0, 
		aggLen = 0, 
		gap = 0;

    memset(buf, '\0', sizeof(buf));
    while( (bytesRead = fread(buf, sizeof(char), sizeof(buf), ifs)) )
    {
        memset(outBuf, '\0', sizeof(outBuf));

		aggLen = 0;	
	
		while(aggLen < bytesRead)
		{
			memset(encBuf, '\0',ENCRYPT_SIZE);
	
			gap = bytesRead - aggLen;	
	        if(gap < ENCRYPT_SIZE)
	    	{
				CopyFromBuffer(buf, aggLen, outBuf, aggLen, gap);
				aggLen += gap;
				continue;
	    	}
			CopyFromBuffer(buf, aggLen, tmpBuf, 0);
			
	    	//BF_ecb_encrypt(tmpBuf, encBuf, &key, BF_ENCRYPT);
			
			CopyFromBuffer(encBuf, 0, outBuf, aggLen);
	
			aggLen += ENCRYPT_SIZE;
		}

        //ofs.write((char*)outBuf, aggLen);
		fwrite(outBuf, sizeof(char), sizeof(outBuf), ofs);
		
        memset(buf, '\0', sizeof(buf));
    }

    fclose(ifs);    
    //ofs.close();
    fclose(ofs);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
bool BlowFish::Decrypt(const char * srcFile, const char * destFile)
{
    //BF_KEY key;
    //InitKey(key);

    FILE* is =  fopen(srcFile, "r");
    if (NULL == is)
    {
        cout << "Failed to open file " << srcFile << ",  for reading" << endl; 
        return false;
    }

    ofstream os;
    os.open(destFile);

    unsigned char buf[BUFFER_SIZE],
               outBuf[BUFFER_SIZE],
		      tmpBuf[ENCRYPT_SIZE],
		      encBuf[ENCRYPT_SIZE];
    
    int bufSize = sizeof(buf),
	 	bytesRead = 0,
		aggLen = 0,
		gap = 0;

    memset(buf, '\0', bufSize);
    while( (bytesRead = fread(buf, sizeof(char), bufSize, is)) )
    {
        memset(outBuf, '\0', sizeof(outBuf));

		aggLen = 0;
	
		while(aggLen < bytesRead)
		{
			memset(encBuf, '\0',ENCRYPT_SIZE);
			gap = bytesRead - aggLen;	
	        if(gap < ENCRYPT_SIZE)
        	{
				CopyFromBuffer(buf, aggLen, outBuf, aggLen, gap);
				aggLen += gap;
				continue;
        	}
	
			CopyFromBuffer(buf, aggLen, tmpBuf, 0);
	
	        //BF_ecb_encrypt(tmpBuf, encBuf, &key, BF_DECRYPT);
	
			CopyFromBuffer(encBuf, 0, outBuf, aggLen);
		
			aggLen += ENCRYPT_SIZE;
		}

        os.write((char*)outBuf, aggLen);
	
        memset(buf, '\0', sizeof(buf));
    }

    fclose(is);
    os.close();
  
    return true;
}


int test(int argc, char * argv[])
{
	char * usage = "Usage: ./Blowfish Encrypt/Decrypt source_file encrypted_file\n";
	
    if(argc < 4)
    {
    	cerr << usage;
    	return -1;
    }
    
    /*if(0 == (strcmp(argv[1],"Encrypt")) )
    {
    	BlowFish::Encrypt(argv[2], argv[3]);
    }
    else if(0 == (strcmp(argv[1],"Decrypt")) )
    {
    	BlowFish::Decrypt(argv[2], argv[3]);
    }
    else
    {
    	cerr << usage;
    }*/
    
    return 0;
}
