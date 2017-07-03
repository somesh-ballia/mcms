#include <iostream>
#include <fstream>
//#include <openssl/blowfish.h>

using namespace std;

#define BUFFER_SIZE  8*512
#define ENCRYPT_SIZE 8//768
#define FILE_NAME   "sagi.tar"
#define ENCRYPT_FILE "out.txt"

//#define BF_LONG unsigned int

/* typedef struct bf_key_st
        {
           BF_LONG P[BF_ROUNDS+2];
           BF_LONG S[4*256];
        } BF_KEY;
*/

class BlowFish
{
    public:
	//key encryption init
	static void InitKey(/*BF_KEY &key*/);

	// ecb encryption/decryption, for more details: "man blowfish"
	static bool Encrypt(const char * srcFile, const char * destFile);
	static bool Decrypt(const char * srcFile, const char * destFile);

	/* Copy "size" chars of "from", starting at "from[fromOffset]"
	   to buffer "to", starting at "to[toOffset]" */
	static bool CopyFromBuffer(unsigned char * from, int fromOffset, 
			 	   			   unsigned char * to,   int toOffset, 
			 	   			   int size = ENCRYPT_SIZE);
};
