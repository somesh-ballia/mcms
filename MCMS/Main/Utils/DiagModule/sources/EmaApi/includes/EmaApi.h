/*============================================================================*/
/*            Copyright ?? 2006 Polycom Israel,Ltd. All rights reserved        */
/*----------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary        */
/* information of Polycom Israel, Ltd. and is protected by law.               */
/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	EmaApi.h                                                      */
/* PROJECT:  	Switch Card - Ema API Module								  */
/* PROGRAMMER:  Eyal Ben-Sasson												  */
/* DESCRIPTION: 											                  */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/

#ifndef EMAAPI_H_
#define EMAAPI_H_

#include "EmaShared.h"
//#include "LSShared.h"
#include "SocketApiTypes.h"

 

void BuildLanStatInfoReq(UINT32 ulOpcode);
void StringWrapper(e_TcpConn eID,UINT32 Msg);
void StringStripper( UINT8* commandRcvd,UINT32 NumBytes );


#endif /*EMAAPI_H_*/
