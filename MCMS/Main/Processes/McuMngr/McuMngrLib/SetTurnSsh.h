// SetTurnSsh.h: interface for the CSetTurnSsh class.
//
//
//Date         Updated By         Description
//
//26/12/06	  Yuri Ratner		Used in XML transaction. 
//========   ==============   =====================================================================



#ifndef SET_TURN_SSH_H_
#define SET_TURN_SSH_H_

#include "SerializeObject.h"


class CSetTurnSsh : public CSerializeObject
{
public:
	CSetTurnSsh();
	virtual ~CSetTurnSsh();
	virtual CSerializeObject* Clone(){return new CSetTurnSsh;}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual const char* NameOf() const { return "CSetTurnSsh";}
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    bool GetTurnSsh()const{return m_IsSshOn;}
    
private:
	// disabled
	CSetTurnSsh(const CSetTurnSsh&);
	CSetTurnSsh& operator=(const CSetTurnSsh&);

    bool m_IsSshOn;
};

#endif /*SET_TURN_SSH_H_*/
