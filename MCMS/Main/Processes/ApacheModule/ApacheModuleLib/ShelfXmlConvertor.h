// ShelfXmlConvertor.h: interface for the CShelfXmlConvertor class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_ShelfXmlConvertor_H__)
#define _ShelfXmlConvertor_H__

#define STATUS_CONTROL_BASE                 0
#define STATUS_CONTROL_LOGIN_EXISTS         STATUS_CONTROL_BASE+0  
#define STATUS_CONTROL_LOGIN_NOT_EXISTS     STATUS_CONTROL_BASE+1  
#define STATUS_CONTROL_NO_PERMISSION        STATUS_CONTROL_BASE+2
#define STATUS_CONTROL_LOGIN_INVALID        STATUS_CONTROL_BASE+3  
#define STATUS_CONTROL_PASSWORD_NOT_VALID   STATUS_CONTROL_BASE+4  


class CShelfXmlConvertor {

public:

	static void Initialize();
	static void Clean();
	static void StringStripper(const char* commandRcvd, unsigned int NumBytes, char** szXMLRequest, unsigned int & ulOpcode, unsigned int & ulMsgId, int & nSlotId);
	static int StringWrapper(char* szXmlResponse, char** szShelfMsgResponse, unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId);

private:
	static int ParseLoginTrans(char *command, char *outCommandArgs[], int *outArgs, unsigned int *outMsgId);
	static int BuildEmaLoginMsg(char **commandArgs, int numArgs, unsigned int ulMsgId, int nSlotId, char** szXMLRequest);
	static int BuildEmaIpmiMsg(unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId, char** szXMLRequest);
	static int BuildEmaLanInfoMsg(unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId, char** szXMLRequest);

	static int BuildShelfGeneralMsg(int status, unsigned int ulOpcode, unsigned int ulMsgId, int nSlotId, ostringstream & strShelfResponse);
	static int GetShelfLoginMsgAuthGroup(char* szXmlResponse, int & authGroup);
	static int BuildShelfLoginMsg(char* szXmlResponse, ostringstream & strShelfResponse, int authGroup);
	static int BuildShelfIpmiEntityListMsg(char* szXmlResponse, ostringstream & strShelfResponse);
	static int BuildShelfLanStatGetPortsListMsg(char* szXmlResponse, ostringstream & strShelfResponse);
};



#endif // !defined(_ShelfXmlConvertor_H__)

