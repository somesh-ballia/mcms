/*
 * CSvcPartyIndParamsWrapper.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: asilver
 */

#include "SvcPartyIndParamsWrapper.h"
#include "SipUtils.h"

/////////////////////////////////////////////////////////////////////////////
CSvcPartyIndParamsWrapper::CSvcPartyIndParamsWrapper()
{
    m_SsrcIds.m_ssrcAudio = 0;
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
    {
        m_SsrcIds.m_ssrcContent[i] = 0;
    }
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
    {
        m_SsrcIds.m_ssrcVideo[i] = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
CSvcPartyIndParamsWrapper& CSvcPartyIndParamsWrapper::operator= (const CSvcPartyIndParamsWrapper &other)
{
    m_SsrcIds.m_ssrcAudio = other.m_SsrcIds.m_ssrcAudio;
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
    {
        m_SsrcIds.m_ssrcContent[i] = other.m_SsrcIds.m_ssrcContent[i];
    }
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
    {
        m_SsrcIds.m_ssrcVideo[i] = other.m_SsrcIds.m_ssrcVideo[i];
    }

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSvcPartyIndParamsWrapper& CSvcPartyIndParamsWrapper::operator= (const SVC_PARTY_IND_PARAMS_S &other)
{
    m_SsrcIds.m_ssrcAudio = other.m_ssrcAudio;
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
    {
        m_SsrcIds.m_ssrcContent[i] = other.m_ssrcContent[i];
    }
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
    {
        m_SsrcIds.m_ssrcVideo[i] = other.m_ssrcVideo[i];
    }

    return *this;
}


/////////////////////////////////////////////////////////////////////////////
void  CSvcPartyIndParamsWrapper::Serialize(CSegment& seg) const
{
    seg << m_SsrcIds.m_ssrcAudio;
    for (int i=0;i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
    {
        seg << m_SsrcIds.m_ssrcContent[i];
    }
    for (int i=0;i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
    {
        seg << m_SsrcIds.m_ssrcVideo[i];
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CSvcPartyIndParamsWrapper::DeSerialize(CSegment& seg)
{

    seg >> m_SsrcIds.m_ssrcAudio;
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
    {
        seg >> m_SsrcIds.m_ssrcContent[i];
    }
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
    {
        seg >> m_SsrcIds.m_ssrcVideo[i];
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CSvcPartyIndParamsWrapper::Print(const char *title) const
{
    CLargeString cstr;

    cstr << title << "\n m_ssrcAudio:" << m_SsrcIds.m_ssrcAudio;

    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
    {
        cstr << "\n m_ssrcContent[" <<  i+1 << "]:" << m_SsrcIds.m_ssrcContent[i];
    }
    for (int i=0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
    {
        cstr << "\n m_ssrcVideo[" <<  i+1 << "]:" << m_SsrcIds.m_ssrcVideo[i];
    }
    PTRACE(eLevelInfoNormal,cstr.GetString());
}

DWORD CSvcPartyIndParamsWrapper::GetSsrcId(int ind, cmCapDataType aDataType)
{
    DWORD ssrc = 0;
    if (aDataType == cmCapAudio)
        ssrc = m_SsrcIds.m_ssrcAudio;
    else if (aDataType == cmCapVideo)
        ssrc = m_SsrcIds.m_ssrcVideo[ind];

    return ssrc;
}
