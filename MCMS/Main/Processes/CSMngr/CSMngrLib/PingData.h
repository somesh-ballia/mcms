//+========================================================================+
//                  CsPinger.H                                             |
//		     Copyright 2009 Polycom, Inc                                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       PingData.h                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Lior                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  6-2009  | Description                                      |
//-------------------------------------------------------------------------|
//			
//+========================================================================+
#ifndef _PINGDATA_H_
#define _PINGDATA_H_

#include "SerializeObject.h"
#include "CsStructs.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "psosxml.h"

class CPingData : public CSerializeObject
{
    CLASS_TYPE_1(CPingData,CSerializeObject)
public:
    CPingData();
    CPingData (const ePingIpType ipType, const char *destination);
    CPingData (const CPingData &other);
    virtual ~CPingData() {};
    const char*   NameOf() const {return "CPingData";}
    
    void SetIpType (const ePingIpType ipType) {m_ipType = ipType;}
    void SetDestination (const char * destination) {strncpy (m_destination, destination, STR_LEN - 1); m_destination[STR_LEN - 1] = 0;}
    void SetPingId (const DWORD pingId) {m_pingId = pingId;}
    void SetPingStatus (const DWORD pingStatus) {m_pingStatus = pingStatus;}
    
    ePingIpType GetIpType () {return m_ipType;}
    char * GetDestination() {return m_destination;}
    DWORD GetPingId() {return m_pingId;}
    STATUS GetPingStatus() {return m_pingStatus;}
    void Dump (const char* title, WORD level);
    void DumpCsMsg (const char* title, WORD level);
    
    virtual void SerializeXml(CXMLDOMElement*& parentNode) const;
	virtual int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject*            Clone(){return new CPingData(*this);}
    

protected:
    char * GetPingIpTypeStr (const ePingIpType ipType);
    
    ePingIpType	m_ipType;
	char		m_destination[STR_LEN];
    DWORD       m_pingId;
    STATUS      m_pingStatus;
};


//+========================================================================+
class CPingGet : public CPingData
{
    CLASS_TYPE_1(CPingGet,CPingData)
public:
    CPingGet();
    CPingGet (const ePingIpType ipType, const char *destination);
    CPingGet (const CPingData &other);
    virtual ~CPingGet() {};
    const char*   NameOf() const {return "CPingGet";}

    virtual void SerializeXml(CXMLDOMElement*& parentNode) const;
	virtual int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject*            Clone(){return new CPingGet(*this);}
};



//+========================================================================+
class CPingSet : public CPingData
{
    CLASS_TYPE_1(CPingSet,CPingData)
        public:
    CPingSet();
    CPingSet (const ePingIpType ipType, const char *destination);
    CPingSet (const CPingData &other);
    virtual ~CPingSet() {};
    const char*   NameOf() const {return "CPingSet";}

    virtual void SerializeXml(CXMLDOMElement*& parentNode) const;
	virtual int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject*            Clone(){return new CPingSet(*this);}
};

#endif
