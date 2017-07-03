#ifndef _ConfPartyManagerLocalApi_H__
#define _ConfPartyManagerLocalApi_H__

#include "PObject.h"
#include "TaskApi.h"
#include "ConfPartyDefines.h"
#include "ConfPartyManagerApi.h"

// Api for ConfParty Manager
const WORD IPSERVICEFROMCSMNGRTOUT      = 14000;
const WORD ADD_OUT_PARTY				= 14001;


class CCommRes;

////////////////////////////////////////////////////////////////////////////
//                        CConfPartyManagerLocalApi
////////////////////////////////////////////////////////////////////////////
class CConfPartyManagerLocalApi : public CConfPartyManagerApi
{
CLASS_TYPE_1(CConfPartyManagerLocalApi,CConfPartyManagerApi )	

public: 
	CConfPartyManagerLocalApi();
	virtual ~CConfPartyManagerLocalApi();  

	virtual const char* NameOf() const { return "CConfPartyManagerLocalApi";}
	
	void AcceptUnreservedParty(const mcTransportAddress* pDestIP, DWORD confId, DWORD undefId, WORD bIsVoice, //WORD bIsSip, eSipFactoryType factoryType = eNotSipFactory, const char* pSipAddress = "", BYTE initialEncryptionValue = AUTO);
			WORD bIsSip/*, BYTE bIsMultiInvite = FALSE*/, eSipFactoryType factoryType=eNotSipFactory,
			const char* pSipAddress="", BYTE initialEncryptionValue=AUTO, const char* pH323useruserField="", WORD bIsMrcHeader = FALSE, WORD bIsWebRtcCall = FALSE, /*BYTE tmpCascadeMode=CASCADE_MODE_NONE,*/ BOOL bIsMsConfInvite = FALSE, WORD srsSessionType= 0,BOOL bIsNonCCCPAvMcu = FALSE);
	void StartMeetingRoom(DWORD confId);
	void                MovePartyToConfOrMeetingRoom(DWORD dwSourceConfId, DWORD dwPartId, ETargetConfType targetConfType, DWORD targetConfId, CCommRes* pConfProfile);
	void LobbyStartAdHocConf(DWORD dwAdHocProfileId,CCommRes* pAdHocRsrv, char* pAdHocConfName=NULL, char* pAdHocNID=NULL, BYTE isSipFactory=FALSE,char* pAdHocConfDisplayName=NULL);
	virtual void CreateDialOutParty(DWORD wMonitorConfID, WORD listId, BYTE interfaceType, char* pPartyURI, BYTE isTel=FALSE, const char* pRefferedByStr=NULL, const char* pReferUri=NULL, const char* pMSAssociatedStr=NULL);
	
	void                StartGateWayConf(DWORD confId, char* confName, char* TargetNumber = NULL);
	void AddRecordingLinkParty(DWORD conferId, BYTE rMuteVideo)	;
	void DisconnectRecordingLinkParty(DWORD conferId);
};

#endif /* _ConfPartyManagerLocalApi_H__ */


