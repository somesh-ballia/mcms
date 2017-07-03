//============================================================================
// Name        : ParseHelper.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2009
// Description : Parse functions
//============================================================================


#include <iostream>
#include <sstream>


#include "DataTypes.h"
#include "ParseHelper.h"


const std::string ParseHelper::XML_COMMENT_START_SEQ = "<!--";
const std::string ParseHelper::XML_COMMENT_END_SEQ = "-->";


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Cuts all comments from XML string
//  Params: IN const string strSrc - source string
//          OUT string strDest - result
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::CleanXmlComments(const std::string& strSrc, std::string& strDest)
{
	if( strSrc.length() < 1 )
	{
		strDest = "";
		return eStatusOk;
	}

	EParserStatus retVal = eStatusOk;
	std::stringstream  strStream (std::stringstream::in | std::stringstream::out);

	std::string::size_type current_position = 0;
	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = std::string::npos;

	do
	{
		// find start of comment
		start_pos = strSrc.find(XML_COMMENT_START_SEQ,current_position);

		// no more comments
		if( start_pos == std::string::npos )
		{
			// copy substring up to end of string
			strStream << strSrc.substr(current_position);
			break;
		}

		// copy substring up to comment beginning
		strStream << strSrc.substr(current_position,start_pos-current_position);

		// find end of comment
		end_pos = strSrc.find(XML_COMMENT_END_SEQ,start_pos);
		if( end_pos == std::string::npos )
			retVal = eStatusBadXml;

		// skip to comment end
		current_position = end_pos + XML_COMMENT_END_SEQ.length();

	}
	while( start_pos != std::string::npos && retVal == eStatusOk );

	if( retVal == eStatusOk )
	{
		// if OK - copy to destination string
		strDest = strStream.str();
	}

	return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Finds XML node in XML tree
//  Params: IN const string strFullXmlString - source string
//          IN const string strNodeName - name of node for search
//          IN const int nNodeIndex - index of node for search (if strFullXmlString has few nodes with the same name )
//          OUT string strNode - result
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusNotFound if node not found
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::FindXmlNode(std::string& strNode, const std::string& strFullXmlString,
		const std::string& strNodeName, const int nNodeIndex)
{
	EParserStatus retVal = eStatusOk;

	// remove comments
	std::string strXml;
	if( eStatusOk != (retVal = CleanXmlComments(strFullXmlString,strXml) ) )
		return retVal;

	std::string::size_type current_position = 0;
	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = 0;

	int currentIndex = 0;

	while( start_pos != std::string::npos  &&  retVal == eStatusOk  &&  currentIndex <= nNodeIndex )
	{
		start_pos = strXml.find("<" + strNodeName,current_position);
		// no more fields of that type
		if( start_pos == std::string::npos )
		{
//			printf("BAD PARSING 1\n strFullXmlString=%s\n",strFullXmlString.c_str());
//			printf("BAD PARSING 2\n strNodeName=%s\n",strNodeName.c_str());
//			printf("BAD PARSING 3\n nNodeIndex=%d\n",nNodeIndex);
			retVal = eStatusNotFound;
			break;
		}

		// if node with name like required but not exactly equal: required <NodeName>, this <NodeNameType> -> skip
		char c = strXml.at(start_pos + strNodeName.length() + 1);
		if( ' ' != c && '\n' != c && '/' != c && '>' != c )
		{
			current_position = start_pos + strNodeName.length();
			continue;
		}

		// skip node if index of node less that desirable index of node
		if( currentIndex < nNodeIndex )
		{
			current_position = start_pos + strNodeName.length();
			currentIndex++;
			continue;
		}

		// if XML node has empty value, it may be like that <EmptyValuedField Attribute1="attr1_value" Attribute2="attr2_value" />
		end_pos = strXml.find(">",start_pos);
		if( end_pos == std::string::npos )
		{
			retVal = eStatusBadXml;
			break;
		}
		if( end_pos > 1  &&  strXml.at(end_pos-1) == '/')
		{
			strNode = strXml.substr(start_pos,end_pos-start_pos+1);
			break;
		}

		// find end of node
		end_pos = strXml.find("</" + strNodeName + ">",start_pos);
		if( end_pos == std::string::npos )
		{
			retVal = eStatusBadXml;
			break;
		}

		// copy value
		strNode = strXml.substr(start_pos,end_pos+strNodeName.length()+3-start_pos);
		break;
	}

	return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Gets inside value from XML string
//  Params: IN const string strNode - full node
//          OUT string strNodeValue - value of node
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::GetXmlNodeValue(const std::string& strNode, std::string& strNodeValue)
{
	EParserStatus retVal = eStatusOk;

	// remove comments
	std::string strXml;
	if( eStatusOk != (retVal = CleanXmlComments(strNode,strXml) ) )
		return retVal;


	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = 0;

	start_pos = strXml.find("<",0);
	if( start_pos == std::string::npos )
	{
		return eStatusBadXml;
	}

	// if XML node has empty value, it may be like that <EmptyValuedField Attribute1="attr1_value" Attribute2="attr2_value" />
	end_pos = strXml.find(">",start_pos);
	if( end_pos == std::string::npos )
	{
		return eStatusBadXml;
	}
	if( end_pos > 1  &&  strXml.at(end_pos-1) == '/')
	{
		strNodeValue = "";
		return eStatusOk;
	}

	// if not empty tag, move start_pos to start of value
	start_pos = end_pos + 1;

	// find end of node
	end_pos = strXml.find("</",start_pos);
	if( end_pos == std::string::npos )
	{
		return  eStatusBadXml;
	}

	// copy value
	strNodeValue = strXml.substr(start_pos,end_pos-start_pos);

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Gets name of XML node
//  Params: IN const string strNode - full node
//          OUT string strNodeName - name of node
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::GetXmlNodeName(const std::string& strNode, std::string& strNodeName)
{
	EParserStatus retVal = eStatusOk;

	// remove comments
	std::string strXml;
	if( eStatusOk != (retVal = CleanXmlComments(strNode,strXml) ) )
		return retVal;


	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = 0;

	start_pos = strXml.find("<",0);
	if( start_pos == std::string::npos )
	{
		return eStatusBadXml;
	}

	start_pos++;

	end_pos = start_pos;
	while( end_pos != std::string::npos  &&  strXml.at(end_pos) != ' ' )
		end_pos++;

	if( end_pos == std::string::npos )
		return eStatusBadXml;

	strNodeName = strXml.substr(start_pos,end_pos-start_pos);

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Gets name of XML node
//  Params: IN const string strNode - full node
//          OUT map mapAttributes - map of attributes
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::GetXmlNodeAttributesMap(const std::string& strNode, std::map<std::string,std::string>& mapAttributes)
{
	EParserStatus retVal = eStatusOk;

	// remove comments
	std::string strXml;
	if( eStatusOk != (retVal = CleanXmlComments(strNode,strXml) ) )
		return retVal;

	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = 0;

	start_pos = strXml.find("<",0);
	if( start_pos == std::string::npos )
	{
		return eStatusBadXml;
	}

	// skip node name
	while( start_pos != std::string::npos  &&  strXml.at(start_pos) != ' '  &&  strXml.at(start_pos) != '>' )
		start_pos++;

	if( start_pos == std::string::npos )
		return eStatusBadXml;

	mapAttributes.clear();

	// if found end of tag - no attributes
	if( strXml.at(start_pos) == '>' )
	{
		return eStatusOk;
	}

	// get all attributes to map
	while( start_pos != std::string::npos )
	{
		// skip all ' ' and '\n' and '\t'
		while( start_pos != std::string::npos  &&  (strXml.at(start_pos) == ' ' || strXml.at(start_pos) == '\n' || strXml.at(start_pos) == '\t' ) )
			start_pos++;

		if( start_pos == std::string::npos  ||  strXml.at(start_pos) == '>'  ||  strXml.at(start_pos) == '/' )
			break;

		end_pos = start_pos;

		// find '='
		while( end_pos != std::string::npos  &&  strXml.at(end_pos) != '=')
			end_pos++;

		if( end_pos == std::string::npos  ||  strXml.at(end_pos) != '=' )
			return eStatusBadXml;

		// get attribute name
		std::string attributeName = strXml.substr(start_pos,end_pos-start_pos);

		// get value of attribute

		// attribute value starts with \" and end with \"
		start_pos = strXml.find('\"',end_pos+1);
		if( start_pos == std::string::npos )
		{
			return eStatusBadXml;
		}
		start_pos++; // point to value, not to \"

		end_pos = strXml.find('\"',start_pos);
		if( end_pos == std::string::npos )
		{
			return eStatusBadXml;
		}

		std::string attributeValue = strXml.substr(start_pos,end_pos-start_pos);
		mapAttributes[attributeName] = attributeValue;

		start_pos = end_pos + 1;
	}

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Gets name of XML node
//  Params: IN const string strNode - full node
//          IN string strAttributeName - name of attribute for search
//          OUT string strAttributeValue - attribute's value
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusNotFound if attribute not found
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::GetXmlNodeAttributeValue(const std::string& strNode, const std::string& strAttributeName,
		std::string& strAttributeValue)
{
	EParserStatus retVal = eStatusOk;

	std::map<std::string,std::string>  mapAttributes;

	if( eStatusOk != (retVal = ParseHelper::GetXmlNodeAttributesMap(strNode,mapAttributes) ) )
		return retVal;

	if( mapAttributes.end() == mapAttributes.find(strAttributeName) )
		return eStatusNotFound;

	strAttributeValue = mapAttributes[strAttributeName];

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Updates value of XML node
//  Params: IN const string strNodeValue - new value for node
//          OUT string strNode - node to change
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//
ParseHelper::EParserStatus ParseHelper::UpdateXmlNodeValue(std::string& strNodeForUpdate, const std::string& strNodeValue)
{
	// get node name
	std::string strNodeName;
	EParserStatus status;
	if( eStatusOk != (status = ParseHelper::GetXmlNodeName(strNodeForUpdate,strNodeName) ) )
		return status;

	std::string strNode = strNodeForUpdate;

	// find first '>'
	std::string::size_type start_pos = strNode.find('>');
	if( start_pos == std::string::npos )
		return eStatusBadXml;

	// if node is empty, it may be like <XmlNodeName attribute1="att1" attribute2="att2" />
	if( start_pos < 1 )
		return eStatusBadXml;
	if( strNode.at(start_pos-1) == '/' )
	{
		// change node type:
		//     from <XmlNodeName attribute1="att1" attribute2="att2" />
		//     to   <XmlNodeName attribute1="att1" attribute2="att2">strNodeValue<\XmlNodeName>
		// 1. remove '/'
		strNode.erase(start_pos-1,1);
		if( strNode.at(start_pos-2) == ' ' )
			strNode.erase(start_pos-2,1);
		// 2. append new node value
		strNode.append(strNodeValue);
		// 3. append closing tag
		strNode.append("<\\" + strNodeName + ">");

		strNodeForUpdate = strNode;
		return eStatusOk;
	}

	// if new node is empty, change type
	//        from <XmlNodeName attribute1="att1" attribute2="att2">oldNodeValue<\XmlNodeName>
	//        to   <XmlNodeName attribute1="att1" attribute2="att2" />
	if( strNodeValue.length() < 1 )
	{
		// 1. erase remains from current position
		strNode.erase(start_pos);
		// 2. append closing tag
		strNode.append(" />");

		strNodeForUpdate = strNode;
		return eStatusOk;
	}

	start_pos++;

	// find closing tag
	std::string strClosingTag = "<\\" + strNodeName + ">";
	std::string::size_type end_pos = strNode.find_last_of(strClosingTag);
	if( end_pos == std::string::npos )
		return eStatusBadXml;

	if( end_pos < strClosingTag.length() )
		return eStatusBadXml;
	// find_last_of - makes reverse search, so, end_pos points to last char in strClosingTag string ('>')
	// move pointer to starting char of tag ('<')
	end_pos -= strClosingTag.length() - 1;

	strNode.erase(start_pos,end_pos-start_pos);
	strNode.insert(start_pos,strNodeValue);

	strNodeForUpdate = strNode;
	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Updates attribute of XML node
//  Params: IN const string strAttributeName - name of attribute to be changed
//          IN string strAttributeValue - new value of attribute
//          OUT string strNode - node to change
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusNotFound if attribute not found
//          ParseHelper::eStatusBadXml if problems with XML found
//
///////////////////////////////////////////////////////////////////////////////////////////////////
ParseHelper::EParserStatus ParseHelper::UpdateXmlNodeAttribute(std::string& strNode, const std::string& strAttributeName,
		const std::string& strAttributeValue)
{
	std::string strForUpdate = strAttributeName + "=\"";

	std::string::size_type start_pos = strNode.find(strForUpdate);
	if( start_pos == std::string::npos )
		return eStatusNotFound;
	start_pos += strForUpdate.length();

	std::string::size_type end_pos = strNode.find("\"",start_pos+1);
	if( end_pos == std::string::npos )
		return eStatusBadXml;

	strNode.erase(start_pos,end_pos-start_pos) ;
	strNode.insert(start_pos,strAttributeValue);

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Parses XML date-time to structure
//  Params: IN const string strDateTime - date-time string like 2009-09-21T14:00:00Z
//          OUT CStructTm tm - structure of date-time
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//
///////////////////////////////////////////////////////////////////////////////////////////////////
ParseHelper::EParserStatus ParseHelper::ParseDateTime(const std::string& strDateTime, CStructTm& tm)
{
	int year=0, month=0, day=0;
	int hour=0, mins=0, secs=0;

	int params = sscanf(strDateTime.c_str(),"%d-%d-%dT%d:%d:%d",&year,&month,&day,&hour,&mins,&secs);
	if( params != 6 )
		return eStatusBadXml;

	tm.m_year = year;
	tm.m_mon = month;
	tm.m_day = day;
	tm.m_hour = hour;
	tm.m_min = mins;
	tm.m_sec = secs;

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Parses URL line to parts
//  Params: IN const string strUrl - URL line
//          OUT string strPrefix - prefix of URL
//          OUT string strIpOrDnsName - IP or DNS name
//          OUT string strSuffix - suffix of URL
//          OUT string strPort - port in use
//  Return values:
//          ParseHelper::eStatusOk
//          ParseHelper::eStatusBadXml if problems with XML found
//  Working samples:
//			123.32.231.123
//			123.23.12.211:8080
//			[1234:1234::5432]:443
//			http://123.12.12.123/EWS/Exchange.asmx
//			http://123.23.12.123:443/EWS/Exchange.asmx
//			http://[1234:1234::5432]/EWS/Exchange.asmx
//			https://vsgausexch01.r13.vsg.local2/EWS/Exchange.asmx
//
////////////////////////////////////////////////////////////////////////////////////////////////////
ParseHelper::EParserStatus ParseHelper::ParseUrl(const std::string strUrl, std::string& strPrefix,
		std::string& strIpOrDnsName, std::string& strSuffix, std::string& strPort)
{
	strPrefix.clear();
	strIpOrDnsName.clear();
	strSuffix.clear();
	strPort.clear();

	if( strUrl.length() < 1 )
		return eStatusNotFound;

	const char* HTTP_PREFIX = "http://";
	const char* HTTPS_PREFIX = "https://";

	std::string strTemp = strUrl;

	std::string::size_type start_pos = std::string::npos;
	std::string::size_type end_pos = std::string::npos;

	// find "http://"
	if( std::string::npos != ( start_pos  = strTemp.find(HTTP_PREFIX) ) )
	{
		strPrefix = HTTP_PREFIX;
		strTemp = strTemp.substr(start_pos+strlen(HTTP_PREFIX));
	}
	// find "https://"
	else if( std::string::npos != (start_pos = strTemp.find(HTTPS_PREFIX) ) )
	{
		strPrefix = HTTPS_PREFIX;
		strTemp = strTemp.substr(start_pos+strlen(HTTPS_PREFIX));
	}

	// if IPv6 address - find '['
	if( '[' == strTemp.at(0) )
	{
		end_pos = strTemp.find(']');
		if( std::string::npos == end_pos )
			return eStatusBadXml;

		strIpOrDnsName = strTemp.substr(0,end_pos+1);
		strTemp = strTemp.substr(end_pos+1);
	}
	else
	{
		// if IPv4 - go till ':' or '/'
		end_pos = 0;
		for( int i=0; i<(int)strTemp.length(); i++, end_pos++ )
		{
			if( strTemp.at(end_pos) == ':' || strTemp.at(end_pos) == '/')
				break;
		}
		if( 0 == end_pos )
			return eStatusBadXml;
		strIpOrDnsName = strTemp.substr(0,end_pos);
		strTemp = strTemp.substr(end_pos);
	}

	// find port
	if( strTemp.length() > 1 && ':' == strTemp.at(0) )
	{
		end_pos = 1;
		for( int i=1; i<(int)strTemp.length(); i++, end_pos++ )
		{
			if( strTemp.at(end_pos) == '/')
				break;
		}
		strPort = strTemp.substr(1,end_pos-1);
		strTemp = strTemp.substr(end_pos);
	}

	// suffix
	strSuffix = strTemp;

	return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Gets parser status as string
//  Params: IN const ParseHelper::EParserStatus status - status to be printed
//  Return values:
//          status name as string
//
///////////////////////////////////////////////////////////////////////////////////////////////////
const char* ParseHelper::StatusAsString(const ParseHelper::EParserStatus status)
{
	switch( status )
	{
		case eStatusOk:
			return "eStatusOk";
		case eStatusNotFound:
			return "eStatusNotFound";
		case eStatusBadXml:
			return "eStatusBadXml";
	}
	return "Unknown!";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


