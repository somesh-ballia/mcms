#include "BFCPH239Translator.h"
#include <stdlib.h>

#define PPC_FLOOR 30 //The FloorID we use
#define I_AM_SERVER 1

void HandleBFCPMessage (UINT8 * rawBFCPmsg, UINT16 msgLen)
{
    
	//Let's assume that we received a BFCP message we want to treat as an H239 indication
	
	//first, initialize the Translator object:
    BFCPH239Translator translator;
	if (statusOK != InitializeTranslatorDefaults(&translator, PPC_FLOOR, kBFCPPriorityNormal,
                                                 1, 1, kBFCPFloorCtrlServer))
		return;

	//Now let's decode the message to a struct, remember that data must be allocated in advance:
	BFCPFloorInfoT  BFCPmsg;
	
	if (statusOK != DecodeBFCPMsg (&translator, rawBFCPmsg, msgLen, &BFCPmsg))
		//Error decoding!
		return;

	//Now try to convert it to H239 struct:
	mcIndRoleToken H239Ind;
	
	UInt16 floorReqID = 0; //The Floor Request ID will be given to us in this case from the BFCP msg
	//Convert the message to H239 indication - we are not waiting for a response for anything so fkUnknownRoleTokenOpcode
	eStatus convertStatus = BFCPToH239Ind (&translator, &BFCPmsg, &H239Ind, &floorReqID, kUnknownRoleTokenOpcode);
	eStatus responseStatus = statusOK;
	switch (convertStatus)
	{
		case statusOK:
			//All is well - Enjoy your H239 indication!
			break;
		case statusBFCPMsgIsQuery:
			{
				// This is a query need to produce a response
				BFCPFloorInfoT  BFCPQueryResponse;
				//let's say the Floor is released
				if (statusOK != CreateFloorStatusFromQuery (&translator, &BFCPmsg, kBFCPStatusReleased ,&BFCPQueryResponse))
					//Error
					return;


				//All is well, so now we can send the response
				break;
			}
		case statusBFCPHello:
			{
				BFCPFloorInfoT BFCPFirstMsg;
				UInt8 binaryMsg[BFCP_MAX_MSG_SIZE];
                UInt32 len = 0;
				responseStatus = statusError;
				if (I_AM_SERVER) // if I am the server, this is Hello I should answer with Hello Ack
					responseStatus = CreateBFCPHelloAck (&translator, binaryMsg, &len);

				if (statusOK != responseStatus)
					//Error creating the Hello Ack Or maybe we are not a Server at all.
					return;
				
				//All is well, so Send the Hello Ack we just created
				break;
			}		
		case  statusBFCPHelloAck:
			//Nothing to do here, of course it's error if we never sent Hello...
			break;
			
		case statusCanNotConvert:
		case statusError:
			{
				//Handle error

				// ....

				break;
			}	
	}
    
}
