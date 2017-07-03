#include "TestProcessBase.h"
#include "ProcessBase.h"



// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestProcessBase );

void TestProcessBase::testValidateString()
{
    const DWORD buffLen = 32;
    char buffer[buffLen];

    DWORD *ptr = (DWORD*)buffer;
    *ptr = 13;
    eStringValidityStatus status = CProcessBase::TestStringValidity(buffer,
                                                                    buffLen,
                                                                    __PRETTY_FUNCTION__,
                                                                    false);
    CPPUNIT_ASSERT(eStringInvalidChar == status);
    
    memset(buffer, 'a', buffLen);
    status = CProcessBase::TestStringValidity(buffer,
                                              buffLen,
                                              __PRETTY_FUNCTION__,
                                              false);
    CPPUNIT_ASSERT(eStringNotNullTerminated == status);

    
    buffer[buffLen / 2] = '\0';
    status = CProcessBase::TestStringValidity(buffer,
                                              buffLen,
                                              __PRETTY_FUNCTION__,
                                              false);
    CPPUNIT_ASSERT(eStringValid == status);
}



