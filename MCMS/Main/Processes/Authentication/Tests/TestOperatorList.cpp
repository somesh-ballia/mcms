// TestOperatorList.cpp

#include "TestOperatorList.h"
#include "OperatorList.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "ChangePassword.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "ObjString.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestOperatorList );

COperatorList* pOperatorList = NULL;

void CTestOperatorList::setUp()
{
	pOperatorList = new COperatorList(TRUE, FALSE);
}

void CTestOperatorList::tearDown()
{
	POBJDELETE(pOperatorList);
}

void CTestOperatorList::testOperatorListConstructor()
{
	CPPUNIT_ASSERT_MESSAGE("CTestOperatorList::testOperatorListConstructor",
                         pOperatorList != NULL);
}

//////////////////////////////////////////////////////////////////////
void CTestOperatorList::testOperatorListAddCancelOperator()
{
	COperator newOperator;

	newOperator.SetLogin("Hello");
	newOperator.SetPassword("World");
	newOperator.SetAuthorization(ORDINARY);

	if (pOperatorList->IsValidToAddUser(newOperator) == STATUS_OK)
		pOperatorList->Add(newOperator);

	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListAddCancelOperator (1)", pOperatorList->FindLogin("Hello") != NOT_FOUND );

    	int authorization = newOperator.GetAuthorization();

	if (pOperatorList->IsValidToDeleteUser(newOperator.GetLogin().c_str(),authorization) == STATUS_OK)
		pOperatorList->Cancel("Hello");

	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListAddCancelOperator (2)", pOperatorList->FindLogin("Hello") == NOT_FOUND );
}

void CTestOperatorList::testOperatorListChangeOperPassword()
{
	int authorization = GUEST;

	pOperatorList->SetTDDState(TRUE);

	COperator oper;

	oper.SetLogin("Hello");
	oper.SetPassword("World");
	oper.SetAuthorization(ORDINARY);
	oper.EncryptPassword();

	if (pOperatorList->IsValidToAddUser(oper) == STATUS_OK)
		pOperatorList->Add(oper);

	std::string user = "Hello";
	std::string pwd = "World";

	if (pOperatorList->IsValidToChangePassword(user, pwd, authorization) == STATUS_OK)
		pOperatorList->UpdatePassword("Hello","Universe");

	oper.SetPassword("Universe");
	oper.EncryptPassword();

    // SAGI: please restore after JITC merge
//	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListChangeOperPassword",
//		pOperatorList->FindOperator(oper) != NOT_FOUND );

    
	if (pOperatorList->IsValidToDeleteUser(oper.GetLogin().c_str(),authorization) == STATUS_OK)
		pOperatorList->Cancel("Hello");

	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListAddCancelOperator",
			pOperatorList->FindLogin("Hello") == NOT_FOUND );
}

//////////////////////////////////////////////////////////////////////
void CTestOperatorList::testOperatorListSaveToLoadFromFile()
{
	COperator oper;

	oper.SetLogin("Hello");
	oper.SetPassword("World");
	oper.SetAuthorization(ORDINARY);
	oper.EncryptPassword();

	int authorization = GUEST;

	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListSaveToLoadFromFile",
		pOperatorList->FindOperator(oper) == NOT_FOUND );

	if (pOperatorList->IsValidToAddUser(oper) == STATUS_OK)
		pOperatorList->Add(oper);

	delete pOperatorList;

	pOperatorList = new COperatorList(TRUE, FALSE);

//	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListSaveToLoadFromFile",
//		pOperatorList->FindOperator(oper) != NOT_FOUND );
    int auth = oper.GetAuthorization();
	if (pOperatorList->IsValidToDeleteUser(oper.GetLogin().c_str(),auth) == STATUS_OK)
		pOperatorList->Cancel("Hello");

	CPPUNIT_ASSERT_MESSAGE( "CTestOperatorList::testOperatorListAddCancelOperator",
		pOperatorList->FindLogin("Hello") == NOT_FOUND );
}

void CTestOperatorList::Test4CharChanged_noChange()
{
	pOperatorList->SetTDDState(TRUE);

	STATUS status = pOperatorList->CheckIfAtLeast4CharChanged("User_pwd1234%^", "User_pwd1234%^");

	CPPUNIT_ASSERT(STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED == status);
}

void CTestOperatorList::Test4CharChanged_1Change()
{
	pOperatorList->SetTDDState(TRUE);

	STATUS status = pOperatorList->CheckIfAtLeast4CharChanged("User_pwd1234%^", "User_pwd2234%^");

	CPPUNIT_ASSERT(STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED == status);
}

void CTestOperatorList::Test4CharChanged_2Change()
{
	pOperatorList->SetTDDState(TRUE);

	STATUS status = pOperatorList->CheckIfAtLeast4CharChanged("User_pwd1234%^", "User_pwd2334%^");

	CPPUNIT_ASSERT(STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED == status);
}

void CTestOperatorList::Test4CharChanged_3Change()
{
	pOperatorList->SetTDDState(TRUE);

	STATUS status = pOperatorList->CheckIfAtLeast4CharChanged("User_pwd1234%^", "User_pwd2344%^");

	CPPUNIT_ASSERT(STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED == status);

}

void CTestOperatorList::Test4CharChanged_4Change()
{
	pOperatorList->SetTDDState(TRUE);
	STATUS status = pOperatorList->CheckIfAtLeast4CharChanged("User_pwd1234%^", "User_pwd2345%^");

	CPPUNIT_ASSERT(STATUS_OK == status);
}

void CTestOperatorList::TestStrongPassword_empty()
{
	CLargeString description;
	STATUS status = CMcmsAuthentication::IsLegalStrongPassword("User", "", description);

	CPPUNIT_ASSERT(STATUS_INVALID_STRONG_PASSWORD == status);
}

//////////////////////////////////////////////////////////////////////
void CTestOperatorList::TestPasswordHistory_currentPwd()
{
	pOperatorList->SetTDDState(TRUE);

	COperator Operator;
	Operator.SetLogin("User");
	Operator.SetPassword("User_pwd1234%^");
	Operator.EncryptPassword();

	pOperatorList->Add(Operator);

	BOOL result = pOperatorList->WasPasswordUsed("User", "User_pwd1234%^");

	CPPUNIT_ASSERT(TRUE == result);

	pOperatorList->Cancel("User");
}

void CTestOperatorList::TestPasswordHistory_Log1()
{
	COperator Operator;
	Operator.SetLogin("User");
	Operator.SetPassword("User_pwd1234%^0");
	Operator.EncryptPassword();
	Operator.SetTDDState(TRUE);

	pOperatorList->Add(Operator);
	pOperatorList->UpdatePassword("User", "User_pwd1234%^1");

	BOOL result = pOperatorList->WasPasswordUsed("User", "User_pwd1234%^0");

	CPPUNIT_ASSERT(TRUE == result);

	pOperatorList->Cancel("User");
}

void CTestOperatorList::TestPasswordHistory_Log10()
{
	COperator Operator;
	Operator.SetLogin("User");
	Operator.SetPassword("User_pwd1234%^0");
	Operator.EncryptPassword();
	Operator.SetTDDState(TRUE);

	pOperatorList->Add(Operator);
	pOperatorList->UpdatePassword("User", "User_pwd1234%^1");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^2");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^3");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^4");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^5");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^6");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^7");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^8");
	pOperatorList->UpdatePassword("User", "User_pwd1234%^9");

	BOOL result = pOperatorList->WasPasswordUsed("User", "User_pwd1234%^0");

	CPPUNIT_ASSERT(TRUE == result);

	pOperatorList->Cancel("User");
}
