#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include "IceMMTransferProcessor.h"


bool IceMMTransferProcessor::Init(CMplMcmsProtocol &mplMsg)
{
	SendForMplApi(mplMsg);
	return true;
};

bool IceMMTransferProcessor::MakeOffer(CMplMcmsProtocol &mplMsg, int &sessID)
{
	SendForMplApi(mplMsg);
	return true;
};

bool IceMMTransferProcessor::MakeAnswer(CMplMcmsProtocol &mplMsg, int &sessID)
{
	SendForMplApi(mplMsg);
	return true;
};

bool IceMMTransferProcessor::MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg)
{
	SendForMplApi(mplMsg);
	return true;
};

bool IceMMTransferProcessor::MakeOfferNoCreate(CMplMcmsProtocol &mplMsg)
{
	SendForMplApi(mplMsg);
	return true;
};

bool IceMMTransferProcessor::CloseSession(CMplMcmsProtocol &mplMsg)
{
	SendForMplApi(mplMsg);
	return true;
};

bool IceMMTransferProcessor::ProcessAnswer(CMplMcmsProtocol &mplMsg)
{
	SendForMplApi(mplMsg);
	return true;
};

#endif	//__DISABLE_ICE__
