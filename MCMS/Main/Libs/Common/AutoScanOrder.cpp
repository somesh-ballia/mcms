//+========================================================================+
//                            AutoScanOrder.cpp                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       AutoScanOrder.H                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#include <ostream>
#include <istream>
#include "AutoScanOrder.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "Segment.h"
#include "ObjString.h" 



// ------------------------------------------------------------

CAutoScanOrder::CAutoScanOrder()
{
	m_autoScanPartiesOrder = new AutoScanMap;
}

/////////////////////////////////////////////////////////////////////////////
CAutoScanOrder::CAutoScanOrder(const CAutoScanOrder &other)
:CPObject(other)
{
	m_autoScanPartiesOrder = new AutoScanMap;
	*this=other;
}

/////////////////////////////////////////////////////////////////////////////
CAutoScanOrder::~CAutoScanOrder()
{
	PDELETE(m_autoScanPartiesOrder);
}
/////////////////////////////////////////////////////////////////////////////
const char*  CAutoScanOrder::NameOf() const
{
	return "CAutoScanOrder";
}

/////////////////////////////////////////////////////////////////////////////
CAutoScanOrder&  CAutoScanOrder::operator = (const CAutoScanOrder& other)
{
	if ( &other == this ) return *this;
	
	if (other.m_autoScanPartiesOrder)
		*m_autoScanPartiesOrder = *other.m_autoScanPartiesOrder;
	
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
void CAutoScanOrder::DeSerialize(WORD format, CSegment& seg)
{
	if (format != NATIVE) PASSERT(1);
	//seg.DumpHex();
	CIstrStream istr (seg);
	//PTRACE2(eLevelInfoNormal,"********CAutoScanOrder::DeSerialize istr: ", istr.str().c_str());
	DeSerialize(format, istr);
}
/////////////////////////////////////////////////////////////////////////////
void CAutoScanOrder::Serialize(WORD format, CSegment& seg)
{
	if (format != NATIVE) PASSERT(1);

	COstrStream pOstr;
	Serialize(format, pOstr);
	//PTRACE2(eLevelInfoNormal,"********CAutoScanOrder::Serialize pOstr: ", pOstr.str().c_str());
	pOstr.Serialize(seg);
	//seg.DumpHex();
}
/////////////////////////////////////////////////////////////////////////////
void  CAutoScanOrder::Serialize(WORD format, std::ostream &ostr)
{
	DWORD size = m_autoScanPartiesOrder->size();
	ostr << size << "\n";
	AutoScanMap::iterator it;
			
	for(it = m_autoScanPartiesOrder->begin();it != m_autoScanPartiesOrder->end();++it)
	{
		ostr << (DWORD)it->first << "\n";
		ostr << (DWORD)it->second << "\n";
	}
	
			
}

/////////////////////////////////////////////////////////////////////////////
void  CAutoScanOrder::DeSerialize(WORD format, std::istream &istr)
{
	DWORD size=0;
    DWORD tmpOrder=0, tmpId=0;
	
    istr >> size;
    for (DWORD i = 0;i < size;i++)
    {
       istr >> tmpOrder;
       istr >> tmpId;
       (*m_autoScanPartiesOrder)[tmpOrder]=tmpId;
    }
}

/////////////////////////////////////////////////////////////////////////////
int CAutoScanOrder::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pPartyOrderNode;
	int maxIndex = 0;
	int order;
	DWORD monitorPartyId;
	
	GET_FIRST_CHILD_NODE(pFatherNode, "PARTY_ORDER", pPartyOrderNode);

	while (pPartyOrderNode)  //no need to check ranges (done with AddCell)
	{
		GET_VALIDATE_MANDATORY_CHILD(pPartyOrderNode,"ID",&monitorPartyId,_0_TO_DWORD);
		GET_VALIDATE_MANDATORY_CHILD(pPartyOrderNode,"ORDER",&order,_0_TO_DWORD);
			
		maxIndex = max(order,maxIndex);
			
		(*m_autoScanPartiesOrder)[order]=monitorPartyId;
		
		GET_NEXT_CHILD_NODE(pFatherNode, "PARTY_ORDER", pPartyOrderNode);
	}

	// this check is to verify that there are no "holes" in the order (i.e: 1,2,3,5)
	if (m_autoScanPartiesOrder->size() && (maxIndex != (int)m_autoScanPartiesOrder->size() - 1))
		PASSERT(345);
	
	Dump();
	
	return nStatus;
}
/////////////////////////////////////////////////////////////////////////////
void CAutoScanOrder::SerializeXml(CXMLDOMElement *pFatherNode)
{
	CXMLDOMElement* pAutoScanOrderNode = pFatherNode->AddChildNode("AUTO_SCAN_ORDER");
	CXMLDOMElement* pPartyOrderNode;
	
	AutoScanMap::iterator it;
	for(it = m_autoScanPartiesOrder->begin();it != m_autoScanPartiesOrder->end();++it)
	{
		pPartyOrderNode =  pAutoScanOrderNode->AddChildNode("PARTY_ORDER");
		pPartyOrderNode->AddChildNode("ORDER",it->first,_0_TO_DWORD);
		pPartyOrderNode->AddChildNode("ID",it->second,_0_TO_DWORD);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CAutoScanOrder::Dump() const
{
	CLargeString cstr;
	AutoScanMap::iterator it;
	cstr << " order    partyId\n";
	cstr << " ----------------\n";
	for(it = m_autoScanPartiesOrder->begin();it != m_autoScanPartiesOrder->end();++it)
		cstr << "   " << it->first << "       " << it->second << "\n";
		
	PTRACE(eLevelInfoNormal,cstr.GetString());
}

//////////////////////////////////////////////////////////////////////
void CAutoScanOrder::RemoveItem(int monitorPartyid)
{
	AutoScanMap::iterator it;
	for(it = m_autoScanPartiesOrder->begin();it != m_autoScanPartiesOrder->end();++it)
	{
		if(monitorPartyid == it->second)
			m_autoScanPartiesOrder->erase(it->first);
	}

}
