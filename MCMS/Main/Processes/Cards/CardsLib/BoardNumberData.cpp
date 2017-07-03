
#include "BoardNumberData.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"

////////////////////////////////////////////////////////////////////////////////////////////
//
//CBoardNumberData
//
////////////////////////////////////////////////////////////////////////////////////////////

CBoardNumberData::CBoardNumberData()
{
   m_boardID = 0;
}

/////////////////////////////////////////////////////////////////////////////
CBoardNumberData& CBoardNumberData:: operator =(const CBoardNumberData &other)
{
  m_boardID = other.m_boardID;
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
CBoardNumberData::CBoardNumberData(const CBoardNumberData &other)
    : CSerializeObject()
{
	m_boardID =other.m_boardID;

	*this=other;
}

/////////////////////////////////////////////////////////////////////////////
CBoardNumberData::~CBoardNumberData()
{
}

/////////////////////////////////////////////////////////////////////////////

int CBoardNumberData::DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pNode,"BOARD_NUMBER", &m_boardID, BOARD_ENUM);
	return nStatus;
}


///////////////////////////////////////////////////////////////////////////
void CBoardNumberData::SerializeXml(CXMLDOMElement*& pNode) const
{
	pNode->AddChildNode("BOARD_NUMBER",m_boardID,BOARD_ENUM);
}

//////////////////////////////////////////////////////////////////////////


