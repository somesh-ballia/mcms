#ifndef NETSERIVESDB_H_
#define NETSERIVESDB_H_

#include <set>

#include "PObject.h"
#include "NetPortsPerService.h"
#include "IPServiceResources.h"
#include "InnerStructs.h"

class CRsrcDesc;

typedef std::set<CNetServiceRsrcs> NetServicesList;

////////////////////////////////////////////////////////////////////////////
//                        CNetServicesDB
////////////////////////////////////////////////////////////////////////////
class CNetServicesDB  : public CPObject
{
	CLASS_TYPE_1(CNetServicesDB, CPObject)

public:
	                        CNetServicesDB();
	virtual                ~CNetServicesDB();
	const char*             NameOf() const                                              { return "CNetServicesDB"; }

	void                    RTMUpdatePortRequest(UPDATE_ISDN_PORT_S* pRTMPortToUpdate, ACK_UPDATE_ISDN_PORT_S* pResult, BOOL isCalledFromUpdate = TRUE );

	STATUS                  AddDialInReservedPorts(const char* serviceName, WORD nNumOfDialIn);
	STATUS                  RemoveDialInReservedPorts(const char* serviceName, WORD nNumOfDialIn);

	NetServicesList*        GetNetServices()                                            { return &m_netServicesList; }
	const CNetServiceRsrcs* FindServiceByName(const char* name);
	CNetPortsPerService*    FindNetPortsPerServiceByName(const char* serviceName);

	STATUS                  SpanConfiguredOnService(CSpanRTM* pSpan);
	STATUS                  SpanRemoved(CSpanRTM* pSpan);

	STATUS                  GetBestSpansListPerBoard(ISDN_SPAN_PARAMS_S & isdn_params, CSpanRTM * best_spans_per_board[MAX_NUM_SPANS_ORDER], int zeroBasedBoardId);
	STATUS                  CountPortsPerBoard(ISDN_SPAN_PARAMS_S& isdn_params, DWORD& numFree, DWORD& numDialOutReserved, int zeroBasedBoardId);

	void                    SetSpanAllocationOrder(eSpanAllocation spanAllocationOrder) { m_SpanAllocationOrder = spanAllocationOrder; }
	eSpanAllocation         GetSpanAllocationOrder()                                    { return m_SpanAllocationOrder; }

private:
	NetServicesList         m_netServicesList;
	eSpanAllocation         m_SpanAllocationOrder;
};

#endif /*NETSERIVESDB_H_*/
