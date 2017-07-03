// ShelfXmlConvertor.cpp

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "TaskApi.h"
#include "InitCommonStrings.h"
#include "XmlMiniParser.h"
#include "ApiStatuses.h"
#include "ObjString.h"
#include "NStream.h"
#include "psosxml.h"
#include "Trace.h"
#include "TraceStream.h"
//#include "McuMngrInternalStructs.h"
#include "ApacheModuleEngine.h"
#include "ShelfXmlConvertor.h"

#define MAX_STRING_PARAM    	24
#define MAX_PARAMS_IN_STRING	128
////////////////////////////////////////////////////////////////////////////////

// IpmiRequest.h
/////////////////////////////////////////////////////////////
//Ipmi Request Types
#define IPMI_ENTITY_LIST_REQ		 512
#define IPMI_FRU_REQ				 513
#define IPMI_SENSOR_LIST_REQ		 514
#define IPMI_SENSOR_READING_LIST_REQ 515
#define IPMI_FAN_INFO_REQ			 516
#define IPMI_GET_FAN_LEVEL_REQ		 517
#define IPMI_SET_FAN_LEVEL_REQ		 518
#define IPMI_GET_LED_INFO_REQ		 519
#define IPMI_GET_LED_STATE_REQ		 520
#define IPMI_DO_RESET_REQ			 521
#define IPMI_GET_EVENT_LOG_REQ		 522

//Ipmi Status Types
/////////////////////////////////////////////////////////////
#define IPMI_CARD_STATUS_NORMAL		 		 0
#define IPMI_CARD_STATUS_MAJOR				 1
#define IPMI_CARD_STATUS_EMPTY				 2
#define IPMI_CARD_STATUS_RESETTING			 3
#define IPMI_CARD_STATUS_DIAGNOSTICS		 4


#define IPMI_SLOT_ID_TYPE_RMX_2000			0
#define IPMI_SLOT_ID_TYPE_MFA_1				1
#define IPMI_SLOT_ID_TYPE_MFA_2				2
#define IPMI_SLOT_ID_TYPE_CPU				3
#define IPMI_SLOT_ID_TYPE_IMA				4
#define IPMI_SLOT_ID_TYPE_SWITCH			5

// IpmiRequest.h
/////////////////////////////////////////////////////////////
//LanInfo Request Types
#define LAN_STAT_INFO_REQ			256
#define LAN_STAT_CLEAR_MAX_REQ		257
#define LAN_STAT_GET_PORTS_LIST_REQ 258

#define LAN_STAT_INFO_IND			256
#define LAN_STAT_CLEAR_MAX_IND		257
#define LAN_STAT_GET_PORTS_LIST_IND 258

#define PRIVATE_OPCODE				0x401

#define STR_AUTH_ADMIN				"administrator"
#define STR_AUTH_ADMIN_READONLY	"administrator_readonly"

#define SUPER_NUM           0
#define ADMINISTRATOR_READONLY_NUM         6

/*
typedef struct SStructToStr
{
	UINT32 	varType;
	UINT32 	varCount;
	UINT32 	jumpOffst;
	INT8 	varString[32];
}TStructToStr,*PTStructToStr;


// Control response message
static TStructToStr atControlResponseMsgArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
};

static TStructToStr atIpmiEntityListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "SubBoardID"},
	{e_unsignedLong,   1,              4,        "IpmbAddress"},
	{e_string,		   1,   MAX_CARD_TYPE_STR_SIZE,   "CardType"},
	{e_unsignedLong,   1,              4,        "NumMezzanine"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_unsignedLong,   1,              4,        "Temperature"},
	{e_unsignedLong,   1,              4,        "Voltage"},
	{e_unsignedLong,   1,              4,        "BitFail"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "EntityId"},
	{e_unsignedLong,   1,              4,        "EntityInstance"},
	{e_unsignedLong,   1,              4,        "Present"},
};

//Lan Stat Get Ports List
static TStructToStr atLanStatGetPortsListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "PortID"},
	{e_unsignedLong,   1,              4,        "Status"},
};
*/

#define ADD_CHILD_TO_MSG(parent,element,value,response) \
if (parent!=NULL)\
{\
	nStatus = parent->GetAndVerifyChildNodeValue(element,value,pszError,ONE_LINE_BUFFER_LENGTH); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_NODE_MISSING && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus != STATUS_NODE_LENGTH_TOO_SHORT && \
       nStatus != STATUS_NODE_LENGTH_TOO_LONG && \
       nStatus != STATUS_VALUE_OUT_OF_RANGE && \
       nStatus != STATUS_IP_ADDRESS_INVALID) \
		response << "$" << element << "=";\
	else                                                    \
		response << "$" << element << "=" << value;\
}


void CShelfXmlConvertor::Initialize()
{

}

////////////////////////////////////////////////////////////////////////////////
void CShelfXmlConvertor::Clean()
{

}

// ParseLoginTrans parses only the 2 following login commands:
// 1)	UserName ???? UserPassword ???? StationName ????
// 2)	UserName ???? UserPassword ????
// ----------------------------------
// "Control$Opcode=1$MsgID=14$SlotID=-1$UserName=POLYCOM$UserPassword=asdfASDF1234!@#$$StationName=EMA.F3-KOBIG"
// "Control$Opcode=1$MsgID=14$SlotID=-1$UserName=POLYCOM$UserPassword=asdfASDF1234!@#$"
// Important: At the end (2 option) there is no delimiter.
int CShelfXmlConvertor::ParseLoginTrans(char *command, char *outCommandArgs[], int *outArgs, unsigned int *outMsgId)
{
    char *pcTokenMsgId = 0;
    char *pcValMsgId = 0;
    char  msgId[10];
    char *pcMgsIdEnd = 0;

    char *pcTokenUser = 0;
    char *pcTokenPassword = 0;
    char *pcValUser = 0;
    char *pcValPassword = 0;
    char *pcTokenStation = 0;
    char *pcValStation = 0;
    int   add_index = 0;
    char commandLen = strlen(command);


    // Message ID
    pcTokenMsgId = strstr(command, "MsgID=");
    if(NULL == pcTokenMsgId)
    {
        return -1;
    }
    pcValMsgId = pcTokenMsgId + strlen("MsgID=");
    pcMgsIdEnd = strstr(pcValMsgId, "$");
    memcpy(msgId, pcValMsgId, pcMgsIdEnd - pcValMsgId);
    msgId[pcMgsIdEnd - pcValMsgId] = '\0';
    *outMsgId = atoi(msgId);

    // User Name
    pcTokenUser = strstr(command, "UserName=");
    if(NULL == pcTokenUser)
    {
        return -1;
    }
    pcValUser = pcTokenUser + strlen("UserName=");

    // Password
    pcTokenPassword = strstr(command, "UserPassword=");
    if(NULL == pcTokenPassword)
    {
        return -1;
    }
    pcValPassword = pcTokenPassword + strlen("UserPassword=");

    // from this point ParseLoginTrans cannot return -1, command changed.
    pcValUser[pcTokenPassword - pcValUser - 1] = '\0';

    add_index = 1;
    pcTokenStation = strstr(pcValPassword, "StationName=");
    if(NULL == pcTokenStation)
    {
        pcTokenStation = command + commandLen;
        add_index = 0;
    }

    pcValPassword[pcTokenStation - pcValPassword - add_index] = '\0';

    outCommandArgs[0] = pcValUser;
    outCommandArgs[1] = pcValPassword;
     *outArgs = 2;

    if(1 == add_index) //has StationName
    {
        pcValStation = pcTokenStation + strlen("StationName=");
         outCommandArgs[2] = pcValStation;
        *outArgs = 3;
    }

    return 0;
}

int CShelfXmlConvertor::BuildEmaLoginMsg(char **commandArgs, int numArgs, unsigned int ulMsgId, int ulSlotId, char** szXMLRequest)
{
       const char *  format = "<TRANS_MCU><TRANS_COMMON_PARAMS><MCU_TOKEN>-1</MCU_TOKEN><MCU_USER_TOKEN>0</MCU_USER_TOKEN><MESSAGE_ID>%d</MESSAGE_ID></TRANS_COMMON_PARAMS><ACTION><LOGIN><MCU_IP><IP>127.0.0.1</IP><LISTEN_PORT>80</LISTEN_PORT><HOST_NAME/></MCU_IP><USER_NAME>%s</USER_NAME><PASSWORD>%s</PASSWORD><STATION_NAME>%s</STATION_NAME><COMPRESSION>true</COMPRESSION><CONFERENCE_RECORDER>false</CONFERENCE_RECORDER><NEW_PASSWORD/><HOTBACKUP_ACTUAL_TYPE>none</HOTBACKUP_ACTUAL_TYPE><CLIENT_IP/></LOGIN></ACTION></TRANS_MCU>";    
       char * pValStation = NULL;
       if(3 == numArgs && NULL != commandArgs[2])
       {
            pValStation = commandArgs[2];
       }
       else
       {
            pValStation = "StationName";            
       }
       
	int len = strlen(format)+strlen(commandArgs[0])+strlen(commandArgs[1])+strlen(pValStation);

	*szXMLRequest = new char[len + 128];

	sprintf(*szXMLRequest, format, ulMsgId, commandArgs[0], commandArgs[1], pValStation);
	return 0;
}

int CShelfXmlConvertor::BuildEmaIpmiMsg(unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId, char** szXMLRequest)
{

       if(IPMI_ENTITY_LIST_REQ == ulOpcode)
       {
               const char *  format = "<TRANS_IPMI_ENTITY_LIST><TRANS_COMMON_PARAMS><MCU_TOKEN>3</MCU_TOKEN><MCU_USER_TOKEN>3</MCU_USER_TOKEN><MESSAGE_ID>%d</MESSAGE_ID></TRANS_COMMON_PARAMS><ACTION><GET><SlotID>%d</SlotID><OBJ_TOKEN>-1</OBJ_TOKEN></GET></ACTION></TRANS_IPMI_ENTITY_LIST>";    
               
        	int len = strlen(format);

        	*szXMLRequest = new char[len+512];

        	sprintf(*szXMLRequest,format,ulMsgId, nSlotId);            
       }
       else
       {
            FTRACEINTO <<  "CShelfXmlConvertor::BuildEmaIpmiMsg: unsupport shelf msg convert. ulOpcode: " << ulOpcode <<" ulMsgId: " << ulMsgId;
       }

	return 0;
}

int CShelfXmlConvertor::BuildEmaLanInfoMsg(unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId, char** szXMLRequest)
{

       if(LAN_STAT_GET_PORTS_LIST_REQ == ulOpcode)
       {
               const char *  format = "<TRANS_LAN_PORT_LIST><TRANS_COMMON_PARAMS><MCU_TOKEN>2</MCU_TOKEN><MCU_USER_TOKEN>2</MCU_USER_TOKEN><MESSAGE_ID>%d</MESSAGE_ID></TRANS_COMMON_PARAMS><ACTION><GET><SlotID>%d</SlotID><OBJ_TOKEN>-1</OBJ_TOKEN></GET></ACTION></TRANS_LAN_PORT_LIST>";    
               
        	int len = strlen(format);

        	*szXMLRequest = new char[len+512];

        	sprintf(*szXMLRequest,format,ulMsgId, nSlotId);            
       }
       else
       {
            FTRACEINTO <<  "CShelfXmlConvertor::BuildEmaLanInfoMsg: unsupport shelf msg convert. ulOpcode: " << ulOpcode <<" ulMsgId: " << ulMsgId;
       }

	return 0;
}

void CShelfXmlConvertor::StringStripper(const char* commandRcvd, unsigned int NumBytes, char** szXMLRequest, unsigned int & ulOpcode, unsigned int & ulMsgId, int & nSlotId)
{
	int		i=0,Args=0;
	char	*pcToken,prString,*pstrString;
	char	*commandArgs[MAX_PARAMS_IN_STRING];
	char reqBuffer[MAX_PARAMS_IN_STRING * MAX_STRING_PARAM];
	unsigned int   ulReqSize=0;
	unsigned int*  ReqPtr = NULL;
	char *command=NULL;
	ulOpcode = 0;
	nSlotId = -1;
	ulMsgId = 0;
	int rc = 0;

	command = (char *)malloc(NumBytes + 1);
	if (command==NULL)
	{
		FTRACEINTO << "CShelfXmlConvertor::StringStripper: command==NULL StripperExit";
		goto StripperExit;
	}

	memcpy(command,commandRcvd,NumBytes);
	command[NumBytes]=0;
	//FTRACEINTO <<  "CShelfXmlConvertor::StringStripper: Entry: " << command;  //TODO: remove this Log before release
	
	if (command[0] == 0)
	{
		FTRACEINTO << "CShelfXmlConvertor::StringStripper: command[0] == 0 StripperExit";
		goto StripperExit;
	}

	memset(commandArgs,0,sizeof(commandArgs));

	 rc = -1;
     // login transaction, parse it differently because password can contain $ - delimiter
     if(NULL != strstr(command, "UserName=") && NULL != strstr(command, "UserPassword="))
     {
         rc = ParseLoginTrans(command, commandArgs, &Args, &ulMsgId);
         ulOpcode = 1;
         nSlotId = -1;
         
         FTRACEINTO << "CShelfXmlConvertor::StringStripper: ParseLoginTrans returned: "<< rc;
         
         if(-1 != rc)
         {
             BuildEmaLoginMsg(commandArgs, Args, ulMsgId, nSlotId, szXMLRequest);
             goto StripperExit;
         }
     }

     // not login OR login parser failed
     FTRACEINTO <<  "CShelfXmlConvertor::StringStripper: Entry: " << command;
     if(-1 == rc)
     {
         pcToken = strtok( command , "$" );
         while( pcToken != NULL )
         {
             commandArgs[i] = pcToken;
             pcToken = strtok( NULL, "$" );
             i++;
             if (i >= MAX_PARAMS_IN_STRING)
             {
                 FTRACEINTO << "Out of range - Over than Parameters : " << MAX_PARAMS_IN_STRING;
                 goto StripperExit;
             }
         }
         Args = i;
         
     }
     memset(reqBuffer, 0, sizeof(reqBuffer));
     ReqPtr = (unsigned int*)&reqBuffer[0];


	// NOTE: Copy all String Params into a buffer - assume they are all dwords !!!
	for(i = 1; i < Args; i++)
	{
		prString=0;
		if (!strcmp(commandArgs[0],"Ipmi"))
		{
			if ((i>4)&&(ulOpcode==IPMI_DO_RESET_REQ))
				prString=1;
		}
		if (prString)
		{
			if(NULL == (pstrString = strstr(commandArgs[i],"="))) continue;
			sscanf(pstrString,"=%23s",(char *)ReqPtr);
			ulReqSize += MAX_STRING_PARAM;
			ReqPtr[MAX_STRING_PARAM - 1]=0;
		}
		else
		{
			if(NULL == (pstrString = strstr(commandArgs[i],"="))) continue;
			sscanf(pstrString,"=%d",ReqPtr);
			ulReqSize += sizeof(unsigned int);
		}

		if(i == 1)
			ulOpcode = *ReqPtr;
		if(i == 2)
			ulMsgId = *ReqPtr;
		if(i == 3)
			nSlotId = *ReqPtr;
/*
		if (prString)
			FTRACEINTO <<  "(StringStripper): Param" << i <<" : " << ReqPtr;
		else
			FTRACEINTO <<  "(StringStripper): Param" << i <<" : " << *ReqPtr;
*/
		if (prString)
			ReqPtr = ReqPtr + MAX_STRING_PARAM/4;
		else ReqPtr++;
	}


	if(!strcmp(commandArgs[0],"LanInfo"))
	{
		BuildEmaLanInfoMsg(ulOpcode, ulMsgId, nSlotId, szXMLRequest);
	}
	else if(!strcmp(commandArgs[0],"Ipmi"))
	{
		BuildEmaIpmiMsg(ulOpcode, ulMsgId, nSlotId, szXMLRequest);
	}
	else if(!strcmp(commandArgs[0],"Control"))
	{
		BuildEmaLoginMsg(commandArgs, Args, ulMsgId, nSlotId, szXMLRequest);
	}
       else
       {
            FTRACEINTO <<  "CShelfXmlConvertor::StringStripper: unsupport shelf msg convert. ulOpcode: " << ulOpcode <<" ulMsgId: " << ulMsgId;
       }

StripperExit:

	free(command);

	return;
}

int CShelfXmlConvertor::BuildShelfGeneralMsg(int status, unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId, ostringstream & strShelfResponse)
{
    switch(ulOpcode)
    {
        case 1:
            strShelfResponse << "Control";
            break;
        case IPMI_ENTITY_LIST_REQ:
            strShelfResponse << "Ipmi";
            break;
        case LAN_STAT_GET_PORTS_LIST_REQ:
            strShelfResponse << "LanInfo";
            break;
        default:
             FTRACEINTO <<  "CShelfXmlConvertor::BuildShelfGeneralMsg: unsupport shelf msg convert. ulOpcode: " << ulOpcode <<" ulMsgId: " << ulMsgId;
    }

    strShelfResponse << "$Opcode=" << ulOpcode;
    strShelfResponse << "$MsgID=" << ulMsgId;
    strShelfResponse << "$SlotID=" << nSlotId;
    strShelfResponse <<	"$Status=" << status;
    strShelfResponse <<	"$Description=" << CProcessBase::GetProcess()->GetStatusAsString(status);
    return 0;
}

int CShelfXmlConvertor::GetShelfLoginMsgAuthGroup(char* szXmlResponse, int & authGroup)
{
    CXMLDOMDocument xmlDoc;
    authGroup = -1;
    HRES parseStatus = xmlDoc.Parse((const char**)&szXmlResponse);

    if(SEC_OK != parseStatus)
    {
        FTRACEINTO << "CShelfXmlConvertor::GetShelfLoginMsgAuthGroup: CXMLDOMDocument::Parse failed";
        return STATUS_FAIL;
    }

	char pszError[ERROR_MESSAGE_LEN];
	pszError[0] = '\0';
    int nStatus = STATUS_OK;
    CXMLDOMElement *pResponseRootNode = xmlDoc.GetRootElement();
    CXMLDOMElement *pActionNode = NULL;
    CXMLDOMElement *pLoginNode = NULL;

    GET_MANDATORY_CHILD_NODE(pResponseRootNode, "ACTION", pActionNode);
    GET_MANDATORY_CHILD_NODE(pActionNode, "LOGIN", pLoginNode);

    std::string element = "";
    pLoginNode->GetAndVerifyChildNodeValue("AUTHORIZATION_GROUP", element, pszError, ONE_LINE_BUFFER_LENGTH);
    if(0 == element.compare(STR_AUTH_ADMIN)) authGroup = SUPER_NUM;
    else if(0 == element.compare(STR_AUTH_ADMIN_READONLY)) authGroup = ADMINISTRATOR_READONLY_NUM;
   
    return STATUS_OK;
}

int CShelfXmlConvertor::BuildShelfLoginMsg(char* szXmlResponse, ostringstream & strShelfResponse, int authGroup)
{
    strShelfResponse << "$AuthorizationGroup=" << authGroup;
    return STATUS_OK;
}
    
int CShelfXmlConvertor::BuildShelfIpmiEntityListMsg(char* szXmlResponse, ostringstream & strShelfResponse)
{
    CXMLDOMDocument xmlDoc;
    HRES parseStatus = xmlDoc.Parse((const char**)&szXmlResponse);
    if(SEC_OK != parseStatus)
    {
        FTRACEINTO << "CShelfXmlConvertor::BuildShelfIpmiEntityListMsg: CXMLDOMDocument::Parse failed";
        return STATUS_FAIL;
    }

	char pszError[ERROR_MESSAGE_LEN];
	pszError[0] = '\0';
    int nStatus = STATUS_OK;
    CXMLDOMElement *pResponseRootNode = xmlDoc.GetRootElement();
    CXMLDOMElement *pActionNode = NULL;
    CXMLDOMElement *pGetNode = NULL;
    CXMLDOMElement *pCardSummaryList = NULL;
    CXMLDOMElement *pTempNode=NULL;

    GET_MANDATORY_CHILD_NODE(pResponseRootNode, "ACTION", pActionNode);
    GET_MANDATORY_CHILD_NODE(pActionNode, "GET", pGetNode);
    GET_MANDATORY_CHILD_NODE(pGetNode, "CARD_SUMMARY_LS", pCardSummaryList);


    int cardsNumber = 0;
    ostringstream strTemp;
    std::string element; 
    DWORD dwSlotID = 0;
    
    GET_FIRST_CHILD_NODE(pCardSummaryList, "CARD_CONTENT",pTempNode);
    while (pTempNode)
    {
        //ADD_CHILD_TO_MSG(pTempNode, "SlotID", element, strTemp); we need specfic handle for Ninja DSP Card's SlotID: 6->1 7->2 8->3
       {
            if (pTempNode!=NULL)
            {
                	nStatus = pTempNode->GetAndVerifyChildNodeValue("SlotID",&dwSlotID,pszError,_0_TO_DWORD);
                	if(nStatus != STATUS_OK &&
                       nStatus != STATUS_NODE_MISSING &&
                       nStatus != STATUS_ENUM_VALUE_INVALID &&
                       nStatus != STATUS_NODE_LENGTH_TOO_SHORT &&
                       nStatus != STATUS_NODE_LENGTH_TOO_LONG &&
                       nStatus != STATUS_VALUE_OUT_OF_RANGE &&
                       nStatus != STATUS_IP_ADDRESS_INVALID)
                		strTemp << "$" << "SlotID" << "=";
                	else
                   {
                        if(eProductTypeNinja == CApacheModuleEngine::GetProductType() && dwSlotID >= DSP_CARD_SLOT_ID_0 && dwSlotID <= DSP_CARD_SLOT_ID_2) dwSlotID = dwSlotID + 1 - DSP_CARD_SLOT_ID_0;
                        strTemp << "$" << "SlotID" << "=" << dwSlotID;
                   }
            }
        }
        ADD_CHILD_TO_MSG(pTempNode, "SubBoardID", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "IpmbAddress", element, strTemp);
        //ADD_CHILD_TO_MSG(pTempNode, "CardType", element, strTemp);   we need specfic handle for NINJA_CNTL to CNTL
        {
            if (pTempNode!=NULL)
            {
                	nStatus = pTempNode->GetAndVerifyChildNodeValue("CardType",element,pszError,ONE_LINE_BUFFER_LENGTH);
                	if(nStatus != STATUS_OK &&
                       nStatus != STATUS_NODE_MISSING &&
                       nStatus != STATUS_ENUM_VALUE_INVALID &&
                       nStatus != STATUS_NODE_LENGTH_TOO_SHORT &&
                       nStatus != STATUS_NODE_LENGTH_TOO_LONG &&
                       nStatus != STATUS_VALUE_OUT_OF_RANGE &&
                       nStatus != STATUS_IP_ADDRESS_INVALID)
                		strTemp << "$" << "CardType" << "=";
                	else
                   {
                        if(eProductTypeNinja == CApacheModuleEngine::GetProductType() && 0 == element.compare("NINJA_CNTL")) element = "RMX";
                        strTemp << "$" << "CardType" << "=" << element;
                   }
            }
        }
        ADD_CHILD_TO_MSG(pTempNode, "NumMezzanine", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "Status", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "Temperature", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "Voltage", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "BitFail", element, strTemp);
        cardsNumber++;
        GET_NEXT_CHILD_NODE(pCardSummaryList, "CARD_CONTENT", pTempNode);
    }
    strShelfResponse << "$NumOfElem=" << cardsNumber << "$NumOfElemFields=9";
    strShelfResponse << strTemp.str();

    return STATUS_OK;
}


int CShelfXmlConvertor::BuildShelfLanStatGetPortsListMsg(char* szXmlResponse, ostringstream & strShelfResponse)
{
    CXMLDOMDocument xmlDoc;
    HRES parseStatus = xmlDoc.Parse((const char**)&szXmlResponse);
    if(SEC_OK != parseStatus)
    {
        FTRACEINTO << "CShelfXmlConvertor::BuildShelfLanStatGetPortsListMsg: CXMLDOMDocument::Parse failed";
        return STATUS_FAIL;
    }

	char pszError[ERROR_MESSAGE_LEN];
	pszError[0] = '\0';
    int nStatus = STATUS_OK;
    CXMLDOMElement *pResponseRootNode = xmlDoc.GetRootElement();
    CXMLDOMElement *pActionNode = NULL;
    CXMLDOMElement *pGetNode = NULL;
    CXMLDOMElement *pPortSummaryList = NULL;
    CXMLDOMElement *pTempNode=NULL;

    GET_MANDATORY_CHILD_NODE(pResponseRootNode, "ACTION", pActionNode);
    GET_MANDATORY_CHILD_NODE(pActionNode, "GET", pGetNode);
    GET_MANDATORY_CHILD_NODE(pGetNode, "PORT_SUMMARY_LS", pPortSummaryList);


    int portNumber = 0;
    ostringstream strTemp;
    std::string element; 
    
    GET_FIRST_CHILD_NODE(pPortSummaryList, "PORT_SUMMARY",pTempNode);
    while (pTempNode)
    {
        ADD_CHILD_TO_MSG(pTempNode, "SlotID", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "PortID", element, strTemp);
        ADD_CHILD_TO_MSG(pTempNode, "Status", element, strTemp);
        portNumber++;
        GET_NEXT_CHILD_NODE(pPortSummaryList, "PORT_SUMMARY", pTempNode);
    }
    strShelfResponse << "$NumOfElem=" << portNumber << "$NumOfElemFields=2";
    strShelfResponse << strTemp.str();

    return STATUS_OK;
}

int CShelfXmlConvertor::StringWrapper(char* szXmlResponse, char** szShelfMsgResponse, unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId)
{
    int ret = STATUS_OK;
    ostringstream strShelfResponse;
    //msg is from 3rd API and ask for Hardware info.
    int status = CXmlMiniParser::GetResponseStatus(szXmlResponse);
    
    if(1 == ulOpcode)
    {
        int authGroup = -1;
        if(status == STATUS_OK)
        {

            GetShelfLoginMsgAuthGroup(szXmlResponse, authGroup);
            if(authGroup == SUPER_NUM || authGroup == ADMINISTRATOR_READONLY_NUM) //only administrator or administrator_readonly can login
            {
                status = STATUS_CONTROL_LOGIN_EXISTS;
            }
            else
            {
                status = STATUS_CONTROL_LOGIN_INVALID;
            }        
        }
        else
        {
             status = STATUS_CONTROL_LOGIN_INVALID;
        }

        if(status == STATUS_CONTROL_LOGIN_INVALID) ret = status;
        BuildShelfGeneralMsg(status, ulOpcode, ulMsgId, nSlotId, strShelfResponse);
        BuildShelfLoginMsg(szXmlResponse, strShelfResponse, authGroup);
        goto done;
    }

    BuildShelfGeneralMsg(status, ulOpcode, ulMsgId, nSlotId, strShelfResponse);     
    if(status == STATUS_OK)
    { 
        //build succeed msg.
        switch(ulOpcode)
        {
            case IPMI_ENTITY_LIST_REQ:
                BuildShelfIpmiEntityListMsg(szXmlResponse, strShelfResponse);
                break;
            case LAN_STAT_GET_PORTS_LIST_REQ:
                BuildShelfLanStatGetPortsListMsg(szXmlResponse, strShelfResponse);
                break;
        }
    }

done:
    int len = strShelfResponse.str().size();
    *szShelfMsgResponse = new char[len + 1];
    memset(*szShelfMsgResponse, 0, len + 1);
    strcpy(*szShelfMsgResponse, strShelfResponse.str().c_str());

    return ret;
}

