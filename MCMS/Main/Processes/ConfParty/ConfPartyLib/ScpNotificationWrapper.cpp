#include "ScpNotificationWrapper.h"
#include "SipUtils.h"

/////////////////////////////////////////////////////////////////////////////
CScpPipeWrapper::CScpPipeWrapper()
{
	m_pipeId			= 0;
	m_notificationType	= (APIU32)eScpNotificationTypeUndefined;
	m_reason			= (APIU32)eStreamIsNowProvided/*0*/;	//eScpNotificationReasonsUndefined;
	m_bIsPermanent		= (APIUBOOL)false;
}

/////////////////////////////////////////////////////////////////////////////
CScpPipeWrapper::CScpPipeWrapper(const CScpPipeWrapper &other)
: CPObject(other)
{
	m_pipeId			= other.m_pipeId;
	m_notificationType	= other.m_notificationType;
	m_reason			= other.m_reason;
	m_bIsPermanent		= other.m_bIsPermanent;
}

/////////////////////////////////////////////////////////////////////////////
CScpPipeWrapper& CScpPipeWrapper::operator= (const CScpPipeWrapper &other)
{
	if (this == &other)
		return *this;

	m_pipeId			= other.m_pipeId;
	m_notificationType	= other.m_notificationType;
	m_reason			= other.m_reason;
	m_bIsPermanent		= other.m_bIsPermanent;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CScpPipeWrapper& CScpPipeWrapper::operator= (const MrmpScpPipe &other)
{
    m_pipeId            = other.unPipeId;
    m_notificationType  = other.unNotifyType;
    m_reason            = other.unReason;
    m_bIsPermanent      = other.bIsPermanent;

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CScpPipeWrapper::operator==(const CScpPipeWrapper &other) const
{
	if ( (m_pipeId				!= other.m_pipeId)				||
		 (m_notificationType	!= other.m_notificationType)	||
		 (m_reason				!= other.m_reason)				||
		 (m_bIsPermanent		!= other.m_bIsPermanent) )
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CScpPipeWrapper::InitDefaults()
{
	m_pipeId			= 0;
	m_notificationType	= (APIU32)eScpNotificationTypeUndefined;
	m_reason			= (APIU32)eStreamIsNowProvided;
	m_bIsPermanent		= (APIUBOOL)false;
}

/////////////////////////////////////////////////////////////////////////////
void CScpPipeWrapper::Dump(std::string sCaller)
{
	char dumpStr[1000];

	snprintf( dumpStr, sizeof(dumpStr) - 1,
			 "_scp_flow_ CScpPipeWrapper::Dump (caller: %s)\n\tpipeId:           %d\n\tnotificationType: %s\n\treason:           %s\n\tisPermanent:      %s",
			 sCaller.c_str(),
			 m_pipeId,
			 ::GetScpNotificationTypeEnumValueAsString( (eScpNotificationType)m_notificationType ),
			 ::GetScpNotificationReasonsEnumValueAsString( (eScpNotificationReasons)m_reason ),
			 (m_bIsPermanent ? "yes" : "no") );
	dumpStr[sizeof(dumpStr) - 1] = 0;

	PTRACE(eLevelInfoNormal, dumpStr);
}

/////////////////////////////////////////////////////////////////////////////
void  CScpPipeWrapper::Serialize(WORD format, CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg << (DWORD)m_pipeId;
	    seg << (DWORD)m_notificationType;
		seg << (DWORD)m_reason;
	    seg << (DWORD)m_bIsPermanent;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CScpPipeWrapper::DeSerialize(WORD format, CSegment& seg)
{
	DWORD dTmpIsPermanent=0;

	if(format ==  NATIVE )
	{
		seg >> (DWORD&)m_pipeId;
	    seg >> (DWORD&)m_notificationType;
		seg >> (DWORD&)m_reason;
		seg >> (DWORD&)dTmpIsPermanent;

		m_bIsPermanent = (BOOL&)dTmpIsPermanent;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CScpNotificationWrapper::CScpNotificationWrapper()
{
	m_channelHandle			= 0;
	m_sequenceNumber		= 0;
	m_remoteSequenceNumber	= 0;
	m_numOfPipes			= 0;

	m_pipes.clear();
}

/////////////////////////////////////////////////////////////////////////////
CScpNotificationWrapper::CScpNotificationWrapper(const CScpNotificationWrapper &other)
: CPObject(other)
{
	m_channelHandle			= other.m_channelHandle;
	m_sequenceNumber		= other.m_sequenceNumber;
	m_remoteSequenceNumber	= other.m_remoteSequenceNumber;
	m_numOfPipes			= other.m_numOfPipes;

	m_pipes.assign(other.m_pipes.begin(), other.m_pipes.end());
}

/////////////////////////////////////////////////////////////////////////////
CScpNotificationWrapper& CScpNotificationWrapper::operator= (const CScpNotificationWrapper &other)
{
	if (this == &other)
		return *this;

	m_channelHandle			= other.m_channelHandle;
	m_sequenceNumber		= other.m_sequenceNumber;
	m_remoteSequenceNumber	= other.m_remoteSequenceNumber;
	m_numOfPipes			= other.m_numOfPipes;

	m_pipes.clear();
	m_pipes.assign(other.m_pipes.begin(), other.m_pipes.end());

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CScpNotificationWrapper& CScpNotificationWrapper::operator= (const MrmpScpStreamsNotificationStruct &other)
{
    m_channelHandle         = other.unChannelHandle;
    m_sequenceNumber        = other.unSequenseNumber;
    m_remoteSequenceNumber  = other.unRemoteSequenseNumber;
    m_numOfPipes            = other.nNumberOfScpPipes;

    m_pipes.clear();
    CScpPipeWrapper tmpPipe;
    for (int i=0; i<m_numOfPipes; i++)
    {
        tmpPipe = other.scpPipe[i];
        m_pipes.push_back(tmpPipe);
    }

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CScpNotificationWrapper::~CScpNotificationWrapper()
{
	m_pipes.clear();
}

/////////////////////////////////////////////////////////////////////////////
bool CScpNotificationWrapper::operator==(const CScpNotificationWrapper &other) const
{
	if ( (m_channelHandle			!= other.m_channelHandle)			||
		 (m_sequenceNumber			!= other.m_sequenceNumber)			||
		 (m_remoteSequenceNumber	!= other.m_remoteSequenceNumber) 	||
		 (m_numOfPipes				!= other.m_numOfPipes)				||
		 (m_pipes.size()			!= other.m_pipes.size()) )
	{
		return false;
	}

	std::list<CScpPipeWrapper>::const_iterator itr_pipes = m_pipes.begin();
	std::list<CScpPipeWrapper>::const_iterator itr_pipes_other = other.m_pipes.begin();
	while ( itr_pipes != m_pipes.end() && itr_pipes_other != other.m_pipes.end() )
	{
		if ( !((*itr_pipes) == (*itr_pipes_other)) )
			return false;

		itr_pipes++;
		itr_pipes_other++;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CScpNotificationWrapper::InitDefaults()
{
	m_channelHandle			= 0;
	m_sequenceNumber		= 0;
	m_remoteSequenceNumber	= 0;
	m_numOfPipes			= 0;

	m_pipes.clear();
}

/////////////////////////////////////////////////////////////////////////////
void CScpNotificationWrapper::Dump(std::string sCaller)
{
	char dumpStr[1000];

	snprintf( dumpStr, sizeof(dumpStr) - 1,
			 "_scp_flow_ CScpNotificationWrapper::Dump (caller: %s)\n\tchannelHandle:   %d\n\tseqNumber:       %d\n\tremoteSeqNumber: %d\n\tnumOfPipes:      %ld",
			 sCaller.c_str(),
			 m_channelHandle,
			 m_sequenceNumber,
			 m_remoteSequenceNumber,
			 m_numOfPipes );
	dumpStr[sizeof(dumpStr) - 1] = 0;

	PTRACE(eLevelInfoNormal, dumpStr);


	if ( !m_pipes.empty() )
	{
		std::string sFullCaller = "CScpNotificationWrapper::";
		sFullCaller += sCaller;

		std::list<CScpPipeWrapper>::iterator itr = m_pipes.begin();
		while ( itr != m_pipes.end() )
		{
			(*itr).Dump(sFullCaller);
			itr++;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
void CScpNotificationWrapper::Serialize(WORD format, CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg << (DWORD)m_channelHandle;
	    seg << (DWORD)m_sequenceNumber;
	    seg << (DWORD)m_remoteSequenceNumber;
	    seg << (DWORD)m_numOfPipes;

		std::list<CScpPipeWrapper>::iterator itr = m_pipes.begin();
		while ( itr != m_pipes.end() )
		{
			(*itr).Serialize(format, seg);
			itr++;
		}
	} // end if(format ==  NATIVE )
}

/////////////////////////////////////////////////////////////////////////////
void  CScpNotificationWrapper::DeSerialize(WORD format, CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg >> (DWORD&)m_channelHandle;
	    seg >> (DWORD&)m_sequenceNumber;
		seg >> (DWORD&)m_remoteSequenceNumber;
		seg >> (DWORD&)m_numOfPipes;

		m_pipes.clear();

		CScpPipeWrapper tmpPipe;
		for (int i=0; i<m_numOfPipes; i++)
		{
			tmpPipe.DeSerialize(format, seg);
			m_pipes.push_back(tmpPipe);
		}
	} // end if(format ==  NATIVE )
}

/////////////////////////////////////////////////////////////////////////////
bool CScpNotificationWrapper::FillPipeById(APIU32 pipeId, CScpPipeWrapper& pipeToFill)
{
	std::list<CScpPipeWrapper>::iterator itr = m_pipes.begin();
	while ( itr != m_pipes.end() )
	{
		if ( (*itr).m_pipeId == pipeId )
		{
			pipeToFill = *itr;
			return true;
		}
		itr++;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CScpNotificationWrapper::AddPipe(const CScpPipeWrapper &newPipe)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CScpNotificationWrapper::AddPipe");

	m_pipes.push_back(newPipe);
	m_numOfPipes++;
}
