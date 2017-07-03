// BinBuilder.c
// Ami Noy

// Includes files
//----------------
#include <stdio.h>
#include <string.h>
#include <XmlDefs.h>
#include <XmlFormat.h>
#include <XmlErrors.h>
#include <Parser.h>
#include <CsHeader.h>
#include <XmlPrintLog.h>
#include <SharedDefines.h>

// Macros
//--------

#undef	SRC
#define SRC	"BinBuilder"

//#define __ElementNameValidation__
//#define __ValueValidation__

#define SizeOfXmlLine	1024

// Static variables
//-------------------
#ifdef __BinXmlTrace__
static char	printBuffer[SizeOfXmlLine];
#endif

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------

// Forward Declarations:
//----------------------
int GenBinBuilder(
    char            **ppMessage,
    genXmlFormat    **ppFormat, 
    char            **xml,
    int             nElements,
    int				binMsgSize,
    int				*pCurBinMsgSize);

// Routines:
//----------

#ifdef __BinXmlTrace__
int BinBuilderPrintXmlMessage(
	char 	*xml)
{
	char	*pStartXmlLine;
	char	*pEndXmlLine;
	
	int		nBytesWritten = 0;
	int 	xmlLineSize;

	pStartXmlLine = xml;
	
	pEndXmlLine = strstr(pStartXmlLine, "\n");
	
	if (!pEndXmlLine) {
		XmlPrintLog("Can't find string", PERR);
		return E_XML_NULL_PTR;	
	}
	
	xmlLineSize = pEndXmlLine - pStartXmlLine;

	if (xmlLineSize > SizeOfXmlLine) {
		XmlPrintLog("Invalid size, xml line size = %d, buffer to print size = %d", PERR, xmlLineSize, SizeOfXmlLine);
		return E_XML_INVALID_LEN;
	}
	
	memcpy(
		&printBuffer[0],
		pStartXmlLine,
		xmlLineSize);

//	pStartXmlLine = strstr(pStartXmlLine, ">");
//	pStartXmlLine += 1;
//	pEndXmlLine = strstr(pStartXmlLine, "</");
//	
//	nBytesWritten += sprintf(&printBuffer[xmlLineSize], "( %s )", pStartXmlLine);
//	
	printBuffer[xmlLineSize + nBytesWritten] = '\0';
	
	XmlPrintLog("%s", PLOG, printBuffer);	
	
	return 0;	

}// BinBuilderPrintXmlMessage
#endif // __BinXmlTrace__
		
//---------------------------------------------------------------------------
//
//	Function name:	ElementNameValidation
//
//	Description:	Check if the name in the format and the name in the XML
//					message are identical
//					
//	Return code:	0				- success
//					negative value	- fail
//			
//---------------------------------------------------------------------------
#ifdef __ElementNameValidation__
int ElementNameValidation(
	char	*formatElemName,
	char	*xmlLine)
{
	char *xmlElemName;

	int length = strlen(formatElemName);

	// skip on the '<' char
	xmlElemName = strstr(xmlLine, "<");
	xmlElemName += 1;

	if (strncmp(
		formatElemName,
		xmlElemName,
		length)) {

		XmlPrintLog("format name = <%s>, xml name = <%s>",
			PERR,
			formatElemName,
			xmlElemName);

		return E_XML_INVALID_PARAMETER;
	}

	return 0;
}
#endif // __ElementNameValidation__

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
	char				**xml,
	int					binMsgSize,
	int					*pCurBinMsgSize,
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
		XmlPrintLog("Invalid union size = %d", PERR, unionSize);
		return 	E_XML_INVALID_LEN;
	}
	
	pUnionFormat = (genXmlFormat *) pUnionTypeTable[unionType];
	
	prevMsgAddress = *ppMessage;
	
	status = GenBinBuilder(
			(char** )ppMessage,
			&pUnionFormat,
			(char**) xml,
			1,
			binMsgSize,
			pCurBinMsgSize);
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
//	Description:	Build the part of binary message where the XML part in dynamic
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int 	HandleDynamicData(				
   	char				**ppMessage,
	genXmlFormat		**ppFormat, 
	char				**xml, 
	int					nElementsInTable,
	int					binMsgSize,
	int					*pCurBinMsgSize)
{
	int status  = 0;
	int strLen;

	genXmlFormat *pTempFormat = *ppFormat;

	while ((*pTempFormat).varType != charArrayVarType)
		(pTempFormat)++;

	strLen = GetXmlVal(*xml, "length");

	if (strLen < 0 || strLen > XmlMessageLen) {
		XmlPrintLog("Invalid strLen = %d, name = %s", PERR, strLen, pTempFormat->name);
		return E_XML_INVALID_LEN;
	}
	
	(*pTempFormat).val = strLen;

	status = GenBinBuilder(
			ppMessage,
			ppFormat,
			xml,
			nElementsInTable,
			binMsgSize,
			pCurBinMsgSize);
	if (status) {

		XmlPrintLog("Type name = %s, len = %d, status = %d",
			PERR,
			(*ppFormat)->name,
			strLen,
			status);
			
		return status;
	}

	return 0;

}// HandleDynamicData

//---------------------------------------------------------------------------
//
//	Function name:	BuildDynamicBinMessage
//
//	Description:	Build the part of binary message where the XML part in dynamic
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int 	BuildDynamicBinMessage(				
   	char				**ppMessage,
	dynamicTableStruct	*dynamicTable, 
	char				**xml, 
	int					nElementsInTable,
	int					binMsgSize,
	int					*pCurBinMsgSize)
{
	int i;
	int j;
	int status = 0;

	int typeCode;
	int typeLen;
	int numOfDynamicTypes;
	int sizeOfDynamicTypes;

	BOOL bTypeFound;
	
	char *prevMsgAddress;
	
	genXmlFormat			*pFormat;
	xmlDynamicProperties	*pXmlDynamicProp;
	xmlDynamicHeader 		*pXmlDynamicHeader;
	dynamicTableStruct		*pTable;

	pXmlDynamicProp = (xmlDynamicProperties *) (*ppMessage - sizeof(xmlDynamicProperties));

	numOfDynamicTypes	= (int) pXmlDynamicProp->numberOfDynamicParts;
	sizeOfDynamicTypes	= (int) pXmlDynamicProp->sizeOfAllDynamicParts;

	if (numOfDynamicTypes < 0 ||  numOfDynamicTypes > 0xFFFF) {
		XmlPrintLog("Invalid num of dynamic types = %d", PERR, numOfDynamicTypes);
		return E_XML_INVALID_PARAMETER;
	}
	
	if ((!numOfDynamicTypes && sizeOfDynamicTypes != 0) ||	
		(sizeOfDynamicTypes < 0)) {
		XmlPrintLog("Invalid size of dynamic types = %d, numOfDynamicTypes = %d", PERR, sizeOfDynamicTypes, numOfDynamicTypes);
		return E_XML_INVALID_LEN;
	}
	
	for (i = 0; i < numOfDynamicTypes; i++) {

		pXmlDynamicHeader = (xmlDynamicHeader*) *ppMessage;

		typeCode = GetXmlVal(*xml, "dynamicType");
		typeLen	 = GetXmlVal(*xml, "dynamicLength");
		
		if (typeCode < 0) {
			XmlPrintLog("Invalid type code = %d", PERR, typeCode);
			return E_XML_INVALID_PARAMETER;	
		}
		
		if (typeLen <= 0) {
			XmlPrintLog("Invalid type len = %d", PERR, typeLen);
			return E_XML_INVALID_LEN;			
		}
		
		if (typeLen > sizeOfDynamicTypes) {
			XmlPrintLog("Invalid type len = %d, sizeOfDynamicTypes = %d", PERR, typeLen, sizeOfDynamicTypes);
		}
		
		bTypeFound = FALSE;
		
		prevMsgAddress = *ppMessage;
		
		for (j = 0; j < nElementsInTable; j++) {

			pTable = &dynamicTable[j];

			if (typeCode == pTable->type) {

				bTypeFound = TRUE;
				
				pFormat = (genXmlFormat *) pTable->format;

				status = GenBinBuilder(
									(char** )ppMessage,
									&pFormat,
									(char**) xml,
									1,
									binMsgSize,
									pCurBinMsgSize);
				if (status) {
						
					XmlPrintLog("code type = %d, type len = %d, status = %d",
						 PERR,
						 typeCode, 
						 typeLen, 
						 status);		
						 
					return status;
				}
				
				*ppMessage = prevMsgAddress + typeLen;
				break;
			}
		}
		
		if (!bTypeFound) {
			
			XmlPrintLog("Dynamic Type %d not found, numOfDynamicTypes = %d, nElementsInTable = %d, ",
				 PERR,
				 typeCode,
				 numOfDynamicTypes,
				 nElementsInTable);
				 
			return E_XML_DYNAMIC_TYPE_NOT_FOUND;	
		}
	}

	return status;

}// BuildDynamicBinMessage

//---------------------------------------------------------------------------
//
//	Function name:	GenBinBuilder
//
//	Description:	Recursive function that build binary message using the XML message 
//					and known format with rules to convert XML to binary.
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GenBinBuilder(
	char			**ppMessage,
	genXmlFormat	**ppFormat, 
	char			**xml,
	int				nElements,
	int				binMsgSize,
	int				*pCurBinMsgSize)
{
	char 	*elementName 	= NULL;
	char 	*pTemp 			= NULL;
	
	int 	i;
	int 	j;
	int 	status;
	int 	nFormatElements;

	int 	xmlType;
	int 	parentVarType;
	int 	varType;
	int		strLength;

	int		dataLen;
	int 	valSizeInBytes;

#ifdef __ValueValidation__
	unsigned long minVal;
	unsigned long maxVal;
	unsigned long rcvValue;
#endif

	for (i = 0; i < nElements; i++) {
		
		pTemp = NULL;
		xmlType = (*ppFormat)->xmlType;

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

#ifdef __BinXmlTrace__
			if (status = BinBuilderPrintXmlMessage(*xml)) {
				XmlPrintLog("rootElemType start: BinBuilderPrintXmlMessage fail, status = %d", PERR, status);
			}
#endif		
			// find next element.
			pTemp = strstr(*xml, "\n");
			
			if (!pTemp) {
				XmlPrintLog("rootElemType: Can't find end of line", PERR);
				return E_XML_INVALID_PARAMETER;
			}
			*xml = pTemp;
			*xml += 1;

			nFormatElements = (*ppFormat)->nElements;
			*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
			
			status = GenBinBuilder(
							ppMessage,
							ppFormat,
							xml,
							nFormatElements,
							binMsgSize,
							pCurBinMsgSize);
			if (status) {
					
				XmlPrintLog("root Type, status = %d", PERR, status);
				return status;		
			}


#ifdef __BinXmlTrace__
			if (status = BinBuilderPrintXmlMessage(*xml)) {
				XmlPrintLog("rootElemType end: BinBuilderPrintXmlMessage fail, status = %d", PERR, status);
			}
#endif		
			break;

		case parentElemType:
		
			elementName = (*ppFormat)->name;
			parentVarType = (*ppFormat)->varType;
			
			// Find next element, only if the current element has name.
			// If it hasn't and we continue to parse the xml message, we'll skip
			// on necessary fileds and lose information.
			if (elementName) {
				
#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("parentElemType start: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif		

#ifdef __ElementNameValidation__
				if (status = ElementNameValidation(
					elementName,
					*xml)) {

					XmlPrintLog("Parent name validation fail, elementName = <%s>, status = %d",
						PERR,
						elementName,
						status);
						
					return status;
				}
#endif
				pTemp = strstr(*xml, "\n");
				if (!pTemp) {
					XmlPrintLog("parentElemType start: Can't find end of line, elementName = %s", PERR, elementName);
					return E_XML_INVALID_PARAMETER;
				}
				*xml = pTemp;
				*xml += 1;			
			}

			nFormatElements = (*ppFormat)->nElements;
			
			// if the child is structure type, we get its pointer from the val field.
			// To avoid from changing the original format, we use temp pointer.
			if (parentVarType == structVarType) {

				int k;
				
				for (k = 0; k < nFormatElements; k++) {
	
					genXmlFormat *tempFormatPtr = (genXmlFormat*)((*ppFormat)->val);
				
					status = GenBinBuilder(
											ppMessage,
											(genXmlFormat**) &tempFormatPtr,
											xml,
											1,//nFormatElements,
											binMsgSize,
											pCurBinMsgSize);
					if (status) {
							
						XmlPrintLog("parent & struct type: Field name: <%s>, status = %d",
	                        PERR,
							elementName,
							status);
							
						return status;		
					}
				}
			
				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);

			} else if (parentVarType == dynamicVarType) {
			
				status = BuildDynamicBinMessage(
									ppMessage,
									(dynamicTableStruct *) ((*ppFormat)->val),
									xml,
									nFormatElements,
									binMsgSize,
									pCurBinMsgSize);
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
									nFormatElements,
									binMsgSize,
									pCurBinMsgSize);
				if (status) {

					XmlPrintLog("Parent & dynamic data type: Field name: <%s>, status = %d",
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
									binMsgSize,
									pCurBinMsgSize,
									(*ppFormat)->minVal,
									(*ppFormat)->maxVal);
				if (status) {

					XmlPrintLog("Parent & union type: Field name: <%s>, status = %d",
						PERR,
                        elementName,
                        status);
                        
					return status;
				}

				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
				
			} else {
				
				*ppFormat = (genXmlFormat *)((char *)*ppFormat + XmlFormatSize);
			
				status = GenBinBuilder(
									ppMessage,
									ppFormat,
									xml,
									nFormatElements,
									binMsgSize,
									pCurBinMsgSize);
				if (status) {
						
					XmlPrintLog("Parent & child type: Field name: <%s>, status = %d",
						PERR,
                        elementName,
                        status);
					
					return status;		
				}
			}
								
			// Find next element, only if the current element has name.
			// If it hasn't and we continue to parse the xml message, we'll skip
			// on necessary fileds and lose information.
			if (elementName) {

#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("parentElemType end: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif
				pTemp = strstr(*xml, "\n");
				if (!pTemp) {
					XmlPrintLog("parentElemType end: Can't find end of line, elementName = %s", PERR, elementName);
					return E_XML_INVALID_PARAMETER;
				}
				*xml = pTemp;
				*xml += 1;
			}

			break;

		case childElemType:
		case siblingElemType:
			
			elementName  = (*ppFormat)->name;
			varType		= (*ppFormat)->varType;
			
#ifdef __ElementNameValidation__
			if (status = ElementNameValidation(
				elementName,
				*xml)) {

				XmlPrintLog("Child name validation fail, elementName = <%s>, status = %d", PERR, elementName, status);
				return status;
			}
#endif

			switch (varType) {

			case charVarType:

				valSizeInBytes = sizeof(char);
				
				if ((*pCurBinMsgSize + valSizeInBytes) > binMsgSize) {
					
					XmlPrintLog("charVarType: Bin buffer too short, name = <%s>, current size = %d, binMsgSize=%d",
						PERR,
						elementName,
						*pCurBinMsgSize,
						binMsgSize);
						
					return (E_XML_BIN_BUFFER_TOO_SHORT + 1);	
				}
				
				*(unsigned char *) *ppMessage = (unsigned char) GetXmlVal(*xml, elementName); 

#ifdef __ValueValidation__
				rcvValue = *(unsigned char *) *ppMessage;
#endif 

#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("charVarType: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif
				
				*ppMessage += valSizeInBytes;
				*pCurBinMsgSize += valSizeInBytes;

				break;

			case stringVarType:

				strLength = (*ppFormat)->val;
				
				if ((*pCurBinMsgSize + strLength) > binMsgSize) {
					
					XmlPrintLog("stringVarType: Bin buffer too short, name = <%s>, current size = %d, binMsgSize=%d, strLen = %d",
						PERR,
						elementName,
						*pCurBinMsgSize,
						binMsgSize,
						strLength);
						
					return (E_XML_BIN_BUFFER_TOO_SHORT + 2);	
				}                
                
                status = GetXmlString(*xml, elementName, *ppMessage);

                if (status) {
					
					XmlPrintLog("stringVarType: Error, name = <%s> ,len = %d, status = %d",
						PERR,
						elementName,
						strLength,
						status);
					
                    return status;
                }

#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("stringVarType: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif

                *ppMessage += strLength;
                *pCurBinMsgSize += strLength;

                break;
                
            case charArrayVarType:
			
				dataLen = (*ppFormat)->val;
				
				if ((*pCurBinMsgSize + dataLen) > binMsgSize) {
					
					XmlPrintLog("charArrayVarType: Bin buffer too short, name = <%s>, current size = %d, dataLen = %d",
						PERR,
						elementName,
						*pCurBinMsgSize,
						dataLen);
						
					return (E_XML_BIN_BUFFER_TOO_SHORT + 3);	
				}    
				            			
				status = GetXmlDynamicPart(*xml, elementName, *ppMessage, dataLen);

				if (status) {

					XmlPrintLog("charArrayVarType: Name not found, name = <%s> , len = %d, status = %d",
						 PERR,
						 elementName,
						 dataLen,
						 status);
						 
					return status;
				}

#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("stringVarType: BinBuilderPrintXmlMessage fail, name = %s, data len = %d, status = %d", PERR, elementName, dataLen, status);
				}
#endif
				pTemp = strstr(*xml, ">");// skip on first element name
				if (!pTemp) {
					XmlPrintLog("charArrayVarType: Can't find end of tag, elementName = %s", PERR, elementName);
					return E_XML_INVALID_PARAMETER;
				}
				*xml = pTemp;
				*xml += 1;// skip on '>'
				*xml += ((dataLen * 3) + 1);// skip on data
				
				*ppMessage += dataLen;
				*pCurBinMsgSize += dataLen;

				break;

			case shortVarType:
			
				valSizeInBytes = sizeof(short);
				
				if ((*pCurBinMsgSize + valSizeInBytes) > binMsgSize) {
					
					XmlPrintLog("shortVarType: Bin buffer too short, name = <%s>, current size = %d",
						PERR,
						elementName,
						*pCurBinMsgSize);
						
					return (E_XML_BIN_BUFFER_TOO_SHORT + 4);	
				}
				    
				*(unsigned short *) *ppMessage = (unsigned short) GetXmlVal(*xml, elementName);

#ifdef __ValueValidation__
				rcvValue = *(unsigned short *) *ppMessage;
#endif 
#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("shortVarType: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif
			
				*ppMessage += valSizeInBytes;
				*pCurBinMsgSize += valSizeInBytes;;

				break;
			
			case shortListVarType:
			
				valSizeInBytes = sizeof(short);
				
				nFormatElements = (*ppFormat)->nElements;

				if (nFormatElements) {

#ifdef __BinXmlTrace__
					if (status = BinBuilderPrintXmlMessage(*xml)) {
						XmlPrintLog("shortListVarType start: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
					}
#endif
					// skip on the "list" tag
					pTemp = strstr(*xml, "list");// skip on first element name
					if (!pTemp) {
						XmlPrintLog("shortListVarType: Can't find list, elementName = %s", PERR, elementName);
						return E_XML_INVALID_PARAMETER;
					}
					*xml = pTemp;
					
					pTemp = strstr(*xml, "\n");
					if (!pTemp) {
						XmlPrintLog("shortListVarType start: Can't find end of line, elementName = %s", PERR, elementName);
						return E_XML_INVALID_PARAMETER;
					}
					*xml = pTemp;
					*xml += 1;

					for (j = 0; j < nFormatElements; j++) {
						
						if ((*pCurBinMsgSize + valSizeInBytes) > binMsgSize) {
							
							XmlPrintLog("shortListVarType: Bin buffer too short, name = <%s>, nFormatElements = %d, current size = %d",
								PERR,
								elementName,
								nFormatElements,
								*pCurBinMsgSize);
								
							return (E_XML_BIN_BUFFER_TOO_SHORT + 5);	
						}
				
						*(unsigned short *) *ppMessage = (unsigned short) GetXmlVal(*xml, elementName);
						
#ifdef __BinXmlTrace__
						if (status = BinBuilderPrintXmlMessage(*xml)) {
							XmlPrintLog("shortListVarType loop: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
						}
#endif
						*ppMessage += valSizeInBytes;
						*pCurBinMsgSize += valSizeInBytes;

						pTemp = strstr(*xml, "\n");
						if (!pTemp) {
							XmlPrintLog("shortListVarType end: Can't find end of line, elementName = %s", PERR, elementName);
							return E_XML_INVALID_PARAMETER;
						}
						*xml = pTemp;
						*xml += 1;
					}
				
				} else {

					XmlPrintLog("Empty list of %s", PWRN, elementName);
				}

#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("shortListVarType end: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif
				break;

			case longVarType:
	
				valSizeInBytes = sizeof(long);
				
				if ((*pCurBinMsgSize + valSizeInBytes) > binMsgSize) {
					
					XmlPrintLog("longVarType: Bin buffer too short, name = <%s>, current size = %d",
						PERR,
						elementName,
						*pCurBinMsgSize);
						
					return (E_XML_BIN_BUFFER_TOO_SHORT + 6);	
				}
				
				*(unsigned long *) *ppMessage = (unsigned long) GetXmlVal(*xml, elementName);

#ifdef __ValueValidation__
				rcvValue = *(unsigned long *) *ppMessage;
#endif 
#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("longVarType: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif
				
				*ppMessage += valSizeInBytes;
				*pCurBinMsgSize += valSizeInBytes;
			
				break;

			case longListVarType:

				valSizeInBytes = sizeof(long);
				
				nFormatElements = (*ppFormat)->nElements;

				if (nFormatElements) {

#ifdef __BinXmlTrace__
					if (status = BinBuilderPrintXmlMessage(*xml)) {
						XmlPrintLog("longListVarType start: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
					}
#endif
					// skip on the "list" tag
					pTemp = strstr(*xml, "list");// skip on first element name
					if (!pTemp) {
						XmlPrintLog("longListVarType: Can't find list, elementName = %s", PERR, elementName);
						return E_XML_INVALID_PARAMETER;
					}
					*xml = pTemp;
					
					pTemp = strstr(*xml, "\n");
					if (!pTemp) {
						XmlPrintLog("longListVarType start: Can't find end of line, elementName = %s", PERR, elementName);
						return E_XML_INVALID_PARAMETER;
					}
					*xml = pTemp;
					*xml += 1;

					for (j = 0; j < nFormatElements; j++) {
	
						if ((*pCurBinMsgSize + valSizeInBytes) > binMsgSize) {
							
							XmlPrintLog("longListVarType: Bin buffer too short, name = <%s>, nFormatElements = %d, current size = %d",
								PERR,
								elementName,
								nFormatElements,
								*pCurBinMsgSize);
								
							return (E_XML_BIN_BUFFER_TOO_SHORT + 7);	
						}

						*(unsigned long *) *ppMessage = (unsigned long) GetXmlVal(*xml, elementName);

#ifdef __BinXmlTrace__
						if (status = BinBuilderPrintXmlMessage(*xml)) {
							XmlPrintLog("longListVarType loop: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
						}
#endif
						
						*ppMessage += valSizeInBytes;
						*pCurBinMsgSize += valSizeInBytes;

						pTemp = strstr(*xml, "\n");
						if (!pTemp) {
							XmlPrintLog("longListVarType end: Can't find end of line, elementName = %s", PERR, elementName);
							return E_XML_INVALID_PARAMETER;
						}
						*xml = pTemp;
						*xml += 1;
					}
				
				} else {
					
					XmlPrintLog("Empty list of %s", PWRN, elementName);
				}

#ifdef __BinXmlTrace__
				if (status = BinBuilderPrintXmlMessage(*xml)) {
					XmlPrintLog("longListVarType end: BinBuilderPrintXmlMessage fail, name = %s, status = %d", PERR, elementName, status);
				}
#endif			
				break;

			default:

				printf("error type = %d", varType);
				break;

			}
		
			// find next child
			pTemp = strstr(*xml, "\n");
			if (!pTemp) {
				XmlPrintLog("find next child: Can't find end of line, elementName = %s", PERR, elementName);
				return E_XML_INVALID_PARAMETER;
			}
			*xml = pTemp;
			*xml += 1;
			
#ifdef __ValueValidation__			
			minVal = (*ppFormat)->minVal;
			maxVal = (*ppFormat)->maxVal;

			if ((rcvValue < minVal) ||
				(rcvValue > maxVal)) {
			
				XmlPrintLog("\nField Name = <%s>, val = %d, min = %d, max = %d", PWRN, elementName, rcvValue, minVal, maxVal);
			}
#endif // __ValueValidation__

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
		
	return 0 ;

}// GenBinBuilder
