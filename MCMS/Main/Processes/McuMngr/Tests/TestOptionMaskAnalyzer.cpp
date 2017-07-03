#include "TestOptionMaskAnalyzer.h"
#include "OptionMaskAnalyzer.h"
#include "Trace.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestOptionMaskAnalyzer );

void CTestOptionMaskAnalyzer::setUp()
{
}

void CTestOptionMaskAnalyzer::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestOptionMaskAnalyzer::testGetFeatures()
{
    unsigned long OPTION_MASK = 0x01890404;
    unsigned long featuresMask = GetFeatureMask(OPTION_MASK);
    printf("%s:%s:%d: %08lx -- encry [%d], telepre [%d], multiple service[%d]\n", __FILE__, __func__, __LINE__
            , featuresMask
            , GetEncryptionFlag(featuresMask)
            , GetTelepresenceFlag(featuresMask)
            , GetMultipleServiceFlag(featuresMask)
          );
    CPPUNIT_ASSERT_EQUAL(1, GetEncryptionFlag(featuresMask));
    CPPUNIT_ASSERT_EQUAL(0, GetTelepresenceFlag(featuresMask));
    CPPUNIT_ASSERT_EQUAL(1, GetMultipleServiceFlag(featuresMask));
} 

void CTestOptionMaskAnalyzer::testGetPartners()
{
    unsigned long OPTION_MASK = 0x01FF0404;
    unsigned long partnerMask = GetPartnerMask(OPTION_MASK);
    printf("%s:%s:%d: %08lx -- Mpmx [%d], Avaya [%d], IBM [%d]\n", __FILE__, __func__, __LINE__
            , partnerMask
            , GetMpmxFlag(partnerMask)
            , GetAvayaFlag(partnerMask)
            , GetIBMFlag(partnerMask)
          );
    CPPUNIT_ASSERT_EQUAL(1, GetMpmxFlag(partnerMask));
    CPPUNIT_ASSERT_EQUAL(1, GetAvayaFlag(partnerMask));
    CPPUNIT_ASSERT_EQUAL(1, GetIBMFlag(partnerMask));
} 

