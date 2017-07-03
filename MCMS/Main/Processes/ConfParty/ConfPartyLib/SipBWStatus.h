#if !defined(_CSIPBandwidthAllocationStatus_H__)
#define _CSIPBandwidthAllocationStatus_H__

#include  "PObject.h"

class CXMLDOMElement;


class CSIPBandwidthAllocationStatus : public CPObject
{
CLASS_TYPE_1(CSIPBandwidthAllocationStatus,CPObject)
public:
       //Constructors
	CSIPBandwidthAllocationStatus();
	CSIPBandwidthAllocationStatus(const CSIPBandwidthAllocationStatus &other);
	CSIPBandwidthAllocationStatus& operator = (const CSIPBandwidthAllocationStatus& other);
    virtual ~CSIPBandwidthAllocationStatus();
    const char*  NameOf() const;
	void   SerializeXml(CXMLDOMElement* pFatherNode);
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void  SetReqBandwidth(const DWORD reqBandwidth);
    DWORD GetReqBandwidth() const;
	void  SetAllocBandwidth(const DWORD allocBandwidth);
    DWORD GetAllocBandwidth() const;

	   // Attributes
	DWORD		m_reqBandwidth;
	DWORD    	m_allocBandwidth;
};


#endif //_CSIPBandwidthAllocationStatus_H__

