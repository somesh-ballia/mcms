
#ifndef BRIDGEPARTYVIDEORELAYIN_H_
#define BRIDGEPARTYVIDEORELAYIN_H_

#include "BridgePartyMediaUniDirection.h"
#include "VideoRelayBridgePartyCntl.h"
#include "Image.h"
#include "UpdatePartyVideoRelayInitParams.h"
#include "SvcToAvcParams.h"
#include "RelayIntraData.h"

//#define TRANSLATOR_IN_CONNECTING_STATE		1
//#define TRANSLATOR_IN_DISCONNECTING_STATE	2
//#define TRANSLATOR_IN_CONNECTED_STATE		3

class CSVCToAVCTranslator;


#define SVC_TO_AVC_REQUIRED_TRANSLATED_LAYER_ID 3

class CBridgePartyVideoRelayIn : public CBridgePartyMediaUniDirection
{
  CLASS_TYPE_1(CBridgePartyVideoRelayIn, CBridgePartyMediaUniDirection)

public:
  // States definition
  	enum STATE{SETUP = (IDLE+1), CONNECTED, DISCONNECTING};
  	virtual void*	GetMessageMap() {return (void*)m_msgEntries;}

  	CBridgePartyVideoRelayIn();
  	CBridgePartyVideoRelayIn(const CBridgePartyVideoRelayIn& rOtherBridgePartyVideRelayoIn);
  	virtual                       ~CBridgePartyVideoRelayIn ();
 // 	CBridgePartyVideoRelayIn& operator =(const CBridgePartyVideoRelayIn&);
  	virtual const char*            NameOf() const   { return "CBridgePartyVideoRelayIn";}


  	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl);
	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl, const CBridgePartyMediaParams* pBridgePartyMediaParams, const char* sitename);

  	virtual void	Connect();
  	virtual void	DisConnect();

  	virtual void 	UpgradeSvcToAvcTranslator();

  	virtual BOOL	IsConnected() { return ( (m_state == CONNECTED) ? TRUE : FALSE ); }
  	virtual BOOL	IsConnecting() {return ((m_state == SETUP) ? TRUE : FALSE ); }
  	virtual BOOL	IsDisConnected() { return ( (m_state == IDLE) ? TRUE : FALSE ); }
  	virtual BOOL    IsDisconnecting() {return ( (m_state == DISCONNECTING) ? TRUE : FALSE ); }

  	virtual const CImage* GetPartyImage(void)const{ return m_pImage;}
 	virtual const CSvcToAvcParams* GetpSvcToAvcParams(void) const { return m_pSvcToAvcParams; }

  	void UpdatePartyInParams(CUpdatePartyVideoRelayInitParams* updatePartyVideoRelayInitParams);
  	void UpdateVideoParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams);


  	virtual void UpdateSelfMute(RequestPriority who, EOnOff eOnOff);


  	//from	SvcToAvcTranslator
	void SvcToAvcTranslatorConnected( EStat status );
  	void SvcToAvcTranslatorDisconnected( EStat status );
 	void UpdateImageOnTranslateSvcToAvc();
  	void SvcAvcTranslatorDecoderOutOfSync();
  	void SvcToAvcTranslatorAskEpForIntra();
  	void UpdateArtOnTranslateVideoSSRC( DWORD dwSSRC );
	//void SvcToAvcTranslatorEnded();
  	void UpdateLastReqUponTranslatorError( DWORD lastRequestId, DWORD lastReqOp);

  	//To SvcToAvcTranslator
  	void UpdateSvcToAvcTranslatorOnFirstAvcImage();
  	void UpdateSvcToAvcTranslatorOnNoAvcImageInConf();
  	CVideoOperationPointsSet* GetConfVideoOperationPointsSet()const;
  	DWORD  GetNumberOfNonRelayImages() const;
  	CBridgePartyCntl* GetBridgePartyCntlPtr()const;
	
	bool                   IsMuted() const;
	CDwordBitMask          GetMuteMask() const;
	bool                   IsMuteByOperator()  const;
	bool                   IsMuteByMCMS()  const;
	bool                   IsMuteByParty() const;
	
	void UpdateArtOnTranslateVideoSSRCAck( DWORD status );
	void UpdateMrmpStreamIsMust( DWORD dwSSRC, DWORD videoRelayInChannelHandle, BOOL bIsMustSsrc );
	void UpdateMrmpStreamIsMustAck( DWORD status );



protected:
//state machines (the common functions with same names as bridgepartyvideoIn is in case we will add common base class)
	virtual void  NullActionFunction(CSegment* pParam);

	virtual void  OnVideoBridgePartyConnect();
	virtual void  OnVideoBridgePartyConnectIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam);

	virtual void  OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsCONNECTING(CSegment* pParam);

	virtual void  OnVideoBridgePartyDisconnect(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectDISCONNECTING(CSegment* pParam);

	virtual void OnVideoBridgeAddAvcImageSETUP(CSegment* pParam);
	virtual void OnVideoBridgeAddAvcImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeAddAvcImageDISCONNECTING(CSegment* pParam);

	virtual void OnVideoBridgeLastAvcImageRemovedSETUP(CSegment* pParam);
	virtual void OnVideoBridgeLastAvcImageRemovedCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeLastAvcImageRemovedDISCONNECTING(CSegment* pParam);

	virtual void OnSvcToAvcTraslatorDisconnectedSETUP(CSegment* pParam);
	virtual void OnSvcToAvcTraslatorDisconnectedCONNECTED(CSegment* pParam);
	virtual void OnSvcToAvcTraslatorDisconnectedDISCONNECTING(CSegment* pParam);

	virtual void OnSvcToAvcTraslatorConnectedSETUP(CSegment* pParam);
	virtual void OnSvcToAvcTraslatorConnectedCONNECTED(CSegment* pParam);
	virtual void OnSvcToAvcTraslatorConnectedDISCONNECTING(CSegment* pParam);

	virtual void OnSvcToAvcTraslatorKillANYCASE(CSegment* pParam);

	virtual void OnVideoBridgeTraslatorDisconnectSETUP(CSegment* pParam);
	virtual void OnVideoBridgeTraslatorDisconnectCONNECTED(CSegment* pParam);

	virtual void OnVideoBridgeUpgradeTraslateIDLE(CSegment* pParam);
	virtual void OnVideoBridgeUpgradeTraslateSETUP(CSegment* pParam);
	virtual void OnVideoBridgeUpgradeTraslateCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeUpgradeTraslateDISCONNECTING(CSegment* pParam);


	void AddImageToConfMix();
	BOOL IsVideoMediaStreamListEmpty();
	bool SaveUpdatedVideoParams(CBridgePartyVideoRelayMediaParams*);

	//translate SVC to AVC
	bool IsTranslateSvcToAvcSupported();
	int  TranslateSvcToAvc();
	bool UpdateSvcParamsWithImageParams();
	void NewSvcToAvcTranslator();
  	bool CreateAndConnectSvcToAvcTranslator();
  	void InitSvcToAvcParams();
 	void ReplayConnectSvcToAvcTranslator( EStat status );
 	void ReplayDisconnectSvcToAvcTranslator( EStat status );

	virtual void MuteYourSelf(RequestPriority who);
	virtual void UnMuteYourSelf(RequestPriority who);
	virtual void GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap);

  	void RemoveSvcToAvcStateMachineFromRoutingTable();
  	int  InitOrUpdateSvcToAvcTranslator();
  	bool UpdateSvcToAvcTranslator();

  	void SetVideoInConnected( EStat status );
	int  UpdateSvcToAvcTranslatorImageUpdatedIfNeeded();
	void OnConnectSvcAvcTranslatorReqExistingTranslator();
	void OnConnectSvcAvcTranslatorReqNotExistingTranslator();

//members
  	CImage* 			m_pImage;
  	BOOL 				m_isReady;
  	bool 				m_bIsImageUpdated;
  	CSvcToAvcParams* 	m_pSvcToAvcParams;
  	CSVCToAVCTranslator* m_pSvcToAvcTranslator;
  	bool				m_bSvcAvcTranslatorReady;
  	bool				m_bNeedToConnectSvcAvcTranslator;
  	bool				m_bNeedToReplayConnectSvcToAvcTranslator;
  	bool				m_bNeedToReplayDisconnectSvcToAvcTranslator;

	PDECLAR_MESSAGE_MAP
};

#endif /* BRIDGEPARTYVIDEORELAYIN_H_ */
