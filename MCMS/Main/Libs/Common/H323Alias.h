//+========================================================================+
//                            H323Alias.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323Alias.h                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri A.                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 07/07/05     |                                                     |
//+========================================================================+


#if !defined(_H323Alias_H_)
#define _H323Alias_H_



#include "PObject.h"
#include "DefinesIpService.h"
#include "IpCsSizeDefinitions.h"


class CXMLDOMElement;
/////////////////////////////////////////////////////////////////////////////
//							CH323Alias
class CH323Alias : public CPObject
{
	CLASS_TYPE_1(CH323Alias,CPObject )
public:
	   //Constructors
    CH323Alias();
    CH323Alias( const CH323Alias &other );
    CH323Alias&  operator=(const CH323Alias& other);
    bool operator==(const CH323Alias& other) const;
    bool operator!=(const CH323Alias& other) const
    {
    	return !(*this == other);
    }
    
    virtual ~CH323Alias() ;
    
    virtual const char* NameOf() const { return "CH323Alias";}
    // Implementation
//    void        Serialize( WORD format, std::ostream  &m_ostr );
//    void        DeSerialize( WORD format, std::istream &m_istr );
    
    WORD        GetAliasType() const;
    void        SetAliasType(const WORD aliasType);
    const char* GetAliasName() const;
    void        SetAliasName(const char* aliasName);
	void        SetAliasDisplayName(const char* aliasDisplayName);
	const char* GetAliasDisplayName() const;
    int		    TestValidity() const;
    void        SerializeXml( CXMLDOMElement *pParentNode ) const;
    int         DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );
    
    
    // Attributes
    WORD  m_aliasType;
    char  m_aliasName[ALIAS_NAME_LEN];
	char m_aliasDisplayName[MaxDisplaySize];
	
};

#endif  // _H323Alias_H_
