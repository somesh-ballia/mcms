#include <iomanip>

#include "ApiProcess.h"
#include "TraceStream.h"
#include "ListenSocket.h"



CApiProcess::CApiProcess()
{
	m_CntMcmsToCsMfa = 0;
	m_CntCsMfaToMcms = 0;
}

CApiProcess::~CApiProcess()
{
}

/////////////////////////////////////////////////////////////////////
WORD CApiProcess::GetConnectionId(DWORD id)
{
	WORD connId = m_CardOrCsConnIdTable.GetConnectionId(id);
	return connId;
}

//////////////////////////////////////////////////////////////////////
void CApiProcess::UpdateCardOrCs2ConnectionId(DWORD id, WORD conId)
{
	TRACEINTO << "\nOpen new connection in : board\\cs id = " << id << "; conn id = " << conId;
	m_CardOrCsConnIdTable.UpdateCardOrCs2ConnectionId(id, conId);
}

//////////////////////////////////////////////////////////////////////
void CApiProcess::CloseConnection(const WORD conId)
{
	TRACEINTO << "\nClose connection in : conn id = " << conId;
	m_CardOrCsConnIdTable.CloseConnection(conId);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CApiProcess::DumpProcessStatistics(std::ostream& answer, CTerminalCommand & command)const
{
	const DWORD total = m_CntMcmsToCsMfa + m_CntCsMfaToMcms;
	
	answer.setf(std::ios::left,std::ios::adjustfield);	
	answer 	<< "Message's Statictics" << endl
			<< "-------------------------------" << endl
			<< std::setw(15) << "MCMS -> CS_MFA:" 	<< std::setw(7) << m_CntMcmsToCsMfa << ((double)(m_CntMcmsToCsMfa * 100)/ total) << " %" << endl
			<< std::setw(15) << "MCMS <- CS_MFA:"	<< std::setw(7) << m_CntCsMfaToMcms << ((double)(m_CntCsMfaToMcms * 100)/ total) << " %" << endl
			<< "-------------------------------" 	<< endl
			<< std::setw(15) << "TOTAL"	<< std::setw(7) << total << "100 %" << endl;
}

//////////////////////////////////////////////////////////////////////
void CApiProcess::DumpConnectionTable(std::ostream &ostr)const
{
	m_CardOrCsConnIdTable.Dump(ostr);
}

//////////////////////////////////////////////////////////////////////
void CApiProcess::TraceCardOrCs2ConnectionId()const
{
	COstrStream ostr;
	DumpConnectionTable(ostr);
	PTRACE(eLevelInfoNormal, ostr.str().c_str());
}

///////////////////////////////////////////////////////////////////////////////
COsQueue* CApiProcess::GetTxQueue(int id,bool showAssert)
{
    if(id < 0 || MAX_NUM_OF_BOARDS <= id)
    {
        PASSERTMSG (100 + id, "CApiProcess::GetTxQueue 1 - illegal Board\\CS Id, (board\\cs id + 100)");
		return NULL;
    }
	
	WORD conId = GetConnectionId(id);
	if( (DEFAULT_CONN_ID == conId)			||
			(CONN_ID_EXISTED_BEFORE == conId)	||
			(CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED == conId) ||
			(CONN_ID_EXISTED_BEFORE_IN_UPGRADE_PROCESS == conId))
	{	
		if( (DEFAULT_CONN_ID == conId) || (CONN_ID_EXISTED_BEFORE == conId) )
		{
			TraceCardOrCs2ConnectionId();
			if (showAssert == true)
			    PASSERTMSG (100 + id, "CApiProcess::GetTxQueue 2 - conId does not exist, (board\\cs id + 100)");
			if (CONN_ID_EXISTED_BEFORE == conId)
			{
				 UpdateCardOrCs2ConnectionId(id,CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED);
			}
		}
		return NULL;
	}

	CListenSocket* listenSocket = GetListenSocketTask();
    if(NULL == listenSocket)
    {
        TRACEINTO << "\nCApiProcess::GetTxQueue 3 - the listener is dead";
        return NULL;
    }
    
	COsQueue *retQ = listenSocket->GetTxMbx(conId);
	if (retQ == NULL)
	{
        TRACEINTO << "\nCApiProcess::GetTxQueue 4 - TxMbx is NULL, connection id = " << conId;
		return NULL;
	}
		
	return retQ;		
}



