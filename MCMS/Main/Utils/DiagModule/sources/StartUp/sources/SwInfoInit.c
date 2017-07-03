#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "DiagDataTypes.h"
#include "SwInfoInit.h"
#include "MplStartup.h"
#include "Print.h"



extern INT32 ulVrsnTxtMissing;

extern INT32 FpgaLoadInit (UINT8 *pucFpgaPiFileName , UINT8 *pucFpgaSwitchFileName);

extern void InitHardware();

int my_strlen(char *pSource)
{
	INT8 c;
	int i=0;
				
	c = *pSource;
	//look for beginning of file-name:		
	while( (c != '=') && (c != '\n') && (c != '\r') && (c != ';') && (c!='\0') )
			c = *(pSource++);

	if(c == '=')
	{
		c = *pSource;		
		while( (c != '\n') && (c != '\r') && (c != ';') && (c!='\0') ) 
		{						
			pSource++;					
			c = *pSource;
			i++;
		}
	}
	 
	return (i);	
}

int my_strncpy(char *pDest, char *pSource, int size)
{
	INT8 c;
	int i=0;
			
	if(pDest == 0)
		return(0);
	
	c = *pSource;
	//look for beginning of file-name:		
	while( (c != '=') && (c != '\n') && (c != '\r') && (c != ';') && (c!='\0') )
			c = *(pSource++);

	if(c == '=')
	{
		c = *pSource;		
		while( (c != '\n') && (c != '\r') && (c != ';') && (c!='\0') && (i<size)) 
		{			
			*pDest = c;
			pSource++;		
			pDest++;
			c = *pSource;
			i++;
		}
	}
	 
	*pDest = '\0';
	return (i);
}

INT8 my_strccpy(char *pDest, char *pSource)
{
	INT8 c , i=0;
	
	MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"my_strccpy");
		
	if(pDest == 0)
		return(0);
	
	c = *pSource;
	//look for beginning of file-name:		
	while( (c != '=') && (c != '\n') && (c != '\r') && (c != ';') && (c!='\0') )
			c = *(pSource++);

	if(c == '=')
	{
		c = *pSource;		
		while( (c != '\n') && (c != '\r') && (c != ';') && (c!='\0') ) 
		{			
			*pDest = c;
			pSource++;		
			pDest++;
			c = *pSource;
			i++;
		}
	}
	 
	*pDest = '\0';
	return (i);
}

void my_changeColonToDot(char *pSource)
{
	MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"my_chageColonToDot");
	
	if(pSource == 0) return;

	while('\0' != *pSource)
	{
		if( ':' == *pSource) *pSource = '.';
		pSource++;
	}
}

void GetVersionNum(char *pDest, char *pSource)
{
	int i;
	char *pVerNum;


	*pDest = '\0';
	pVerNum = 0;

	i = 0;
	while(pSource[i] != '\0')
	{
		if(pSource[i] == '.')
		{
				pVerNum = &(pSource[i+1]);
				break;
		}
		i++;
	}

	if(pVerNum != 0)
	{
		i=0;
		while(pVerNum[i] != '\0' && (i<15) )
		{
			pDest[i] = pVerNum[i];
			i++;
		}
		pDest[i] = '\0';
	}

	return;
}

 
