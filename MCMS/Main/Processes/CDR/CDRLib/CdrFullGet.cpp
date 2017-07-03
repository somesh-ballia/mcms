#include "CdrFullGet.h"

#include "psosxml.h"
#include "CDRDetal.h"
#include "CDRShort.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "TraceStream.h"

CCdrFullGet::CCdrFullGet(void) :
    m_confID(0xFFFFFFFF),
    m_partID(CCdrShort::kFilePartIndexUndefined),
    m_no_partID(false),
    m_cdr_long(NULL)
{ }

// Virtual
CCdrFullGet::~CCdrFullGet(void)
{
	if (m_cdr_long != NULL)
	{
		delete m_cdr_long;
	}
}

// Virtual
const char* CCdrFullGet::NameOf(void) const
{
    return GetCompileType();
}

// Virtual
CSerializeObject* CCdrFullGet::Clone(void)
{
    CCdrFullGet* ret = new CCdrFullGet;
    ret->m_confID = m_confID;
    ret->m_partID = m_partID;
    ret->m_no_partID = m_no_partID;
    ret->m_cdr_long = m_cdr_long ? new CCdrLongStruct(*m_cdr_long) : NULL;

    return ret;
}

void CCdrFullGet::SetCDRLong(CCdrLongStruct* cdr_long)
{
    m_cdr_long = cdr_long;
}

void CCdrFullGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    PASSERTMSG_AND_RETURN(NULL == m_cdr_long, "Illegal flow");
    m_cdr_long->SerializeXml(pFatherNode, m_no_partID);
}

int CCdrFullGet::DeSerializeXml(CXMLDOMElement *pActionNode,
                                char *pszError,
                                const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "ID", &m_confID, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "FILE_PART_INDEX", &m_partID, _1_TO_DWORD);
	
	// Comes API that doesn't support File Part Index
	if (STATUS_NODE_MISSING == nStatus)
	{
	    m_no_partID = true;

	    // API that doesn't support File Part Index always looks for first index
	    m_partID = CCdrShort::kFilePartIndexFirst;

	    // Fixes status to OK, otherwise the error returns to a client
	    nStatus = STATUS_OK;
	}
	else
	{
	    m_no_partID = false;
	}

	return nStatus;
}

DWORD CCdrFullGet::GetConfId(void) const
{
	return m_confID;
}

DWORD CCdrFullGet::GetFilePartIndex(void) const
{
    return m_partID;
}
