#ifndef _SIPSCM
#define _SIPSCM

#include "IpScm.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipCall.h"
#include "CommConf.h"




class CSipComMode: public CIpComMode
{
CLASS_TYPE_1(CSipComMode, CIpComMode )
public:
	// Constructors
	CSipComMode();
	CSipComMode(const CSipComMode& other);
	virtual ~CSipComMode();
	virtual const char* NameOf() const { return "CSipComMode";}

	// Initializations
	void Create(const CSipCall& call);
	void Create(const sipSdpAndHeadersSt& sdp, cmCapDirection eDirection, int audioIndex = 0, int videoIndex = 0, int dataIndex = 0, int contentIndex = 0);
	void Create(const CSipCaps& caps, cmCapDirection eDirection, int audioIndex = 0, int videoIndex = 0, int dataIndex = 0, int contentIndex = 0);

	void CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol,BYTE partyEncryptionMode = AUTO);

    // Operations
	CSipComMode& operator= (const CSipComMode &other);
	// virtual void SetAudioFromH320Mode(const CComMode &h320ComMode, cmCapDirection direction = cmCapReceiveAndTransmit);
	void UpdateVideoOutRateIfNeeded(const CSipComMode& rPreferredMode, DWORD vidRateTx, RemoteIdent remoteIdent, BYTE bIsMrcCall, BYTE isFecOrRedOn=FALSE);
};

#endif
