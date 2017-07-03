#ifndef AVCTOSVCARTTRANSLATOR_H_
#define AVCTOSVCARTTRANSLATOR_H_

#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "MplMcmsStructs.h"
#include "ConfPartyDefines.h"
#include "IsdnParty.h"
#include "IpRtpReq.h"
#include "MrcStructs.h"


class CHardwareInterface;
class CRsrcParams;

class CAvcToSvcArtTranslator : public CStateMachineValidation {
CLASS_TYPE_1(CAvcToSvcArtTranslator,CStateMachineValidation)

public:
	CAvcToSvcArtTranslator ();
	virtual ~CAvcToSvcArtTranslator ();
	virtual const char* NameOf() const { return "CAvcToSvcArtTranslator";}

	enum STATE {SETUP = (IDLE+1), CONNECTED, DISCONNECTING};

	// state machine functions
	virtual void	HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual void*	GetMessageMap();

	virtual void  Create(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S &avcToSvcArtTranslator);
	virtual void  Connect();
	virtual void  Disconnect();
	virtual BOOL  DispatchEvent(OPCODE event, CSegment* pParam = NULL);

	// others
//	bool  IsReadyToBeKilled();

public:
	// Action functions


	DWORD GetSsrc();
	PartyRsrcID GetPartyRsrcId();
	ConfRsrcID GetConfRsrcId();



protected:
	// set / get

	// other functions
	void    OnMplOpenArtAck(STATUS status);
	void    OnMplOpenRtpChannelAck(STATUS status);
	void    OnMplOpenMrmpChannelAck(CSegment* pParam, STATUS status);
	void    SendOpenRtpChannelRequest();
	void    SendOpenArtRequest();
	void    SendOpenMrmpChannelRequest();
	void    FillOpenRtpChannelStruct(TUpdatePortOpenRtpChannelReq &stOpenRtpChannel);
	void    FillMrmpOpenChannelRequestStruct(MrmpOpenChannelRequestStruct &stMrmpOpenChannel);
	void    SetConnected();
	void    CreateHardwareInterfaces(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S &avcToSvcArtTranslator);
	void    AddToRoutingTable(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S &avcToSvcArtTranslator);
	void    RemoveFromRoutingTable();
	void    SendCloseMrmpChannelRequest();
	void    FillMrmpCloseChannelRequestStruct(MrmpCloseChannelRequestStruct &stMrmpCloseChannel);
	void    OnMplCloseMrmpChannelAck(STATUS status);
	void    SendCloseArtRequest();
	void    OnMplCloseArtAck(STATUS status);


	void	OnAvcToSvcArtTranslatorConnectIDLE(CSegment* pParam);
	void	OnAvcToSvcArtTranslatorConnectSETUP(CSegment* pParam);
	void	OnAvcToSvcArtTranslatorConnectCONNECTED(CSegment* pParam);
	void	OnAvcToSvcArtTranslatorConnectDISCONNECTING(CSegment* pParam);

	void    OnMplAckSETUP(CSegment* pParam);
	void    OnMplAckIDLE(CSegment* pParam);
	void    OnMplAckCONNECTED(CSegment* pParam);
	void    OnMplAckDISCONNECTING(CSegment* pParam);

	void    OnAvcToSvcArtTranslatorOpenToutIDLE(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorOpenToutSETUP(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorOpenToutCONNECTED(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorOpenToutDISCONNECTING(CSegment* pParam);

	void    OnAvcToSvcArtTranslatorDisconnectIDLE(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorDisconnectSETUP(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorDisconnectCONNECTED(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorDisconnectDISCONNECTING(CSegment* pParam);

	void    OnAvcToSvcArtTranslatorCloseToutIDLE(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorCloseToutSETUP(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorCloseToutCONNECTED(CSegment* pParam);
	void    OnAvcToSvcArtTranslatorCloseToutDISCONNECTING(CSegment* pParam);


protected:

	PDECLAR_MESSAGE_MAP;

	CHardwareInterface          *m_pMfaInterface;
	CHardwareInterface          *m_pMrmpInterface;
	DWORD						m_ssrc;
	bool						m_bIsReadyToBeKilled;
	DWORD 						m_lastReqId; // for mcu internal problem print in case request fails / timeout
	DWORD 						m_lastReq;
	CPartyApi					*m_pPartyApi;
	APIS32                      m_channelHandle;

private:
	ConfRsrcID			        m_confRsrcId;
	PartyRsrcID                 m_partyRsrcId;
	STATUS                      m_statusOnCloseMrmpChannel;
	STATUS                      m_statusOnCloseArt;

	DISALLOW_COPY_AND_ASSIGN(CAvcToSvcArtTranslator);

};

#endif /* AVCTOSVCARTTRANSLATOR_H_ */
