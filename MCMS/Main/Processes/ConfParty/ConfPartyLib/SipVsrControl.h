/*
 * SipVsrControl.h
 *
 *  Created on: Jul 7, 2013
 *      Author: abental
 */

#ifndef SIPVSRCONTROL_H_
#define SIPVSRCONTROL_H_

#include "StateMachine.h"
#include "TaskApp.h"
#include "IpMfaOpcodes.h"
#include "IpPartyMonitorDefinitions.h"
#include "HardwareInterface.h"
#include "IpCommonDefinitions.h"

class CPartyApi;

//=============================================================
// SIP VSR states (idle implied/inherited from CStateMachine)
//=============================================================
const WORD sVSR_INIT	= 110;
const WORD sVSR_READY	= 111;

//====================================
// SIP VSR constants and definitions
//====================================
#define MS_SVC_PT_RTV	121
#define MS_SVC_PT_H264	122
#define NUMERIC_NULL	(-1)

enum __aspectRatioEnumeration
{
	MS_SVC_FOUR_BY_THREE 	= 1,
	MS_SVC_SIXTEEN_BY_NINE	= 2,
	MS_SVC_ONE_BY_ONE		= 4,
	MS_SVC_THREE_BY_FOUR	= 8,
	MS_SVC_NINE_BY_SIXTEEN	= 16,
	MS_SVC_TWENTY_BY_THREE	= 32
};


////// VSR structure required by party, external API..
	typedef struct
	{
					DWORD 	payload_type; //SVC or RTV
					DWORD 	aspect_ratio; //from decision matrix
					DWORD 	max_width; //from decision matrix
					DWORD 	max_height; //from decision matrix
					DWORD 	frame_rate; // enum: by msft RTP
					DWORD 	min_bitrate; //msft RTP: take care of units
					DWORD   maxNumOfPixels;
					DWORD   num_of_must;
					DWORD   num_of_may;
					APIU16  qualityReportHistogram[VSR_NUM_OF_QUALITY_LEVELS];
	} ST_VSR_PARAMS;

	typedef struct
	{
					DWORD sender_ssrc; // RA
					DWORD partyId;
					DWORD msi; // source any on P2P
					DWORD key_frame; // bool: default YES
					DWORD num_vsrs_params; //1 for svc/rtv only, 2 - (deafault) rtv or svc
					ST_VSR_PARAMS st_vsrs_params[VSR_MAX_ENTRIES];
	} ST_VSR_SINGLE_STREAM;

	typedef struct
	{
					DWORD 					num_vsrs_streams;
					ST_VSR_SINGLE_STREAM 	st_vsrs_single_stream[MAX_STREAM_LYNC_2013_CONN];

	} ST_VSR_MUTILPLE_STREAMS;
////// end external API


class CSipVsrCtrl : public CStateMachine
{
	CLASS_TYPE_1(CSipVsrCtrl, CStateMachine)

public:

	// General state-machine interface
	//==================================
	explicit CSipVsrCtrl(CTaskApp * pOwnerTask,CPartyApi* pPartyApi);
	virtual const char* NameOf() const	{ return "CSipVsrCtrl";}
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}
	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual mcDmuxLync2013InfoReq* getDemuxInfo(){return NULL;}
	virtual ~CSipVsrCtrl();

	// State machine events
	//=======================
	void OnIpCmVsrMessageInd(CSegment* pParam);
	void OnIpCmVsrInappropriateEvent(CSegment* pParam);
	void OnIpCmVsrInitAckTout(CSegment* pParam);
	void InitializeAcked();

	// General services
	//===================
	void TriggerRcvVsr();
	virtual BOOL VideoSync(DWORD SSRC, BOOL isSynched);
	
	//LYNC2013_FEC_RED:
	virtual void PacketlossOnRecDirectionNeedToSendVsr(DWORD ssrc, DWORD fecPercent);

	// P2P specific pure virtuals - Should be implemented in P2P, should issue a warning/assert in others
	//=====================================================================================================
	virtual void Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcBasicLync2013InfoReq& initInfo) = 0;
	virtual BOOL SendVsr(const ST_VSR_SINGLE_STREAM& vsr) = 0;

	// AvMcu specific pure virtuals - Should be implemented in AvMcu, should issue a warning/assert in others
	//=========================================================================================================
	virtual void Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcAvMcuLync2013InfoReq& initInfo) = 0;
	virtual BOOL SendVsr(const ST_VSR_MUTILPLE_STREAMS& vsr) = 0;

protected:

	// Types
	//========
	typedef enum __RX_VSR_POLICY
	{
		eIgnoreRxVsr,
		eRaiseIntraReq,
		eSendVsrToParty
	} RxVsrPolicy_t;

	// Types
	//========
	typedef enum __TX_VSR_POLICY
	{
		eIgnoreTxVsr,
		ePendVsr,
		eSendVsrToRemote
	} TxVsrPolicy_t;

	// Disabling replications of this class
	//======================================
	CSipVsrCtrl& operator=(const CSipVsrCtrl&);
	CSipVsrCtrl(CSipVsrCtrl&);

	// Service utils
	//================
	BOOL 			SendMsSvcInitMsg(BYTE* initMsg, int size, DWORD opcode) const;
	void 			Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi);
	BOOL 			SendVsrInMplFormat(const ST_VSR_SINGLE_STREAM& srcVsr, DWORD vsrSlot);
	DWORD 			SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode) const;
	void			SendVsrToParty(const TCmRtcpVsrInfo& vsrInfo);
	RxVsrPolicy_t	RxVsrPolicy(const TCmRtcpVsrInfo& newVsr);
	TxVsrPolicy_t	TxVsrPolicy();
	BOOL			IncorrectUsage(const char* caller) const;
	inline void 	InterpolateEntryData(TCmRtcpVsrEntry& cmVsrEntry, int entryNum) const;
	inline DWORD 	MaxPixelsPerAspectRatio(BYTE aspectWidth, BYTE aspectHeight, DWORD width, DWORD height) const;
	inline DWORD 	SipFRToVsrFR(DWORD sipFR) const;
	inline DWORD 	VsrFRToSipFR(DWORD vsrFR) const;
	inline DWORD 	SipAspectToVsrAspect(DWORD sipAspect) const;
	inline DWORD 	VsrAspectToSipAspect(DWORD vsrAspect) const;
	inline void		NewRxVsr() 				{m_lastRxVsr.numberOfEntries = (APIU8) NUMERIC_NULL;}
	inline BOOL		IsFirstRxVsr() const	{return (m_lastRxVsr.numberOfEntries == (APIU8) NUMERIC_NULL);}
	inline BOOL		CompareEmbVsrs(TCmRtcpVsrInfo& lVsr, const TCmRtcpVsrInfo& rVsr);
	inline BOOL		ComparePartyVsrs(const ST_VSR_SINGLE_STREAM& lVsr, const ST_VSR_SINGLE_STREAM& rVsr) const;

	//LYNC2013_FEC_RED:
	DWORD GetVsrQRIndexFromFlag() const;

	// Inherited classes communication
	//==================================
	virtual void					SetReqId(DWORD vsrNdx, APIU16 requestId)	= 0;
	virtual APIU16					GetReqId(DWORD vsrNdx) const 				= 0;
	virtual DWORD					GetPartyId(DWORD SSRC) const 				= 0;
	virtual BOOL 					ResendMsSvcInitMsg() const					= 0;
	virtual BOOL 					ResendVsrMsg() 								= 0;
	virtual ST_VSR_SINGLE_STREAM*	GetLastTxVsr(DWORD SSRC, DWORD& slotNum)	= 0;

	// Data
	//=======
	CHardwareInterface*	m_pMfaInterface;
	CPartyApi*			m_pPartyApi;
	BOOL				m_vsrPending;
	TCmRtcpVsrInfo		m_lastRxVsr;

	PDECLAR_MESSAGE_MAP
};


class CSipVsrCtrlP2P: public CSipVsrCtrl
{
public:
	// General PObject interface
	//============================
	virtual const char* NameOf() const	{ return "CSipVsrCtrlP2P";}
	explicit CSipVsrCtrlP2P(CTaskApp * pOwnerTask, CPartyApi* pPartyApi);

	// P2P specific virtuals
	//========================
	virtual void Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcBasicLync2013InfoReq& initInfo);
	virtual BOOL SendVsr(const ST_VSR_SINGLE_STREAM& vsr);

	// AvMcu specific pure virtuals - should not be called in this context
	//======================================================================
	virtual void Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcAvMcuLync2013InfoReq& initInfo) {IncorrectUsage("AvMcu style Initialize");}
	virtual BOOL SendVsr(const ST_VSR_MUTILPLE_STREAMS& vsr) {return IncorrectUsage("AvMcu style SendVsr");}

protected:
	// Disabling replications of this class
	//======================================
	CSipVsrCtrlP2P& operator=(const CSipVsrCtrl&);
	explicit CSipVsrCtrlP2P(CSipVsrCtrl&);

	// Service utils
	//================
	BOOL SendMsSvcInitMsg(BYTE* initMsg) const {return CSipVsrCtrl::SendMsSvcInitMsg(initMsg, sizeof(mcBasicLync2013InfoReq), CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ);}

	// Base class communication
	//===========================
	virtual void			SetReqId(DWORD vsrNdx, APIU16 requestId)	{m_requestId = requestId;}
	virtual APIU16			GetReqId(DWORD vsrNdx) const				{return m_requestId;}
	virtual DWORD			GetPartyId(DWORD SSRC) const 				{return m_mplInit.connectedParties[0].unPartyId;}
	virtual BOOL 			ResendMsSvcInitMsg()  const					{return (m_mplInit.bIsAVMCU != (DWORD) NUMERIC_NULL ? SendMsSvcInitMsg((BYTE*)&m_mplInit) : FALSE);}
	virtual BOOL 			ResendVsrMsg()								{return SendVsrInMplFormat(m_lastTxVsr, 0);}
	ST_VSR_SINGLE_STREAM*	GetLastTxVsr(DWORD SSRC, DWORD& slotNum)	{return &m_lastTxVsr;}

	// Data
	//=======
	ST_VSR_SINGLE_STREAM	m_lastTxVsr;
	DWORD					m_requestId;
	mcBasicLync2013InfoReq	m_mplInit;
};

class CSipVsrCtrlAvMcu: public CSipVsrCtrl
{
public:
	// General PObject interface
	//============================
	virtual const char* NameOf() const	{ return "CSipVsrCtrlAvMcu";}
	explicit CSipVsrCtrlAvMcu(CTaskApp * pOwnerTask, CPartyApi* pPartyApi);

	// General services
	//===================
	//BOOL SendMux(DWORD SSRC, BOOL isSynched);

	// P2P specific virtuals - should not be called in this context
	//===============================================================
	virtual void Initialize(CHardwareInterface*, CPartyApi*, mcBasicLync2013InfoReq&) {IncorrectUsage("P2P style Initialize");}
	virtual BOOL SendVsr(const ST_VSR_SINGLE_STREAM&) {return IncorrectUsage("P2P style SendVsr");}

	// AvMcu specific virtuals
	//==========================
	virtual void Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, mcAvMcuLync2013InfoReq& initInfo);
	virtual BOOL SendVsr(const ST_VSR_MUTILPLE_STREAMS& vsr);
	virtual mcDmuxLync2013InfoReq* getDemuxInfo(){return &m_dmuxMsg;}

protected:
	// Disabling replications of this class
	//======================================
	CSipVsrCtrlAvMcu& operator=(const CSipVsrCtrl&);
	explicit CSipVsrCtrlAvMcu(CSipVsrCtrl&);

	// Service utils
	//================
	BOOL SendMsSvcInitMsg(BYTE* initMsg) 		const {return CSipVsrCtrl::SendMsSvcInitMsg(initMsg, sizeof(mcAvMcuLync2013InfoReq), CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ);}
	BOOL SendDmux(const ST_VSR_MUTILPLE_STREAMS& vsr);
	BOOL MarkForSignificantChanges(const ST_VSR_MUTILPLE_STREAMS& vsr, BOOL* sigChangeArr) const;

	// Base class communication
	//===========================
	virtual void					SetReqId(DWORD vsrNdx, APIU16 requestId) {DBGPASSERT_AND_RETURN(vsrNdx >= MAX_STREAM_LYNC_2013_CONN); m_requestId[vsrNdx] = requestId;}
	virtual APIU16					GetReqId(DWORD vsrNdx) const			{return (vsrNdx < MAX_STREAM_LYNC_2013_CONN? m_requestId[vsrNdx] : (APIU16) NUMERIC_NULL);}
	virtual BOOL 					ResendVsrMsg()							{return SendVsr(m_lastTxVsr);}
	virtual DWORD					GetPartyId(DWORD SSRC) const;
	virtual ST_VSR_SINGLE_STREAM*	GetLastTxVsr(DWORD SSRC, DWORD& slotNum);
	virtual BOOL 					ResendMsSvcInitMsg() const;

	// Data
	//=======
	ST_VSR_MUTILPLE_STREAMS	m_lastTxVsr;
	DWORD					m_requestId[MAX_STREAM_LYNC_2013_CONN];

	//=====================================================================================
	// Dmux and init basically hold the same database, since init-s are less real time,
	// data will be kept in dmux format, so conversions will happen only at init retries.
	//=====================================================================================
	mcDmuxLync2013InfoReq m_dmuxMsg;
	BOOL m_wasEverActive[MAX_STREAM_LYNC_2013_CONN];
	BOOL m_isEncrypted;
};

#endif /* SIPBFCPCONTROL_H_ */
