//////////////////////////////////////////////////////////////////////
//
// EpSimSipSubscription.h: SIP subscriptions / notifications classes.
//
//////////////////////////////////////////////////////////////////////

#ifndef __EPSIMSIPSUBSCRIPTION_H__
#define __EPSIMSIPSUBSCRIPTION_H__


#include "PObject.h"




const int MAX_SIP_SIBSCRIPTIONS = 50;

class CTaskApi;
class CMplMcmsProtocol;
class CCommSetAddSubscription;
class CCommSetGetNotification;


/////////////////////////////////////////////////////////////////////////////
class CEpSimSipSubscription : public CPObject
{
CLASS_TYPE_1(CEpSimSipSubscription,CPObject)

public:
	CEpSimSipSubscription();
	CEpSimSipSubscription(const char* pszConfName,const char* pszSubscriberName,const char* pszEvent);
	virtual const char* NameOf() const { return "CEpSimSipSubscription";}
	virtual ~CEpSimSipSubscription();


	const char* GetConfName() const { return m_szConfName; }
	const char* GetSubscriberName() const { return m_szSubscriberName; }
	const char* GetEvent() const { return m_szEvent; }
	const char* GetLastNotification() const { return m_pszSubscrLastNotif; }
	void SetLastNotification(const char* pszNotification);

protected:
		// data members
	char	m_szConfName[H243_NAME_LEN];
	char	m_szSubscriberName[H243_NAME_LEN];
	char	m_szEvent[H243_NAME_LEN];
	char*	m_pszSubscrLastNotif;
};


/////////////////////////////////////////////////////////////////////////////
class CEpSimSipSubscriptionsMngr : public CPObject
{
CLASS_TYPE_1(CEpSimSipSubscriptionsMngr,CPObject)
public:
	CEpSimSipSubscriptionsMngr();
	virtual ~CEpSimSipSubscriptionsMngr();

	virtual const char* NameOf() const { return "CEpSimSipSubscriptionsMngr";}

	void Init(CTaskApi* pCsApi);

	STATUS ProcessBatchRequest(CCommSetAddSubscription* pAddSubscriptionSet);
	STATUS ProcessBatchRequest(CCommSetGetNotification* pGetNotificationSet);
	STATUS ProcessCsRequest(CMplMcmsProtocol* pMplMcmsProt);

	void DumpList(const DWORD traceLevel=eLevelInfoNormal) const;
	void SendSipSubscribeInd(const CEpSimSipSubscription* pSubscr,const WORD expires=3600) const;
	DWORD CreateSipHeadersBuffer(BYTE** ppHeadersBuffer,const DWORD rejectStatus,
			const char* pszConfName,const char* pszSubscriberName, const char* pszEvent) const;

protected:
		// Utilities
	STATUS CsSipNotificationMsg(CMplMcmsProtocol* pMplMcmsProt);

protected:
		// data members
	CTaskApi*    			m_pCsApi;
	CEpSimSipSubscription*	m_paSubscriptions[MAX_SIP_SIBSCRIPTIONS];
};





#endif /* __EPSIMSIPSUBSCRIPTION_H__ */







