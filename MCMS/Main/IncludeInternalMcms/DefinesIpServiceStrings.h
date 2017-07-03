// DefinesIpServiceStrings.h

#ifndef DEFINESIPSERVICESTRINGS_H_
#define DEFINESIPSERVICESTRINGS_H_

#define MANAGEMENT_NETWORK_NAME "Management Network"
#define NUM_OF_IP_SERVICES 8

static const char *ConfigurationSipServerModeStr[] = 
{
	"auto",
	"manually"
};

static const char *TransportTypeStr[] = 
{
	"unkownTransportType",
	"udp",
	"tcp",
    "tls"
};

static const char *ServerStatusStr[] = 
{
	"auto",
	"specify",
	"off"
};

static const char *RegistrationModeStr[] = 
{
	"redirect",
	"polling",
	"move",
	"dns",
	"forking"
};

static const char *ipServiceTypeStr[] = 
{
	"ipServiceType_Signaling",   // eipServiceType_Signaling
	"ipServiceType_Management",  // eipServiceType_Management
	"ipServiceType_Control"      // eipServiceType_Control
};

static const char *ipVlanModeType[] = 
{
	"Vlan Mode 1",   // eVlanMode_1
	"Vlan Mode 2",   // eVlanMode_2
};

#endif  // DEFINESIPSERVICESTRINGS_H_
