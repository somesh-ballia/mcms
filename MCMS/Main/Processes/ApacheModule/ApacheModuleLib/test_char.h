/*
 * test_char.h
 *
 *  Created on: May 6, 2012
 *      Author: stanny
 */

#ifndef TEST_CHAR_H_
#define TEST_CHAR_H_
//http://ci.apache.org/projects/httpd/trunk/doxygen/test__char_8h_source.html
/* this file is automatically generated by gen_test_char, do not edit */
 #define T_ESCAPE_SHELL_CMD     (1)
 #define T_ESCAPE_PATH_SEGMENT  (2)
 #define T_OS_ESCAPE_PATH       (4)
 #define T_HTTP_TOKEN_STOP      (8)
 #define T_ESCAPE_LOGITEM       (16)
 #define T_ESCAPE_FORENSIC      (32)
 #define T_ESCAPE_URLENCODED    (64)
 

 static const unsigned char test_char_table[256] = {
     32,126,126,126,126,126,126,126,126,126,127,126,126,126,126,126,126,126,126,126,
     126,126,126,126,126,126,126,126,126,126,126,126,14,64,95,70,65,102,65,65,
     73,73,1,64,72,0,0,74,0,0,0,0,0,0,0,0,0,0,104,79,
     79,72,79,79,72,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,79,95,79,71,0,71,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,79,103,79,65,126,118,118,118,118,118,118,118,118,118,118,118,118,
     118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
     118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
     118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
     118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
     118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
     118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118 
};
 
 /* c2x takes an unsigned, and expects the caller has guaranteed that
  * 0 <= what < 256... which usually means that you have to cast to
  * unsigned char first, because (unsigned)(char)(x) first goes through
  * signed extension to an int before the unsigned cast.
  *
  * The reason for this assumption is to assist gcc code generation --
  * the unsigned char -> unsigned extension is already done earlier in
  * both uses of this code, so there's no need to waste time doing it
  * again.
  */
 static const char c2x_table[] = "0123456789abcdef";
 
 static APR_INLINE unsigned char *c2x(unsigned what, unsigned char prefix,
                                      unsigned char *where)
{
 #if APR_CHARSET_EBCDIC
     what = apr_xlate_conv_byte(ap_hdrs_to_ascii, (unsigned char)what);
 #endif /*APR_CHARSET_EBCDIC*/
     *where++ = prefix;
     *where++ = c2x_table[what >> 4];
     *where++ = c2x_table[what & 0xf];
     return where;
 }
 
#endif /* TEST_CHAR_H_ */
