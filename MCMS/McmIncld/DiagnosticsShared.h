/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	DiagnosticsOpcodes.h                                                  */
/* PROJECT:  	Shared module - EMA API 									  */
/* PROGRAMMER:  Bracha Schushan												  */
/* DESCRIPTION: Shared Diagnostics Information Between EMA ,Switch CM, MFA CM,*/
/* 				Card Diagnostics & MCMS Modules.				              */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/

#ifndef DIAG_SHARED_H_
#define DIAG_SHARED_H_

// Diagnostics requests opcodes (0x300 - 0x37F)
#define	EMA_ENTER_DIAG_MODE_REQ		0x300
#define	EMA_GET_TEST_LIST_REQ 		0x301
#define	EMA_START_TEST_REQ			0x302
#define	EMA_GET_UNITS_STATE_REQ		0x303
#define	EMA_GET_TEST_STATUS_REQ		0x304
#define	EMA_STOP_TEST_REQ			0x305
#define	EMA_GET_ERROR_LIST_REQ 		0x306

// Diagnostics indications opcodes (Same as the opcodes of the requests)
#define	EMA_ENTER_DIAG_MODE_IND		0x300
#define	EMA_GET_TEST_LIST_IND 		0x301
#define	EMA_START_TEST_IND			0x302
#define	EMA_GET_UNITS_STATE_IND		0x303
#define	EMA_GET_TEST_STATUS_IND		0x304
#define	EMA_STOP_TEST_IND			0x305
#define	EMA_GET_ERROR_LIST_IND 		0x306

#endif /*DIAG_SHARED_H_*/

