

#ifndef _CONNECTEDISDNVOICEPARTYCNTL
#define _CONNECTEDISDNVOICEPARTYCNTL

#include "IsdnPartyCntl.h"

class CConnectedIsdnVoicePartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CConnectedIsdnVoicePartyCntl, CIsdnPartyCntl)
public:
	CConnectedIsdnVoicePartyCntl();
	CConnectedIsdnVoicePartyCntl& operator= (const CConnectedIsdnVoicePartyCntl & other);
	~CConnectedIsdnVoicePartyCntl(void);
	virtual const char* NameOf() const { return "CConnectedIsdnVoicePartyCntl";}
	// Operations
	virtual void*    GetMessageMap();
	void ChangeScm();
	void OnEndAvcToSvcArtTranslatorConnectAnycase(CSegment* pParam);
	void OnAvcSvcAdditionalPartyRsrcIndAnycase(CSegment* pParam);
	void OnSetPartyAvcSvcMediaStateAnycase(CSegment* pParam);
	void OnEndAudioUpgradeToMixAvcSvcAnycase(CSegment* pParam);
	
protected:
	PDECLAR_MESSAGE_MAP
	void SetPartyConnectedWithProblemAndFaulty();
	void SendAllocateMixResourcesRequest();
	void FillAdditionalAllocPartyParams(ALLOC_PARTY_REQ_PARAMS_S &stAllocateAdditionalPartyParams);
	void AddAdditionalPartyRsrcToRsrcDescVector(ALLOC_PARTY_IND_PARAMS_S_BASE  &stAllocateAdditionalPartyIndParamsBase);
	void DumpAllocIndToTrace(ALLOC_PARTY_IND_PARAMS_S_BASE& pParam);
};

#endif //_CONNECTEDVOICEPARTYCNTL


