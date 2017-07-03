/*$Header: /MCMS/MAIN/include/H221.H 14    25/12/01 20:10 Ron $*/
/*======================================================================== 
                              H221.H                                         
              Copyright 1995 Pictel Technologies Ltd.                       
                     All Rights Reserved.                                  
-------------------------------------------------------------------------- 
   NOTE: This software contains valuable trade secrets and proprietary     
   information of Pictel Technologies Ltd. and is protected by law.        
   It may not be copied or distributed in any form or medium, disclosed    
   to third parties, reverse engineered or used in any manner without      
   prior written authorization from Pictel Technologies Ltd.               
-------------------------------------------------------------------------- 
   FILE:                                                                   
   SUBSYSTEM:                                                              
   PROGRAMMER:                                                             
-------------------------------------------------------------------------- 
   Who | Date       | Description                                          
-------------------------------------------------------------------------- 
    |   |                                      
=========================================================================*/



#ifndef ENCRYPTION_DEFINES_H
#define ENCRYPTION_DEFINES_H


// moved from H320AudioCaps.h
#define NUM_OF_SUPPORTED_ENCRYPTION_ALG 1
#define ENCRYPTION_ALG_LENGTH_IN_BYTES 3
#define P0_CONTENT_LENGTH 1
#define P9_CONTENT_LENGTH 3
#define P3_CONTENT_LENGTH 129
#define AES_128_IDENTIFIER (0x08)
#define AES_PARAM (0x01)
#define AES_MEDIA (0x00)

// PO Privacy Systems masks
// Content:(MSB)0000XDRM(LSB)  '1'/'0' supported/not ,X=ISO 8732, D=Diffie-Hellman, R=RSA, M=unspecified/manual key entry.
#define ISO_8732_MASK (0x08)
#define DIFFIE_HELMAN_MASK (0x04)
#define RSA_MASK (0x02)
#define UNSPECIFIED_MASK (0x01)
#define INVALID_P0_MASK (0x240)

// status for asn1 
#define STATUS_ASN1_DESERIALIZE_FAILURE 701
#define STATUS_ASN1_DECODING_FAILED 702

#define UNUSED_HALF_KEY 0
#define P3_HALF_KEY 3
#define P4_HALF_KEY 4

#define LINK_IDENTIFIER_LENGTH sizeof(DWORD)
#define CHECK_CODE_LENGTH 8
#define DERIVED_SHARED_SECRET_LENGTH 16
#define INIT_VECTOR_LEN 16
#define SESSION_KEY_INFO_LEN 64
#define BITSTRING_UNUSED_BITS_FIELD_LEN 1
#define CONTEXT_SPECIFIC 0x80
#define SF_LEN_FIELD_LENGTH  1 //ASN.1 one BYTE (Short Form) for length
#define LF_LEN_FIELD_OCTET1_LENGTH 1 //ASN.1 Long form length field - first byte which specifies the length of the length field
#define MAX_ASN1_MSG_PARAMS 5

// asn1 len of transmited messages
// p3 = context(1)+generator_len(1)+unused_bits(1)+generator(1) + context(1)+prime_len(2)+unused_bits(1)+prime(128) + context(1)+halfkey_len(2)+unused_bits(1)+halfkey(128)
// p3 = 1+1+1+1 + 1+2+1+128 + 1+2+1+128 = 268
// p4 = unused_bits(1)+halfkey(128)  = 1+128 = 129
// p5 = context(1)+ident_len(1)+unused_bits(1)+ident(4) + context(1)+checkcode_len(1)+unused_bits(1)+checkcode(4) = 1+1+1+4 + 1+1+1+8 = 18
// p6 = context(1)+initial_vector_len(1)+unused_bits(1)+initia_lvector(16) + context(1)+keys_info_len(1)+unused_bits(1)+keys_info(64) = 1+1+1+16 + 1+1+1+64 = 86
#define P1_ASN1_LEN 3
#define P3_ASN1_LEN 268
#define P4_ASN1_LEN 129
#define P5_ASN1_LEN 18
#define P6_ASN1_LEN 86

#define ECS_BLOCK_SIZE                 16
#define ECS_BLOCK_DATA_SIZE            9

#define SE_HEADER_SINGLE_BLOCK         0x0
#define SE_HEADER_FIRST_BLOCK          0x20
#define SE_HEADER_INTERMEDIATE_BLOCK   0x40
#define SE_HEADER_LAST_BLOCK           0x60
#define BAD_ECS_BLOCK                  0xFF

#define PRINT_THE_KEYS true
#define DONT_PRINT_THE_KEYS "****************"


// defined in file: /IncludeInternalMcms/H221.h

// /* H.233 and H.234 P msgs Identifiers */
// #define P0_Identifier	(0x80)
// #define P1_Identifier	(0x81)
// #define P2_Identifier	(0x82)
// #define P3_Identifier	(0xA3)	
// #define P4_Identifier	(0x84)
// #define P5_Identifier	(0xA5)
// #define P6_Identifier	(0xA6)
// #define P8_Identifier	(0xC0)
// #define P9_Identifier	(0xC1)
// #define P11_Identifier	(0xAB)
// #define NULL_Identifier	(0xDF)


// /*MUX->MCMS local msg Identifiers */
// #define P3_First_Half_Identifier  (0xA7)
// #define P3_Second_Half_Identifier (0xAB)



// should be defined: 


#endif // ENCRYPTION_DEFINES_H
