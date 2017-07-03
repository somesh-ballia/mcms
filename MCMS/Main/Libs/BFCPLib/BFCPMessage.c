//+=========================================================
//
//  Filename: BFCPMessage.c
//
//  Description: This code contains the BFCP Messaage support
//               for SIP P+C.
//
//  Copyright:   Polycom, Inc. 2007
//  Revision 1 - by Lior Baram 1/4/2008
//   Memory should be allocated outside this module instead
//   the use of static memory as in the original version
//   (see pBFCPMsg commented below)
//+=========================================================
//+=========================================================
//               INCLUDES
//+=========================================================
#include <stdio.h>
#include <string.h>
#define __LINUX__

#ifdef WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#elif defined (__LINUX__)
//#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/tcp.h>
//#include <linux/in.h>
#include <linux/errno.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SD_BOTH SHUT_RDWR
#define closesocket close
#define ioctlsocket ioctl
#endif
typedef struct timeval timeval;


#include "BFCPMessage.h"
#include "OverRidePrintf.h"

extern UINT8 bytefieldSet(UINT8 value, int bitfield, int nStartBit, int nBits);
extern UINT8 bytefieldGet(UINT8  value, int nStartBit, int nBits);
//---------------------------------------------------------
//               TYPEDEFS & DEFINITIONS
//---------------------------------------------------------
//#define BFCP_DEBUG 1
#define BFCP_MAX_MSG_SIZE 256

static char *gBFCPPrimitiveString[] =
{
    "Invalid",
    "FloorRequest",
    "FloorRelease",
    "FloorRequestQuery",
    "FloorRequestStatus",
    "UserQuery",
    "UserStatus",
    "FloorQuery",
    "FloorStatus",
    "ChairAction",
    "ChairActionAck",
    "Hello",
    "HelloAck",
    "Error",
    "FloorRequestStatusAck",
    "ErrorAck",
    "FloorStatusAck",
    "Goodbye",
    "GoodbyeAck",
    "TandbergFloorRequestStatusAck",
    "TandbergFloorStatusAck"
};

static char *gBFCPAttrString[] =
{
    "Invalid",
    "Beneficiary-ID",
    "Floor-ID",
    "Floor-Request-Id",
    "Priority",
    "Request-Status",
    "Error-Code",
    "Error-Info",
    "Participant-Provided-Info",
    "Status-Info",
    "Supported-Attributes",
    "Supported-Primitives",
    "User-Display-Name",
    "User-URI",
    "Beneficiary-Information",
    "Floor-Request-Information",
    "Requested-By-Information",
    "Floor-Request-Status",
    "Overall-Request-Status",
    "Floor-Request-Status-Ack",
    "Error-Ack",
    "Floor-Status-Ack",
    "Goodbye",
    "Goodbye-Ack"
};

static char *gBFCPStatusString[] =
{
    "Invalid",
    "Pending",
    "Accepted",
    "Granted",
    "Denied",
    "Cancelled",
    "Released",
    "Revoked"
};

#if WIN32
#define getSocketError(n) WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#else
#define getSocketError(n) errno
#endif


//---------------------------------------------------------
//               GLOBALS
//---------------------------------------------------------

//Static memory area used to build all bfcp messages.  It is assumed
//that the user of this data will use it immediately
//static UInt8 pBFCPMsg[BFCP_MAX_MSG_SIZE];

//---------------------------------------------------------
//               Internal Functions
//---------------------------------------------------------

//---------------------------------------------------------
// BuildBFCPCommonHeader
//
// Build the header the starts each BFCP message.
// Return the length of the header.
//---------------------------------------------------------
int BuildBFCPCommonHeader(BFCPCommonHeader_T *pHeader, BFCPFloorInfoT *pFloorInfo, eBFCPPrimitive eType, Boolean bIsToSetInitiatorBit)
{
    int length = 0;
    UINT8 *header = (UINT8*)( (char*)pHeader) ;

#ifdef BFCP_DEBUG
    printf("SIP Bld BFCP %s\n", gBFCPPrimitiveString[eType]);
	printf("  Transaction ID: %d\n", pFloorInfo->transactionID);
	printf("  User ID: %d\n", pFloorInfo->userID);
	printf("  conference ID: %d\n", pFloorInfo->conferenceID);
#endif

    if (pFloorInfo && eType > kBFCPInvalidPrimitive && eType < kBFCPMaxPrimitive)
    {
        memset(pHeader, 0, sizeof(BFCPCommonHeader_T));
        header[0] = bytefieldSet(0,1,5,3);                 //Version: 3 bit

        if (bIsToSetInitiatorBit)
        {
        	header[0] = bytefieldSet(header[0],1,4,1);                 //I bit: 1 bit // http://potaroo.net/ietf/idref/draft-sandbakken-dispatch-bfcp-udp/
        	header[0] = bytefieldSet(header[0],0,0,4);                 //Reserved: 4 bit
        }
        else
        	header[0] = bytefieldSet(header[0],0,0,5);                 //Reserved: 5 bit

        header[1] = bytefieldSet(0,eType, 0, 8);           //Primitive: 1 byte
        header[2] = (0>>8) & 0xff;                                 //Payload length: 2 bytes
        header[3] = 0 & 0xff;
        header[4] = (pFloorInfo->conferenceID >> 24) & 0xff;  //ConferenceID: 4 bytes
        header[5] = (pFloorInfo->conferenceID >> 16) & 0xff;
        header[6] = (pFloorInfo->conferenceID >> 8) & 0xff;
        header[7] = pFloorInfo->conferenceID & 0xff;
        header[8] = (pFloorInfo->transactionID >> 8) & 0xff;  //TransactionID: 2 bytes
        header[9] = pFloorInfo->transactionID & 0xff;
        header[10] = (pFloorInfo->userID >> 8) & 0xff;        //UserID: 2 bytes
        header[11] = pFloorInfo->userID & 0xff;
        length = 12;
    }

    return length;
}

//---------------------------------------------------------
// UpdateBFCPCommonHeaderPayloadLength
//
// Update the common header length field with the value passed in
//---------------------------------------------------------
Boolean UpdateBFCPCommonHeaderPayloadLength(BFCPCommonHeader_T *pHeader, int length)
{
    UINT8 *header = (UINT8*)( (char*)pHeader) ;

    if( (pHeader && length < 0) || header==NULL ) //shira
        return false;

    header[2] = (length >> 8) & 0xff;
    header[3] = length & 0xff;

    return true;
}

//---------------------------------------------------------
// BuildBFCPAttr
//
// Build one of the BFCP attributes
//---------------------------------------------------------
int BuildBFCPAttr(void *pHeader, eBFCPAttributeID eType, int Mandatory, int id, int Status, int QueuePosition)
{
    int length = 0;
    UINT8 *header = (UINT8*)( (char*)pHeader) ;
    if (pHeader && eType > kBFCPAttrInvalid && eType < kBFCPAttrMaxAttribute)
    {
        length = 4;  // For now the only messages we support sending are of length 4
        header[0] = bytefieldSet(0,eType,1,7);          // AttrType: 7 bit
        header[0] = bytefieldSet(header[0],Mandatory,0,1);      // Mandatory:1 bit
        header[1] = bytefieldSet(0,length, 0, 8);       // Length: 1 byte
        if(eType == kBFCPAttrRequestStatus)
        {
            header[2] = bytefieldSet(0,Status,0,8); // Status: 1 byte
            header[3] = bytefieldSet(0,QueuePosition,0,8); // Queueposition: 1 byte
#ifdef BFCP_DEBUG
            printf("  %s: %s QueuePosition:%d\n", gBFCPAttrString[eType], gBFCPStatusString[Status], QueuePosition);
#endif
        }
        else
        {
            header[2] = (id >> 8) & 0xff;                     // floorId or FloorReqID: 2 bytes
            header[3] = id & 0xff;
#ifdef BFCP_DEBUG
            switch (eType)
            {
                case kBFCPAttrFloorRequestInfo:
                case kBFCPAttrOverallRequestStatus:
                    printf("  %s\n", gBFCPAttrString[eType]);
                    printf("    floorRequestID:%d\n", id);
                    break;

                case kBFCPAttrFloorRequestStatus:
                    printf("  %s\n", gBFCPAttrString[eType]);
                    printf("    floorID:%d\n", id);
                    break;

                default:
                    printf("  %s: %d\n", gBFCPAttrString[eType], id);
                    break;
            }
#endif
        }
    }

    return length;
}

//---------------------------------------------------------
// ParseBFCPCommonHeader
//
// retrieve the values from the common header
//---------------------------------------------------------
int ParseBFCPCommonHeader(void *buf, BFCPCommonHeader_T *pHeader)
{
    UINT8 *header = (UINT8*)( (char*)buf) ;
    if(!buf || !pHeader)
    {
        printf("Null buffer or null header pointer passed\n");
        return false;
    }

    pHeader->Version =  bytefieldGet(header[0],5,3);
    pHeader->Reserved = bytefieldGet(header[0],0,5);
    pHeader->Primitive = bytefieldGet(header[1],0,8);
    pHeader->PayloadLength = header[2] << 8;
    pHeader->PayloadLength |= header[3];
    pHeader->ConferenceID  = (header[4] << 24);
    pHeader->ConferenceID |= (header[5] << 16);
    pHeader->ConferenceID |= (header[6] << 8);
    pHeader->ConferenceID |= (header[7]);
    pHeader->TransactionID = (header[8] << 8);
    pHeader->TransactionID |= header[9];
    pHeader->UserID  = header[10] << 8;
    pHeader->UserID |= header[11];

#ifdef BFCP_DEBUG
    printf("SIP Parse BFCP %s\n", gBFCPPrimitiveString[pHeader->Primitive]);
	printf("  Transaction ID: %d\n", pHeader->TransactionID);
	printf("  User ID: %d\n", pHeader->UserID);
	printf("  Conference ID: %d\n", pHeader->ConferenceID);
#endif
    return 12;  //Return fixed length of header, 12 bytes
}

//---------------------------------------------------------
// getBaseAttr
//
// Get the first three header fields from an attribute
//---------------------------------------------------------
void getBaseAttr(UInt8 * pMsg, eBFCPAttributeID *pType, int *pManditory, int *pLength)
{
    *pType  = bytefieldGet(pMsg[0],1,7);
    *pManditory = bytefieldGet(pMsg[0],0,1);
    *pLength = bytefieldGet(pMsg[1], 0, 8);
}


//---------------------------------------------------------
// BFCPDumpMsg
//
// Debug routine used to dump the BFCP message
//---------------------------------------------------------
void BFCPDumpMsg(UInt8 * pMsg, int len)
{
    int i;

    for (i=0; i < len; i++)
    {
        printf("%.2x",pMsg[i]);
        if ((i+1)%8 == 0)
            printf("\n");
    }

    printf("\n");

}
//---------------------------------------------------------
//               External Functions
//---------------------------------------------------------
//---------------------------------------------------------
// BFCPInit
//
// Initialize the BFCP Parameters
//---------------------------------------------------------
void BFCPInit(BFCPObject_T *pBFCP)
{
    pBFCP->port = 0;
    pBFCP->listenSockfd = 0;
    pBFCP->setup = kSetupActive;
    pBFCP->connection = kConnectionNew;
    pBFCP->TCPTimeOut = 0;
    pBFCP->signalType = kBFCP_Invalid;
    pBFCP->initialOfferSignalType = kBFCP_Invalid;
    pBFCP->streamLabel = 0;
    pBFCP->lastFakeRoleProvIDTime = 0;
    pBFCP->blockFloorStatus = false;
    pBFCP->helloSent = false;
    pBFCP->floorCtrlType = kBFCPFloorCtrlInvalid;
    pBFCP->floorInfo.conferenceID = 0;
    pBFCP->floorInfo.userID = 0;
    pBFCP->floorInfo.transactionID = 1;
    pBFCP->floorInfo.floorRequestID = 0;
    pBFCP->floorInfo.floorID = 0;
    pBFCP->floorInfo.floorStatus = kBFCPStatusInvalid;
    pBFCP->floorInfo.errorCode = 0;
}

//---------------------------------------------------
// BFCPSetMsgHandler
//
// Set the message handler to call when a BFCP message
// arrives
//---------------------------------------------------
int BFCPSetMsgHandler(BFCPObject_T *pBFCP, void *pUserDefined, BFCPMsgEvent msgEvent)
{
    pBFCP->msgRcvdCallback = msgEvent;
    pBFCP->pUserData = pUserDefined;

    return 1;
}


int BFCPSetConnHandler(BFCPObject_T *pBFCP, void *pUserDefined, BFCPMsgEvent msgEvent)
{
    pBFCP->connFailCallback = msgEvent;
    pBFCP->pUserData = pUserDefined;

    return 1;
}
int BFCPSetConnUpHandler(BFCPObject_T *pBFCP, void *pUserDefined, BFCPMsgEvent msgEvent)
{
    pBFCP->connUpCallback = msgEvent;
    pBFCP->pUserData = pUserDefined;

    return 1;
}

//---------------------------------------------------
// BFCPCreateServerSocket
//
// Create a server socket and return the socket ID.
//---------------------------------------------------
int BFCPCreateServerSocket(BFCPObject_T *pBFCP, char * pAddress)
{
    struct sockaddr_in   serv_addr;
    unsigned int addrLen = sizeof(struct sockaddr_in);
    int rc;

    /* initialize structures for binding to listen port */
    memset ((void *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family   = AF_INET;
    // Only the active adapter/address should be used
    serv_addr.sin_addr.s_addr = inet_addr(pAddress);
    serv_addr.sin_port = 0;

    if ((pBFCP->listenSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        printf("BFCPCreateServerSocket Could not open a server socket\n");
        return 0;
    }

    if ((rc=bind(pBFCP->listenSockfd, (struct sockaddr *) &serv_addr, addrLen)) < 0)
    {
        printf("BFCPCreateServer Socket couldn't bind TCP to local address.\n");
        closesocket(pBFCP->listenSockfd);
        return 0;
    }

    //Retrieve the port which was used
    getsockname(pBFCP->listenSockfd, (struct sockaddr *)&serv_addr, &addrLen);
    pBFCP->port = ntohs(serv_addr.sin_port);

    listen(pBFCP->listenSockfd,5);
    printf("BFCPCreateServer Socket listening on port %d\n", pBFCP->port);
    return (1);
}

//---------------------------------------------------
// BFCPCloseServerSocket
//
// Close the server socket
//---------------------------------------------------
int BFCPCloseServerSocket(BFCPObject_T *pBFCP)
{
    if (pBFCP->listenSockfd != 0)
    {
        shutdown(pBFCP->listenSockfd, SD_BOTH);
        closesocket(pBFCP->listenSockfd);
        printf("BFCPCloseServer Socket port %d\n", pBFCP->port);
    }
    pBFCP->listenSockfd = 0;


    return 1;
}


//---------------------------------------------------
// BFCPPollForData
//
// BFCP Poll socket for data.  Call callbacks as appropriate
//---------------------------------------------------
void BFCPPollForData(BFCPObject_T *pBFCP)
{
    int yes = true;
    int no = false;
    int msgLen = 256;
    unsigned char msg[256];
    int result = 0;
    fd_set readfs;
    fd_set writefs;
    fd_set exceptfs;
    int bytesRead = 0;
    struct sockaddr_in   cli_addr;
    unsigned int clilen = 0;
    int numSockets = 0;
    timeval timeout;

    timeout.tv_sec  = 0;
    timeout.tv_usec  = 0;

    //Check for a new connection on the server socket or new data coming in
    //check for data on active sockets.
    FD_ZERO(&readfs);
    FD_ZERO(&writefs);
    FD_ZERO(&exceptfs);

    //Add the server socket to the list
    if (pBFCP->listenSockfd != 0)
    {
        FD_SET(pBFCP->listenSockfd,&readfs);
        if (pBFCP->listenSockfd >= numSockets)
        {
            numSockets = pBFCP->listenSockfd + 1;
        }
    }
    if (pBFCP->sockfd != 0)
    {
        FD_SET(pBFCP->sockfd,&readfs);
        if (pBFCP->sockfd >= numSockets)
        {
            numSockets = pBFCP->sockfd + 1;
        }
    }

    if (numSockets > 0)
    {
        if((result = select(numSockets, &readfs, &writefs, &exceptfs,&timeout)) <= 0)
        {
            return;
        }

        //Check for a new connection coming in.
        if ((pBFCP->listenSockfd != 0) && (FD_ISSET(pBFCP->listenSockfd,&readfs)))
        {
            int newSockfd;
            clilen = sizeof(cli_addr);
            newSockfd = accept(pBFCP->listenSockfd,(struct sockaddr *) &cli_addr, &clilen);

            if(newSockfd <= 0)
            {
                printf("BFCPPollForData: Failed: Accept on a TCP socket failed\n");
            }
            else
            {
                //if (_stricmp(inet_ntoa(cli_addr.sin_addr), pSipNic->proxyRegistrar.registration.proxyAddress) != 0)
                if (pBFCP->sockfd != 0)
                    closesocket(newSockfd);
                else
                {
                    pBFCP->sockfd = newSockfd;
                    printf("BFCPPollForData - new Socket %d connected to %s:%d\n", pBFCP->sockfd, pBFCP->remoteAddr, pBFCP->port);
                }
            }
        }

        if ((pBFCP->sockfd != 0) && (FD_ISSET(pBFCP->sockfd,&readfs)))
        {
            bytesRead = recv(pBFCP->sockfd,msg,msgLen,0);
            if (bytesRead > 0)
            {
                BFCP_ParseMsgPrimitive(pBFCP, msg, bytesRead);
            }
            else
            {
                if (bytesRead == 0)
                {
                    printf("BFCPPollForData:Remote endpoint did a graceful socket shutdown, shutting down our side, SOCK: %d\n",
                        pBFCP->sockfd);
                    printf("BFCPPollForData:Resetting NIC socket %d because it was closed.\n",pBFCP->sockfd);
                    pBFCP->sockfd = 0;
                }
                else
                {
                    //If no data read, then change the state of the socket.
                    printf("BFCPPollForData:read select on socket %d failed\n",pBFCP->sockfd);
                    pBFCP->sockfd = 0;
                }
            }
        }

#ifdef WIN32
        if ((pBFCP->sockfd != 0) && (FD_ISSET(pBFCP->sockfd,&exceptfs)))
        {
            int NetError = 0;
            NetError = WSAGetLastError();
            printf("BFCPPollForData:write select on socket %d failed, error: %d\n",
                pBFCP->sockfd, NetError);
            closesocket(pBFCP->sockfd);
            pBFCP->sockfd = 0;
        }pMsgLen
#endif
    }
}


//---------------------------------------------------------
// BFCPCreateConnection
//
// Connect to the remote BFCP endpoint
//---------------------------------------------------------
int BFCPCreateConnection(BFCPObject_T *pBFCP, char * pLocalAddress)
{
    struct sockaddr_in   remote_addr;
    struct sockaddr_in   serv_addr;
    struct linger Linger;
    int yes = true;
    int no = false;

    /* initialize structures for binding to listen port */
    memset ((void *) &remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family    = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(pBFCP->remoteAddr);
    remote_addr.sin_port   = htons(pBFCP->port);

    memset ((void *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family   = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(pLocalAddress);
    serv_addr.sin_port     = htons(pBFCP->port);

    if ((pBFCP->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        printf("BFCP Client : Could not open stream socket.\n");
        return (0);
    }
    setsockopt(pBFCP->sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes));
    Linger.l_onoff = true;
    Linger.l_linger = 0;
    setsockopt(pBFCP->sockfd, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));

    //Set TCP_NODELAY/No nagle algorithm.
    setsockopt(pBFCP->sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));
    //ioctlsocket(pBFCP->sockfd,FIONBIO,(unsigned long *)&yes);

    if (bind(pBFCP->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        closesocket(pBFCP->sockfd);
        pBFCP->sockfd = 0;
        printf("BFCPCreateConnection - Client couldn't bind to local address.\n");
    }

    if (connect(pBFCP->sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        int errval = getSocketError((long)pRemoteAddr->dSockFd);
#ifdef WIN32
        if (errval !=WSAEWOULDBLOCK)
#else
            if ((errval != EWOULDBLOCK) && (errval != EINPROGRESS))
#endif // WIN32
            {
                printf("BFCPCreateConnection - Socket %d connection failed to  %s:%d err=%d\n",
                    pBFCP->sockfd, pBFCP->remoteAddr, pBFCP->port, errval);
                closesocket(pBFCP->sockfd);
                pBFCP->sockfd = 0;
                return 0;
            }
    }

    printf("BFCPCreateConnection - Socket %d connected to %s:%d\n", pBFCP->sockfd, pBFCP->remoteAddr, pBFCP->port);

    return 1;
}

//---------------------------------------------------------
// BFCPDisconnect
//
// Disconnect and close any sockets having to do with BFCP
//---------------------------------------------------------
int BFCPDisconnect(BFCPObject_T *pBFCP)
{
    BFCPCloseServerSocket(pBFCP);
    if (pBFCP->sockfd != 0)
    {
        printf("BFCPDisconnect - close client Socket %d\n", pBFCP->sockfd);
        shutdown(pBFCP->sockfd, SD_BOTH);
        closesocket(pBFCP->sockfd);
        pBFCP->sockfd = 0;
    }

    return 1;
}

//---------------------------------------------------------
// BFCPSendMsg
//
// Send a BFCP message to the remote endpoint
//---------------------------------------------------------
int BFCPSendMsg(BFCPObject_T *pBFCPObject, unsigned char * pMsg, int len)
{
    int sentLen = 0;

    if (pBFCPObject->sockfd != 0)
    {
        if ((sentLen = send(pBFCPObject->sockfd, pMsg, len, 0)) != len)
            printf("BFCPSendMsg - Unable to send message to remote\n");
    }

    return sentLen;
}

//---------------------------------------------------------
//    BFCP_BuildHello - function to build a BFCP
//        hello message to make sure the server is alive
//---------------------------------------------------------
void BFCP_BuildHello(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen)
{
    *pMsgLen = 0;
    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
//        *pMsg = pBFCPMsg;

        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPHello, false);
        if (*pMsgLen > 0)
        {
            //    Increment the transaction ID.
            pFloorInfo->transactionID++;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
				printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorID, 1,pFloorInfo->floorID,0,0);
            *pMsgLen += payloadLen;
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}


//---------------------------------------------------------
//    BFCP_BuildHelloAck - function to build a BFCP
//        helloAck message to respond to a hello message
//---------------------------------------------------------
void BFCP_BuildHelloAck(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen, char * pRemoteProduct, eBFCPSignalType bfcpTransType)
{
    *pMsgLen = 0;

    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
        int i, length, crntByte, padBytes;
        // *pMsg = pBFCPMsg;

        Boolean bIsToSetIBit = false;

        if (bfcpTransType == kBFCP_UDP)
        	bIsToSetIBit = true;

        // Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPHelloAck, bIsToSetIBit);
        if (*pMsgLen > 0)
        {
            //    Increment the transaction ID.
            //pFloorInfo->transactionID++;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
            	printf("Warning: BFCP Message too long\n");
            	pMsg = NULL;
            	return;
            }

            //Build Supported-Primitives - we support everything but chair action and chair action ack
            pHeader = pMsg + *pMsgLen;
            pHeader[0] = bytefieldSet(0,kBFCPAttrSupportedPrimitives,1,7);          // AttrType: 7 bit
            pHeader[0] = bytefieldSet(pHeader[0],1,0,1); // Mandatory bit
            length = crntByte= 2;
            for (i = kBFCPFloorRequest; i < kBFCPMaxPrimitive; i++)
            {
                if ((i != kBFCPChairAction) && (i != kBFCPChairActionAck))
                {
                    pHeader[crntByte++] = i;
                    length++;
                }
            }

            //pHeader[1] = bytefieldSet(0,length, 0, 8);

            //Pad it to be a word long
            if (length%4 != 0)
            {
                padBytes = 4 - length%4;
                for (i=0; i < padBytes; i++)
                    pHeader[crntByte++] = 0;

                length = length + 4 - length%4;
            }

            pHeader[1] = bytefieldSet(0,length, 0, 8);

            *pMsgLen += length;
            payloadLen += length;

            // Tandberg chokes on this
            //if ((pRemoteProduct != NULL) && (strstr (pRemoteProduct, "TANDBERG") == NULL))
            if ((pRemoteProduct == NULL) || ((pRemoteProduct != NULL) && (strstr (pRemoteProduct, "TANDBERG") == NULL)))
            {
                //Build Supported-Attributes - What the heck, let's say we support them all...
                pHeader = pMsg + *pMsgLen;
                pHeader[0] = bytefieldSet(0,kBFCPAttrSupportedAttrs,1,7);          // AttrType: 7 bit
                pHeader[0] = bytefieldSet(pHeader[0],1,0,1);      // Mandatory:1 bit
                length = crntByte= 2;
                for (i = kBFCPAttrBeneficiaryID; i < kBFCPAttrMaxAttribute; i++)
                {
                    pHeader[crntByte++] = i<<1;
                    length++;
                }

                //pHeader[1] = bytefieldSet(0,length, 0, 8);

                //Pad it to be a word long
                if (length%4 != 0)
                {
                    padBytes = 4 - length%4;
                    for (i=0; i < padBytes; i++)
                        pHeader[crntByte++] = 0;

                    length = length + 4 - length%4;
                }

                pHeader[1] = bytefieldSet(0,length, 0, 8);

                *pMsgLen += length;
                payloadLen += length;
            }

            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}

//---------------------------------------------------------
//    BFCP_BuildFloorRequest - function to build a BFCP
//        floor request message. This message is used to
//        request a content token. Only the FloorID attribute
//        is supported.
//---------------------------------------------------------
void BFCP_BuildFloorRequest(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen)
{
    *pMsgLen = 0;
    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
        //*pMsg = pBFCPMsg;

        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPFloorRequest, false);
        if (*pMsgLen > 0)
        {
            //    Increment the transaction ID.
            pFloorInfo->transactionID++;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorID, 1,pFloorInfo->floorID,0,0);
            *pMsgLen += payloadLen;
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}

//---------------------------------------------------------
//    BFCP_BuildFloorRelease - function to build a BFCP
//        floor release message. This message is used to
//        release a content token. Only a single FloorID attribute
//        is supported.
//---------------------------------------------------------
void BFCP_BuildFloorRelease(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen)
{
    *pMsgLen = 0;

    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
        //*pMsg = pBFCPMsg;
        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPFloorRelease, false);
        if (*pMsgLen > 0)
        {
            pFloorInfo->transactionID++;
            pHeader = pMsg + *pMsgLen;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorRequestID, 1,
                pFloorInfo->floorRequestID,0,0);
            *pMsgLen += payloadLen;
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}

//---------------------------------------------------------
//    BFCP_BuildFloorStatus - function to build a BFCP
//        floor status message. This message is used to send the
//        token owner notification message. Only the FLOOR-ID
//        attribute is supported but you can supply any number
//        of them.
//---------------------------------------------------------
void BFCP_BuildFloorStatus(BFCPFloorInfoT *pFloorInfo, eBFCPStatusValues status, UInt8 *pMsg, UInt32 *pMsgLen, eBFCPSignalType bfcpTransType, Boolean bIsServerResponse)
{
    *pMsgLen = 0;

    if (pFloorInfo)
    {
        UInt8 * pHeader;
        int payloadLen = 0, hdrLen = 0;

        //*pMsg = pBFCPMsg;
        //    Initialize the message components.

        Boolean bIsToSetIBit = false;

		if (bfcpTransType == kBFCP_UDP && bIsServerResponse)
			bIsToSetIBit = true;

        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPFloorStatus, bIsToSetIBit);
        if (*pMsgLen > 0)
        {
            UInt8 *pFloorReqInfoHdr, *pOverallReqStatusHdr;
            int floorReqInfoLen = 0;
            int overallReqInfoLen = 0;

            //Build Floor-ID
            pHeader = pMsg + *pMsgLen;
            payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorID, 1, pFloorInfo->floorID,0,0);
            *pMsgLen += payloadLen;

            //Build Floor-Request-Info
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pFloorReqInfoHdr = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pFloorReqInfoHdr, kBFCPAttrFloorRequestInfo, 1, pFloorInfo->floorRequestID, 0, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Build Overall-Request-Status
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pOverallReqStatusHdr = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pOverallReqStatusHdr, kBFCPAttrOverallRequestStatus, 1, pFloorInfo->floorRequestID, 0, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            overallReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Build Request Status
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pHeader, kBFCPAttrRequestStatus, 1, 0, status, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            overallReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Build Floor-Request-Status
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorRequestStatus, 1, pFloorInfo->floorID, 0, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            overallReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Update the grouped attributes length
            pFloorReqInfoHdr[1] = floorReqInfoLen;
            pOverallReqStatusHdr[1] = overallReqInfoLen;

            //update the payloadlength field of BFCPCommonHeader
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}

void BFCP_BuildFloorStatusAck(BFCPFloorInfoT *pFloorInfo, eBFCPStatusValues status,UInt8 *pMsg, UInt32 *pMsgLen)
{
    *pMsgLen = 0;
    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
//        *pMsg = pBFCPMsg;

        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPFloorStatusAck, false);

        if (*pMsgLen > 0)
        {
            //    Increment the transaction ID.
            pFloorInfo->transactionID++;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
            	printf("Warning: BFCP_FloorStatusAck Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorID, 1,pFloorInfo->floorID,0,0);
            *pMsgLen += payloadLen;
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}

//---------------------------------------------------------
//    BFCP_BuildFloorRequestStatus - function to build a BFCP
//        floor request status message. This message is used to
//        notify the token requester of the token request status
//        Accepted/Granted/Pending/Released
//---------------------------------------------------------
void BFCP_BuildFloorRequestStatus(BFCPFloorInfoT *pFloorInfo, eBFCPStatusValues status, UInt8 *pMsg, UInt32 *pMsgLen, eBFCPSignalType bfcpTransType, Boolean bIsServerResponse)
{
    *pMsgLen = 0;

    if (pFloorInfo)
    {
        UInt8 * pHeader;
        int payloadLen = 0, hdrLen = 0;
        //    Allocate the complete message.
        //*pMsg = pBFCPMsg;

        Boolean bIsToSetIBit = false;

		if (bfcpTransType == kBFCP_UDP && bIsServerResponse)
			bIsToSetIBit = true;

        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPFloorRequestStatus, bIsToSetIBit);
        if (*pMsgLen > 0)
        {
            UInt8 *pFloorReqInfoHdr, *pOverallReqStatusHdr;
            int floorReqInfoLen = 0;
            int overallReqInfoLen = 0;

            //Build Floor-Request-Info
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pFloorReqInfoHdr = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pFloorReqInfoHdr, kBFCPAttrFloorRequestInfo, 1, pFloorInfo->floorRequestID, 0, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Build Overall-Request-Status
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pOverallReqStatusHdr = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pOverallReqStatusHdr, kBFCPAttrOverallRequestStatus, 1, pFloorInfo->floorRequestID, 0, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            overallReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Build Request Status
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pHeader, kBFCPAttrRequestStatus, 1, 0, status, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            overallReqInfoLen += hdrLen;
            payloadLen += hdrLen;

           //Build Floor-Request-Status
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pMsg = NULL;
                return;
            }
            pHeader = pMsg + *pMsgLen;
            hdrLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorRequestStatus, 1, pFloorInfo->floorID, 0, 0);
            *pMsgLen += hdrLen;
            floorReqInfoLen += hdrLen;
            overallReqInfoLen += hdrLen;
            payloadLen += hdrLen;

            //Update the grouped attributes length
            pFloorReqInfoHdr[1] = floorReqInfoLen;
            pOverallReqStatusHdr[1] = overallReqInfoLen;

            //update the payloadlength field of BFCPCommonHeader
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
        }
    }
}

//---------------------------------------------------------
//    BFCP_BuildGoodbye - function to build a BFCP
//        goodbye message. This message is used to
//        initiate goodbye request from remote
//---------------------------------------------------------
void BFCP_BuildGoodbye(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen)
{
	*pMsgLen = 0;

	if (pFloorInfo)
	{
		UInt8 * pHeader;
		unsigned int payloadLen = 0;
//        *pMsg = pBFCPMsg;

		//    Initialize the message components.
		*pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPGoodbye, false);
		if (*pMsgLen > 0)
		{
			//    Increment the transaction ID.
//			pFloorInfo->transactionID++;
			if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
			{
				printf("Warning: BFCP Message too long\n");
				pMsg = NULL;
				return;
			}
			pHeader = pMsg + *pMsgLen;
			payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorID, 1,pFloorInfo->floorID,0,0);
			*pMsgLen += payloadLen;
			UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
		}
	}
}

//---------------------------------------------------------
//    BFCP_BuildGoodbyeAck - function to build a BFCP
//        goodbye ack message. This message is used to
//        ack on goodbye request from remote
//---------------------------------------------------------
void BFCP_BuildGoodbyeAck(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen, char * pRemoteProduct, eBFCPSignalType bfcpTransType)
{
	*pMsgLen = 0;

	if (pFloorInfo)
	{
		UInt8 * pHeader;
		unsigned int payloadLen = 0;
//        *pMsg = pBFCPMsg;

		Boolean bIsToSetIBit = false;

		if (bfcpTransType == kBFCP_UDP)
			bIsToSetIBit = true;

		//    Initialize the message components.
		*pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pMsg, pFloorInfo, kBFCPGoodbyeAck, bIsToSetIBit);
		if (*pMsgLen > 0)
		{
			if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
			{
				printf("Warning: BFCP Message too long\n");
				pMsg = NULL;
				return;
			}
			pHeader = pMsg + *pMsgLen;
			payloadLen = BuildBFCPAttr(pHeader, kBFCPAttrFloorID, 1,pFloorInfo->floorID,0,0);
			*pMsgLen += payloadLen;
			UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pMsg, payloadLen/4);
		}
	}
}
//---------------------------------------------------------
//    BFCP_ParseMsgPrimitive - function to return the BFCP
//        message primitive from a supplied BFCP message
//---------------------------------------------------------
void BFCP_ParseMsgPrimitive(BFCPObject_T *pBFCP, UInt8 *pMsg, int msgLen)
{
    eBFCPPrimitive primitive = kBFCPInvalidPrimitive;
    BFCPFloorInfoT floorInfo;
    memset(&floorInfo,0,sizeof(BFCPFloorInfoT));

    if (pMsg)
    {
        BFCPCommonHeader_T BFCPHeader;

        int offset 	= 0;
        int primLen = 0;
        int attrLen, i;

        eBFCPAttributeID type;

        int manditory, length;

        UInt8 *pAttr;

        while (msgLen > 0)
        {
            pMsg += offset;
            //Retrieve the primitive from the common header.
            offset = ParseBFCPCommonHeader(pMsg, &BFCPHeader);

            floorInfo.userID 		= BFCPHeader.UserID;
            floorInfo.conferenceID  = BFCPHeader.ConferenceID;
            floorInfo.transactionID = BFCPHeader.TransactionID;

            primitive 	= BFCPHeader.Primitive;

            if (primitive == 0x5b)
            {
            	BFCPHeader.Primitive 	= kTandbergBFCPFloorRequestStatusAck;
            	primitive 				= BFCPHeader.Primitive;
            }
            else if  (primitive == 0x5d)
            {
            	BFCPHeader.Primitive 	= kTandbergBFCPFloorStatusAck;
            	primitive 				= BFCPHeader.Primitive;
            }

            primLen 	= BFCPHeader.PayloadLength*4;

            //Walk through all the attributes and fill in the known values.
            while (primLen > 0)
            {
                pAttr = pMsg + offset;
                getBaseAttr(pAttr, &type, &manditory, &length);
                attrLen = length;
                switch (type)
                {
                case kBFCPAttrFloorID:
                    floorInfo.floorID  = pAttr[2] << 8;
                    floorInfo.floorID |= pAttr[3];
#ifdef BFCP_DEBUG
                    printf("  %s: %d\n", gBFCPAttrString[type], floorInfo.floorID);
#endif
                    break;

                case kBFCPAttrFloorRequestID:
                    floorInfo.floorRequestID  = pAttr[2] << 8;
                    floorInfo.floorRequestID |= pAttr[3];
#ifdef BFCP_DEBUG
                    printf("  %s\n",gBFCPAttrString[type]);
                    printf("    floorRequestID %d\n",  floorInfo.floorRequestID);
#endif
                    break;

                case kBFCPAttrRequestStatus:
                    floorInfo.floorStatus = pAttr[2];
#ifdef BFCP_DEBUG
                    printf("  %s\n",gBFCPAttrString[type]);
                    printf("    %s QueuePosition:%d\n", gBFCPStatusString[floorInfo.floorStatus], pAttr[3]);
#endif
                    break;

                case kBFCPAttrFloorRequestStatus:
                    floorInfo.floorID  = pAttr[2] << 8;
                    floorInfo.floorID |= pAttr[3];
                    attrLen = 4;  // This is a grouped attribute, do not skip over the rest of the attributes
#ifdef BFCP_DEBUG
                    printf("  %s\n",gBFCPAttrString[type]);
                    printf("    floorID %d\n", floorInfo.floorID);
#endif
                    break;

                case kBFCPAttrErrorCode:
                    floorInfo.errorCode = pAttr[2];
#ifdef BFCP_DEBUG
                    printf("  %s\n",gBFCPAttrString[type]);
                    printf("    error-code %d\n", floorInfo.errorCode);
#endif
                    break;


                case kBFCPAttrOverallRequestStatus:
                case kBFCPAttrFloorRequestInfo:
                    floorInfo.floorRequestID  = pAttr[2] << 8;
                    floorInfo.floorRequestID |= pAttr[3];
                    attrLen = 4;  // This is a grouped attribute, do not skip over the rest of the attributes
#ifdef BFCP_DEBUG
                    printf("  %s\n",gBFCPAttrString[type]);
                    printf("    floorRequestID %d\n", floorInfo.floorRequestID);
#endif
                    break;

                case kBFCPAttrSupportedAttrs:
                   printf("\n");
                   printf("      ");
                    for (i=2; i<attrLen; i++)
                    {
                        // Check index, there may be unknown attributes
                        if (((pAttr[i]&0xfe)>>1) < kBFCPAttrMaxAttribute)
                        {
                       printf(" %s", gBFCPAttrString[(pAttr[i]&0xfe)>>1]);
                    }
                        else
                        {
                            printf(" Unknown");
                        }
                    }
                   printf("\n");

                    //Account for padding
                    if ((attrLen%4) != 0)
                        attrLen = attrLen + 4 - attrLen%4;
                    break;

                case kBFCPAttrSupportedPrimitives:
#ifdef BFCP_DEBUG
                   printf("\n");
                   printf("      ");
                    for (i=2; i<attrLen; i++)
                    {
                        // Check index, there may be unknown primitives
                        if (pAttr[i] < kBFCPMaxPrimitive)
                        {
                            printf(" %s", gBFCPPrimitiveString[pAttr[i]]);
                        }
                        else
                        {
                            printf(" Unknown");
                        }
                    }
                   printf("\n");
#endif
                    //Account for padding
                    if ((attrLen%4) != 0)
                        attrLen = attrLen + 4 - attrLen%4;
                    break;

                default:
                	printf(" Unsupported");
                    break;

                }
                offset += attrLen;
                primLen -= attrLen;
                if (attrLen == 0)
                    break;
            }
            msgLen -= offset;
            if (pBFCP->msgRcvdCallback)
                (pBFCP->msgRcvdCallback)(pBFCP->pUserData, &floorInfo, primitive);
        }
    }
}
