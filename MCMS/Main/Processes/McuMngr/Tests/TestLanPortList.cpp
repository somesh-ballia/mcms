#include "TestLanPortList.h"
#include "LanPortList.h"
#include "LanPort.h"
#include "IpmiSensorDescrToType.h"
#include "GetSysTemperature.h"
#include "GetIFFeatures.h"
#include <string.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestLanPortList );

class LanPortListFake : public CLanPortList
{
public: 
    vector<LanPortSummary> const & GetSummary() const
    {
        return m_lans;
    }
};

class LanPortFake : public CLanPort
{
public: 
    LanPortInfo const & GetInfo() const
    {
        return m_portInfo;
    }
};

void TestLanPortList::setUp()
{
}


void TestLanPortList::tearDown()
{
}

//#define TEST_GET_IF_FEATURES
void TestLanPortList::testLanList()
{
#ifdef TEST_GET_IF_FEATURES
    LanPortListFake fake;
    fake.Update();
    vector<LanPortSummary> const & ports = fake.GetSummary();
    int const count = ports.size();
    fprintf(stderr, "NIC count=%d\n", fake.GetSummary().size());
    for (int i=0; i<count; ++i)
    {
        fprintf(stderr, "NIC[%d] = { %d, %d, %d }\n", i, ports[i].status, ports[i].portID, ports[i].slotID);
    }
#endif
}

#ifdef TEST_GET_IF_FEATURES
extern GetIFFeaturesFunc GetIFFeatures;
extern bool GetIFFeaturesByCmdLine(int ethNo, IFFeatures *iff);
#endif
void TestLanPortList::testLanPort()
{
#ifdef TEST_GET_IF_FEATURES
    LanPortListFake fake;
    fake.Update();
    vector<LanPortSummary> const & ports = fake.GetSummary();
    int const count = ports.size();
    for (int i=0; i<count; ++i)
    {
        LanPortFake lanPort;
        lanPort.Update(i);
        LanPortInfo const & info = lanPort.GetInfo();
        fprintf(stderr, "NIC[%d}: {%08x,%08x,%08x; %08x,%08x}, {%s,%s,%s,%s,%s,%s,%s,%s; %s,%s,%s,%s,%s,%s,%s,%s}\n"
                , ports[i].slotID
                , info.actLinkAutoNeg
                , info.actLinkMode
                , info.actLinkStatus
                , info.advLinkAutoNeg
                , info.advLinkMode

                , info.txOctets
                , info.maxTxOctets
                , info.txPackets
                , info.maxTxPackets
                , info.txBadPackets
                , info.maxTxBadPackets
                , info.txFifoDrops
                , info.maxTxFifoDrops

                , info.rxOctets
                , info.maxRxOctets
                , info.rxPackets
                , info.maxRxPackets
                , info.rxBadPackets
                , info.maxRxBadPackets
                , info.rxCRC
                , info.maxRxCRC
                );
    }
#endif
}


