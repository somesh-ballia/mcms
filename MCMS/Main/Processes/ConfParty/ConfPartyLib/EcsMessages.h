//+========================================================================+
//                   EcsMessages.h                                         |
//		     Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       EcsMessages.h                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ron                                                         |
//-------------------------------------------------------------------------|
// Who  | Date  01-2009  | Description                                     |
//-------------------------------------------------------------------------|
//			                                                   |
//+========================================================================+

#ifndef _ECS_MESSAGES_H_
#define _ECS_MESSAGES_H_

// includes
#include "PObject.h"
#include "EncryptionKeyServerAPI.h"
#include "EncryptionDefines.h"
#include <ostream>
#include "TraceStream.h"

// classes declarations
class CSegment;

// defines
typedef struct{
  BYTE media;
  BYTE alg;
  BYTE params;
}EcsAlgS;
//===============================================================================================================
// P_Message base class for ECS message  
//===============================================================================================================
class P_Message : public CPObject
{
  CLASS_TYPE_1(P_Message, CPObject)
public:
  // constructors
  P_Message(BYTE identifier);
  virtual ~P_Message();

  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  // Serialize/DeSerialize == Decode/Encode of ASN.1 format (SERIALEMBD)
  virtual void Serialize(WORD format,CSegment& dest) = 0;
  virtual DWORD DeSerialize(WORD format,CSegment& source) = 0;
  // print
  virtual void Dump()const;

  // get
  BYTE GetIdentifier()const;
  
protected:

  // ASN.1 decoding functions
  DWORD GetDecodeASN1Length(CSegment& pParam,DWORD& decoded_bytes);
  BYTE DecodeASN1ParamContext(CSegment& pParam,DWORD& decoded_bytes,WORD param_num);
  WORD DecodeASN1NumOfUnusedBits(CSegment& pParam,DWORD& decoded_bytes);
  void DecodeAsn1BitString(CSegment& source,BYTE* pDest,DWORD length,DWORD& decoded_bytes);

  // ASN.1 encoding functions
  void EncodeLengthInAsn1(DWORD length, CSegment& dest, DWORD& encoded_bytes)const;
  void EncodeASN1ParamContext(WORD param_num,CSegment& dest,DWORD& encoded_bytes)const;
  DWORD GetNumOfBytesForEncodingLengthInAsn1(DWORD data_length)const;
  void EncodeAsn1BitString(BYTE* pSource,DWORD source_length,CSegment& dest,DWORD& encoded_bytes);

  // ASN.1 general functions
  void  DumpHex(const char* buffer_name,BYTE* data_buff,DWORD data_buffer_size,DWORD trace_level) const;
  
  // for Dump() function
  virtual void Dump(COstrStream& msg)const;
  void  DumpHex(COstrStream& msg,const char* data_buff,int data_len)const;

  // attributes
  BYTE m_identifier;

  // unused
  // void  DumpHexByte(const char* buffer_name,BYTE data_byte) const;

};
//===============================================================================================================
// Request Privacy System (P0)
// Content:(MSB)0000XDRM(LSB)  '1'/'0' supported/not ,X=ISO 8732, D=Diffie-Hellman, R=RSA, M=unspecified/manual key entry.
//===============================================================================================================
class P0_Message : public P_Message
{
  CLASS_TYPE_1(P0_Message, P_Message)
public:
  // constructors
  P0_Message();
  P0_Message(BYTE encryption_systems_bit_mask);
  virtual ~P0_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);
  
  // Public / API - specific message api
  bool IsSupportDiffieHellman()const;

private:
  BYTE m_encryption_systems_bit_mask;
};
//===============================================================================================================
// Cannot encrypt (P1) The sender of this message will not use an encryption system.
// Content: no content
//===============================================================================================================
class P1_Message : public P_Message
{
  CLASS_TYPE_1(P1_Message, P_Message)
public:
  // constructors
  P1_Message();
  virtual ~P1_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);
};
//===============================================================================================================
// Failure to start encryption system (P2) could be due to a key exchange failure, for security reasons, no cause is given.
// Content: no content
//===============================================================================================================
class P2_Message : public P_Message
{
  CLASS_TYPE_1(P2_Message, P_Message)
public:
  // constructors
  P2_Message();
  virtual ~P2_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);
};
//===============================================================================================================
// *key* exchange information (P3) first key exchange message
// Content: primitive root, prime and intermediate result
//===============================================================================================================
class P3_Message : public P_Message
{
  CLASS_TYPE_1(P3_Message, P_Message)
public:
  // constructors
  P3_Message();
  P3_Message(DWORD generator,BYTE* pPrime,BYTE* pHalfKey);
  virtual ~P3_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);

  virtual void Dump()const;

  // Public / API - specific message api
  bool IsGeneratorSupported()const;
  bool IsPrimeSupported()const;
  DWORD GetPrimeType()const;

  // for GenerateSharedSecretWithPrime
  DWORD GetGenerator()const;
  BYTE* GetPrime();
  BYTE* GetHalfKey();

protected:
  virtual void Dump(COstrStream& msg)const;

private:
  DWORD m_generator;
  BYTE m_pPrime[PRIME_NUMBER_SIZE];
  BYTE m_pHalfKey[HALF_KEY_SIZE];
};
//===============================================================================================================
// Intermediate *key* exchange information (P4) second (response) key exchange message
// Content: intermediate result
//===============================================================================================================
class P4_Message : public P_Message
{
  CLASS_TYPE_1(P4_Message, P_Message)
public:
  // constructors
  P4_Message();
  P4_Message(BYTE* pHalfKey);
  virtual ~P4_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);

  // for GenerateSharedSecretWithPrime
  BYTE* GetHalfKey();

private:
  BYTE m_pHalfKey[HALF_KEY_SIZE];
};
//===============================================================================================================
// check code information from MCU (P5).
// Content: link identifier and check code
//===============================================================================================================
class P5_Message : public P_Message
{
  CLASS_TYPE_1(P5_Message, P_Message)
public:
  // constructors
  P5_Message();
  P5_Message(BYTE* linkIdentifier,BYTE* checkCode);
  virtual ~P5_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);

private:
  BYTE m_LinkIdentifier[LINK_IDENTIFIER_LENGTH]; // sizeof(DWORD) we are using the connection id ??
  BYTE m_CheckCode[CHECK_CODE_LENGTH]; //check code - 8
};
//===============================================================================================================
// Session Key Information (P6).
// Content: initialization vector used for the encryption of the session key data, and the encrypted session key information 
//===============================================================================================================
class P6_Message : public P_Message
{
  CLASS_TYPE_1(P6_Message, P_Message)
public:
  // constructors
  P6_Message();
  P6_Message(BYTE* initialVector,BYTE* encryptedSessionKeyInfo);
  virtual ~P6_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);

  // get  
  BYTE* GetInitialVector();
  BYTE* GetEncryptedSessionKeyInfo();

private:
  BYTE m_InitialVector[INIT_VECTOR_LEN]; //initialization vector - 16 Bytes for AES128
  BYTE m_EncryptedSessionKeyInfo[SESSION_KEY_INFO_LEN]; //session key info - 64 Bytes for AES128
};
//===============================================================================================================
// decryption  algorithms-available information (P8).
// Content: list of algorithms that a terminal is capable of decrypting
//===============================================================================================================
class P8_Message : public P_Message
{
  CLASS_TYPE_1(P8_Message, P_Message)
public:
  // constructors
  P8_Message();
  P8_Message(const P8_Message& otherP8);
  P8_Message& operator=(const P8_Message& otherP8);
  virtual ~P8_Message();
  
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);

  // Public / API - specific message api
  void CreateLocal();
  bool IsSupportAES128()const;

private:
  WORD m_num_of_alg;
  EcsAlgS* m_pAlgArray;
};
//===============================================================================================================
// algorithm-in-use information (P9)
// Content: Encryption algorithm
//===============================================================================================================
class P9_Message : public P_Message
{
  CLASS_TYPE_1(P9_Message, P_Message)
public:
  // constructors
  P9_Message();
  virtual ~P9_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);

  // Public / API - specific message api
  bool IsSupportAES128()const;
  void CreateLocal();

private:
  WORD m_isSet;
  EcsAlgS m_Alg;
};
//===============================================================================================================
// Cryptographic Service Message (P11)
// Content: A primitive string of text used for ISO 8732 key managmment we should not get that message in Diffie-Hellman
//===============================================================================================================
class P11_Message : public P_Message
{
  CLASS_TYPE_1(P11_Message, P_Message)
public:
  // constructors
  P11_Message();
  virtual ~P11_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);
};
//===============================================================================================================
// Channel idle message 
// Content: no content - MUX shuold not send this message to MCMS 
//===============================================================================================================
class NULL_Message : public P_Message
{
  CLASS_TYPE_1(NULL_Message, P_Message)
public:
  // constructors
  NULL_Message();
  virtual ~NULL_Message();
  // CPObject 
  virtual const char*  NameOf() const;

  // Public / API
  virtual void Serialize(WORD format,CSegment& dest);
  virtual DWORD DeSerialize(WORD format,CSegment& source);
};
//===============================================================================================================
#endif // _ECS_MESSAGES_H_
