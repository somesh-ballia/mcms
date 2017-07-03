// SocketRxTask.cpp

#include "SocketRxTask.h"
#include <errno.h>
#include <netinet/in.h>

#include "Trace.h"
#include "Segment.h"
#include "ObjString.h"
#include "SocketApi.h"
#include "TraceStream.h"
#include "WrappersCommon.h"
#include "StatusesGeneral.h"
#include "OsSocketConnected.h"

CSocketRxTask::CSocketRxTask(void) :
  CSocketTask(NULL)
{}

CSocketRxTask::CSocketRxTask(COsSocketConnected* pSocketDesc) :
  CSocketTask(pSocketDesc)
{}

// Virtual
const char* CSocketRxTask::NameOf(void) const
{
  return GetCompileType();
}

STATUS CSocketRxTask::Read(char* buffer,
		int len,
		int& sizeRead,
		BYTE partialRcv /* = FALSE */)

{
	STATUS res = STATUS_OK;
	if (m_connection == NULL)
	{
		TRACEINTO_HIGH_SOCKET << "m_connection is NULL TBD to be removed ";
		return STATUS_FAIL;
	}
	UnlockRelevantSemaphore(); // Added for BRIDGE-13399 
	int read_erronum = 0;
	res = m_connection->Read(buffer, len, sizeRead, *this, partialRcv);
	if (STATUS_OK != res)
    	read_erronum = errno;
	LockRelevantSemaphore(); // Added for BRIDGE-13399

	if (STATUS_OK != res)
	{
		TRACEINTO_ERR_SOCKET << "E_X_C_E_P_T_I_O_N -errornum:" << read_erronum;
		HandleDisconnect();
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

int CSocketRxTask::GetPrivateFileDescriptor()
{
  int ans = -1;
  if (m_connection != NULL)
  {
	  ans = m_connection->GetDescriptor();

	  if (ans == -1)
		  StartTimer(OPEN_SOCKET_TIMER, 5 * SECOND);
  }
  return ans;
}

void CSocketRxTask::HandlePrivateFileDescriptor()
{
	if (IsValidTimer(OPEN_SOCKET_TIMER))
		DeleteTimer(OPEN_SOCKET_TIMER);
	
	ReceiveFromSocket();
}

bool CSocketRxTask::ReadValidate_TPKT_Header(const char* bufHdr, DWORD& len) const
{
	const TPKT_HEADER_S *temp_TPKT_Header = (const TPKT_HEADER_S *)bufHdr;
	int result = ntohs(temp_TPKT_Header->payload_len) - sizeof(TPKT_HEADER_S);
	if (0 >= result ||
		  3 != temp_TPKT_Header->version_num ||
		  0 != temp_TPKT_Header->reserved)
	{
		return false;
	}

	len = result;
	return true;
}

void CSocketRxTask::OnCorruptedTPKT(const char* bufHdr, const char* msg)
{
	const TPKT_HEADER_S* hdr = (const TPKT_HEADER_S*)bufHdr;
	TRACEINTO_ERR_SOCKET << (msg ? msg : "No message") << ": " << CTPKTHeaderWrapper(*hdr);
	HandleDisconnect();
}

void CSocketRxTask::OnFailReadFromSocket(const char* msg)
{
  PASSERTMSG(true, msg);
  HandleDisconnect();
}
