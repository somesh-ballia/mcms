//+========================================================================+
//                            H323Alias.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323Alias.cpp                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri A.                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 07/07/05     |                                                     |
//+========================================================================+


#include "H323Alias.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "Transactions.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "ObjString.h"


/////////////////////////////////////////////////////////////////////////////
//						   CH323Alias

const char* InvalidDisplayChars = "#,;:/<>\0";


CH323Alias::CH323Alias()
{
    m_aliasType = PARTY_H323_ALIAS_H323_ID_TYPE;
    m_aliasName[0] = '\0';
}

////////////////////////////////////////////////////////////////////////////
CH323Alias::CH323Alias( const CH323Alias &other):CPObject(other)
{
    m_aliasType = other.m_aliasType;
    strncpy(m_aliasName,other.m_aliasName,ALIAS_NAME_LEN);
}

////////////////////////////////////////////////////////////////////////////
CH323Alias& CH323Alias::operator=(const CH323Alias& other)
{
    m_aliasType = other.m_aliasType;
    strncpy(m_aliasName,other.m_aliasName,ALIAS_NAME_LEN);
    
    return *this;
}
////////////////////////////////////////////////////////////////////////////
bool CH323Alias::operator==(const CH323Alias& other) const
{
	return ( (m_aliasType == other.m_aliasType) &&
			 (0 == strncmp(m_aliasName, other.m_aliasName, ALIAS_NAME_LEN)) );
}


////////////////////////////////////////////////////////////////////////////
CH323Alias::~CH323Alias()
{
}

/*
////////////////////////////////////////////////////////////////////////////
void CH323Alias::Serialize( WORD format, std::ostream  &m_ostr )
{
    m_ostr << m_aliasType  << "\n";
    m_ostr << m_aliasName  << "\n";
}

////////////////////////////////////////////////////////////////////////////
void CH323Alias::DeSerialize( WORD format, std::istream &m_istr )
{
    m_istr >> m_aliasType;
    m_istr.ignore(1);
    m_istr.getline( m_aliasName, ALIAS_NAME_LEN+1, '\n');
}
*/

/////////////////////////////////////////////////////////////////////////////
WORD  CH323Alias::GetAliasType () const
{
    return m_aliasType;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Alias::SetAliasType(const WORD aliasType)
{
    m_aliasType = aliasType;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CH323Alias::GetAliasName() const
{
    return m_aliasName;
}

/////////////////////////////////////////////////////////////////////////////
void   CH323Alias::SetAliasName(const char* aliasName)
{
    strncpy(m_aliasName, aliasName, sizeof(m_aliasName) - 1);
    m_aliasName[sizeof(m_aliasName) -1]='\0';

    //replace invalid characters in the name
    for (int i=0; InvalidDisplayChars[i] != '\0'; i++)
    {
        CObjString::ReplaceChar(m_aliasName, InvalidDisplayChars[i],'.');
    }
}

/////////////////////////////////////////////////////////////////////////////
void   CH323Alias::SetAliasDisplayName(const char* aliasDisplayName)
{
 	memset(m_aliasDisplayName,0,sizeof(m_aliasDisplayName));
    strncpy(m_aliasDisplayName, aliasDisplayName, sizeof(m_aliasDisplayName) - 1);
    m_aliasDisplayName[sizeof(m_aliasDisplayName) -1]='\0';
	
}
/////////////////////////////////////////////////////////////////////////////
const char*  CH323Alias::GetAliasDisplayName() const
{
    return m_aliasDisplayName;
}

/////////////////////////////////////////////////////////////////////////////
void CH323Alias::SerializeXml( CXMLDOMElement *pParentNode) const
{
	if(pParentNode!=NULL)
	{
		CXMLDOMElement *pTempNode=pParentNode->AddChildNode("ALIAS");
		pTempNode->AddChildNode("NAME",m_aliasName);
		pTempNode->AddChildNode("ALIAS_TYPE",m_aliasType,ALIAS_TYPE_ENUM);
	}
}

///////////////////////////////////////////////////////////////////////////
int CH323Alias::DeSerializeXml( CXMLDOMElement *pActionNode,char *pszError )
{
	int nStatus;
    GET_VALIDATE_ASCII_CHILD(pActionNode,"NAME",m_aliasName,ALIAS_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"ALIAS_TYPE",&m_aliasType,ALIAS_TYPE_ENUM);
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
int CH323Alias::TestValidity() const
{
    int status = STATUS_OK;

	// if alias type is none alias name must be empty
	if( m_aliasType == 0 && strlen(m_aliasName) != 0)
		return STATUS_INVALID_ALIAS;

	// validity check for E164 and Party number types
	if( m_aliasType == PARTY_H323_ALIAS_E164_TYPE || m_aliasType == PARTY_H323_ALIAS_PARTY_NUMBER_TYPE)
        for(WORD i=0; i< strlen(m_aliasName); i++)
        {	
			 if( (m_aliasName[i] > '9' ||  m_aliasName[i] < '0') && 
				  m_aliasName[i] != '*' && m_aliasName[i] != '#' )
				return STATUS_ALIAS_E164_TYPE_NAMES_MUST_BE_ALPHA_NUMERIC;
        }
    // validity check for email id type    
 	if(  m_aliasType == PARTY_H323_ALIAS_EMAIL_ID_TYPE) {
 		// space found
		if( strchr(m_aliasName,' ') ) 
	   		return STATUS_ILLEGAL_EMAIL_ID_ALIAS;	   		
		if( strchr(m_aliasName,'@') ) 
		// '@' FOUND
		{
			// check if has more '@' or begin with '@'
	    	if(  strchr(m_aliasName,'@') != strrchr(m_aliasName,'@') || 
	    		m_aliasName[0] == '@'  || m_aliasName[strlen(m_aliasName)-1] == '@')
 	      		return STATUS_ILLEGAL_EMAIL_ID_ALIAS;	  
		}
	  	else return STATUS_ILLEGAL_EMAIL_ID_ALIAS;
 	}

	// general validity checking
    if( strstr(m_aliasName,",") )
    	return STATUS_ALIAS_NAMES_CANNOT_INCLUDE_A_COMMA;
    if( strstr(m_aliasName,";") )
        return STATUS_ALIAS_NAMES_CANNOT_INCLUDE_SEMICOLON;
	if( strstr(m_aliasName,"'") || strstr(m_aliasName,"\"") ) 
    	return STATUS_ALIAS_NAMES_CANNOT_INCLUDE_QUOTATION_MARK;

    return status;
}

