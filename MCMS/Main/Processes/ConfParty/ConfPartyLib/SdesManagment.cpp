// SdesManagment.cpp

#include "SdesManagment.h"
#include "SslFunc.h"

extern "C"
{
  //Api for SIP - generating and decryption of keys
  INT32 CreateRandom128BitsOpenSSL(UINT8 *pucRandKey);
  INT32 CreateRandom112BitsOpenSSL(UINT8 *pucRandKey);
}

void CreateBase64MasterSaltKey(char** pBase64MasterSaltKey)
{
	UINT8 masterKey[sizeOf128Key + SIZE_OF_112_SDES_KEY + 1];

	CreateRandom128BitsOpenSSL(masterKey);
	CreateRandom112BitsOpenSSL(masterKey + sizeOf128Key);

	std::string str = CSslFunctions::ToBase64(masterKey, SIZE_OF_CONC_SDES_KEY);
	strncpy(*pBase64MasterSaltKey, str.c_str(), str.length());
}

BOOL IsMasterSaltKeyValid(char *pBase64MasterSaltKey, int *pDecodedLen, int *pEncodedLen)
{
	BOOL bIsMasterSaltKeyValid = TRUE;
	int decodedStrLen = 0;
	int encodedStrLen = strlen(pBase64MasterSaltKey);

	std::string strDecoded = CSslFunctions::FromBase64((const unsigned char*)pBase64MasterSaltKey, encodedStrLen);
	decodedStrLen = strlen(strDecoded.c_str());
	if(decodedStrLen != SIZE_OF_CONC_SDES_KEY)
		bIsMasterSaltKeyValid = FALSE;
	*pDecodedLen = decodedStrLen;
	*pEncodedLen = encodedStrLen;

	return bIsMasterSaltKeyValid;
}
