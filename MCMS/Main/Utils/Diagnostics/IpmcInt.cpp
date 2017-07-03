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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

UINT32 unRedLedId;
UINT32 unGreenLedId;
UINT32 unAmberLedId;


INT8 cRedNum;
INT8 cGreenNum;
INT8 cAmberNum;

char caHwSlotIdResponseHeader[7]   = {0xbc,0x0,0x5,0x0,0x0,0x40,0xa};

INT32 recv_hex_response(unsigned char *data, int *length , int nFd);

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

    fd = serial_open("/dev/ttyS0");
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
#ifdef UNIT_TEST
		printf("IPMC: serial_set_param failed");
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
#ifdef UNIT_TEST
		printf("IPMC: SerialConnectionFD opened");
#endif
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
#ifdef UNIT_TEST
    printf("IPMC: SerialConnectionFD closed");
#endif
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
#ifdef UNIT_TEST
		printf("%4d> %s\n", x, hex_rs);
#endif
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


INT32 IpmcGetHWSlotId()
{
	UINT8 cResponse[100];
	INT32 lcount =  99 , rc = 0;
	INT32 i , nFd;
	INT32 slotId = -1;
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
#ifdef UNIT_TEST
		printf("IpmcGetHWSlotId : write_ipmc_interface error %d",rc);
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
#ifdef UNIT_TEST
			printf("Invalid response for slot id");
#endif
			eIpmcStatus = eIpmcGetHWSlotIdFail;
			goto IpmcGetHWSlotId_end;
		}
#ifdef UNIT_TEST
		printf("Slot Id = 0x%x",slotId);
#endif
	}
	else
	{
#ifdef UNIT_TEST
		printf("recv_hex_response return 0x%x",rc);
#endif
		eIpmcStatus = eIpmcRcvHexResponseFail;
		goto IpmcGetHWSlotId_end;
	}


  IpmcGetHWSlotId_end:

    close_ipmc_interface(nFd);
	return slotId;
}


INT32 LedInterface(UINT32 unLedColor , UINT32 unStatus)
{
    unRedLedId = 1;
	cRedNum = 2;
    unGreenLedId = 2;
	cGreenNum = 3;
    unAmberLedId = 3;
	cAmberNum = 4;
	
	INT32 nFd , rc = 0;
	INT8 pcBuf[30];
	UINT8 cResponse[100];
	INT32 lcount =  99;
	INT8 cLedNum =0;
	INT8 cLedColor =0;
	
	EIpmcStatus eIpmcStatus = eIpmcSuccess;
	
	static UINT32 unRedLedStatus = TURN_OFF , unGreenLedStatus = TURN_OFF , unAmberLedStatus = TURN_OFF;

	for(int i = 0; i < 30; i++)
		pcBuf[i] = 0;
	
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
			snprintf(pcBuf, sizeof(pcBuf), "%s %02x %02x %02x %02x]\r",SetFruLedState,cLedNum,0xff,0,cLedColor) ;
			break;
		}
		
		case TURN_OFF:
		{
			//turn of the leds
			snprintf(pcBuf, sizeof(pcBuf), "%s %02x %02x %02x %02x]\r",SetFruLedState,cLedNum,0,0,cLedColor) ;
			break;
		}
		
		case BLINK_LED:
		{
			//turn of the leds
			snprintf(pcBuf, sizeof(pcBuf), "%s %02x %02x %02x %02x]\r",SetFruLedState,cLedNum,0x30,0x30,cLedColor) ;
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

	if (eIpmcStatus == eIpmcWriteIpmcFail)
    {
        close_ipmc_interface(nFd); //[Dotan.H 24/05/2010 VNGFE-2756 commented not to close tty0]
    }

#ifdef MFA_BOARD_PRINT
	MfaBoardPrintIpmc(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"led = %x , action = %x",unLedColor,unStatus);					
#endif
	return (INT32)eIpmcStatus;	
}

#define READ_REMOTE_SW2_STATUS_START    "[180034402028B8"

INT32 IpmcIsDiagMode(void)
{
	int  isToClearStatus = 0;
	char infoReqString[50];
	UINT8 cResponse[50];
	unsigned int chkSum,mySlotId;
	INT32 lcount =  99 , rc = 0;
	INT32 nFd;

	memset(cResponse,0,50);

	//mySlotId = IpmcGetHWSlotId();
	//fprintf(stderr,"Solt ID = %u\n",mySlotId);
	mySlotId = 1;
	mySlotId <<=1; //shifted left, required by protocol
	chkSum = mySlotId + isToClearStatus + 0x11 + 0x1 + 0x55;
	chkSum = chkSum & 0xff;
	chkSum = 0x100 - chkSum;

	snprintf(infoReqString, sizeof(infoReqString),
			"%s%02x00110100%02x55%02x]\r", READ_REMOTE_SW2_STATUS_START, mySlotId, isToClearStatus, chkSum);

	nFd = open_ipmc_interface_wrap();
	if(nFd < 0)
	{
		fprintf(stderr,"open_ipmc_interface_wrap() is error\n");	
		return 0;
	}

	do
	{
		rc = write_ipmc_interface(infoReqString,nFd) ;
		if(rc < 0)
		{
			fprintf(stderr,"write_ipmc_interface() is error\n");	
			goto IpmcIsDiagMode_end;
		}

		rc = recv_hex_response(cResponse,&lcount,nFd) ;
		if(rc == 0)
		{
			printf("recv_hex_response = %d %d %d %d %d %d %d %d %d %d\n %d %d %d %d %d %d %d %d %d %d\n %d %d %d %d %d %d %d %d %d %d\n %d %d %d %d %d %d %d %d %d %d\n %d %d %d %d %d %d %d %d %d %d\n",
				                                                                                cResponse[0],cResponse[1],cResponse[2],cResponse[3],cResponse[4],
																								cResponse[5],cResponse[6],cResponse[7],cResponse[8],cResponse[9],
																								cResponse[10],cResponse[11],cResponse[12],cResponse[13],cResponse[14],
																								cResponse[15],cResponse[16],cResponse[17],cResponse[18],cResponse[19],
																								cResponse[20],cResponse[21],cResponse[22],cResponse[23],cResponse[24],
																								cResponse[25],cResponse[26],cResponse[27],cResponse[28],cResponse[29],
																								cResponse[30],cResponse[31],cResponse[32],cResponse[33],cResponse[34],
																								cResponse[35],cResponse[36],cResponse[37],cResponse[38],cResponse[39],
																								cResponse[40],cResponse[41],cResponse[42],cResponse[43],cResponse[44],
																								cResponse[45],cResponse[46],cResponse[47],cResponse[48],cResponse[49]);
			if (cResponse[12] == 1)
			{
				printf("recv_hex_response[12] = (%c)\n",cResponse[12]);
			    close_ipmc_interface(nFd);
				return 1;//ok
			}
			else if(cResponse[12] == 0)
			{
				close_ipmc_interface(nFd);
				return 2;
			}
		}
		else
		{
			fprintf(stderr,"recv_hex_response() is error\n");	
			goto IpmcIsDiagMode_end;
		}
	}while( (cResponse[6] == 'c'||cResponse[6] == 'C')&&cResponse[7] == '3');


  
  IpmcIsDiagMode_end:
    close_ipmc_interface(nFd);			
    return 0;
}


