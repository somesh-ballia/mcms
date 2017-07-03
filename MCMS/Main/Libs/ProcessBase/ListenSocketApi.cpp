#include "ListenSocketApi.h"
#include "SystemFunctions.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" void listenSocketEntryPoint(void* appParam);

/////////////////////////////////////////////////////////////////////////////
CListenSocketApi::CListenSocketApi(SOCKET_ENTRY_POINT rx, SOCKET_ENTRY_POINT tx, WORD port, DWORD ip, const char* interface)
	: rxEntry_(rx)
	, txEntry_(tx)
	, address_()
	, interface_(interface)
	, createMode_(eTxRxConnection)
	, maxConnections_(CListenSocket::DEFAULT_MAX_NUM_CONNECTIONS)
{
	address_.ipVersion = eIpVersion4;
	address_.transportType = eTransportTypeTcp;
	address_.distribution = eDistributionUnicast;
	address_.port = port;
	address_.addr.v4.ip = ip;
}

/////////////////////////////////////////////////////////////////////////////
CListenSocketApi::CListenSocketApi(SOCKET_ENTRY_POINT rx, SOCKET_ENTRY_POINT tx, WORD port, const char* ip)
	: rxEntry_(rx)
	, txEntry_(tx)
	, address_()
	, interface_(NULL)
	, createMode_(eTxRxConnection)
	, maxConnections_(CListenSocket::DEFAULT_MAX_NUM_CONNECTIONS)
{
	address_.ipVersion = eIpVersion4;
	address_.transportType = eTransportTypeTcp;
	address_.distribution = eDistributionUnicast;
	address_.port = port;
	address_.addr.v4.ip = SystemIpStringToDWORD(ip);
}

/////////////////////////////////////////////////////////////////////////////
CListenSocketApi::CListenSocketApi(SOCKET_ENTRY_POINT rx, SOCKET_ENTRY_POINT tx, const mcTransportAddress& address, const char* interface/* = NULL*/)
	: rxEntry_(rx)
	, txEntry_(tx)
	, address_(address)
	, interface_(interface)
	, createMode_(eTxRxConnection)
	, maxConnections_(CListenSocket::DEFAULT_MAX_NUM_CONNECTIONS)
{
}

/////////////////////////////////////////////////////////////////////////////
CListenSocketApi::~CListenSocketApi()
{
}

/////////////////////////////////////////////////////////////////////////////
void  CListenSocketApi::Create(COsQueue& creatorRcvMbx)
{
	CTaskApi::Create(creatorRcvMbx);

	m_appParam
		<< (void*)rxEntry_
		<< (void*)txEntry_
		<< address_.port
		<< address_.addr.v4.ip
		<< interface_
		<< (DWORD)createMode_
		<< maxConnections_;

	LoadApp(listenSocketEntryPoint);
}

/////////////////////////////////////////////////////////////////////////////
