#include <ostream>
#include <stdio.h>
#include <istream>
#include "UserDefinedInfo.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "Segment.h"

////////////////////////////////////////////////////////////////////////////
//                        CUserDefinedInfo
////////////////////////////////////////////////////////////////////////////
CUserDefinedInfo::CUserDefinedInfo()
{
  m_dwConfId  = 0xFFFFFFFF;
  m_dwPartyId = 0xFFFFFFFF;
  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    m_UserDefinedInfo[i][0] = '\0'; // against memory leak

  m_AdditionalInfo[0] = '\0';
}

//--------------------------------------------------------------------------
CUserDefinedInfo::~CUserDefinedInfo()
{
}

//--------------------------------------------------------------------------
CUserDefinedInfo::CUserDefinedInfo(const CUserDefinedInfo& other) : CPObject(other)
{
  m_dwConfId  = other.m_dwConfId;
  m_dwPartyId = other.m_dwPartyId;

  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    m_UserDefinedInfo[i] = other.m_UserDefinedInfo[i];
}

//--------------------------------------------------------------------------
void CUserDefinedInfo::Serialize(WORD format, std::ostream& m_ostr)
{
  // assuming format = OPERATOR_MCMS

  m_ostr << m_dwConfId << "\n";
  m_ostr << m_dwPartyId << "\n";

  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    m_UserDefinedInfo[i].Serialize(format, m_ostr);

  m_AdditionalInfo.Serialize(format, m_ostr);
}

//--------------------------------------------------------------------------
void CUserDefinedInfo::DeSerialize(WORD format, std::istream& m_istr)
{
  // assuming format = OPERATOR_MCMS
  m_istr >> m_dwConfId;
  m_istr.ignore(1);

  m_istr >> m_dwPartyId;
  m_istr.ignore(1);

  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    m_UserDefinedInfo[i].DeSerialize(format, m_istr);

  m_AdditionalInfo.DeSerialize(format, m_istr);
}

//--------------------------------------------------------------------------
void CUserDefinedInfo::SerializeXml(CXMLDOMElement* pParent)
{
  CXMLDOMElement* pTempNode = pParent->AddChildNode("CONTACT_INFO_LIST");
  char            szNodeName[20];

  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
  {
    if (!m_UserDefinedInfo[i].IsEmpty())
    {
      if (i == 0)
        strcpy(szNodeName, "CONTACT_INFO");
      else
        snprintf(szNodeName, sizeof(szNodeName), "CONTACT_INFO_%d", i+1);

      pTempNode->AddChildNode(szNodeName, m_UserDefinedInfo[i]);
    }
  }
  if (!m_AdditionalInfo.IsEmpty())
    pTempNode->AddChildNode("ADDITIONAL_INFO", m_AdditionalInfo);
}

//--------------------------------------------------------------------------
int CUserDefinedInfo::DeSerializeXml(CXMLDOMElement* pParent, char* pszError)
{
  CXMLDOMElement* pNode;
  int  nStatus;
  char szNodeName[20];

  for (int i = 0; i < MAX_USER_INFO_ITEMS; ++i)
  {
    m_UserDefinedInfo[i].Clear();
    if (i == 0)
    {
      GET_FIRST_CHILD_NODE(pParent, "CONTACT_INFO", pNode);
    }
    else
    {
      snprintf(szNodeName, sizeof(szNodeName), "CONTACT_INFO_%d", i+1);
      GET_NEXT_CHILD_NODE(pParent, szNodeName, pNode);
    }
    if (pNode)
      GET_VALIDATE(pNode, m_UserDefinedInfo[i], ONE_LINE_BUFFER_LENGTH);
  }

  m_AdditionalInfo.Clear();
  GET_NEXT_CHILD_NODE(pParent, "ADDITIONAL_INFO", pNode);
  if (pNode)
    GET_VALIDATE(pNode, m_AdditionalInfo, ONE_LINE_BUFFER_LENGTH);

  return 0;
}

//--------------------------------------------------------------------------
void CUserDefinedInfo::SetUserDefinedInfo(const char* UserInfo, int InfoNumber)
{
  if (NULL != UserInfo)
  {
    if (InfoNumber >= 0 && InfoNumber < MAX_USER_INFO_ITEMS)
      m_UserDefinedInfo[InfoNumber] = UserInfo;
  }
}

//--------------------------------------------------------------------------
const char* CUserDefinedInfo::GetUserDefinedInfo(int InfoNumber) const
{
  if (InfoNumber >= 0 && InfoNumber < MAX_USER_INFO_ITEMS)
    return m_UserDefinedInfo[InfoNumber].GetString();

  return NULL;
}
