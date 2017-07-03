/**
 * \file sha1.h
 */
#ifndef _SHA1_H
#define _SHA1_H

#ifdef __cplusplus
extern "C" {
#endif


/* ------------ definitions for base64 ------------ */

// use  BASE_64_OUT_LEN_CALC to calc the output size of base64  
#define BASE_64_OUT_LEN_CALC(len) ((len + 2) / 3 * 4 + 1)

// base64 transforms binary array bin of size len into array of chars of printable ascii
// of size BASE_64_OUT_LEN_CALC(len).
// the result will be stored in the asciOut,
// asciOut must be allocated before calling base64,
// use BASE_64_OUT_LEN_CALC(len) for allocation    
void base64 (const char *bin, size_t len, char *asciOut);
    

/* ------------ end definitions for base64 ------------ */

    
/**
 * \brief          SHA-1 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[5];     /*!< intermediate digest state  */
    char buffer[64];   /*!< data block being processed */
    char ipad[64];     /*!< HMAC: inner padding        */
    char opad[64];     /*!< HMAC: outer padding        */
}
sha1_context;

/**
 * \brief          SHA-1 context setup
 *
 * \param ctx      context to be initialized
 */
void sha1_starts( sha1_context *ctx );

/**
 * \brief          SHA-1 process buffer
 *
 * \param ctx      SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha1_update( sha1_context *ctx, char *input, int ilen );

/**
 * \brief          SHA-1 final digest
 *
 * \param ctx      SHA-1 context
 * \param output   SHA-1 checksum result
 */
void sha1_finish( sha1_context *ctx, char *output );

/**
 * \brief          Output = SHA-1( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-1 checksum result
 */
void sha1( char *input, int ilen,
           char *output );

/**
 * \brief          Output = SHA-1( file contents )
 *
 * \param path     input file name
 * \param output   SHA-1 checksum result
 *
 * \return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int sha1_file( char *path, char *output );

/**
 * \brief          SHA-1 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void sha1_hmac_starts( sha1_context *ctx,
                       char *key, int keylen );

/**
 * \brief          SHA-1 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha1_hmac_update( sha1_context *ctx,
                       char *input, int ilen );

/**
 * \brief          SHA-1 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-1 HMAC checksum result
 */
void sha1_hmac_finish( sha1_context *ctx, char *output );

/**
 * \brief          Output = HMAC-SHA-1( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-1 result
 */
void sha1_hmac( char *key, int keylen,
                char *input, int ilen,
                char *output );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int sha1_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* sha1.h */
