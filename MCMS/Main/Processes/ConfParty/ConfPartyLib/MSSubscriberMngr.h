/*
 * MSSubscriberMngr.h
 *
 *  Created on: Oct 2, 2013
 *      Author: dkrasnopolsky
 */

#ifndef MSSUBSCRIBERMNGR_H_
#define MSSUBSCRIBERMNGR_H_

#include "MSAvMCUMngr.h"

////////////////////////////////////////////////////////////////////////////
//                        CMSSubscriberMngr
////////////////////////////////////////////////////////////////////////////
class CMSSubscriberMngr: public CMSAvMCUMngr
{
public:
	              CMSSubscriberMngr();
	virtual      ~CMSSubscriberMngr();
	              CMSSubscriberMngr(const CMSSubscriberMngr &other);

	const char*   NameOf() const  { return "CMSSubscriberMngr"; }
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}

	void          Create(CRsrcParams* pRsrcParams, CConf* pConf, sipSdpAndHeadersSt* MsConfReq, DWORD PartyId, CSipNetSetup* SipNetSetup, DWORD ServiceId, char* FocusUri);
	void          Subscribe();
	void          OnSipBeNotify(CSegment* pSeg);
	void          OnTimerSubscribe(CSegment* pParam);
	void          SendSubscribe(CSipNetSetup* SipNetSetup, BYTE ToStopSubscribe);
	void          RemoveEventPackageConnection();
	void          OnSipBeNotifyDisconnect(CSegment* pParam);
	void 		  TerminateEventPackageConnection(CSipNetSetup* SipNetSetup);

	void 		OnTimerDisconnectMngr(CSegment* pParam);

	static STATUS ProcessBeNotify(CSegment* pSeg, PartyRsrcID id = INVALID);


protected:

	PDECLAR_MESSAGE_MAP
};

#endif /* MSSUBSCRIBERMNGR_H_ */
