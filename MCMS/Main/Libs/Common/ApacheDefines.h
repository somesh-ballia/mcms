#ifndef _APACHEDEFINES_H
#define _APACHEDEFINES_H


// #define SUPER			0
// #define ORDINARY        1
// #define AUTH_OPERATOR   2
// #define RECORDING_USER  3
// #define RECORDING_ADMIN 4
// #define GUEST           100
// #define ANONYMOUS       101

// #define DEL_OPERATOR      40
// #define NEW_OPERATOR      41
// #define UPDATE_OPERATOR   42
// #define LOG_IN_OPERATOR   43
// #define LOG_OUT_OPERATOR  44

#define OS_XPE		0
#define OS_LINUX	1

#define CONN_TIMEOUT_MINUTES	15

#define MAX_CONNECTIONS			100

#define REQUEST_TIMEOUT_SHORT	500 // was 20000
//#define REQUEST_TIMEOUT_LONG	REQUEST_TIMEOUT_SHORT+500 // 5 seconds longer than REQUEST_TIMEOUT_SHORT 
#define REQUEST_TIMEOUT_LONG	20*SECOND // used for Installer process

#define CREATE_DIR_NONE		2
#define CREATE_DIR_ALL		1
#define CREATE_DIR_ADMIN	0

#define ENTITY_MANAGEMENT	0
#define ENTITY_SHELF		1
#define SECURITY_TOKEN_REQUEST_TIME_OUT  60*SECOND
#define SECURITY_TOKEN_MAX 120
#endif //_APACHEDEFINES_H

