#include "TestGetEthBondStatus.h"
#include "GetEthBondStatus.h"
#include "IpmiSensorEnums.h"
#include "DefinesGeneral.h"
#include <string.h>
#include <stdlib.h>


CPPUNIT_TEST_SUITE_REGISTRATION( CTestGetEthBondStatus );

std::string const SIMULATE_BONDING_FILE = MCU_TMP_DIR+"/bonding";
char const * bond0Content = "Ethernet Channel Bonding Driver: v3.7.0 (June 2, 2010)\n"
"\n"
"Bonding Mode: fault-tolerance (active-backup)\n"
"Primary Slave: None\n"
"Currently Active Slave: eth1\n"
"MII Status: up\n"
"MII Polling Interval (ms): 100\n"
"Up Delay (ms): 0\n"
"Down Delay (ms): 0\n"
"\n"
"Slave Interface: eth1\n"
"MII Status: up\n"
"Speed: 1000 Mbps\n"
"Duplex: full\n"
"Link Failure Count: 0\n"
"Permanent HW addr: e0:db:55:08:0b:5d\n"
"Slave queue ID: 0\n"
"\n"
"Slave Interface: eth2\n"
"MII Status: down\n"
"Speed: 100 Mbps\n"
"Duplex: full\n"
"Link Failure Count: 0\n"
"Permanent HW addr: e0:db:55:08:0b:5e\n"
"Slave queue ID: 0\n"
"\n"
"Slave Interface: eth3\n"
"MII Status: down\n"
"Speed: 100 Mbps\n"
"Duplex: full\n"
"Link Failure Count: 0\n"
"Permanent HW addr: e0:db:55:08:0b:5f\n"
"Slave queue ID: 0\n";

std::string const BAD_SIMULATE_BONDING_FILE = MCU_TMP_DIR+"/bad-bonding";
char const * badBond0Content = "Ethernet Channel Bonding Driver: v3.7.0 (June 2, 2010)\n"
"\n"
"Bonding Mode: load balancing (round-robin)\n"
"MII Status: down\n"
"MII Polling Interval (ms): 0\n"
"Up Delay (ms): 0\n"
"Down Delay (ms): 0\n";

void CTestGetEthBondStatus::setUp()
{
    {
        FILE * fp = fopen(SIMULATE_BONDING_FILE.c_str(), "w");
        if (fp)
        {
            fwrite(bond0Content, 1, strlen(bond0Content), fp);
            fclose(fp);
        }
    }
}

void CTestGetEthBondStatus::tearDown()
{
    {
        unlink(SIMULATE_BONDING_FILE.c_str());
    }
}

extern int IsBondEnabled(char const * name);
extern int GetActiveBondingSlave(char const * name, EthBondingInfo & info);
extern int GetAllBondingSlaves(char const * name, EthBondingInfo & info);
extern void GetBackupBondingSlaves(EthBondingInfo & info);
//////////////////////////////////////////////////////////////////////
void CTestGetEthBondStatus::testGetNormalStatus()
{
    CPPUNIT_ASSERT_EQUAL(1, IsBondEnabled(SIMULATE_BONDING_FILE.c_str()));

    EthBondingInfo info;

    {
        GetActiveBondingSlave(SIMULATE_BONDING_FILE.c_str(), info);
        CPPUNIT_ASSERT_EQUAL(1, info.activeEthNo);
    }

    {
        GetAllBondingSlaves(SIMULATE_BONDING_FILE.c_str(), info);
        CPPUNIT_ASSERT_EQUAL(3, info.slaveEthCount);
        CPPUNIT_ASSERT_EQUAL(1, info.slaveEthNos[0]);
        CPPUNIT_ASSERT_EQUAL(2, info.slaveEthNos[1]);
        CPPUNIT_ASSERT_EQUAL(3, info.slaveEthNos[2]);
    }

    {
        GetBackupBondingSlaves(info);
        CPPUNIT_ASSERT_EQUAL(2, info.slaveEthCount);
        CPPUNIT_ASSERT_EQUAL(2, info.slaveEthNos[0]);
        CPPUNIT_ASSERT_EQUAL(3, info.slaveEthNos[1]);
    }
} 

void CTestGetEthBondStatus::testGetLinkStatus()
{
    EthBondingInfo info1;
    {
        bzero(&info1, sizeof(info1));
    }

    EthBondingInfo info2;
    {
        bzero(&info2, sizeof(info2));
        info2.isBondingEnabled = true;
        info2.activeEthNo = 1;
        info2.slaveEthCount = 2;
        info2.slaveEthNos[0] = 0;
        info2.slaveEthNos[1] = 2;
    }

    EthBondingInfo info3;
    {
        bzero(&info3, sizeof(info2));
        info3.isBondingEnabled = true;
        info3.activeEthNo = 0;
        info3.slaveEthCount = 2;
        info3.slaveEthNos[0] = 1;
        info3.slaveEthNos[1] = 2;
    }

    int const LINK_UP = 1;
    int const LINK_DOWN = 0;

    {
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_INACTIVE, GetLinkStatusWithBondingInfo(0, LINK_DOWN, info1));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_INACTIVE, GetLinkStatusWithBondingInfo(0, LINK_DOWN, info2));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_INACTIVE, GetLinkStatusWithBondingInfo(0, LINK_DOWN, info3));
    }

    {
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_ACTIVE, GetLinkStatusWithBondingInfo(0, LINK_UP, info1));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_STANDBY, GetLinkStatusWithBondingInfo(0, LINK_UP, info2));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_ACTIVE, GetLinkStatusWithBondingInfo(0, LINK_UP, info3));
    }

    {
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_INACTIVE, GetLinkStatusWithBondingInfo(1, LINK_DOWN, info1));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_INACTIVE, GetLinkStatusWithBondingInfo(1, LINK_DOWN, info2));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_INACTIVE, GetLinkStatusWithBondingInfo(1, LINK_DOWN, info3));
    }

    {
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_ACTIVE, GetLinkStatusWithBondingInfo(1, LINK_UP, info1));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_ACTIVE, GetLinkStatusWithBondingInfo(1, LINK_UP, info2));
        CPPUNIT_ASSERT_EQUAL((int)LAN_STATUS_STANDBY, GetLinkStatusWithBondingInfo(1, LINK_UP, info3));
    }
}

void CTestGetEthBondStatus::testGetBadBondingStatus()
{
    {
        FILE * fp = fopen(SIMULATE_BONDING_FILE.c_str(), "w");
        if (fp)
        {
            fwrite(bond0Content, 1, strlen(bond0Content), fp);
            fclose(fp);
        }
    }

    CPPUNIT_ASSERT_EQUAL(0, IsBondEnabled(BAD_SIMULATE_BONDING_FILE.c_str()));
    CPPUNIT_ASSERT_EQUAL(1, IsBondEnabled(SIMULATE_BONDING_FILE.c_str()));

    {
        unlink(SIMULATE_BONDING_FILE.c_str());
    }
}

