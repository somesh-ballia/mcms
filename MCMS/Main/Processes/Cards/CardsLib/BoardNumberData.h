
#if !defined(_BoardNumberData_H__)
#define _BoardNumberData_H__


#include "SerializeObject.h"
#include "CardsProcess.h"

////////////////////////////////////////////////////////////////////////////////////////////
//
//CBoardNumberData
//
////////////////////////////////////////////////////////////////////////////////////////////
class CBoardNumberData : public CSerializeObject
{
CLASS_TYPE_1(CBoardNumberData, CSerializeObject)
public:

	//Constructors
	CBoardNumberData();
	virtual const char* NameOf() const { return "CBoardNumberData";}
	CBoardNumberData(const CBoardNumberData &other);
	CBoardNumberData& operator = (const CBoardNumberData& other);
	virtual ~CBoardNumberData();

	void   SerializeXml(CXMLDOMElement*& pNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement* pNode, char *pszError, const char* action);

	virtual CSerializeObject* Clone() {return new CBoardNumberData;}

    WORD  GetBoardId() {return m_boardID; };

protected:

    WORD                  m_boardID;
};
/////////////////////////////////////////////////////////////////////////////////////


#endif
