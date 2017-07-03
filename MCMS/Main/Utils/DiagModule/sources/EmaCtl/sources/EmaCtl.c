/*============================================================================*/
/*            Copyright ?? 2006 Polycom Israel,Ltd. All rights reserved        */
/*----------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary        */
/* information of Polycom Israel, Ltd. and is protected by law.               */
/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	EmaCtl.c                                                      */
/* PROJECT:  	Switch Card - Ema API Module								  */
/* PROGRAMMER:  Giyora Achrack												  */
/* DESCRIPTION: */
/*				*/
/*				*/
/*																			  */
/*============================================================================*/

#include <string.h>

#include "emaCtl.h"
#include "LinuxSystemCallsApi.h"
#include "McmsApi.h"
#include "MplMcmsStructs.h"
//#include "CardsStructs.h"
#include "AuthenticationStructs.h"
#include "emaCtl.h"
#include "EmaShared.h"
#include "SocketApiTypes.h"
#include "openssl/ssl.h"
#include "Print.h"
#include "sha1.h"
#include "EmaApi.h"

#define ENC_B64_PASSWD_MAX_LENGTH (128)

extern UINT32 unEmaKeepAliveFail;
static TSwitchUserList s_switchUserList;
static FILE *s_emaUserFile=NULL;
static int s_emaResetWatchdog=-1;
static TStructToStr s_emaCtlRs[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string      ,   1,            100,      "Description"},
	{e_unsignedLong,   1,              4,      "AuthorizationGroup"}

};

#define STR_USER_NAME_BEGIN		"<USER_NAME>"
#define STR_USER_NAME_END			"</USER_NAME>"
#define STR_PASSWORD_BEGIN		"<ENCRYPTED_PASSWORD>"
#define STR_PASSWORD_END			"</ENCRYPTED_PASSWORD>"
#define STR_AUTH_GROUP_BEGIN		"<AUTHORIZATION_GROUP>"
#define STR_AUTH_GROUP_END		"</AUTHORIZATION_GROUP>"

#define STR_AUTH_ADMIN				"administrator"
#define STR_AUTH_ADMIN_READONLY	"administrator_readonly"

#define SUPER_NUM           0
#define ADMINISTRATOR_READONLY_NUM         6


void updateEmaWatchdogTimer_(const char *file, int line)
{
//    printf("\n-- WD --: %s:%d->%s: emaResetWD %d\n", file, line, __FUNCTION__, s_emaResetWatchdog + 1);
    s_emaResetWatchdog++;
}


void SetEmaWatchdogTimer_(const char *file, int line, int var)
{
//    printf("\n-- WD --: %s:%d->%s: emaResetWD %d\n", file, line, __FUNCTION__, s_emaResetWatchdog);
    s_emaResetWatchdog = var;
}

int readEmaWatchdogTimer()
{
   return s_emaResetWatchdog;
}


void reStartEmaWatchdogTimer_(const char *file, int line)
{
//    printf("\n--- WD --> %s:%d: \n", file, line, __FUNCTION__);
    s_emaResetWatchdog=0;
    unEmaKeepAliveFail = 0;
}

/****************************************************************************
* Prototype:    void
* Description:  read rma users file.
* Return Value: None
* Arguments:    None
*****************************************************************************/
BOOL GetXMLContent(char ** ppcurrent, const char * begin, const char * end, char * content, int size)
{
	BOOL ret = FALSE;
	char * ptBegin, * ptEnd;
	if(ppcurrent == NULL || (*ppcurrent) == NULL) goto done;

	if(NULL == (ptBegin = strstr(*ppcurrent, begin)))
	{
		*ppcurrent = NULL;
		goto done;
	}

	ptBegin += strlen(begin);

	if(NULL == (ptEnd = strstr(ptBegin, end)))
	{
		*ppcurrent = NULL;
		goto done;
	}

	memset(content, 0, size);
	if(ptEnd - ptBegin < size)
	{
		memcpy(content , ptBegin, ptEnd - ptBegin);
	}
	else
	{
        	MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "GetXMLContent:  ptEnd - ptBegin (%d) >= size (%d);", ptEnd - ptBegin, size);	
	}

	*ppcurrent = ptEnd +  strlen(end);
	ret = TRUE;
done:
	return ret;
}

void EmaUserInitEncOperatorDBXml(void)
{
	size_t rc;
	int ind;
	long size;
	char * content = NULL;
	char authGroup[64];
	s_emaUserFile=fopen("/mcms/Cfg/EncOperatorDB.xml","r+");
	if (s_emaUserFile == NULL)
	{
        	MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "EmaUserInitEncOperatorDBXml: fopen failed: /mcms/Cfg/EncOperatorDB.xml");
		goto done;
	}

	//find out file size.
	fseek(s_emaUserFile, 0L, SEEK_END);
	size = ftell(s_emaUserFile);
	if(-1 == size)
	{
		MfaBoardPrint(CM_INITIATOR_PRINT,
		                      PRINT_LEVEL_ERROR,
		                      PRINT_TO_TERMINAL,
		                      "EmaUserInitEncOperatorDBXml: getting file size failed: /mcms/Cfg/EncOperatorDB.xml");
		goto done;
	}

	fseek(s_emaUserFile, 0L, SEEK_SET);

	content = (char *)malloc(size+1);
	if (content == NULL)
	{
        	MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "EmaUserInitEncOperatorDBXml: malloc content failed. size: %ld", size);
		goto done;
	}
	memset(content, 0, size + 1);
	
	rc = fread(content, sizeof(char), size, s_emaUserFile);
	if (rc != (size_t)size)
	{
        	MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "EmaUserInitEncOperatorDBXml: fread rc(%lu) != size(%ld)", rc, size);
	}

	//Get Username, EncPasswd, AuthorizationGroup
	char * pcurrent = content;
	ind = 0;
	s_switchUserList.numEmaUsers = 0;
	while(ind < MAX_OPERATORS_IN_MCU
		&& GetXMLContent(&pcurrent, STR_USER_NAME_BEGIN, STR_USER_NAME_END, s_switchUserList.emaUserList.usersList[ind].login, OPERATOR_NAME_LEN-1))
	{
		if(FALSE == GetXMLContent(&pcurrent, STR_PASSWORD_BEGIN, STR_PASSWORD_END, s_switchUserList.emaUserList.usersList[ind].password, OPERATOR_PWD_LEN-1))
		{
			MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "EmaUserInitEncOperatorDBXml: GetXMLContent password failed. user name: %s", s_switchUserList.emaUserList.usersList[ind].login);
			continue;
		}

		if(FALSE == GetXMLContent(&pcurrent, STR_AUTH_GROUP_BEGIN, STR_AUTH_GROUP_END, authGroup, sizeof(authGroup)))
		{
			MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "EmaUserInitEncOperatorDBXml: GetXMLContent auth group failed. user name: %s", s_switchUserList.emaUserList.usersList[ind].login);
			continue;
		}
		else
		{
			//only super user can login
			if(0 == strcmp(authGroup, STR_AUTH_ADMIN)) s_switchUserList.emaUserList.usersList[ind].authorizationGroup = SUPER_NUM;
			else if(0 == strcmp(authGroup, STR_AUTH_ADMIN_READONLY)) s_switchUserList.emaUserList.usersList[ind].authorizationGroup = ADMINISTRATOR_READONLY_NUM;
			else continue;
		}
		
		s_switchUserList.numEmaUsers++;
		ind++;
	}

	printf("\n yosi - num of users = %d",s_switchUserList.numEmaUsers);
	for (ind=0;ind<s_switchUserList.numEmaUsers;ind++)
	{
		printf("\n YOSI- start updateEmaUserList rc=%x group %x %s %s\n",rc,s_switchUserList.emaUserList.usersList[ind].authorizationGroup,
	   	                                              s_switchUserList.emaUserList.usersList[ind].login,
		                                              s_switchUserList.emaUserList.usersList[ind].password);
	}
	
done:
	
	if(content)
	{
		free(content);
		content = NULL;
	}
	
	if(s_emaUserFile)
	{
		fclose(s_emaUserFile);
		s_emaUserFile = NULL;
	}
}

void ToBase64(const unsigned char* input, int length, char* output)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;
	
	//MfaBoardPrint(CM_INITIATOR_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL, "(%s %s %d) input = %s",__FILE__,__FUNCTION__,__LINE__, input);
	
	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	(void) BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	
	//MfaBoardPrint(CM_INITIATOR_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL, "(%s %s %d) bptr->length = %d bptr->data = %s",__FILE__,__FUNCTION__,__LINE__, bptr->length, bptr->data);
	
	
	memcpy(output, bptr->data, bptr->length-1);
	output[bptr->length-1] = 0;
	
	BIO_free_all(b64);
	
	//MfaBoardPrint(CM_INITIATOR_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL, "(%s %s %d) buff = %s",__FILE__,__FUNCTION__,__LINE__, output);
	
	return;
}


void SHA256_Encryption(char* source, char* dest)
{
	unsigned char encPwd[33];
	memset(encPwd, '\0', 33);

	const EVP_MD* sha256_type = EVP_sha256();
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit(&ctx, sha256_type);
	EVP_DigestInit_ex(&ctx, sha256_type, NULL);

	EVP_DigestUpdate(&ctx, source, strlen(source));
	EVP_DigestFinal_ex(&ctx, encPwd, NULL);
	EVP_MD_CTX_cleanup(&ctx);

	ToBase64(encPwd, 32, dest);
}

/****************************************************************************
* Prototype:    int
* Description:  Apply sha1 and base64 on password2test and compare it to DBpassword
* Return Value: 0 - no match, 1 - match
* Arguments:    DBpassword : password from DB, its in sha1 + base64
*               password2test: password from user, its in clear text
*****************************************************************************/
int IsPasswordMatch(char *DBpassword, char *password2test)
{
	char caSha256B64Res[ENC_B64_PASSWD_MAX_LENGTH] = {0};
	int rc = FALSE;
	printf("in function IsPasswordMatch");
#if 0
    if(flashUtilIsJITCMode())
    {
        // in JITC mode switch does not validates users.
        // MCMS should have done this before.
        MfaBoardPrint(CM_INITIATOR_PRINT,
                      PRINT_LEVEL_ERROR,
                      PRINT_TO_TERMINAL,
                      "IsPasswordMatch: In JITC mode, no password check, return true");
        return TRUE;
    }
#endif

	    // not JITC mode
	MfaBoardPrint(CM_INITIATOR_PRINT,
	                      PRINT_LEVEL_ERROR,
	                      PRINT_TO_TERMINAL,
	                      "IsPasswordMatch: No JITC mode, check password sha256");

	SHA256_Encryption(password2test, caSha256B64Res);

	//now compare the 2 encrypted passwd's
	rc = (0 == strcmp(DBpassword, caSha256B64Res) ? TRUE : FALSE);

	if(rc == FALSE)
	{
	    // not JITC mode
	    MfaBoardPrint(CM_INITIATOR_PRINT,
	                      PRINT_LEVEL_ERROR,
	                      PRINT_TO_TERMINAL,
	                      "IsPasswordMatch: No JITC mode, check password sha1 + base64");
	
	    char sha1res[20];
	    char base641res[100];  // len has to be at least (sizeof(sha1res) + 2) / 3 * 4 + 1)
	    int rc = FALSE;

	    // apply sha1 encryption
	    sha1( (char *)password2test, strlen(password2test), sha1res);

	    // apply base64
	    base64(sha1res, sizeof(sha1res), base641res);

	    // compare paswords: DB vs User
	    rc = (0 == strcmp(DBpassword, base641res) ? TRUE : FALSE);
		printf(" Yosi - DBpassword  = %s, base641res = %s, password2test = %s rc= %d \n",DBpassword,base641res,password2test,rc);
	}
	
    // not JITC mode
/*     MfaBoardPrint(CM_INITIATOR_PRINT, */
/*                   PRINT_LEVEL_ERROR, */
/*                   PRINT_TO_TERMINAL, */
/*                   "IsPasswordMatch: user: POLYCOM, DBpassword: %s ? password2test: %s, base641res: %s -> rc = %d", */
/*                   DBpassword, password2test, base641res, rc); */

    return rc;
}

/****************************************************************************
* Prototype:    int
* Description:  check ema users List and file.
* Return Value: 0 -on success
* Arguments:    None
*****************************************************************************/
int checkEmaUsers(INT8 *pUser,INT8 *pPass,UINT16 *authorizationGroup)
{
	int ind,retCode=STATUS_CONTROL_LOGIN_INVALID;
	printf("yosi - inside checkEmaUsers num_of_users = %d",s_switchUserList.numEmaUsers);
	for (ind=0;ind<s_switchUserList.numEmaUsers;ind++)
	{
		printf("yosi - user = %s ",s_switchUserList.emaUserList.usersList[ind].login);
		if ( 0 == strcmp(s_switchUserList.emaUserList.usersList[ind].login,pUser) )
		{
		 if (TRUE == IsPasswordMatch((char*)(s_switchUserList.emaUserList.usersList[ind].password), (char*)pPass))
			{
				*authorizationGroup =s_switchUserList.emaUserList.usersList[ind].authorizationGroup;
				retCode = STATUS_CONTROL_LOGIN_EXISTS;
				break;
			}
			else retCode = STATUS_CONTROL_LOGIN_INVALID;
		}
	}
	return retCode;
}

void sendMsgToWrapper(int *cmds,int rc,UINT32 authorizationGroup)
{
	struct SMsgToSend
	{
	  TSpecGnrlHdr 	specGnrlHdr;
	  INT8         	msgDscr[sizeof(s_emaCtlRs)];
	  TEmaIndHeader emaIndHeader;
	  UINT32        authorizationGroup;
	};
	struct SMsgToSend *pMsgToSend;

    if(NULL == (pMsgToSend = malloc(sizeof(struct SMsgToSend)))) return;
    memset(pMsgToSend,0,sizeof(struct SMsgToSend));
    pMsgToSend->specGnrlHdr.ulMsgOffset = 4 + sizeof(s_emaCtlRs);
	memcpy(pMsgToSend->msgDscr,s_emaCtlRs,sizeof(s_emaCtlRs));

	pMsgToSend->emaIndHeader.ulOpcode = cmds[0];
	pMsgToSend->emaIndHeader.ulMsgID  = cmds[1];
	pMsgToSend->emaIndHeader.ulSlotID = cmds[2];
	pMsgToSend->emaIndHeader.ulStatus = rc;

	pMsgToSend->authorizationGroup = authorizationGroup;
	StringWrapper(eMaxTcpConnections,(INT32)pMsgToSend);
}

void handleEmaCtlMsg(char **commandArgs,int numArgs)
{
	int retCode,ind,rc,cmd[8]= {0},opcode=-1,msgId=-1,cmdInd=0;
	UINT16 authorizationGroup;
	char strCmd[256],*str1,*str,userPass[2][256]={"",""};

	for (ind=1;ind<numArgs ;ind++)
	{
		memset(strCmd, 0, sizeof(strCmd));
		strncpy(strCmd,commandArgs[ind],sizeof(strCmd)-1);
		str = (char *)strtok(strCmd,"=");
		if (str != NULL)
		{
			if (strcmp(str,"UserName")==0)
			{
				str1=strstr(commandArgs[ind],"=");
				if (str1)
					sscanf(++str1,"%255s",userPass[0]);
				//					printf("UserName %s\n",userPass[0]);
			}
			else if (strcmp(str,"UserPassword")==0)
			{
				str1=strstr(commandArgs[ind],"=");
				if (str1)
					sscanf(++str1,"%255s",userPass[1]);
				//					printf("UserPassword %s\n",userPass[1]);
			}
			else
			{
				str1=strstr(commandArgs[ind],"=");
				if (str1)
					sscanf(++str1,"%d",&cmd[cmdInd++]);
			}
		}
	}
	printf("yosi - parse control massage cmd=%d ",cmd[0]);
	if (cmd[0] == 1)
	{
		printf("yosi - before checkEmaUsers  ");
		rc=checkEmaUsers(userPass[0],userPass[1],&authorizationGroup);
		sendMsgToWrapper(cmd,rc,authorizationGroup);
	}
	else if (cmd[0] == 3)
		reStartEmaWatchdogTimer();
}

// commandArgs[0] : user name
// commandArgs[1] : user password
void handleEmaLoginMsg(char **commandArgs, int numArgs, int ulMsgId, int ulSlotId)
{
    int rc = 0;
    UINT16 authorizationGroup = 0;
    int cmd[3];

    cmd[0] = 1;       // opcode
    cmd[1] = ulMsgId; // msg id
    cmd[2] = ulSlotId;// slot id

    rc = checkEmaUsers(commandArgs[0], commandArgs[1], &authorizationGroup);
 
	
   sendMsgToWrapper(cmd, rc, authorizationGroup);
}


/*void checkEmaUsersfromEma(void)
{
	char *xxx;
	MfaBoardPrint(CM_INITIATOR_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"========================== Start SWITCH program ======================");
    xxx=malloc(500);
	strcpy(xxx,"Control$Opcode=544$MsgID=765$SlotID=121$UserName=POLYC$UserPassword=STAM");
	StringStripper(xxx,strlen(xxx));
    xxx=malloc(500);
	strcpy(xxx,"Control$Opcode=1$MsgID=765$SlotID=121$UserName=POLYC$UserPassword=STAM");
	StringStripper(xxx,strlen(xxx));
    xxx=malloc(500);
	strcpy(xxx,"Control$Opcode=544$MsgID=765$SlotID=121$UserName=POLYCOM$UserPassword=STA");
	StringStripper(xxx,strlen(xxx));
    xxx=malloc(500);
	strcpy(xxx,"Control$Opcode=544$MsgID=765$SlotID=121$UserName=$UserPassword=");
	StringStripper(xxx,strlen(xxx));
    xxx=malloc(500);
	strcpy(xxx,"Control$Opcode=544$MsgID=765$SlotID=121$UserName=POLYCOM$UserPassword=STAM");
	StringStripper(xxx,strlen(xxx));
    xxx=malloc(500);
	strcpy(xxx,"Control$Opcode=544$MsgID=765$SlotID=121$UserName=polycom$UserPassword=STAM");
	StringStripper(xxx,strlen(xxx));
}*/

