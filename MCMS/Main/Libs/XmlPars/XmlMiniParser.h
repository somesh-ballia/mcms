// XmlMiniParser.h: interface for the CXmlMiniParser class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_XmlMiniParser_H__)
#define _XmlMiniParser_H__

class CXmlMiniParser {

public:

	static char* GetActionName(const char* pszSearchString, char** pszEndActionName);
	static char* GetTransName(const char* pszSearchString, char** pszEndTransName);
	static int GetMessageId(const char* pszSearchString);
	static int GetConnectionId(const char* pszSearchString);
	static int GetYourToken1(const char* pszSearchString);
	static int GetYourToken2(const char* pszSearchString);
	static int GetResponseStatus(const char* pszSearchString);
//	static char* GetStationName(const char* pszSearchString, char** pszEndStationName);
	static char* GetFilenameInURI(const char* pszURI);
	static int GetAuthorization(const char* pszSearchString);
	static bool GetUserName(const char* pszLoginAction, char* pszUserName);
	static bool GetStationName(const char* pszLoginAction, char* pszStationName);
	static int GetNumberFromStartResponse(const char* startResponce);
	static void RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag);

private:

	static char* GetElementStringValue(const char* pszSearchString, const char* pszElementOpenTag,
									   const char* pszElementCloseTag, char** pszEndElementValue);
};

#endif // !defined(_XmlMiniParser_H__)
