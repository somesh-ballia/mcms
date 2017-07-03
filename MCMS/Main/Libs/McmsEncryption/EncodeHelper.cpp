// EncodeHelper.cpp

#include "EncodeHelper.h"
#include "StatusesGeneral.h"


#include <string.h>

// fipssyms.h should be first to take FIPS version of AES_set_encrypt_key
#include <openssl/fipssyms.h>
#include <openssl/aes.h>

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

#if !defined(FAR)
	#define FAR
#endif /* INT8 */

#if !defined(TYPE_Uint8)
	typedef unsigned char Uint8, *pUint8;
	#define TYPE_Uint8
#endif /* UINT8 */

#if !defined(TYPE_Uint32)
	typedef unsigned int Uint32, *pUint32;
	#define TYPE_Uint32
#endif /* UINT8 */

extern "C"
{
	FAR void DecryptCBCOpenSSL(const Uint8 *pucSharedKey, Uint8 *pucIVector, const Uint8 *pucInput, Uint8 *pucOutput);
}

std::string EncodeHelper::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string EncodeHelper::base64_decode(const std::string& encoded_string)
{
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

std::string EncodeHelper::DecodeProprietaryPolycomString(const std::string& encoded_string)
{
	unsigned char CAL_KEY_BYTES[] =
	{
		0x27, 0xD6, 0xB0, 0xDC,
		0x7B, 0xB1, 0x2D, 0x3E,
		0x3B, 0xDC, 0x9F, 0x45,
		0x1F, 0xC7, 0x88, 0x0E
	};

	unsigned char CAL_INITIAL_VECTOR[] =
	{
		0x1B, 0x06, 0x03, 0x8D,
		0x33, 0x05, 0x39, 0xB7,
		0x09, 0x53, 0x9A, 0xBE,
		0x1F, 0x5A, 0x10, 0xBC
	};

	std::string strFromBase64 = base64_decode(encoded_string);

	char szEncodedBytes[128];
	memset(szEncodedBytes,'\0',128);

	memcpy(szEncodedBytes,strFromBase64.c_str(),strFromBase64.length());

	char szDecodedBytes[128];
	memset(szDecodedBytes,'\0',128);

	DecryptCBCOpenSSL(CAL_KEY_BYTES,CAL_INITIAL_VECTOR,(const unsigned char*)szEncodedBytes,(unsigned char*)szDecodedBytes);

	int i=0;
	while( i < 127 && isalnum(szDecodedBytes[i]) )
		i++;
	szDecodedBytes[i] = '\0';

	std::string strRetVal = szDecodedBytes;

	return strRetVal;
}

void EncodeHelper::EncodeAes(const std::string& plaintext, std::string &ciphertext)
{
	if (plaintext.length() == 0)
		return;

	unsigned char CAL_KEY_256_BYTES[] =
	{
		0x27, 0xD6, 0xB0, 0xDC,
		0x7B, 0xB1, 0x2D, 0x3E,
		0x3B, 0xDC, 0x9F, 0x45,
		0x1F, 0xC7, 0x88, 0x0E,
		0x27, 0xD6, 0xB0, 0xDC,
		0x7B, 0xB1, 0x2D, 0x3E,
		0x3B, 0xDC, 0x9F, 0x45,
		0x1F, 0xC7, 0x88, 0x0E
	};

	unsigned char CAL_INITIAL_VECTOR_256_BYTES[] =
	{
		0x1B, 0x06, 0x03, 0x8D,
		0x33, 0x05, 0x39, 0xB7,
		0x09, 0x53, 0x9A, 0xBE,
		0x1F, 0x5A, 0x10, 0xBC,
		0x1B, 0x06, 0x03, 0x8D,
		0x33, 0x05, 0x39, 0xB7,
		0x09, 0x53, 0x9A, 0xBE,
		0x1F, 0x5A, 0x10, 0xBC
	};

	unsigned char indata[AES_BLOCK_SIZE+1];
	unsigned char outdata[AES_BLOCK_SIZE+1];

	AES_KEY key;
	AES_set_encrypt_key(CAL_KEY_256_BYTES, 256, &key);

	int num=0;
	char* pInPointer = (char*)(plaintext.c_str());
	int leftLength = plaintext.length();
	int numCharsToRead = AES_BLOCK_SIZE;

	while (1)
	{
		memset(indata, 0, AES_BLOCK_SIZE+1);
		memset(outdata, 0, AES_BLOCK_SIZE+1);

		if (leftLength >= AES_BLOCK_SIZE)
			numCharsToRead = AES_BLOCK_SIZE;
		else
			numCharsToRead = leftLength;

		memcpy(indata, pInPointer, numCharsToRead);

		AES_cfb128_encrypt(indata, outdata, numCharsToRead, &key, CAL_INITIAL_VECTOR_256_BYTES, &num, AES_ENCRYPT);

		ciphertext += (char*)outdata;

		pInPointer += numCharsToRead;
		leftLength -= numCharsToRead;

		if ((numCharsToRead < AES_BLOCK_SIZE) || (leftLength == 0))
			break;
	}
}

void EncodeHelper::DecodeAes(const std::string& ciphertext, std::string &plaintext)
{
	if (ciphertext.length() == 0)
		return;

	unsigned char CAL_KEY_256_BYTES[] =
	{
		0x27, 0xD6, 0xB0, 0xDC,
		0x7B, 0xB1, 0x2D, 0x3E,
		0x3B, 0xDC, 0x9F, 0x45,
		0x1F, 0xC7, 0x88, 0x0E,
		0x27, 0xD6, 0xB0, 0xDC,
		0x7B, 0xB1, 0x2D, 0x3E,
		0x3B, 0xDC, 0x9F, 0x45,
		0x1F, 0xC7, 0x88, 0x0E
	};

	unsigned char CAL_INITIAL_VECTOR_256_BYTES[] =
	{
		0x1B, 0x06, 0x03, 0x8D,
		0x33, 0x05, 0x39, 0xB7,
		0x09, 0x53, 0x9A, 0xBE,
		0x1F, 0x5A, 0x10, 0xBC,
		0x1B, 0x06, 0x03, 0x8D,
		0x33, 0x05, 0x39, 0xB7,
		0x09, 0x53, 0x9A, 0xBE,
		0x1F, 0x5A, 0x10, 0xBC
	};

	unsigned char indata[AES_BLOCK_SIZE+1];
	unsigned char outdata[AES_BLOCK_SIZE+1];

	AES_KEY key;
	AES_set_encrypt_key(CAL_KEY_256_BYTES, 256, &key);

	int num=0;
	char* pInPointer = (char*)(ciphertext.c_str());
	int leftLength = ciphertext.length();
	int numCharsToRead = AES_BLOCK_SIZE;

	while (1)
	{
		memset(indata, 0, AES_BLOCK_SIZE+1);
		memset(outdata, 0, AES_BLOCK_SIZE+1);

		if (leftLength >= AES_BLOCK_SIZE)
			numCharsToRead = AES_BLOCK_SIZE;
		else
			numCharsToRead = leftLength;

		memcpy(indata, pInPointer, numCharsToRead);

		AES_cfb128_encrypt(indata, outdata, numCharsToRead, &key, CAL_INITIAL_VECTOR_256_BYTES, &num, AES_DECRYPT);

		plaintext += (char*)outdata;

		pInPointer += numCharsToRead;
		leftLength -= numCharsToRead;

		if ((numCharsToRead < AES_BLOCK_SIZE) || (leftLength == 0))
			break;
	}
}

void EncodeHelper::EncryptPassword(const std::string& srcPass, std::string& dstPass)
{
  std::string sEnc;
  std::string base64tempEnc;


  EncodeHelper eH;
  eH.EncodeAes(srcPass, sEnc);
  base64tempEnc = eH.base64_encode((unsigned char*)sEnc.c_str(), strlen(sEnc.c_str()));

  dstPass = base64tempEnc;
}

void EncodeHelper::DecryptPassword(const std::string& srcPass, std::string& dstPass)
{

	std::string base64tempEnc;
	std::string base64tempDec;

	EncodeHelper eH;


	if (srcPass.length() == 0)
		return;

	base64tempEnc = srcPass.c_str();
	base64tempDec = eH.base64_decode(base64tempEnc);
	eH.DecodeAes(base64tempDec, dstPass);
}

//NOTE - this function takes 2 OPENED files params and it does NOT close them!!!
//its the responsibility of the caller to maintain the files (open/close them)!!!
STATUS EncodeHelper::AesEncryptDecrypt(FILE* ifp, FILE* ofp, E_EncDec eEncDec)
{
#define AES_KEY_NUM_OF_BITS (256)
    INT32 bytes_read, bytes_written;
    UINT8 indata[AES_BLOCK_SIZE];
    UINT8 outdata[AES_BLOCK_SIZE];

    unsigned char CAL_KEY_256_BYTES[] =
    {
        0x27, 0xD6, 0xB0, 0xDC,
        0x7B, 0xB1, 0x2D, 0x3E,
        0x3B, 0xDC, 0x9F, 0x45,
        0x1F, 0xC7, 0x88, 0x0E,
        0x27, 0xD6, 0xB0, 0xDC,
        0x7B, 0xB1, 0x2D, 0x3E,
        0x3B, 0xDC, 0x9F, 0x45,
        0x1F, 0xC7, 0x88, 0x0E
    };

    unsigned char CAL_INITIAL_VECTOR_256_BYTES[] =
    {
        0x1B, 0x06, 0x03, 0x8D,
        0x33, 0x05, 0x39, 0xB7,
        0x09, 0x53, 0x9A, 0xBE,
        0x1F, 0x5A, 0x10, 0xBC,
        0x1B, 0x06, 0x03, 0x8D,
        0x33, 0x05, 0x39, 0xB7,
        0x09, 0x53, 0x9A, 0xBE,
        0x1F, 0x5A, 0x10, 0xBC
    };
    /* data structure that contains the key itself */
    AES_KEY key;
    /* set where on the 128 bit encrypted block to begin encryption*/
    INT32 num = 0;

    //MfaBoardPrint(IP_MEDIA_PRINT,CRITICAL_TRACE,"(%s %s %d) START eEncDec = %d\n",__FILE__,__FUNCTION__, __LINE__, eEncDec);

    //sanity on params
    if(!ifp || !ofp || ((eEncDec != E_ENCRYPT) && (eEncDec != E_DECRYPT))) {
        //MfaBoardPrint(IP_MEDIA_PRINT,CRITICAL_TRACE,"(%s %s %d) EmbError - bad params\n",__FILE__,__FUNCTION__, __LINE__);
        return STATUS_FAIL;
    }

    /* set the encryption key */
    AES_set_encrypt_key(CAL_KEY_256_BYTES, AES_KEY_NUM_OF_BITS, &key);

    while(1) {
        bytes_read = fread(indata, 1, AES_BLOCK_SIZE, ifp);

        if(eEncDec == E_ENCRYPT) {
            AES_cfb128_encrypt(indata, outdata, bytes_read, &key, CAL_INITIAL_VECTOR_256_BYTES, &num, AES_ENCRYPT);
        }
        else if(eEncDec == E_DECRYPT) {
            AES_cfb128_encrypt(indata, outdata, bytes_read, &key, CAL_INITIAL_VECTOR_256_BYTES, &num, AES_DECRYPT);
        }



        bytes_written = fwrite(outdata, 1, bytes_read, ofp);
        if(bytes_read < AES_BLOCK_SIZE) {
            break;
        }
    }
   // MfaBoardPrint(IP_MEDIA_PRINT,CRITICAL_TRACE,"(%s %s %d) END GOOD\n",__FILE__,__FUNCTION__, __LINE__);
    return STATUS_OK;
}




