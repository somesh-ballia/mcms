//============================================================================
// Name        : ParseHelper.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2009
// Description : Parse functions
//============================================================================


#if !defined(__ParseHelper_h__)
#define __ParseHelper_h__


#include <map>
#include "StructTm.h"


class ParseHelper
{
public:

	enum EParserStatus { eStatusOk = 0, eStatusNotFound, eStatusBadXml, };


	static EParserStatus CleanXmlComments(const std::string& strSrc, std::string& strDest);

	static EParserStatus FindXmlNode(std::string& strNode, const std::string& strFullXmlString,
			const std::string& strNodeName, const int nNodeIndex = 0);

	static EParserStatus GetXmlNodeValue(const std::string& strNode, std::string& strNodeValue);

	static EParserStatus GetXmlNodeName(const std::string& strNode, std::string& strNodeName);

	static EParserStatus GetXmlNodeAttributesMap(const std::string& strNode, std::map<std::string,std::string>& mapAttributes);

	static EParserStatus GetXmlNodeAttributeValue(const std::string& strNode, const std::string& strAttributeName,
			std::string& strAttributeValue);

	static EParserStatus UpdateXmlNodeValue(std::string& strNode, const std::string& strNodeValue);

	static EParserStatus UpdateXmlNodeAttribute(std::string& strNode, const std::string& strAttributeName, const std::string& strAttributeValue);

	static EParserStatus ParseDateTime(const std::string& strDateTime, CStructTm& tm);

	static EParserStatus ParseUrl(const std::string strUrl, std::string& strPrefix, std::string& strIpOrDnsName,
			std::string& strSuffix, std::string& strPort);

	static const char* StatusAsString(const EParserStatus status);

public:

	const static std::string XML_COMMENT_START_SEQ;
	const static std::string XML_COMMENT_END_SEQ;
};


#endif /* __ParseHelper_h__ */
