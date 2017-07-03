// XmlEngineApi.h
// Ami Noy
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __XML_ENGINE_API_H__
#define __XML_ENGINE_API_H__



//---------------------------------------------------------------------------
//
//	Function name:	InitXmlEngine
//
//	Description:	Initiliaze xml engine
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int InitXmlEngine();

//---------------------------------------------------------------------------
//
//	Function name:	GetXmlMessageLen
//
//	Description:	Get the request xml message length to enable buffer allocation
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetXmlMessageLen(
	char	*pBinMessage,		// IN
	int		*pXmlMessageLen);	// OUT

//---------------------------------------------------------------------------
//
//	Function name:	GetXmlMessage
//
//	Description:	Get the xml message from the binary message
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetXmlMessage(
	int		xmlMessageLen,	// IN
	char	*pXmlMessage);	// OUT


//---------------------------------------------------------------------------
//
//	Function name:	GetBinMessageSize
//
//	Description:	Get the request binary message size to enable buffer allocation
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetBinMessageSize(
	char	*pXmlMessage,		// IN
	int		*pBinMessageSize,	// OUT
	int		*pOpcode);			// OUT

//---------------------------------------------------------------------------
//
//	Function name:	GetBinMessage
//
//	Description:	Get the binary message from the xml message
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetBinMessage(
	char	*pXmlMessage,		// IN
	int		rcvXmlMessageSize,	// IN
	char	*pBinMessage);		// OUT


#endif // __XML_ENGINE_API_H__

//---------------------------------------------------------------------------
//
//	Function name:	InitXmlTraceLevel
//
//	Description:	For MCMS use - get trace level from ststem.cfg, TRUE = full, FALSE = errors and warnings
//					
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int InitXmlTraceLevel(BOOL bTraceLevel);

#ifdef __cplusplus
}
#endif
