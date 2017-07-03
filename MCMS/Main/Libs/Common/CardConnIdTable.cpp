#include <iostream>
using namespace std;


#include "CardConnIdTable.h"
#include "HlogApi.h"


CCardOrCsConnIdTable::CCardOrCsConnIdTable()
{
	for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
	{
		m_cardOrCs2ConnectionId[i] = DEFAULT_CONN_ID;
		m_boardIdDisconnectionsCounter[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////
CCardOrCsConnIdTable::~CCardOrCsConnIdTable()
{
}

//////////////////////////////////////////////////////////////////////
WORD CCardOrCsConnIdTable::GetConnectionId(DWORD id)
{
	WORD result = DEFAULT_CONN_ID;
	if(id < MAX_NUM_OF_BOARDS)
	{
		result = m_cardOrCs2ConnectionId[id];
	}

	else // illegal boardId
	{
		PASSERTMSG(id + 100, "cs\board id is not legal(error core is cs\boardId + 100)");
	}

	return result;
}

//update the CsId and CS connection for CSApi
//			 BoardId and card connection for MplApi
//////////////////////////////////////////////////////////////////////
void CCardOrCsConnIdTable::UpdateCardOrCs2ConnectionId(DWORD id, WORD newConId)
{
	if(id < MAX_NUM_OF_BOARDS)
	{
		// check that it's a reconnection after the sockect was disconnected
		WORD curConId = m_cardOrCs2ConnectionId[id];
		if ( (CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED != newConId) &&	// ('CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED' is not a reconnection)
		     ( (CONN_ID_EXISTED_BEFORE == curConId) || (CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED == curConId) ) ) // (socket was disconnected)
		{
			CHlogApi::SocketReconnect(id);
		}

		m_cardOrCs2ConnectionId[id] = newConId;
	}

	else // illegal boardId
	{
		PASSERTMSG(id + 100, "cs\board id is not legal(error core is cs\boardId + 100)");
	}
}

//////////////////////////////////////////////////////////////////////
void CCardOrCsConnIdTable::CloseConnection(const WORD conId)
{
	for(DWORD id = 0 ; id < MAX_NUM_OF_BOARDS ; id++)
	{
		if( conId == m_cardOrCs2ConnectionId[id] )
		{
			m_cardOrCs2ConnectionId[id] = CONN_ID_EXISTED_BEFORE;
			CHlogApi::SocketDisconnect(id);

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CCardOrCsConnIdTable::CloseConnectionNoFault(const WORD conId)
{
	for(DWORD boardId = 0 ; boardId < MAX_NUM_OF_BOARDS ; boardId++)
	{
		if( conId == m_cardOrCs2ConnectionId[boardId] )
		{
			m_cardOrCs2ConnectionId[boardId] = CONN_ID_EXISTED_BEFORE_IN_UPGRADE_PROCESS;
			CHlogApi::SocketDisconnectNoFault(boardId);

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CCardOrCsConnIdTable::Dump(std::ostream &ostr) const
{
	ostr << "Board\\Cs Id to Connection Id table ";

	ostr << "(Default = " << DEFAULT_CONN_ID
	     << ", ExistedBefore = " << CONN_ID_EXISTED_BEFORE
	     << ", ExistedBeforeAndSomeoneTriedToConnect = " << CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED << ")"
	     << endl ;

	ostr << "BID" << "\t\t" << "CID" << endl; 
	for(int id = 0 ; id < MAX_NUM_OF_BOARDS ; id++)
	{
		ostr << id << "\t\t" << m_cardOrCs2ConnectionId[id] << endl ;
	}
}
