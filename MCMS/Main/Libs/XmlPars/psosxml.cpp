// psosxml.cpp

#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <netinet/in.h>

#include "psosxml.h"
#include "StructTm.h"
#include "ObjString.h"
#include "XmlApi.h"
#include "StringsMaps.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "Macros.h"
#include "XmlDefines.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "DefinesGeneral.h"
#include "OsFileIF.h"

#include "EncodingConvertor.h"
#include "UnicodeDefines.h"

#include "InternalProcessStatuses.h"

#define NIL(Type) (Type*)0
#define MAX_NODE_NAME 100
#define MAX_XML_SIZE 1048576 // 1 MB //200000

#define START_PROLOG "<?"
#define END_PROLOG "?>"

#define START_COMMENT "<!--"
#define END_COMMENT "-->"

#define START_DOCTYPE "<!DOCTYPE"
#define END_DOCTYPE ">"

#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

CXMLDOMNamedNodeMap::CXMLDOMNamedNodeMap(void)
{
	m_wNumbMembersInArray	= 0;
	m_pNodeListHead=NULL;
	m_pNodeListLast=NULL;
	m_pNodeListCurrent=NULL;
}

CXMLDOMNamedNodeMap::~CXMLDOMNamedNodeMap(void)
{
	CXMLDOMNode*	pNodeListHead;
	while(m_pNodeListHead!=NULL)
	{
		pNodeListHead=m_pNodeListHead->m_pNextSibling;
		delete m_pNodeListHead;
		m_pNodeListHead=pNodeListHead;

	}
}

/*
		Purpose: Indicates the number of items in the collection.
		Parameters:	listLength [out, retval] Number of items in the collection.
		Return Values:	SEC_OK  Value returned if successful.
						ERR_INVALIDARG Value returned if listLength is Null.
*/
HRES CXMLDOMNamedNodeMap::get_length(long *listLength)
{
	*listLength = m_wNumbMembersInArray;
	return SEC_OK;
}


/*
		Purpose: Retrieves the attribute with the specified name.
		Parameters:	name [in] Name of the attribute.
					namedItem [out, retval] CXMLDOMNode object for the specified attribute.
					Returns Null if the attribute node is not in this collection.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value when returning Null.
						ERR_INVALIDARG Value returned if namedItem is Null.
*/
HRES CXMLDOMNamedNodeMap::getNamedItem(char* pName,CXMLDOMNode **Item)
{
	HRES hres = ERR_FAIL;
	char *szNodeName;

	*Item = NULL;

	CXMLDOMNode* CurrentNode = m_pNodeListHead;

	while(CurrentNode!=NULL)
	{
		CurrentNode->get_nodeName(&szNodeName);
		if (strcmp(szNodeName,pName)==0)
		{
			*Item=CurrentNode;
			hres = SEC_OK;
			break;
		}
		CurrentNode = CurrentNode->m_pNextSibling;
	}

	return hres;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNamedNodeMap::nextNode(/* [retval][out] */ CXMLDOMNode **nextItem)
{
	HRES hrsResult = SEC_OK;

	if (m_pNodeListCurrent!=NULL)
		m_pNodeListCurrent = m_pNodeListCurrent->m_pNextSibling;
	else if (m_pNodeListHead!=NULL)
		m_pNodeListCurrent = m_pNodeListHead;
	else
		hrsResult=ERR_FAIL;

	(*nextItem) = m_pNodeListCurrent;

	return hrsResult;

}

HRES CXMLDOMNamedNodeMap::reset( void)
{
	m_pNodeListCurrent = NULL;
	return SEC_OK;
}

/*
		Purpose: Adds the supplied node to the collection
		Parameters:	newItem [in] Attribute to be added to the collection.
					nameItem [out, retval] Attribute successfully added to the collection. If Null, no object is created.


		Return Values:	SEC_OK Value returned if successful.
						ERR_INVALIDARG Value returned if newItem is Null.
						ERR_FAIL Value returned if an error occurs.
*/
HRES CXMLDOMNamedNodeMap::setNamedItem(CXMLDOMNode *newItem)
{
	HRES HRES = SEC_OK;

	if (m_pNodeListHead==NULL)
		m_pNodeListHead=m_pNodeListLast=newItem;
	else
	{
		m_pNodeListLast->m_pNextSibling = newItem;
		m_pNodeListLast = newItem;
	}

	m_wNumbMembersInArray++;

	return HRES;
}

CXMLDOMNodeList::CXMLDOMNodeList(void)
{
	m_wNumbMembersInArray	= 0;
	m_wCurrentItem			= 0;
	m_pNodeListHead=NULL;
	m_pNodeListLast=NULL;
	m_pNodeListCurrent=NULL;

}

CXMLDOMNodeList::~CXMLDOMNodeList(void)
{
	CXMLDOMNode*	pNodeListHead;
	while(m_pNodeListHead!=NULL)
	{
		pNodeListHead=m_pNodeListHead->m_pNextSibling;
		delete m_pNodeListHead;
		m_pNodeListHead=pNodeListHead;

	}
}

HRES CXMLDOMNodeList::addNode(/* [retval][out] */ CXMLDOMNode *nextItem)
{
	HRES hres = SEC_OK;

	if (m_pNodeListHead==NULL)
		m_pNodeListHead=m_pNodeListLast=nextItem;
	else
	{
		m_pNodeListLast->m_pNextSibling = nextItem;
		m_pNodeListLast = nextItem;
	}
	m_wNumbMembersInArray++;

	return hres;
}

HRES CXMLDOMNodeList::getNodeByName(CXMLDOMNode **Item,char *pName,char *pszRetErrorBuffer,int *status)
{
	HRES hres = ERR_FAIL;
	char *szNodeName;

	*Item = NULL;

	CXMLDOMNode* CurrentNode = m_pNodeListHead;

	while(CurrentNode!=NULL)
	{
		CurrentNode->get_nodeName(&szNodeName);
		if (strcmp(szNodeName,pName)==0)
		{
			*Item=CurrentNode;
			hres = SEC_OK;
			break;
		}
		CurrentNode = CurrentNode->m_pNextSibling;
	}

	if (hres == ERR_FAIL)
	{

		if (status)
		{
			*status=STATUS_NODE_MISSING;
			sprintf(pszRetErrorBuffer,"Element - '%s'",pName);
		}

	}

	return hres;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNodeList::getNodeDecValueByName(char *pName,int *Value)
{
	HRES hres = ERR_FAIL;
	char *szNodeName;
	CXMLDOMNode* CurrentNode = m_pNodeListHead;

	while (CurrentNode!=NULL)
	{
		CurrentNode->get_nodeName(&szNodeName);
		if (strcmp(szNodeName,pName)==0)
		{
			CurrentNode->get_nodeValue(&szNodeName);
			if(szNodeName!=NULL)
			{
				*Value=atoi(szNodeName);
				hres = SEC_OK;
			}
			break;
		}
		CurrentNode = CurrentNode->m_pNextSibling;
	}

	return hres;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNodeList::getNodeValueByName(char *pName,char**name)
{
	HRES hres = ERR_FAIL;
	char *szNodeName;
	CXMLDOMNode* CurrentNode = m_pNodeListHead;

	while (CurrentNode!=NULL)
	{
		CurrentNode->get_nodeName(&szNodeName);
		if (strcmp(szNodeName,pName)==0)
		{
			CurrentNode->get_nodeValue(name);
			hres = SEC_OK;
			break;
		}
		CurrentNode = CurrentNode->m_pNextSibling;
	}

	return hres;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/* parameters:		The number of items in the collection
	   return values:	SEC_OK The value returned if successful.
						ERR_INVALIDARG The value returned if listLength  is Null.*/
HRES CXMLDOMNodeList::get_length(/* [retval][out] */ long *listLength)
{
	HRES hrsResult = SEC_OK;

	*listLength = m_wNumbMembersInArray;

	return hrsResult;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/* parameters:		nextItem . The next node in the collection, or Null if there is no next node.
	   return values:	SEC_OK The value returned if successful.
						ERR_INVALIDARG The value returned if nextItem   is Null.*/

HRES CXMLDOMNodeList::nextNode(/* [retval][out] */ CXMLDOMNode **nextItem)
{
	HRES hrsResult = SEC_OK;

	if (m_pNodeListCurrent!=NULL)
		m_pNodeListCurrent = m_pNodeListCurrent->m_pNextSibling;
	else if (m_pNodeListHead!=NULL)
		m_pNodeListCurrent = m_pNodeListHead;
	else
		hrsResult=ERR_FAIL;

	(*nextItem) = m_pNodeListCurrent;

	return hrsResult;

}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNodeList::nextNode(/* [retval][out] */ CXMLDOMNode **nextItem,char *pName)
{
	CXMLDOMNode *pNode;
	char *pszVal=NULL;
	while(nextNode(&pNode)!=ERR_FAIL)
	{
		if(pNode)
		{
			pNode->get_nodeName(&pszVal);
			if(pszVal)
			{
				if(strcmp(pszVal,pName)==0)
				{
					(*nextItem)=pNode;
					return SEC_OK;
				}
			}
		}
		else
		{
			//this is the end of the list
			*nextItem=NULL;
			return ERR_FAIL;
		}

	}
	*nextItem=NULL;

	return ERR_FAIL;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/*		This method reinitializes the iterator to point before the
				first node in the CXMLDOMNodeList so that the next call to
				nextNode returns the first item in the list.
		return values: The value returned if successful.	*/
HRES CXMLDOMNodeList::reset( void)
{
	m_wCurrentItem = 0;
	m_pNodeListCurrent = NULL;
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
										   CObjString& strValue,
										   char *pszError,
										   int type,
                                           bool isAscii)
{

	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		return childElm->GetAndValidate(strValue, pszError,  type, isAscii);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;

}


//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
										   std::string & strValue,
										   char *pszError,
										   int type,
                                           bool isAscii)
{

	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		return childElm->GetAndValidate(strValue, pszError,  type, isAscii);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;

}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
										   char** pValue,
										   char *pszError,
										   int type,
                                           bool isAscii)
{

	int nStatus = STATUS_OK;

	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		return childElm->GetAndValidate(pValue, pszError,  type, isAscii);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
                                           char* pValue,
                                           char *pszError,
                                           int type,
                                           bool isAscii)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		return childElm->GetAndValidate(pValue, pszError,  type, isAscii);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,WORD* pValue,char *pszError,int type)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		return childElm->GetAndValidate(pValue, pszError,  type);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;


}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,BYTE* pValue,char *pszError,int type)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		 return childElm->GetAndValidate(pValue, pszError,  type);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,int* pValue,char *pszError,int type)
{

	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		nStatus = childElm->GetAndValidate(pValue, pszError,  type);
		return nStatus;
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;

}



//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
										   DWORD* pValue,
										   char *pszError,
										   int type)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *childElm;

	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName)==SEC_OK)
	{
		//BYTE bIsNumber;
		return childElm->GetAndValidate(pValue, pszError,  type);//,bIsNumber);
	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}

	return nStatus;

}


//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
										   CStructTm* time,
										   char *pszError,
										   int type)
{
	int nStatus= STATUS_OK;

	CXMLDOMElement *childElm;
	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName) == SEC_OK)
	{
		return childElm->GetAndValidate(time,pszError,  type);

	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMNodeList::GetAndVerifyNodeValue(char* pszNodeName,
										   mcTransportAddress* pValue,
										   char *pszError,
										   int type)
{
	int nStatus= STATUS_OK;

	CXMLDOMElement *childElm;
	if (getNodeByName((CXMLDOMNode **)&childElm, pszNodeName) == SEC_OK)
	{
		return childElm->GetAndValidate(pValue,pszError,  type);

	}
	else
	{
		nStatus = STATUS_NODE_MISSING;
	}

	if(nStatus != STATUS_OK)
	{
		if(pszError)
			sprintf(pszError,"Element - '%s'",pszNodeName);
	}
	return nStatus;
}

CXMLDOMNode::CXMLDOMNode(void)
{
	m_szpNodeName =NULL;
	m_pParent = NULL;
	m_pNextSibling=NULL;
}

CXMLDOMNode::~CXMLDOMNode(void)
{
	DEALLOCBUFFER(m_szpNodeName);
}

CXMLDOMNode::CXMLDOMNode(const CXMLDOMNode& other)
{
	int length=strlen(other.m_szpNodeName);
	m_szpNodeName=new char[length+1];

	strncpy(m_szpNodeName,other.m_szpNodeName,length+1);
	m_szpNodeName[length] = '\0';
	m_pNextSibling=other.m_pNextSibling;
	m_pParent=other.m_pParent;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/* parameters:		name [out, retval] Node name, which varies depending on the node type.
	   return values:	SEC_OK Value returned if successful.
						ERR_INVALIDARG Value returned if name is Null. */
HRES  CXMLDOMNode::get_nodeName(/* [retval][out] */ char** name)
{
	HRES hRes = SEC_OK;
	if(m_szpNodeName == NIL(char))
	{
		*name = NIL(char);
		hRes = ERR_INVALIDARG;
	}
	else
	{
		*name = m_szpNodeName;
		hRes = SEC_OK;
	}
	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////
DWORD CXMLDOMNode::GetDumpStringLength(int offset, BYTE bPrettyXml/* = FALSE*/)
{
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNode::DumpDataAsStringArrayEx(CStrArray& strArray,
										  WORD &Compressionlevel)
{
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNode::DumpDataAsStringImpl(std::ostream &ostr,
                                      DWORD offset,
                                      BYTE  bPrettyXml)
{
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMNode::DumpDataAsStringImpl(CStrArray* strArray, char* longstr, DWORD offset, int &row, BYTE bPretty/*=FALSE*/)
{
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////////////////
HRES  CXMLDOMNode::set_nodeName( const char *name)
{
	HRES hRes = SEC_OK;

	if (m_szpNodeName)
		DEALLOCBUFFER(m_szpNodeName);

	int length=strlen(name);
	m_szpNodeName=new char[length+1];

	strncpy(m_szpNodeName,name,length+1);
	m_szpNodeName[length] = '\0';

	return hRes;
}

char* CXMLDOMNode::RemoveCommentsFromString(const char* szGivenString)
{
	if (szGivenString == NIL(char))
	{
		DBGFPASSERT(1);
		return NIL(char);
	}

	char* szStringWhithoutComments=NULL;
	const char* pWasFoundHere = NIL(char);
	DWORD wOrigStringLen=0;
	char* pPlaceToWriteTo=NULL;
	const char* pPlaceToReadFrom=NULL;

	if (strstr(szGivenString, START_COMMENT) != NIL(char))
	{
		wOrigStringLen = strlen(szGivenString) + 1;

		// Remove comments
		szStringWhithoutComments = new char[wOrigStringLen];
		*szStringWhithoutComments = '\0';
		pPlaceToWriteTo = szStringWhithoutComments;
		const char* pPlaceToReadFrom = szGivenString;

		while (TRUE)
		{
			pWasFoundHere = strstr(pPlaceToReadFrom, START_COMMENT);
			// if not found
			if (pWasFoundHere == NIL(char))
			{
				memcpy(pPlaceToWriteTo, pPlaceToReadFrom, strlen(pPlaceToReadFrom) + 1);
				break;
			}
			else
			{
				strncat(pPlaceToWriteTo, pPlaceToReadFrom, pWasFoundHere - pPlaceToReadFrom);
				pPlaceToWriteTo = pPlaceToWriteTo + (pWasFoundHere - pPlaceToReadFrom);
				pPlaceToReadFrom = pWasFoundHere;

				pWasFoundHere = strstr(pPlaceToReadFrom, END_COMMENT);
				if (pWasFoundHere == NIL(char))
					pPlaceToReadFrom = pPlaceToReadFrom + strlen(START_COMMENT);
				else
					pPlaceToReadFrom = pWasFoundHere + strlen(END_COMMENT);
			}
		}
	}

	if (strstr(szGivenString, START_PROLOG) != NIL(char))
	{
		if (wOrigStringLen == 0)
			wOrigStringLen = strlen(szGivenString) + 1;

		// Remove prolog
		ALLOCBUFFER(szStringWhithoutProlog, wOrigStringLen);
		szStringWhithoutProlog[0] = '\0';
		pPlaceToWriteTo = szStringWhithoutProlog;
		if (szStringWhithoutComments)
			pPlaceToReadFrom = szStringWhithoutComments;
		else
			pPlaceToReadFrom = szGivenString;

		pWasFoundHere = NIL(char);

		while (TRUE)
		{
			pWasFoundHere=strstr(pPlaceToReadFrom, START_PROLOG);
			// if not found
			if (pWasFoundHere == NIL(char))
			{
				memcpy(pPlaceToWriteTo, pPlaceToReadFrom, strlen(pPlaceToReadFrom) + 1);
				break;
			}
			else
			{
				strncat(pPlaceToWriteTo, pPlaceToReadFrom, pWasFoundHere - pPlaceToReadFrom);
				pPlaceToWriteTo = pPlaceToWriteTo + (pWasFoundHere - pPlaceToReadFrom);
				pPlaceToReadFrom = pWasFoundHere;

				pWasFoundHere = strstr(pPlaceToReadFrom, END_PROLOG);
				if (pWasFoundHere == NIL(char))
				{
					FPASSERTSTREAM(true, "Unable to find " << END_PROLOG);
					delete [] szStringWhithoutProlog;
					szStringWhithoutProlog = NIL(char);
					break;
				}
				else
				{
					pPlaceToReadFrom = pWasFoundHere + strlen(END_PROLOG);
				}
			}
		}

		DEALLOCBUFFER(szStringWhithoutComments);
		return szStringWhithoutProlog;
	}
	else
	{
		if (szStringWhithoutComments)
    {
      return szStringWhithoutComments;
    }
    else
    {
      wOrigStringLen = strlen(szGivenString) + 1;
      ALLOCBUFFER(szStringToReturn, wOrigStringLen);
      strncpy(szStringToReturn, szGivenString, wOrigStringLen);
      return szStringToReturn;
    }
  }
}

void CXMLDOMNode::GoToNon_EmptyChar(const char ** pPlaceToReadFrom)
{
	if( (pPlaceToReadFrom == NULL) || (*pPlaceToReadFrom == NULL) )
	{
		DBGFPASSERT(1);
		return;
	}

	const unsigned char* szRunningPointer = (unsigned char*)*pPlaceToReadFrom;
	while(TRUE)
	{
		if ( (*szRunningPointer <= ' ') && (*szRunningPointer != '\0') )
		{
			szRunningPointer++;
		}
		else
			break;
	}

	*pPlaceToReadFrom = (char*)szRunningPointer;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BYTE CXMLDOMNode::GetNodeName(const char* *pPlaceToReadFrom)
{
	if(pPlaceToReadFrom == NIL(const char*) || (*pPlaceToReadFrom)==NIL(char))
	{
		DBGFPASSERT(1);
		return FALSE;
	}
	char *strName = new char[MAX_NODE_NAME];
	const char* szRunningPointer = *pPlaceToReadFrom;
	char* pPointeToWrite = strName;
	DWORD  wNumbWrittenChars = 0;
	GoToNon_EmptyChar(&szRunningPointer);
	// Look for a first letter in the name
	if((*szRunningPointer >= 'a' && *szRunningPointer <= 'z') ||
		((*szRunningPointer) == '_' ||
		(*szRunningPointer >= 'A' && *szRunningPointer <= 'Z')))
	{
		*pPointeToWrite = *szRunningPointer;
		pPointeToWrite++;
		szRunningPointer++;
		wNumbWrittenChars++;
	}
	else
	{
        //FPASSERTMSG(TRUE, szRunningPointer);
        //FPASSERTMSG(TRUE, strName);

        DBGFPASSERT(1);
		DEALLOCBUFFER(strName);

		return FALSE;
	}
	/// fill name with next valid chars
	while(wNumbWrittenChars < (MAX_NODE_NAME))
	{
		if((*szRunningPointer >= 'a' && *szRunningPointer <= 'z') ||
			(*szRunningPointer >= 'A' && *szRunningPointer <= 'Z') ||
			(*szRunningPointer >= '0' && *szRunningPointer <= '9') ||
			(*szRunningPointer == '_') || (*szRunningPointer == '-') ||
			(*szRunningPointer == '.') || (*szRunningPointer == '='))
		{

			*pPointeToWrite = *szRunningPointer;
			pPointeToWrite++;
			szRunningPointer++;
			wNumbWrittenChars++;

		}
		else
			break;
	}
	*pPointeToWrite = '\0';
	*pPlaceToReadFrom = (char*)szRunningPointer;

	int length=strlen(strName);

	if (m_szpNodeName)
		DEALLOCBUFFER(m_szpNodeName);

	m_szpNodeName=new char[length+1];
	memcpy(m_szpNodeName,strName,length);
	m_szpNodeName[length]='\0';

	DEALLOCBUFFER(strName);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
CXMLDOMNode* CXMLDOMNode::GetParent()
{
	return m_pParent;
}

//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMNode::SetParent(CXMLDOMNode
							* pParent)
{
	m_pParent = pParent;
}

//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMNode::GetHierarchyString(char* pszHierarchy)
{
	if(m_pParent)
	{
		m_pParent->GetHierarchyString(pszHierarchy);
		strcat(pszHierarchy,"\\");
	}

	strcat(pszHierarchy,m_szpNodeName);
}
//////////////////////////////////////////////////////////////////////////////////
//The size of the new array, with all the special characters.
int CXMLDOMNode::NeededSizeForNewArray(const char* org_str, BOOL isAllocSpaceForSpecialChar)
{
	int new_length = 0;

    if(isAllocSpaceForSpecialChar)
    {
        DWORD len = strlen(org_str);
        for (DWORD i=0; i<len; i++)
        {
            if (org_str[i]=='&')
                new_length+=5;
            else if(org_str[i]=='<')
                new_length+=4;
            else if(org_str[i]=='>')
                new_length+=4;
            else if(org_str[i]=='\'')
                new_length+=6;
            else if(org_str[i]=='"')
                new_length+=6;
            else
                new_length++;
        }
    }
    else
    {
        new_length = strlen(org_str);
    }

    return new_length;
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMNode::ChangeSpecialChar(const char* org_str, BYTE bRemoveSpecialChar, char* new_string)
{
	if (bRemoveSpecialChar)
	{
		DWORD textPointer = strlen(new_string);
		DWORD len = strlen(org_str);
		for (DWORD i=0; i<len; i++)
		{
			if (org_str[i]=='&')
			{
				strcat(new_string, "&amp;");
				textPointer+=5;
				new_string[textPointer]='\0';
			}
			else if(org_str[i]=='<')
			{
				strcat(new_string, "&lt;");
				textPointer+=4;
				new_string[textPointer]='\0';
			}
			else if(org_str[i]=='>')
			{
				strcat(new_string, "&gt;");
				textPointer+=4;
				new_string[textPointer]='\0';
			}
			else if(org_str[i]=='\'')
			{
				strcat(new_string, "&apos;");
				textPointer+=6;
				new_string[textPointer]='\0';
			}
			else if(org_str[i]=='"')
			{
				strcat(new_string, "&quot;");
				textPointer+=6;
				new_string[textPointer]='\0';
			}
			else
			{
				new_string[textPointer] = org_str[i];
				new_string[textPointer+1]='\0';
				++textPointer;
			}
		}
	}
	else
	{
		DWORD textPointer = strlen(new_string);
		DWORD len = strlen(org_str);
		for (DWORD i=0; i<len; i++)
		{
			if (strncmp(org_str+i, "&amp;", 5)==0)
			{
				new_string[textPointer] = '&';
				++textPointer;
				i+=4;
				new_string[textPointer]='\0';
			}
			else if(strncmp(org_str+i, "&lt;", 4)==0)
			{
				new_string[textPointer] = '<';
				++textPointer;
				i+=3;
				new_string[textPointer]='\0';
			}
			else if(strncmp(org_str+i, "&gt;", 4)==0)
			{
				new_string[textPointer] = '>';
				++textPointer;
				i+=3;
				new_string[textPointer]='\0';
			}
			else if(strncmp(org_str+i, "&apos;", 6)==0)
			{
				new_string[textPointer] = '\'';
				++textPointer;
				i+=5;
				new_string[textPointer]='\0';
			}
			else if(strncmp(org_str+i, "&quot;", 6)==0)
			{
				new_string[textPointer] = '\"';
				++textPointer;
				i+=5;
				new_string[textPointer]='\0';
			}
			else
			{
				new_string[textPointer] = org_str[i];
				new_string[textPointer+1]='\0';
				++textPointer;
			}
		}
	}
}

HRES CXMLDOMElement::nextChildNode(CXMLDOMElement **nextItem)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodesNoReset(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->nextNode((CXMLDOMNode **)nextItem);

	return ERR_FAIL;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::nextChildNode(CXMLDOMElement **nextItem,char *pName)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodesNoReset(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->nextNode((CXMLDOMNode **)nextItem,pName);

	return ERR_FAIL;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::firstChildNode(CXMLDOMElement **nextItem)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->nextNode((CXMLDOMNode **)nextItem);

	return ERR_FAIL;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::firstChildNode(CXMLDOMElement **nextItem,char *pName)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->nextNode((CXMLDOMNode **)nextItem,pName);

	return ERR_FAIL;
}

HRES CXMLDOMElement::getChildNodeByName(CXMLDOMElement **Item,char *pName,char *pszRetErrorBuffer,int *status)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodesNoReset(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->getNodeByName((CXMLDOMNode **)Item,pName,pszRetErrorBuffer,status);

	return ERR_FAIL;
}

HRES CXMLDOMElement::getChildNodeValueByName(char *pName,char** name)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->getNodeValueByName(pName,name);

	return ERR_FAIL;
}

HRES CXMLDOMElement::getChildNodeDecValueByName(char *pName,int *Value)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->getNodeDecValueByName(pName,Value);

	return ERR_FAIL;
}

HRES CXMLDOMElement::ResetChildList()
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->reset();

	return ERR_FAIL;
}

HRES CXMLDOMElement::get_nodeType(DOMNodeEnum *type)
{
	*type = ENUM_NODE_ELEMENT;
	return SEC_OK;
}

HRES CXMLDOMElement::get_nodeValue(char** value)
{
	HRES HRES = SEC_OK;
	if(value == NIL(char*))
		HRES = ERR_INVALIDARG;

	if(HRES == SEC_OK && m_pTextValue == NIL(char))
	{
		HRES = SEC_FALSE;
		*value = NIL(char);
	}
	if(HRES == SEC_OK)
		*value = m_pTextValue;

	return HRES;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::get_nodeValue(CObjString &value)
{
	HRES HRES = SEC_OK;

	if(HRES == SEC_OK && m_pTextValue == NIL(char))
	{
		HRES = SEC_FALSE;
		//*value = NIL(char);
	}
	if(HRES == SEC_OK)
	{
		value = m_pTextValue;
	}

	return HRES;
}

CXMLDOMElement::CXMLDOMElement(const char *pName)
{
	set_nodeName(pName);
	m_pTextValue = NIL(char);
	m_pParent = NULL;
}

CXMLDOMElement::CXMLDOMElement(const char *pName,const char *pValue)
{
	set_nodeName(pName);
	SetValue(pValue);
	m_pParent = NULL;
}

CXMLDOMElement::CXMLDOMElement(const char *pName,int nValue)
{
	set_nodeName(pName);
	char pValue[128];
	sprintf(pValue,"%d",nValue);
	SetValue(pValue);
	m_pParent = NULL;
}

CXMLDOMElement::CXMLDOMElement(void)
{
	m_pTextValue = NIL(char);
	m_pParent = NULL;
}

CXMLDOMElement::CXMLDOMElement(const CXMLDOMElement& other)
	:CXMLDOMNode(other)
{
	m_pTextValue = NIL(char);
	m_pParent = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement::~CXMLDOMElement(void)
{
	DEALLOCBUFFER(m_pTextValue);
}

/*
		Purpose: Contains a node list containing the children nodes.
		Parameters:	childList [out, retval] A list of children in the current node.
		Return Values:	SEC_OK Value returned if successful.
						ERR_INVALIDARG Value returned if childList is Null.
*/
HRES CXMLDOMElement::get_childNodes(CXMLDOMNodeList **childList)
{
	HRES hres = SEC_OK;

	if(childList == NIL(CXMLDOMNodeList *))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
	{
		*childList = &m_ListChildElements;
		m_ListChildElements.reset();
	}
	return hres;
}

HRES CXMLDOMElement::get_childNodesNoReset(CXMLDOMNodeList **childList)
{
	HRES hres = SEC_OK;

	if(childList == NIL(CXMLDOMNodeList *))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
		*childList = &m_ListChildElements;

	return hres;
}

/*
		Purpose: Retrieves the value of the attribute.
		Parameters:	name [in] Name of the attribute to return.
			value [out, retval] String that contains the attribute value.
			The empty string is returned if the named attribute does not have a specified or default value.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value when no attribute with the given name is found.
						ERR_INVALIDARG Value returned if name is Null.
*/
HRES CXMLDOMElement::getAttribute(/* [in] */ char* name,/* [retval][out] */ char** value)
{
	HRES hres = SEC_OK;
	if(name == NIL(char))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
	{
		CXMLDOMNode *namedItem = NIL(CXMLDOMNode);
		HRES hReturnValue = m_MapAttributes.getNamedItem(name,&namedItem);
		switch(hReturnValue)
		{
			case SEC_OK:
			{
				char* pNodesValue = NIL(char);
				HRES hReturnValue2 = namedItem->get_nodeValue(&pNodesValue);
				switch(hReturnValue2)
				{
				case SEC_FALSE:
				case SEC_OK:
					*value = pNodesValue;
					break;
				default:
					hres = ERR_INVALIDARG;
				}
				break;
			}
			case SEC_FALSE:
				hres = SEC_FALSE;
				break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}
	return hres;
}

/*
		Purpose: Retrieves the attribute node
		Parameters:	name [in] Name of the attribute to be retrieved.
					attributeNode [out, retval] CXMLDOMAttribute object that is
					returned with the supplied name, or Null if the named attribute
					cannot be found on this element.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned when no attribute with the given name is found.
						ERR_INVALIDARG Value returned if name is Null.
*/
HRES CXMLDOMElement::getAttributeNode( char* name, CXMLDOMAttribute **attributeNode)
{
	HRES hres = SEC_OK;
	if(name == NIL(char))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
	{
		CXMLDOMNode *namedItem = NIL(CXMLDOMNode);
		HRES hReturnValue = m_MapAttributes.getNamedItem(name,&namedItem);
		switch(hReturnValue)
		{
		case SEC_OK:
			*attributeNode = (CXMLDOMAttribute *)namedItem;
			break;
		case SEC_FALSE:
			*attributeNode = NIL(CXMLDOMAttribute);
			hres = SEC_FALSE;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		}

	}
	return hres;
}

void CXMLDOMElement::AddChildNode(CXMLDOMNode* element)
{
	if(element)
		((CXMLDOMElement*)element)->SetParent(this);

	HRES hRes = m_ListChildElements.addNode(element);
	if(hRes != SEC_OK)
		DBGFPASSERT(1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element)
		element->SetParent(this);

	HRES hRes = m_ListChildElements.addNode(element);
	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}
	return element;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement* CXMLDOMElement::AddChildNodeWithNsPrefix(const char *pChildName, const char *pszNsPrefix)
{
	std::string sTmp(pszNsPrefix);
	sTmp += pChildName;

	return AddChildNode(sTmp.c_str());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement* CXMLDOMElement::AddChildNode(const std::string& sChildName)
{
	return AddChildNode(sChildName.c_str());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement* CXMLDOMElement::AddChildNodeWithNsPrefix(const std::string& sChildName, const std::string &sNsPrefix)
{
	return AddChildNodeWithNsPrefix(sChildName.c_str(), sNsPrefix.c_str());
}

CXMLDOMElement* CXMLDOMElement::AddChildNodeWithNsPrefix(const char *pChildName, const char *pszNsPrefix, DWORD nValue,	int type /*= -1*/)
{
	std::string sTmp(pszNsPrefix);
	sTmp += pChildName;

	return AddChildNode(sTmp.c_str(), nValue, type);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,DWORD nValue,int type)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element)
	{
		element->SetParent(this);
		if (type!=-1)
		{
			const char *pDescription;
			if(type==IP_ADDRESS)
			{
				delete element;
				return AddIPChildNode(pChildName,nValue);
			}
			else if(CStringsMaps::GetDescription(type,nValue,&pDescription))
			{
				if (pDescription&&(strcmp("MIN",pDescription)==0||strcmp("MAX",pDescription)==0))
				{
					element->SetValue(nValue);
				}
				else
					element->SetValue(pDescription);
			}
			else
			{

				ALLOCBUFFER(msg,356);
				memset(msg ,356,'\0');

				int nMaxLen=0,nMinLen=0;
				if (CStringsMaps::IsDefineType(type))
				{
					//				BYTE bMaxRestricted=FALSE; VNGR-21606 - approved with David Rabkin

					if (CStringsMaps::GetValue(type,nMaxLen,"MAX"))
					{
						//					bMaxRestricted=TRUE; VNGR-21606 - approved with David Rabkin
						//its probably a range value lets find if the added value has the right range
						if (nValue > (DWORD)nMaxLen )
						{
							CreateErrorMessage_ForAddFuncs
							(STATUS_VALUE_OUT_OF_RANGE,msg,pChildName,type,nMinLen,nMaxLen,nValue);

							FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::AddChildNode (DWORD)  ", msg);
							DBGFPASSERT(1);

						}

					}
					if (CStringsMaps::GetValue(type,nMinLen,"MIN"))
					{
						if (nValue < (DWORD)nMinLen )
						{
							CreateErrorMessage_ForAddFuncs
							(STATUS_VALUE_OUT_OF_RANGE,msg,pChildName,type,nMinLen,nMaxLen,nValue);

							FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::AddChildNode (DWORD)  ", msg);
							DBGFPASSERT(1);

						}

					}
				}
				else
				{
					CreateErrorMessage_ForAddFuncs(STATUS_ENUM_VALUE_INVALID,msg,pChildName,type,nMinLen,nMaxLen,nValue);
					FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::AddChildNode (DWORD)  ", msg);
					DBGFPASSERT(1);
				}

				element->SetValue(nValue);
				DEALLOCBUFFER(msg);
			}
		}
		else
		{
			char pValue[128];
			//itoa(nValue,pValue,10);
			sprintf(pValue,"%d",(int)nValue);
			element->SetValue(pValue);
		}

		HRES hRes = m_ListChildElements.addNode(element);
		if(hRes != SEC_OK)
		{
			DBGFPASSERT(1);
			delete element;
			return NULL;
		}
		return element;
	}
	else
	{
		DBGFPASSERT(2);
		return NULL;
	}
}


CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,int nValue,int type)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element)
	{
		element->SetParent(this);
		if (type!=-1)
		{
			const char *pDescription;
			if(type==IP_ADDRESS)
			{
				delete element;
				return AddIPChildNode(pChildName,nValue);
			}
			else if(CStringsMaps::GetDescription(type,nValue,&pDescription))
			{
				if (pDescription&&(strcmp("MIN",pDescription)==0||strcmp("MAX",pDescription)==0))
				{
					element->SetValue(nValue);
				}
				else
					element->SetValue(pDescription);
			}
			else
			{

				ALLOCBUFFER(msg,356);
				memset(msg ,356,'\0');

				int nMaxLen=0,nMinLen=0;
				if (CStringsMaps::IsDefineType(type))
				{
					//				BYTE bMaxRestricted=FALSE; VNGR-21606 - approved with David Rabkin

					if (CStringsMaps::GetValue(type,nMaxLen,"MAX"))
					{
						//					bMaxRestricted=TRUE; VNGR-21606 - approved with David Rabkin
						//its probably a range value lets find if the added value has the right range
						if ((nValue!= -1) && (DWORD)nValue > (DWORD)nMaxLen )
						{
							CreateErrorMessage_ForAddFuncs
							(STATUS_VALUE_OUT_OF_RANGE,msg,pChildName,type,nMinLen,nMaxLen,nValue);

							FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::AddChildNode (DWORD)  ", msg);
							DBGFPASSERT(1);

						}

					}
					if (CStringsMaps::GetValue(type,nMinLen,"MIN"))
					{
						if ( (nValue!= -1) && nValue < nMinLen )
						{
							CreateErrorMessage_ForAddFuncs
							(STATUS_VALUE_OUT_OF_RANGE,msg,pChildName,type,nMinLen,nMaxLen,nValue);

							FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::AddChildNode (DWORD)  ", msg);
							DBGFPASSERT(1);

						}

					}
				}
				else
				{
					CreateErrorMessage_ForAddFuncs(STATUS_ENUM_VALUE_INVALID,msg,pChildName,type,nMinLen,nMaxLen,nValue);
					FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::AddChildNode (DWORD)  ", msg);
					DBGFPASSERT(1);
				}

				element->SetValue(nValue);
				DEALLOCBUFFER(msg);
			}
		}
		else
		{
			char pValue[128];
			//itoa(nValue,pValue,10);
			sprintf(pValue,"%d",(int)nValue);
			element->SetValue(pValue);
		}

		HRES hRes = m_ListChildElements.addNode(element);
		if(hRes != SEC_OK)
		{
			DBGFPASSERT(1);
			delete element;
			return NULL;
		}
		return element;
	}
	else
	{
		DBGFPASSERT(2);
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////
CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,WORD nValue)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element == NULL)
		return element;

	element->SetParent(this);
	element->SetValue(nValue);

	HRES hRes = m_ListChildElements.addNode(element);
	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}
	return element;

}

//////////////////////////////////////////////////////////////////////////////////
CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,int nValue)
{
	DWORD dw=nValue;
	return AddChildNode(pChildName,dw,-1);

}

CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,long nValue)
{
	DWORD dw=nValue;
	return AddChildNode(pChildName,dw,-1);

}

CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,const char *pValue, BOOL isRmSpecialChar)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element == NULL)
		return element;
		element->SetParent(this);

	element->SetValue(pValue, isRmSpecialChar);

	HRES hRes = m_ListChildElements.addNode(element);
	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}
	return element;

}

CXMLDOMElement* CXMLDOMElement::AddChildNodeWithNsPrefix(const char *pChildName, const char *pszNsPrefix, const char *pValue, BOOL isRmSpecialChar)
{
	std::string sTmp(pszNsPrefix);
	sTmp += pChildName;

	return AddChildNode(sTmp.c_str(), pValue, isRmSpecialChar);

}

//////////////////////////////////////////////////////////////////////////////////
CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,CObjString &strValue)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element == NULL)
		return element;
		element->SetParent(this);

	element->SetValue(strValue.GetString());

	HRES hRes = m_ListChildElements.addNode(element);
	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}
	return element;

}

CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,
                                             const COstrStream &strStream)
{
	return AddChildNode(pChildName,strStream.str());
}

//////////////////////////////////////////////////////////////////////////////////
CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,const std::string &strValue)
{
	CXMLDOMElement *element = new CXMLDOMElement(pChildName);

	if(element == NULL)
		return element;
		element->SetParent(this);

	//element->SetValue(strValue.GetString());
	element->SetValue(strValue.data());

	HRES hRes = m_ListChildElements.addNode(element);
	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}
	return element;

}

CXMLDOMElement* CXMLDOMElement::AddChildNode(const char *pChildName,const CStructTm &time)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element)
	{
		element->SetParent(this);

		char szTime[256];

		int Year=time.m_year;
		if (Year<1900)
			Year+=1900;

		sprintf(szTime,"%d-%2d-%2dT%2d:%2d:%2d",Year,time.m_mon,time.m_day,time.m_hour,time.m_min,time.m_sec);
		int len=strlen(szTime);
		for (int i=0;i<len;i++)
		{
			if (szTime[i]==' ')
				szTime[i]='0';
		}

		element->SetValue(szTime);

		HRES hRes = m_ListChildElements.addNode(element);
		if(hRes != SEC_OK)
		{
			DBGFPASSERT(1);
			delete element;
			return NULL;
		}
		return element;
	}
	else
	{
		DBGFPASSERT(2);
		return NULL;
	}
}

CXMLDOMElement* CXMLDOMElement::AddIPChildNode(const char *pChildName,DWORD dwIP)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element == NULL)
		return element;
		element->SetParent(this);

	char szIP[20];
	SystemDWORDToIpString(dwIP, szIP);

	element->SetValue(szIP);
	HRES hRes = m_ListChildElements.addNode(element);

	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}

	return element;
}

CXMLDOMElement* CXMLDOMElement::AddIPChildNode(const char *pChildName,mcTransportAddress IPAddr,BYTE bIsIpV6)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element == NULL)
		return element;
		element->SetParent(this);

	char szIP[64];
	if ((bIsIpV6 == FALSE && IPAddr.ipVersion == (APIU32)eIpVersion4) ||  (bIsIpV6 &&IPAddr.ipVersion == (APIU32)eIpVersion6))
	{
		ipToString(IPAddr, szIP,0); // With Brackets
		element->SetValue(szIP);
	}
	else
	{
		if (bIsIpV6 == FALSE)
			element->SetValue("0.0.0.0");
		else
			element->SetValue("::");
	}

	HRES hRes = m_ListChildElements.addNode(element);

	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}

	return element;
}

CXMLDOMElement* CXMLDOMElement::AddIPv6ChildNode(const char *pChildName, const APIU8* iPv6Addr6, const char *pMask, BOOL brackets/*=FALSE*/)
{
	CXMLDOMElement *element=new CXMLDOMElement(pChildName);

	if(element == NULL)
		return NULL;
		element->SetParent(this);

	char szIP[IPV6_ADDRESS_LEN];
	memset(szIP, 0, IPV6_ADDRESS_LEN);
	::ipV6ToString(iPv6Addr6, szIP , brackets);

	if (!pMask)
	{
		element->SetValue(szIP);
	}
	else
	{
	  char szFullIP[ (IPV6_ADDRESS_LEN+IPV6_ADDRESS_LEN+16) ];
		memset(szFullIP, 0, (IPV6_ADDRESS_LEN+IPV6_ADDRESS_LEN+16) );

		// VNGR-23764 - change back from  snprintf to sprintf, maybe cut ipv6 adress caused the loop, added 16 chars to prevent overflow (this was the only change that may cause the issue)
		//snprintf(szFullIP, sizeof(szFullIP), "%s/%s", szIP, pMask);
		//sprintf(szFullIP, "%s/%s", szIP, pMask);

		//Chnage back to snprintf to solve KW issue. We think the + 16 that were added is enough in this case (judith & Nati)
		snprintf(szFullIP, sizeof(szFullIP), "%s/%s", szIP, pMask);


		element->SetValue(szFullIP);
	}

	HRES hRes = m_ListChildElements.addNode(element);

	if(hRes != SEC_OK)
	{
		DBGFPASSERT(1);
		delete element;
		return NULL;
	}

	return element;
}

BYTE CXMLDOMElement::GetElementText(const char**pPlaceToReadFrom)
{
	if(pPlaceToReadFrom == NIL(const char*) || (*pPlaceToReadFrom)==NIL(char))
	{
		DBGFPASSERT(1);
		return FALSE;
	}
	const char* szRunningPointer = *pPlaceToReadFrom;

	GoToNon_EmptyChar(&szRunningPointer);
	if(*szRunningPointer == '<' || *szRunningPointer == '>')
	{
		*pPlaceToReadFrom = szRunningPointer;
		// no text
		return TRUE;
	}
	const char* szStartText = *pPlaceToReadFrom;
	const char* szEndText =  *pPlaceToReadFrom;
	while(TRUE)
	{
		if(*szEndText == '<' && *(szEndText+1) == '/')
			break;
		if(*szEndText == '\0')
			break;
		szEndText++;
	}
	// suppose '<' exists

	if(m_pTextValue == NIL(char))
	{
		DWORD wSize = szEndText - szStartText + 1;

		m_pTextValue = new char[wSize];

		memset(m_pTextValue, '\0', wSize);

		char* org_str = new char[wSize];
		memcpy(org_str,szStartText,wSize-1);
		org_str[wSize-1]='\0';
		strcpy(m_pTextValue,org_str);

		PDELETEA(org_str);
	}
	else
		DBGFPASSERT(1);

	*pPlaceToReadFrom = szEndText;

	return TRUE;
}

BYTE CXMLDOMElement::GetChildNodes(const char**pPlaceToReadFrom)
{
	if(pPlaceToReadFrom == NIL(const char*) || (*pPlaceToReadFrom)==NIL(char))
		return FALSE;
	const char* szRunningPointer = *pPlaceToReadFrom;

	HRES HRES = SEC_OK;
	while(TRUE)
	{
		GoToNon_EmptyChar(&szRunningPointer);
		if(*szRunningPointer!='<')
		{
			DBGFPASSERT(1);
			return FALSE;
		}
		else
		{
			if(*(szRunningPointer+1) == '/')
				break;
		}
		GoToNon_EmptyChar(&szRunningPointer);
		CXMLDOMElement* pNewChildElement = new CXMLDOMElement;
		HRES = pNewChildElement->Parse(&szRunningPointer);
		if(HRES != SEC_OK)
		{
			// parsing attbi fails
			DBGFPASSERT(1);
			delete pNewChildElement;
			return FALSE;
		}
		else
			// success
			AddChildNode(pNewChildElement);

	}
	*pPlaceToReadFrom = szRunningPointer;
	return TRUE;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CXMLDOMElement::AddAttribute(const CXMLDOMAttribute* attribite)
{
	HRES hRes = m_MapAttributes.setNamedItem((CXMLDOMNode *)attribite);
	if(hRes != SEC_OK)
		DBGFPASSERT(1);
}

void CXMLDOMElement::AddAttribute(const char* pszName, const char* pszValue)
{
	CXMLDOMAttribute* pAttribute = new CXMLDOMAttribute();
	pAttribute->set_nodeName(pszName);
	pAttribute->SetValueForElement(pszValue);
	AddAttribute(pAttribute);

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BYTE CXMLDOMElement::GetAttributes(const char**pPlaceToReadFrom)
{
	if(pPlaceToReadFrom == NIL(const char*) || (*pPlaceToReadFrom)==NIL(char))
	{
		DBGFPASSERT(1);
		return FALSE;
	}
	const char* szRunningPointer = *pPlaceToReadFrom;


	HRES HRES = SEC_OK;
	while(TRUE)
	{
		GoToNon_EmptyChar(&szRunningPointer);
		if(*szRunningPointer == '/')
			break;
		if((*szRunningPointer >= 'a' && *szRunningPointer <= 'z') ||
			(*szRunningPointer >= 'A' && *szRunningPointer <= 'Z')
			|| ((*szRunningPointer) == '_')){
			CXMLDOMAttribute* pNewAttribute = new CXMLDOMAttribute;
			HRES = pNewAttribute->Parse(&szRunningPointer);
			if(HRES != SEC_OK)
			{
				// parsing attbi fails
				DBGFPASSERT(1);
				delete pNewAttribute;
				return FALSE;
			}
			else
				// success
				AddAttribute(pNewAttribute);
		}
		else
			break;
	}
	*pPlaceToReadFrom = szRunningPointer;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMElement::SetValue(const char* ezText, BOOL isRmSpecialChar)
{
	if(ezText == NULL)
		return;

	DEALLOCBUFFER(m_pTextValue);

	int len = NeededSizeForNewArray(ezText, isRmSpecialChar);
	m_pTextValue = new char[len+1];
	memset(m_pTextValue, '\0', len+1);

    if(isRmSpecialChar)
    {
        ChangeSpecialChar(ezText, TRUE, m_pTextValue);
    }
    else
    {
        strncpy(m_pTextValue, ezText, len);
        m_pTextValue[len] = '\0';
    }
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMElement::SetValue(int nValue)
{
	char pValue[128];
	//itoa(nValue,pValue,10);
	sprintf(pValue,"%d",nValue);
	SetValue(pValue);
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMElement::SetValue(DWORD nValue)
{
	char pValue[128];
	//ultoa(nValue,pValue,10);
	sprintf(pValue,"%d",(int)nValue);
	SetValue(pValue);
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMElement::SetValue(long nValue)
{
	char pValue[128];
	//ultoa(nValue,pValue,10);
	sprintf(pValue,"%d",(int)nValue);
	SetValue(pValue);
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMElement::SetValue(WORD nValue)
{
	char pValue[128];
	int nIntValue;

	memset((void*)(&nIntValue),0,sizeof(int));
	nIntValue = nValue;


	sprintf(pValue,"%d",(int)nValue);
	SetValue(pValue);
}
//////////////////////////////////////////////////////////////////////////////////
void CXMLDOMElement::SetValue(BYTE nValue)
{
	char pValue[128];
	int nIntValue;

	memset((void*)(&nIntValue),0,sizeof(int));
	nIntValue = nValue;

	sprintf(pValue,"%d",(int)nValue);
	SetValue(pValue);
}
//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::AddStringToDump(CStrArray* strArray,char* longstr,int &index,char *pStr)
{
	if (pStr == NULL)
		return SEC_OK;
	int lenStr=strlen(pStr);


	if (longstr!=NULL)  //add dump to longstring
	{
		memcpy(longstr + index,pStr,lenStr);
		index=index+lenStr;
	}
	else //add dump to strArray
	{
			if(strArray->Add(pStr,lenStr)==SEC_FALSE)
				return SEC_FALSE;
	}

	return SEC_OK;
}

BYTE CXMLDOMElement::MsValidate(char ** pstrError)
{
	BYTE retVal=TRUE;
	*pstrError=NULL;
#ifdef WINNT
#ifdef __HIGHC__
	if(::GetpSystemCfg()->GetMsxmlValidator())
	{
		char *strResponseName;
		if (get_nodeName(&strResponseName)==SEC_OK)
		{
			CMSValidator validator;
			char *pStrXml;

			if(DumpDataAsLongStringEx(&pStrXml) == SEC_OK)
			{
				if (validator.Validate(pStrXml,strResponseName,strResponseName)==FALSE)
				{
					char *err=validator.GetErrorString();
					int len=strlen(err);
					*pstrError=new char [len+1];
					strcpy(*pstrError,err);
					retVal=FALSE;
				}

				//check when to delete (only if status==ok?)
				delete [] pStrXml;
			}


		}
	}
#endif
#endif
	return retVal;

}

int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,mcTransportAddress* pValue,char *pszError,int type)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type);

	return ERR_FAIL;
}

int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,std::string &pValue,char *pszError,int type, bool isAscii)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type, isAscii);

	return ERR_FAIL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,CObjString& strValue,char *pszError,int type, bool isAscii)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,strValue,pszError,type, isAscii);

	return ERR_FAIL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,char **pValue,char *pszError,int type, bool isAscii)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type, isAscii);

	return ERR_FAIL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,char* pValue,char *pszError,int type, bool isAscii)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type, isAscii);

	return ERR_FAIL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,DWORD* pValue,char *pszError,int type)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type);

	return ERR_FAIL;
}
//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,WORD* pValue,char *pszError,int type)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type);

	return ERR_FAIL;
}
//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,BYTE* pValue,char *pszError,int type)
{
	CXMLDOMNodeList* pChildElementsList = NULL;
	int nStatus;
	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
	{
		 nStatus = pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type);
		return nStatus;
	}

	return ERR_FAIL;
}
//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,CStructTm* time,char *pszError,int type)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,time,pszError,type);

	return ERR_FAIL;
}
//////////////////////////////////////////////////////////////////////////////////
int CXMLDOMElement::GetAndVerifyChildNodeValue(char* pszNodeName,int* pValue,char *pszError,int type)
{
	CXMLDOMNodeList* pChildElementsList = NULL;

	get_childNodes(&pChildElementsList);

	if(pChildElementsList)
		return pChildElementsList->GetAndVerifyNodeValue(pszNodeName,pValue,pszError,type);

	return ERR_FAIL;
}

DWORD CXMLDOMElement::GetDumpStringLength(int offset, BYTE bPrettyXml/* = FALSE*/)
{
	DWORD length;

	CXMLDOMNode *namedItem = NIL(CXMLDOMNode);

	m_ListChildElements.reset();
	m_ListChildElements.nextNode(&namedItem);

	length = 1 + strlen(m_szpNodeName); //"<" and m_szpNodeName
	if (bPrettyXml==TRUE)
		length += 2 * offset + 2; // 2 set of tabs, 2 new lines

	if(m_pTextValue)
	{
		length++; //">"
		length += strlen(m_pTextValue);
	}
	else
	{
		if(namedItem==NULL)
		{
			length += 2; //"/>"
			return length;
		}
		else
		{
			length += 1; //">"
		}
	}

	while(namedItem)
	{
		length += namedItem->GetDumpStringLength(offset + 1, bPrettyXml);
		namedItem = namedItem->m_pNextSibling;
	}

	length += 2; //"</"
	length += strlen(m_szpNodeName);
	length += 1; //">"
	return length;
}

HRES CXMLDOMElement::DumpDataAsLongStringEx(char** longstr, BYTE bPrettyXml)
{
	DWORD totalLength = GetDumpStringLength(0, bPrettyXml) + 1;

	ALLOCBUFFER(szResultString, totalLength + 2);

	int index=0;

	HRES result = DumpDataAsStringImpl(NULL, szResultString, 0, index, bPrettyXml);
//	szResultString[totalLength+1] = '\0';

	*longstr = szResultString;

	return result;
}
//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::DumpDataAsStringArrayEx(CStrArray& strArray,
                                             WORD &CompressionLevel,
                                             BYTE bResetArray/*=TRUE*/,
                                             BYTE bPrettyXml/*=FALSE*/)
{
	int row=0;
	HRES result;

	if (bResetArray!=FALSE)
		strArray.Reset();

	strArray.SetCompressionLevel(CompressionLevel);

	result=DumpDataAsStringImpl(&strArray, NULL, 0, row, bPrettyXml);

	strArray.Finalize();
	CompressionLevel=strArray.GetCompressionLevel();

	return result;
}

HRES CXMLDOMElement::DumpDataAsStringImpl(CStrArray* strArray,
                                          char* longstr,
                                          DWORD offset,
                                          int &index,
                                          BYTE bPrettyXml)
{
	//either send a stringarray or a long string to this function but not both
	CXMLDOMNode *namedItem = NIL(CXMLDOMNode);

	m_ListChildElements.reset();
	m_ListChildElements.nextNode(&namedItem);

	if (bPrettyXml==TRUE)
	{
		for(DWORD i = 0 ; i < offset ; i++)
		{
			if(AddStringToDump(strArray,longstr,index,"\t")==SEC_FALSE)
				return SEC_FALSE;
		}
	}

	if(AddStringToDump(strArray,longstr,index,"<")==SEC_FALSE)
		return SEC_FALSE;

	if(AddStringToDump(strArray,longstr,index,m_szpNodeName)==SEC_FALSE)
		return SEC_FALSE;

	if(m_pTextValue)
	{
		if(AddStringToDump(strArray,longstr,index,">")==SEC_FALSE)
			return SEC_FALSE;
		if(AddStringToDump(strArray,longstr,index,m_pTextValue)==SEC_FALSE)
			return SEC_FALSE;
	}
	else
	{
		if(namedItem==NULL)
		{
			return AddStringToDump(strArray,longstr,index,"/>");

		}
		else
		{
			if(AddStringToDump(strArray,longstr,index,">")==SEC_FALSE)
				return SEC_FALSE;
		}
	}

	DWORD numOfChilds = 0;
	while(namedItem)
	{
		if (bPrettyXml==TRUE)
		{
			if(AddStringToDump(strArray,longstr,index,"\n")==SEC_FALSE)
				return SEC_FALSE;
		}

		if (namedItem->DumpDataAsStringImpl(strArray,longstr, offset + 1, index, bPrettyXml)==SEC_FALSE)
			return SEC_FALSE;

		namedItem = namedItem->m_pNextSibling;
		numOfChilds++;
	}

	if(numOfChilds > 0 && bPrettyXml==TRUE)
	{
		if(AddStringToDump(strArray,longstr,index,"\n")==SEC_FALSE)
			return SEC_FALSE;

		for(DWORD i = 0 ; i < offset ; i++)
		{
			if(AddStringToDump(strArray,longstr,index,"\t")==SEC_FALSE)
				return SEC_FALSE;
		}
	}

	if(AddStringToDump(strArray,longstr,index,"</")==SEC_FALSE)
		return SEC_FALSE;
	if(AddStringToDump(strArray,longstr,index,m_szpNodeName)==SEC_FALSE)
		return SEC_FALSE;
	if(AddStringToDump(strArray,longstr,index,">")==SEC_FALSE)
		return SEC_FALSE;
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////////////////
HRES CXMLDOMElement::DumpDataAsStringImpl(std::ostream &ostr,
                                          DWORD offset,
                                          BYTE  bPrettyXml)
{
	//either send a stringarray or a long string to this function but not both
	CXMLDOMNode *namedItem = NIL(CXMLDOMNode);

	m_ListChildElements.reset();
	m_ListChildElements.nextNode(&namedItem);

	if (bPrettyXml==TRUE)
	{
		for(DWORD i = 0 ; i < offset ; i++)
		{
            ostr << "\t";
		}
	}

    ostr << "<" ;
    ostr << m_szpNodeName;


	if(m_pTextValue)
	{
        ostr << ">";
        ostr << m_pTextValue;
	}
	else
	{
		if(namedItem==NULL)
		{
            ostr << "/>";
            return SEC_OK;
		}
		else
		{
            ostr << ">";
		}
	}

	DWORD numOfChilds = 0;
	while(namedItem)
	{
		if (bPrettyXml==TRUE)
		{
            ostr <<  std::endl;
		}

		namedItem->DumpDataAsStringImpl(ostr,
                                        offset + 1,
                                        bPrettyXml);

		namedItem = namedItem->m_pNextSibling;
		numOfChilds++;
    }

	if(numOfChilds > 0 && bPrettyXml==TRUE)
	{
        ostr << std::endl;

		for(DWORD i = 0 ; i < offset ; i++)
		{
            ostr << "\t";
		}
	}

    ostr << "</";
    ostr << m_szpNodeName;
    ostr << ">";

	return SEC_OK;
}


STATUS CXMLDOMElement::WriteXmlFile(const char *file_name)
{
	STATUS status = STATUS_OK;
	CStrArray strArray;
	WORD CompressionLevel = 0;

	if( DumpDataAsStringArrayEx(strArray,CompressionLevel,TRUE,TRUE) == SEC_OK )
	{
		FILE*  infile = fopen(file_name,"w+");	// w+ Delete all file content and enable writing it

		if( infile )
		{
			DWORD numOfAllocBuffers = strArray.GetNumOfAllocatedBuffers();
			DWORD totalAllocLength	= strArray.GetTotalAllocatedLength();

			for (DWORD i = 0 ; i < numOfAllocBuffers ; i++)
			{
				if( strArray[i] != NULL )
				{
					// the last chunk is not full
					DWORD dwStrLen = (	numOfAllocBuffers - 1 == i
										?
										totalAllocLength % DUMP_BUF_SIZE : DUMP_BUF_SIZE);

					DWORD iWasWritten = fwrite(strArray[i],sizeof(char),dwStrLen,infile);

					if( iWasWritten != dwStrLen )
					{
						status = STATUS_WRITE_FILE_FAILED;
						FTRACESTR(eLevelError) << "CXMLDOMElement::WriteXmlFile: failed to write data, file = " << file_name;
					}
				}
				else
					break;
			}
			int fcloseReturn = fclose(infile);
			if (FCLOSE_SUCCESS != fcloseReturn)
			{
				FTRACESTR(eLevelError) << "CXMLDOMElement::WriteXmlFile: failed to close file = "<< file_name;
				status = STATUS_CLOSE_FILE_FAILED;
			}
		}
		else
		{
			FTRACESTR(eLevelError) << "CXMLDOMElement::WriteXmlFile: failed open file - " << file_name;
			status = STATUS_OPEN_FILE_FAILED;
		}
 	}
	else
	{
		FTRACESTR(eLevelError) << "CXMLDOMElement::WriteXmlFile: failed to dump file data as string";
		status = STATUS_DUMP_XML_DATA_AS_STRING_FAILED;
	}
	return status;

}

/* Assume szStringToParse points to '<'
		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Parsing fail
*/
HRES CXMLDOMElement::Parse(const char** szStringToParse)
{
    if(szStringToParse == NIL(const char*) || *szStringToParse == NIL(char) )
	{
		DBGFPASSERT(1);
		return SEC_FALSE;
	}
	// move it one step ahead


	const char* pPlaceToReadFrom = *szStringToParse;
	GoToNon_EmptyChar(&pPlaceToReadFrom);

	if(*pPlaceToReadFrom != '<')
	{
		DBGFPASSERT(1);
		return SEC_FALSE;
	}

	pPlaceToReadFrom++;

	if(!GetNodeName(&pPlaceToReadFrom))
	{
		DBGFPASSERT(1);
		return SEC_FALSE;
	}
	if(!GetAttributes(&pPlaceToReadFrom))
	{
		DBGFPASSERT(1);
		return SEC_FALSE;
	}
	GoToNon_EmptyChar(&pPlaceToReadFrom);
	if(*pPlaceToReadFrom == '>')
	{ // closing element name section
		*pPlaceToReadFrom++;

		if(!GetElementText(&pPlaceToReadFrom))
		{
			DBGFPASSERT(1);
			return SEC_FALSE;

		}
		if(!GetChildNodes(&pPlaceToReadFrom))
		{
			DBGFPASSERT(1);
			return SEC_FALSE;

		}
		GoToNon_EmptyChar(&pPlaceToReadFrom);
		// look for closing </element_name>
		if(strncmp(pPlaceToReadFrom,"</",2) == 0)
		{
			pPlaceToReadFrom = pPlaceToReadFrom+2;
			if(!strncmp((pPlaceToReadFrom),m_szpNodeName,strlen(m_szpNodeName)))
			{
				const char* pFindCloser = pPlaceToReadFrom;
				pFindCloser = strchr( pFindCloser, '>' );
				if(pFindCloser != NIL(char))
				{
					pPlaceToReadFrom = pFindCloser;
					pPlaceToReadFrom++;
				}
			}
		}
		else
		{
			DBGFPASSERT(1);
			return SEC_FALSE;
		}

	}
	else if((*pPlaceToReadFrom == '/') && (*(pPlaceToReadFrom+1)=='>'))
		pPlaceToReadFrom = pPlaceToReadFrom + 2;
	else
	{
		FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::Parse", *szStringToParse);
		DBGFPASSERT(1);
		return SEC_FALSE;
	}
	* szStringToParse = pPlaceToReadFrom;

	return SEC_OK;
}

/*
		Purpose: Contains the element name.
		Parameters:	tagName [out, retval] String that represents the element's name.
			For example, the tag name is "book" in the following tag.
			<book ISBN="1572318546">
		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value when returning Null.
						ERR_INVALIDARG Value returned if tagName is Null.
*/
HRES CXMLDOMElement::get_tagName(/* [retval][out] */ char** tagName)
{
	HRES hres = SEC_OK;
	if(tagName == NIL(char*))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
		*tagName = m_szpNodeName;

	return hres;

}

/*
		Purpose: Contains the first child of the node.
		Parameters:	firstChild [out, retval] First child node. If there are no such children, it returns Null.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned when there are no children.
						ERR_INVALIDARG Value returned if firstChild is Null.
*/
HRES  CXMLDOMElement::get_firstChild(/* [retval][out] */ CXMLDOMNode **firstChild)
{
	HRES hres = SEC_OK;
	if(firstChild == NIL(CXMLDOMNode *))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
	{
		m_ListChildElements.reset();
		CXMLDOMNode* pIXMLDOMNode = NIL(CXMLDOMNode);
		HRES hResList = m_ListChildElements.nextNode(&pIXMLDOMNode);
		switch(hResList)
		{
		case SEC_OK:
			*firstChild = pIXMLDOMNode;
			break;
		case ERR_INVALIDARG:
			*firstChild = NIL(CXMLDOMNode);
			hres = SEC_FALSE;
			break;
		default:
			hres = ERR_INVALIDARG;
		}
	}
	return hres;
}

void CXMLDOMElement::DumpDataAsStringWithAttribute(/* [retval][out] */char** szDumpStr,
                                      /* [retval][out] */ DWORD* dwStrLen, DWORD offset, BYTE bPrettyXml)
{
	long wNumbAttrib = 0;
	m_MapAttributes.get_length(&wNumbAttrib);
	long wNumbChilds = 0;
	m_ListChildElements.get_length(&wNumbChilds);

	// read a data from all attributes
	char** ppArrayAttrib = NULL;
	DWORD wAttrinStringLenght = 0;
	if( wNumbAttrib > 0)
	{
		m_MapAttributes.reset();
		// array for saving strings from every attribite
		ppArrayAttrib = new char*[wNumbAttrib];
		memset(ppArrayAttrib,0,wNumbAttrib);
		DWORD dwLength = 0;
		DWORD i=0;
		CXMLDOMNode* pCurrentNode;
		m_MapAttributes.nextNode(&pCurrentNode);

		// run over all attributes and get their data
		while(pCurrentNode!=NULL)
		{
			pCurrentNode->DumpDataAsStringWithAttribute(&ppArrayAttrib[i],&dwLength, offset+1, bPrettyXml);
			wAttrinStringLenght = wAttrinStringLenght + dwLength;
			m_MapAttributes.nextNode(&pCurrentNode);
			i++;
		}
	}
	// now the same with childs nodes:
	char** ppArrayChilds = NULL;
	DWORD dwChildsStringLenght = 0;
	if(wNumbChilds>0)
	{
		m_ListChildElements.reset();
		ppArrayChilds = new char*[wNumbChilds];
		DWORD dwLength = 0;
		CXMLDOMNode* pCurrentNode;

		m_ListChildElements.nextNode(&pCurrentNode);
		DWORD i=0;

		while(pCurrentNode!=NULL)
		{
			pCurrentNode->DumpDataAsStringWithAttribute(&ppArrayChilds[i],&dwLength, offset+1, bPrettyXml);
			dwChildsStringLenght = dwChildsStringLenght + dwLength;
			m_ListChildElements.nextNode(&pCurrentNode);
			i++;
		}
	}
	// now create your own string:
	DWORD nodeNameLen = strlen(m_szpNodeName);
	DWORD dwLenOfResultString = nodeNameLen+
						nodeNameLen + wAttrinStringLenght+ dwChildsStringLenght+200;

	if(m_pTextValue!=NULL)
		dwLenOfResultString = dwLenOfResultString+strlen(m_pTextValue);

	DWORD dwLenWasPrinted = 0;
	ALLOCBUFFER(szResultString,dwLenOfResultString);
	dwLenWasPrinted = dwLenWasPrinted + nodeNameLen+10;

	// print name of the node
	//sprintf(szResultString,"<%s",m_szpNodeName);
	DWORD len = 0;

	if (bPrettyXml==TRUE)
	{
		for(DWORD i = 0 ; i < offset ; i++)
		{
			szResultString[len] = '\t';
			len++;
		}
	}

	szResultString[len] = '<';
	len++;

	memcpy(szResultString+len, m_szpNodeName, nodeNameLen);
	len+=nodeNameLen;
	// print all attributes
	for(int j=0;j<wNumbAttrib;j++)
	{
		DWORD arrLen = strlen(ppArrayAttrib[j]);
		dwLenWasPrinted = dwLenWasPrinted +arrLen;
		if(dwLenWasPrinted < dwLenOfResultString)
		{
			//strcat(szResultString,ppArrayAttrib[j]);
			memcpy(szResultString+len, ppArrayAttrib[j], arrLen);
			len += arrLen;
		}
		else
		{
			strcat(szResultString+len,"ERR!!");
			len+=5;
			* szDumpStr = szResultString;
			*dwStrLen = dwLenOfResultString;
		}
		delete[] ppArrayAttrib[j];
	}
	// end of attributes-close!
	dwLenWasPrinted = dwLenWasPrinted +2;
	szResultString[len] = '>';
	++len;

	if (bPrettyXml==TRUE)
	{
		if (wNumbChilds>0)
		{
			strcat(szResultString, "\n");
			len++;
		}
	}

	// print all childs elements
	for(int k=0;k<wNumbChilds;k++)
	{
		DWORD arrLen = strlen(ppArrayChilds[k]);
		dwLenWasPrinted = dwLenWasPrinted +arrLen+1;

		if(dwLenWasPrinted < dwLenOfResultString)
		{
			memcpy(szResultString+len, ppArrayChilds[k], arrLen);
			len+=arrLen;
		}
		else
		{
			strcat(szResultString,"ERR!!");
			len+=5;
			*szDumpStr = szResultString;
			*dwStrLen = dwLenOfResultString;
		}
		delete[] ppArrayChilds[k];
	}
	// print text of the node
	if( m_pTextValue != NULL )
	{
		DWORD textLen = strlen(m_pTextValue);
		dwLenWasPrinted = dwLenWasPrinted + textLen;
		//strcat(szResultString,m_pTextValue);
		memcpy(szResultString+len, m_pTextValue, textLen);
		len+=textLen;
	}

	//end element with "/>" instead of </name> only if possible

	if(0 == wNumbChilds && NULL == m_pTextValue)
	{
		*(szResultString+len-1)='\0';
		if (bPrettyXml==TRUE)
		{
			strcat(szResultString, "/>\n");
			dwLenWasPrinted = dwLenWasPrinted + 3;
		}
		else
		{
			strcat(szResultString, "/>");
			dwLenWasPrinted = dwLenWasPrinted + 2;
		}
	}
	else
	{
		if( m_pTextValue == NULL && bPrettyXml==TRUE)	//if there is no text in the node
		{
			for(DWORD i = 0 ; i < offset ; i++)
			{
				szResultString[len] = '\t';
				len++;
			}
		}
		szResultString[len] = '<';
		++len;
		szResultString[len] = '/';
		++len;
		DWORD nodeLen = strlen(m_szpNodeName);
		memcpy(szResultString+len, m_szpNodeName, nodeLen);
		len+=nodeLen;

		if (bPrettyXml==TRUE)
		{
			strcat(szResultString, ">\n");
			len+=2;
		}
		else
		{
			strcat(szResultString, ">");
			len++;
		}

		szResultString[len] = '\0';
		dwLenWasPrinted = dwLenWasPrinted + 3 + nodeLen;
	}

	* szDumpStr = szResultString;
	*dwStrLen = dwLenWasPrinted;
	delete []ppArrayChilds; //added
	delete []ppArrayAttrib;

}

HRES CXMLDOMAttribute::get_nodeType(DOMNodeEnum *type)
{
	*type = ENUM_NODE_ATTRIBUTE;
	return SEC_OK;
}

BYTE CXMLDOMAttribute::GetAttributeName(const char** pPlaceToReadFrom)
{
	if(pPlaceToReadFrom == NIL(const char*) || (*pPlaceToReadFrom)==NIL(char))
	{
		DBGFPASSERT(1);
		return FALSE;
	}
	char *strName = new char[MAX_NODE_NAME];
	const char* szRunningPointer = *pPlaceToReadFrom;
	char* pPointeToWrite = strName;
	DWORD  wNumbWrittenChars = 0;
	GoToNon_EmptyChar(&szRunningPointer);
	// Look for a first letter in the name
	if((*szRunningPointer >= 'a' && *szRunningPointer <= 'z') ||
		((*szRunningPointer) == '_' ||
		(*szRunningPointer >= 'A' && *szRunningPointer <= 'Z')))
	{
		*pPointeToWrite = *szRunningPointer;
		pPointeToWrite++;
		szRunningPointer++;
		wNumbWrittenChars++;
	}
	else
	{
		delete []strName;
		DBGFPASSERT(1);
		return FALSE;
	}
	/// fill name with next valid chars
	while(wNumbWrittenChars < (MAX_NODE_NAME))
	{
		if((*szRunningPointer >= 'a' && *szRunningPointer <= 'z') ||
			(*szRunningPointer >= 'A' && *szRunningPointer <= 'Z') ||
			(*szRunningPointer >= '0' && *szRunningPointer <= '9') ||
			(*szRunningPointer == '_') || (*szRunningPointer == '-') ||
			(*szRunningPointer == '.') || (*szRunningPointer == ':'))
		{

			*pPointeToWrite = *szRunningPointer;
			pPointeToWrite++;
			szRunningPointer++;
			wNumbWrittenChars++;

		}
		else
			break;
	}
	*pPointeToWrite = '\0';
	*pPlaceToReadFrom = (char*)szRunningPointer;

	int length=strlen(strName);

	DEALLOCBUFFER(m_szpNodeName);

	m_szpNodeName=new char[length+1];
	strcpy(m_szpNodeName,strName);

	delete []strName;
	return TRUE;
}

BYTE CXMLDOMAttribute::GetAttributeValue(const char** szStringToParse)
{
	if(szStringToParse == NIL(const char*))
	{
		DBGFPASSERT(1);
		return FALSE;
	}
	const char* pPlaceToReadFrom = *szStringToParse;
	GoToNon_EmptyChar(&pPlaceToReadFrom);
	char cOpenChar = 0;
	const char* pStartValueString = NIL(char);
	if(*pPlaceToReadFrom != '"' && *pPlaceToReadFrom != '\'')	// ok - no value
		return TRUE;
	else
	{
		cOpenChar = *pPlaceToReadFrom;
		pPlaceToReadFrom++;
		pStartValueString = pPlaceToReadFrom;
	}
	while(*pPlaceToReadFrom != cOpenChar/* && *pPlaceToReadFrom != ' '*/)	//Judith - space should be allowed in attribute value
		pPlaceToReadFrom++;

	if(*pPlaceToReadFrom != cOpenChar)
	{
		FTRACEINTO << "CXMLDOMAttribute::GetAttributeValue - error. pPlaceToReadFrom = "<<pPlaceToReadFrom<<" while cOpenChar = "<<cOpenChar;
		DBGFPASSERT(1);
		return FALSE;
	}
	else
	{
		m_szTextValue = new char[pPlaceToReadFrom - pStartValueString+1];
		m_szTextValue[0] = '\0';
		strncpy(m_szTextValue,pStartValueString,pPlaceToReadFrom - pStartValueString);
		m_szTextValue[pPlaceToReadFrom - pStartValueString] = '\0';
		pPlaceToReadFrom++; // after close char
		*szStringToParse = pPlaceToReadFrom;
	}
	return TRUE;
}

CXMLDOMAttribute::CXMLDOMAttribute(void)
{
	m_szTextValue = NIL(char);
}

CXMLDOMAttribute::CXMLDOMAttribute(const CXMLDOMAttribute& other)
	:CXMLDOMNode(other)
{
	strcpy(m_szTextValue,other.m_szTextValue);

}

CXMLDOMAttribute::~CXMLDOMAttribute(void)
{
	if(m_szTextValue != NIL(char))
		delete[] m_szTextValue;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Contains the attribute name.
		Parameters:	attributeName [out, retval] Name of the attribute.
			The value is the same as the nodeName property of the CXMLDOMNode object
		Return Values:	SEC_OK Value returned if successful
						SEC_FALSE Value when returning Null.
						ERR_INVALIDARG Value returned if attributeName is Null.
*/
HRES  CXMLDOMAttribute::get_name( /* [retval][out] */ char** attributeName)
{
	HRES hres = SEC_OK;
	if(attributeName == NIL(char*))
		hres = ERR_INVALIDARG;

	if(hres == SEC_OK)
	{
		if(m_szpNodeName == NIL(char))
		{
			*attributeName = NIL(char);
			hres = SEC_FALSE;
		}
	}
	if(hres == SEC_OK)
		*attributeName = m_szpNodeName;
	return hres;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: In our small version of parser Attribute has no child
		Parameters:	childList [out, retval] A list of children in the current node.
		Return Values:	SEC_OK Value returned if successful.
						ERR_INVALIDARG Value returned if childList is Null.
*/
HRES CXMLDOMAttribute::get_childNodes(CXMLDOMNodeList **childList)
{
	HRES hres = SEC_OK;
	if(childList == NIL(CXMLDOMNodeList *))
		hres = ERR_INVALIDARG;
	if(hres == SEC_OK)
		*childList = NIL(CXMLDOMNodeList);
	return hres;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Contains the first child of the node.
		Parameters:	firstChild [out, retval] First child node. If there are no such children, it returns Null.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned when there are no children.
						ERR_INVALIDARG Value returned if firstChild is Null.
*/
HRES  CXMLDOMAttribute::get_firstChild(/* [retval][out] */ CXMLDOMNode **firstChild)
{
	HRES hres = SEC_OK;
	if(firstChild == NIL(CXMLDOMNode *))
		hres = ERR_INVALIDARG;
	if(hres == SEC_OK)
	{
		*firstChild = NIL(CXMLDOMNode);
		hres = SEC_FALSE;
	}
	return hres;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Contains the first child of the node.
		Parameters:	lastChild [out, retval] First child node. If there are no such children, it returns Null.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned when there are no children.
						ERR_INVALIDARG Value returned if lastChild is Null.
*/
HRES  CXMLDOMAttribute::get_lastChild(/* [retval][out] */ CXMLDOMNode **lastChild)
{
	HRES hres = SEC_OK;
	if(lastChild == NIL(CXMLDOMNode *))
	{
		DBGFPASSERT(1);
		hres = ERR_INVALIDARG;
	}
	if(hres == SEC_OK)
	{
		DBGFPASSERT(1);
		*lastChild = NIL(CXMLDOMNode);
		hres = SEC_FALSE;
	}
	return hres;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Contains the text associated with the node.
		Parameters:	value [out, retval][in] Node value;Contains a string representing the value of the attribute.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value when returning Null.
						ERR_INVALIDARG Value returned if value is Null.
						ERR_FAIL  Value returned if an error occurs.
*/
HRES CXMLDOMAttribute::get_nodeValue(char** value)
{
	HRES hres = SEC_OK;
	if(value == NIL(char*))
	{
		DBGFPASSERT(1);
		hres = ERR_INVALIDARG;
	}
	if(hres == SEC_OK)
	{
		if(m_szTextValue == NIL(char))
		{
			DBGFPASSERT(1);
			*value = NIL(char);
			hres = SEC_FALSE;
		}
	}
	if(hres == SEC_OK)
		*value = m_szTextValue;
	return hres;
}

HRES CXMLDOMAttribute::get_nodeValue(CObjString& value)
{
	HRES hres = SEC_OK;

	if(hres == SEC_OK)
	{
		if(m_szTextValue == NIL(char))
		{
			DBGFPASSERT(1);
			//*value = NIL(char);
			hres = SEC_FALSE;
		}
	}
	if(hres == SEC_OK)
		value = m_szTextValue;
	return hres;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
HRES CXMLDOMAttribute::Parse(const char** szStringToParse)
{
	HRES hres = SEC_OK;
	if(szStringToParse == NIL(const char*))
	{
		DBGFPASSERT(1);
		return  ERR_INVALIDARG;
	}
	BYTE bIsEqualWasFound = 0;
	const char* pPlaceToReadFrom = *szStringToParse;


	if(!GetAttributeName(&pPlaceToReadFrom))
	{
		DBGFPASSERT(1);
		return  ERR_FAIL;
	}
	GoToNon_EmptyChar(&pPlaceToReadFrom);
	if(*pPlaceToReadFrom != '=')
	{
		FPTRACE2(eLevelInfoHigh, "CXMLDOMAttribute::Parse ,didn't find '\"'. ", pPlaceToReadFrom);

		DBGFPASSERT(1);
		return  SEC_FALSE;
	}
	else
		pPlaceToReadFrom++;

	if(!GetAttributeValue(&pPlaceToReadFrom))
	{
		DBGFPASSERT(1);
		return  ERR_FAIL;
	}
	* szStringToParse = pPlaceToReadFrom;
	return hres;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CXMLDOMAttribute::DumpDataAsStringWithAttribute(char** szDumpStr,
                                        DWORD* dwStrLen, DWORD offset, BYTE bPrettyXml)
{
	DWORD dwSize = 0;
	dwSize = strlen(m_szpNodeName)+10;
	if(m_szTextValue != NIL(char))
		dwSize = dwSize + strlen(m_szTextValue);
	ALLOCBUFFER(szBufToReturn,dwSize);

	if(m_szTextValue != NIL(char))
		sprintf(szBufToReturn," %s=\"%s\"",m_szpNodeName,m_szTextValue);
	else
		sprintf(szBufToReturn," %s",m_szpNodeName);

	* szDumpStr = szBufToReturn;
	* dwStrLen = dwSize;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CXMLDOMAttribute::SetValueForElement(const char* ezValue)
{
	if(ezValue)
	{
		int len = NeededSizeForNewArray(ezValue);
		m_szTextValue = new char[len+1];
		*m_szTextValue = '\0';

		ChangeSpecialChar(ezValue, TRUE, m_szTextValue);
	}
}

/*
		Purpose: Loads an XML document from the specified location.
		Parameters:	xmlSource [in] Indicator of the source XML to parse.
					isSuccessful [out, retval] True if the load succeeded; False if the load failed.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned if the load fails.
						ERR_INVALIDARG Value returned if isSuccessful is Null.
*/
HRES CXMLDOMDocument::load(/* [in] */ const char* xmlSource,/* [retval][out] */ unsigned char *isSuccessful)
{
	if(isSuccessful == NULL)
	{
		DBGFPASSERT(1);
		return ERR_INVALIDARG;
	}
	HRES hRes = SEC_OK;
	FILE* infile =fopen(xmlSource,"r");
	if (infile==NULL)
	{
		DBGFPASSERT(1);
		*isSuccessful = 0;
		return SEC_FALSE;
	}
	const int fileSize = GetFileSize(xmlSource);

	ALLOCBUFFER(sxStringFromFile, fileSize + 1);
	sxStringFromFile[0] = '\0';
	DWORD wNumbWasReaded = fread(sxStringFromFile,sizeof( char ), fileSize,infile);
	if (wNumbWasReaded==0)
	{
		DBGFPASSERT(1);
		hRes = SEC_FALSE;
	}
	else
		sxStringFromFile[wNumbWasReaded]='\0';
	char* pStringWithoutComments = RemoveCommentsFromString(sxStringFromFile);
	m_pRootElement = new CXMLDOMElement;
	const char* pStringToParse=pStringWithoutComments;
	HRES HRES = m_pRootElement->Parse(&pStringToParse);
	if(HRES != SEC_OK)
	{
		DBGFPASSERT(1);
		delete m_pRootElement;
		m_pRootElement = NIL(CXMLDOMElement);
		hRes = HRES;
	}
	if(sxStringFromFile!=pStringWithoutComments)
		delete []pStringWithoutComments;

	DEALLOCBUFFER(sxStringFromFile);
	fclose(infile);
	if(hRes == SEC_OK)
		*isSuccessful = 1;
	else
		*isSuccessful = 0;

	return hRes;
}

CXMLDOMDocument::CXMLDOMDocument(void)
        :m_EncodingCharset(MCMS_INTERNAL_STRING_ENCODE_TYPE), m_pRootElement(NIL(CXMLDOMElement))
{
}

CXMLDOMDocument::CXMLDOMDocument(const string & encodingCharset)
        :m_EncodingCharset(encodingCharset), m_pRootElement(NIL(CXMLDOMElement))
{
}

CXMLDOMDocument::CXMLDOMDocument(CXMLDOMElement *pRootElem, const string & encodingCharset)
        :m_EncodingCharset(encodingCharset), m_pRootElement(pRootElem)
{
}

CXMLDOMDocument::~CXMLDOMDocument(void)
{
	if(m_pRootElement!= NIL(CXMLDOMElement))
    {
		PDELETE(m_pRootElement);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
HRES CXMLDOMDocument::get_nodeType(DOMNodeEnum *type)
{
	*type = ENUM_NODE_DOCUMENT;
	return SEC_OK;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Return root node
		Parameters:	firstChild [out, retval] First child node. If there are no such children, it returns Null.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned when there are no children.
						ERR_INVALIDARG Value returned if firstChild is Null.
*/
HRES  CXMLDOMDocument::get_firstChild(/* [retval][out] */ CXMLDOMNode **firstChild)
{
	HRES hres = SEC_OK;
	if(firstChild == NIL(CXMLDOMNode *))
	{
		DBGFPASSERT(1);
		hres = ERR_INVALIDARG;
	}
	if(hres == SEC_OK)
	{
		DBGFPASSERT(1);
		*firstChild = NIL(CXMLDOMNode);
		hres = SEC_FALSE;
	}
	if(hres == SEC_OK)
		*firstChild = m_pRootElement;

	return hres;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Return root node
		Parameters:	lastChild [out, retval] First child node. If there are no such children, it returns Null.

		Return Values:	SEC_OK Value returned if successful.
						SEC_FALSE Value returned when there are no children.
						ERR_INVALIDARG Value returned if lastChild is Null.
*/
HRES  CXMLDOMDocument::get_lastChild(/* [retval][out] */ CXMLDOMNode **lastChild)
{
	return get_firstChild(lastChild);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                   All thos functions arn't supposed be called
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
HRES CXMLDOMDocument::get_childNodes(CXMLDOMNodeList **childList)
{
	DBGFPASSERT(1);
	childList =NIL(CXMLDOMNodeList *);
	return ERR_INVALIDARG;

}

DWORD CXMLDOMDocument::GetDumpStringLength(int offset, BYTE bPrettyXml/* = FALSE*/)
{
	return m_pRootElement->GetDumpStringLength(0);
}

HRES CXMLDOMDocument::DumpDataAsStringArrayEx(CStrArray& strArray,WORD &Compressionlevel)
{
	return m_pRootElement->DumpDataAsStringArrayEx(strArray,Compressionlevel);
}

STATUS CXMLDOMDocument::WriteXmlFile(const char *file_name)
{
	return m_pRootElement->WriteXmlFile(file_name);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
		Purpose: Contains the text associated with the node. In case od Element alway returns Contains Null
		Parameters:
		Return Values:	SEC_FALSE (for get_nodeValue only) Value when returning Null.
*/
HRES CXMLDOMDocument::get_nodeValue(char** value)
{
	DBGFPASSERT(1);
	*value = NIL(char);
	return SEC_FALSE;
}

HRES CXMLDOMDocument::get_nodeValue(CObjString& value)
{
	DBGFPASSERT(1);
	//value = NIL(char);
	return SEC_FALSE;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void CXMLDOMDocument::DumpDataAsStringWithAttribute(/* [retval][out] */char** szDumpStr,
                                       /* [retval][out] */ DWORD* dwStrLen, DWORD offset, BYTE bPrettyXml)
{
	char* pContentOfTree = NULL;
	char* pDataToReturn = NULL;
	DWORD dwLenghtDataOfTree = 0;

	m_pRootElement->DumpDataAsStringWithAttribute(&pContentOfTree,&dwLenghtDataOfTree, offset, bPrettyXml);

    DWORD commentLen = 0;//strComment.length();

    DWORD totalDataLen = dwLenghtDataOfTree + 10 + commentLen;

    pDataToReturn = new char[totalDataLen];

//    strncpy(pDataToReturn, strComment.c_str(), commentLen);
	strncpy(pDataToReturn + commentLen, pContentOfTree, dwLenghtDataOfTree);
	strcat(pDataToReturn,"    ");

	delete []pContentOfTree;

	*szDumpStr = pDataToReturn;
	*dwStrLen = totalDataLen;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void CXMLDOMDocument::DumpDataAsLongStringEx(/* [retval][out] */char** szDumpStr,
                                       		 /* [retval][out] */ DWORD* dwStrLen,/*, DWORD offset, BYTE bPrettyXml*/
                                                                BYTE bPrettyXml)
{
	char* pContentOfTree = NULL;
	char* pDataToReturn = NULL;
	DWORD dwLenghtDataOfTree = 0;

	m_pRootElement->DumpDataAsLongStringEx(szDumpStr, bPrettyXml);

	*dwStrLen = strlen(*szDumpStr);
}

HRES CXMLDOMDocument::Parse(const char** szStringToParse)
{
	HRES hRes = SEC_OK;
	char* pStringWithoutComments = RemoveCommentsFromString(*szStringToParse);	//Judith - to check
	m_pRootElement = new CXMLDOMElement;
	//char* pStringToParse=pStringWithoutComments;									//Judith - to check
	const char* pStringToParse=pStringWithoutComments;	//	Judith - to check - line added
	HRES HRES = m_pRootElement->Parse(&pStringToParse);
	if(HRES != SEC_OK)
	{
		DBGFPASSERT(1);
		delete m_pRootElement;
		m_pRootElement = NIL(CXMLDOMElement);
		hRes = HRES;
	}
	if(*szStringToParse!=pStringWithoutComments)
		delete [] pStringWithoutComments;				//Judith - to check
	return hRes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXMLDOMElement* CXMLDOMDocument::GetRootElement(void)
{
	return m_pRootElement;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BYTE ParseXMLFile(const char* szFileName, CXMLDOMDocument *pXMLDocument)
{
	if(szFileName == NIL(char))
		return FALSE;

	unsigned char result = FALSE;
	HRES loadResult = pXMLDocument->load(szFileName,&result);
	if (loadResult!=SEC_OK)
		return FALSE;

	return result;
}


int IXMLServiceUtilities::GetNumber(char* pszValue,BYTE& bIsNumber)
{
	if(!pszValue)
	{
		bIsNumber = FALSE;
		return 0;
	}

	char* pStopScan;
	int nRetVal;
	int len=strlen(pszValue);

	if(!len)
	{
		bIsNumber = FALSE;
		return 0;
	}

	bIsNumber = TRUE;

	//if the string is "bigger" then the max DWORD value (0xffffffff) the strtoul will cause an overflow

	if(strcmp(pszValue,"-1")==0)
	{
		bIsNumber = TRUE;
		return 0xFFFFFFFF;
	}
	else if(strlen(pszValue)>10)
	{
		bIsNumber=FALSE;
		return 0;
	}
	else if (len==10)
	{
		char *pChar=pszValue;
		for (int i=0;i<len;i++)
		{
			if (isdigit(*pChar)==0)
			{
				bIsNumber=FALSE;
				return 0;
			}
			pChar++;
		}
		if(strcmp(pszValue,"4294967295")>0)
		{
			bIsNumber=FALSE;
			return 0;
		}
	}
	else
	{
		char *pChar=pszValue;
		for (int i=0;i<len;i++)
		{
			if (isdigit(*pChar)==0)
			{
				bIsNumber=FALSE;
				return 0;
			}
			pChar++;
		}
	}

	nRetVal = strtoul(pszValue,&pStopScan,10);


	if(pStopScan && strlen(pStopScan))
		bIsNumber = FALSE;

	return nRetVal;
}

int IXMLServiceUtilities::ParseTimeString(char* pszTime,CStructTm& time)
{
	char *pszTimePtr,*pszTemp;
	const char CurrValSize = 11;
	char szCurrVal[CurrValSize];
	BYTE bIsNumber;
	int nStatus = STATUS_OK;


	if(!pszTime)
		nStatus = STATUS_DATE_TIME_INVALID;
	else
	{
		pszTimePtr = pszTime;

		pszTemp = strstr(pszTimePtr,"-");

		if(!pszTemp)
			nStatus = STATUS_DATE_TIME_INVALID;
		else
		{
			strncpy(szCurrVal,pszTimePtr,min(pszTemp - pszTimePtr, CurrValSize - 1));
			szCurrVal[min(pszTemp - pszTimePtr, CurrValSize - 1)] = 0;
			time.m_year = GetNumber(szCurrVal,bIsNumber);
			pszTimePtr = pszTemp+1;

			pszTemp = strstr(pszTimePtr,"-");

			if(!pszTemp || !bIsNumber)
				nStatus = STATUS_DATE_TIME_INVALID;
			else
			{
				strncpy(szCurrVal,pszTimePtr,pszTemp - pszTimePtr);
				szCurrVal[pszTemp - pszTimePtr] = 0;
				time.m_mon = GetNumber(szCurrVal,bIsNumber);
				pszTimePtr = pszTemp+1;

				pszTemp = strstr(pszTimePtr,"T");

				if(!pszTemp || !bIsNumber)
					nStatus = STATUS_DATE_TIME_INVALID;
				else
				{
					strncpy(szCurrVal,pszTimePtr,pszTemp - pszTimePtr);
					szCurrVal[pszTemp - pszTimePtr] = 0;
					time.m_day = GetNumber(szCurrVal,bIsNumber);
					pszTimePtr = pszTemp+1;

					pszTemp = strstr(pszTimePtr,":");

					if(!pszTemp || !bIsNumber)
						nStatus = STATUS_DATE_TIME_INVALID;
					else
					{
						strncpy(szCurrVal,pszTimePtr,pszTemp - pszTimePtr);
						szCurrVal[pszTemp - pszTimePtr] = 0;
						time.m_hour = GetNumber(szCurrVal,bIsNumber);
						pszTimePtr = pszTemp+1;

						pszTemp = strstr(pszTimePtr,":");

						if(!pszTemp || !bIsNumber)
							nStatus = STATUS_DATE_TIME_INVALID;
						else
						{
							strncpy(szCurrVal,pszTimePtr,pszTemp - pszTimePtr);
							szCurrVal[pszTemp - pszTimePtr] = 0;
							time.m_min = GetNumber(szCurrVal,bIsNumber);
							pszTimePtr = pszTemp+1;

							if(!strlen(pszTimePtr) || !bIsNumber)
								nStatus = STATUS_DATE_TIME_INVALID;
							else
							{
								time.m_sec = GetNumber(pszTimePtr,bIsNumber);

								if(!bIsNumber)
									nStatus = STATUS_DATE_TIME_INVALID;
							}
						}
					}
				}
			}
		}
	}

	return nStatus;
}

int IXMLServiceUtilities::DwordToIPString(DWORD DwordIP,char* pszIP)
{
	if(!pszIP)
		return FALSE;

	WORD HWord=HIWORD(DwordIP);
	WORD LWord=LOWORD(DwordIP);
	BYTE byteArray[4];
	byteArray[0]=HIBYTE(HWord);
	byteArray[1]=LOBYTE(HWord);
	byteArray[2]=HIBYTE(LWord);
	byteArray[3]=LOBYTE(LWord);

	sprintf(pszIP,"%d.%d.%d.%d",byteArray[0],byteArray[1],byteArray[2],byteArray[3]);

	return TRUE;
}

int IXMLServiceUtilities::ReturnErrorMsg(char* element,char* pszError)
{
	sprintf(pszError,"Element - '%s'",element);
	return STATUS_NODE_MISSING;
}

int CXMLDOMElement::InnerGetAndValidate(char** pValue, char *pszError, int type,int &nMinLen,int &nMaxLen)
{
	char* pVarValue=NULL;
	int nStatus = STATUS_OK;
	BYTE bMaxRestricted=FALSE;

	get_nodeValue(&pVarValue);
	if(!pVarValue)
		pVarValue = "";

	if (!CStringsMaps::IsDefineType(type))
	{
		DBGFPASSERT(1);
		//if the type is not define in the strmaps
		nStatus = STATUS_INVALID_KEY;
	}
	else
	{
		if(nMaxLen!=0)
		{
			bMaxRestricted=TRUE;
			if(strlen(pVarValue) > (DWORD)nMaxLen)
				nStatus = STATUS_NODE_LENGTH_TOO_LONG;
		}
		else if (CStringsMaps::GetValue(type,nMaxLen,"MAX"))
		{
			bMaxRestricted=TRUE;
			if(strlen(pVarValue) > (DWORD)nMaxLen)
				nStatus = STATUS_NODE_LENGTH_TOO_LONG;


		}
		if(CStringsMaps::GetValue(type,nMinLen,"MIN"))
		{
			if((int)strlen(pVarValue) < nMinLen)
				nStatus = STATUS_NODE_LENGTH_TOO_SHORT;

		}
		else
		{
			if(bMaxRestricted==FALSE)
			{
				nStatus = STATUS_MIN_MAX_MISSING_IN_KEY;
				DBGFPASSERT(1);
			}
		}
	}

	if(pValue)
		*pValue = pVarValue;

    return nStatus;
}

int CXMLDOMElement::GetAndValidate(CObjString& strValue,
                                   char *pszError,
                                   int type,
                                   bool isAscii)
{

	int nStatus = STATUS_OK;
	char* pszVal;
	int Min=0;

	int Max=strValue.GetAllocLength();
	nStatus = InnerGetAndValidate(&pszVal, pszError,  type,Min,Max);

	if(nStatus == STATUS_OK)
	{
        if(isAscii && eStringValid != CObjString::IsLegalAsciiString(pszVal, strlen(pszVal) + 1))
        {
            nStatus = STATUS_NODE_VALUE_NOT_ASCII;
        }
        else
        {
            strValue = pszVal;
        }
	}
	else
	{
		if(nStatus==STATUS_INVALID_KEY||nStatus==STATUS_MIN_MAX_MISSING_IN_KEY)
		{
			//DBGFFPASSERT(1);

		}
		else if(nStatus==STATUS_NODE_LENGTH_TOO_LONG)
		{
			// copy and set the status == STATUS_OK to avoid compatability problems

			CreateErrorMessage_ForGetFuncs(nStatus,pszError,type,Min,Max);
			FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (char *)  ", pszError);

			char *pString = new char[Max+1];
            CEncodingConvertor::CutUtf8String(pszVal, pString, Max);

			strValue = pString;//the operator = does not cut the string according to the max size
			delete [] pString;
		}
		else if(nStatus == STATUS_NODE_LENGTH_TOO_SHORT)
		{

		}
	}

	if(nStatus != STATUS_OK)
	{
		CreateErrorMessage_ForGetFuncs(nStatus,pszError,type,Min,Max);
		FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (char*)  ", pszError);
	}

    if(nStatus == STATUS_OK || nStatus == STATUS_NODE_LENGTH_TOO_LONG)
    {
        const int pVarValueLen = strlen(strValue.GetString());
        if(0 < pVarValueLen)
        {
            char * tmpNewVal = new char [pVarValueLen + 1];
            memset(tmpNewVal, 0, pVarValueLen);

            ChangeSpecialChar(strValue.GetString(), FALSE, tmpNewVal);

            strValue = tmpNewVal;

            delete [] tmpNewVal;
        }
    }

	return nStatus;
}

int CXMLDOMElement::GetAndValidate(std::string& strValue,
                                   char* pszError,
                                   int type,
                                   bool isAscii)
{
  char*  pszVal;
  int    Min = 0;
  int    Max = 0;
  STATUS nStatus = InnerGetAndValidate(&pszVal, pszError, type, Min, Max);
  switch (nStatus)
  {
  case STATUS_OK:
    if (isAscii &&
        eStringValid != CObjString::IsLegalAsciiString(pszVal, strlen(pszVal) + 1))
      nStatus = STATUS_NODE_VALUE_NOT_ASCII;
    else
      strValue = pszVal;
    break;

  case STATUS_NODE_LENGTH_TOO_LONG:
    {
      char* pString = new char[Max + 1];
      pString[Max] = '\0';
      CEncodingConvertor::CutUtf8String(pszVal, pString, Max);

      strValue = pString;
      delete [] pString;

      CreateErrorMessage_ForGetFuncs(nStatus, pszError, type, Min, Max);
      FTRACEWARN << "Invalid XML: " << pszError
                 << ": STATUS_NODE_LENGTH_TOO_LONG"
                 << ", min " << Min << ", max " << Max;

      // Set STATUS_OK to avoid compatibility problems
      nStatus = STATUS_OK;
    }
    break;

  case STATUS_INVALID_KEY:
  case STATUS_NODE_LENGTH_TOO_SHORT:
  case STATUS_MIN_MAX_MISSING_IN_KEY:
    break;
  }

  if (nStatus != STATUS_OK)
  {
    CreateErrorMessage_ForGetFuncs(nStatus, pszError, type, Min, Max);
    FTRACEWARN << "Invalid XML: " << pszError << ": status " << nStatus
               << ", min " << Min << ", max " << Max;
  }
  else
  {
    if (!strValue.empty())
    {
      char* nval = (char*) malloc(strValue.length() + 1);
      if(nval)
      {
        nval[0] = '\0';
        ChangeSpecialChar(strValue.c_str(), FALSE, nval);
        strValue = nval;
        free(nval);
      }
      else
      {
        FPASSERTMSG(1, "CXMLDOMElement::GetAndValidate - malloc failed");
      }
    }
  }

  return nStatus;
}

int CXMLDOMElement::GetAndValidate(char** pValue,
                                   char *pszError,
                                   int type,
                                   bool isAscii)
{
  char* pszVal;
  int Min = 0, Max = 0;

  *pValue = NULL;
  int nStatus = InnerGetAndValidate(&pszVal, pszError, type, Min, Max);

  switch (nStatus)
  {
    case STATUS_INVALID_KEY:
    case STATUS_MIN_MAX_MISSING_IN_KEY:
      //DBGPASSERT(1);
      break;

    case STATUS_OK:
    // Previously, if the length of the text in the node was greater than or less than required,
    // the text was not copied to the target buffer, but the return code was OK.
    // Dmitry Krasnopolsky: I decided always copy the text to the target buffer regardless of its length,
    // but return an error code if different lengths.
    case STATUS_NODE_LENGTH_TOO_LONG:
    case STATUS_NODE_LENGTH_TOO_SHORT:
      if (isAscii && eStringValid != CObjString::IsLegalAsciiString(pszVal, strlen(pszVal) + 1))
        nStatus = STATUS_NODE_VALUE_NOT_ASCII;
      else
        *pValue = pszVal;
      break;
  }

  if (nStatus != STATUS_OK)
  {
    CreateErrorMessage_ForGetFuncs(nStatus, pszError, type, Min, Max);
    FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (char*)  ", pszError);
  }

  return nStatus;
}


int CXMLDOMElement::GetAndValidate(char* pValue,
                                   char* pszError,
                                   int type,
                                   bool isAscii)
{
	int   nStatus = STATUS_OK;
	char* pszVal  = NULL;
	int   Min     = 0, Max = 0;

	nStatus = InnerGetAndValidate(&pszVal, pszError, type, Min, Max);

	if (nStatus == STATUS_OK)
	{
		if (pValue)
		{
			if (isAscii && eStringValid != CObjString::IsLegalAsciiString(pszVal, strlen(pszVal) + 1))
			{
				nStatus = STATUS_NODE_VALUE_NOT_ASCII;
			}
			else
			{
				int len = Max < (int)strlen(pszVal) ? Max : strlen(pszVal);
				strncpy(pValue, pszVal, len);
				pValue[len] = '\0';
			}
		}
		else
		{
			DBGFPASSERT(1);
			nStatus = STATUS_API_INTERNAL_ERROR;
		}
	}
	else
	{
		if (nStatus == STATUS_INVALID_KEY || nStatus == STATUS_MIN_MAX_MISSING_IN_KEY)
		{
			DBGFPASSERT(1);
		}
		else if (nStatus == STATUS_NODE_LENGTH_TOO_LONG)
		{
			// copy and set the status == STATUS_OK to avoid compatability problems

			CreateErrorMessage_ForGetFuncs(nStatus, pszError, type, Min, Max);

			if (pszVal)
				FTRACEINTO << "Element:" << pszError << ", pszVal:" << pszVal << ", Min:" << Min << ", Max:" << Max << ", Status:STATUS_NODE_LENGTH_TOO_LONG";

			if (pValue)
			{
				if (pszVal)
					CEncodingConvertor::CutUtf8String(pszVal, pValue, Max);

				nStatus = STATUS_OK;
			}
			else
			{
				DBGFPASSERT(1);
				nStatus = STATUS_API_INTERNAL_ERROR;
			}
		}
		else if (nStatus == STATUS_NODE_LENGTH_TOO_SHORT)
		{
		}
	}

	if (nStatus != STATUS_OK)
	{
		CreateErrorMessage_ForGetFuncs(nStatus, pszError, type, Min, Max);
		FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (char*)  ", pszError);
	}

	if (nStatus == STATUS_OK || nStatus == STATUS_NODE_LENGTH_TOO_LONG)
	{
		const int pVarValueLen = strlen(pValue);
		if (0 < pVarValueLen)
		{
			char* tmpNewVal = new char [pVarValueLen + 1];
			memset(tmpNewVal, 0, pVarValueLen + 1);

			ChangeSpecialChar(pValue, FALSE, tmpNewVal);

			int len = Max < pVarValueLen ? Max : pVarValueLen;
			strncpy(pValue, tmpNewVal, len);
			pValue[len] = '\0';

			delete [] tmpNewVal;
		}
	}

	return nStatus;
}

int CXMLDOMElement::InnerGetAndValidate(DWORD *pValue, char *pszError, int type,int &nMinVal,int &nMaxVal,BYTE &bisIPAddress/*=FALSE*/)
{
	char* pVarValue=NULL;
	DWORD nVal=0;
	BYTE bIsNumber=FALSE;
	int nStatus = STATUS_OK;
	bisIPAddress = FALSE;

	get_nodeValue(&pVarValue);
	if(!pVarValue)
		pVarValue = "";


	if(!CStringsMaps::GetValue(type,(int &)nVal,pVarValue))
	{

		nVal = IXMLServiceUtilities::GetNumber(pVarValue,bIsNumber);
		if (type==IP_ADDRESS)
		    bisIPAddress=TRUE;

		if(bisIPAddress)
		{
			nStatus=InnerGetAndValidateIPAddress(&nVal,pszError);
		}
		else if (!CStringsMaps::IsDefineType(type))
		{
			DBGFPASSERT(type);
			//if the type is not define in the strmaps
			nStatus = STATUS_INVALID_KEY;

		}
		else if(bIsNumber)
		{
			bIsNumber=TRUE;

			//if we are compiling in the operator side don't check the
			//max min length according to the string maps
			//checking the length according to the strmaps may cause compatability problems

			BYTE bMaxRestricted=FALSE;
			if (CStringsMaps::GetValue(type,nMaxVal,"MAX"))
			{
				if((nMaxVal!= -1) && nVal > (DWORD)nMaxVal)
					nStatus = STATUS_VALUE_OUT_OF_RANGE;
				bMaxRestricted=TRUE;

			}
			if(CStringsMaps::GetValue(type,nMinVal,"MIN"))
			{
				if((nMinVal!= -1) && nVal < (DWORD)nMinVal)
					nStatus = STATUS_VALUE_OUT_OF_RANGE;

			}
			else
			{
				//************* replaced****************
				//Since numbers always have min and max values
				//if the value is not restricted to min or max then its invalid
				//if(bMaxRestricted==FALSE)
				//	nStatus = STATUS_NODE_VALUE_INVALID;
				//***************************************

				if(bMaxRestricted==FALSE)
				{
					bIsNumber=FALSE;
					//if min max was not set for this element then its probably an enum
					nStatus = STATUS_ENUM_VALUE_INVALID;
				}
			}
		}
		else
		{

			//failure in case its an enumeration
			nStatus = STATUS_ENUM_VALUE_INVALID;
		}
	}

	if(nStatus == STATUS_OK)
	{
		*pValue=nVal;
	}

	return nStatus;
}


int CXMLDOMElement::InnerGetAndValidate(int *pValue, char *pszError, int type,int &nMinVal,int &nMaxVal,BYTE &bisIPAddress/*=FALSE*/)
{
	char* pVarValue=NULL;
	int nVal=0;
	BYTE bIsNumber=FALSE;
	int nStatus = STATUS_OK;
	bisIPAddress = FALSE;


	get_nodeValue(&pVarValue);
	if(!pVarValue)
		pVarValue = "";


	if(!CStringsMaps::GetValue(type,(int &)nVal,pVarValue))
	{

		nVal = IXMLServiceUtilities::GetNumber(pVarValue,bIsNumber);
		if (type==IP_ADDRESS)
		    bisIPAddress=TRUE;

		if(bisIPAddress)
		{
			DWORD dwVal = (DWORD)nVal;
			nStatus=InnerGetAndValidateIPAddress(&dwVal,pszError);
		}
		else if (!CStringsMaps::IsDefineType(type))
		{
			DBGFPASSERT(type);
			//if the type is not define in the strmaps
			nStatus = STATUS_INVALID_KEY;

		}
		else if(bIsNumber)
		{
			bIsNumber=TRUE;

			//if we are compiling in the operator side don't check the
			//max min length according to the string maps
			//checking the length according to the strmaps may cause compatability problems

			BYTE bMaxRestricted=FALSE;
			if (CStringsMaps::GetValue(type,nMaxVal,"MAX"))
			{
				if((nMaxVal!= -1) && nVal > nMaxVal)
				{
					nStatus = STATUS_VALUE_OUT_OF_RANGE;
				}
				bMaxRestricted=TRUE;

			}
			if(CStringsMaps::GetValue(type,nMinVal,"MIN"))
			{
				if((nMinVal!= -1) && nVal < nMinVal)
				{
					nStatus = STATUS_VALUE_OUT_OF_RANGE;
				}
			}
			else
			{
				//************* replaced****************
				//Since numbers always have min and max values
				//if the value is not restricted to min or max then its invalid
				//if(bMaxRestricted==FALSE)
				//	nStatus = STATUS_NODE_VALUE_INVALID;
				//***************************************

				if(bMaxRestricted==FALSE)
				{
					bIsNumber=FALSE;
					//if min max was not set for this element then its probably an enum
					nStatus = STATUS_ENUM_VALUE_INVALID;
				}
			}
		}
		else
		{

			//failure in case its an enumeration
			nStatus = STATUS_ENUM_VALUE_INVALID;
		}
	}

	if(nStatus == STATUS_OK)
	{
		*pValue=nVal;
	}

	return nStatus;
}


int CXMLDOMElement::GetAndValidate(int* pValue,char *pszError,int type)
{
	int dwVal;
	int nStatus = STATUS_OK;
	int nMin=0;
	int nMax=0;
	BYTE bIsIPAddress=FALSE;

	nStatus = InnerGetAndValidate(&dwVal,pszError,type,nMin,nMax,bIsIPAddress);

	if(nStatus == STATUS_OK)
	{
		*pValue=dwVal;
	}
	else
	{
		CreateErrorMessage_ForGetFuncs(nStatus,pszError,type,nMin,nMax);
		if (bIsIPAddress==FALSE)
		    FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (int)  ", pszError);
	}

	return nStatus;
}

int CXMLDOMElement::GetAndValidate(DWORD* pValue,char *pszError,int type)
{
	DWORD dwVal;
	int nStatus = STATUS_OK;
	int nMin=0;
	int nMax=0;
	BYTE bIsIPAddress=FALSE;

	nStatus = InnerGetAndValidate(&dwVal,pszError,type,nMin,nMax,bIsIPAddress);

	if(nStatus == STATUS_OK)
	{
		*pValue=dwVal;
	}
	else
	{
		CreateErrorMessage_ForGetFuncs(nStatus,pszError,type,nMin,nMax);
		if (bIsIPAddress==FALSE)
		    FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (DWORD)  ", pszError);
	}

	return nStatus;
}

int CXMLDOMElement::GetAndValidate(BYTE* pValue,char *pszError,int type)
{
	DWORD dwVal;
	int nStatus = STATUS_OK;
	int nMin=0;
	int nMax=0;
	BYTE bIsIPAddress = FALSE;

	nStatus = InnerGetAndValidate(&dwVal,pszError,type,nMin,nMax,bIsIPAddress);

	if(nStatus == STATUS_OK)
	{
		if(dwVal<=0xff)
			*pValue=(BYTE)dwVal;
		else
		{
			nMax=0xff;
			nStatus= STATUS_VALUE_OUT_OF_RANGE;
		}
	}

	if(nStatus!=STATUS_OK)
	{
		CreateErrorMessage_ForGetFuncs(nStatus,pszError,type,nMin,nMax);
		FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (BYTE)  ", pszError);
	}

	return nStatus;
}

int CXMLDOMElement::GetAndValidate(WORD* pValue,char *pszError,int type)
{
	DWORD dwVal;
	int nStatus = STATUS_OK;
	int nMin=0;
	int nMax=0;
	BYTE bIsIPAddress = FALSE;

	nStatus = InnerGetAndValidate(&dwVal,pszError,type,nMin,nMax,bIsIPAddress);

	if(nStatus == STATUS_OK)
	{
		if(dwVal<=0xffff)
			*pValue=(WORD)dwVal;
		else
		{
			nMax=0xffff;
			nStatus= STATUS_VALUE_OUT_OF_RANGE;
		}
	}
	if(nStatus != STATUS_OK)
	{
		CreateErrorMessage_ForGetFuncs(nStatus,pszError,type,nMin,nMax);
		if (bIsIPAddress==FALSE)
		    FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (WORD)  ", pszError);
	}

	return nStatus;
}

int CXMLDOMElement::InnerGetAndValidateIPAddress(DWORD *pValue,char *pszError)
{
	char *pszIP = NIL(char);
	int nStatus=STATUS_OK;
	DWORD ip;

	get_nodeValue(&pszIP);

	if(pszIP)
	{
		ip=SystemIpStringToDWORD(pszIP, eHost);
	}
	else
		ip=0xFFFFFFFF;

	if (ip==0xFFFFFFFF)
	{
		if(NULL != pszIP && strcmp("255.255.255.255",pszIP) == 0)
		{
			*pValue =ip;
			nStatus=STATUS_OK;
		}
		else
		{
			FTRACESTR(eLevelDebug) << "CXMLDOMElement::InnerGetAndValidateIPAddress ";
			nStatus=STATUS_IP_ADDRESS_INVALID;
		}
	}
	else
	{
		nStatus = STATUS_OK;
		*pValue =ip;
	}

	return nStatus;
}

int CXMLDOMElement::InnerGetAndValidateIPAddress(mcTransportAddress* pValue,
                                                 char*)
{
        char* pszIP = NULL;
        mcTransportAddress ip;
        memset(&ip, 0, sizeof(mcTransportAddress));

        get_nodeValue(&pszIP);
        if (!pszIP || (!strcmp(pszIP, "[::]")) || (!strcmp(pszIP, "::")))
          pszIP = "";

        if (pszIP)
          stringToIp(&ip, pszIP);
        else
        {
          if (ip.ipVersion == (enIpVersion)eIpVersion4)
            ip.addr.v4.ip = 0xFFFFFFFF;
          else
            memset(ip.addr.v6.ip, 0xff, sizeof(ip.addr.v6.ip));
        }

        if (isIpTaNonValid(&ip))
        {
          if (pszIP && (strcmp("255.255.255.255", pszIP) == 0 || strcmp("255::255::255::255::255::255::255::255", pszIP) == 0))
          {
            memcpy(pValue, &ip, sizeof(mcTransportAddress));
            return STATUS_OK;
          }
        }
        else
        {
          memcpy(pValue, &ip, sizeof(mcTransportAddress));
          return STATUS_OK;
        }
        return STATUS_IP_ADDRESS_INVALID;
}

int CXMLDOMElement::GetAndValidate(CStructTm* time, char *pszError,int type)
{
	int nStatus = STATUS_OK;
	char * pVarValue;
	get_nodeValue(&pVarValue);
	if(!pVarValue)
		pVarValue = "";

	CStructTm tmpTime;
	nStatus=IXMLServiceUtilities::ParseTimeString(pVarValue,tmpTime);

	if(nStatus == STATUS_OK)
	{
		*time=tmpTime;
	}
	else
	{
		CreateErrorMessage_ForGetFuncs(nStatus,pszError,DATE_TIME,0,0);
		FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate (CStructTm)  ", pszError);
	}

	return nStatus;
}

int CXMLDOMElement::GetAndValidate(mcTransportAddress* pval,
                                   char *pszError,
                                   int type)
{
	mcTransportAddress dwVal;
	STATUS nStatus = InnerGetAndValidateIPAddress(&dwVal, pszError);
	if (nStatus == STATUS_OK)
	{
	  *pval = dwVal;
	  return STATUS_OK;
	}

	CreateErrorMessage_ForGetFuncs(nStatus, pszError, type, 0, 0);
	//FPTRACE2(eLevelInfoHigh, "CXMLDOMElement::GetAndValidate(IP)  ", pszError);

	return nStatus;
}

void CXMLDOMElement::CreateErrorMessage_ForGetFuncs(int nErrorStatus,
                                                    char* pszRetErrorBuffer,
                                                    int type,
                                                    int nMinLimit,
                                                    int nMaxLimit)
{
	char *pTextValue;
	char *szStatus=new char[256]; AUTO_DELETE_ARRAY(szStatus);
	if(m_pTextValue)
		pTextValue=m_pTextValue;
	else
		pTextValue="";

	if(!pszRetErrorBuffer || !m_szpNodeName)
		return;
///// vngr-10491 , when The node name is too long the szstatus will overflow
// so we left some space for the format message and cut the text to 200 characters
	if (strlen(pTextValue)>200)
		pTextValue[201]='\0';  //cut the size of message ID

	if((nErrorStatus == STATUS_NODE_LENGTH_TOO_LONG) && (nMaxLimit > 0))
		sprintf(szStatus,"Limited to max num of characters (%d)",nMaxLimit);
	if(nErrorStatus == STATUS_NODE_LENGTH_TOO_SHORT)
		sprintf(szStatus,"Limited to min num of characters (%d)",nMinLimit);
	else if((nErrorStatus == STATUS_VALUE_OUT_OF_RANGE) && (nMaxLimit > 0) && (nMaxLimit >= nMinLimit))
		sprintf(szStatus,"Value %s; The allowed range is %d - %d",pTextValue,nMinLimit,nMaxLimit);
	else if(nErrorStatus == STATUS_INVALID_KEY||nErrorStatus ==STATUS_MIN_MAX_MISSING_IN_KEY||nErrorStatus ==STATUS_API_INTERNAL_ERROR)
		sprintf(szStatus,"Key: %d ",type);
	else if(nErrorStatus == STATUS_ENUM_VALUE_INVALID)
		sprintf(szStatus,"Invalid enum: %s, Key: %d",pTextValue,type);
	else if(nErrorStatus == STATUS_NODE_VALUE_INVALID)
		sprintf(szStatus,"Invalid value: %s, Key: %d",pTextValue,type);
	else if(nErrorStatus == STATUS_IP_ADDRESS_INVALID)
		sprintf(szStatus,"Invalid IP address: %s, Key: %d",pTextValue,type);
	else if(nErrorStatus == STATUS_ATM_ADDRESS_INVALID)
		sprintf(szStatus,"Invalid ATM address: %s, Key: %d",pTextValue,type);
	else if(nErrorStatus == STATUS_DATE_TIME_INVALID)
		sprintf(szStatus,"Invalid date-time: %s, Key: %d",pTextValue,type);
	else
	{
		sprintf(pszRetErrorBuffer,"Element: '%s'",m_szpNodeName);
		return;
	}


	sprintf(pszRetErrorBuffer,"%s (Element: '%s')",szStatus,m_szpNodeName);
}

void CXMLDOMElement::CreateErrorMessage_ForAddFuncs(int nErrorStatus, char* pszRetErrorBuffer,
													const char *pChildName, int type,
													int nMinLimit,int nMaxLimit, DWORD nValue)
{
	char *pTextValue;
	char *szStatus=new char[256];
	if(m_pTextValue)
		pTextValue=m_pTextValue;
	else
		pTextValue="";

	if(!pszRetErrorBuffer || !m_szpNodeName)
	{
		delete [] szStatus;
		return;
	}

	///// vngr-10491 , when The node name is too long the szstatus will overflow
	// so we left some space for the format message and cut the text to 200 characters
		if (strlen(pTextValue)>200)
			pTextValue[201]='\0';  //cut the size of message ID

	if((nErrorStatus == STATUS_NODE_LENGTH_TOO_LONG) && (nMaxLimit > 0))
		sprintf(szStatus,"Limited to max num of characters (%d)",nMaxLimit);
	if(nErrorStatus == STATUS_NODE_LENGTH_TOO_SHORT)
		sprintf(szStatus,"Limited to min num of characters (%d)",nMinLimit);
	else if((nErrorStatus == STATUS_VALUE_OUT_OF_RANGE) && (nMaxLimit > 0) && (nMaxLimit >= nMinLimit))
		sprintf(szStatus,"Value: %d; The allowed range is %d - %d",(int)nValue,nMinLimit,nMaxLimit);
	else if(nErrorStatus == STATUS_INVALID_KEY||nErrorStatus ==STATUS_MIN_MAX_MISSING_IN_KEY||nErrorStatus ==STATUS_API_INTERNAL_ERROR)
		sprintf(szStatus," Key: %d ",type);
	else if(nErrorStatus == STATUS_ENUM_VALUE_INVALID)
		sprintf(szStatus,"Invalid enum: %s; Value: %d; Key: %d", pTextValue,(int)nValue,type);
	else if(nErrorStatus == STATUS_NODE_VALUE_INVALID)
		sprintf(szStatus,"Invalid value: %s; Value: %d; Key: %d", pTextValue,(int)nValue,type);
	else
	{
		sprintf(pszRetErrorBuffer,"Element: '%s', Father Element: '%s'", pChildName, m_szpNodeName);
		delete [] szStatus;
		return;
	}

	sprintf(pszRetErrorBuffer,"%s (Element: '%s', Father Element: '%s')",
								szStatus, pChildName, m_szpNodeName);
	delete [] szStatus;
}

CXMLDOMElement *BuildGeneralResponse(STATUS status,
									 const char *pStatusDescription,
									 const char *pStatusDescriptionEx,
									 DWORD MessageId,
									 WORD action,
									 DWORD userToken1,
									 DWORD userToken2)
{
	CXMLDOMElement *pXMLResponse=new CXMLDOMElement("RESPONSE_GENERAL");
	CXMLDOMElement *pStatusNode=pXMLResponse->AddChildNode("RETURN_STATUS");

	pStatusNode->AddChildNode("ID",status);
	pStatusNode->AddChildNode("DESCRIPTION",pStatusDescription);
	pStatusNode->AddChildNode("YOUR_TOKEN1",userToken1);
	pStatusNode->AddChildNode("YOUR_TOKEN2",userToken2);
	pStatusNode->AddChildNode("MESSAGE_ID",MessageId);
	if(pStatusDescriptionEx)
		pStatusNode->AddChildNode("DESCRIPTION_EX",pStatusDescriptionEx);
	pXMLResponse->AddChildNode("ACTION_TYPE",action,ACTION_TYPE_ENUM);

	return pXMLResponse;
}
