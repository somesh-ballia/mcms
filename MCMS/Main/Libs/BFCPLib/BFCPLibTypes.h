#include "BFCPMessage.h"
#include "DataTypes.h"
//#include "dummy.h"

#define BFCP_MAX_MSG_SIZE 256
//return status from the library functions
typedef enum
{
	statusOK,
    statusBFCPMsgIsQuery,
	statusBFCPHello,
	statusBFCPHelloAck,
	statusBFCPGoodbye,
	statusBFCPGoodbyeAck,
	statusCanNotConvert,
    statusError
} eStatus;

//The Translator object
typedef struct BFCPH239Translator_S
{
    UInt32 validFlag;
    UInt16 h239FloorID;
	eBFCPPriority priority;
    BFCPObject_T BFCPObj;
    char lastDecodedMsg[BFCP_MAX_MSG_SIZE];
} BFCPH239Translator;
