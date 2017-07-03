// EncodeHelper.h

#ifndef ENCODEHELPER_H_
#define ENCODEHELPER_H_

#include <string>
#include "Macros.h"

typedef enum {
    E_ENCRYPT,
    E_DECRYPT
} E_EncDec;


class EncodeHelper : CNonCopyable
{
 public:
	static inline bool is_base64(unsigned char c) {
	  return (isalnum(c) || (c == '+') || (c == '/'));
	}
	static std::string base64_encode(unsigned char const*, unsigned int len);
	static std::string base64_decode(const std::string& s);

	static std::string DecodeProprietaryPolycomString(const std::string& encoded_string);

	void EncodeAes(const std::string& plaintext, std::string &ciphertext);
	void DecodeAes(const std::string& ciphertext, std::string &plaintext);

	void EncryptPassword(const std::string& srcPass, std::string& dstPass);
	void DecryptPassword(const std::string& srcPass, std::string& dstPass);

	UINT32 AesEncryptDecrypt(FILE* ifp, FILE* ofp, E_EncDec eEncDec);

};

#endif
