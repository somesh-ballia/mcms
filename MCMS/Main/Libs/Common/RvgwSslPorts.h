#ifndef RVGWSSLPORTS_H_
#define RVGWSSLPORTS_H_


#define MAX_RVGW_PORTS	7
// 8 max ports
static const char *rvgwSslPorts[] =
{
	"3050",
	"3051",
	"3052",
	"3053",
	"3054",
	"3055",
	"3056",
	"3057"
};

static const char *rvgwAliasNames[] =
{
	"rvgw1",
	"rvgw2",
	"rvgw3",
	"rvgw4",
	"rvgw5",
	"rvgw6",
	"rvgw7",
	"rvgw8"
};

const char * mapAliasToPort(const char *pAlias);
const char * mapPortToAlias(const char *pPort);
#endif
