#include "TestAvcSvcCap.h"
#include "AvcSvcCapStruct.h"
#include "Licensing.h"
#include "VersionStruct.h"
#include "ProductType.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestAvcSvcCap );

int const DEFAULT_CP_PARTY_COUNT = 30;

void CTestAvcSvcCap::setUp()
{
    m_pLicensing = new CLicensing();
    m_pLicensing->SetTotalNumOfCpParties(DEFAULT_CP_PARTY_COUNT);
    m_pLicensing->SetIsSvcEnabled(YES);
    m_pLicensing->SetIsTelepresenceEnabled(YES);
}

void CTestAvcSvcCap::tearDown()
{
    delete m_pLicensing;
    m_pLicensing = 0;
}

extern void GetAvcSvcCap_SoftMcu(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap);
extern void GetAvcSvcCap_SoftMcuMfw(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap);
extern void GetAvcSvcCap_Gesher(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap);
extern void GetAvcSvcCap_HWRMX(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap);
extern void GetAvcSvcCap_SoftMcuEdge(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap);
//////////////////////////////////////////////////////////////////////
void CTestAvcSvcCap::testGetCapSoft()
{
    AvcSvcCap cap;
    VERSION_S ver;
    bzero(&ver, sizeof(ver));

    GetAvcSvcCap_SoftMcu(*m_pLicensing, ver, cap);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CP_PARTY_COUNT, cap.cpPortCount);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportAvcCp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAvcVsw);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportMixedCp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportMixedVsw);

    //CPPUNIT_ASSERT_EQUAL(1, cap.supportCascadeAvc);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportCascadeSvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpAvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportTip);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportAvcCifPlus);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportItp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAudioOnlyConf);
} 

void CTestAvcSvcCap::testGetCapSoftMfw()
{
    AvcSvcCap cap;
    VERSION_S ver;
    bzero(&ver, sizeof(ver));

    GetAvcSvcCap_SoftMcuMfw(*m_pLicensing, ver, cap);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CP_PARTY_COUNT, cap.cpPortCount);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAvcCp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAvcVsw);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportSvc);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportMixedCp);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportMixedVsw);

    //CPPUNIT_ASSERT_EQUAL(0, cap.supportCascadeAvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportCascadeSvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpAvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpSvc);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportTip);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportItp);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportAudioOnlyConf);
}

void CTestAvcSvcCap::testGetCapGesher()
{
    AvcSvcCap cap;
    VERSION_S ver;
    bzero(&ver, sizeof(ver));

    GetAvcSvcCap_Gesher(*m_pLicensing, ver, cap);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CP_PARTY_COUNT, cap.cpPortCount);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAvcVsw);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportMixedCp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportMixedVsw);

    //CPPUNIT_ASSERT_EQUAL(1, cap.supportCascadeAvc);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportCascadeSvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpAvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportTip);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportItp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAudioOnlyConf);
}

void CTestAvcSvcCap::testGetCapHwLowVer()
{
    AvcSvcCap cap;
    VERSION_S ver;
    {
        bzero(&ver, sizeof(ver));
        ver.ver_major = 7;
        ver.ver_minor = 8;
    }

    GetAvcSvcCap_HWRMX(*m_pLicensing, ver, cap);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CP_PARTY_COUNT, cap.cpPortCount);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportAvcVsw);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportSvc);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportMixedCp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportMixedVsw);

    //CPPUNIT_ASSERT_EQUAL(1, cap.supportCascadeAvc);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportCascadeSvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpAvc);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportSrtpSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportTip);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportItp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAudioOnlyConf);
}

void CTestAvcSvcCap::testGetCapHwLittleHighVer()
{
    VERSION_S ver;
    {
        bzero(&ver, sizeof(ver));
        ver.ver_major = 7;
        ver.ver_minor = 9;
    }

    this->testGetCapHwWithHighVer(ver);
}

void CTestAvcSvcCap::testGetCapHwMuchHighVer()
{
    VERSION_S ver;
    {
        bzero(&ver, sizeof(ver));
        ver.ver_major = 8;
        ver.ver_minor = 2;
    }

    this->testGetCapHwWithHighVer(ver);
}

void CTestAvcSvcCap::testGetCapHwWithHighVer(VERSION_S const & ver)
{
    AvcSvcCap cap;

    GetAvcSvcCap_HWRMX(*m_pLicensing, ver, cap);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_CP_PARTY_COUNT, cap.cpPortCount);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportAvcVsw);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportMixedCp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportMixedVsw);

    //CPPUNIT_ASSERT_EQUAL(1, cap.supportCascadeAvc);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportCascadeSvc);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportSrtpAvc);
    //CPPUNIT_ASSERT_EQUAL(0, cap.supportSrtpSvc);
    CPPUNIT_ASSERT_EQUAL(1, cap.supportTip);
    //CPPUNIT_ASSERT_EQUAL(1, cap.supportItp);
    CPPUNIT_ASSERT_EQUAL(0, cap.supportAudioOnlyConf);
}

typedef void (*GetAvcSvcCapFunc)(CLicensing const &, VERSION_S const & ver, AvcSvcCap & cap);
extern GetAvcSvcCapFunc FindGetAvcSvcCapFunc(eProductType type);
void CTestAvcSvcCap::testGetFunc()
{
    CPPUNIT_ASSERT_EQUAL((GetAvcSvcCapFunc)GetAvcSvcCap_SoftMcuMfw, FindGetAvcSvcCapFunc(eProductTypeSoftMCUMfw));
    CPPUNIT_ASSERT_EQUAL((GetAvcSvcCapFunc)GetAvcSvcCap_Gesher, FindGetAvcSvcCapFunc(eProductTypeGesher));
    CPPUNIT_ASSERT_EQUAL((GetAvcSvcCapFunc)GetAvcSvcCap_HWRMX, FindGetAvcSvcCapFunc(eProductTypeRMX1500));
    CPPUNIT_ASSERT_EQUAL((GetAvcSvcCapFunc)GetAvcSvcCap_SoftMcuEdge, FindGetAvcSvcCapFunc(eProductTypeEdgeAxis));
} 

