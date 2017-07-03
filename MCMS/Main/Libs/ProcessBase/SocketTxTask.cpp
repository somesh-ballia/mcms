#include "SocketTxTask.h"
#include "OsSocketConnected.h"

#include "StatusesGeneral.h"

#include "TraceStream.h"

CSocketTxTask::CSocketTxTask(bool dropBlocked/* = false*/, COsSocketConnected* pSocketDesc/* = NULL*/)
	: CSocketTask(pSocketDesc)
	, m_dropBlocked(dropBlocked)
{}

STATUS CSocketTxTask::Write(const char* buffer, size_t size)
{
	if (!m_connection)
	{
		TRACEINTO_HIGH_SOCKET << "m_connection is NULL!";
		return STATUS_FAIL;
	}
	if (!buffer)
	{
		TRACEINTO_HIGH_SOCKET << "buffer is NULL!";
		return STATUS_FAIL;
	}

	UnlockRelevantSemaphore();
	STATUS res = m_connection->Write(buffer, size, m_dropBlocked);
	LockRelevantSemaphore();

	if (res != STATUS_OK)
	{
		HandleDisconnect();
		return STATUS_FAIL;
	}

	return STATUS_OK;
}
