//+========================================================================+
//                  GKCall.h									  		   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GKCall.h	                                                   |
// SUBSYSTEM:  Processes/Gatekeeper/GatekeeperLib		                   |                       |
// PROGRAMMER: Yael A.	                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |  Attributes and basix functionality of calls through |
//	   |			|													   |
//						 												   |
//						                                                   |
//+========================================================================+                        


#ifndef GKCALL_H_
#define GKCALL_H_

#include "StateMachine.h"
#include "IpCommonDefinitions.h"
#include "GKGlobals.h"
#include "GKManagerApi.h"


typedef enum
{
	eNoSend,
	eWaitToSendFirstTime, //1 - at first time or if the party answers
	eSendFirstTime,       //2 - after first sending
	eSendSecondTime, 	  //3 - after second sending
}eErrorHandlingStatus;

inline eErrorHandlingStatus operator++(eErrorHandlingStatus& e, int) 
{
	eErrorHandlingStatus eTemp = e;
	e = (eErrorHandlingStatus)(e+1); 
	return eTemp;
}



class CGkCall : public CStateMachine
{
	CLASS_TYPE_1(CGkCall ,CStateMachine)      

public:
		
	CGkCall(const COsQueue& gkManagerRcvMbx, DWORD mcmsConnId, DWORD partyId = 0, DWORD serviceId = 0, DWORD confId = 0, char* conferenceId = NULL, char* callId = NULL,
			int	crv = 0, BYTE bIsDialIn = 0, eCallStatus callState = eInvalidCallState);
	virtual ~CGkCall();
	void Dump(std::ostream& msg) const;
	virtual const char* NameOf() const { return "CGkCall";}
//	friend bool operator<(const CGkCall&, const CGkCall&);	
	
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
	void* GetMessageMap();
	
	//Operations:
	void 		UpdateParams(char conferenceId[Size16], char callId[Size16], BYTE bIsDialIn, eCallStatus callState);
	void 		UpdateCallParamsAccordingToARQInd(int crv, char conferenceId[Size16], char callId[Size16]);
	void 		SetCallState(eCallStatus callState);
   
    eCallStatus GetCallState() const;
    DWORD  		GetServiceId()const ; 
    const char* GetCallId() const;
	const char* GetConferenceId() const;
	DWORD 		GetConnId() const;
	DWORD 		GetPartyId() const;
	DWORD 		GetConfRsrcId() const;
	int   		GetCrv() const;
	BYTE  		GetIsDialIn() const;
    BYTE  		IsCallInHoldState() const;
     void 		SetSeviceId(DWORD seviceId);
    void 		SetConnId(DWORD connId);
    void 		SetConfRsrcId(DWORD confId);

    //error handling 
    void    	InitErrorHandlingStatus();
    void 		StartPartyKeepAliveFlow();
    void 		StartPartyKeepAliveTimer();
    void		OnTimerPartyKeepAlive(CSegment *pMsg);
    void 		OnPartyAnswerToKeepAlive();

protected:
	//Attributes:
	DWORD		m_mcmsConnId;
	DWORD		m_partyId;
	DWORD		m_serviceId;
	DWORD		m_ConfRsrcId;
	char		m_conferenceId[Size16];
	char		m_callId[Size16];
	int			m_crv;
	BYTE		m_bIsDialIn;
	eCallStatus m_callState;
	
	//error handling members:
	CGKManagerApi*  	  m_pGkManagerApi;
	eErrorHandlingStatus  m_eErrorHandlingStatus;
		
//	UINT8	authKey[LengthEncAuthKey];	//len = 128
//	char	sId[2];

	PDECLAR_MESSAGE_MAP       
	                              
private:
	CGkCall(const CGkCall &other);				

};




#endif /*GKCALL_H_*/

