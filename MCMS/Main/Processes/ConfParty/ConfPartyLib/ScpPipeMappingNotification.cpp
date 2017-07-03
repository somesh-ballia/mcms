#include "ScpPipeMappingNotification.h"


////////////CScpPipeMappingObj///////////////

CScpPipeMappingObj::CScpPipeMappingObj()
{
	m_pipeId = 0;
	m_csrc = 0;
}

//////////////////////////////////////////////
CScpPipeMappingObj::CScpPipeMappingObj(const CScpPipeMappingObj &other)
:CPObject(other)
{
	m_pipeId = other.m_pipeId;
	m_csrc = other.m_csrc;
}
/////////////////////////////////////////////
CScpPipeMappingObj & CScpPipeMappingObj:: operator= (const CScpPipeMappingObj &other)
{
	if (this == &other)
		return *this;

	m_pipeId = other.m_pipeId;
	m_csrc	 = other.m_csrc;
	return *this;
}
//////////////////////////////////////////////
void CScpPipeMappingObj::InitDefaults()
{
	m_pipeId = 0;
	m_csrc = 0;
}

//////////////////////////////////////////////
void CScpPipeMappingObj ::Serialize(WORD format,CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg << (DWORD)m_pipeId;
		seg << (DWORD)m_csrc;
	}
}
//////////////////////////////////////////////
void CScpPipeMappingObj::DeSerialize(WORD format,CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg >> (DWORD&)m_pipeId;
	    seg >> (DWORD&)m_csrc;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////CScpPipeMappingNotification///////////////////////////////////////////////////////

CScpPipeMappingNotification::CScpPipeMappingNotification()
{
	m_sequenceNumber		= 0;
	m_remoteSequenceNumber	= 0;

	m_listOfPipes.clear();

}

//////////////////////////////////////////////
CScpPipeMappingNotification::CScpPipeMappingNotification(const CScpPipeMappingNotification &other)
: CPObject(other)
{
	m_sequenceNumber		= other.m_sequenceNumber;
	m_remoteSequenceNumber	=other.m_remoteSequenceNumber;

	m_listOfPipes.clear();
	m_listOfPipes.assign(other.m_listOfPipes.begin(), other.m_listOfPipes.end());
}

//////////////////////////////////////////////
CScpPipeMappingNotification::~CScpPipeMappingNotification()
{
	m_listOfPipes.clear();
}

//////////////////////////////////////////////
CScpPipeMappingNotification& CScpPipeMappingNotification::operator= (const CScpPipeMappingNotification &other)
{
	if (this == &other)
			return *this;

	m_sequenceNumber		= other.m_sequenceNumber;
	m_remoteSequenceNumber	= other.m_remoteSequenceNumber;

	m_listOfPipes.clear();
	m_listOfPipes.assign(other.m_listOfPipes.begin(), other.m_listOfPipes.end());

	return *this;
}

//////////////////////////////////////////////
void CScpPipeMappingNotification::DeletePipesList()
{
//	for (std::list <CScpPipeMappingObj>::iterator itr = m_listOfPipes.begin(); itr != m_listOfPipes.end(); ++itr)
//			delete (*itr);
	m_listOfPipes.clear();
}
//////////////////////////////////////////////
void CScpPipeMappingNotification::InitDefaults()
{
	m_sequenceNumber		= 0;
	m_remoteSequenceNumber	= 0;

	DeletePipesList();
}

//////////////////////////////////////////////
void CScpPipeMappingNotification::Serialize(WORD format, CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg << (DWORD)m_sequenceNumber;
		seg << (DWORD)m_remoteSequenceNumber;

		int listSize = m_listOfPipes.size();
		seg << (DWORD)listSize;

		for (std::list <CScpPipeMappingObj>::iterator itr = m_listOfPipes.begin(); itr != m_listOfPipes.end(); ++itr)
		{
			(*itr).Serialize(format, seg);
		}
	} // end if(format ==  NATIVE )
}

//////////////////////////////////////////////
void CScpPipeMappingNotification::DeSerialize(WORD format, CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg >> (DWORD&)m_sequenceNumber;
		seg >> (DWORD&)m_remoteSequenceNumber;

		DWORD listSize;
		seg >> (DWORD&)listSize;

		DeletePipesList();
		CScpPipeMappingObj tmpPipe;
		for(DWORD i=0; i<listSize; ++i)
		{
			tmpPipe.DeSerialize(format, seg);
			m_listOfPipes.push_back(tmpPipe);
		}
	} // end if(format ==  NATIVE )
}

//////////////////////////////////////////////
void CScpPipeMappingNotification::Dump()
{
	 std::ostringstream ostr;

	 ostr << "CScpPipeMappingNotification::Dump\n";
	 ostr << "=================================\n";

	 ostr << "m_sequenceNumber       = " << m_sequenceNumber << "\n";
	 ostr << "m_remoteSequenceNumber = " << m_remoteSequenceNumber << "\n";

	 for (std::list <CScpPipeMappingObj>::iterator itr = m_listOfPipes.begin(); itr != m_listOfPipes.end(); ++itr)
	 {
		 ostr << "Pipe parameters:" << "\n";
		 ostr << "	pipeId:" << (*itr).GetPipeId() << "\n";
		 ostr << "	csrc  :" << (*itr).GetCsrc() << "\n";

	 }

	 PTRACE(eLevelInfoNormal,ostr.str().c_str());
}

//////////////////////////////////////////////
void CScpPipeMappingNotification::InitFromSourcesParams(CVideoRelaySourcesParams& rvideoSourcesRequest)
{
	//m_sequenceNumber
	m_remoteSequenceNumber = rvideoSourcesRequest.GetSeqNum();//this is the seq number of the scp request the EP sent.
	std::list <CVideoRelaySourceApi>  vidStreamList = rvideoSourcesRequest.GetVideoSourcesList();
	std::list <CVideoRelaySourceApi>::iterator itr = vidStreamList.begin();
	CScpPipeMappingObj tmpPipe;
	for(; itr!=vidStreamList.end();itr++)
	{
		DWORD pipeId = (*itr).GetPipeId();
		DWORD csrc = (*itr).GetSyncSource();
		tmpPipe.SetPipeId(pipeId);
		tmpPipe.SetCsrc(csrc);
		m_listOfPipes.push_back(tmpPipe);
	}
}

//////////////////////////////////////////////
void CScpPipeMappingNotification::RetrievePipesList(std::list <CScpPipeMappingObj> & otherList)
{
	otherList.clear();
	otherList.assign(m_listOfPipes.begin(), m_listOfPipes.end());
}
