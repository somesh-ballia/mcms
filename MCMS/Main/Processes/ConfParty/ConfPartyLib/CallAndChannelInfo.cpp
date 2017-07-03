//// Holds all IP Party info regarding channel and call info.
#include <memory.h>
#include <time.h>
#include "TraceStream.h"
#include "CallAndChannelInfo.h"
#include "Trace.h"
#include "Macros.h"
#include "H323Scm.h"
#include "ConfPartyGlobals.h"

//////////////////////////////////////////////////////////////////////////////
////////////////////// Class CCall ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
CCall::CCall()
{
	m_callIndex = 0;
	m_status = 0;
	m_mcmsConnId = 0;
	m_channelsCounter = 0;
	for (int i = 0; i < MaxChannelsPerCall; i++)
		m_pChannelsArray[i] = NULL;
	m_numOfInternalChannels = 0;
    for (int i = 0; i < MAX_INTERNAL_CHANNELS; i++)
        m_pInternalChannelsArray[i] = NULL;

	memset(m_sourceInfo,'\0',MaxAddressListSize);
	memset(m_srcInfoAliasesType, 0, sizeof (m_srcInfoAliasesType));
	memset(m_destinationInfo,'\0',MaxAddressListSize);
	memset(m_destInfoAliasesType, 0, sizeof(m_destInfoAliasesType));
	m_bCanMapAlias = FALSE;
	memset(m_callId,'\0',Size16);
	memset(m_conferenceId,'\0',MaxConferenceIdSize);
	m_referenceValueForEp = 0;
	m_referenceValueForGk = 0;
	m_callType = cmCallTypeP2P;									
	m_model = cmCallModelTypeDirect;									
	m_bIsActiveMc = FALSE;							
	m_bIsOrigin = FALSE;								
	m_bIsClosingProcess = FALSE;						
	m_callCloseInitiator = NoInitiator;						
	m_bH245Establish = FALSE; 										   
	m_maxRate = 0;								
	m_minRate = 0;								
	m_rate = 0;									
	m_bandwidth = 0;								
	m_masterSlaveStatus = 0;						
	m_rmtType = cmEndpointTypeTerminal;
	m_dynamicPayloadTypeIndex = 96;	
	mcTerminalParams TerParams;
//	char temp[MaxAddressListSize] = "\0";
//	memcpy(TerParams.partyAddress,temp,MaxAddressListSize);
// IpV6
	memset(&TerParams,0,sizeof(mcTerminalParams));
	TerParams.endpointType = cmEndpointTypeTerminal;
	TerParams.callSignalAddress.ipVersion = eIpVersion4;

	SetSrcTerminalParams(TerParams);
	SetDestTerminalParams(TerParams);
	
	memset(&m_setupH245Address,   0, sizeof(mcTransportAddress));
	memset(&m_answerH245Address,  0, sizeof(mcTransportAddress));
	memset(&m_controlH245Address, 0, sizeof(mcTransportAddress));

    m_numOfSrcAliases = 0;
    m_numOfDestAliases = 0;
    
    memset(&m_lprCapStruct,   0, (sizeof(lprCapCallStruct)*NUM_OF_LPR_CAPS));
    memset(&m_callTransient,  0, sizeof(mcCallTransient));

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CCall::~CCall()
{
	for (int i = 0; i < MaxChannelsPerCall; i++)
	{
		if (m_pChannelsArray[i] != NULL)
		{
			POBJDELETE(m_pChannelsArray[i]);
			m_pChannelsArray[i] = NULL;
		}
	}
    for (int i = 0; i < MAX_INTERNAL_CHANNELS; i++)
    {
        if (m_pInternalChannelsArray[i] != NULL)
        {
            POBJDELETE(m_pInternalChannelsArray[i]);
            m_pInternalChannelsArray[i] = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallIndex(DWORD callIndex)
{
	m_callIndex = callIndex;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetCallIndex ()
{
	return m_callIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallStatus(int status)
{
	m_status = status;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCall::GetCallStatus()
{
	return m_status;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetConnectionId(WORD conId)
{
	m_mcmsConnId = conId;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCall::GetConnectionId()
{
	return m_mcmsConnId;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::IncreaseChannelsCounter()
{
	m_channelsCounter++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::DecreaseChannelsCounter()
{
	if (m_channelsCounter > 0)
		m_channelsCounter--;
	else
		PTRACE(eLevelError, "CCall::DecreaseChannelsCounter - m_channelsCounter<=0");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetChannelsCounter()
{
	return m_channelsCounter;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CChannel** CCall::GetChannelsArray()
{
	return m_pChannelsArray;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CChannel* CCall::GetSpecificChannel(DWORD index, bool aIsExternal) const
{
	int i=0;
	if (aIsExternal)
	{
        if(index >= MaxChannelsPerCall)
            return NULL;
    /*	for(i = 0;i<MaxChannelsPerCall;i++)
        {
            if(m_pChannelsArray[i])
                if(m_pChannelsArray[i]->GetIndex() == index)
                        return m_pChannelsArray[i];
        }*/
        return m_pChannelsArray[index];
	}

	if(index >= MAX_INTERNAL_CHANNELS)
        return NULL;
    return m_pInternalChannelsArray[index];
}

BYTE CCall::AreAllInternalChannelsConnected()
{
    CChannel *pChannel = NULL;
    for (int i = 0; i < MAX_INTERNAL_CHANNELS; i++)
    {
        pChannel = m_pInternalChannelsArray[i];
        if (pChannel && !pChannel->IsChannelCsConnected())
        {
            TRACEINTO << "mix_mode: Channel #" << i << " is not connected. Channel state=" << pChannel->GetCsChannelState();
            return FALSE;
        }
    }
    TRACEINTO << "mix_mode: All internal channels are connected";
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int	CCall::SetCurrentChannel(APIU32 csIndex, APIU32 mcmsIndex, CChannel **pChannel,APIS8 *pIndex) 
{
		int i=0;
		for(i=0;i<MaxChannelsPerCall;i++)
		{
			if (m_pChannelsArray[i])
			{
				// Check if mcms index is equals
				if (m_pChannelsArray[i]->GetIndex() == mcmsIndex)
				{
					// if we have a pmIndex, we should compare it too.
					if (m_pChannelsArray[i]->GetCsIndex() && csIndex)
					{
						if(m_pChannelsArray[i]->GetCsIndex() == csIndex)
						{
							*pChannel = m_pChannelsArray[i];
							*pIndex = i;
							return 0;

						}
					}
					// if we don't have a pmIndex we compare only the mcms index.
					else
					{
						*pChannel = m_pChannelsArray[i];
						*pIndex = i;
						return 0;
					}
				}
			}
		}			


	TRACEINTO << "CCall::SetCurrentChannel:Channel Not found!!! mc index = " << mcmsIndex << " CsIndex = " << csIndex;	
	*pIndex = NA;
	return 1;
	
}

//////////////////////////////////////////////////////////
int CCall::FindNextAvailableIndexInInternalChannelArr() const
{
    int index = NA;
    for (int i = 0; i < MAX_INTERNAL_CHANNELS && index == NA; i++)
    {
        if (m_pInternalChannelsArray[i] == NULL)
            index = i;
    }
    return index;
}

/////////////////////////////////////////////////////////////////////////////////
CChannel*  CCall::AddChannelInternal(CComModeH323* pComMode, cmCapDataType aMediaType, cmCapDirection aDirection, CapEnum aCapCode, DWORD rate, APIU32 aSsrcId)
{
    const std::list <StreamDesc> rStreams = pComMode->GetStreamsListForMediaMode(aMediaType, cmCapReceive, kRolePeople);
    ERoleLabel eRole = kRolePeople;
    TRACEINTO << "mix_mode: Adding internal channel aMediaType=" << ::GetTypeStr(aMediaType)
              << " aDirection=" << ::GetDirectionStr(aDirection) << " eRole=" << ::GetRoleStr(eRole);
    std::ostringstream ostr;
    bool channelExists = false;

    BOOL isTransmitting = (aDirection == cmCapTransmit? TRUE : FALSE);
    CChannel* pChannel = GetInternalChannelBySsrc(aMediaType, isTransmitting, eRole, aSsrcId);

    if (!pChannel)
    {
//      SystemCoreDump(FALSE);
        int i = FindNextAvailableIndexInInternalChannelArr();
        if (i != NA)
        {
            pChannel = new CChannel;
            pChannel->SetIndex((DWORD)i+1);
            pChannel->InitChannelParams(this, isTransmitting, aCapCode, eRole, rate, rate, NULL, 0, 0, FALSE, kUnKnownMediaType);
            pChannel->SetStreamsList(rStreams, aSsrcId);
            pChannel->SetCsChannelState(kConnectingState);
            m_pInternalChannelsArray[i] = pChannel;
            m_numOfInternalChannels++;
            TRACEINTO << "i = " << i << ", aMediaType=" << ((WORD)(aMediaType)) <<", eRole=" << ((WORD)(eRole))<< ", aDirection=" << ((WORD)(aDirection));
        }
        else
        {
            PASSERT(m_mcmsConnId);
            TRACEINTO << "No room for this channel";
        }
    }
    else
    {
        CSmallString msg;
        msg << "Channel of type " << aMediaType << " direction " << aDirection << "  already exists in call " << m_mcmsConnId << "\n";
        PTRACE2(eLevelError,"CCall::AddChannelInternal: ",msg.GetString());
        PASSERT(m_mcmsConnId);
        pChannel = NULL; //for the return value
    }
    return pChannel;
}

////////////////////////////////////////////////////////////////////////////
// this function Initialize the McChannel parameters and insert it to the pChannelsArray
// return 0 when failed
// and none zero for success
void CChannel::InitChannelParams(CCall *apCall, BOOL bIsOutgoingChannel, CCapSetInfo capInfo,ERoleLabel eRole,
                   DWORD rate, DWORD localCapRate, mcIndIncomingChannel *pinChnlInd, APIS32 status, APIU32 CsChannelIndex,
                   APIU16 isLpr, EenMediaType encAlg)
{
  TRACEINTO << "CsChannelIndex - " << CsChannelIndex;

  SetChannelParams(0,NULL);
  SetCallPointer(apCall);
  SetCsChannelState(kConnectingState);
  SetStreamState(kStreamOffState);
  SetChannelCloseInitiator((DWORD)NoInitiator);
  SetMaxSkew(0);
  SetChannelDirection(bIsOutgoingChannel);    //  Pm response
  SetPayloadType(capInfo.GetPayloadType());
  SetCapNameEnum(capInfo);
  SetType(capInfo.GetCapType());
  SetRoleLabel(eRole);
  SetIsRejectChannel(FALSE);
  SetIsStreamOnSent(FALSE);
  SetIsEncrypted(FALSE);
  SetEncryptionType(kUnKnownMediaType);

  // LPR
  SetIsLprSupported(isLpr);

  if(bIsOutgoingChannel || !pinChnlInd)
  {
    SetSizeOfChannelParams(0);
    SetIsActive(TRUE);
    SetSessionId(-1);
    SetCsIndex(0);
    SetStatus(0);
    if(encAlg != kUnKnownMediaType)
    {
      SetIsEncrypted(TRUE);
      SetEncryptionType(encAlg);
    }
  }
  else
  {
    SetSizeOfChannelParams(pinChnlInd->sizeOfChannelParams);
    if(pinChnlInd->bIsActive)
      SetIsActive(TRUE);
    else
      SetIsActive(FALSE);

    SetSessionId(pinChnlInd->sessionId);
    SetCsIndex(CsChannelIndex);
    SetStatus(status);
    SetIsEncrypted(pinChnlInd->bIsEncrypted);
    SetEncryptionType((EenMediaType)pinChnlInd->encryptionAlgorithm);
    SetDynamicPayloadType(pinChnlInd->dynamicPayloadType);
    SetIsH263Plus(pinChnlInd->bUsedH263Plus);
  }

  if ((capInfo.GetPayloadType() != _AnnexQ) && (capInfo.GetPayloadType() != _RvFecc))
    SetRate(rate);
  else
    SetRate(localCapRate);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CChannel*  CCall::FindChannelInList(DWORD sameSessionCahhnelIndex) const
{
	int i=0;
	for(i=0;i<MaxChannelsPerCall;i++)
	{
		if(m_pChannelsArray[i])
			if(m_pChannelsArray[i]->GetIndex() == sameSessionCahhnelIndex)
					return m_pChannelsArray[i];						
	}			
	return NULL;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CChannel* CCall::FindChannelInList(cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole, bool aIsExternal) const
{
	CChannel *pRes = NULL;
	int i = GetChannelIndexInList(aIsExternal, eType,bIsTransmiting,eRole);
	pRes = GetSpecificChannel(i, aIsExternal);
	return pRes;
	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetChannelIndexInList(CChannel *pChannel, bool aIsExternal) const
{
	cmCapDataType eType = pChannel->GetType();
	BYTE bIsTransmiting = pChannel->IsOutgoingDirection();
	ERoleLabel eRole    = pChannel->GetRoleLabel();
	return GetChannelIndexInList(aIsExternal, eType, bIsTransmiting, eRole);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetChannelIndexInList(bool aIsExternal, cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole) const
{
	// sometimes the bIsTransmitting is TRUE (255) and sometimes it's 1
	bIsTransmiting = bIsTransmiting ? TRUE: FALSE;

    CChannel* const* channelsPtrArr;
    int max = 0;
    if (aIsExternal)
    {
        channelsPtrArr = m_pChannelsArray;
        max = MaxChannelsPerCall;
    }
    else
    {
        channelsPtrArr = m_pInternalChannelsArray;
        max = MAX_INTERNAL_CHANNELS;
    }

	for(int i=0; i < max; i++)
	{
		CChannel *pMcChannel = channelsPtrArr[i];
		if(pMcChannel)
		{
			if((pMcChannel->GetType() == eType) && (pMcChannel->IsOutgoingDirection() == bIsTransmiting)) 
			{
				if ((pMcChannel->GetRoleLabel() == eRole) || (pMcChannel->GetRoleLabel() & eRole) )
					return i;//we need the == for role people, which is 0.
			}
		}
	}			
	return max;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCall::UpdateMcChannelParams1(BOOL bIsTransmit, CChannel *&pMcChannel, CCapSetInfo capInfo,ERoleLabel eRole,
								DWORD rate, mcIndIncomingChannel *pinChnlInd, EenMediaType encType, APIS32 status)
{
	pMcChannel = new CChannel;
	int i = 0;

	// enter the channel structure to the channel array
	for (i = 0; i < MaxChannelsPerCall; i++)
	{
		if (!m_pChannelsArray[i]) 
		{
			m_pChannelsArray[i] = pMcChannel;
			break; 
		}
	}
	if (i == MaxChannelsPerCall)
	{
		PASSERTMSG(102,"CCall::UpdateMcChannelParams1: channel number exceeded");
		return i;
	}

	IncreaseChannelsCounter();
	(m_pChannelsArray[i])->UpdateChannelParams(this,m_channelsCounter,bIsTransmit,capInfo,eRole,rate,pinChnlInd,encType,status);
	return i;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::RemoveChannel(CChannel *pChannel)
{
	DWORD arrayIndex = GetChannelIndexInList(pChannel, true);
	RemoveChannel(arrayIndex);
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::RemoveChannelInternal(CChannel *pChannel)
{
    if (!pChannel)
    {
        return;
    }

    for(int i=0; i < MAX_INTERNAL_CHANNELS; i++)
    {
        if (m_pInternalChannelsArray[i] == pChannel)
        {
            return RemoveChannelInternal(i);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::RemoveChannel(DWORD arrayIndex)
{
    if((arrayIndex < MaxChannelsPerCall) && (m_pChannelsArray[arrayIndex] != NULL))
    {
        POBJDELETE(m_pChannelsArray[arrayIndex]);
        DecreaseChannelsCounter();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::RemoveChannelInternal(DWORD arrayIndex)
{
    if((arrayIndex < MAX_INTERNAL_CHANNELS) && (m_pInternalChannelsArray[arrayIndex] != NULL))
    {
        POBJDELETE(m_pInternalChannelsArray[arrayIndex]);
        if (m_numOfInternalChannels > 0)
            m_numOfInternalChannels--;
        else
            PTRACE(eLevelError, "CCall::RemoveChannelInternal - m_numOfInternalChannels<=0");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::UpdateCallSignalAddress(ipAddressIf* ipAddress, APIU32 port, mcTransportAddress* pCallSignalAddressToSet, BYTE isIpV6)
{
	// IpV6
		if (port != 0)
			pCallSignalAddressToSet->port = port;
	
	if (!isIpV6)
	{
		if (ipAddress->v4.ip != 0)
			pCallSignalAddressToSet->addr.v4.ip = ipAddress->v4.ip;
	}
	else
	{
		// IpV6
		APIU8	ipNull[16] 	= {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
								  	   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
								  	
		if ( memcmp(ipAddress->v6.ip,ipNull,sizeof(ipAddress->v6.ip)) != 0)		
			memcpy(pCallSignalAddressToSet->addr.v6.ip, ipAddress->v6.ip, 16);
	}	
		
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::UpdateSourceCallSignalAddressPort(APIU32 port)
{
	m_srcTerminal.callSignalAddress.port = port;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
mcTerminalParams CCall::GetSrcTerminalParams()
{
	return m_srcTerminal;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallSignalAddress(const mcTransportAddress& callSignalAddress, mcTransportAddress* pCallSignalAddressToSet)
{
	//ip version
	if (callSignalAddress.ipVersion > eIpVersion6)
	{
		PASSERTMSG((DWORD)callSignalAddress.ipVersion,"CCall::SetCallSignalAddress - cmTransport type unknown");
		return;
	}
	pCallSignalAddressToSet->ipVersion = callSignalAddress.ipVersion;
	
	//port
	pCallSignalAddressToSet->port = callSignalAddress.port;
	
	//distribution
	if (callSignalAddress.distribution > eDistributionMulticast)
	{
		PASSERTMSG((DWORD)callSignalAddress.distribution,"CCall::SetCallSignalAddress - Distribution type unknown");
		return;
	}
	pCallSignalAddressToSet->distribution = callSignalAddress.distribution;
	
	//transportType
	if (callSignalAddress.transportType > eTransportTypeTls)
	{
		PASSERTMSG((DWORD)callSignalAddress.transportType, "CCall::SetCallSignalAddress - TransportType unkown");
		return;
	}
	pCallSignalAddressToSet->transportType = callSignalAddress.transportType;

	//ip
	if (pCallSignalAddressToSet->ipVersion == eIpVersion4)	// Case IpV4
		pCallSignalAddressToSet->addr.v4.ip = callSignalAddress.addr.v4.ip;
	else // Case IpV6
	{
		pCallSignalAddressToSet->addr.v6.scopeId = callSignalAddress.addr.v6.scopeId;
		memcpy(pCallSignalAddressToSet->addr.v6.ip, callSignalAddress.addr.v6.ip, 16);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSrcTerminalCallSignalAddress(const mcTransportAddress& callSignalAddress)
{
	SetCallSignalAddress(callSignalAddress, &m_srcTerminal.callSignalAddress);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSrcTerminalParams(mcTerminalParams srcTerParams)
{
	memcpy(m_srcTerminal.partyAddress,srcTerParams.partyAddress,MaxAddressListSize);
	if (srcTerParams.endpointType > cmEndpointTypeSET)
	{
		PASSERTMSG((DWORD)srcTerParams.endpointType,"CCall::SetSrcTerminalParams - End point type unknown");
		return;
	}
	m_srcTerminal.endpointType = srcTerParams.endpointType;
	SetCallSignalAddress(srcTerParams.callSignalAddress, &m_srcTerminal.callSignalAddress);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
mcTerminalParams CCall::GetDestTerminalParams()
{
	return m_destTerminal;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetDestTerminalCallSignalAddress(const mcTransportAddress& callSignalAddress)
{
	SetCallSignalAddress(callSignalAddress, &m_destTerminal.callSignalAddress);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetDestTerminalParams(mcTerminalParams destTerParams)
{
	memcpy(m_destTerminal.partyAddress,destTerParams.partyAddress,MaxAddressListSize);
	if (destTerParams.endpointType > cmEndpointTypeSET)
	{
		PASSERTMSG((DWORD)destTerParams.endpointType,"CCall::SetDestTerminalParams - End point type unknown");
		return;
	}
	m_destTerminal.endpointType = destTerParams.endpointType;
	SetCallSignalAddress(destTerParams.callSignalAddress, &m_destTerminal.callSignalAddress);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCall::GetSourceInfoAlias()
{
	return m_sourceInfo;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCall::SetSourceInfoAlias(const char* sourceInfo)
{
	memset(m_sourceInfo,'\0',MaxAddressListSize);

	if (strlen(sourceInfo) > MaxAddressListSize - 1)
	{
		PASSERTMSG(256,"CCall::SetSourceInfoAlias - SourceInfo size not valid");
		return;
	}
	memcpy(m_sourceInfo,sourceInfo,strlen(sourceInfo)+1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD* CCall::GetSrcInfoAliasType()
{
	return m_srcInfoAliasesType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSrcInfoAliasType(DWORD* srcInfoAliasesType)
{
    m_numOfSrcAliases = 0;
	memset(m_srcInfoAliasesType, 0, sizeof(m_srcInfoAliasesType));
	for (int i = 0; i < MaxNumberOfAliases; i++)
    {
		m_srcInfoAliasesType[i] = srcInfoAliasesType[i];
        if (m_srcInfoAliasesType[i])
            m_numOfSrcAliases++;
    }
    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCall::GetDestinationInfoAlias()
{
	return m_destinationInfo;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetDestinationInfoAlias(char* destinationInfo)
{
	memset(m_destinationInfo,'\0',MaxAddressListSize);

	if (strlen(destinationInfo) > MaxAddressListSize)
	{
		PASSERTMSG(256,"CCall::SetDestinationInfoAlias - DestinationInfo size not valid");
		return;
	}
	
	memcpy(m_destinationInfo,destinationInfo,MaxAddressListSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD* CCall::GetDestInfoAliasType()
{
	return m_destInfoAliasesType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetDestInfoAliasType(DWORD* destInfoAliasesType)
{
    m_numOfDestAliases = 0;
	memset(m_destInfoAliasesType, 0, sizeof(m_destInfoAliasesType));
	for (int i = 0; i < MaxNumberOfAliases; i++)
    {
		m_destInfoAliasesType[i] = destInfoAliasesType[i];
        if (m_destInfoAliasesType[i])
            m_numOfDestAliases++;
    }
    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCall::GetCanMapAlias()
{
	return m_bCanMapAlias;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCanMapAlias(BOOL canMapAlias)
{
	if (canMapAlias != TRUE && canMapAlias != FALSE)
	{
		PASSERTMSG(canMapAlias,"CCall::SetCanMapAlias - canMapAlias not bulian");
		return;
	}
	m_bCanMapAlias = canMapAlias;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCall::GetCallId()
{
	return m_callId;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallId(const char* callId)
{
	memset(m_callId,'\0',Size16);
	memcpy(m_callId,callId,Size16);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCall::GetConferenceId()
{
	return m_conferenceId;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetConferenceId(const char* conferenceId)
{
	memset(m_conferenceId,'\0',MaxConferenceIdSize);
/*	int confIdLen = strlen(conferenceId);
	if (confIdLen > MaxConferenceIdSize)
		PASSERTMSG((DWORD)MaxConferenceIdSize,"CCall::SetConferenceId - ConferenceId size not valid");*/
	
	memcpy(m_conferenceId,conferenceId,MaxConferenceIdSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCall::GetReferenceValueForEp()
{
	return m_referenceValueForEp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetReferenceValueForEp(int RefValForEp)
{
	m_referenceValueForEp = RefValForEp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCall::GetReferenceValueForGk()
{
	return m_referenceValueForGk;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetReferenceValueForGk(int RefValForGk)
{
	m_referenceValueForGk = RefValForGk;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
cmCallType CCall::GetCallType()
{
	return m_callType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallType(cmCallType callType)
{
	if ( callType > cmCallTypeN2Nw)
	{
		PASSERTMSG(callType,"CCall::SetCallType - callType not valid");
		return;
	}
	else
		m_callType = callType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
cmCallModelType CCall::GetCallModelType()
{
	return m_model;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallModelType(cmCallModelType callModelType)
{
	if (callModelType > cmCallModelTypeGKRouted)
	{
		PASSERTMSG(callModelType,"CCall::SetCallModelType - callModelType not valid");
		return;
	}
	else
		m_model = callModelType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCall::GetIsActiveMc()
{
	return m_bIsActiveMc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetIsActiveMc(BOOL isActiveMc)
{
//	if (isActiveMc != TRUE && isActiveMc != FALSE)
//	{
//		PASSERTMSG(isActiveMc,"CCall::SetIsActiveMc - isActiveMc not valid");
//		return;
//	}
	m_bIsActiveMc = isActiveMc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCall::GetIsOrigin()
{
	return m_bIsOrigin;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetIsOrigin(BOOL isOrigin)
{
	if (isOrigin != TRUE && isOrigin != FALSE)
	{
		PASSERTMSG(isOrigin,"CCall::SetIIsOrigin - isOrigin not valid");
		return;
	}
	m_bIsOrigin = isOrigin;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCall::GetIsClosingProcess()
{
	return m_bIsClosingProcess;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetIsClosingProcess(BOOL isClosingProcess)
{
	if (isClosingProcess != TRUE && isClosingProcess != FALSE)
	{
		PASSERTMSG(isClosingProcess,"CCall::SetIsClosingProcess - isClosingProcess not valid");
		return;
	}
	m_bIsClosingProcess = isClosingProcess;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetCallCloseInitiator()
{
	return m_callCloseInitiator;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallCloseInitiator(DWORD callCloseInitiator)
{
	if (callCloseInitiator > (DWORD)GkInitiator)
	{
		PASSERTMSG(callCloseInitiator,"CCall::SetCallCloseInitiator - callCloseInitiator not valid");
		m_callCloseInitiator = NoInitiator;
		return;
	}
	m_callCloseInitiator = (initiatorOfClose)callCloseInitiator;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
mcTransportAddress* CCall::GetSetupH245Address()
{
	return &m_setupH245Address;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSetupH245Address(mcTransportAddress setUpH245Addr)
{
	if (setUpH245Addr.ipVersion > eIpVersion6)
	{
		PASSERTMSG((DWORD)setUpH245Addr.ipVersion,"2CCall::SetSetupH45Address - cmTransport type unknown");
		return;
	}
	
	m_setupH245Address.ipVersion = setUpH245Addr.ipVersion;
	if (setUpH245Addr.distribution > eDistributionMulticast)
	{
		PASSERTMSG((DWORD)setUpH245Addr.distribution,"CCall::SetSetupH45Address - Distribution type unknown");
		return;
	}

	m_setupH245Address.distribution = setUpH245Addr.distribution;

	if (setUpH245Addr.transportType > eTransportTypeTls)
	{
		PASSERTMSG((DWORD)setUpH245Addr.transportType,"CCall::SetSetupH45Address - transportType type unknown");
		return;
	}

	m_setupH245Address.transportType = setUpH245Addr.transportType;

	m_setupH245Address.port = setUpH245Addr.port;
	if (m_setupH245Address.ipVersion == eIpVersion4)
	{	// Case IpV4
		m_setupH245Address.addr.v4.ip = setUpH245Addr.addr.v4.ip;
	}
	else
	{		// Case IpV6
		m_setupH245Address.addr.v6.scopeId = setUpH245Addr.addr.v6.scopeId;
		APIU32 i = 0;
		for (i = 0; i < 16; i++)
			m_setupH245Address.addr.v6.ip[i] = setUpH245Addr.addr.v6.ip[i];
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
mcTransportAddress* CCall::GetAnswerH245Address()
{
	return &m_answerH245Address;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetAnswerH245Address(mcXmlTransportAddress answerH245Address)
{
	if (answerH245Address.transAddr.ipVersion > eIpVersion6)
	{
		PASSERTMSG((DWORD)answerH245Address.transAddr.ipVersion,"CCall::SetAnswerH245Address - cmTransport type unknown");
		return;
	}
	
	m_answerH245Address.ipVersion = answerH245Address.transAddr.ipVersion;
	if (answerH245Address.transAddr.distribution > eDistributionMulticast)
	{
		PASSERTMSG((DWORD)answerH245Address.transAddr.distribution,"CCall::SetAnswerH245Address - Distribution type unknown");
		return;
	}

	m_answerH245Address.distribution = answerH245Address.transAddr.distribution;

	if (answerH245Address.transAddr.transportType > eTransportTypeTls)
	{
		PASSERTMSG((DWORD)answerH245Address.transAddr.transportType,"CCall::SetAnswerH245Address - transportType type unknown");
		return;
	}

	m_answerH245Address.transportType = answerH245Address.transAddr.transportType;

	m_answerH245Address.port = answerH245Address.transAddr.port;
	if (m_answerH245Address.ipVersion == eIpVersion4)
	{		// Case IpV4
		m_answerH245Address.addr.v4.ip = answerH245Address.transAddr.addr.v4.ip;
	}
	else
	{		// Case IpV6
		m_answerH245Address.addr.v6.scopeId = answerH245Address.transAddr.addr.v6.scopeId;
		APIU32 i = 0;
		for (i = 0; i < 16; i++)
			m_answerH245Address.addr.v6.ip[i] = answerH245Address.transAddr.addr.v6.ip[i];
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
mcTransportAddress* CCall::GetControlH245Address()
{
	return &m_controlH245Address;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetControlH245Address(mcTransportAddress controlH245Address)
{
	if (controlH245Address.ipVersion > eIpVersion6)
	{
		PASSERTMSG((DWORD)controlH245Address.ipVersion,"CCall::SetControlH245Address - cmTransport type unknown");
		return;
	}
	m_controlH245Address.ipVersion = controlH245Address.ipVersion;
	
	if (controlH245Address.distribution > eDistributionMulticast)
	{
		PASSERTMSG((DWORD)controlH245Address.distribution,"CCall::SetControlH245Address - Distribution type unknown");
		return;
	}
	m_controlH245Address.distribution = controlH245Address.distribution;

	if (controlH245Address.transportType > eTransportTypeTls)
	{
		PASSERTMSG((DWORD)controlH245Address.transportType,"CCall::SetControlH245Address - transportType type unknown");
		return;
	}
	m_controlH245Address.transportType = controlH245Address.transportType;

	m_controlH245Address.port = controlH245Address.port;
	if (m_controlH245Address.ipVersion == eIpVersion4)		// Case IpV4
		m_controlH245Address.addr.v4.ip = controlH245Address.addr.v4.ip;
	else
	{// Case IpV6
		m_controlH245Address.addr.v6.scopeId = controlH245Address.addr.v6.scopeId;
		APIU32 i = 0;
		for (i = 0; i < 16; i++)
			m_controlH245Address.addr.v6.ip[i] = controlH245Address.addr.v6.ip[i];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCall::SetH245Establish(BOOL h245Establish)
{
	if (h245Establish != TRUE && h245Establish != FALSE)
	{
		PASSERTMSG(h245Establish,"CCall::SetH245Establish - h245Establish not valid");
		return;
	}
	m_bH245Establish = h245Establish;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCall::GetH245Establish()
{
	return m_bH245Establish;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetMaxRate()
{
	return m_maxRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetMaxRate( DWORD maxRate)
{
	m_maxRate = maxRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetMinRate()
{
	return m_minRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetMinRate( DWORD minRate)
{
	m_minRate = minRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetRate()
{
	return m_rate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetRate( DWORD rate)
{
	m_rate = rate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetBandwidth()
{
	return m_bandwidth;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetBandwidth( DWORD bandwidth)
{
	m_bandwidth = bandwidth;
}

int CCall::GetMasterSlaveStatus()
{
	return m_masterSlaveStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetMasterSlaveStatus(int masterSlaveStatus)
{
	m_masterSlaveStatus = masterSlaveStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
mcCallTransient CCall::GetCallTransient()
{
	return m_callTransient;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallTransientDisplay(const char* pDisplay)
{
	size_t len = strlen(pDisplay);
	if (len > MaxDisplaySize)
	{
		PASSERTMSG((DWORD)MaxDisplaySize,"CCall::SetCallTransient - callTransient.sDisplay not valid");
		return;
	}
	m_callTransient.sDisplaySize = len;
	memcpy(m_callTransient.sDisplay, pDisplay, MaxDisplaySize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallTransientUserUser(const char* pUserUser)
{
	size_t len = strlen(pUserUser);
	if (len > MaxUserUserSize)
	{
		PASSERTMSG(MaxUserUserSize,"CCall::SetCallTransient - callTransient.userUser not valid");
		return;
	}
	m_callTransient.userUserSize = len;
	memcpy(m_callTransient.userUser, pUserUser, MaxUserUserSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCall::GetCallTransientUserUser()
{
	if (m_callTransient.userUser)
		return m_callTransient.userUser;
	else
		return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetCallTransient(const mcCallTransient callTransient)
{
	if (strlen(callTransient.sDisplay) > MaxDisplaySize)
	{
		PASSERTMSG((DWORD)MaxDisplaySize,"CCall::SetCallTransient - callTransient.sDisplay not valid");
		return;
	}
	if (strlen(callTransient.userUser) > MaxUserUserSize)
	{
		PASSERTMSG(MaxUserUserSize,"CCall::SetCallTransient - callTransient.userUser not valid");
		return;
	}
	memcpy(m_callTransient.sDisplay,callTransient.sDisplay,MaxDisplaySize);
	memcpy(m_callTransient.userUser,callTransient.userUser,MaxUserUserSize);
	m_callTransient.sDisplaySize = callTransient.sDisplaySize;
	m_callTransient.userUserSize = callTransient.userUserSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCall::GetRmtType()
{
	return m_rmtType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetRmtType( DWORD rmtType)
{
	if (rmtType > (DWORD)cmEndpointTypeSET)
	{
		PASSERTMSG(rmtType,"CCall::SetRmtType - rmtType not valid");
		m_rmtType = cmEndpointTypeUndefined;
		return;
	}
	else
		m_rmtType = (cmEndpointType)rmtType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetUserUserNameAndSize(APIS8* userUser, APIS32 userUserSize)
{
	memcpy(m_callTransient.userUser,userUser,userUserSize);
	m_callTransient.userUserSize = userUserSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetDestinationEpType(cmRASEndpointType EpType)
{
	m_destTerminal.endpointType = EpType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSourceEpType(cmRASEndpointType EpType)
{
	m_srcTerminal.endpointType = EpType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetControlH245AddressPort(APIU16 controlPort)
{
	m_controlH245Address.port = controlPort;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSpecificChannel(APIS32 index,CChannel* pChannel)
{
	if(index < MaxChannelsPerCall)
	{
		m_pChannelsArray[index] = pChannel;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetSrcTerminalPartyAddr(const char* srcparty, APIU32 size)
{
	memcpy(m_srcTerminal.partyAddress,srcparty,size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetDestTerminalPartyAddr(const char* destparty, APIU32 size)
{
	memcpy(m_destTerminal.partyAddress,destparty,size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetChannelsCounter(DWORD counter)
{
	m_channelsCounter = counter;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCall::SetLprCapStruct(lprCapCallStruct* lprStruct, BYTE direction)
{
	if (direction > 1)
		PASSERT(direction);
	else
	{
		m_lprCapStruct[direction].versionID = lprStruct->versionID;
		m_lprCapStruct[direction].minProtectionPeriod = lprStruct->minProtectionPeriod;
		m_lprCapStruct[direction].maxProtectionPeriod = lprStruct->maxProtectionPeriod;
		m_lprCapStruct[direction].maxRecoverySet = lprStruct->maxRecoverySet;
		m_lprCapStruct[direction].maxRecoveryPackets = lprStruct->maxRecoveryPackets;
		m_lprCapStruct[direction].maxPacketSize = lprStruct->maxPacketSize;
	}
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
lprCapCallStruct* CCall::GetLprCapStruct(BYTE direction)
{
	if (direction > 1)
	{
		PASSERT(direction);
		return NULL;
	}
	
	return &(m_lprCapStruct[direction]);
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
CChannel* CCall::GetInternalChannel(cmCapDataType eMediaType, BOOL aIsTransmitDirection, ERoleLabel eRole, DWORD aRtpConnectionId) const
{
    CChannel* pChannel      = NULL;
    BYTE      bChannelFound = NO;

    TRACEINTO << "mix_mode: Looking for internal channel eMediaType=" << ::GetTypeStr(eMediaType)
              << " eDirection=" << (aIsTransmitDirection?"Transmit":"Receive") << " eRole=" << ::GetRoleStr(eRole) << " aRtpConnectionId=" << aRtpConnectionId;

    for (int i = 0; i < MAX_INTERNAL_CHANNELS && bChannelFound == NO; i++)
    {
        if (m_pInternalChannelsArray[i] &&
            m_pInternalChannelsArray[i]->GetType() == eMediaType &&
            ((m_pInternalChannelsArray[i]->GetRoleLabel() == eRole) || (m_pInternalChannelsArray[i]->GetRoleLabel() & eRole)) &&
            (m_pInternalChannelsArray[i]->IsOutgoingDirection() == aIsTransmitDirection) &&
            (aRtpConnectionId == m_pInternalChannelsArray[i]->GetRtpConnectionId()))
        {
            pChannel = m_pInternalChannelsArray[i];
            bChannelFound = YES;
            TRACEINTO << "Channel found";
        }
    }
    return pChannel;
}

///////////////////////////////////////////////////////////////////////
CChannel* CCall::GetInternalChannelBySsrc(cmCapDataType eMediaType, BOOL eDirection, ERoleLabel eRole, DWORD aSsrcId) const
{
    CChannel* pChannel      = NULL;
    BYTE      bChannelFound = NO;

    TRACEINTO << "mix_mode: Looking for internal channel eMediaType=" << ::GetTypeStr(eMediaType)
              << " eDirection=" << (eDirection?"Transmit":"Receive") << " eRole=" << ::GetRoleStr(eRole) << " aSsrcId=" << aSsrcId;

    for (int i = 0; i < MAX_INTERNAL_CHANNELS && bChannelFound == NO; i++)
    {
        if (m_pInternalChannelsArray[i] &&
            m_pInternalChannelsArray[i]->GetType() == eMediaType &&
            ((m_pInternalChannelsArray[i]->GetRoleLabel() == eRole) || (m_pInternalChannelsArray[i]->GetRoleLabel() & eRole)) &&
            (m_pInternalChannelsArray[i]->IsOutgoingDirection() == eDirection))
        {
            // check ssrc
            const std::list <StreamDesc> streamsDescList = m_pInternalChannelsArray[i]->GetStreams();
            std::list <StreamDesc>::const_iterator itr_streams;

            for(itr_streams=streamsDescList.begin(); itr_streams!=streamsDescList.end(); itr_streams++)
            {
                if (itr_streams->m_pipeIdSsrc == aSsrcId || aSsrcId == INVALID)
                {
                    pChannel = m_pInternalChannelsArray[i];
                    bChannelFound = YES;
                    TRACEINTO << "Channel found";
                    break;
                }
            }
        }
    }
    return pChannel;
}


//////////////////////////////////////////////////////////////////////
// Media detection, modified from original in SIP
// Return Value: TRUE: disconnect call
//				 FALSE: not disconnect call
BYTE CCall::HandleMediaDetectionInd(kChanneltype  ChannelType, BYTE  isRTP)
{
	PTRACE(eLevelInfoNormal,"CCall::HandleMediaDetectionInd - H323 version");
	time_t now 			= time((time_t*)NULL);
	int connectionIndex	=0;
	BYTE disconnectCall	= FALSE;
	bool errFound 		= FALSE;

	switch (ChannelType)
	{
		case kIpAudioChnlType:
			{
				if(isRTP)
					m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_AUDIO_RTP] = now;
				else
					m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_AUDIO_RTCP] = now;
			}
			break;
		case kIpVideoChnlType:
			{
				if(FALSE == GetMediaDetectionHasVideo())
				{
					PTRACE(eLevelInfoNormal,"CCall::HandleMediaDetectionInd no Video Channel being monitored! Ignore this indication!");
					errFound = TRUE;
				}
				else
				{
					if(isRTP)
						m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_VIDEO_RTP] = now;
					else
						m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_VIDEO_RTCP] = now;
				}
			}
			break;
		default:
			PTRACE(eLevelInfoNormal,"CCall::HandleMediaDetectionInd -- NOT the expected indication!");
			break;
	}

	//=========================================================================================================================
	// Max monitored connection, simplified with SVC removed.  Simplified logic: accumulate 2/4 media disconnection ind-s for
	// audio-only/video call accordingly.  If all ind-s fall in the recent time window, fail the call
	//=========================================================================================================================
	int	MaxMonitoredConnection = MEDIA_DETECTION_MAX_CONNECTION;  // The default value, check all the 4 channels for AVC video calls
	if(!GetMediaDetectionHasVideo())
	{
		MaxMonitoredConnection = MEDIA_DETECTION_VIDEO_RTCP;
		//Only Audio connections, set the max connection number as the first Video connection
	}

	for(connectionIndex=0; !errFound && connectionIndex<MaxMonitoredConnection &&
	                       m_stMediaDetectInfo.MediaDisconnectedRecently(connectionIndex, now);
						   connectionIndex++)
	{
		if(now < m_stMediaDetectInfo.lastIndTime[connectionIndex])
		{
			PTRACE2INT(eLevelError, "CCall::HandleMediaDetectionInd -- ERROR!-- ", m_stMediaDetectInfo.lastIndTime[connectionIndex]);
			errFound = TRUE;
		}
	}

	if(!errFound && MaxMonitoredConnection == connectionIndex)
	{
		PTRACE(eLevelInfoNormal, "CCall::HandleMediaDetectionInd -- Disconnect call!!!!");
		disconnectCall = TRUE;
	}

	return disconnectCall;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  Class CChannel /////////////////////////////////////////////////////////
CChannel::CChannel()
{
	m_pCall = NULL;									
	m_index = 0;										
	m_csIndex = 0;
	m_status = 0;
	m_directionInOut = FALSE;
	m_bIsActive = FALSE;									
	m_channelCloseInitiator = NoInitiator;
	m_payloadType = _UnKnown;
	m_nameEnum = eUnknownAlgorithemCapCode;
	m_dataType = cmCapEmpty;
	m_roleLabel = kRolePeople;		
	m_sizeOfChannelParams = 0;
	m_pChannelParams = NULL;
	m_rate = 0;				
	m_maxSkew = 0;				
	m_eCsChannelState = kDisconnectedState;					
	m_eStreamState = kStreamOffState;	
	m_bIsRejectChannel = FALSE;
	m_bIsStreamOnSent = FALSE;
	m_bIsEncrypted = 0;
	m_encryType = kUnKnownMediaType;
	m_sessionId = 0;
	m_isDbc2       = FALSE;
	m_dynamicPayloadType 	   = 0;
	m_bIsH263Plus        	   = 0;	
	m_eCmUdpChannelState  	   = kNotSendOpenYet;
//	m_eCsChannelDisconnectCase = kNotCsChannelDisconnectCase;
	memset(m_sessionKey,'0',sizeOf128Key);
	memset(m_encSessionKey,'0',sizeOf128Key);	
	memset(&m_RmtAddress,   0, sizeof(mcTransportAddress));	
	
	m_rtcpPort = 0;
	m_rtcpRmtPort = 0;
	m_channelHandle = 0;
	m_eRtpPortChannelState = kRtpPortNotSendOpenYet;
	
	m_isSupportLpr = 0;
	m_seqNumRtp = 0;
	m_seqNumCm = 0;
    m_rtpConnectionId = INVALID;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CChannel::~CChannel()
{
	PDELETEA(m_pChannelParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::UpdateChannelParams(CCall *pCall,int channelArrayIndex, BOOL bIsOutgoingChannel, CCapSetInfo capInfo,ERoleLabel eRole,
					   DWORD rate, mcIndIncomingChannel *pinChnlInd, EenMediaType encMediaType, APIU32 incomingChnlStatus)
{
	if ( ! CPObject::IsValidPObjectPtr(pCall))
	{
		PASSERTMSG(1,"CChannel::UpdateChannelParams - pCall is not valid");
		return;
	}
	if (channelArrayIndex < 0)
	{
		PASSERTMSG(2,"CChannel::UpdateChannelParams - Channel Array not valid");
		return;
	}
	if (bIsOutgoingChannel != TRUE && bIsOutgoingChannel != FALSE)
	{
		PASSERTMSG(bIsOutgoingChannel,"CChannel::UpdateChannelParams - bIsOutgoingChannel not valid");
		return;
	}
	if (eRole > kRoleUnknown)
	{
		PASSERTMSG((DWORD)eRole,"CChannel::UpdateChannelParams - Role label not valid");
		return;
	}
	if (encMediaType > kAES_CTR)
	{
		PASSERTMSG((DWORD)encMediaType,"CChannel::UpdateChannelParams -  Enc type not valid");
		return;
	}
	
	m_pChannelParams					= NULL;
	m_pCall								= pCall;
	m_index 							= (DWORD)channelArrayIndex;
	m_eCsChannelState					= kConnectingState;
	m_eStreamState						= kStreamOffState;
	m_channelCloseInitiator				= NoInitiator;
	m_maxSkew							= 0;
	m_directionInOut					= bIsOutgoingChannel;		//  Cs response
	m_payloadType						= capInfo.GetPayloadType();
	m_nameEnum							= (CapEnum)capInfo;
	m_dataType							= capInfo.GetCapType(); 
	m_roleLabel						    = eRole;
	m_bIsRejectChannel					= FALSE;
	m_bIsStreamOnSent					= FALSE;
	m_bIsEncrypted						= FALSE;
	m_encryType							= kUnKnownMediaType;
	
	
	if(bIsOutgoingChannel)
	{
		m_sizeOfChannelParams				= 0;
		m_bIsActive							= TRUE;
		m_csIndex							= 0;
		m_status							= 0;
		m_sessionId							= -1;

		if(encMediaType != kUnKnownMediaType)
		{
			m_bIsEncrypted			= TRUE;
			m_encryType				= encMediaType;
		}
	}
	else
	{
		m_sizeOfChannelParams				= pinChnlInd->sizeOfChannelParams;
		m_bIsActive							= pinChnlInd->bIsActive;
		m_sessionId							= pinChnlInd->sessionId;
		m_csIndex							= pinChnlInd->channelIndex;
		m_status							= incomingChnlStatus;
		m_bIsEncrypted						= pinChnlInd->bIsEncrypted;
		m_encryType							= (EenMediaType)pinChnlInd->encryptionAlgorithm;
	}
	
	m_rate = rate;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetIndex()
{
	return m_index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIndex(DWORD index)
{
	m_index = index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetCsIndex()
{
	return m_csIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetCsIndex(DWORD csIndex)
{
	m_csIndex = csIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
int CChannel::GetStatus()
{
	return m_status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetStatus(int status)
{
	m_status = status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::IsOutgoingDirection()
{
	return m_directionInOut;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
cmCapDirection CChannel::GetChannelDirection()
{
	cmCapDirection eDirection = IsOutgoingDirection()? cmCapTransmit: cmCapReceive;
	return eDirection;
}
	

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetChannelDirection(BOOL channelDirection)
{
/*	if (channelDirection != TRUE && channelDirection != FALSE)
	{
		PASSERTMSG(channelDirection,"CChannel::SetChannelDirection - channelDirection not valid");
		return;
	}*/
	m_directionInOut = channelDirection;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::GetIsActive()
{
	return m_bIsActive;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsActive(BOOL isActive)
{
/*	if (isActive != TRUE && isActive != FALSE)
	{
		PASSERTMSG(isActive,"CChannel::SetIsActive - isActive not valid");
		return;
	}
*/
	m_bIsActive = isActive;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetChannelCloseInitiator()
{
	return (DWORD)m_channelCloseInitiator;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetChannelCloseInitiator(DWORD channelCloseInitiator)
{
	if (channelCloseInitiator > (DWORD)GkInitiator)
	{
		PASSERTMSG(channelCloseInitiator,"CChannel::SetChannelCloseInitiator - Channel close initiator not valid");
		m_channelCloseInitiator = NoInitiator;
	}
	else
		m_channelCloseInitiator = (initiatorOfClose)channelCloseInitiator;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetPayloadType()
{
	return (DWORD)m_payloadType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetPayloadType(DWORD payloadType)
{
	m_payloadType = (payload_en)payloadType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CapEnum CChannel::GetCapNameEnum()
{
	return m_nameEnum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetCapNameEnum(CapEnum capNameEnum)
{
	if (capNameEnum > eUnknownAlgorithemCapCode)
	{
		PASSERTMSG(capNameEnum,"CChannel::SetCapNameEnum - Cap Enum not valid");
		m_nameEnum = eUnknownAlgorithemCapCode;
	}
	else
		m_nameEnum = capNameEnum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
cmCapDataType CChannel::GetType()
{
	return m_dataType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetType(cmCapDataType dataType)
{
	if (dataType > cmCapAudioTone)
	{
		PASSERTMSG((DWORD)dataType,"CChannel::SetType - Data type not valid");
		m_dataType = cmCapEmpty;
	}
	else
		m_dataType = dataType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
ERoleLabel CChannel::GetRoleLabel()
{
	return m_roleLabel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetRoleLabel(ERoleLabel roleLabel)
{
	if (roleLabel >= kRoleUnknown)
	{
		PASSERTMSG(roleLabel,"CChannel::SetRoleLabel - Role label not valid");
		m_roleLabel = kRoleUnknown;
	}
	else
	{
		m_roleLabel = roleLabel;	
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetSizeOfChannelParams()
{
	return m_sizeOfChannelParams;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetSizeOfChannelParams(DWORD sizeOfChannelParams)
{
	m_sizeOfChannelParams = sizeOfChannelParams;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CChannel::GetChannelParams()
{
	return m_pChannelParams;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetChannelParams(DWORD size, char *pChannelParams)
{
	if (m_pChannelParams)
		PDELETEA(m_pChannelParams);
	if(size)
	{
		m_pChannelParams = (char *)new BYTE[size];
		memset(m_pChannelParams, 0, size);
		memcpy(m_pChannelParams, pChannelParams, size);
	}
	SetSizeOfChannelParams(size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetRate()
{
	return m_rate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetRate(DWORD rate)
{
	m_rate = rate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CChannel::GetMaxSkew()
{
	return m_maxSkew;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetMaxSkew(DWORD maxSkew)
{
	m_maxSkew = maxSkew;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CChannel::IsChannelConnected()
{
	return IsChannelCsConnected() && IsChannelCmConnected();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CChannel::IsChannelCsConnected()
{
	return (m_eCsChannelState == kConnectedState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
ECsChannelState CChannel::GetCsChannelState()
{
	return m_eCsChannelState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetCsChannelState(ECsChannelState eCsChannelState)
{
	m_eCsChannelState = eCsChannelState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::IsCsChannelStateConnecting()
{
	return (m_eCsChannelState > kFirstConnectingState && m_eCsChannelState < kLastConnectingState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::IsCsChannelStateDisconnecting()
{
	return (m_eCsChannelState > kFirstDisconnectingState && m_eCsChannelState < kLastDisconnectingState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
EStreamState CChannel::GetStreamState()
{
	return m_eStreamState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetStreamState(EStreamState streamState)
{
	m_eStreamState = streamState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::GetIsRejectChannel()
{
	return m_bIsRejectChannel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsRejectChannel(BOOL isRejectChannel)
{
	if (isRejectChannel != TRUE && isRejectChannel != FALSE)
	{
		PASSERTMSG(isRejectChannel,"CChannel::SetIsRejectChannel - isRejectChannel not valid");
		return;
	}
	m_bIsRejectChannel = isRejectChannel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::GetIsStreamOnSent()
{
	return m_bIsStreamOnSent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsStreamOnSent(BOOL isStreamOnSent)
{
	if (isStreamOnSent != TRUE && isStreamOnSent != FALSE)
	{
		PASSERTMSG(isStreamOnSent,"CChannel::SetIsStreamOnSent - isStreamOnSent not valid");
		return;
	}

	m_bIsStreamOnSent = isStreamOnSent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChannel::GetIsEncrypted()
{
	return m_bIsEncrypted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsEncrypted(BOOL isEncrypted)
{
	if (isEncrypted != TRUE && isEncrypted != FALSE)
	{
		PASSERTMSG(isEncrypted,"CChannel::SetIsEncrypted - isEncrypted not valid");
		return;
	}

	m_bIsEncrypted = isEncrypted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
EenMediaType CChannel::GetEncryptionType()
{
	return m_encryType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetEncryptionType(EenMediaType encType)
{
	if (encType > kAES_CTR)
	{
		PASSERTMSG((DWORD)encType,"CChannel::SetEncryptionType -  Enc type not valid");
		m_encryType = kUnKnownMediaType;
	}
	else
		m_encryType = encType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CChannel::GetSessionId()
{
	return m_sessionId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetSessionId(APIS32 sessionId)
{
	m_sessionId = sessionId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
APIS8 CChannel::GetIsDbc2()
{
	return m_isDbc2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsDbc2(APIS8 isDbc2)
{
	m_isDbc2 = isDbc2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetCallPointer(CCall *pCall)
{
	m_pCall = pCall;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CChannel::GetDynamicPayloadType()
{
	return m_dynamicPayloadType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetDynamicPayloadType(APIU8 dynamicPayloadType)
{
	m_dynamicPayloadType = dynamicPayloadType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsH263Plus(APIU8 bIsH263Plus)
{
	m_bIsH263Plus = bIsH263Plus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CChannel::IsH263Plus()
{
	return m_bIsH263Plus;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetCmUdpChannelState(ECmUdpChannelState eCmState)
{
	m_eCmUdpChannelState = eCmState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
ECmUdpChannelState CChannel::GetCmUdpChannelState()
{
	return m_eCmUdpChannelState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CChannel::IsChannelCmConnected()
{
	return (m_eCmUdpChannelState == kRecieveOpenAck);
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetCsChannelDisconnectCase(ECsChannelDisconnectCase eCsChannelDisconnectCase)
{
	m_eCsChannelDisconnectCase = eCsChannelDisconnectCase;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
ECsChannelDisconnectCase CChannel::GetCsChannelDisconnectCase()
{
	return m_eCsChannelDisconnectCase;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function only for test */
int CCall::TestUpdateMcChannelParams(BOOL bIsTransmit, CChannel *&pMcChannel, CCapSetInfo capInfo,ERoleLabel eRole,
								DWORD rate, mcIndIncomingChannel *pinChnlInd, EenMediaType encType, APIS32 status)
{
	pMcChannel = new CChannel;
	int i = 0;

	// enter the channel structure to the channel array
	for (i = 0; i < MaxChannelsPerCall; i++)
	{
		if (!m_pChannelsArray[i]) 
		{
			m_pChannelsArray[i] = pMcChannel;
			break; 
		}
	}
	if (i == MaxChannelsPerCall)
	{
		PASSERTMSG(102,"CCall::TestUpdateMcChannelParams: channel number exceeded");
		return i;
	}

	IncreaseChannelsCounter();
	(m_pChannelsArray[i])->TestUpdateChannelParams(this,m_channelsCounter,bIsTransmit,capInfo,eRole,rate,pinChnlInd,encType,status);
	return i;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* function only for test!! */
void CChannel::TestUpdateChannelParams(CCall *pCall,int channelArrayIndex, BOOL bIsOutgoingChannel, CCapSetInfo capInfo,ERoleLabel eRole,
					   DWORD rate, mcIndIncomingChannel *pinChnlInd, EenMediaType encMediaType, APIU32 incomingChnlStatus)
{
	if ( ! CPObject::IsValidPObjectPtr(pCall))
	{
		PASSERTMSG(1,"CChannel::UpdateChannelParams - pCall is not valid");
		return;
	}
	if (channelArrayIndex < 0)
	{
		PASSERTMSG(2,"CChannel::UpdateChannelParams - Channel Array not valid");
		return;
	}
	if (bIsOutgoingChannel != TRUE && bIsOutgoingChannel != FALSE)
	{
		PASSERTMSG(bIsOutgoingChannel,"CChannel::UpdateChannelParams - bIsOutgoingChannel not valid");
		return;
	}
	if (eRole > kRoleUnknown)
	{
		PASSERTMSG((DWORD)eRole,"CChannel::UpdateChannelParams - Role label not valid");
		return;
	}
	if (encMediaType > kAES_CTR)
	{
		PASSERTMSG((DWORD)encMediaType,"CChannel::UpdateChannelParams -  Enc type not valid");
		return;
	}
	
	m_pChannelParams					= NULL;
	m_pCall								= pCall;
	m_index 							= (DWORD)channelArrayIndex;
	m_eCsChannelState					= kConnectingState;
	m_eStreamState						= kStreamOffState;
	m_channelCloseInitiator				= NoInitiator;
	m_maxSkew							= 0;
	m_directionInOut					= bIsOutgoingChannel;		//  Cs response
	m_payloadType						= capInfo.GetPayloadType();
	m_nameEnum							= (CapEnum)capInfo;
	m_dataType							= capInfo.GetCapType(); 
	m_roleLabel						    = eRole;
	m_bIsRejectChannel					= FALSE;
	m_bIsStreamOnSent					= FALSE;
	m_bIsEncrypted						= FALSE;
	m_encryType							= kUnKnownMediaType;	
	
	if(bIsOutgoingChannel)
	{
		m_sizeOfChannelParams				= 0;
		m_bIsActive							= TRUE;
		m_csIndex							= 0;
		m_status							= 0;
		m_sessionId							= -1;

		if(encMediaType != kUnKnownMediaType)
		{
			m_bIsEncrypted			= TRUE;
			m_encryType				= encMediaType;
		}
	}
	else
	{
		m_sizeOfChannelParams				= pinChnlInd->sizeOfChannelParams;
		m_bIsActive							= pinChnlInd->bIsActive;
		m_sessionId							= pinChnlInd->sessionId;
		m_csIndex							= pinChnlInd->channelIndex;
		m_status							= incomingChnlStatus;
		m_bIsEncrypted						= pinChnlInd->bIsEncrypted;
		m_encryType							= (EenMediaType)pinChnlInd->encryptionAlgorithm;
	}
	
	m_rate = rate;
}

///////////////////////////////////////////////////////////////////////////////////
void CChannel::SetH235SessionKey(APIU8*	pSessionKey)
{
	memset(m_sessionKey,'0',sizeOf128Key);	
	memcpy(m_sessionKey, pSessionKey, sizeOf128Key);
}
///////////////////////////////////////////////////////////////////////////////////
APIU8* CChannel::GetH235SessionKey()
{
	return m_sessionKey;
}
///////////////////////////////////////////////////////////////////////////////////
void CChannel::SetH235EncryptedSessionKey(APIU8*	pEncSessionKey)
{
	memset(m_encSessionKey,'0',sizeOf128Key);	
	memcpy(m_encSessionKey, pEncSessionKey, sizeOf128Key);
}
///////////////////////////////////////////////////////////////////////////////////	
APIU8* CChannel::GetH235EncryptedSessionKey()
{
	return m_encSessionKey;
}

///////////////////////////////////////////////////////////////////////////////////
void CChannel::SetRtpPortChannelState(EArtRtpPortChannelState eRtpPortState)
{
	m_eRtpPortChannelState = eRtpPortState;
	Dump("CChannel::SetRtpPortChannelState");
}
///////////////////////////////////////////////////////////////////////////////////
EArtRtpPortChannelState CChannel::GetRtpPortChannelState()
{
	return m_eRtpPortChannelState;
}

///////////////////////////////////////////////////////////////////////////////////
void CChannel::SetRmtAddress(mcTransportAddress pRmtTrAddr)
{
	memcpy(&m_RmtAddress,&pRmtTrAddr,sizeof(mcTransportAddress));
}



///////////////////////////////////////////////////////////////////////////////////
void CChannel::SetIsLprSupported(WORD isSupportLpr)
{
	m_isSupportLpr = isSupportLpr;
}

///////////////////////////////////////////////////////////////////////////////////
WORD CChannel::GetIsLprSupported()
{
	return m_isSupportLpr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::Dump(char * header)
{
    CSuperLargeString str;
    str << "CChannel::Dump " << header << "\n";

    str << "m_arrIndexInCall      = " << this->GetIndex() << "\n"
        << "m_mcmsIndex           = " << this->GetCsIndex() << "\n"
        << "m_eMediaType          = " << ::GetTypeStr(this->GetType()) << "\n"
        << "m_eDirection          = " << ::GetDirectionStr(this->GetChannelDirection()) << "\n"
        << "m_eRole               = " << ::GetRoleStr(this->GetRoleLabel()) << "\n"
        << "m_eCsChannelState    = " << ::CsChannelStateToString(this->GetCsChannelState()) << "\n"
        << "m_eRtpConnectionState = " << ::ArtRtpPortChannelStateToString(GetRtpPortChannelState()) << "\n"
        << "m_eCmUdpChannelState  = " << ::CmUdpChannelStateToString(this->GetCmUdpChannelState()) << "\n";

    if (m_rtpConnectionId != INVALID)
        str << "m_rtpConnectionId  = " << m_rtpConnectionId << "\n";

     if(m_streams.size()!=0)
     {
         COstrStream msg1;
         DumpStreamsList(msg1);
         str << msg1.str().c_str();
     }
     else {
         str << "There are no streams \n";
     }

    PTRACE (eLevelInfoNormal, str.GetString());
}

void CChannel::DumpStreamsList(std::ostream& msg)
{
    if (m_streams.empty())
        return;

    msg << "\n- - - - Streams list : - - - - - - - -" << "\n";
    std::list<StreamDesc>::const_iterator itr_streams;
    for (itr_streams = m_streams.begin();itr_streams != m_streams.end();itr_streams++)
    {
        msg << "ssrc:" << itr_streams->m_pipeIdSsrc;
        if (this->GetType()==cmCapVideo)
        {
            msg << " width:" << itr_streams->m_width;
            msg << " height:" << itr_streams->m_height;
            msg << " frame rate:" << itr_streams->m_frameRate;
        }
        msg << " m_bitRate:" << itr_streams->m_bitRate;
        if (itr_streams->m_specificSourceSsrc)
            msg << " specified ssrc:" << itr_streams->m_sourceIdSsrc;

        msg << " payload type:" << itr_streams->m_payloadType;
        msg << " priority: " << itr_streams->m_priority;
        msg << " isLegal: " << itr_streams->m_isLegal;
        msg << " isAvcToSvcVsw: " << itr_streams->m_isAvcToSvcVsw;

        msg << "\n";
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CChannel::SetStreamsList(const std::list <StreamDesc>&  rStreams, APIU32 aSsrcId)
{
    TRACEINTO << "aSsrcId = " << aSsrcId;

    m_streams.clear();
    if (aSsrcId == INVALID)
    {
        m_streams.assign(rStreams.begin(), rStreams.end());
        Dump("CSipChannel::SetStreamsList");
        return;
    }

    std::list<StreamDesc>::const_iterator itr_streams;
    for (itr_streams = rStreams.begin(); itr_streams != rStreams.end(); itr_streams++)
    {
        if (itr_streams->m_pipeIdSsrc == aSsrcId)
        {
            TRACEINTO << "Adding stream " << aSsrcId << " to channel";
            m_streams.push_back(*itr_streams);
        }
    }
    Dump("CSipChannel::SetStreamsList");
}
