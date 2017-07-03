// keycode_gen_cDlg.h : header file
//
// = = = = = ////////////////////////////////#ifdef __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif		/* __cplusplus */

/*
__declspec(dllexport) void __cdecl generateKeyCode(char keycode[], char serial[], char option[], char c_type);
__declspec(dllexport) void __cdecl getOptionsFromKeyCode(char option_mask[], char key_code[]);
__declspec(dllexport) void __cdecl  substring(char str[], char src[], int start, int end);
__declspec(dllexport) long __cdecl getValidOptionValue(char sourceKey[],char serialNumber[]);
*/
	
// = = = = = /////////////#ifdef __cplusplus
// = = = = = /////////////}
// = = = = = /////////////#endif		/* __cplusplus */

// = = = = = ////////////////////////////////#else

	void generateKeyCode(char keycode[], char serial[], char option[], char c_type);
	void getOptionsFromKeyCode(char option_mask[], char key_code[]);
    void substring(char str[], char src[], int start, int end);
	long getValidOptionValue(char sourceKey[],char serialNumber[]);

// = = = = = ////////////////////////////////#endif		/* __cplusplus */

// = = = = = new = = = = = //
#ifdef __cplusplus
}
#endif		/* __cplusplus */
// = = = = = new = = = = = //



//#ifndef  i386   /* For ALPHA  (SAK) */
//#define LITTLE_ENDIAN 
//typedef          long int int64;
//typedef unsigned long int uint64;
//typedef          int int32;
//typedef unsigned int uint32;
//#else  /* i386*/
//#define LITTLE_ENDIAN 
//AI long int is encient hiostr in 32bit world but is important for embeded system!
//typedef          long long int int64;
//typedef unsigned long long int uint64;
//////

typedef          long int int64;
typedef unsigned long int uint64;
typedef          long int int32;
typedef unsigned long int uint32;
//#endif /*i386*/

typedef struct {
    uint32 state[5];
    uint32 count[2];
    unsigned char buffer[64];
} SHA1_CTX;


#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

#define OPTION_KEY_LENGTH	25
#define OPTION_BITMASK_SIZE 22 //const int OPTION_BITMASK_SIZE = 22;
#define KEY_FORMAT_LENGTH    4 //const int KEY_FORMAT_LENGTH   = 4;
#define OPTION_FORMAT_MASK   9 //const int OPTION_FORMAT_MASK  = 9;


void digout(char code[],unsigned char digestBits[], int size);
void SHA1Transform(uint32 state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, uint32 len);/* JHB */
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

