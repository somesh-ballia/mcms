#include "NStream.h"
#include "ConfPartyManagerLocalApi.h"
#include "ConfPartyOpcodes.h"
#include "CommRes.h"


////////////////////////////////////////////////////////////////////////////
//                        CConfPartyManagerLocalApi
////////////////////////////////////////////////////////////////////////////
CConfPartyManagerLocalApi::CConfPartyManagerLocalApi()
{
}

//--------------------------------------------------------------------------
CConfPartyManagerLocalApi::~CConfPartyManagerLocalApi()
{
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::AddRecordingLinkParty(DWORD conferId, BYTE rMuteVideo)
{
	 //This message arrives from Cconf
	CSegment*  seg = new CSegment;

	*seg << rMuteVideo << conferId;

	 SendMsg(seg, ADD_RECORDING_LINK_PARTY);
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::DisconnectRecordingLinkParty(DWORD conferId)
{
	 //This message arrives from Cconf
	CSegment*  seg = new CSegment;

	*seg << conferId ;

	 SendMsg(seg, DISCONNECT_RECORDING_LINK_PARTY);
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::AcceptUnreservedParty(const mcTransportAddress* pDestIP, DWORD confId,DWORD undefId, WORD bIsVoice, //shiraITP - 43
														WORD bIsSip/*, BYTE bIsMultiInvite*/, eSipFactoryType factoryType,
														const char* pSipAddress, BYTE initialEncryptionValue,
														const char* pH323useruserField,
														WORD bIsMrcHeader, WORD bIsWebRtcCall, /*BYTE tmpCascadeMode,*/
														BYTE bIsMsConfInvite, WORD srsSessionType,BYTE bIsNonCCCPAvMcu)
 {
    //This message arrives from Lobby
	CSegment*  seg = new CSegment;

    if(pDestIP == NULL)
    {
       BYTE tmp[sizeof(mcTransportAddress)];
       memset(tmp, 0,sizeof(mcTransportAddress) );
       seg->Put((BYTE*)tmp, sizeof(mcTransportAddress));
    }
    else
	seg->Put((BYTE*)pDestIP,sizeof(mcTransportAddress));

	*seg << confId
	     << undefId
	     << bIsVoice
		 << bIsSip
		 << (BYTE)bIsMsConfInvite
		 << (BYTE)factoryType
		 << (BYTE)bIsNonCCCPAvMcu
		 << pSipAddress
		 << (BYTE)initialEncryptionValue
		 << pH323useruserField
		 << bIsMrcHeader
		 << bIsWebRtcCall
		 << srsSessionType;
//		 << (BYTE)tmpCascadeMode;	//--- patch for ignoring Encryption in Cascade


  SendMsg(seg,ACCEPT_UNRESERVED_PARTY);
}
//--------------------------------------------------------------------------
/*void CConfPartyManagerLocalApi::AcceptAVMCUInvitation(const mcTransportAddress* pDestIP, DWORD confId,const char* pSipAddress,
													const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD sdpLen)
{
	//This message arrives from Lobby
		CSegment*  seg = new CSegment;

	    if(pDestIP == NULL)
	    {
	       BYTE tmp[sizeof(mcTransportAddress)];
	       memset(tmp, 0,sizeof(mcTransportAddress) );
	       seg->Put((BYTE*)tmp, sizeof(mcTransportAddress));
	    }
	    else
		seg->Put((BYTE*)pDestIP,sizeof(mcTransportAddress));

		*seg << confId
		     << pSipAddress
			 << sdpLen;

		seg->Put((BYTE*)pSdpAndHeaders, sdpLen);

		 SendMsg(seg,ACCEPT_AV_MCU_INVITATION);

}
*/
//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::StartMeetingRoom(DWORD confId)
{
   //This message arrives from Lobby
	CSegment*  seg = new CSegment;

	*seg << confId;
	 SendMsg(seg, START_MEETING_ROOM);
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::LobbyStartAdHocConf(DWORD dwAdHocProfileId,CCommRes* pAdHocRsrv, char* pAdHocConfName, char* pAdHocNID, BYTE isSipFactory,char* pAdHocConfDisplayName)
{
	COstrStream str;

	if(pAdHocRsrv)
	{
		pAdHocRsrv->Serialize(NATIVE, str);
	}
	else
		PASSERT(1);

	str << (WORD)isSipFactory<<"\n";
	if(pAdHocConfName)
	{
		str << pAdHocConfName<<"\n";
	}
	else
	{
		str << "";
	}

	if(pAdHocNID)
	{
		str << pAdHocNID<<"\n";
	}
	else
	{
		str << '\0';
	}

	if(pAdHocConfDisplayName)
	{
		str << pAdHocConfName<<"\n";
	}
	else
	{
		str << "";
	}

	CSegment*  seg = new CSegment;
  	*seg << str.str().c_str();

	SendMsg(seg, LOBBY_START_AD_HOC_CONF);
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::StartGateWayConf(DWORD confId,char* confName, char* TargetNumber)
{
	//This message arrives from Lobby
	CSegment*  seg = new CSegment;

	*seg << confId
		 << confName;
	if( TargetNumber && strlen(TargetNumber) )
	{
		*seg << (WORD)strlen(TargetNumber);
		*seg << TargetNumber;
	}
	else
		*seg << (WORD)0;

	 SendMsg(seg, START_GATEWAY_CONF);
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::MovePartyToConfOrMeetingRoom(DWORD dwSourceConfId, DWORD dwPartId, ETargetConfType targetConfType, DWORD targetConfId, CCommRes* pConfProfile)
{
	CSegment*  seg = new CSegment;

	*seg << (DWORD)dwSourceConfId
		 << (DWORD)dwPartId
		 << (DWORD)targetConfType;

	if ((eOnGoingConf == targetConfType) || (eMeetingRoom == targetConfType))
		*seg << (DWORD)targetConfId;	// for On-Going & MR only (irrelevant for Ad-Hoc)

	if (eAdHoc == targetConfType && pConfProfile)
		pConfProfile->Serialize(NATIVE, *seg);		// for Ad-Hoc only (irrelevant for On-Going & MR)

	SendMsg(seg, MOVE_PARTY_TO_CONF_OR_MR);
}

//--------------------------------------------------------------------------
void CConfPartyManagerLocalApi::CreateDialOutParty(DWORD wMonitorConfID, WORD listId, BYTE interfaceType, char* pPartyURI, BYTE isTel, const char* pRefferedByStr, const char* pReferUri, const char* pMSAssociatedStr)
{
	CSegment*  seg = new CSegment;

	*seg << wMonitorConfID
		 << listId
		 << interfaceType
		 << pPartyURI
		 << isTel;

	if(pReferUri)
		*seg << pReferUri;
	else
		*seg << "";

	if(pRefferedByStr)
		*seg << pRefferedByStr;
	else
		*seg << "";

	if(pMSAssociatedStr)
		*seg << pMSAssociatedStr;
	else
		*seg << "";

	SendMsg(seg, ADD_OUT_PARTY);
}
