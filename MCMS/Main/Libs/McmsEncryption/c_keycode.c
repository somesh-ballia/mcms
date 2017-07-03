/*
This key code generator using the original implementation of 
SHA-1 in C
By Steve Reid <sreid@sea-to-sky.net>
100% Public Domain

Modified by Igor Androsov to be used for Polycom key code generator.
The core of SHA1 still intact the part that was changed is
void SHA1Init(SHA1_CTX* context) function initialization values
have been shuffeld for security not to match the standard SSL SHA1.

Additional functions has been added to provide API interface to key 
code generator and format the results.

List of new functions:

void substring(char str[], char src[], int start, int end)
void getOptionMaskHexValue(char optx[], char option[], char c_type)
void StringToUpper(char result[], char src[], int size)
void formatKeyCode(char c_type, char key[])
void digout(char code[], unsigned char digestBits[], int size) 

The Exported API:

  generateKeyCode - use to generate key code
  getOptionsFromKeyCode - use to get options mask from a given key code


*/

/* #define SHA1HANDSOFF  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "c_keycode.h"



/* #include <process.h> */

/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(uint32 state[5], unsigned char buffer[64])
{
uint32 a, b, c, d, e;
typedef union {
    unsigned char c[64];
    uint32 l[16];
} CHAR64LONG16;
CHAR64LONG16* block;

#ifdef SHA1HANDSOFF
static unsigned char workspace[64];
    block = (CHAR64LONG16*)workspace;
    memcpy(block, buffer, 64);
#else
    block = (CHAR64LONG16*)buffer;
#endif
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
}


/* SHA1Init - Initialize new context */

void SHA1Init(SHA1_CTX* context)
{
    /* SHA1 initialization constants 
       As defined for SSL standard
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
	**********/

	/* Shuffeled driver keys used for key code generation */
	context->state[0] = 0x98BADCFE;
	context->state[1] = 0xC3D2E1F0;
	context->state[2] = 0x67452301;
	context->state[3] = 0x10325476;
	context->state[4] = 0xEFCDAB89;

    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Update(SHA1_CTX* context, unsigned char* data, uint32 len)	/* JHB */
{
	uint32 i, j;
    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) 
	{
        memcpy(&context->buffer[j], data, (i = 64-j));
        SHA1Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) 
		{
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
	uint32 i;	
	unsigned char finalcount[8];

    for (i = 0; i < 8; i++) 
	{
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }

    SHA1Update(context, (unsigned char *)"\200", 1);
    while ((context->count[0] & 504) != 448) 
	{
        SHA1Update(context, (unsigned char *)"\0", 1);
    }
    
	SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++) 
	{
        digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
    /* Wipe variables */
    i = 0;	
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(finalcount, 0, 8);

#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
    SHA1Transform(context->state, context->buffer);
#endif
}
  
// Add key code methods here
/**
 * Print out the digest in a form that can be easily compared
 * to the test vectors.
 */
void digout(char code[], unsigned char digestBits[], int size) 
{
	char c1; 
	//char c2;
	int i;

  for ( i = 0; i < size; i++) 
  {
       c1 = (char) ((digestBits[i] >> 4) & 0xf);
       //c2 = (char) (digestBits[i] & 0xf);
       c1 = (char) ((c1 > 9) ? 'a' + (c1 - 10) : '0' + c1);
       //c2 = (char) ((c2 > 9) ? 'a' + (c2 - 10) : '0' + c2);
       code[i] = toupper(c1);
       //sb.append(c2);            
  }
  
}

/**
 * formatKeyCode
 * Format KeyCode 20 character string as XXXX-XXXX-XXXX-XXXX-XXXX
 * Append Type of key to start (K/U/S)
 *
 * @param s_type - Type of key code generated K/U/S
 * @param key - Generated hash value of key code and options
 * @return - String value of complete finished key code
 */
 void formatKeyCode(char c_type, char key[])
 {
	char res[30] = {0};
	char tmp[30] = {0};
	char val[2] = {0};
    int count = 0;
	int i;

	strncpy(tmp,key,sizeof(tmp)-1);

	res[0] = c_type;
	res[1] = '\0';

    for ( i = 0; i < (int)strlen(tmp); i++)
    {
      if (count == KEY_FORMAT_LENGTH)
      {
		  strcat(res,"-\0");
          count = 0;
      }

      // Add 1 to count for appended prefix to make format consitent
      
      if ((i == 0) /*&& (c_type != 'X')*/)
          count++;
      val[0] = tmp[i];
	  strcat(res,val);
      count++;
    }
	memset(key,0,strlen(key));
    strcat(key,res);
 }

 /**
  * StringToUpper
  *
  * Convert any string array of characters to Upper case
  *
  * @return result - character array (string) result of uppercase conversion
  * @param src - the character array input string to convert may be in mixed characters
  * @param size - size of input array src.
  */
 void StringToUpper(char result[], char src[], int size)
 {
	 int i;

	 for ( i = 0; i < size; i++)
		result[i] = toupper(src[i]);
 }

 /**
  * getOptionMaskHexValue
  *
  * Format optinos mask in hex value. Selected options are not required and
  * based on type of key code is generated (K/S/U)
  *
  * @param option - Value of options bit mask as String data type
  * @param s_type - Type of key code being generated (K/U/S)
  * @return - String value of options bit mask
  */
 void getOptionMaskHexValue(char optx[], char option[], char c_type)
 {
   char str[OPTION_BITMASK_SIZE] = {0};
   char tmp[OPTION_BITMASK_SIZE] = {0};
   long lTest = 0;


   if (c_type == 'K'  || c_type == 'X') // Enable options for products
   {
	// Convert decimal option mast to HEX number without leading 0x.....
	// This process is not required as value input must allways be provided in hex
	// 4/14/2003 change comment
	/////////////////////////////////////////
	// lTest = atol(option);
	// sprintf(str,"%X",lTest);
	/////////////////////////////////////////
	 strncpy(str,option,sizeof(str)-1);
     // Pad with 0 to 5 characters total
     if (strlen(str) <= OPTION_FORMAT_MASK && strlen(str) > 0)
     {
	   int i;
       for ( i = 0; i < OPTION_FORMAT_MASK - (int)strlen(str); i++) strcat(tmp,"0\0");
       strcat(tmp, str);
     }
	 else
	 {
       strcat(tmp, str);
	 }

	

   }   
   else if (c_type == 'U') // Upgrade product format
   {//Generate Hex Version bitmask (NOT decimal) for security purposes
		int j;
		strncpy(tmp, option, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
		for (j = 0; j < (int)strlen(tmp); j++)
		{
			if (tmp[j] == '.')
				tmp[j] = '0';
		}
     
 	 lTest = atol(tmp);
	 sprintf(str,"%X",(unsigned int)lTest);
	 memset(tmp,0,strlen(tmp));

	 


     // Pad with 0 to 9 characters total
     if (strlen(str) < OPTION_FORMAT_MASK && (int)strlen(str) > 0)
     {
       int i;

       for ( i = 0; i < OPTION_FORMAT_MASK - (int)strlen(str); i++) strcat(tmp,"0\0");

       strcat(tmp,str);
       strcpy(str, tmp);
     }
   }
	else strcpy(tmp,"000000000");   


   StringToUpper(optx, tmp, strlen(tmp));

 }

/**
 * substring
 * Utility function to get substring by position
 */
 void substring(char str[], char src[], int start, int end)
 {
	 int size = 0;
	 int pos = 0;
	 int count = 0;
	 int i;

	 if (str == NULL)
		 return;

	size = strlen(src);
	if (size < end || size < start)
		return;


	for ( i = 0; i < size; i++)
	{
		if (i >= start && i <= end)
		{
			str[count] = src[i];
			count++;
		}
	}
	str[count] = '\0';
 }

/**
 * getOptionsFromKeyCode
 * Function to get option mask from key code value
 *
 * @param key_code - character array (0 terminated string) with formatted key code XXXX-XXXX-....
 * @return option_mask - character array pointer to be filled by option mask from key code.
 *                       The caller must ensure that option_mask has (10 characters) enogh 
 *						 memory allocated
 *                       to store option mask in Hex value.
 */
void getOptionsFromKeyCode(char option_mask[], char key_code[])
{
	char str[30] = {0};
	char tmp[30] = {0};
	int size = 0;
    int pos = 0;
	int i;

   // Remove format characters to get option mask
   size = (int)strlen(key_code);

   for ( i = 0; i < size; i++)
   {
	   if (key_code[i] != '-')
	   {
			str[pos] = key_code[i];
			pos++;
	   }
   }
   // Ensure that data has at least valid options mask size of 8 to do the parsing
   size = (int)strlen(str);
   if ((int)strlen(str) > 8)
   {
		substring(tmp, str, size-8, size);     
		strcpy(option_mask, tmp);
   }
}

/**
 * generateKeyCode
 * Key Generator driver method exported for external use as API
 *
 * @param serial - Serial number of the product for which key code is generated
					provided as character array (string null terminiated)
 * @param option - Poduct's option mask or version number
 * @param s_type - Type of key code gereated (K/U/X) K - options, U - upgrade,
 * X - pure software product (NO Serial #) Serial # is substituted.
 * @return keycode - Formated String representation of key code XXXXX-XXXXX-XXXX.....
 */

void generateKeyCode(char keycode[], 
					 char serial[], 
					 char option[], 
					 char c_type)

{
	SHA1_CTX context;
	int optionLength;
	unsigned char buffer[16384] = {0};
	//char output[80];
	int digest_array_size = 10; //default value for digest array size
	unsigned char digest[20] = {0};
	char optx[24] = {0};
	char c_type_upper;
    // Combine serial # and Option or Version number
    char str[200] = {0};

	
	
	char str1[30] = {0};
	int i;
	int size = 0;
    int pos = 0;

	size = (int)strlen(option);

   for ( i = 0; i < size; i++)
   {
	   if (option[i] != '-')
	   {
			str1[pos] = option[i];
			pos++;
	   }
   }
   strcpy(option, str1);

	
	
	
	
	
	
	strcpy((char*)&buffer,serial);
	strcat((char*)&buffer,option);
	// Make Type allways Upper case for validation
	c_type_upper = toupper(c_type);
    // Set up intail value for SHA1
    if (c_type_upper == 'K' || c_type_upper == 'U' || c_type_upper == 'X')
      digest_array_size  = 10;
    else
      digest_array_size  = 20;

	// Perform algorithm encoding
    SHA1Init(&context);
    SHA1Update(&context, buffer, strlen((char*)buffer));
    SHA1Final(digest, &context);

	// Get the key code value in readable format
    digout(keycode,digest,digest_array_size);


    getOptionMaskHexValue(optx, option, c_type_upper);
	

    // Save options bit maske or version in class
    if (c_type_upper == 'K' || c_type_upper == 'U' || c_type_upper == 'X')
	{
		strcat(keycode, "0");
		optionLength = strlen(optx) - 8;

		if (optionLength < 0)
		{
			optionLength = 0;
		}
		
		strcat(keycode, optx + optionLength);

	}
    // Save formated key code into class member variable
	formatKeyCode(c_type_upper, keycode);


}


/**
 * Return the value of the options to turn on
 * based on the kye entered by the user
 * Written by J Ratcliffe - For VSX
**/
long getValidOptionValue(char sourceKey[],char serialNumber[])
{
	long rc = 0; // no option turned on

	// do we have a key to work with
	// has to be atleast 9 char long
	if ( sourceKey != NULL && strlen(sourceKey) > OPTION_BITMASK_SIZE )
	{
		const char key_type = toupper( sourceKey[0] );
		char option_mask[10] = ""; // 
		char validKey[OPTION_KEY_LENGTH]   = "";

		// get the option mask from the key
		getOptionsFromKeyCode(option_mask,sourceKey);
		// generate the real key base on serial number and options
		// the value returned from getOptionsFromKeyCode is 1 char to long
		generateKeyCode(validKey,serialNumber,(option_mask + 1),key_type);

		// do we have a valid key
		if ( strlen( validKey ) > OPTION_BITMASK_SIZE )
		{
			// compare the keys
			if (strncmp(sourceKey,validKey,strlen(validKey) ) == 0 )
			{
                sscanf(option_mask,"%x",(unsigned int*)&rc);
			}
		}
	}

	return rc;
}
