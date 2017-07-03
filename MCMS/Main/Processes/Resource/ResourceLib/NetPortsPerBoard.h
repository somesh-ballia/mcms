#ifndef NETPORTSPERBOARD_H_
#define NETPORTSPERBOARD_H_


#include "PObject.h"
#include "IPServiceResources.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "InnerStructs.h"


class CRsrcDesc;
class CSpanRTM;

class CNetPortsPerBoard : public CPObject
{
	CLASS_TYPE_1(CNetPortsPerBoard,CPObject)
public:
	CNetPortsPerBoard();
	virtual ~CNetPortsPerBoard();
    const char * NameOf() const {return "CNetPortsPerBoard";}
    
    STATUS SpanConfiguredOnService(CSpanRTM* pSpan);
    STATUS SpanRemoved(CSpanRTM* pSpan);
    
    STATUS CountTotalPorts(DWORD &numFree, DWORD &numDialOutReserved);
    
    void GetBestSpansList( CSpanRTM* spanList[MAX_NUM_SPANS_ORDER]);
    
private:
	CSpanRTM* GetSpan(WORD unitId);
	CSpanRTM* m_RTMSpans[MAX_NUM_SPANS_ORDER];
	
	int GetSpanIndexFromSpanId(int spanid) {return spanid-1;}

};

#endif /*NETPORTSPERBOARD_H_*/
