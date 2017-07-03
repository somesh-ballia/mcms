//#include "tdd.h"
#include "Print.h"
#include "LinuxSystemCallsApi.h"

INT32 ut_retValgetEnv;

extern INT32 ut_retValmsgGet;
extern INT8 *ut_retValgetenv;
extern UINT8 ut_flagpErrorWasCalled;
extern UINT8 *ut_valpErrorLastMessage;

extern void AddFailure(UINT8 *pucTestName,UINT8 *pucMsg , INT32 uLine , UINT8 *pucFile);
extern void AddSuccess();

void test_CreateMessaageQueue_NoMssgGet_ReturnsMinus1();
void test_CreateMessaageQueue_MsgGetReturns1_ReturnSameValue();
void test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError();
void test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError_getenv();


#define TEST_ASSERT(condition,FailMsg,TestName)\
		if (condition) {AddSuccess();} else {AddFailure(TestName, FailMsg , __LINE__ , __FILE__);}

void RunSuite_TestLinuxSystemCalls()
{
	test_CreateMessaageQueue_NoMssgGet_ReturnsMinus1();
	test_CreateMessaageQueue_MsgGetReturns1_ReturnSameValue();
	test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError();
	test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError_getenv();
}

void test_CreateMessaageQueue_NoMssgGet_ReturnsMinus1()
{
	INT32 result;
	
	ut_retValmsgGet = -1;
	result = CreateMessageQueue();
	TEST_ASSERT(result==-1,
				"CreateMessageQueue should have returned -1 when getting -1 from getmssg",
				"test_CreateMessaageQueue_NoMssgGet_ReturnsMinus1");
}



void test_CreateMessaageQueue_MsgGetReturns1_ReturnSameValue()
{
	INT32 result;
	
	ut_retValmsgGet = -1;
	result = CreateMessageQueue();
	TEST_ASSERT(result==-1,
				"CreateMessageQueue should have returned 1 when getting 1 from getmssg",
				"test_CreateMessaageQueue_MsgGetReturns1_ReturnSameValue");
}



void test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError()
{
	INT32 result;
	
	ut_retValmsgGet = -1;
	ut_valpErrorLastMessage="Check The perror";
	result = CreateMessageQueue();
	TEST_ASSERT(result==-1,
				"CreateMessageQueue should have returned -1 when printing error",
				"test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError");
}


void test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError_getenv()
{
	INT32 result;
	
	ut_retValmsgGet = -1;
	ut_valpErrorLastMessage="Check The perror";
	ut_retValgetenv = "HOME";
	result = CreateMessageQueue();
	TEST_ASSERT(result==-1,
				"CreateMessageQueue should have returned -1 when printing error",
				"test_CreateMessaageQueue_MsgGetReturnsMinus1_ReturnError_getenv");
}


