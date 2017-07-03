#include <ostream>
#include "PostXmlHeader.h"
#include "OperCfg.h"
#include "Segment.h"



//////////////////////////////////////////////////////////////////////
CPostXmlHeader::CPostXmlHeader()
{
    m_Len = 0;
    m_Authorization = GUEST;
    m_IsCompressed = false;
    m_IsAudit = false;
	
	
    
	m_ifNoneMatch = 0;
	  m_ConnId =0;
}

//////////////////////////////////////////////////////////////////////
CPostXmlHeader::CPostXmlHeader(DWORD len,
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
                               DWORD ifNoneMatch,DWORD ConnId)
{
    m_Len = len;
    m_Authorization = author;
    m_IsCompressed = isCompressed;
    m_IsAudit = isAudit;
    m_ifNoneMatch = ifNoneMatch;

    m_EncodingCharset = encodingCharset;
    m_WorkStation = workStation;
    m_UserName = userName;
    m_CLientIp = clientIp;
    m_transName = transName;
    m_transDesc = transDesc;
    m_transFailureDesc = transFailureDesc;
    m_ConnId =ConnId;
}

CPostXmlHeader::~CPostXmlHeader()
{
}


//////////////////////////////////////////////////////////////////////
void CPostXmlHeader::Dump(std::ostream &ostr) const
{
    ostr << "CPostXmlHeader::Dump\n"
         << m_Len << '\n'
         << m_Authorization << '\n'
         << m_IsCompressed << '\n'
         << (m_IsAudit ? "True" : "False") << '\n'
         << m_ifNoneMatch << '\n'
         << m_EncodingCharset << '\n'
         << m_WorkStation << '\n'
         << m_UserName << '\n'
         << m_CLientIp << '\n'
         << m_transName <<'\n'
    	 << m_transDesc<<'\n'
    	 << m_transFailureDesc << '\n'
    	 << m_ConnId;
}

//////////////////////////////////////////////////////////////////////
void CPostXmlHeader::Serialize(CSegment & segment)
{
    segment << m_Len;
    segment << m_Authorization;
    segment << (WORD)(m_IsCompressed ? 1 : 0);
    segment << (WORD)(m_IsAudit ? 1 : 0);
    segment << m_ifNoneMatch;
    segment << m_EncodingCharset;
    segment << m_WorkStation;
    segment << m_UserName;
    segment << m_CLientIp;
    segment << m_transName;
    segment << m_transDesc;
    segment << m_transFailureDesc;
    segment << m_ConnId;
}

//////////////////////////////////////////////////////////////////////
void CPostXmlHeader::DeSerialize(CSegment & segment)
{
    segment >> m_Len;
    segment >> m_Authorization;

    WORD tmp = 0;
    segment >> tmp;
    m_IsCompressed = (1 == tmp);

    segment >> tmp;
    m_IsAudit = (1 == tmp);
    segment >> m_ifNoneMatch;
    segment >> m_EncodingCharset;
    segment >> m_WorkStation;
    segment >> m_UserName;
    segment >> m_CLientIp;

    segment >> m_transName;
    segment >> m_transDesc;
    segment >> m_transFailureDesc;
    segment >> m_ConnId;
}
