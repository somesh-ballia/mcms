   	#ifndef _ENCRYPTION_KEY_SERVER_API_H_
#define _ENCRYPTION_KEY_SERVER_API_H_


#define		SHARED_SECRET_LENGTH			128
#define		SHARED_SECRET_LENGTH_TO_SEND	32
#define		XMIT_RCV_KEY_LENGTH			    16

#define		PRIME_NUMBER_SIZE	(SHARED_SECRET_LENGTH)
#define		HALF_KEY_SIZE		(SHARED_SECRET_LENGTH)


//------------------------------ Structures -----------------------------------


typedef struct
{
	BYTE		halfKey [HALF_KEY_SIZE];
} HALF_KEY_S;


typedef struct
{
	BYTE			status;
	HALF_KEY_S	halfKey;
	HALF_KEY_S	randomNumber;
} HALF_KEY_IND_S;



#endif //_ENCRYPTION_KEY_SERVER_API_H_

