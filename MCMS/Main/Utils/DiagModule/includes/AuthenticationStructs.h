// AuthenticationStructs.h
//
//////////////////////////////////////////////////////////////////////
#ifndef AUTHENTICATIONSTRUCTS_H_
#define AUTHENTICATIONSTRUCTS_H_


#include "SharedDefines.h"
#include "DiagDataTypes.h"



typedef struct
{
	APIU16    authorizationGroup;
	UINT8     login[OPERATOR_NAME_LEN];     //20 characters, so it will be aligned by 4
	UINT8     password[OPERATOR_PWD_LEN];

	APIU32    future_use1;
	APIU32    future_use2;
} USER_S;


typedef struct
{
	USER_S    usersList[MAX_OPERATORS_IN_MCU];
} USERS_LIST_S;


#endif /*AUTHENTICATIONSTRUCTS_H_*/
