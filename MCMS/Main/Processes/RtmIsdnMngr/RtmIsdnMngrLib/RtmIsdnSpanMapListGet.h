#ifndef RTMISDNSPANMAPLISTGET_H_
#define RTMISDNSPANMAPLISTGET_H_


#include "psosxml.h"
#include "SerializeObject.h"

class CRtmIsdnMngrProcess;

class CRtmIsdnSpanMapListGet : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnSpanMapListGet, CSerializeObject)
public:
	CRtmIsdnSpanMapListGet();
	CRtmIsdnSpanMapListGet(const CRtmIsdnSpanMapListGet &other);
	CRtmIsdnSpanMapListGet& operator = (const CRtmIsdnSpanMapListGet& other);
	virtual ~CRtmIsdnSpanMapListGet();


	virtual void  SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		      DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CRtmIsdnSpanMapListGet;}

protected:
 	CRtmIsdnMngrProcess* m_pProcess;
 };



#endif /*RTMISDNSPANMAPLISTGET_H_*/
