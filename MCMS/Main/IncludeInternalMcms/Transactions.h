// Transactions.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TRANSACTIONS_H)
#define _TRANSACTIONS_H

enum eMessageTypes
{
	eMsgTypeInvalid		= -1,
	
	eMsgTypeSetRequest	= 0,
	eMsgTypeGetRequest	= 1,

	//DONT FORGET TO UPDATE THIS
	NUM_OF_MSG_TYPES  =  2
};


#define SET_REQUEST      2
#define GET_REQUEST      3
#define SET_CONFIRM      4     
#define GET_CONFIRM      5

#define UNKNOWN_REQUEST  6


#define MAX_PRIVATE_NAME_LEN	50
#define ERROR_MESSAGE_LEN		1024
#define MAX_ACTION_NAME_LEN     80


#endif //(_TRANSACTIONS_H)
