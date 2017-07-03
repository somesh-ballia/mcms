#ifndef _POST_XML_HEADER
#define _POST_XML_HEADER

#include <string>
using namespace std;

#include "PObject.h"

class CSegment;


class CPostXmlHeader : public CPObject
{
    CLASS_TYPE_1(CPostXmlHeader, CPObject)
public:
    CPostXmlHeader();
    CPostXmlHeader(DWORD len,
                   WORD author,
                   bool isCompressed,
                   const string & encodingCharset,
                   const string & workStation,
                   const string & userName,
                   const string & clientIp,
                   bool isAudit,
                   const string & transName,
                   const string & transDesc,
                   const string & transFailureDesc,
                   DWORD ifNoneMatch,DWORD ConnId);
				   
				   
    virtual ~CPostXmlHeader();
    virtual const char* NameOf() const { return "CPostXmlHeader";}
	virtual void Dump(std::ostream&) const;

    void Serialize(CSegment & segment);
    void DeSerialize(CSegment & segment);

    DWORD GetLen()const{return m_Len;}
    void SetLen(DWORD len){m_Len = len;}

    WORD GetAuthorization()const{return m_Authorization;}
    void SetAuthorization(WORD author){m_Authorization = author;}

    bool GetIsCompressed()const{return m_IsCompressed;}
    void SetIsCompressed(bool isCompressed){m_IsCompressed = isCompressed;}

    const string& GetEncodingCharset()const{return m_EncodingCharset;}
    void Set(const string &charset){m_EncodingCharset = charset;}

    void SetWorkStation(const string & station){m_WorkStation = station;}
    const string& GetWorkStation()const{return m_WorkStation;}

    void SetUserName(const string & user){m_UserName = user;}
    const string& GetUserName()const{return m_UserName;}

    void SetClientIp(const string & clientIp){m_CLientIp = clientIp;}
    const string& GetClientIp()const{return m_CLientIp;}

    void SetIsAudit(bool isAudit){m_IsAudit = isAudit;}
    bool GetIsAudit()const{return m_IsAudit;}

    void SetTransName(const string & name) { m_transName = name; }
    const string& GetTransName() const { return m_transName; }

    void SetTransDesc(const string & desc) { m_transDesc = desc; }
    const string& GetTransDesc() const { return m_transDesc; }

    void SetTransFailureDesc(const string & desc) { m_transFailureDesc = desc; }
    const string& GetTransFailureDesc() const { return m_transFailureDesc; }

    void SetConnId(const DWORD & conn) { m_ConnId = conn; }
    const DWORD& GetConnId() const { return m_ConnId; }
   
    DWORD GetIfNoneMatch()const{return m_ifNoneMatch;}
    void SetIfNoneMatch(DWORD ifNoneMatch){m_ifNoneMatch = ifNoneMatch;}

private:
    CPostXmlHeader(const CPostXmlHeader&);
    CPostXmlHeader&operator=(const CPostXmlHeader&);



    DWORD m_Len;
    WORD m_Authorization;
    bool m_IsCompressed;
    bool m_IsAudit;
    DWORD  m_ifNoneMatch;

    string m_EncodingCharset;
    string m_WorkStation;
    string m_UserName;
    string m_CLientIp;
    string m_transName;
    string m_transDesc;
    string m_transFailureDesc;
    DWORD  m_ConnId;
};





#endif // _POST_XML_HEADER
