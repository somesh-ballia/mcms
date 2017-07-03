/*
 * SipTransSvcUpgradeToMixed.h
 *
 *  Created on: Jan 18, 2013
 *      Author: enissim
 */

#ifndef SIPTRANSSVCUPGRADETOMIXED_H_
#define SIPTRANSSVCUPGRADETOMIXED_H_

#include "SIPTransaction.h"


class CSipTransSvcUpgradeToMixed : public CSipTransaction
{
	CLASS_TYPE_1(CSipTransSvcUpgradeToMixed, CSipTransaction)
public:
	CSipTransSvcUpgradeToMixed();
	CSipTransSvcUpgradeToMixed(CTaskApp * pOwnerTask);
	virtual ~CSipTransSvcUpgradeToMixed();
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	virtual const char* NameOf() const {return "CSipTransSvcUpgradeToMixed";}
	void OnSipPartyUpgradeToMixed(CSegment* pParam);
	void OnSipPartyChannelsUpdated(CSegment* pParam);
	void SendUpdateSvcChannelsToMixed(CSipComMode* pTargetMode);
	void OnSipEndVideoUpgradeToMix(CSegment* pParam);
protected:
	PDECLAR_MESSAGE_MAP
};

#endif /* SIPTRANSSVCUPGRADETOMIXED_H_ */
