/*
 * SipEventPackageCommon.h
 *
 *  Created on: Aug 30, 2012
 *      Author: bguelfand
 */

#ifndef SIPEVENTPACKAGECOMMON_H_
#define SIPEVENTPACKAGECOMMON_H_

enum{
	e_Pending,
	e_DialingOut,
	e_DialingIn,
	e_Alerting,
	e_OnHold,
	e_Connected,
	e_MutedViaFocus,
//	e_UnMutedViaFocus,
	e_Disconnecting,
	e_Disconnected
}typedef eEndPointStatusType;

enum{
	eConnectionChanged,
	eMuteChanged,
	eSpeakerChanged,
	eActionNoChanged,
	eAdded,
	eDeleted
}typedef eActionType;


enum{
	eRecvOnly,
	eSendOnly,
	eSendRecv,
	eInactive
}typedef eMediaStatusType;

enum{
	eNoChange = 0,
	ePartialData,
	eFullData,
	eDelletedData // just for conf event
}typedef eTypeState;


enum{
	eAdministrator,
	eChair,
	eParticipant
}typedef eRoleType;

enum{
	ePending,
	eCalling,
	eRinging,
	eOnHold,
	eConnected,
	eDisconnecting,
	eDisconnected,
	eBlocked,
	eRemoved
}typedef eStatusType;

enum{
	eDialedIn,
	eDialedOut
}typedef eJoinModeType;

enum{
	eNotDisconnected,
	eDeparted,
	eBooted,
	eFailed,
	eBusy
}typedef eDisconnectReasonType;

enum{
	eAudio,
	eVoip,
	eVideoT,
	eData
}typedef eMediaContentType;


enum{
	eRegular,
	eSendNotifyBeforeDelete,
	eNotifyWasSent
}typedef eItemState;

#define UPDATE_STATE	if(eNoChange == m_state) m_state = ePartialData;

#endif /* SIPEVENTPACKAGECOMMON_H_ */
