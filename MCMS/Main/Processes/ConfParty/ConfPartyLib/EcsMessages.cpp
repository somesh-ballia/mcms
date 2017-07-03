//+========================================================================+
//                   EcsMessages.cpp                                       |
//		     Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       EcsMessages.cpp                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ron                                                         |
//-------------------------------------------------------------------------|
// Who  | Date  01-2008  | Description                                     |
//-------------------------------------------------------------------------|
//			                                                   |
//+========================================================================+

// includes
#include "EcsMessages.h"
#include "EncryptionDefines.h"
#include "H221.h"
#include "Segment.h"
#include "StatusesGeneral.h"
#include "EncryptionKey.h"

#include <iomanip>
using namespace std;

BYTE TagValue[MAX_ASN1_MSG_PARAMS] = { 0, 1, 2, 3, 4 };
BYTE Significant_Bits[8] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 };

//===============================================================================================================
// P0_Message
//===============================================================================================================
P0_Message::P0_Message() :
		P_Message(P0_Identifier), m_encryption_systems_bit_mask(0)
{
}
//===============================================================================================================
P0_Message::P0_Message(BYTE encryption_systems_bit_mask) :
		P_Message(P0_Identifier), m_encryption_systems_bit_mask(encryption_systems_bit_mask)
{
}
//===============================================================================================================
P0_Message::~P0_Message()
{
	// removing keys stamp from memory
	m_encryption_systems_bit_mask = 0;
}
//===============================================================================================================
const char* P0_Message::NameOf() const
{
	return "P0_Message";
}
//===============================================================================================================
void P0_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
			// for embedded - full ASN.1 format
			dest << (BYTE)P0_Identifier;
			dest << (BYTE)P0_CONTENT_LENGTH; // 1
			dest << (BYTE)m_encryption_systems_bit_mask;
			break;
		case NATIVE :
		default:
			// native - class data
			dest << (BYTE)m_encryption_systems_bit_mask;
			break;
	}
}
//===============================================================================================================
DWORD P0_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			BYTE data_len = 0;
			source >> data_len;
			if (data_len != (BYTE)P0_CONTENT_LENGTH)
			{
				PTRACE2INT(eLevelInfoNormal, "P0_Message::DeSerialize wrong data len =  ", (DWORD )data_len);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			source >> m_encryption_systems_bit_mask;
			break;
		}
		case NATIVE :
		default:
		{
			source >> m_encryption_systems_bit_mask;
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================
bool P0_Message::IsSupportDiffieHellman() const
{
	bool ret_val = false;
	if ((m_encryption_systems_bit_mask & DIFFIE_HELMAN_MASK)!= 0){
		ret_val = true;
	}
	return ret_val;
}
//===============================================================================================================
// P8_Message
//===============================================================================================================
P8_Message::P8_Message() :
		P_Message(P8_Identifier), m_num_of_alg(0), m_pAlgArray(NULL)
{
}
//===============================================================================================================
P8_Message::~P8_Message()
{
	// removing keys stamp from memory
	m_num_of_alg = 0;
	delete[] m_pAlgArray;
}
//===============================================================================================================
const char* P8_Message::NameOf() const
{
	return "P8_Message";
}
//===============================================================================================================
void P8_Message::CreateLocal()
{
	m_num_of_alg = 1;
	m_pAlgArray = new EcsAlgS[m_num_of_alg];

	m_pAlgArray[0].media = AES_MEDIA;
	m_pAlgArray[0].alg = AES_128_IDENTIFIER;
	m_pAlgArray[0].params = AES_PARAM;
}
//===============================================================================================================
P8_Message::P8_Message(const P8_Message& otherP8) :
		P_Message(P8_Identifier)
{
	m_num_of_alg = otherP8.m_num_of_alg;
	m_pAlgArray = new EcsAlgS[m_num_of_alg];
	for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
	{
		m_pAlgArray[alg_index].media = otherP8.m_pAlgArray[alg_index].media;
		m_pAlgArray[alg_index].alg = otherP8.m_pAlgArray[alg_index].alg;
		m_pAlgArray[alg_index].params = otherP8.m_pAlgArray[alg_index].params;
	}
}
//===============================================================================================================
P8_Message& P8_Message::operator=(const P8_Message& otherP8)
{
	if (this == &otherP8)
	{
		return *this;
	}
	m_num_of_alg = otherP8.m_num_of_alg;
	delete[] m_pAlgArray;
	m_pAlgArray = new EcsAlgS[m_num_of_alg];
	for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
	{
		m_pAlgArray[alg_index].media = otherP8.m_pAlgArray[alg_index].media;
		m_pAlgArray[alg_index].alg = otherP8.m_pAlgArray[alg_index].alg;
		m_pAlgArray[alg_index].params = otherP8.m_pAlgArray[alg_index].params;
	}
	return *this;
}
//===============================================================================================================
void P8_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			dest << (BYTE)P8_Identifier;
			dest << (BYTE)(ENCRYPTION_ALG_LENGTH_IN_BYTES * m_num_of_alg); // 3*m_num_of_alg
			for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
			{
				dest << m_pAlgArray[alg_index].media;
				dest << m_pAlgArray[alg_index].alg;
				dest << m_pAlgArray[alg_index].params;
			}
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			dest << m_num_of_alg;
			for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
			{
				dest << m_pAlgArray[alg_index].media;
				dest << m_pAlgArray[alg_index].alg;
				dest << m_pAlgArray[alg_index].params;
			}
			break;
		}
	}
}
//===============================================================================================================
DWORD P8_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			BYTE data_len = 0;
			source >> data_len;
			m_num_of_alg = data_len / ENCRYPTION_ALG_LENGTH_IN_BYTES;
			if (m_num_of_alg == 0 || m_num_of_alg > 16)
			{
				PTRACE2INT(eLevelInfoNormal, "P8_Message::DeSerialize wrong data len =  ", (DWORD )data_len);
				m_num_of_alg = 0;
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			m_pAlgArray = new EcsAlgS[m_num_of_alg];
			for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
			{
				source >> m_pAlgArray[alg_index].media;
				source >> m_pAlgArray[alg_index].alg;
				source >> m_pAlgArray[alg_index].params;
			}
			break;
		}
		case NATIVE :
		default:
		{
			source >> m_num_of_alg;
			if (m_num_of_alg > 0)
			{
				m_pAlgArray = new EcsAlgS[m_num_of_alg];
				for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
				{
					source >> m_pAlgArray[alg_index].media;
					source >> m_pAlgArray[alg_index].alg;
					source >> m_pAlgArray[alg_index].params;
				}
			}
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================
bool P8_Message::IsSupportAES128() const
{
	bool ret_val = false;
	for (WORD alg_index = 0; alg_index < m_num_of_alg; alg_index++)
	{
		if (m_pAlgArray[alg_index].alg == AES_128_IDENTIFIER)
		{
			ret_val = true;
			break;
		}
	}
	return ret_val;
}
//===============================================================================================================
// P9_Message
//===============================================================================================================
P9_Message::P9_Message() :
		P_Message(P9_Identifier), m_isSet(0)
{
	m_Alg.media = 0;
	m_Alg.alg = 0;
	m_Alg.params = 0;
}
//===============================================================================================================
P9_Message::~P9_Message()
{
	// removing keys stamp from memory
	m_Alg.media = 0;
	m_Alg.alg = 0;
	m_Alg.params = 0;
}
//===============================================================================================================
const char* P9_Message::NameOf() const
{
	return "P9_Message";
}

//===============================================================================================================
void P9_Message::CreateLocal()
{
	m_isSet = 1;
	m_Alg.media = AES_MEDIA;
	m_Alg.alg = AES_128_IDENTIFIER;
	m_Alg.params = AES_PARAM;
}
//===============================================================================================================
void P9_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
			// for embedded - full ASN.1 format
			dest << (BYTE)P9_Identifier;
			dest << (BYTE)ENCRYPTION_ALG_LENGTH_IN_BYTES; // 3
			dest << m_Alg.media;
			dest << m_Alg.alg;
			dest << m_Alg.params;
			break;
		case NATIVE :
		default:
			// native - class data
			dest << m_isSet;
			dest << m_Alg.media;
			dest << m_Alg.alg;
			dest << m_Alg.params;
			break;
	}
}
//===============================================================================================================
DWORD P9_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			BYTE data_len = 0;
			source >> data_len;
			if (data_len != ENCRYPTION_ALG_LENGTH_IN_BYTES)
			{
				PTRACE2INT(eLevelInfoNormal, "P9_Message::DeSerialize wrong data len =  ", (DWORD )data_len);
				m_isSet = 0;
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			m_isSet = 1;
			source >> m_Alg.media;
			source >> m_Alg.alg;
			source >> m_Alg.params;
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			source >> m_isSet;
			source >> m_Alg.media;
			source >> m_Alg.alg;
			source >> m_Alg.params;
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================
bool P9_Message::IsSupportAES128() const
{
	bool ret_val = false;
	if (m_Alg.alg == AES_128_IDENTIFIER)
	{
		ret_val = true;
	}
	return ret_val;
}
//===============================================================================================================

//===============================================================================================================
// P3_Message
//===============================================================================================================
P3_Message::P3_Message() :
		P_Message(P3_Identifier), m_generator(0)
{
}
//===============================================================================================================
P3_Message::P3_Message(DWORD generator, BYTE* pPrime, BYTE* pHalfKey) :
		P_Message(P3_Identifier), m_generator(generator)
{
	if (pPrime != NULL)
	{
		memcpy(m_pPrime, pPrime, PRIME_NUMBER_SIZE);
	}
	if (pHalfKey != NULL)
	{
		memcpy(m_pHalfKey, pHalfKey, HALF_KEY_SIZE);
	}
}
//===============================================================================================================
P3_Message::~P3_Message()
{
	// removing keys stamp from memory
	if (m_pPrime != NULL)
	{
		memset(m_pPrime, '\0', PRIME_NUMBER_SIZE);
	}
	if (m_pHalfKey != NULL)
	{
		memset(m_pHalfKey, '\0', HALF_KEY_SIZE);
	}
}
//===============================================================================================================
const char* P3_Message::NameOf() const
{
	return "P3_Message";
}
//===============================================================================================================
void P3_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			// encode identifier
			dest << (BYTE)P3_Identifier;

			// encode total content length
			DWORD p3_total_content_len = (DWORD)P3_ASN1_LEN;
			DWORD encoded_bytes = 0;
			EncodeLengthInAsn1(p3_total_content_len, dest, encoded_bytes);

			// encode generator
			EncodeASN1ParamContext(0, dest, encoded_bytes);
			DWORD generator_bytes = 1; // we use 2/3/5 => 1 byte
			BYTE* generator_buff = new BYTE[generator_bytes];
			generator_buff[0] = (BYTE)POLYCOM_DH_GENERATOR;
			EncodeAsn1BitString(generator_buff, generator_bytes, dest, encoded_bytes);

			// encode prime
			EncodeASN1ParamContext(1, dest, encoded_bytes);
			EncodeAsn1BitString(m_pPrime, PRIME_NUMBER_SIZE, dest, encoded_bytes);

			// encode half key
			EncodeASN1ParamContext(2, dest, encoded_bytes);
			EncodeAsn1BitString(m_pHalfKey, HALF_KEY_SIZE, dest, encoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P3_Message::Serialize encoded_bytes =  ", (DWORD )encoded_bytes);
			delete[] generator_buff;
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			dest << m_generator;
			dest.Put(m_pPrime, PRIME_NUMBER_SIZE);
			dest.Put(m_pHalfKey, HALF_KEY_SIZE);
			break;
		}
	}
}
//===============================================================================================================
DWORD P3_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			DWORD decoded_bytes = 0;
			DWORD total_content_length = GetDecodeASN1Length(source, decoded_bytes);
			PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize total_content_length =  ", (DWORD )total_content_length);

			// decode generator
			BYTE param_context = DecodeASN1ParamContext(source, decoded_bytes, 0);
			DWORD generator_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (generator_length > sizeof(DWORD))
			{
				PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize wrong generator_length =  ", (DWORD )generator_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize generator_length =  ", (DWORD )generator_length);
			BYTE* generator_buffer = new BYTE[generator_length];
			DecodeAsn1BitString(source, generator_buffer, generator_length, decoded_bytes);
			BYTE generator_str[sizeof(DWORD)] = { 0, 0, 0, 0 };
			for (WORD gen_index = 0; gen_index < generator_length && gen_index < sizeof(DWORD); gen_index++)
			{
				generator_str[gen_index] = generator_buffer[gen_index];
			}
			m_generator = (DWORD)(*generator_str);
			delete[] generator_buffer;
			PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize m_generator =  ", (DWORD )m_generator);

			// decode prime
			param_context = DecodeASN1ParamContext(source, decoded_bytes, 1);
			WORD prime_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (prime_length > PRIME_NUMBER_SIZE)
			{
				PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize wrong prime_length =  ", (DWORD )prime_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize prime_length =  ", (DWORD )prime_length);
			DecodeAsn1BitString(source, m_pPrime, prime_length, decoded_bytes);

			// decode half key
			param_context = DecodeASN1ParamContext(source, decoded_bytes, 2);
			WORD half_key_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (half_key_length > HALF_KEY_SIZE)
			{
				PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize wrong half_key_length =  ", (DWORD )prime_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize half_key_length =  ", (DWORD )half_key_length);
			DecodeAsn1BitString(source, m_pHalfKey, half_key_length, decoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P3_Message::DeSerialize decoded_bytes =  ", (DWORD )decoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			source >> m_generator;
			source.Get(m_pPrime, PRIME_NUMBER_SIZE);
			source.Get(m_pHalfKey, HALF_KEY_SIZE);
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================
bool P3_Message::IsGeneratorSupported() const
{
	bool ret_val = false;
	if ((m_generator == POLYCOM_DH_GENERATOR) || (m_generator == TANBERG_H320_DH_GENERATOR))
	{
		ret_val = true;
	}
	else if (m_generator == TANBERG_H323_DH_GENERATOR)
	{
		PTRACE(eLevelInfoNormal, "P3_Message::IsGeneratorSupported - receive P3 with TANBERG_H323_DH_GENERATOR");
		ret_val = true;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "P3_Message::IsGeneratorSupported - receive P3 with unsupported generator = ", m_generator);
	}
	return ret_val;
}
//===============================================================================================================
bool P3_Message::IsPrimeSupported() const
{
	if (m_generator != GetPrimeType())
	{
		return false;
	}
	return true;
}
//===============================================================================================================
DWORD P3_Message::GetPrimeType() const
{
	DWORD ret_val = 0;
	WORD polycomBytes = 0;
	WORD tandbergH320Bytes = 0;
	for (WORD byte_index = 0; byte_index < PRIME_NUMBER_SIZE; byte_index++)
	{
		if (m_pPrime[byte_index] == polycomPrime[byte_index])
		{
			polycomBytes++;
		}
		if (m_pPrime[byte_index] == tandbergH320Prime[byte_index])
		{
			tandbergH320Bytes++;
		}
	}
	if (polycomBytes == PRIME_NUMBER_SIZE)
	{
		ret_val = POLYCOM_DH_GENERATOR;
	}
	else if (tandbergH320Bytes == PRIME_NUMBER_SIZE)
	{
		ret_val = TANBERG_H320_DH_GENERATOR;
	}
	return ret_val;
}
//===============================================================================================================
DWORD P3_Message::GetGenerator() const
{
	return m_generator;
}
//===============================================================================================================
BYTE* P3_Message::GetPrime()
{
	return m_pPrime;
}
//===============================================================================================================
BYTE* P3_Message::GetHalfKey()
{
	return m_pHalfKey;
}
//===============================================================================================================
void P3_Message::Dump(COstrStream& msg) const
{
	if (PRINT_THE_KEYS)
	{
		msg << "m_generator = " << m_generator << "\n";
		msg << "m_pPrime    = ";
		DumpHex(msg, (const char*)m_pPrime, PRIME_NUMBER_SIZE);
		msg << "\n";
		msg << "m_pHalfKey    = ";
		DumpHex(msg, (const char*)m_pHalfKey, HALF_KEY_SIZE);
		msg << "\n";
	}
	else
	{
		msg << DONT_PRINT_THE_KEYS;
	}
}
//===============================================================================================================
void P3_Message::Dump() const
{
	COstrStream msg;
	P_Message::Dump(msg);
	Dump(msg);
	PTRACE2(eLevelInfoNormal, "P3_Message::Dump:\n", (char* )msg.str().c_str());
}
//===============================================================================================================

//===============================================================================================================
// P4_Message
//===============================================================================================================
P4_Message::P4_Message() :
		P_Message(P4_Identifier)
{
}
//===============================================================================================================
P4_Message::P4_Message(BYTE* pHalfKey) :
		P_Message(P4_Identifier)
{
	if (pHalfKey != NULL)
	{
		memcpy(m_pHalfKey, pHalfKey, HALF_KEY_SIZE);
	}
}
//===============================================================================================================
P4_Message::~P4_Message()
{
	// removing keys stamp from memory
	if (m_pHalfKey != NULL)
	{
		memset(m_pHalfKey, '\0', HALF_KEY_SIZE);
	}
}
//===============================================================================================================
const char* P4_Message::NameOf() const
{
	return "P4_Message";
}
//===============================================================================================================
void P4_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			// encode identifier
			dest << (BYTE)P4_Identifier;

			// encode total content length
			// DWORD p4_total_content_len = (DWORD)P4_ASN1_LEN;
			DWORD encoded_bytes = 0;
			//      EncodeLengthInAsn1(p4_total_content_len,dest,encoded_bytes);

			// encode half key
			EncodeAsn1BitString(m_pHalfKey, HALF_KEY_SIZE, dest, encoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P4_Message::Serialize encoded_bytes =  ", (DWORD )encoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			dest.Put(m_pHalfKey, HALF_KEY_SIZE);
			break;
		}
	}
}
//===============================================================================================================
DWORD P4_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			DWORD decoded_bytes = 0;
			DWORD total_content_length = GetDecodeASN1Length(source, decoded_bytes);
			PTRACE2INT(eLevelInfoNormal, "P4_Message::DeSerialize total_content_length =  ", (DWORD )total_content_length);

			// decode half key
			WORD half_key_length = total_content_length - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (half_key_length > HALF_KEY_SIZE)
			{
				PTRACE2INT(eLevelInfoNormal, "P4_Message::DeSerialize wrong half_key_length =  ", (DWORD )half_key_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P4_Message::DeSerialize half_key_length =  ", (DWORD )half_key_length);
			DecodeAsn1BitString(source, m_pHalfKey, half_key_length, decoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P4_Message::DeSerialize decoded_bytes =  ", (DWORD )decoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			source.Get(m_pHalfKey, HALF_KEY_SIZE);
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================
BYTE* P4_Message::GetHalfKey()
{
	return m_pHalfKey;
}
//===============================================================================================================

//===============================================================================================================
// P5_Message
//===============================================================================================================
P5_Message::P5_Message() :
		P_Message(P5_Identifier)
{
}
//===============================================================================================================
P5_Message::P5_Message(BYTE* linkIdentifier, BYTE* checkCode) :
		P_Message(P5_Identifier)
{
	if (linkIdentifier != NULL)
	{
		memcpy(m_LinkIdentifier, linkIdentifier, LINK_IDENTIFIER_LENGTH);
	}
	if (checkCode != NULL)
	{
		memcpy(m_CheckCode, checkCode, CHECK_CODE_LENGTH);
	}
}
//===============================================================================================================
P5_Message::~P5_Message()
{
	// removing keys stamp from memory
	if (m_LinkIdentifier != NULL)
	{
		memset(m_LinkIdentifier, '\0', LINK_IDENTIFIER_LENGTH);
	}
	if (m_CheckCode != NULL)
	{
		memset(m_CheckCode, '\0', CHECK_CODE_LENGTH);
	}
}
//===============================================================================================================
const char* P5_Message::NameOf() const
{
	return "P5_Message";
}
//===============================================================================================================
void P5_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			// encode identifier
			dest << (BYTE)P5_Identifier;

			// encode total content length
			DWORD p5_total_content_len = (DWORD)P5_ASN1_LEN;
			DWORD encoded_bytes = 0;
			EncodeLengthInAsn1(p5_total_content_len, dest, encoded_bytes);

			// encode m_LinkIdentifier
			EncodeASN1ParamContext(0, dest, encoded_bytes);
			EncodeAsn1BitString(m_LinkIdentifier, LINK_IDENTIFIER_LENGTH, dest,encoded_bytes);

			// encode m_CheckCode
			EncodeASN1ParamContext(1, dest, encoded_bytes);
			EncodeAsn1BitString(m_CheckCode, CHECK_CODE_LENGTH, dest, encoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P5_Message::Serialize encoded_bytes =  ", (DWORD )encoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			dest.Put(m_LinkIdentifier, LINK_IDENTIFIER_LENGTH);
			dest.Put(m_CheckCode, CHECK_CODE_LENGTH);
			break;
		}
	}
}
//===============================================================================================================
DWORD P5_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			DWORD decoded_bytes = 0;
			DWORD total_content_length = GetDecodeASN1Length(source, decoded_bytes);
			PTRACE2INT(eLevelInfoNormal, "P5_Message::DeSerialize total_content_length =  ", (DWORD )total_content_length);

			// decode link identifier
			BYTE param_context = DecodeASN1ParamContext(source, decoded_bytes, 0);
			WORD link_identifier_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (link_identifier_length > LINK_IDENTIFIER_LENGTH)
			{
				PTRACE2INT(eLevelInfoNormal, "P5_Message::DeSerialize wrong link_identifier_length =  ", (DWORD )link_identifier_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;

			}
			PTRACE2INT(eLevelInfoNormal, "P5_Message::DeSerialize link_identifier_length =  ", (DWORD )link_identifier_length);
			DecodeAsn1BitString(source, m_LinkIdentifier, link_identifier_length, decoded_bytes);

			// decode check code
			param_context = DecodeASN1ParamContext(source, decoded_bytes, 1);
			WORD check_code_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (check_code_length > CHECK_CODE_LENGTH)
			{
				PTRACE2INT(eLevelInfoNormal, "P5_Message::DeSerialize wrong check_code_length =  ", (DWORD )check_code_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P5_Message::DeSerialize check_code_length =  ", (DWORD )check_code_length);
			DecodeAsn1BitString(source, m_CheckCode, check_code_length, decoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P5_Message::DeSerialize decoded_bytes =  ", (DWORD )decoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			source.Get(m_LinkIdentifier, LINK_IDENTIFIER_LENGTH);
			source.Get(m_CheckCode, CHECK_CODE_LENGTH);
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================

//===============================================================================================================
// P6_Message
//===============================================================================================================
P6_Message::P6_Message() :
		P_Message(P6_Identifier)
{
}
//===============================================================================================================
P6_Message::P6_Message(BYTE* initialVector, BYTE* encryptedSessionKeyInfo) :
		P_Message(P6_Identifier)
{
	if (initialVector != NULL)
	{
		memcpy(m_InitialVector, initialVector, INIT_VECTOR_LEN);
	}
	if (encryptedSessionKeyInfo != NULL)
	{
		memcpy(m_EncryptedSessionKeyInfo, encryptedSessionKeyInfo, SESSION_KEY_INFO_LEN);
	}
}
//===============================================================================================================
P6_Message::~P6_Message()
{
	// removing keys stamp from memory
	if (m_InitialVector != NULL)
	{
		memset(m_InitialVector, '\0', INIT_VECTOR_LEN);
	}
	if (m_EncryptedSessionKeyInfo != NULL)
	{
		memset(m_EncryptedSessionKeyInfo, '\0', SESSION_KEY_INFO_LEN);
	}
}
//===============================================================================================================
const char* P6_Message::NameOf() const
{
	return "P6_Message";
}
//===============================================================================================================
void P6_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			// encode identifier
			dest << (BYTE)P6_Identifier;

			// encode total content length
			DWORD p6_total_content_len = (DWORD)P6_ASN1_LEN;
			DWORD encoded_bytes = 0;
			EncodeLengthInAsn1(p6_total_content_len, dest, encoded_bytes);

			// encode m_InitialVector
			EncodeASN1ParamContext(0, dest, encoded_bytes);
			EncodeAsn1BitString(m_InitialVector, INIT_VECTOR_LEN, dest, encoded_bytes);

			// encode m_EncryptedSessionKeyInfo
			EncodeASN1ParamContext(1, dest, encoded_bytes);
			EncodeAsn1BitString(m_EncryptedSessionKeyInfo, SESSION_KEY_INFO_LEN, dest, encoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P6_Message::Serialize encoded_bytes =  ", (DWORD )encoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			dest.Put(m_InitialVector, INIT_VECTOR_LEN);
			dest.Put(m_EncryptedSessionKeyInfo, SESSION_KEY_INFO_LEN);
			break;
		}
	}
}
//===============================================================================================================
DWORD P6_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		{
			// ASN.1 format (identifier already read before - to create the P0 class)
			DWORD decoded_bytes = 0;
			DWORD total_content_length = GetDecodeASN1Length(source, decoded_bytes);
			PTRACE2INT(eLevelInfoNormal, "P6_Message::DeSerialize total_content_length =  ", (DWORD )total_content_length);

			// decode m_InitialVector
			BYTE param_context = DecodeASN1ParamContext(source, decoded_bytes, 0);
			WORD initial_vector_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (initial_vector_length > INIT_VECTOR_LEN)
			{
				PTRACE2INT(eLevelInfoNormal, "P6_Message::DeSerialize wrong initial_vector_length =  ", (DWORD )initial_vector_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P6_Message::DeSerialize initial_vector_length =  ", (DWORD )initial_vector_length);
			DecodeAsn1BitString(source, m_InitialVector, initial_vector_length, decoded_bytes);

			// decode m_EncryptedSessionKeyInfo
			param_context = DecodeASN1ParamContext(source, decoded_bytes, 1);
			WORD session_key_info_length = GetDecodeASN1Length(source, decoded_bytes) - BITSTRING_UNUSED_BITS_FIELD_LEN;
			if (session_key_info_length > SESSION_KEY_INFO_LEN)
			{
				PTRACE2INT(eLevelInfoNormal, "P6_Message::DeSerialize wrong session_key_info_length =  ", (DWORD )session_key_info_length);
				deserialize_status = STATUS_ASN1_DESERIALIZE_FAILURE;
				break;
			}
			PTRACE2INT(eLevelInfoNormal, "P6_Message::DeSerialize session_key_info_length =  ", (DWORD )session_key_info_length);
			DecodeAsn1BitString(source, m_EncryptedSessionKeyInfo, session_key_info_length, decoded_bytes);

			PTRACE2INT(eLevelInfoNormal, "P6_Message::DeSerialize decoded_bytes =  ", (DWORD )decoded_bytes);
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			source.Get(m_InitialVector, INIT_VECTOR_LEN);
			source.Get(m_EncryptedSessionKeyInfo, SESSION_KEY_INFO_LEN);
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================
BYTE* P6_Message::GetInitialVector()
{
	return m_InitialVector;
}
//===============================================================================================================
BYTE* P6_Message::GetEncryptedSessionKeyInfo()
{
	return m_EncryptedSessionKeyInfo;
}
//===============================================================================================================

//===============================================================================================================
// P1_Message
//===============================================================================================================
P1_Message::P1_Message() :
		P_Message(P1_Identifier)
{
}
//===============================================================================================================
P1_Message::~P1_Message()
{
}
//===============================================================================================================
const char* P1_Message::NameOf() const
{
	return "P1_Message";
}
//===============================================================================================================
void P1_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			dest << (BYTE)P1_Identifier;
			// no content
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			// no content
			break;
		}
	}
}
//===============================================================================================================
DWORD P1_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		case NATIVE :
		default:
		{
			// no content
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================

//===============================================================================================================
// P2_Message
//===============================================================================================================
P2_Message::P2_Message() :
		P_Message(P2_Identifier)
{
}
//===============================================================================================================
P2_Message::~P2_Message()
{
}
//===============================================================================================================
const char* P2_Message::NameOf() const
{
	return "P2_Message";
}
//===============================================================================================================
void P2_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			dest << (BYTE)P2_Identifier;
			// no content
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			// no content
			break;
		}
	}
}
//===============================================================================================================
DWORD P2_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		case NATIVE :
		default:
		{
			// no content
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================

//===============================================================================================================
// P11_Message
//===============================================================================================================
P11_Message::P11_Message() :
		P_Message(P11_Identifier)
{
}
//===============================================================================================================
P11_Message::~P11_Message()
{
}
//===============================================================================================================
const char* P11_Message::NameOf() const
{
	return "P11_Message";
}
//===============================================================================================================
void P11_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			dest << (BYTE)P11_Identifier;
			// no content
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			// no content
			break;
		}
	}
}
//===============================================================================================================
DWORD P11_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		case NATIVE :
		default:
		{
			// no content
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================

//===============================================================================================================
// NULL_Message
//===============================================================================================================
NULL_Message::NULL_Message() :
		P_Message(NULL_Identifier)
{
}
//===============================================================================================================
NULL_Message::~NULL_Message()
{
}
//===============================================================================================================
const char* NULL_Message::NameOf() const
{
	return "NULL_Message";
}
//===============================================================================================================
void NULL_Message::Serialize(WORD format, CSegment& dest)
{
	switch (format)
	{
		case SERIALEMBD :
		{
			// for embedded - full ASN.1 format
			dest << (BYTE)NULL_Identifier;
			// no content
			break;
		}
		case NATIVE :
		default:
		{
			// native - class data
			// no content
			break;
		}
	}
}
//===============================================================================================================
DWORD NULL_Message::DeSerialize(WORD format, CSegment& source)
{
	DWORD deserialize_status = STATUS_OK;
	switch (format)
	{
		case SERIALEMBD :
		case NATIVE :
		default:
		{
			// no content
			break;
		}
	}
	return deserialize_status;
}
//===============================================================================================================

//===============================================================================================================
// P_Message
//===============================================================================================================
P_Message::P_Message(BYTE identifier) :
		m_identifier(identifier)
{
}
//===============================================================================================================
P_Message::~P_Message()
{
	// removing keys stamp from memory
	m_identifier = 0;
}
//===============================================================================================================
const char* P_Message::NameOf() const
{
	return "P_Message";
}
//===============================================================================================================
DWORD P_Message::GetDecodeASN1Length(CSegment& pParam, DWORD& decoded_bytes)
{
	BYTE current_byte = 0;

	pParam >> current_byte;
	decoded_bytes++;

	BYTE first_len_octet = current_byte;
	DWORD decoded_length = 0;
	if (first_len_octet & 0x80)
	{ //MSB=1 - Long Form Length
		PTRACE2INT(eLevelInfoNormal, "P_Message::GetDecodeASN1Length: long form = ", (DWORD )first_len_octet);
		BYTE num_of_byte_for_len = first_len_octet & 0x7F;
		if (num_of_byte_for_len > 4)
		{
			PTRACE2INT(eLevelInfoNormal, "P_Message::GetDecodeASN1Length wrong num_of_byte_for_len (>4) = ", num_of_byte_for_len);
			return 0;
		}
		BYTE* len_bytes = new BYTE[num_of_byte_for_len];
		for (WORD len_byte_index = 0; len_byte_index < num_of_byte_for_len; len_byte_index++)
		{
			pParam >> len_bytes[len_byte_index];
			decoded_bytes++;
		}

		WORD len_byte_index = 0;
		for (WORD j = num_of_byte_for_len; j > 0; j--)
		{
			BYTE byte_val = len_bytes[len_byte_index++];
			decoded_length |= (byte_val << ((j - 1) * 8));
		}
		delete[] len_bytes;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "P_Message::GetDecodeASN1Length: short form= ", (DWORD )first_len_octet);
		decoded_length = first_len_octet;
	}
	if (PRINT_THE_KEYS)
	{
		PTRACE2INT(eLevelInfoNormal, "P_Message::GetDecodeASN1Length decoded_length = ", decoded_length);
	}
	return decoded_length;
}
//===============================================================================================================
BYTE P_Message::DecodeASN1ParamContext(CSegment& pParam, DWORD& decoded_bytes, WORD param_num)
{
	BYTE param_context = 0;
	pParam >> param_context;
	decoded_bytes++;
	if (param_context != (CONTEXT_SPECIFIC | TagValue[param_num]))
	{
		PTRACE2INT(eLevelInfoNormal, "P_Message::DecodeASN1ParamContext::DecodeASN1Event wrong param_context = ", (DWORD )param_context);
		return 0;
	}
	PTRACE2INT(eLevelInfoNormal, "P_Message::DecodeASN1ParamContext::DecodeASN1Event param_context = ", (DWORD )param_context);
	return param_context;
}
//===============================================================================================================
WORD P_Message::DecodeASN1NumOfUnusedBits(CSegment& pParam, DWORD& decoded_bytes)
{
	BYTE current_byte = 0;
	pParam >> current_byte;
	decoded_bytes++;
	BYTE unused_bits = current_byte;
	if (unused_bits >= 8)
	{
		PTRACE2INT(eLevelInfoNormal, "P_Message::DecodeASN1NumOfUnusedBits wrong number of  unused_bits = ", (DWORD )unused_bits);
		return 0;
	}
	PTRACE2INT(eLevelInfoNormal, "P_Message::DecodeASN1NumOfUnusedBits number of  unused_bits = ", (DWORD )unused_bits);
	return (WORD)unused_bits;
}
//===============================================================================================================
void P_Message::DecodeAsn1BitString(CSegment& source, BYTE* pDest, DWORD length, DWORD& decoded_bytes)
{
	WORD unused_bits = DecodeASN1NumOfUnusedBits(source, decoded_bytes);

	if (unused_bits == 0)
	{
		PTRACE(eLevelInfoNormal, "P_Message::DecodeAsn1BitString unused_bits == 0");
		for (WORD data_index = 0; data_index < length; data_index++)
		{
			source >> pDest[data_index];
			decoded_bytes++;
		}
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "P_Message::DecodeAsn1BitString unused_bits = ", (DWORD )unused_bits);
		BYTE shift_size = 8 - unused_bits;
		BYTE current_byte = 0;
		for (WORD data_index = 0; data_index < length; data_index++)
		{
			source >> current_byte;
			decoded_bytes++;
			// remove unused bits from current byte
			// 1) reset unused bits (11111111 -> 11111100)
			WORD tmp = (WORD)current_byte & Significant_Bits[unused_bits];
			// 2) remove used bites to start of BYTE (00000000|11111100 -> 00111111|00000000)
			tmp = tmp << shift_size;
			// 3) move firs to second BYTE (00111111|00000000 -> 00000000|00111111)
			// abd take last byte
			BYTE clean_data_byte = (BYTE)((tmp & 0xFF00) >> 8);
			pDest[data_index] = clean_data_byte;
		}
	}

	DumpHex("P_Message::DecodeAsn1BitString pDest = ", pDest, length, eLevelInfoNormal);

}
//===============================================================================================================
void P_Message::DumpHex(const char* buffer_name, BYTE* data_buff, DWORD data_buffer_size, DWORD trace_level) const
{

	if (!PRINT_THE_KEYS)
	{
		PTRACE2(trace_level, buffer_name, DONT_PRINT_THE_KEYS);
		return;
	}

	if (data_buffer_size > 256)
	{ // limit dump size for PTRACE macro
		data_buffer_size = 256;
		PTRACE2INT(eLevelInfoNormal, "P_Message::DumpHex data_buffer_size > 256 - print first 256 bytes, data_buffer_size= ", (DWORD )data_buffer_size);
	}

	DWORD HexCharsPerDataByte = 5; // ",0xFF"
	DWORD secure_tail = 16 * 5;
	DWORD message_buffer_size = data_buffer_size * HexCharsPerDataByte + secure_tail;
	char* msgStr = new char[message_buffer_size];
	memset(msgStr, '\0', message_buffer_size);

	char temp[16];
	memset(temp, '\0', 16);

	for (DWORD byte_index = 0; byte_index < data_buffer_size; byte_index++)
	{
		if (byte_index == 0)
			sprintf(temp, "{0x%02x", (unsigned char)(data_buff[byte_index]));
		else if (byte_index == (data_buffer_size - 1))
			sprintf(temp, ",0x%02x}", (unsigned char)(data_buff[byte_index]));
		else
			sprintf(temp, ",0x%02x", (unsigned char)(data_buff[byte_index]));

		strcat(msgStr, temp);
	}
	PTRACE2(trace_level, buffer_name, msgStr);

	delete[] msgStr;
}
//===============================================================================================================
void P_Message::EncodeLengthInAsn1(DWORD length, CSegment& dest, DWORD& encoded_bytes) const
{
	if (PRINT_THE_KEYS)
	{
		PTRACE2INT(eLevelInfoNormal, "P_Message::EncodeLengthInAsn1: length = ", (DWORD )length);
	}
	// insert total message length
	if (length > 127) //if asn1 msg length needs long form
	{
		BYTE num_of_base_len_bytes = (BYTE)(GetNumOfBytesForEncodingLengthInAsn1(length) - 1); // -1 ==> not including LF_LEN_FIELD_OCTET1_LENGTH
		if (PRINT_THE_KEYS)
		{
			PTRACE2INT(eLevelInfoNormal, "P_Message::EncodeLengthInAsn1: num_of_base_len_bytes = ", (DWORD )num_of_base_len_bytes);
		}

		dest << (BYTE)(0x80 | num_of_base_len_bytes); //first bit = 1 in Long-form length first length-field octet
		encoded_bytes++;

		DWORD byte_val = 0;
		for (BYTE j = num_of_base_len_bytes; j > 0; j--)
		{
			// encode each byte in the length DWORD
			byte_val = length & (0xFF << (8 * (j - 1)));
			byte_val = byte_val >> (8 * (j - 1));
			dest << (BYTE)(byte_val & 0xFF);
			encoded_bytes++;
		}
	}
	else
	{
		if (PRINT_THE_KEYS)
		{
			PTRACE(eLevelInfoNormal, "P_Message::EncodeLengthInAsn1: num_of_base_len_bytes = 1");
		}
		dest << (BYTE)length;
		encoded_bytes++;
	}
}
//===============================================================================================================

void P_Message::EncodeASN1ParamContext(WORD param_num, CSegment& dest, DWORD& encoded_bytes) const
{
	PTRACE(eLevelInfoNormal, "P_Message::EncodeASN1ParamContext");
	BYTE param_context = (BYTE)(CONTEXT_SPECIFIC | TagValue[param_num]);
	dest << param_context;
	encoded_bytes++;
}
//===============================================================================================================
DWORD P_Message::GetNumOfBytesForEncodingLengthInAsn1(DWORD data_length) const
{
	DWORD encoding_num_of_bytes = 0;
	if (data_length > 127)
	{
		encoding_num_of_bytes += LF_LEN_FIELD_OCTET1_LENGTH;
		if ((data_length & 0xFF) == data_length)
		{
			encoding_num_of_bytes += 1;
		}
		else if ((data_length & 0xFFFF) == data_length)
		{
			encoding_num_of_bytes += 2;
		}
		else if ((data_length & 0xFFFFFF) == data_length)
		{
			encoding_num_of_bytes += 3;
		}
		else
		{
			encoding_num_of_bytes += 4;
		}
	}
	else
	{
		encoding_num_of_bytes += SF_LEN_FIELD_LENGTH;
	}
	return encoding_num_of_bytes;
}
//===============================================================================================================
void P_Message::EncodeAsn1BitString(BYTE* pSource, DWORD source_length, CSegment& dest, DWORD& encoded_bytes)
{
	DumpHex("P_Message::EncodeAsn1BitString pDest = ", pSource, source_length, eLevelInfoNormal);

	// encode length
	DWORD content_len = source_length + BITSTRING_UNUSED_BITS_FIELD_LEN;
	EncodeLengthInAsn1(content_len, dest, encoded_bytes);

	// insert unused bits
	dest << (BYTE)0; //no unused bits at the end of string
	encoded_bytes++;

	// insert parameter data
	for (DWORD data_byte = 0; data_byte < source_length; data_byte++)
	{
		dest << pSource[data_byte];
		encoded_bytes++;
	}
}
//===============================================================================================================
BYTE P_Message::GetIdentifier() const
{
	return m_identifier;
}
//===============================================================================================================
void P_Message::Dump(COstrStream& msg) const
{
	msg << "m_identifier = ";
	switch (m_identifier)
	{
		case P0_Identifier:
			msg << "P0_Identifier";
			break;
		case P1_Identifier:
			msg << "P1_Identifier";
			break;
		case P2_Identifier:
			msg << "P2_Identifier";
			break;
		case P3_Identifier:
			msg << "P3_Identifier";
			break;
		case P4_Identifier:
			msg << "P4_Identifier";
			break;
		case P5_Identifier:
			msg << "P5_Identifier";
			break;
		case P6_Identifier:
			msg << "P6_Identifier";
			break;
		case P8_Identifier:
			msg << "P8_Identifier";
			break;
		case P9_Identifier:
			msg << "P9_Identifier";
			break;
		case P11_Identifier:
			msg << "P11_Identifier";
			break;
		case NULL_Identifier:
			msg << "NULL_Identifier";
			break;
		default:
			msg << "unknown identifier = " << m_identifier;
			break;
	}
	msg << "\n";
}
//===============================================================================================================
void P_Message::DumpHex(COstrStream& msg, const char* data_buff, int data_len) const
{
	const WORD msgBufSize = 2048;
	char* msgStr = new char[msgBufSize];
	memset(msgStr, '\0', msgBufSize);
	char temp[16];
	memset(temp, '\0', 16);

	//============= Ron - hex trace of messages ===================

	DWORD buflen = data_len; // m_pMplMcmsProt->getDataLen() received from mux maximum buffer - caused exception

	for (DWORD byte_index = 0; byte_index < buflen; byte_index++)
	{
		if (0 == byte_index)
			sprintf(temp, "{0x%02x", (unsigned char)(data_buff[byte_index]));
		else if (byte_index == (buflen - 1))
			sprintf(temp, ",0x%02x}", (unsigned char)(data_buff[byte_index]));
		else
			sprintf(temp, ",0x%02x", (unsigned char)(data_buff[byte_index]));

		strcat(msgStr, temp);
	}

	msg << msgStr;
	delete[] msgStr;
}
//===============================================================================================================
void P_Message::Dump() const
{
	COstrStream msg;
	Dump(msg);
	PTRACE2(eLevelInfoNormal, "P_Message::Dump:\n", (char* )msg.str().c_str());
}
//===============================================================================================================
// unused funtions
//===============================================================================================================
// void  P_Message::DumpHexByte(const char* buffer_name,BYTE data_byte) const
// {
//   bool dump=false;
//   if(dump){
//     char        temp[16];
//     memset(temp,'\0',16);

//     sprintf(temp,"0x%02x",data_byte);

//     PTRACE2(eLevelInfoNormal,buffer_name,temp);
//   }
// }
//===============================================================================================================
// eathra inter-op debug
// void P_Message::DecodeAsn1BitStringIgnoreUnusedBits(CSegment& source,BYTE* pDest,DWORD length,DWORD& decoded_bytes)
// {
//   WORD unused_bits = DecodeASN1NumOfUnusedBits(source,decoded_bytes);

//   if(unused_bits != 0){
//     PTRACE2INT(eLevelInfoNormal,"P_Message::DecodeAsn1BitString IGNORING (correct unused_bits to 0) unused_bits = ",(DWORD)unused_bits);
//   }
//   PTRACE(eLevelInfoNormal,"P_Message::DecodeAsn1BitString unused_bits == 0");
//   for(WORD data_index=0;data_index<length;data_index++){
//     source >> pDest[data_index];
//     decoded_bytes++;
//   }
//   DumpHex("P_Message::DecodeAsn1BitString pDest = ",pDest,length,eLevelInfoNormal);
// }
//===============================================================================================================
