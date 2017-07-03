/*
 * SipTransAvcUpgradeToMixed.h
 *
 *  Created on: Jan 18, 2013
 *      Author: enissim
 */

#ifndef SIPTRANSAVCUPGRADETOMIXED_H_
#define SIPTRANSAVCUPGRADETOMIXED_H_

#include "SIPTransaction.h"


class CSipTransAvcUpgradeToMixed : public CSipTransaction
{
	CLASS_TYPE_1(CSipTransAvcUpgradeToMixed, CSipTransaction)
public:
	CSipTransAvcUpgradeToMixed();
	CSipTransAvcUpgradeToMixed(CTaskApp * pOwnerTask);
	virtual ~CSipTransAvcUpgradeToMixed();
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	virtual const char* NameOf() const {return "CSipTransAvcUpgradeToMixed";}
	void OnSipPartyUpgradeToMixed(CSegment* pParam);
	void OnPartyTranslatorArtsConnected();
	void OnSipPartyChannelsUpdated(CSegment* pParam);
	void OnSipEndVideoUpgradeToMix(CSegment* pParam);
protected:
	PDECLAR_MESSAGE_MAP
};

#endif /* SIPTRANSAVCUPGRADETOMIXED_H_ */
