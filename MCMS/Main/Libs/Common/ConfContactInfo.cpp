//+========================================================================+
//                           ConfContactInfo.CPP                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfContactInfo.CPP                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date  january-2006  | Description                                |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#include <ostream>
#include <istream>
#include "ConfContactInfo.h"
#include <stdio.h>
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "Segment.h"

// ------------------------------------------------------------

CConfContactInfo::CConfContactInfo() 
{
    m_dwConfId= 0xFFFFFFFF; 
   	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		m_ContactInfo[i][0]='\0'; //against memory leak 
}
/////////////////////////////////////////////////////////////////////////////
CConfContactInfo::~CConfContactInfo()
{
}
/////////////////////////////////////////////////////////////////////////////
const CConfContactInfo&  CConfContactInfo::operator =(const CConfContactInfo & other)
{
	m_dwConfId = other.m_dwConfId;
	
	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		m_ContactInfo[i] = other.m_ContactInfo[i];

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
CConfContactInfo::CConfContactInfo(const CConfContactInfo &other):CPObject(other)
{
	*this = other;	
}
/////////////////////////////////////////////////////////////////////////////
void   CConfContactInfo::Serialize(WORD format, std::ostream &m_ostr)
{
	// assuming format = OPERATOR_MCMS
	m_ostr <<  m_dwConfId << "\n";
	
	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		m_ContactInfo[i].Serialize(format,m_ostr);
}

void   CConfContactInfo::SerializeXml(CXMLDOMElement *pParent)
{
	CXMLDOMElement *pTempNode=pParent->AddChildNode("CONTACT_INFO_LIST");
	char szNodeName[20];
	
	strcpy(szNodeName, "CONTACT_INFO");

	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	{
		if ( !(m_ContactInfo[i].IsEmpty()) )
		{
			if (i!=0)
				snprintf(szNodeName, sizeof(szNodeName), "CONTACT_INFO_%d", i+1);
			pTempNode->AddChildNode(szNodeName,m_ContactInfo[i]);
			strcpy(szNodeName, "CONTACT_INFO");
		}
	}
}

int   CConfContactInfo::DeSerializeXml(CXMLDOMElement *pParent,char* pszError)
{
	CXMLDOMElement *pNode;
	int nStatus;
	char szNodeName[20];

	strcpy(szNodeName, "CONTACT_INFO");
	GET_FIRST_CHILD_NODE(pParent, szNodeName, pNode);

	for (int i = 0; i < MAX_CONF_INFO_ITEMS; ++i)
	{
		m_ContactInfo[i].Clear();
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
			GET_VALIDATE(pNode, m_ContactInfo[i], ONE_LINE_BUFFER_LENGTH);
	}
	return 0;

}
/////////////////////////////////////////////////////////////////////////////
void   CConfContactInfo::DeSerialize(WORD format, std::istream &m_istr)
{
	// assuming format = OPERATOR_MCMS
	m_istr >> m_dwConfId;
	
	m_istr.ignore(1);
	
	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		m_ContactInfo[i].DeSerialize(format,m_istr);	
}
/////////////////////////////////////////////////////////////////////////////
void   CConfContactInfo::SetConfId(const DWORD  confId)
{
	m_dwConfId = confId;
}
/////////////////////////////////////////////////////////////////////////////
DWORD  CConfContactInfo::GetConfId()  const
{
	return m_dwConfId;
}
/////////////////////////////////////////////////////////////////////////////
void   CConfContactInfo::SetContactInfo(const char* info,int ContactNumber)
{
	if(NULL != info)
	{			
		if(ContactNumber >= 0 && ContactNumber < MAX_CONF_INFO_ITEMS)
			m_ContactInfo[ContactNumber] = info;		
	}
}
/////////////////////////////////////////////////////////////////////////////
const char*  CConfContactInfo::GetContactInfo(int ContactNumber)  const
{
	if(ContactNumber >= 0 && ContactNumber < MAX_CONF_INFO_ITEMS)
		return m_ContactInfo[ContactNumber].GetString();

	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
const char*  CConfContactInfo::NameOf() const
{
	return "CConfContactInfo";
}
