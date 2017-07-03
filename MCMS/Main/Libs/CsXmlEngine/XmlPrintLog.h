// XmlPrintLog.h
// Ami Noy

#ifndef _XMLPRINTLOG_H_
#define _XMLPRINTLOG_H_

// Macros:
// -------
//#define __XmlTrace__

typedef enum {
	exmlNoPrint = 0,
    exmlErr = 1,
    exmlWrn = 2,
    exmlLog = 3,
} emPrintKeyLevel;

typedef enum {
	emXmlTraceNoTrace 	= 0,
	emXmlTraceErrors	= 1,
	emXmlTraceErrWrn	= 2,
	emXmlTraceMessages	= 3,
	emXmlTraceFull		= 4,	
} emXmlTraceLevel;

#define PERR                exmlErr
#define PWRN                exmlWrn
#define PLOG                exmlLog

int XmlPrintLogInit();

char *SetMemoryToAsciiMessage(
	unsigned long	binAddress,
	int				size);
	
char* XmlPrintBytes(
	char 	*binToPrint,
	int		varType,
	int 	numOfBytes);
	
int XmlPrintLog(
    char *printFormat,  /* IN : printf format parameters */
    int  keyLevel,     /* IN : Unique printing source key */
    ...);


#endif //_XMLPRINTLOG_H_
