//+========================================================================+
//                     EndpointLinker.h							           |
//				Copyright 2009 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
//-------------------------------------------------------------------------|
// FILE:       EndpointLinker.h											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Victor                                                        |
//+========================================================================+

#ifndef __ENDPOINTLINKER_H__
#define __ENDPOINTLINKER_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "EndpointH323.h"

////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEndpointLinker : public CEndpointH323
{
	CLASS_TYPE_1(CEndpointLinker,CEndpointH323)

public:
	// constructors, distructors and operators
	CEndpointLinker( CTaskApi *pCSApi, const CCapSet & rCap, const CH323Behavior &rBehav );

	DWORD GetCallerCallIndex() const { return m_callerCsCallIndex; }
	
protected:
	virtual void OnCsCallSetupReq( CMplMcmsProtocol *pMplProtocol );
	void NotifyCallOfferingInd(mcReqCallSetup *setup);
	
	virtual void OnCallAnswerReq( CMplMcmsProtocol *pMplProtocol );
	void NotifyCallConnInd(mcReqCallAnswer *answer);

	virtual void OnCsCreateCntlReq( CMplMcmsProtocol* pMplProtocol );
	void ForwardCapabilityInd( CMplMcmsProtocol* pMplProtocol );

	virtual void OnCsReCapReq( CMplMcmsProtocol* pMplProtocol );

	virtual void OnOutgoingChnlReq( CMplMcmsProtocol* pMplProtocol );
	void ForwardIncomingChannelInd( CMplMcmsProtocol* pMplProtocol );

	virtual void OnCsIncomingChnlResponse( CMplMcmsProtocol* pMplProtocol );
	void ForwardOutgoingChnlResInd( CMplMcmsProtocol* pMplProtocol );

	virtual void OnChnlDropReq( CMplMcmsProtocol* pMplProtocol );
	void ForwardStartChnlCloseInd( CMplMcmsProtocol* pMplProtocol );

	virtual void OnCallDropReq( CMplMcmsProtocol* pMplProtocol );
	void ForwardCallIdleInd( CMplMcmsProtocol* pMplProtocol );
	virtual void OnCallCloseConfirmReq( CMplMcmsProtocol* pMplProtocol );

	virtual void OnRoleTokenReq(CMplMcmsProtocol* pMplProtocol);
	void ForwardRoleTokenInd(CMplMcmsProtocol* pMplProtocol);

	virtual void OnChannelOnReq(CMplMcmsProtocol* pMplProtocol);
	void ForwardChannelOnInd(CMplMcmsProtocol* pMplProtocol);

	virtual void OnChnlNewRateReq(CMplMcmsProtocol* pMplProtocol);
	void ForwardChanNewRateInd(CMplMcmsProtocol* pMplProtocol);
	void ForwardFlowControIndInd(CMplMcmsProtocol* pMplProtocol);

	BYTE* CreateIndicationAndEncryToken(size_t ind_size, const encTokensHeaderStruct &encryTokens, size_t &newSize);
	CMplMcmsProtocol* CreateSwitchMplMcmsMsg(const CMplMcmsProtocol &MplProtocol, DWORD opcode, BYTE *pIndication, DWORD nDataLen);

	virtual void SendCsCallConnectedInd( CMplMcmsProtocol* pMplProtocol );

private:
	DWORD m_callerConfID;
	DWORD m_callerPartyID;
	DWORD m_callerConnectionID;
	DWORD m_callerCsCallIndex;
	WORD m_callerCsHandle;
	WORD m_callerCsSrcUnit;

	APIS32 m_callerTermType;
	APIS32 m_calleeTermType;


};

#endif // __ENDPOINTLINKER_H__ 

