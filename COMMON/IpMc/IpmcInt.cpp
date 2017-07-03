/*===================================================================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                                      */
/*-------------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                              */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                    */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                          */
/* prior written authorization from Polycom Israel Ltd.                                                              */
/*-------------------------------------------------------------------------------------------------------------------*/
/* FILE:     IpmcInt.c																						 */
/* PROJECT:  Gideon CM                                                                                               */
/* PROGRAMMER:  Raviv Haim                                              .                                            */
/*                                                                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
/*              |                     |                                                                              */
/*===================================================================================================================*/



#include "DataTypes.h"
#include "serial.h"
#include "IpmcInt.h"
#include <Trace.h>


#ifndef MFA_BOARD_PRINT
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#endif


#ifdef MFA_BOARD_PRINT
#include "Print.h"
#endif


UINT32 unRedLedId;
UINT32 unGreenLedId;
UINT32 unAmberLedId;


INT8 cRedNum;
INT8 cGreenNum;
INT8 cAmberNum;


char caHwSlotIdResponseHeader[7]   = {0xbc,0x0,0x5,0x0,0x0,0x40,0xa};
char caEepromReadResponseHeader[4] = {0x2c,0x0,0x11,0x0};

char caGetFruLedPropertiesHeader[5]     = {0xb4,0x00,0x05,0x00,0x00};
char caGetLedColorCapabilitiesHeader[5] = {0xb4,0x00,0x06,0x00,0x00};
char caGetIPMCVersion[4]                = {0x4C,0x00,0xF2,0x56};
char caSetWatchDogHeader[4]             = {0x1c,0x00,0x24,0x00};
char caResetWatchDogHeader[4]           = {0x1c,0x00,0x22,0x00};

int debug_rx_chars = 1 ;
INT32 recv_hex_response(unsigned char *data, int *length , int nFd);

INT32 GetLedColorCapabilitiesFunc(INT8 cNumOfLeds , int nFd);
void FindLedColor(UINT32 unPlaceInBoard , INT8 cLedColor , INT8 eColorNum);

const INT32 NOT_INITIALIZED = 0xA5A5A5A5;
static INT32 snSerialConnectionFD = NOT_INITIALIZED;

//[Dotan.H 24/05/2010 open_ipmc_interface wrapper to open only once]
INT32 open_ipmc_interface_wrap()
{
    if (NOT_INITIALIZED == snSerialConnectionFD)
    {
        snSerialConnectionFD = open_ipmc_interface();
    }

    return snSerialConnectionFD;
}

/* open serial device */
INT32 open_ipmc_interface()
{
	INT32 fd , rc = 0;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

#ifdef MFA_BOARD_PRINT
	fd = serial_open("/dev/tts/1");
#else
    fd = serial_open("/dev/ttyS0");
#endif
	if (fd < 0)
	{
		printf("can not open serial interface to IPMC \n");
		return eIpmcSerialOpenFail;
	}

	rc = serial_updateIRQ(SERIAL_IRQ,fd);
	if(!rc)
	{
		printf(" Serial interface update IRQ Failed \n") ;
		rc = -1;

		eIpmcStatus = eIpmcSerialUpdateIRQFail;
		goto open_ipmc_interface_end;
	}

	rc = serial_set_param(IPMC_SERIAL_RATE,fd) ;
	if(rc == -1)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"serial_set_param failed");
#endif
		eIpmcStatus = eIpmcSerialSetParamFail;
		goto open_ipmc_interface_end;
	}

  open_ipmc_interface_end:

	if (rc == -1)
	{
		close_ipmc_interface(fd);
		return (INT32)eIpmcStatus;
	}
	else
	{
        FPTRACE(eLevelInfoNormal,"snSerialConnectionFD opened");
		return fd;
	}
}





INT32 write_ipmc_interface(char *str , int nFd)
{
	return serial_write_line(str,nFd) ;
}

void close_ipmc_interface(int nFd)
{
	serial_close(nFd) ;
    snSerialConnectionFD = NOT_INITIALIZED;
    FPTRACE(eLevelInfoNormal,"snSerialConnectionFD closed");
}

/*
  Receive IPMI response from the device
  Length: in - buffer size, out - response length
  Returns: error code, 0 = success
*/
INT32 recv_hex_response(unsigned char *data, int *length , int nFd)
{
    INT8 hex_rs[MAX_MSG * 2 + 16];
    INT32 x, i, j, l;
    INT8 ch, *p, *pp;
    INT8 str_hex[8];

    if (!length) {
        return -1;
    }
    l = *length;
    *length = 0;

    for (i = 0; i < LINES_LIMIT; i++) {
        x = serial_read_line(hex_rs , (sizeof(hex_rs) - 1) , nFd);
        hex_rs[sizeof(hex_rs) - 1] = 0;
        if (x == 0) {
            /* timeout */
            return ERR_TIMEOUT;
        } else if (x < 0) {
            /* error */
            return x;
        }
        if (debug_rx_chars) {
#ifdef MFA_BOARD_PRINT
            MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "%4d> %s\n", x, hex_rs);
#endif
        }
        if (x > 0) {
            if (strchr(hex_rs, '[') && strchr(hex_rs, ']')) {
                break;
            }
        }
    }

    /* find the left bracket */
    p = strchr(hex_rs, '[');
    if (!p) {
        return ERR_INVALID_RESPONSE;
    }

    /* find the right bracket */
    p++;
    pp = strchr(p, ']');
    if (!pp) {
        return ERR_INVALID_RESPONSE;
    }
    *pp = 0;

    /* was it an error? */
    if (strncmp(p, "ERR ", 4) == 0) {
        serial_write_line("\r\r\r\r",nFd);
        SLEEP(1);
        serial_flush(nFd);
        x = strtol(p + 4, &p, 16);
        if (x < 0x100 && *p == '\0') {
    	    return x | ERR_SYNTAX;
        } else {
            return ERR_INVALID_RESPONSE;
        }
    }

    /* parse the response */
    i = 0;
    j = 0;
    while (*p) {
        if (i >= l) {
            return i;
        }
        x = 0;
        ch = *(p++);
        if (!ch) {
            if (j > 0) {
                return ERR_INVALID_RESPONSE;
            }
            break;
        }
        if (isspace(ch)) {
            if (j > 1) {
                x = 1;
            } else if (j > 0) {
                return ERR_INVALID_RESPONSE;
            } else {
                continue;
            }
        } else { /* not a space */
            str_hex[j++] = ch;
            if (j >= 2) {
                x = 1;
            }
        }
        if (x) {
            /* parse the hex number */
            str_hex[j] = 0;
            if (j > 0) {
                data[i++] = (unsigned char) strtol(str_hex, &pp, 16);
                if (*pp != 0) {
                    break;
                }
            }
            j = 0;
        }
    }

    *length = i;
    return 0;
}

UINT32 IpmcSendCommand(char *strCmd,char *pResponse)
{
	//UINT8 cResponse[2024];
	INT32 lcount =  2024 , rc = 0;
	INT32 i , nFd;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;
	INT32 status=0;

	if(strCmd==NULL)
	{
		status=10;
		return (INT32)eIpmcOpenInterfaceFail;
	}

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		status=11;
		return (INT32)eIpmcOpenInterfaceFail;
	}



	rc = write_ipmc_interface(strCmd,nFd) ;

	lcount=2024-1;
	pResponse[0]='\0';
	rc=serial_read_ex((char *)pResponse,lcount,nFd);
	if(rc>=0&&rc<2024)
	{
		pResponse[rc]='\0';
		status=13;
	}


  error:

      if (eIpmcStatus == eIpmcWriteIpmcFail)
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
      }

	return status;
}


UINT32 IpmcGetHWSlotId()
{
	UINT8 cResponse[100];
	INT32 lcount =  99 , rc = 0;
	INT32 i , nFd;
	UINT32 slotId = 0x77;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;
	INT8 buf[200];


	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return slotId;
	}

	rc = write_ipmc_interface(GetHWSlotIdReq,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmcGetHWSlotId : write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto IpmcGetHWSlotId_end;
	}

	rc = recv_hex_response(cResponse,&lcount,nFd) ;
	if(rc == 0)
	{
		if(memcmp(caHwSlotIdResponseHeader,cResponse,sizeof(caHwSlotIdResponseHeader) ) == 0)
			slotId = cResponse[sizeof(caHwSlotIdResponseHeader) ] ;
		else
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Invalied response for slot id");
#endif
			eIpmcStatus = eIpmcGetHWSlotIdFail;
			goto IpmcGetHWSlotId_end;
		}
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Slot Id = 0x%x",slotId);
#endif
	}
	else
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"recv_hex_response return 0x%x",rc);
#endif
		eIpmcStatus = eIpmcRcvHexResponseFail;
		goto IpmcGetHWSlotId_end;
	}


  IpmcGetHWSlotId_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return slotId;
}



void IpmcBuildEepromMsg(INT8 *pcBuf , UINT16 usOffset , UINT8 ucCount , UINT32 unEepromType , UINT32 unBuildFlag , INT8 *pcData)
{
	INT8 uchighOffsetByte ,uclowOffsetByte;


	uchighOffsetByte = (char) (usOffset >> 8) ;
	uclowOffsetByte = (char) usOffset ;

	if (unEepromType == BOARD_EEPROM)
	{
		if (unBuildFlag == READ_EEPROM)
		{
			sprintf(pcBuf,"%s %02x %02x %02x]\r",ReadBoardEepromStartHeaderReq,uclowOffsetByte,uchighOffsetByte,ucCount) ;
		}
	}
	else
	{
		if (unBuildFlag == READ_EEPROM)
		{
			sprintf(pcBuf,"%s %02x %02x %02x]\r",ReadBpEepromStartHeaderReq,uclowOffsetByte,uchighOffsetByte,ucCount) ;
		}
		else
		{
			sprintf(pcBuf,"%s %02x %02x %s]\r",WriteBpEepromStartHeaderReq,uclowOffsetByte,uchighOffsetByte,pcData);
		}
	}
}



INT32 IpmcReadEeprom(INT8 *pcBuf , UINT16 usOffset , UINT8 ucCount , UINT32 unEepromType)
{
	INT8 cReqBuf[100];
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	UINT8 ucByteCount;
	UINT32 ulNumOfReq,ulReqIndex;
	UINT32 ulNumOfBytesLeft;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	if(ucCount <= MAX_EEPROM_BYTES_PER_READ)
	{
		IpmcBuildEepromMsg(cReqBuf,usOffset,ucCount,unEepromType,READ_EEPROM,pcBuf) ;

		rc = write_ipmc_interface(cReqBuf,nFd) ;
		if(rc < 0)
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmcReadEeprom: write_ipmc_interface error %d",rc);
#endif
			eIpmcStatus = eIpmcWriteIpmcFail;
			goto IpmcReadEeprom_end;
		}

		rc = recv_hex_response(cResponse,&lcount,nFd) ;
		if(memcmp(caEepromReadResponseHeader,cResponse,sizeof(caEepromReadResponseHeader)) == 0)
		{
			ucByteCount = cResponse[sizeof(caEepromReadResponseHeader)] ;
			if(ucCount == ucByteCount)
			{
				memcpy(pcBuf,&cResponse[sizeof(caEepromReadResponseHeader) + 1],ucByteCount) ;
			}
			else
			{
#ifdef MFA_BOARD_PRINT
				MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(IpmcReadEeprom) : ucCount = %d , ucByteCount = %d",ucCount,ucByteCount);
#endif
				eIpmcStatus = eIpmcReadEepromFail;
				goto IpmcReadEeprom_end;
			}
		}
		else
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(IpmcReadEeprom) memcmp1 : caEepromReadResponseHeader = %s,cResponse = %s",caEepromReadResponseHeader,cResponse);
#endif
			eIpmcStatus = eIpmcRcvHexResponseFail;
			goto IpmcReadEeprom_end;
		}
	}
	else
	{
		/* Calc How many req. needed */
		ulNumOfReq = (ucCount / MAX_EEPROM_BYTES_PER_READ) + 1 ;
		for(ulReqIndex = 0 ;ulReqIndex < ulNumOfReq ; ulReqIndex++)
		{
			if(ulReqIndex != (ulNumOfReq - 1))
			{
				ulNumOfBytesLeft = MAX_EEPROM_BYTES_PER_READ ;
			    IpmcBuildEepromMsg(cReqBuf,(usOffset + (ulReqIndex * MAX_EEPROM_BYTES_PER_READ)),MAX_EEPROM_BYTES_PER_READ,unEepromType,READ_EEPROM,0) ;
			}
			else
			{
				ulNumOfBytesLeft = (ucCount % MAX_EEPROM_BYTES_PER_READ) ;
   			    IpmcBuildEepromMsg(cReqBuf,(usOffset + (ulReqIndex * MAX_EEPROM_BYTES_PER_READ)),ulNumOfBytesLeft,unEepromType,READ_EEPROM,0) ;
			}

			rc = write_ipmc_interface(cReqBuf,nFd) ;
			if(rc < 0)
			{
#ifdef MFA_BOARD_PRINT
				MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmcReadEeprom: write_ipmc_interface error %d \n",rc);
#endif
				eIpmcStatus = eIpmcWriteIpmcFail;
				goto IpmcReadEeprom_end;
			}

			rc = recv_hex_response(cResponse,&lcount,nFd) ;
			if(memcmp(caEepromReadResponseHeader,cResponse,sizeof(caEepromReadResponseHeader)) == 0)
			{
				ucByteCount = cResponse[sizeof(caEepromReadResponseHeader)] ;
				if(ulNumOfBytesLeft == ucByteCount)
				{
					memcpy(&pcBuf[(ulReqIndex * MAX_EEPROM_BYTES_PER_READ)] ,&cResponse[sizeof(caEepromReadResponseHeader) + 1],ucByteCount) ;
				}
				else
				{
#ifdef MFA_BOARD_PRINT
					MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(IpmcReadEeprom) : ulNumOfBytesLeft = %d , ucByteCount = %d",ulNumOfBytesLeft,ucByteCount);
#endif
					eIpmcStatus = eIpmcReadEepromFail;
					goto IpmcReadEeprom_end;
				}
			}
			else
			{
#ifdef MFA_BOARD_PRINT
				MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(IpmcReadEeprom) memcmp2 : caEepromReadResponseHeader = %s,cResponse = %s",caEepromReadResponseHeader,cResponse);
#endif
				eIpmcStatus = eIpmcRcvHexResponseFail;
				goto IpmcReadEeprom_end;
			}
		}
	}


  IpmcReadEeprom_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;
}






INT32 IpmcWriteEeprom(INT8 *pcBuf,UINT16 usOffset,UINT8 ucCount,UINT32 unEepromType)
{
	INT8 cReqBuf[100] , cTempBuf[22];
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	UINT8 ucByteCount;
	UINT32 ulNumOfReq,ulReqIndex;
	UINT32 ulNumOfBytesLeft;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap() ;
	if (nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	if (ucCount <= MAX_EEPROM_BYTES_PER_WRITE)
	{
		IpmcBuildEepromMsg(cReqBuf,usOffset,ucCount,unEepromType,WRITE_EEPROM,pcBuf);

		rc = write_ipmc_interface(cReqBuf,nFd) ;
		if(rc < 0)
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmcWriteEeprom : write_ipmc_interface error %d \n",rc);
#endif
			eIpmcStatus = eIpmcWriteIpmcFail;
			goto IpmcWriteEeprom_end;
		}

		rc = recv_hex_response(cResponse,&lcount,nFd) ;
	}
	else
	{
		/* Calc How many req. needed */
		ulNumOfReq = (ucCount / MAX_EEPROM_BYTES_PER_WRITE) + 1 ;
		for(ulReqIndex = 0 ;ulReqIndex < ulNumOfReq ; ulReqIndex++)
		{
			memset((void*)&cTempBuf[0],0,22);

			if(ulReqIndex != (ulNumOfReq - 1))
			{
				ulNumOfBytesLeft = MAX_EEPROM_BYTES_PER_WRITE ;
				memcpy((void*)&cTempBuf[0],pcBuf,ulNumOfBytesLeft);
				pcBuf += ulNumOfBytesLeft;
				IpmcBuildEepromMsg(cReqBuf,(usOffset + (ulReqIndex * MAX_EEPROM_BYTES_PER_READ)) ,MAX_EEPROM_BYTES_PER_WRITE,unEepromType,WRITE_EEPROM,cTempBuf) ;
			}
			else
			{
				ulNumOfBytesLeft = (ucCount % MAX_EEPROM_BYTES_PER_READ) ;
				memcpy((void*)&cTempBuf[0],pcBuf,ulNumOfBytesLeft);
				IpmcBuildEepromMsg(cReqBuf,(usOffset + (ulReqIndex * MAX_EEPROM_BYTES_PER_READ)),ulNumOfBytesLeft,unEepromType,WRITE_EEPROM,cTempBuf) ;
			}

			rc = write_ipmc_interface(cReqBuf,nFd) ;
			if(rc < 0)
			{
#ifdef MFA_BOARD_PRINT
				MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmcWriteEeprom : write_ipmc_interface error %d \n",rc);
#endif
				eIpmcStatus = eIpmcWriteIpmcFail;
				goto IpmcWriteEeprom_end;
			}

			rc = recv_hex_response(cResponse,&lcount,nFd) ;
		}
	}


  IpmcWriteEeprom_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;
}



INT32 IpmcGetFruLedProperties()
{
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT32 i ;
	INT32 nFd , rc = 0;
	INT8 buf[200];
	INT8 cNumOfLeds;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(GetFruLedProperties,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmcGetFruLedProperties : write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto IpmcGetFruLedProperties_end;
	}

	rc = recv_hex_response(cResponse,&lcount,nFd) ;
	if(rc == 0)
	{
		if(memcmp(caGetFruLedPropertiesHeader,cResponse,sizeof(caGetFruLedPropertiesHeader) ) == 0)
		{
			//we got the right indication
			cNumOfLeds = cResponse[sizeof(caGetFruLedPropertiesHeader)] ;
			rc = GetLedColorCapabilitiesFunc(cNumOfLeds,nFd);
			if (rc != 0)
			{
#ifdef MFA_BOARD_PRINT
				MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetLedColorCapabilitiesFunc returned error %d",rc);
#endif
				eIpmcStatus = eIpmcGetFruLedPropertiesFail;
				goto IpmcGetFruLedProperties_end;
			}
		}
		else
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetFruLedProperties");
#endif
			eIpmcStatus = eIpmcGetFruLedPropertiesFail;
			goto IpmcGetFruLedProperties_end;
		}
	}
	else
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"recv_hex_response return 0x%x",rc);
#endif
		eIpmcStatus = eIpmcRcvHexResponseFail;
		goto IpmcGetFruLedProperties_end;
	}

  IpmcGetFruLedProperties_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;
}




INT32 GetLedColorCapabilitiesFunc(INT8 cNumOfLeds , int nFd)
{
	UINT32 i;
	INT8   cLedColor , cColorNum , cBitShift = 1;
	INT8 pcBuf[30];
	UINT8 cResponse[100];
	INT32 lcount =  99  , rc = 0;

	for (i = 0 ; i < 4 ; i++)
	{
		//get the led color capabilities
		sprintf(pcBuf,"%s %02x]\r",GetLedColorCapabilities,i/*cLedColor*/);
		rc = write_ipmc_interface(pcBuf,nFd);
		if(rc < 0)
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetLedColorCapabilitiesFunc: write_ipmc_interface error %d",rc);
#endif
			return (INT32)eIpmcWriteIpmcFail;
		}

		rc = recv_hex_response(cResponse,&lcount,nFd);
		if(rc == 0)
		{
			if(memcmp(caGetLedColorCapabilitiesHeader,cResponse,sizeof(caGetLedColorCapabilitiesHeader) ) == 0)
			{
				//we got the right indication
				cLedColor = cResponse[sizeof(caGetLedColorCapabilitiesHeader)];
				cColorNum = cResponse[sizeof(caGetLedColorCapabilitiesHeader) + 2];
				FindLedColor(i,cLedColor,cColorNum);

			}
			else
			{
#ifdef MFA_BOARD_PRINT
				MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetLedColorCapabilities");
#endif
				return (INT32)eIpmcGetLedColorCapabilitiesFuncFail;
			}
		}
		else
		{
#ifdef MFA_BOARD_PRINT
			MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"recv_hex_response return 0x%x",rc);
#endif
			return (INT32)eIpmcRcvHexResponseFail;
		}

	}

	return eIpmcSuccess;
}




void FindLedColor(UINT32 unPlaceInBoard , INT8 cLedColor , INT8 cColorNum)
{
    unRedLedId = 1;
	cRedNum = 2;
    unGreenLedId = 2;
	cGreenNum = 3;
    unAmberLedId = 3;
	cAmberNum = 4;
	if ( (cLedColor & LED_SUPPORT_RED) == LED_SUPPORT_RED)
	{
		//we have a red color
		unRedLedId = 1;
		cRedNum = 2;
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"red unRedLedId = %x , cRedNum = %x",unRedLedId,cRedNum);
#endif
	}

	if ( (cLedColor & LED_SUPPORT_GREEN) == LED_SUPPORT_GREEN)
	{
		//we have a green color
		unGreenLedId = 2;
		cGreenNum = 3;
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"green unGreenLedId = %x , cGreenNum = %x",unGreenLedId,cGreenNum);
#endif
	}

	if ( (cLedColor & LED_SUPPORT_AMBER) == LED_SUPPORT_AMBER)
	{
		//we have a amber color
		unAmberLedId = 3;
		cAmberNum = 4;
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"amber unAmberLedId = %x , cAmberNum = %x",unAmberLedId,cAmberNum);
#endif
	}
}




INT32 LedInterface(UINT32 unLedColor , UINT32 unStatus)
{
	INT32 nFd , rc = 0;
	INT8 pcBuf[30];
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT8 cLedNum , cLedColor;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	memset((void*)pcBuf, 0, sizeof(pcBuf));

	static UINT32 unRedLedStatus = TURN_OFF , unGreenLedStatus = TURN_OFF , unAmberLedStatus = TURN_OFF;

	switch (unLedColor)
	{
		case RED:
		{
			if ( ((unRedLedStatus == TURN_OFF) && (unStatus == TURN_OFF)) || ((unRedLedStatus == TURN_ON) && (unStatus == TURN_ON)) )
			{
				return 0;
			}

			unRedLedStatus = unStatus;

			cLedColor = cRedNum;
			cLedNum = unRedLedId;
			break;
		}

		case GREEN:
		{
			if ( ((unGreenLedStatus == TURN_OFF) && (unStatus == TURN_OFF)) || ((unGreenLedStatus == TURN_ON) && (unStatus == TURN_ON)) )
			{
				return 0;
			}
			unGreenLedStatus = unStatus;

			cLedColor = cGreenNum;
			cLedNum = unGreenLedId;
			break;
		}

		case AMBER:
		{
			if ( ((unAmberLedStatus == TURN_OFF) && (unStatus == TURN_OFF)) || ((unAmberLedStatus == TURN_ON) && (unStatus == TURN_ON)) )
			{
				return 0;
			}
			unAmberLedStatus = unStatus;

			cLedColor = cAmberNum;
			cLedNum = unAmberLedId;
			break;
		}
	}

	//get the status
	switch (unStatus)
	{
		case TURN_ON:
		{
			//turn on the leds
			snprintf(pcBuf, 30, "%s %02x %02x %02x %02x]\r",SetFruLedState,cLedNum,0xff,0,cLedColor) ;
			break;
		}

		case TURN_OFF:
		{
			//turn of the leds
			snprintf(pcBuf, 30, "%s %02x %02x %02x %02x]\r",SetFruLedState,cLedNum,0,0,cLedColor) ;
			break;
		}

		case BLINK_LED:
		{
			//turn of the leds
			snprintf(pcBuf, 30, "%s %02x %02x %02x %02x]\r",SetFruLedState,cLedNum,0x30,0x30,cLedColor) ;
			break;
		}
	}

	nFd = open_ipmc_interface_wrap() ;
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(pcBuf,nFd);
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"LedInterface: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto LedInterface_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);

  LedInterface_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

#ifdef MFA_BOARD_PRINT
	MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"led = %x , action = %x",unLedColor,unStatus);
#endif
	return (INT32)eIpmcStatus;
}





INT32 SetWatchDogFunc(UINT32 unWatchDogInterval)
{
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT8 cReqBuf[100];
	EIpmcStatus eIpmcStatus = eIpmcSuccess;
    UINT8 unIntervalLow;
    UINT8 unIntervalHigh;


	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	//build the function
    //Bracha: use high and low interval
    unIntervalLow = unWatchDogInterval;
    unIntervalHigh = (unWatchDogInterval&0xff00) >> 8;

    snprintf(cReqBuf, 100, "%s %02x %02x]\r", SetWatchDog, unIntervalLow, unIntervalHigh);

//    sprintf(cReqBuf,"%s %02x %02x]\r",SetWatchDog,unWatchDogInterval,0) ;

	rc = write_ipmc_interface(cReqBuf,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SetWatchDogFunc: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetWatchDogFunc_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);
    if (0 == rc)
    {
        if (memcmp(caSetWatchDogHeader, cResponse,sizeof(caSetWatchDogHeader)) == 0)
        {
                goto SetWatchDogFunc_end;
        }
        else
        {
            eIpmcStatus = eIpmcRcvHexResponseFail;
            goto SetWatchDogFunc_end;
        }
    }
    else
    {
        eIpmcStatus = eIpmcRcvHexResponseFail;
        goto SetWatchDogFunc_end;
    }

  SetWatchDogFunc_end:

    FPTRACE(eLevelInfoNormal,"::before close_ipmc_interface");
	if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
    {
        close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                    // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
    }
    FPTRACE(eLevelInfoNormal,"::after close_ipmc_interface");

	return (INT32)eIpmcStatus;
}



INT32 ResetWatchDogFunc()
{
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(ResetWatchDog,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ResetWatchDogFunc: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto ResetWatchDogFunc_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);
    if (0 == rc)
    {
        if (memcmp(caResetWatchDogHeader, cResponse,sizeof(caResetWatchDogHeader)) == 0)
        {
                goto ResetWatchDogFunc_end;
        }
        else
        {
            FPTRACE(eLevelInfoNormal,"::recv_hex_response invalid response");
            eIpmcStatus = eIpmcRcvHexResponseFail;
            goto ResetWatchDogFunc_end;
        }
    }
    else
    {
        FPTRACE(eLevelInfoNormal,"::recv_hex_response Error code returnd rc");
        eIpmcStatus = eIpmcRcvHexResponseFail;
        goto ResetWatchDogFunc_end;
    }

  ResetWatchDogFunc_end:

    FPTRACE(eLevelInfoNormal,"ResetWatchDogFunc::before close_ipmc_interface");
      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }
    FPTRACE(eLevelInfoNormal,"ResetWatchDogFunc::after close_ipmc_interface");

	return (INT32)eIpmcStatus;
}

INT32 TurnOffWatchDogFunc()
{
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(TurnOffWatchDog,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TurnOffWatchDogFunc: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto TurnOffWatchDogFunc_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);

  TurnOffWatchDogFunc_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;
}




INT32 SetPostKeepAliveResultFunc(UINT32 unUnitStatus , UINT32 unKeepALive)
{
	INT8 pcBuf[30] , tmp;
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	tmp = (unUnitStatus >> 24);


	if (unKeepALive == 1)
	{
		if (tmp < 0x10)
		{
			sprintf(pcBuf,"%s %x%x]\r",SetKeepAliveResult,0,unUnitStatus);
		}
		else
		{
			sprintf(pcBuf,"%s %x]\r",SetKeepAliveResult,unUnitStatus);
		}
	}
	else
	{
		if (tmp < 0x10)
		{
			sprintf(pcBuf,"%s %x%x]\r",SetPostResult,0,unUnitStatus);
		}
		else
		{
			sprintf(pcBuf,"%s %x]\r",SetPostResult,unUnitStatus);
		}
	}

	nFd = open_ipmc_interface_wrap() ;
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(pcBuf,nFd);
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SetPostKeepAliveResultFunc: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetPostKeepAliveResultFunc_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);

  SetPostKeepAliveResultFunc_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;
}



INT32 SetPowerOffCpu()
{
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;


	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(SetPowerOffCpuData,nFd);
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SetPowerOffCpu: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetPowerOffCpu_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);

  SetPowerOffCpu_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;
}




INT32 SetIPMCCPUTemperature(UINT32 Temperature)
{
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT8 cReqBuf[100];
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	//build the function
	sprintf(cReqBuf,"%s %02x]\r",SetCPUTemperature,Temperature) ;

	rc = write_ipmc_interface(cReqBuf,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SetIPMCCPUTemperature: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetIPMCCPUTemperatureFunc_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);

  SetIPMCCPUTemperatureFunc_end:

    FPTRACE(eLevelInfoNormal,"SetIPMCCPUTemperature::before close_ipmc_interface");
      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }
    FPTRACE(eLevelInfoNormal,"SetIPMCCPUTemperature::after close_ipmc_interface");

	return (INT32)eIpmcStatus;
}


INT32 SetIPMCHardDriveTemperature(UINT32 Temperature)
{
	INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT8 cReqBuf[100];
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}


	//build the function
	sprintf(cReqBuf,"%s %02x]\r",SetHDTemperature,Temperature) ;

	rc = write_ipmc_interface(cReqBuf,nFd) ;
	if(rc < 0)
	{
#ifdef MFA_BOARD_PRINT
		MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SetIPMCHardDriveTemperature: write_ipmc_interface error %d",rc);
#endif
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetIPMCHardDriveTemperature_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);


  SetIPMCHardDriveTemperature_end:

    FPTRACE(eLevelInfoNormal,"SetIPMCHardDriveTemperature::before close_ipmc_interface");
      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }
    FPTRACE(eLevelInfoNormal,"SetIPMCHardDriveTemperature::after close_ipmc_interface");

	return (INT32)eIpmcStatus;
}

INT32 ReadIPMCVersionFromChip(char * VersionBuf)
{	UINT8 cResponse[100];
	INT32 lcount =  99 , rc = 0;
	INT32  nFd;
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap() ;
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}

	rc = write_ipmc_interface(GetVersionNumber,nFd) ;
	if(rc < 0)
	{
		eIpmcStatus = eIpmcWriteIpmcFail;
	}
	else
    {
        rc = recv_hex_response(cResponse,&lcount,nFd) ;
        if(rc == 0)
        {
            if(memcmp(caGetIPMCVersion,cResponse,sizeof(caGetIPMCVersion) ) == 0)
            {
                strncpy(VersionBuf,(char *)(&cResponse[sizeof(caGetIPMCVersion)]),6);
            }
            else
            {
                eIpmcStatus = eIpmcReadIPMCVersionFromChipFail;
            }
        }
        else
        {
            eIpmcStatus = eIpmcRcvHexResponseFail;
        }
    }
    if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
    {
        close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                    // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
    }
    return (INT32)eIpmcStatus;

}


INT32 SetPostCommand(UINT32 unPostMessage)
{
    INT32 nFd , rc = 0;
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT8 cReqBuf[100];
	EIpmcStatus eIpmcStatus = eIpmcSuccess;

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		return (INT32)eIpmcOpenInterfaceFail;
	}
	memset(cReqBuf, 0 , 100);
	//build the function
	sprintf(cReqBuf,"%s %02x %02x %02x %02x]\r",SetPostResult,unPostMessage,0 ,0,0) ;
	rc = write_ipmc_interface(cReqBuf,nFd) ;

	if(rc < 0)
	{
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetPostCommand_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);

    rc = write_ipmc_interface(cReqBuf,nFd) ;

	if(rc < 0)
	{
		eIpmcStatus = eIpmcWriteIpmcFail;
		goto SetPostCommand_end;
	}

	//read the serial to clean the buffer
	rc = recv_hex_response(cResponse,&lcount,nFd);


  SetPostCommand_end:

      if ((eIpmcStatus == eIpmcWriteIpmcFail)||(eIpmcStatus == eIpmcRcvHexResponseFail))
      {
          close_ipmc_interface(nFd);  //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
                                      // Bracha: VNGFE-7110. Close nFd on eIpmcRcvHexResponseFail.
      }

	return (INT32)eIpmcStatus;

}





#ifdef MFA_BOARD_PRINT
INT32 PrintIpmcMessage(UINT32 unPrintSource , UINT32 unPrintLevel , UINT32 PrintFlag , INT8 *pcFormat, ...)
{
    INT8  string[1000];

    va_list tVarList;

    va_start(tVarList,pcFormat);
    vsprintf(string,pcFormat,tVarList);

    MfaBoardPrint(unPrintSource,unPrintLevel,PrintFlag,"%s",string);

    return 0;
}
#endif

