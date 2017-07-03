#include "MccfTxSocket.h"

#include "SocketApi.h"
#include "SecuredSocketConnected.h"

#include "MccfMsgFactory.h"

#include "MccfPackagesRegistrar.h" // get all the registered MCCF packages

#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"

#include "TraceStream.h"
#include "PrettyTable.h"

/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMccfTxSocket)
	ONEVENT(MCCF_APP_SERVER_ACK,            ANYCASE, CMccfTxSocket::OnAppServerACK)
	ONEVENT(MCCF_REQUEST_ACK,               ANYCASE, CMccfTxSocket::OnMccfRequestDone)
	ONEVENT(MCCF_IVR_PACKAGE_RESPONSE,      ANYCASE, CMccfTxSocket::OnIvrResponse)
	ONEVENT(MCCF_CDR_PACKAGE_RESPONSE,      ANYCASE, CMccfTxSocket::OnCdrResponse)
	ONEVENT(MCCF_CHANNEL_OPENED,            ANYCASE, CMccfTxSocket::OnMccfChannelOpened) // notification on MCCF channel being opened
	ONEVENT(SOCKET_WRITE,                   ANYCASE, CMccfTxSocket::NullActionFunction)
PEND_MESSAGE_MAP(CMccfTxSocket, CSocketTxTask);

/////////////////////////////////////////////////////////////////////////////
//  task creation function
void MccfTxEntryPoint(void* appParam)
{
	CSegment* pSeg = reinterpret_cast<CSegment*>(appParam);

	bool bSecured;
	*pSeg >> (BYTE&)bSecured;

	pSeg->ResetRead();

	FTRACEINTO << "Secured connection:" << bSecured;

	COsSocketConnected* pConnection = bSecured ? new CSecuredSocketConnected : new COsSocketConnected;
	CMccfTxSocket* pTxSocket = new CMccfTxSocket(pConnection);
	pTxSocket->Create(*pSeg);
}

/////////////////////////////////////////////////////////////////////////////
CMccfTxSocket::CMccfTxSocket(COsSocketConnected* pSocketDesc)
	: CSocketTxTask(false, pSocketDesc)
{
}

/////////////////////////////////////////////////////////////////////////////
CMccfTxSocket::~CMccfTxSocket()
{
	if (messages_.size())
	{
		CPrettyTable<
			size_t,
			HMccfMessage,
			MccfMethodEnum,
			size_t,
			std::string>
		t("#", "hRequest", "type", "body size", "transaction");

		t.SetCaption("Requests without ACK. Purging...");

		size_t i = 0;
		CMccfMsgFactory& f = CMccfMsgFactory::instance();

		for (Messages::iterator it = messages_.begin(); it != messages_.end(); ++it)
		{
			HMccfMessage h = *it; // must copy the handle, as std::set::iterator dereferences const data
			IMccfMessage& r = **h;

			t.Add(++i, h, r.MsgType(), r.BodyLength(), r.transaction());
			f.Release(h);
		}

		TRACEINTO << '\n' << t.Get();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMccfTxSocket::OnMccfChannelOpened(CSegment* pSeg)
{
	// forward the message AS IS to the MCCF manager task
	CSegment* pMsg = new CSegment(*pSeg);

	CManagerApi api(eProcessMCCFMngr);
	api.SendMsg(pMsg, MCCF_CHANNEL_OPENED, m_twinTask); // the answer will be sent to the RX
}

/////////////////////////////////////////////////////////////////////////////
void CMccfTxSocket::OnMccfRequestDone(CSegment* pSeg)
{
	HMccfMessage hrequest;
	*pSeg >> hrequest;

	messages_.erase(hrequest);
	TRACEINTO << "hrequest:" << hrequest;

	CMccfMsgFactory::instance().Release(hrequest);
}

/////////////////////////////////////////////////////////////////////////////
void CMccfTxSocket::OnAppServerACK(CSegment* pSeg)
{
	HMccfMessage hrequest;
	size_t timeout = 0;

	*pSeg >> hrequest >> timeout;

	messages_.insert(hrequest);

	std::string buffer;
	CMccfMsgFactory::const_instance().EncodeACK(hrequest, buffer, timeout);

	TRACEINTO << "hrequest:" << hrequest << "\n*** start ***\n" << buffer << "*** end ***";
	STATUS status = Write(buffer.c_str(), buffer.size());
}

/////////////////////////////////////////////////////////////////////////////
void CMccfTxSocket::OnIvrResponse(CSegment* pSeg)
{
	HMccfMessage hrequest;
	CMccfIvrPackage::StaticTraits::TApiBaseObject response;

	*pSeg >> hrequest >> response;

	size_t timeout = 0;
	bool isUpdateReport = false;
	DWORD seqNum = 1;

	if (hrequest)
		*pSeg >> timeout >> isUpdateReport >> seqNum;

	PASSERTSTREAM_AND_RETURN(hrequest && !*hrequest, "Multiple response for MCCF request " << hrequest << " is detected");

	const CMccfMsgFactory& f = CMccfMsgFactory::instance();
	const CMccfIvrPackage& p = CMccfIvrPackage::instance();

	std::string out;
	f.EncodeResponse(out, p, response, timeout, hrequest,isUpdateReport,seqNum);

	TRACEINTO << "hrequest:" << hrequest << "\n*** start ***\n" << out << "*** end ***";
	STATUS status = Write(out.c_str(), out.size());

	if (hrequest && !isUpdateReport)
	{
		messages_.erase(hrequest);
		f.Release(hrequest);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMccfTxSocket::OnCdrResponse(CSegment* pSeg)
{
	CMccfCdrPackage::StaticTraits::TApiBaseObject response;
	*pSeg >> response;

	const CMccfMsgFactory& f = CMccfMsgFactory::instance();
	const CMccfCdrPackage& p = CMccfCdrPackage::instance();

	std::string out;
	f.EncodeResponse(out, p, response);

	TRACEINTO << "\n*** start ***\n" << out << "*** end ***";
	STATUS status = Write(out.c_str(), out.size());
}

/////////////////////////////////////////////////////////////////////////////
