#ifndef _DIAGNOSTICS_API_EXT_H
#define _DIAGNOSTICS_API_EXT_H

#include "McmsApi.h"
//#include "McmsSim.h"
//#include "TBStructs.h"
//#include "HostCommonDefinitions.h"

/*****************************************************************************/
/*	Diagnostics & EMA simulation API  										 */
/*****************************************************************************/
//#define EMA_DIAG_TEST_REQ_OPCODE			1000019
//#define DIAG_EMA_TEST_RESULT_OPCODE			1000020
//#define EMA_DIAG_TEST_LIST_REQ_OPCODE		1000021
//#define EMA_ENTER_TEST_MODE_IND_OPCODE	1000022
//#define EMA_RESET_MFA_REQ_OPCODE			1000023
//#define DIAG_EMA_TEST_LIST_IND_OPCODE		1000024
//#define EMA_RESET_TO_SHELF_MSG_OPCODE		1000025

typedef struct
{
	UINT32	unTestChoice;
	UINT32	unBoardNum;

}TEmaTestReqMsgParams;

typedef struct
{
	TGeneralMcmsCommonHeader	tGeneralMcmsCommonHeader;
    TEmaTestReqMsgParams		tEmaTestReqMsgParams;
    
}TEmaTestReqMesaage;

typedef struct
{
//	UINT32		unTestChoice;
	UINT32		unBoardNum;
	UINT32		unTestNum;
//	UINT8		testString[4];
	APIUBOOL	testStatus; // 1 - PASS; 0 - FAIL
}TDiagTestResultParams;

typedef struct
{
	TGeneralMcmsCommonHeader		tGeneralMcmsCommonHeader;
    TDiagTestResultParams			tDiagTestResultParams;
}TDiagTestResultMessage;

// Message to the shelf management
typedef struct SIpmiResetData
{                                                                                                                                              
	UINT32 opcode;                
	UINT32 msgId;
	UINT32 slotId;
	UINT32 resetType;
}TIpmiResetData;

typedef struct
{
	TGeneralMcmsCommonHeader		tGeneralMcmsCommonHeader;
    TIpmiResetData					tIpmiResetData;
}TEMAResetToShelfMessage;

extern void 	DiagSwitchTestModeIndicationMsg(UINT32 unBoardNum);
extern void 	DiagSwitchTestListMsg(UINT32 unBoardNum);
extern void 	DiagSwitchTestStatusMsg(UINT32 unBoardNum, APIUBOOL testStatus);
extern void 	SwitchDiagTestModeRequestMsg(UINT32 unBoardNum);
extern void		SwitchDiagTestRequestMsg(UINT32 testNum, UINT32 boardNum);
extern void		SwitchShelfResetRequestMsg(UINT32 ucBoardId);

#endif

