// XmlBuilder.c
// Ami Noy

// Includes files
//----------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <XmlDefs.h>
#include <XmlFormat.h>
#include <XmlErrors.h>
#include <XmlBuilder.h>
#include <CsHeader.h>
#include <XmlPrintLog.h>
#include <SharedDefines.h>

// Macros
//--------

#undef	 SRC
#define  SRC	"XmlBuilder"

//	Static variables
//-------------------
static xmlElemConstLenSt elemAndLenTable[] = {
	{startRootType, 	StartBracketsLen + NewLineSignLen}, // strlen("<>") + strlen('\n')
	{endRootType,		EndBracketsLen + NewLineSignLen},	// strlen("</>" + strlen('\n')
	{startParentType,	StartBracketsLen + NewLineSignLen}, // strlen("<>") + strlen('\n')
	{endParentType,		EndBracketsLen + NewLineSignLen},	// strlen("</>" + strlen('\n')
	{childValType,		StartBracketsLen + HexSignLen + EndBracketsLen  + NewLineSignLen}, // strlen("<>") + strlen ("0x") + strlen("</>") + strlen('\n')
	{childArrayType,	StartBracketsLen + EndBracketsLen  + NewLineSignLen}, // strlen("<>") + strlen("</>") + strlen('\n')
	{startListType,		StartBracketsLen + ListLen + NewLineSignLen}, // strlen("<>") + strlen("_list") + strlen('\n')
	{endListType,		EndBracketsLen	 + ListLen + NewLineSignLen}, // strlen("</>") + strlen("_list") + strlen('\n')
};

// Global variables
//------------------


// External variables
//------------------
extern int xmlTraceLevel;

//	External Routines:
//--------------------
extern char *SetMemoryToAsciiMessage(
	unsigned long	binAddress,
	int				size);
// Forward Declarations:
//----------------------


// Routines:
//----------
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
	int				depth)
{
	int totalLen = 0;
	int elemLen = strlen(pElement);
	int argLen	= 0;

	int	depthLen = depth * 2;// two characters of spaces ("  ") for every depth level

	if (arg)
		argLen = strlen(arg);

	totalLen += elemAndLenTable[elemType].constLen;

	if ((elemType == childValType)	||
		(elemType == childArrayType))
		elemLen = elemLen * 2;

	totalLen += (currentXmlLen + elemLen + argLen + depthLen);
		
	if (totalLen < XmlMessageLen)
		return TRUE;

	XmlPrintLog("No free memory at xml buffer for <%s>, max len = %d, sum len = %d", PERR, pElement, XmlMessageLen, totalLen);
	
	return FALSE;

}// IsXmlBufferHasSpace

static int MyStrLen(char *pMessage, int size, int *strlen) 
{
	int i;
	int status = 0;
	
	BOOL bNullFound = FALSE;
	
	for (i = 0; i < size; i++) {

		if (!pMessage[i]) {
			
			bNullFound 	= TRUE;
			*strlen 	= i;
			break;	
		}		

		if (!isprint((int)pMessage[i])) {
			
			XmlPrintLog("String isn't printable, size = %d, index in string = %d", PERR, size, i);
			status = E_XML_STRING_IS_NOT_PRINTABLE;
		}
	}
	
	if (!bNullFound) {
		XmlPrintLog("Invalid String, size = %d", PERR, size);
		*strlen = size;
		return E_XML_INVALID_STR_LEN;
	}	
	
	return status;

}// MyStrLen

//---------------------------------------------------------------------------
//
//	Function name:	AddXmlNode
//
//	Description:	Add space to string by the depth of nesting
//					
//	Return code:
//			
//---------------------------------------------------------------------------
static int AddXmlNode(char *message, int depth, int curXmlLen)
{
  int i;
  int len = curXmlLen;

  for (i = 0; i < depth; i++)
    len += sprintf(&message[len] , "  ");

  return len;
}

//---------------------------------------------------------------------------
//
//	Function name:	HandleUnionData
//
//	Description:	Build the part of XML message where the binary part is union
//					
//	Return code:
//			0					- success
//			negative value		- error
//---------------------------------------------------------------------------
static int HandleUnionData(
   	char				**ppMessage,
	int					*pUnionTypeTable, 
	char				*xml, 
	int					depth,
	int					*pCurXmlLen,
	int					nUnionTypes,
	int					sizeOfUnion)
{
	int status;
	int unionType;
	int	unionSize;
	
	char *prevMsgAddress;
	
	xmlUnionPropertiesSt 	*pXmlUnionProp;
	genXmlFormat 			*pUnionFormat;
	
	pXmlUnionProp = ((xmlUnionPropertiesSt *) (*ppMessage - sizeof(xmlUnionPropertiesSt)));
	
	unionType = pXmlUnionProp->unionType;
	unionSize = pXmlUnionProp->unionSize;
	
	if ((unionType < 0) || (unionType > nUnionTypes)) {
		XmlPrintLog("Invalid union type = %d", PERR, unionType);
		return 	E_XML_INVALID_PARAMETER;
	}
	
	if ((unionSize < 0) || (unionSize > sizeOfUnion)) {
		XmlPrintLog("Invalid union size = %d,  sizeOfUnion in format = %d", PERR, unionSize, sizeOfUnion);
		return 	E_XML_INVALID_LEN;
	}
	
	pUnionFormat = (genXmlFormat *) pUnionTypeTable[unionType];
	
	prevMsgAddress = *ppMessage;
	
	status = GenXmlBuilder(
			ppMessage,
			&pUnionFormat,
			xml,
			depth,
			1,
			pCurXmlLen);
	if (status) {

		XmlPrintLog("Union type = %d, union len = %d, status = %d",
			PERR,
			unionType,
			unionSize,
			status);
			
		return status;
	}
	
	*ppMessage = prevMsgAddress + unionSize;
	
	return 0;
	
}// HandleUnionData

//---------------------------------------------------------------------------
//
//	Function name:	HandleDynamicData
//
//	Description:	Build the part of XML message whereit has dynamic part
//					
//	Return code:
//			0					- success
//			negative value		- error
//---------------------------------------------------------------------------
static int HandleDynamicData(
   	char				**ppMessage,
	genXmlFormat		**ppFormat, 
	char				*xml, 
	int					depth,
	int					*pCurXmlLen)
{
	int  status = 0;
	int  strLen;

	genXmlFormat *ppTempFormat = *ppFormat;

	while ((*ppTempFormat).varType != charArrayVarType)
		(ppTempFormat)++;

	strLen = *((unsigned long *) *ppMessage);

    if (strLen < 0 || strLen > XmlMessageLen) {
    	XmlPrintLog("Invalid strLen = %d", PERR, strLen);
        return E_XML_INVALID_LEN;
    }
        
	(*ppTempFormat).val = strLen;

	status = GenXmlBuilder(
			ppMessage,
			ppFormat,
			xml,
			depth,
			1,
			pCurXmlLen);
	if (status) {

		XmlPrintLog("Type name = %s, len = %d, status = %d",
			PERR,
			(*ppFormat)->name,
			strLen,
			status);
			
		return status;
	}

	return 0;
		
}// HandleDynamicArray
 
//---------------------------------------------------------------------------
//
//	Function name:	BuildDynamicXmlMessage
//
//	Description:	Build the part of XML message where the binary part is dynamic
//					
//	Return code:
//			0					- success
//			negative value		- error
//---------------------------------------------------------------------------
static int BuildDynamicXmlMessage(
   	char				**ppMessage,
	dynamicTableStruct	*dynamicTable, 
	char				*xml, 
	int					depth,
	int					nElementsInTable,
	int					*pCurXmlLen)
{
	int i;
	int j;
	int status = 0;

	int typeCode;
	int typeLen;
	int numOfDynamicTypes;
	int sizeOfDynamicTypes;
	
	BOOL bTypeFound;
	BOOL bSizeProblem = FALSE;
	
	char *prevMsgAddress;

	genXmlFormat			*pFormat;
	xmlDynamicProperties	*pXmlDynamicProp;
	xmlDynamicHeader 		*pXmlDynamicHeader;
	dynamicTableStruct		*pTable;
	
	pXmlDynamicProp = (xmlDynamicProperties *) (*ppMessage - sizeof(xmlDynamicProperties));

	numOfDynamicTypes	= (int) pXmlDynamicProp->numberOfDynamicParts;
	sizeOfDynamicTypes	= (int) pXmlDynamicProp->sizeOfAllDynamicParts;

	if (numOfDynamicTypes < 0 ||  numOfDynamicTypes > 0xFFFF) {
		XmlPrintLog("Invalid num of dynamic types = %d, nElementsInTable = %d", PERR, numOfDynamicTypes, nElementsInTable);
		return E_XML_INVALID_PARAMETER;
	}
	
	if ((!numOfDynamicTypes && sizeOfDynamicTypes != 0) ||	
		(sizeOfDynamicTypes < 0)) {
		XmlPrintLog("Invalid size of dynamic types = %d, numOfDynamicTypes = %d", PERR, sizeOfDynamicTypes, numOfDynamicTypes);
		return E_XML_INVALID_LEN;
	}
	
	for (i = 0; i < numOfDynamicTypes; i++) {

		pXmlDynamicHeader = (xmlDynamicHeader*) *ppMessage;

		typeCode = (int) pXmlDynamicHeader->dynamicType;
		typeLen	 = (int) pXmlDynamicHeader->dynamicLength;
		
		if (typeCode < 0) {
			XmlPrintLog("Invalid type code = %d", PERR, typeCode);
			return E_XML_INVALID_PARAMETER;	
		}
		
		if (typeLen <= 0) {
			XmlPrintLog("Invalid type len = %d", PERR, typeLen);
			return E_XML_INVALID_LEN;			
		}
		
		if (typeLen > sizeOfDynamicTypes) {
			XmlPrintLog("Type code = %d, Invalid type len = %d, sizeOfDynamicTypes = %d", PERR, typeCode, typeLen, sizeOfDynamicTypes);
			bSizeProblem = TRUE;
		}
		
		bTypeFound = FALSE;
		
		prevMsgAddress = *ppMessage;
		
		for (j = 0; j < nElementsInTable; j++) {
			
			pTable = &dynamicTable[j];

			if (typeCode == pTable->type) {

				bTypeFound = TRUE;
				
				pFormat = (genXmlFormat *) pTable->format;

				if (bSizeProblem) {
					
					if (pFormat->name)
						XmlPrintLog("Field name = %s, xml type = %d, var type = %d", PERR, pFormat->name, pFormat->xmlType, pFormat->varType);
					
					bSizeProblem = FALSE;
				}
				
				status = GenXmlBuilder(
									ppMessage,
									&pFormat,
									xml,
									depth,
									1,
									pCurXmlLen);
				if (status) {

					XmlPrintLog("Field name: <%s>, xml type = %d, var type = %d",
						PWRN,
                        pFormat->name,
						pFormat->xmlType,
						pFormat->varType);

					return status;
				}

				*ppMessage = prevMsgAddress + typeLen;
				break;
			}
		}
		
		if (!bTypeFound) {
			
			XmlPrintLog("Dynamic Type %d not found, numOfDynamicTypes = %d, nElementsInTable = %d, ",
				 typeCode,
				 numOfDynamicTypes,
				 nElementsInTable);
				 
			return E_XML_DYNAMIC_TYPE_NOT_FOUND;	
		}
	}

	return status;

}// BuildDynamicXmlMessage

//---------------------------------------------------------------------------
//
//	Function name:	GenXmlBuilder
//
//	Description:	Recursive function that build XML message using the binary message 
//					and known format with rules to convert binary to XML.
//					
//	Return code:
//			0					- success
//			negative value		- error
//---------------------------------------------------------------------------
int GenXmlBuilder(
	char			**ppMessage,
	genXmlFormat	**ppFormat, 
	char			*xml, 
	int				depth,
	int				nElements,
	int				*pCurXmlLen)
{
	char sTempVal[256];
	char *elementName;

	int				i;
	int				beginXmlLine;
	unsigned long  	j;
	int status 		= 0;
	int nextDepth;
	
	int xmlType;
	int parentVarType;
	int childVarType;
	unsigned long	nFormatElements;
	
	int	strLength = 0;
	int	dataLen   = 0;
	int	preDefineLen = 0;

	unsigned long  	val = 0;
	int				valSizeInBytes;

	int	bIsHandle;

	for (i = 0; i < nElements; i++) {
	
		bIsHandle = FALSE;

		xmlType		= (*ppFormat)->xmlType;
		
		switch (xmlType) {	
		
		case noElemType:
		
#ifdef __NumOfElementsInFormatValidation__
			if (i != (nElements -1)) {
			
				XmlPrintLog("Number of elements is too big, xmlType = %d, varType = %d, name = <%s>, nElements = %d, val = %x",
					PERR,
					xmlType,
					(*ppFormat)->varType,
					(*ppFormat)->name,
					nElements,
					(*ppFormat)->val);	
								
				return E_XML_NUM_OF_ELEMENTS_TOO_BIG;
			}
#endif			
			break;
			
		case rootElemType:
			
			nFormatElements = (*ppFormat)->nElements;
			*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
			
			status = GenXmlBuilder(
							ppMessage,
							ppFormat,
							xml,
							depth + 1,
							nFormatElements,
							pCurXmlLen);
			if (status) {
	
				XmlPrintLog("root type, status = %d", PERR, status);

				return status;
			}

			break;

		case parentElemType:
			
			elementName = (*ppFormat)->name;
			parentVarType = (*ppFormat)->varType;

			if (elementName) {

				if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, NULL, startParentType, depth)) {

					AddXmlParentNodeStart(xml, elementName, &pCurXmlLen);
					nextDepth = depth + 1;
				
				} else
					return (E_XML_XML_BUFFER_TOO_SHORT + 1);
			
			} else 
				nextDepth = depth;
	
			nFormatElements = (*ppFormat)->nElements;

			// if the child is structure type, we get its pointer from the val field.
			// To avoid from changing the original format, we use temp pointer.
			if (parentVarType == structVarType) {

				unsigned long k;
				
				for (k = 0; k < nFormatElements; k++) {
	
					genXmlFormat *tempFormatPtr = (genXmlFormat*)((*ppFormat)->val);
					
					status = GenXmlBuilder(
											ppMessage,
											((genXmlFormat**) &tempFormatPtr),
											xml,
											nextDepth,
											1,//nFormatElements,
											pCurXmlLen);
					if (status) {
	
						XmlPrintLog("parent & struct type: Field name: <%s>, status = %d",
	                        PERR,
							elementName,
							status);
	
						return status;
					}
				}
				
//				if ((i == (nElements - 1)) && 
//					(nElements > 0)) {
				
				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
//				}

			} else if (parentVarType == dynamicVarType) {
			// if the child is dynamic type, we get its pointer from the val field.
			// otherwise, we get it from the next format.

				status = BuildDynamicXmlMessage(
									ppMessage,
									(dynamicTableStruct*) ((*ppFormat)->val),
									xml,
									nextDepth,
									nFormatElements,
									pCurXmlLen);
				if (status) {

					XmlPrintLog("parent & dynamic type: Field name: <%s>, status = %d",
						PERR,
                        elementName,
                        status);

					return status;
				}

				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
			
			} else if (parentVarType == dynamicDataVarType) {

				genXmlFormat *tempFormatPtr = (genXmlFormat*)((*ppFormat)->val);

				status = HandleDynamicData(
									ppMessage,
									(genXmlFormat**) &tempFormatPtr,
									xml,
									nextDepth,
									pCurXmlLen);
				if (status) {

					XmlPrintLog("parent & dynamic data type: Field name: <%s>, status = %d",
						PERR,
                        elementName,
                        status);

					return status;
				}

				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);

			} else if (parentVarType == unionVarType) {

				int *unionFormatTablePtr = (int*)((*ppFormat)->val);

				status = HandleUnionData(
									ppMessage,
									unionFormatTablePtr,
									xml,
									nextDepth,
									pCurXmlLen,
									(*ppFormat)->minVal,
									(*ppFormat)->maxVal);
				if (status) {
		
					XmlPrintLog("parent & union type: Field name: <%s>, status = %d",
						PERR,
                        elementName,
                        status);

					return status;
				}

				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
				
			} else {
				
				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
			
				status = GenXmlBuilder(
									ppMessage,
									ppFormat,
									xml,
									nextDepth,
									nFormatElements,
									pCurXmlLen);
				if (status) {

					XmlPrintLog("parent & child type: Field name: <%s>, status = %d",
						PERR,
                        elementName,
                        status);

					return status;
				}
			}
			
			if (elementName) {

				if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, NULL, endParentType, depth)) {
					AddXmlParentNodeEnd(xml, elementName, &pCurXmlLen);
				} else 
					return (E_XML_XML_BUFFER_TOO_SHORT + 2);
			}
			
			break;

		case childElemType:
		case siblingElemType:
			
			elementName = (*ppFormat)->name;
			childVarType = (*ppFormat)->varType;

			switch (childVarType) {

			case charVarType:
				
				valSizeInBytes = sizeof(char);
				
				val = *(unsigned char *) *ppMessage;

				*ppMessage += valSizeInBytes;

				break;

			case stringVarType:

				preDefineLen = (*ppFormat)->val;
				
				status = MyStrLen(*ppMessage, preDefineLen, &strLength);
				if (status) {
								
					XmlPrintLog("stringVarType: name = <%s>, max string len = %d, status = %d",
							PERR,
							elementName,
							preDefineLen,
							status);
					
					status = 0;
				}
				
////				strLength = strlen(*ppMessage);
//				
//				if (strLength > preDefineLen) {
//					
//					XmlPrintLog("stringVarType: name = <%s>, strLength = %d, max string len = %d",
//							PERR,
//							elementName,
//							strLength,
//							preDefineLen);
//					
//					strLength = preDefineLen;
//				}
						               
				if (IsXmlBufferHasSpace(*pCurXmlLen + strLength, elementName, *ppMessage, childArrayType, depth)) {
					AddXmlChildArrayNode(xml, elementName, *ppMessage, stringVarType, strLength, &pCurXmlLen);
				} else
					return (E_XML_XML_BUFFER_TOO_SHORT + 3);

                //binary message need to use length from format, but xml need real length of string - this should be test
				*ppMessage += preDefineLen;

				bIsHandle = TRUE;

				break;
		
            case charArrayVarType:

                dataLen = (*ppFormat)->val;
            
               	if (dataLen < 0 || dataLen > XmlMessageLen) {
           						
					XmlPrintLog("charArrayVarType: name = <%s>, dataLen = %d",
						PERR,
						elementName,
						dataLen);
						
                    return E_XML_INVALID_LEN;                   
               	}
      
      			if (IsXmlBufferHasSpace(*pCurXmlLen + (dataLen * 3 + 1), elementName, NULL, childArrayType, depth)) {
      			
	      			int index;
	      			
      				char *pStartXmlLine = &xml[*pCurXmlLen];
      				      				
      				sTempVal[1] = 0;
      				
      				*pCurXmlLen = AddXmlNode(xml, depth, *pCurXmlLen);
					*pCurXmlLen += sprintf(&xml[*pCurXmlLen], "<%s>", elementName);	
					
					for (index = 0; index < dataLen; index++) {
						
						sTempVal[0] = *(*ppMessage + index);
						*pCurXmlLen += sprintf(&xml[*pCurXmlLen], " %02x", (unsigned char) sTempVal[0]);			
					}
					if (dataLen)
						*pCurXmlLen += sprintf(&xml[*pCurXmlLen], " ");
									
      				*pCurXmlLen += sprintf(&xml[*pCurXmlLen], "</%s>", elementName);
      				
      				if (xmlTraceLevel == emXmlTraceFull)
   						XmlPrintLog("%s, {%s}", PLOG, pStartXmlLine, SetMemoryToAsciiMessage((unsigned long)*ppMessage, dataLen));
   						
   					*pCurXmlLen += sprintf(&xml[*pCurXmlLen], "\n");
   					
      			} else
                    return (E_XML_XML_BUFFER_TOO_SHORT + 3);

                *ppMessage += dataLen;

                bIsHandle = TRUE;

                break;
    		
			case shortVarType:
			
				valSizeInBytes = sizeof(short);
				
				val = *(unsigned short *) *ppMessage;

				*ppMessage += valSizeInBytes;
			
				break;
			
			case shortListVarType:
			
				valSizeInBytes = sizeof(short);
				
				nFormatElements = (*ppFormat)->nElements;

				// in case we are dealing with an array
				if (nFormatElements) {
					
					if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, NULL, startListType, depth)) {
						AddXmlListNodeStart(xml, elementName, &pCurXmlLen);
					} else 
						return (E_XML_XML_BUFFER_TOO_SHORT + 4);

					depth += 1;
					for (j = 0; j < nFormatElements; j++) {

						val = *(unsigned long*) *ppMessage;

						*ppMessage += valSizeInBytes;
						
						sprintf(sTempVal, "%lx",(long unsigned int)val);

						if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, sTempVal, childValType, depth))
						{
							AddXmlChildValNode(xml, elementName, val, shortListVarType, valSizeInBytes, &pCurXmlLen);
						}
						else
						{
							return (E_XML_XML_BUFFER_TOO_SHORT + 5);
						}
					}
					depth -= 1;
			
					if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, NULL, endListType, depth)) {
						AddXmlListNodeEnd(xml, elementName, &pCurXmlLen);
					} else
						return (E_XML_XML_BUFFER_TOO_SHORT + 6);
					
					bIsHandle = TRUE;
				
				} else {

					XmlPrintLog("Empty short list, elementName = <%s>",
						PWRN,
                        elementName);
				}

				break;
	
			case longVarType:
				
				valSizeInBytes = sizeof(long);
				
				val = *(unsigned long *) *ppMessage;

				*ppMessage += valSizeInBytes;

				break;

			case longListVarType:
					
				valSizeInBytes = sizeof(long);
				
				nFormatElements = (*ppFormat)->nElements;

				if (nFormatElements) {
					
					if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, NULL, startListType, depth)) {
						AddXmlListNodeStart(xml, elementName, &pCurXmlLen);
					} else
						return (E_XML_XML_BUFFER_TOO_SHORT + 7);

					depth += 1;
					for (j = 0; j < nFormatElements; j++) {

						val = *(unsigned long *) *ppMessage;

						*ppMessage += valSizeInBytes;
	
						sprintf(sTempVal, "%x",(unsigned int)val);

						if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, sTempVal, childValType, depth)) {
							AddXmlChildValNode(xml, elementName, val, longListVarType, valSizeInBytes, &pCurXmlLen);
						} else 
							return (E_XML_XML_BUFFER_TOO_SHORT + 8);
					}
					depth -= 1;

					if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, NULL, endListType, depth)) {
						AddXmlListNodeEnd(xml, elementName, &pCurXmlLen);
					} else 
						return (E_XML_XML_BUFFER_TOO_SHORT + 9);
					
					bIsHandle = TRUE;
				
				} else {
					
					XmlPrintLog("Empty long list, elementName = <%s>",
						PWRN,
                        elementName);
				}

				
				break;

			default:
				
				bIsHandle = TRUE;
				
				valSizeInBytes = 0;
				
				XmlPrintLog("Unkown var type, type = %d, elementName = <%s>",
					PERR,
                    childVarType,
					elementName);

				break;

			}
				
			if (!bIsHandle) {

			  sprintf(sTempVal, "%x",(unsigned int )val);

				if (IsXmlBufferHasSpace(*pCurXmlLen, elementName, sTempVal, childValType, depth)) {
					AddXmlChildValNode(xml, elementName, val, childVarType, valSizeInBytes, &pCurXmlLen);
				} else 
					return (E_XML_XML_BUFFER_TOO_SHORT + 10);
			}

			*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);

			break;
		}		
	}
	
#ifdef __NumOfElementsInFormatValidation__
	if ((*ppFormat)->xmlType != noElemType) {
			
		XmlPrintLog("Number of elements is too small, xmlType = %d, varType = %d, name = <%s>, nElements = %d, val = %x",
			PERR,
			(*ppFormat)->xmlType,
			(*ppFormat)->varType,
			(*ppFormat)->name,
			nElements,
			(*ppFormat)->val);
			
		status = E_XML_NUM_OF_ELEMENTS_TOO_SMALL;			
	} 
#endif
		
	return status;

}// GenXmlBuilder

