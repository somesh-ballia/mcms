#ifndef RTMISDNSPANMAPDEL_H_
#define RTMISDNSPANMAPDEL_H_


#include "SerializeObject.h"


class CRtmIsdnSpanMapDel : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnSpanMapDel, CSerializeObject)

public:
	CRtmIsdnSpanMapDel();
	CRtmIsdnSpanMapDel(const CRtmIsdnSpanMapDel &other);
	virtual ~CRtmIsdnSpanMapDel();


	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone(){return new CRtmIsdnSpanMapDel;}

    WORD	GetBoardId() const;                 
    void	SetBoardId(const WORD boardId);   
	
    WORD	GetSpanId() const;                 
    void	SetSpanId(const WORD spanId);   

private:
	WORD m_boardId;
	WORD m_spanId;
};


#endif /*RTMISDNSPANMAPDEL_H_*/
