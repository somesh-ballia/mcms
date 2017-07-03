/*
 * WhiteList.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: stanny
 */

#include "WhiteList.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"
#include <algorithm>
#include "TraceStream.h"

CWhiteList::CWhiteList() {
	
	m_enableWhiteList = false;

}

CWhiteList::~CWhiteList() {
	// TODO Auto-generated destructor stub
	m_ipList.clear();
}
////////////////////////////////////////////////////////////////////////////
CWhiteList& CWhiteList::operator = (const CWhiteList &rOther)
{
	m_enableWhiteList = rOther.m_enableWhiteList;
	m_ipList = rOther.m_ipList;
	return *this;
}


////////////////////////////////////////////////////////////////////////////
bool CWhiteList::operator!=(const CWhiteList& other)const
{
	return (!(*this == other));
}

////////////////////////////////////////////////////////////////////////////
bool CWhiteList::operator==(const CWhiteList &rHnd)const
{
	
	if(this == &rHnd)
		return true;

	if(m_enableWhiteList != rHnd.m_enableWhiteList)
		return false;
	//check size of list;
	if(m_ipList.size() != rHnd.m_ipList.size())
		return false;
	
	//m_ipList.sort();
	///rHnd.m_ipList.sort();
	std::list<string>::const_iterator  it;
	std::list<string>::const_iterator  rl;
	
	
	for(rl = rHnd.m_ipList.begin();rl !=rHnd.m_ipList.end();rl++)
	{
		it = find(m_ipList.begin(),m_ipList.end(),*rl);
		if(it == m_ipList.end())
			return false;
	}
	
	/*for(it=m_ipList.begin(); it!=m_ipList.end(); ++it)
	{		
		bool found = false;
		string item1 = *it;
		FTRACESTR(eLevelInfoNormal) << "CWhiteList - item1 " << item1.c_str();
		for(il = rHnd.m_ipList.begin();il !=m_ipList.end();il++)
		{
			
			string item2 = *il;
			FTRACESTR(eLevelInfoNormal) << "CWhiteList - item2 " << item2.c_str();
			if(item1 == item2)
			{
				found =true;
				break;
			}
		}
		if(!found)
			return false;		
	}*/	
	return true;
}


void  CWhiteList::WriteListToSegment(std::list<std::string>& iplist , CSegment* pWhiteListSeg)
{
	std::list<string>::const_iterator  it;
	for(it=iplist.begin(); it!=iplist.end(); ++it)
	{
		std::string tmpIp = *it;
		(*pWhiteListSeg) << tmpIp;
	}
}
  
void  CWhiteList::WriteWhiteListToSegment(CSegment* pWhiteListSeg)
{
	std::list<string>::const_iterator  it;
	std::list<string> ipv4list,ipv6list;
	
	for(it=m_ipList.begin(); it!=m_ipList.end(); ++it)
	{
		std::string tmpIp = *it;
		replace(tmpIp.begin(), tmpIp.end(), '*', '0');
		if(isIpV4Str(tmpIp.c_str()))		
			ipv4list.push_back(*it);		
		else
			ipv6list.push_back(*it);
	}
	DWORD iptype = SEG_IPV4;
	if(!ipv4list.empty())
	{
		FTRACESTR(eLevelInfoNormal) << "CWhiteList - write ipv4 to segment";
		pWhiteListSeg->Put(iptype);	 
		pWhiteListSeg->Put((DWORD)ipv4list.size());
		WriteListToSegment(ipv4list,pWhiteListSeg);
	}
	iptype = SEG_IPV6;
	if(!ipv6list.empty())
	{
		FTRACESTR(eLevelInfoNormal) << "CWhiteList - write ipv6 to segment";
		pWhiteListSeg->Put(iptype);	 
		pWhiteListSeg->Put((DWORD)ipv6list.size());
		WriteListToSegment(ipv6list,pWhiteListSeg);
	}
}
void CWhiteList::Dump(ostream& msg) const
{
	msg << "WhiteList enabled =" << (int)m_enableWhiteList << "\n";
	msg <<" WhiteList ip's\n";
	std::list<string>::const_iterator  it;	
	for(it=m_ipList.begin(); it!=m_ipList.end(); ++it)
	{
		msg << *it;
	}
}

void  CWhiteList::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pIpPermissionsNode = pFatherNode->AddChildNode("IP_PERMISSIONS");									  
	pIpPermissionsNode->AddChildNode("ENABLE_WHITE_LIST",m_enableWhiteList,_BOOL);
	CXMLDOMElement* pWhiteList = pIpPermissionsNode->AddChildNode("WHITE_LIST");
	std::list<string>::const_iterator  it;	
	for(it=m_ipList.begin(); it!=m_ipList.end(); ++it)
	{
		CXMLDOMElement* pIpItem = pWhiteList->AddChildNode("IP_ITEM");
		pIpItem->AddChildNode("IP_ATTR",*it);		 
	}
}
int   CWhiteList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	CXMLDOMElement *pFatherNode = NULL;
	CXMLDOMElement *pChildNode = NULL;
	DWORD nStatus=STATUS_OK;
	GET_VALIDATE_CHILD(pActionNode,"ENABLE_WHITE_LIST",&m_enableWhiteList,_BOOL);
	if(STATUS_NODE_MISSING == nStatus)
		return nStatus;
	GET_FIRST_CHILD_NODE(pActionNode, "WHITE_LIST", pFatherNode);
	m_ipList.clear();
	 if(pFatherNode == NULL)
		 return STATUS_OK;
	 GET_FIRST_CHILD_NODE(pFatherNode, "IP_ITEM", pChildNode);
	 if(pChildNode == NULL)
	 		 return STATUS_OK;
	 
	 while (pChildNode)
	  {
		 string ipItem;
		GET_VALIDATE_CHILD(pChildNode,"IP_ATTR",ipItem,_0_TO_NEW_FILE_NAME_LENGTH);  
		m_ipList.push_back(ipItem);
	    GET_NEXT_CHILD_NODE(pFatherNode, "IP_ITEM", pChildNode);
	  }
	 
	 return STATUS_OK;
}
bool CWhiteList::IsIPRangeValid(std::string& ip)
{
	int countRangeChar = count(ip.begin(), ip.end(), '*');
	if(countRangeChar > 2)
		return false;
	//find second '.' character
	size_t pos = ip.find('.'); //first position
	pos = ip.find('.',pos+1);
	if(pos == string::npos)
		return false;
	std::string subIp = ip.substr(0,pos);
	if(subIp.find("*") != string::npos)
		return false;
	replace(ip.begin(), ip.end(), '*', '0');
	if(!isIpV4Str(ip.c_str()))		
		return false;
	return true;
}
bool CWhiteList::IsWhiteListValid()
{
	if(m_enableWhiteList && ((int)m_ipList.size() == 0))
			return false;
	
	if(MAX_WHITE_LIST_SIZE < (int)m_ipList.size() )
		return false;
	std::list<string>::const_iterator  it;
	for(it=m_ipList.begin(); it!=m_ipList.end(); ++it)
	{
		std::string tmpIp = *it;
		if(tmpIp.find('*') != string::npos)
		{			
			if(!IsIPRangeValid(tmpIp))
				return false;
		}
		
		if(!isIpV4Str(tmpIp.c_str()))
		{
			char*  cIpv6 = (char*)(tmpIp.c_str());
			mcTransportAddress tmpIPv6Addr;
			::stringToIpV6(&tmpIPv6Addr,cIpv6);
			if(isApiTaNull(&tmpIPv6Addr))
				return false;
		}
	}
	return true;
}
