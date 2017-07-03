// RecordingLinkDBGet.h: interface for the CRecordingLinkDBGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//1/7/07		Keren			  Class for Get XML Recording Link List
//========   ==============   =====================================================================


#ifndef RECORDINGLINKDBGET_H_
#define RECORDINGLINKDBGET_H_

#include "psosxml.h"
#include "SerializeObject.h"
#include "RecordingLinkDB.h"

class CRecordingLinkDBGet : public CSerializeObject
{
CLASS_TYPE_1(CRecordingLinkDBGet, CSerializeObject)
public:

	//Constructors
	CRecordingLinkDBGet();
	CRecordingLinkDBGet(const CRecordingLinkDBGet &other);
	CRecordingLinkDBGet& operator = (const CRecordingLinkDBGet& other);
	virtual ~CRecordingLinkDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CRecordingLinkDBGet();}

	const char * NameOf() const {return "CRecordingLinkDBGet";}
	
  
protected:

	

};

#endif /*RECORDINGLINKDBGET_H_*/
