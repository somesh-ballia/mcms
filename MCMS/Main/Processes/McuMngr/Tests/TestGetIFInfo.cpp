#include "TestGetIFInfo.h"
#include "GetIFStatistics.h"
#include "GetIFStatus.h"
#include "GetIFFeatures.h"
#include <string.h>
#include <stdio.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestGetIFInfo );

static GetIFFeaturesFunc s_prevGetIFFeaturesFunc;
void TestGetIFInfo::setUp()
{
    s_prevGetIFFeaturesFunc = GetIFFeatures;
}


void TestGetIFInfo::tearDown()
{
    GetIFFeatures = s_prevGetIFFeaturesFunc;
}


void TestGetIFInfo::testGetIFStatistics()
{
    int const ethNo = 0;
    IFStatistics stat;
    bool const result = GetIFStatistics(ethNo, &stat);
    CPPUNIT_ASSERT(result);
    printf("if statistics: bytes recv %s, bytes sent %s\n", stat.bytesRecved, stat.bytesSent);
}

extern bool GetIFFeaturesByFunctionCall(int ethNo, IFFeatures *iff);
extern bool IsLinkUpByFunctionCall(int ethNo, int & linkUp);
void TestGetIFInfo::testGetIFFeatures()
{
    GetIFFeatures = GetIFFeaturesByFunctionCall;
    int const ethNo = 0;
    IFFeatures iff;
    (*GetIFFeatures)(ethNo, &iff);
    printf("iff: (%08x, %d), (%08x, %d)\n", iff.advertisedLinkMode, iff.advertisedAutoNeg, iff.activeLinkMode, iff.activeAutoNeg);

    IsLinkUp = IsLinkUpByFunctionCall;
    int isLinkUp = 0;
    (*IsLinkUp)(ethNo, isLinkUp);
    printf("eth0 is link detected: %d\n", isLinkUp);
}

void TestGetIFInfo::testGetIFStatus()
{
    int const isUp = (*IsIFUp)("eth0");
    printf("eth0 up status: %d\n", isUp);
}

