//=============================================================================
//                            M3C
//            Copyright (C) 2000 Polycom Israel Ltd.
//                   All Rights Reserved.
//-----------------------------------------------------------------------------
// This file contains confidential information proprietary to Polycom Israel Ltd.
// The use or disclosure of any information contained in this file without the
// written consent of an officer of Polycom Israel Ltd. is expressly forbidden.
//-----------------------------------------------------------------------------
// FILE: psosxml.h
// SUBSYSTEM: Audio Bridge
// PROGRAMMER: Dmitry Hayes
//-----------------------------------------------------------------------------
// Who | Date       | Description
//-----------------------------------------------------------------------------
// Dima| May-13-2002|  created
//=============================================================================


#ifndef _PSOS_XML_
#define _PSOS_XML_

#include <string>
#include "NStream.h"
#include "StrArray.h"
#include "IpAddressDefinitions.h"

#define DUMP_BUF_SIZE              90000
#define COMMPRESS_BUFFER		   10000

//get (and validate) the value of a child element, according to the supplied type.
//returns status only when the node is not missing and the status is not OK
#define GET_VALIDATE_CHILD(parent,element,value,Type) \
if (parent!=NULL)\
{\
	nStatus=parent->GetAndVerifyChildNodeValue(element,value,pszError,Type); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_NODE_MISSING && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus != STATUS_NODE_LENGTH_TOO_SHORT && \
       nStatus != STATUS_NODE_LENGTH_TOO_LONG && \
       nStatus != STATUS_VALUE_OUT_OF_RANGE && \
       nStatus != STATUS_IP_ADDRESS_INVALID) \
		return nStatus;\
}

//get (and validate string in US-ASCII encoding) the value of a child element, according to the supplied type
//returns status only when the node is not missing and the status is not OK
#define GET_VALIDATE_ASCII_CHILD(parent,element,value,Type) \
if (parent!=NULL)\
{\
    nStatus=parent->GetAndVerifyChildNodeValue(element,value,pszError,Type, true); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_NODE_MISSING && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus != STATUS_IP_ADDRESS_INVALID) \
		return nStatus;\
}

//get (and validate) the value of a mandatory child element, according to the supplied type.
#define GET_VALIDATE_MANDATORY_CHILD(parent,element,value,Type) \
if (parent!=NULL) \
{\
	nStatus=parent->GetAndVerifyChildNodeValue(element,value,pszError,Type); \
	if(nStatus!=STATUS_OK&&nStatus!=STATUS_ENUM_VALUE_INVALID && nStatus!=STATUS_IP_ADDRESS_INVALID)\
		return nStatus; \
}\
else\
{\
	if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
		return STATUS_NODE_MISSING;\
}

//get (and validate string in US-ASCII encoding) the value of a child element, according to the supplied type
//returns status only when the node is not missing and the status is not OK
#define GET_VALIDATE_MANDATORY_ASCII_CHILD(parent,element,value,Type) \
if (parent!=NULL)\
{\
    nStatus = parent->GetAndVerifyChildNodeValue(element,value,pszError,Type, true); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus != STATUS_IP_ADDRESS_INVALID) \
		return nStatus; \
}\
else\
{\
	if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
		return STATUS_NODE_MISSING;\
}

//get (and validate) the value of the current element ,according to the supplied type.
//returns status only when the node is not missing and the status is not OK
#define GET_VALIDATE(pNode,value,Type) \
if (pNode!=NULL)\
{\
	nStatus=pNode->GetAndValidate(value,pszError,Type); \
	if(nStatus != STATUS_OK && \
       nStatus!=STATUS_NODE_MISSING && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus!=STATUS_IP_ADDRESS_INVALID) \
		return nStatus;\
}

//get (and validate string in US-ASCII encoding) the value of the current element ,according to the supplied type.
//returns status only when the node is not missing and the status is not OK
#define GET_VALIDATE_ASCII(pNode,value,Type) \
if (pNode!=NULL)\
{\
	nStatus=pNode->GetAndValidate(value,pszError,Type, true); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_NODE_MISSING && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus != STATUS_IP_ADDRESS_INVALID) \
		return nStatus;\
}

//get (and validate) the value of the mandatory current element, according to the supplied type.
#define GET_VALIDATE_MANDATORY(pNode,value,Type) \
if (pNode!=NULL)\
{\
	nStatus=pNode->GetAndValidate(value,pszError,Type); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus!=STATUS_IP_ADDRESS_INVALID) \
		return nStatus;\
}\
else\
{\
	if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
		return STATUS_NODE_MISSING;\
}

//get (and validate string in US-ASCII encoding) the value of the mandatory current element, according to the supplied type.
#define GET_VALIDATE_MANDATORY_ASCII(pNode,value,Type) \
if (pNode!=NULL)\
{\
	nStatus=pNode->GetAndValidate(value,pszError,Type, true); \
	if(nStatus != STATUS_OK && \
       nStatus != STATUS_ENUM_VALUE_INVALID && \
       nStatus != STATUS_IP_ADDRESS_INVALID) \
		return nStatus;\
}\
else\
{\
	if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
		return STATUS_NODE_MISSING;\
}

//get a mandatory node according to the supplied name
#define GET_MANDATORY_CHILD_NODE(parent, element, pChild) \
if (parent!=NULL) \
{\
	parent->getChildNodeByName(&pChild,element);\
	if(!pChild)\
	{\
		pChild=NULL;\
		if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
			return STATUS_NODE_MISSING;\
	}\
}\
else\
{\
	pChild=NULL;\
	if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
		return STATUS_NODE_MISSING;\
}

//get a node according to the supplied name
#define GET_CHILD_NODE(parent, element, pChild) \
if (parent!=NULL)\
	nStatus = parent->getChildNodeByName(&pChild,element);\
else\
	pChild=NULL;

//get first mandatory child node from a nodes list.
#define GET_FIRST_MANDATORY_CHILD_NODE(parent, element, pChild) \
if (parent!=NULL) \
{\
	parent->firstChildNode(&pChild,element); \
	if( !pChild ) \
	{\
		pChild=NULL;\
		if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
			return STATUS_NODE_MISSING;\
	}\
}\
else\
{\
	pChild=NULL;\
	if(IXMLServiceUtilities::ReturnErrorMsg(element, pszError)!=STATUS_OK)\
		return STATUS_NODE_MISSING;\
}

// Gets first child node from a nodes list
#define GET_FIRST_CHILD_NODE(parent, element, pChild) \
  if (NULL != parent) \
    parent->firstChildNode(&pChild, element); \
  else \
    pChild = NULL;

//get next child node from a nodes list.
#define GET_NEXT_CHILD_NODE(parent, element, pChild) \
if (parent!=NULL)\
	parent->nextChildNode(&pChild,element);\
else\
	pChild=NULL;

#define CHECK_STATUS(RestrictedValue)	if((RestrictedValue.m_nStatus == STATUS_OK) || ((RestrictedValue.m_nStatus == STATUS_NODE_MISSING) && RestrictedValue.m_bAllowMissing))  RestrictedValue.Reset(); else return RestrictedValue.m_nStatus;
#define CHECK_STATUS_plus_Action(RestrictedValue,Action)	if((RestrictedValue.m_nStatus == STATUS_OK) || ((RestrictedValue.m_nStatus == STATUS_NODE_MISSING) && RestrictedValue.m_bAllowMissing)) { if(RestrictedValue.m_nStatus == STATUS_OK) Action; RestrictedValue.Reset(); } else return RestrictedValue.m_nStatus;

///////////////////////////////////////////////////////////////////////////////////////


// translation of Microsoft's data types
//------------------------------------------

typedef enum {
	SEC_OK = 0, SEC_FALSE, ERR_FAIL, // contradiction between Microsofts and PSOS E_FAIL
	// to solve in time of mcms port to XP, now changed to S_FAIL
	ERR_INVALIDARG
} HRES;

typedef enum {
	ENUM_NODE_INVALID = 0,
	ENUM_NODE_ELEMENT = ENUM_NODE_INVALID + 1,
	ENUM_NODE_ATTRIBUTE = ENUM_NODE_ELEMENT + 1,
	ENUM_NODE_TEXT = ENUM_NODE_ATTRIBUTE + 1,
	ENUM_NODE_CDATA_SECTION = ENUM_NODE_TEXT + 1,
	ENUM_NODE_ENTITY_REFERENCE = ENUM_NODE_CDATA_SECTION + 1,
	ENUM_NODE_ENTITY = ENUM_NODE_ENTITY_REFERENCE + 1,
	ENUM_NODE_PROCESSING_INSTRUCTION = ENUM_NODE_ENTITY + 1,
	ENUM_NODE_COMMENT = ENUM_NODE_PROCESSING_INSTRUCTION + 1,
	ENUM_NODE_DOCUMENT = ENUM_NODE_COMMENT + 1,
	ENUM_NODE_DOCUMENT_TYPE = ENUM_NODE_DOCUMENT + 1,
	ENUM_NODE_DOCUMENT_FRAGMENT = ENUM_NODE_DOCUMENT_TYPE + 1,
	ENUM_NODE_NOTATION = ENUM_NODE_DOCUMENT_FRAGMENT + 1
} DOMNodeEnum;

//------------------------------------------
// definitions and limitation for the first vers of parser
//#define MAX_NUMB_MEMB_IN_NODELIST 50
const WORD MAX_NUMB_MEMB_IN_NODELIST = 50;
//------------------------------------------

class CXMLDOMNode;
class CXMLDOMAttribute;
class CStructTm;
class CObjString;
//class CAtmAddr;
class ZlibEngine;

class CXMLDOMNamedNodeMap {
	//----------------------------------------------------------------
	// part of standart Microsoft interface
public:
	HRES get_length(long *listLength);
	HRES getNamedItem(char* name, CXMLDOMNode **namedItem);
	HRES setNamedItem(CXMLDOMNode *newItem);
	HRES nextNode(/* [retval][out] */CXMLDOMNode **nextItem);
	HRES reset(void);
	// end of standart Microsoft interface
	//----------------------------------------------------------------
public:
	CXMLDOMNamedNodeMap(void);
	~CXMLDOMNamedNodeMap(void);
protected:
	CXMLDOMNode* m_pNodeListHead;
	CXMLDOMNode* m_pNodeListLast;
	CXMLDOMNode* m_pNodeListCurrent;
	WORD m_wNumbMembersInArray;

};
//-------------------------------------------------------------------
class CXMLDOMNodeList {

public:
	//----------------------------------------------------------------
	// part of standart Microsoft interface

	HRES get_length(/* [retval][out] */long *listLength);
	HRES nextNode(/* [retval][out] */CXMLDOMNode **nextItem);
	HRES nextNode(/* [retval][out] */CXMLDOMNode **nextItem, char *pName);

	HRES reset(void);
	HRES getNodeByName(CXMLDOMNode **Item, char *pName,
			char *pszRetErrorBuffer = NULL, int *status = NULL);
	HRES getNodeValueByName(char *pName, char** name);
	HRES getNodeDecValueByName(char *pName, int *Value);

	// end of standart Microsoft interface
	//----------------------------------------------------------------

	int GetAndVerifyNodeValue(char* pszNodeName, CObjString& strValue,
			char *pszError, int type = -1, bool isAscii = false);
	int GetAndVerifyNodeValue(char* pszNodeName, std::string& pValue,
			char *pszError, int type = -1, bool isAscii = false);
	int GetAndVerifyNodeValue(char* pszNodeName, char **pValue, char *pszError,
			int type = -1, bool isAscii = false);
	int GetAndVerifyNodeValue(char* pszNodeName, char* pValue, char *pszError,
			int type = -1, bool isAscii = false);

	int GetAndVerifyNodeValue(char* pszNodeName, DWORD* pValue, char *pszError,
			int type = -1);
	int GetAndVerifyNodeValue(char* pszNodeName, WORD* pValue, char *pszError,
			int type = -1);
	int GetAndVerifyNodeValue(char* pszNodeName, BYTE* pValue, char *pszError,
			int type = -1);
	int GetAndVerifyNodeValue(char* pszNodeName, CStructTm* time,
			char *pszError, int type = -1);
	int GetAndVerifyNodeValue(char* pszNodeName, int* pValue, char *pszError,
			int type = -1);
	// IpV6
	int GetAndVerifyNodeValue(char* pszNodeName, mcTransportAddress* pValue,
			char *pszError, int type = -1);
	//int GetAndVerifyNodeValue(char* pszNodeName,DWORD_IP *pValue,char *pszError,char *pszType=NULL);


	//int GetAndVerifyNodeValue(char* pszNodeName,CAtmAddr *pValue,char *pszError,int type=-1);


public:
	HRES addNode(/* [retval][out] */CXMLDOMNode *nextItem);
	CXMLDOMNodeList(void);
	virtual ~CXMLDOMNodeList(void);
protected:
	// simplest implementation - will be changed to list in next versions
	CXMLDOMNode* m_pNodeListHead;
	CXMLDOMNode* m_pNodeListLast;
	CXMLDOMNode* m_pNodeListCurrent;

	WORD m_wNumbMembersInArray;
	// number of next returned item
	WORD m_wCurrentItem;
};
//-------------------------------------------------------------------
class CXMLDOMNode {
public:
	//----------------------------------------------------------------
	// part of standart Microsoft interface
	virtual HRES get_nodeType(DOMNodeEnum *type)=0;
	virtual HRES get_nodeName(/* [retval][out] */char** name);

	virtual HRES get_childNodes(CXMLDOMNodeList **childList) = 0;
	virtual HRES get_firstChild(/* [retval][out] */CXMLDOMNode **firstChild) = 0;

	//    virtual HRES  get_previousSibling(/* [retval][out] */ CXMLDOMNode **previousSibling) = 0;

	//   virtual HRES  get_nextSibling(/* [retval][out] */ CXMLDOMNode **nextSibling) = 0;
	virtual HRES get_nodeValue(char** value)=0;
	virtual HRES get_nodeValue(CObjString &value)=0;
	// end of standart Microsoft interface
	//----------------------------------------------------------------

	char* RemoveCommentsFromString(const char* szGivenString);//, bool & outIsWasAllocation);

public:
	//----------- part of non standart interface , can be changed ----
	CXMLDOMNode(void);
	CXMLDOMNode(const CXMLDOMNode& other);
	virtual ~CXMLDOMNode(void);
	virtual void DumpDataAsStringWithAttribute(
			/* [retval][out] */char** szDumpStr,/* [retval][out] */
			DWORD* dwStrLen, DWORD offset = 0, BYTE bPrettyXml = FALSE)=0;
	virtual HRES Parse(const char** szStringToParse)=0;
	const char* PointToNextReadableChar(const char* string);
	virtual HRES DumpDataAsStringArrayEx(CStrArray& strArray,
			WORD &Compressionlevel);
	virtual DWORD GetDumpStringLength(int offset, BYTE bPrettyXml = FALSE);
	BYTE GetNodeName(const char* *pPlaceToReadFrom);
	void GoToNon_EmptyChar(const char* *pPlaceToReadFrom);
	HRES set_nodeName(const char *name);
	virtual HRES DumpDataAsStringImpl(CStrArray* strArray,
                                      char* longstr,
                                      DWORD offset,
                                      int &index,
                                      BYTE bPretty = FALSE);

    virtual HRES DumpDataAsStringImpl(std::ostream &ostr,
                                      DWORD offset,
                                      BYTE  bPrettyXml);

	virtual void ChangeSpecialChar(const char* org_str,
			BYTE bRemoveSpecialChar, char* new_str);
	virtual int NeededSizeForNewArray(const char* org_str,
			BOOL isAllocSpaceForSpecialChar = TRUE);

	CXMLDOMNode* GetParent();
	void SetParent(CXMLDOMNode* pParent);
	void GetHierarchyString(char* pszHierarchy);
	CXMLDOMNode* m_pNextSibling;
protected:
	char* m_szpNodeName;
	CXMLDOMNode* m_pParent;

protected:
	virtual HRES get_childNodesNoReset(CXMLDOMNodeList **childList) {
		return ERR_FAIL;
	}
	;

};
//--------------------------------------------------------------------
class CXMLDOMElement: public CXMLDOMNode {
	friend class CXMLDOMNodeList;
public:
	//----------------------------------------------------------------
	// part of standart Microsoft interface
	virtual HRES get_nodeType(DOMNodeEnum *type);
	virtual HRES get_tagName(/* [retval][out] */char** tagName);
	virtual HRES getAttribute(/* [in] */char* name,/* [retval][out] */
			char** value);
	virtual HRES getAttributeNode(/* [in] */char* name,/* [retval][out] */
			CXMLDOMAttribute **attributeNode);
	virtual HRES get_nodeValue(char** value);
	virtual HRES get_nodeValue(CObjString &value);
	virtual HRES get_childNodes(CXMLDOMNodeList **childList);
	virtual HRES get_firstChild(/* [retval][out] */CXMLDOMNode **firstChild);

	// end of standart Microsoft interface
	//----------------------------------------------------------------

	int GetAndValidate(CObjString& strValue, char *pszError, int type,
			bool isAscii = false);
	int GetAndValidate(std::string& strValue, char *pszError, int type,
			bool isAscii = false);
	int GetAndValidate(char** pValue, char *pszError, int type, bool isAscii =
			false);
	int GetAndValidate(char* pValue, char *pszError, int type, bool isAscii =
			false);

	int GetAndValidate(DWORD *pValue, char *pszError, int type);
	int GetAndValidate(CStructTm* time, char *pszError, int type);
	int GetAndValidate(int* pValue, char *pszError, int type);
	int GetAndValidate(BYTE* pValue, char *pszError, int type);
	int GetAndValidate(WORD* pValue, char *pszError, int type);
	// IpV6
	int GetAndValidate(mcTransportAddress* pValue, char *pszError, int type);
	//	int GetAndValidate(CAtmAddr *pValue,char *pszError,int type);


	//HRES GetChildNodesByName(char *pName,CXMLDOMElement **pList,int &size);
public:
	CXMLDOMElement(void);
	CXMLDOMElement(const char *pName, const char *pValue);
	CXMLDOMElement(const char *pName);
	CXMLDOMElement(const char *pName, int nValue);
	CXMLDOMElement(const CXMLDOMElement& other);
	virtual ~CXMLDOMElement(void);
	void SetValue(int nValue);
	void SetValue(const char* ezText, BOOL isRmSpecialChar = TRUE);
	void SetValue(DWORD nValue);
	void SetValue(long nValue);
	void SetValue(WORD nValue);
	void SetValue(BYTE nValue);
	void AddAttribute(const CXMLDOMAttribute* attribite);
	void AddAttribute(const char* pszName, const char* pszValue);
	void AddChildNode(CXMLDOMNode* element);
	CXMLDOMElement* AddChildNode(const char *pChildName);
	CXMLDOMElement* AddChildNodeWithNsPrefix(const char *pChildName, const char *pszNsPrefix);
	CXMLDOMElement* AddChildNode(const std::string& sChildName);
	CXMLDOMElement* AddChildNodeWithNsPrefix(const std::string& sChildName, const std::string &sNsPrefix);
	CXMLDOMElement* AddChildNode(const char *pChildName, int nValue);
	CXMLDOMElement* AddChildNode(const char *pChildName, int nValue,int type);
	CXMLDOMElement* AddChildNode(const char *pChildName, DWORD nValue, int type = -1);
	CXMLDOMElement* AddChildNodeWithNsPrefix(const char *pChildName, const char *pszNsPrefix,
			DWORD nValue, int type = -1);
	CXMLDOMElement* AddChildNode(const char *pChildName, const CStructTm &time);
	CXMLDOMElement* AddChildNode(const char *pChildName, WORD nValue);
	CXMLDOMElement* AddChildNode(const char *pChildName, long nValue);
	CXMLDOMElement* AddChildNode(const char *pChildName, const char *pValue,
			BOOL isRmSpecialChar = TRUE);
	CXMLDOMElement* AddChildNodeWithNsPrefix(const char *pChildName, const char *pszNsPrefix, const char *pValue,
			BOOL isRmSpecialChar = TRUE);
	CXMLDOMElement* AddChildNode(const char *pChildName, CObjString &strValue);
	CXMLDOMElement* AddChildNode(const char *pChildName,
			const std::string &strValue);
	CXMLDOMElement* AddChildNode(const char *pChildName,
			const COstrStream &strStream);
	// IpV6
	CXMLDOMElement* AddIPChildNode(const char *pChildName,
			mcTransportAddress IPAddr, BYTE bIsIpV6 = FALSE);
	CXMLDOMElement* AddIPv6ChildNode(const char *pChildName,
			const APIU8* IPv6Addr6, const char *pMask = NULL, BOOL brackets =
					FALSE);
	//CXMLDOMElement* AddChildNode(const char *pChildName,const CAtmAddr *atmAdd);
	void DumpDataAsStringWithAttribute(/* [retval][out] */char** szDumpStr,/* [retval][out] */
			DWORD* dwStrLen, DWORD offset = 0, BYTE bPrettyXml = FALSE);
	virtual HRES Parse(const char** szStringToParse);
	virtual HRES DumpDataAsStringArrayEx(CStrArray& strArray,
			WORD &Compressionlevel, BYTE bResetArray = TRUE, BYTE bPrettyXml =
					FALSE);
	virtual HRES DumpDataAsLongStringEx(char** longstr,
			BYTE bPrettyXml = FALSE);

	virtual HRES DumpDataAsStringImpl(CStrArray* strArray,
                                      char* longstr,
                                      DWORD offset,
                                      int &index,
                                      BYTE bPrettyXml);

	virtual HRES DumpDataAsStringImpl(std::ostream &ostr,
                                      DWORD offset,
                                      BYTE  bPrettyXml);


	virtual DWORD GetDumpStringLength(int offset, BYTE bPrettyXml = FALSE);
	BYTE GetAttributes(const char**pPlaceToReadFrom);
	BYTE GetElementText(const char**pPlaceToReadFrom);
	BYTE GetChildNodes(const char**pPlaceToReadFrom);
	virtual HRES get_childNodesNoReset(CXMLDOMNodeList **childList);

	int GetAndVerifyChildNodeValue(char* pszNodeName, std::string& strValue,
			char *pszError, int type = -1, bool isAscii = false);
	int GetAndVerifyChildNodeValue(char* pszNodeName, CObjString& strValue,
			char *pszError, int type = -1, bool isAscii = false);
	int GetAndVerifyChildNodeValue(char* pszNodeName, char **pValue,
			char *pszError, int type = -1, bool isAscii = false);
	int GetAndVerifyChildNodeValue(char* pszNodeName, char* pValue,
			char *pszError, int type = -1, bool isAscii = false);

	int GetAndVerifyChildNodeValue(char* pszNodeName, DWORD* pValue,
			char *pszError, int type = -1);
	int GetAndVerifyChildNodeValue(char* pszNodeName, WORD* pValue,
			char *pszError, int type = -1);
	int GetAndVerifyChildNodeValue(char* pszNodeName, BYTE* pValue,
			char *pszError, int type = -1);
	int GetAndVerifyChildNodeValue(char* pszNodeName, CStructTm* time,
			char *pszError, int type = -1);
	int GetAndVerifyChildNodeValue(char* pszNodeName, int* pValue,
			char *pszError, int type = -1);
	// IpV6
	int GetAndVerifyChildNodeValue(char* pszNodeName,
			mcTransportAddress* pValue, char *pszError, int type = -1);
	//	int GetAndVerifyChildNodeValue(char* pszNodeName,DWORD_IP *pValue,char *pszError,char *pszType=NULL);

	virtual STATUS WriteXmlFile(const char *file_name);

	HRES nextChildNode(/* [retval][out] */CXMLDOMElement **nextItem);
	HRES nextChildNode(CXMLDOMElement **nextItem, char *pName);
	HRES firstChildNode(CXMLDOMElement **nextItem);
	HRES firstChildNode(CXMLDOMElement **nextItem, char *pName);
	HRES ResetChildList();
	HRES getChildNodeValueByName(char *pName,char**name);
	HRES getChildNodeDecValueByName(char *pName, int *Value);
	BYTE MsValidate(char ** pstrError);
	HRES getChildNodeByName(CXMLDOMElement **Item, char *pName,
			char *pszRetErrorBuffer = NULL, int *status = NULL);

private:
	char* m_pTextValue;
	CXMLDOMNamedNodeMap m_MapAttributes;
	CXMLDOMNodeList m_ListChildElements;
	HRES AddStringToDump(CStrArray* strArray, char* longstr, int &index,
			char *pStr);

	int InnerGetAndValidate(DWORD *pValue, char *pszError, int type,
			int &nMinLen, int &nMaxLen,BYTE &bisIPAddress);

	int InnerGetAndValidate(int *pValue, char *pszError, int type,
			int &nMinLen, int &nMaxLen,BYTE &bisIPAddress);

	int InnerGetAndValidate(char** pValue, char *pszError, int type,
			int &nMinLen, int &nMaxLen);

	void CreateErrorMessage_ForGetFuncs(int nErrorStatus,
			char* pszRetErrorBuffer, int type, int nMinLimit, int nMaxLimit);
	void CreateErrorMessage_ForAddFuncs(int nErrorStatus,
			char* pszRetErrorBuffer, const char *pChildName, int type,
			int nMinLimit, int nMaxLimit, DWORD nValue);
	int InnerGetAndValidateIPAddress(DWORD *pValue, char *pszError);
	int
			InnerGetAndValidateIPAddress(mcTransportAddress *pValue,
					char *pszError);
	CXMLDOMElement* AddIPChildNode(const char *pChildName, DWORD dwIP);
	//int GetAndValidate(DWORD_IP *pValue,char *pszError,char *pszType);


};
//--------------------------------------------------------------------
class CXMLDOMAttribute: public CXMLDOMNode {
public:
	//----------------------------------------------------------------
	// part of standart Microsoft interface
	virtual HRES get_name( /* [retval][out] */char** attributeName);

	virtual HRES get_nodeType(DOMNodeEnum *type);
	virtual HRES get_nodeValue(char** value);
	virtual HRES get_nodeValue(CObjString& value);
	virtual HRES get_childNodes(CXMLDOMNodeList **childList);
	virtual HRES get_firstChild(/* [retval][out] */CXMLDOMNode **firstChild);
	virtual HRES get_lastChild(/* [retval][out] */CXMLDOMNode **lastChild);

	// end of standart Microsoft interface
	//----------------------------------------------------------------
public:
	CXMLDOMAttribute(void);
	CXMLDOMAttribute(const CXMLDOMAttribute& other);
	virtual ~CXMLDOMAttribute(void);
	void SetValueForElement(const char* ezValue);
	void DumpDataAsStringWithAttribute(/* [retval][out] */char** szDumpStr,/* [retval][out] */
			DWORD* dwStrLen, DWORD offset = 0, BYTE bPrettyXml = FALSE);
	virtual HRES Parse(const char** szStringToParse);
	BYTE GetAttributeName(const char** pPlaceToReadFrom);
	BYTE GetAttributeValue(const char** pPlaceToReadFrom);
private:
	char* m_szTextValue;

};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CXMLDOMDocument: public CXMLDOMNode {
public:
	virtual HRES load(/* [in] */const char* xmlSource,/* [retval][out] */
			unsigned char *isSuccessful);
	virtual HRES get_lastChild(/* [retval][out] */CXMLDOMNode **lastChild);
	virtual HRES get_firstChild(/* [retval][out] */CXMLDOMNode **firstChild);
	virtual HRES get_nodeType(DOMNodeEnum *type);
	virtual HRES get_childNodes(CXMLDOMNodeList **childList);
	virtual HRES get_nodeValue(char** value);
	virtual HRES get_nodeValue(CObjString& value);
	virtual HRES DumpDataAsStringArrayEx(CStrArray& strArray,
			WORD &Compressionlevel);
	virtual void DumpDataAsLongStringEx(/* [retval][out] */char** szDumpStr,
	/* [retval][out] */DWORD* dwStrLen, BYTE bPrettyXml = FALSE);
	virtual DWORD GetDumpStringLength(int offset, BYTE bPrettyXml = FALSE);
	CXMLDOMDocument(void);
	CXMLDOMDocument(const string & encodingCharset);
	CXMLDOMDocument(CXMLDOMElement *pRootElem, const string & encodingCharset);
	~CXMLDOMDocument(void);
	virtual void DumpDataAsStringWithAttribute(
			/* [retval][out] */char** szDumpStr,/* [retval][out] */
			DWORD* dwStrLen, DWORD offset = 0, BYTE bPrettyXml = FALSE);
	virtual HRES Parse(const char** szStringToParse);
	CXMLDOMElement* GetRootElement(void);
	virtual STATUS WriteXmlFile(const char *file_name);

private:
	const string m_EncodingCharset;
	CXMLDOMElement* m_pRootElement;
};

class IXMLServiceUtilities {
public:

	static int GetNumber(char* pszValue, BYTE& bIsNumber);
	static int ParseTimeString(char* pszTime, CStructTm& time);
	static int DwordToIPString(DWORD DwordIP, char* pszIP);
	static void CreateErrorMessage(const char* pszNodeName, int nErrorStatus,
			char* pszRetErrorBuffer, const char* pszFatherNodeName = 0,
			int nMaxLimit = 0, int nMinLimit = 0);
	static int ReturnErrorMsg(char* element, char* pszError);
};

#endif //_PSOS_XML_
