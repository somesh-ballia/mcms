#ifndef _OPERATORDEFINES_H
#define _OPERATORDEFINES_H


#define SUPER           0
#define ORDINARY        1
#define AUTH_OPERATOR   2
#define RECORDING_USER  3
#define RECORDING_ADMIN 4
#define AUDITOR         5
#define ADMINISTRATOR_READONLY         6
#define GUEST           100
#define ANONYMOUS       101

#define DEL_OPERATOR      40
#define NEW_OPERATOR      41
#define UPDATE_OPERATOR   42
#define LOG_IN_OPERATOR   43
#define LOG_OUT_OPERATOR  44

enum eAudibleAlarmType
{
	eAudibleAlarmTypeNone = 0,
	eAwaitingOperatorAssistance = 1
};

#endif //_OPERATORDEFINES_H

