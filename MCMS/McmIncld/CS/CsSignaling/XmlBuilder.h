#ifndef _XMLBUILDER_H_
#define _XMLBUILDER_H_

#include "XmlFormat.h"

#define ValIsNode		TRUE

#define AddXmlRootStart(message, element, pCurXmlLen) \
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "<%s>",element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s", PLOG, &message[beginXmlLine]);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");
	
	 
#define AddXmlRootEnd(message, element, pCurXmlLen) \
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "</%s>",element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s", PLOG, &message[beginXmlLine]);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");

#define AddXmlParentNodeStart(message, element, pCurXmlLen) \
{ \
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen = AddXmlNode(message, depth, **pCurXmlLen);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "<%s>",element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s", PLOG, &message[beginXmlLine]);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");\
}

#define AddXmlParentNodeEnd(message, element, pCurXmlLen) \
{ \
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen = AddXmlNode(message, depth, **pCurXmlLen);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "</%s>",element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s", PLOG, &message[beginXmlLine]);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");\
}

#define AddXmlChildValNode(message, element, val, varType, valSize, pCurXmlLen) \
{\
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen = AddXmlNode(message, depth, **pCurXmlLen);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "<%s>0x%x</%s>",\
			element,\
			val,\
			element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s, ( %s)", PLOG, &message[beginXmlLine], XmlPrintBytes((char *)&val, varType, valSize));\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");\
}

#define AddXmlChildArrayNode(message, element, val, varType, length, pCurXmlLen) \
{\
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen = AddXmlNode(message, depth, **pCurXmlLen);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "<%s>", element);\
	memcpy(&message[**pCurXmlLen], val, length);\
	**pCurXmlLen += length;\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "</%s>", element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s, { %s}", PLOG, &message[beginXmlLine], XmlPrintBytes((char *)&val[0], varType, length));\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");\
}

#define AddXmlListNodeStart(message, element, pCurXmlLen) \
{\
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen = AddXmlNode(message, depth, **pCurXmlLen);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "<%s_list>",element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s", PLOG, &message[beginXmlLine]);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");\
}

#define AddXmlListNodeEnd(message, element, pCurXmlLen) \
{\
	beginXmlLine = **pCurXmlLen;\
	**pCurXmlLen = AddXmlNode(message, depth, **pCurXmlLen);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "</%s_list>",element);\
	if (xmlTraceLevel == emXmlTraceFull) \
		XmlPrintLog("%s", PLOG, &message[beginXmlLine]);\
	**pCurXmlLen += sprintf(&message[**pCurXmlLen], "\n");\
}

#define StartBracketsLen 	2 // '<' '>'
#define EndBracketsLen 		3 // '<' '/' '>'
#define NewLineSignLen		1 // '/n'
#define HexSignLen			2 // '0' 'x'
#define ListLen				5 // "_list"


typedef enum {
	
	startRootType,
	endRootType,
	startParentType,
	endParentType,
	childValType,
	childArrayType,
	startListType,
	endListType,	

} xmlValidationElemTypes;

typedef struct {	

	int type;
	int constLen;

} xmlElemConstLenSt;

//---------------------------------------------------------------------------
//
//	Function name:	IsXmlBufferHasSpace
//
//	Description:	Check if buffer has enough space for element
//					
//	Return code:
//			
//---------------------------------------------------------------------------
int IsXmlBufferHasSpace(
	int				currentXmlLen,
	char			*pElement,
	char			*arg,
	int				elemType,
	int				depth);

int GenXmlBuilder(
	char			**ppMessage,
	genXmlFormat	**ppFormat, 
	char			*xml, 
	int				depth,
	int				nElements,
	int				*pCurXmlLen);

#endif // _XMLBUILDER_H_
