#if !defined(_H323GkStatus_H__)
#define _H323GkStatuss_H__

#include  "PObject.h"

class CXMLDOMElement;


class CH323GatekeeperStatus : public CPObject
{
CLASS_TYPE_1(CH323GatekeeperStatus,CPObject)
public:
       //Constructors
    CH323GatekeeperStatus();                   
    CH323GatekeeperStatus(const CH323GatekeeperStatus &other);
  	CH323GatekeeperStatus& operator = (const CH323GatekeeperStatus& other);
    virtual ~CH323GatekeeperStatus();
    const char*  NameOf() const;
	void   SerializeXml(CXMLDOMElement* pFatherNode);
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    void  SetGkState(const BYTE gkState);                 
    BYTE  GetGkState() const;
	void  SetReqBandwidth(const DWORD reqBandwidth);                 
    DWORD GetReqBandwidth() const;
	void  SetAllocBandwidth(const DWORD allocBandwidth);                 
    DWORD GetAllocBandwidth() const;
	void  SetRequestIntoInterval(const WORD requestIntoInterval);                 
    WORD  GetRequestIntoInterval() const;
	void  SetGkRouted(const BYTE gkRouted);                 
    BYTE  GetGkRouted() const;

	   // Attributes
	BYTE		m_gkState;
	DWORD		m_reqBandwidth;
	DWORD    	m_allocBandwidth;
	WORD 		m_requestIntoInterval;
	BYTE 		m_gkRouted;       //YES/NO
};


#endif //_H323GkStatuss_H__

