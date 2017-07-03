#ifndef __GET_ETH_BOND_STATUS_H__
#define __GET_ETH_BOND_STATUS_H__

enum
{
    MAX_NIC_COUNT = 8
};

struct EthBondingInfo
{
    int isBondingEnabled;
    int activeEthNo;
    int slaveEthCount;
    int slaveEthNos[MAX_NIC_COUNT];
};

int GetEthBondingInfo(EthBondingInfo & info);
int GetLinkStatusWithBondingInfo(int lanNo, int isLinkUp, EthBondingInfo const & info);

#endif

