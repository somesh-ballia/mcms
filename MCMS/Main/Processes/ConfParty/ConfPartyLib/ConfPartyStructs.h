//
// ConfPartyStructs.h
//

#if !defined(_CONFPARTYSTRUCTS_H__)
#define _CONFPARTYSTRUCTS_H__

#define IDENTIFIER_STR_SIZE 256
#define NUM_OF_IDENTIFIERS 5
typedef struct
{
    char	ipAddress[IDENTIFIER_STR_SIZE];
    char	identifier[NUM_OF_IDENTIFIERS][IDENTIFIER_STR_SIZE];
} IP_EXT_DB_STRINGS;


#endif //_CONFPARTYSTRUCTS_H__
