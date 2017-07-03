#include "MccfRxSocket.h"
#include "SecuredSocketConnected.h"

#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"

#include "MCCFMngrManager.h"

#include "MccfPackageFactory.h"
#include "MccfMsgFactory.h"
#include "ApiBaseObject.h"

#include "ManagerApi.h"

#include "SysConfigKeys.h"
#include "ConfigHelper.h"

#include "TraceStream.h"
#include "PrettyTable.h"

#include <memory>
#include <netinet/in.h>
#include <arpa/inet.h>

/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMccfRxSocket)
	ONEVENT(SOCKET_WRITE,             ANYCASE, CMccfRxSocket::NullActionFunction)
	ONEVENT(MCCF_CHANNEL_SYNC,        ANYCASE, CMccfRxSocket::OnChannelSync)
	ONEVENT(MCCF_CHANNEL_DROP,        ANYCASE, CMccfRxSocket::OnChannelDrop)
	ONEVENT(MCCF_MESSAGE_QUEUE_TIMER, ANYCASE, CMccfRxSocket::OnMessageQueueTimer)
PEND_MESSAGE_MAP(CMccfRxSocket, CSocketRxTask);

/////////////////////////////////////////////////////////////////////////////
//  task creation function
void MccfRxEntryPoint(void* appParam)
{
	CSegment* pSeg = reinterpret_cast<CSegment*>(appParam);

	bool bSecured;
	*pSeg >> (BYTE&)bSecured;

	pSeg->ResetRead();

	FTRACEINTO << "Secured connection:" << bSecured;

	COsSocketConnected* pConnection = bSecured ? new CSecuredSocketConnected : new COsSocketConnected;
	CMccfRxSocket* pRxSocket = new CMccfRxSocket(pConnection);
	pRxSocket->Create(*pSeg);
}

/////////////////////////////////////////////////////////////////////////////
CMccfRxSocket::CMccfRxSocket(COsSocketConnected* pSocketDesc)
	: CSocketRxTask(pSocketDesc)
	, context_(*this)
{
}

/////////////////////////////////////////////////////////////////////////////
CMccfRxSocket::~CMccfRxSocket()
{
	TRACEINTO << "AppServerID:" << AppServerID();

	CSegment* pSeg = new CSegment;
	*pSeg << AppServerID();

	CManagerApi api(eProcessMCCFMngr);
	api.SendMsg(pSeg, MCCF_CHANNEL_CLOSED);

	if (!messages_.empty())
	{
		CPrettyTable<
			size_t,
			HMccfMessage,
			MccfMethodEnum,
			size_t,
			std::string>
		t("#", "hRequest", "type", "body size", "transaction");

		t.SetCaption("Requests to be dispatched. Cleaning...");

		CMccfMsgFactory& f = CMccfMsgFactory::instance();

		for (size_t i = 0; !messages_.empty(); messages_.pop())
		{
			HMccfMessage& hrequest = messages_.front().hrequest;
			IMccfMessage& r = **hrequest;

			t.Add(i, hrequest, r.MsgType(), r.BodyLength(), r.transaction());
			f.Release(hrequest);
		}

		TRACEINTO << '\n' << t.Get();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::OnChannelSync(CSegment* pSeg)
{
	TRACEINTO << context_;

	// TODO: move here dispatching the SYNC request to respective subscribers
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::OnChannelDrop(CSegment* pSeg)
{
	TRACEINTO << context_;
	HandleDisconnect();
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::OnMessageQueueTimer(CSegment* /*pSeg*/)
{
	if (m_selfKill)
		return;

	const time_t now = time(NULL);
	const double delay = GetSystemCfgFlagInt<DWORD>(CFG_KEY_WAIT_BEFORE_DISPATCH_MCCF);

	for ( ; !messages_.empty(); messages_.pop())
	{
		if (difftime(now, messages_.front().stamp) >= delay)
			DispatchMessage(messages_.front().hrequest);
		else
			break;
	}

	if (!messages_.empty())
	{
		const double delta = difftime(now, messages_.front().stamp);
		StartTimer(MCCF_MESSAGE_QUEUE_TIMER, static_cast<DWORD>((delay - delta) * SECOND));
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::OnSync() const
{
	CSegment* pSeg = new CSegment;

	mcTransportAddress addr;

	addr.ipVersion = eIpVersion4; // TODO: add support for IPv6
	addr.addr.v4.ip = GetRemoteIp();
	addr.port = 0;

	*pSeg << AppServerContext(reinterpret_cast<AppServerContext::AppServerID>(AppServerID()), addr, context_.dialogID());

	// send the message to the twin, so that it will forward the message to the manager with the correct response mailbox
	SendMessageToTwin(pSeg, MCCF_CHANNEL_OPENED);
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::ReceiveFromSocket()
{
	const size_t BUF_SIZE = 1024;

	std::string buffer;
	buffer.reserve(BUF_SIZE);

	if (!ReadHeader(buffer))
	{
		if (!m_selfKill)
		{
			PASSERTSTREAM(true, "Cannot read");
			HandleDisconnect();
		}

		return;
	}

	MccfErrorCodesEnum status = mec_OK;

	// NOTE: this message will be deleted later in the flow upon response sending
	HMccfMessage hrequest = CMccfMsgFactory::const_instance().CreateHeader(context_, buffer.c_str(), buffer.size(), status);

	if (hrequest)
	{
		IMccfMessage* request = *hrequest;
		TRACEINTO << "hrequest:" << hrequest << " @" << request << ", status:" << status;

		if (ReadMessage(*request))
			EnqueueMessage(hrequest);
		else
			DispatchMessage(hrequest);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CMccfRxSocket::ReadMessage(IMccfMessage& request)
{
	const size_t size = request.BodyLength();

	if (size)
	{
		char* buffer = new char[size + 1];
		int read = 0;

		Read(buffer, size, read, false); // read the whole message at once

		if (static_cast<size_t>(read) == size)
		{
			buffer[size] = 0;
			request.ParseBody(buffer, size);
		}
		else
			PASSERTSTREAM(!m_selfKill, "Read:" << read);

		delete [] buffer;
	}

	return size;
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::EnqueueMessage(const HMccfMessage& hrequest)
{
	StampedMessage req = { time(NULL), hrequest };
	messages_.push(req);

	TRACEINTO << "hrequest:" << hrequest;

	if (!IsValidTimer(MCCF_MESSAGE_QUEUE_TIMER))
		StartTimer(MCCF_MESSAGE_QUEUE_TIMER, GetSystemCfgFlagInt<DWORD>(CFG_KEY_WAIT_BEFORE_DISPATCH_MCCF) * SECOND);
}

/////////////////////////////////////////////////////////////////////////////
bool CMccfRxSocket::DispatchMessage(HMccfMessage& hrequest) const
{
	PASSERT_AND_RETURN_VALUE(!hrequest, false);

	IMccfMessage& request = **hrequest;

	SendACK(hrequest);

	CSegment* seg = new CSegment;

	bool ok = (request.status() == mec_OK);

	if (ok)
	{
	    struct in_addr tIn;
        tIn.s_addr= htonl(GetRemoteIp());
        char * pIp = static_cast<char*>(inet_ntoa(tIn));
        
		*seg << AppServerID() << hrequest << pIp;

		TRACEINTO << "Going to dispatch message request:" << hrequest << " from " << pIp;
		ok = request.Dispatch(seg, *m_twinTask);
	}

	if (!ok)
	{
		seg->Reset();
		*seg << hrequest;

		SendMessageToTwin(seg, MCCF_REQUEST_ACK);
	}

	return ok;
}

/////////////////////////////////////////////////////////////////////////////
void CMccfRxSocket::SendACK(const HMccfMessage& hrequest) const
{
	CSegment* pSeg = new CSegment;

	const size_t timeout = 15; // TODO: the actual time-out value is to be put here
	*pSeg << hrequest << timeout;

	SendMessageToTwin(pSeg, MCCF_APP_SERVER_ACK);
}

/////////////////////////////////////////////////////////////////////////////
// Reads the whole header, while translating every new line (in either Unix / Mac or Windows style) to \0
bool CMccfRxSocket::ReadHeader(std::string& buffer)
{
	char data = 0;
	int read = 0;

	static const std::string SIGNATURE = "CFW ";

	STATUS status = STATUS_OK;

	while (buffer.size() < SIGNATURE.size())
	{
		status = Read(&data, sizeof(data), read);

		if (STATUS_OK != status)
			break;

		buffer += data;
	}

	bool signatureOk = (buffer == SIGNATURE);
	PASSERTSTREAM_AND_RETURN_VALUE(!signatureOk && !m_selfKill, "Unexpected Signature:" << buffer, false);

	bool bCR = false; // previous symbol was a CR ('\r')
	bool bLF = false; // previous symbol was a LF ('\n')
	bool bCRLF = false; // previous 2 symbols were CR LF pair

	const char CR = '\r';
	const char LF = '\n';

	const char EOL = '\0';

	for ( ; ; )
	{
		status = Read(&data, sizeof(data), read);

		if (STATUS_OK != status)
			break;

		if (LF == data)
		{
			if (bLF)
				break; // stop reading on double LF

			if (bCR)
			{
				if (bCRLF)
					break; // stop reading on double CR LF
			}
			else // translate every pair of CR LF to EOL
				buffer += EOL;

			bCRLF = bCR;

			bLF = true;
			bCR = false;
		}
		else if (CR == data)
		{
			if (bCR)
				break; // stop reading on double CR

			if (!bCRLF)
				buffer += EOL; // translate CR into EOL, but in case of when it is a pair of CR LF

			bCR = true;
			bLF = false;
			// !! do NOT clear here the bCRLF
		}
		else
		{
			buffer += data;
			bCR = bLF = bCRLF = false;
		}
	}

	TRACEINTO
		<< "AppServerID:" << AppServerID()
		<< ", signature is OK:" << BOOL2STR(signatureOk)
		<< ", status:" << status << "\n";

	return signatureOk && STATUS_OK == status;
}

/////////////////////////////////////////////////////////////////////////////

