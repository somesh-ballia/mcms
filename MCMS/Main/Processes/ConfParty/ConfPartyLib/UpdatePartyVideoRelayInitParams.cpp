#include "UpdatePartyVideoRelayInitParams.h"


///////////////////////////////////////////////////////////////////////////////////////////
CUpdatePartyVideoRelayInitParams::CUpdatePartyVideoRelayInitParams()
{

}
///////////////////////////////////////////////////////////////////////////////////////////
CUpdatePartyVideoRelayInitParams::CUpdatePartyVideoRelayInitParams(const CBridgePartyInitParams& pPartyInitParams)
{
	m_UpdateInParams = NULL;
	m_UpdateOutParams = NULL;

	m_pConfAppBridgeParams = new CConfAppBridgeParams();
	InitConfAppParams(pPartyInitParams.GetConfAppParams());

	m_pConfName = pPartyInitParams.GetConfName() ;
	m_pPartyName = pPartyInitParams.GetPartyName();

	if(pPartyInitParams.GetMediaInParams())
	{
		m_UpdateInParams = new CBridgePartyVideoRelayMediaParams(*(CBridgePartyVideoRelayMediaParams*)pPartyInitParams.GetMediaInParams());
	}

	if(pPartyInitParams.GetMediaOutParams())
	{
		m_UpdateOutParams = new CBridgePartyVideoRelayMediaParams(*(CBridgePartyVideoRelayMediaParams*)pPartyInitParams.GetMediaOutParams());

	}

}
///////////////////////////////////////////////////////////////////////////////////////////
CUpdatePartyVideoRelayInitParams::~CUpdatePartyVideoRelayInitParams()
{

}
///////////////////////////////////////////////////////////////////////////////////////////
void CUpdatePartyVideoRelayInitParams::AllocateInParams()
{

	if(m_UpdateInParams == NULL)
		m_UpdateInParams = new CBridgePartyVideoRelayMediaParams;

}
///////////////////////////////////////////////////////////////////////////////////////////
void CUpdatePartyVideoRelayInitParams::InitiateMediaOutParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyMediaParams)
{
	AllocateOutParams();
	*(CBridgePartyVideoRelayMediaParams *) m_UpdateOutParams = *pBridgePartyMediaParams;
}
///////////////////////////////////////////////////////////////////////////////////////////
void CUpdatePartyVideoRelayInitParams::InitiateMediaInParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyMediaParams)
{
	AllocateInParams();
	*(CBridgePartyVideoRelayMediaParams *) m_UpdateInParams = *pBridgePartyMediaParams;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CUpdatePartyVideoRelayInitParams::AllocateOutParams()
{
	if(m_UpdateOutParams == NULL)
			m_UpdateOutParams = new CBridgePartyVideoRelayMediaParams;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CUpdatePartyVideoRelayInitParams::InitiateMediaOutParams(const CBridgePartyVideoOutParams* pBridgePartyMediaParams)
{
	//assert and do nothing need to use the relay type
	PTRACE(eLevelError, "CUpdatePartyVideoRelayInitParams::InitiateMediaOutParams for relay wrong function, need to send with pBridgePartyMediaParams params");
}
///////////////////////////////////////////////////////////////////////////////////////////
void CUpdatePartyVideoRelayInitParams::InitiateMediaInParams(const CBridgePartyVideoInParams* pBridgePartyMediaParams)
{
	//assert and do nothing need to use the relay type
	PTRACE(eLevelError, "CUpdatePartyVideoRelayInitParams::InitiateMediaOutParams for relay wrong function, need to send with pBridgePartyMediaParams params");
}


///////////////////////////////////////////////////////////////////////////////////////////

EStat CUpdatePartyVideoRelayInitParams::UpdateVideoInParams(const CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	//assert and do nothing need to use the relay type
	PTRACE(eLevelError, "CUpdatePartyVideoRelayInitParams::UpdateVideoInParams for relay wrong function, need to send with pBridgePartyMediaParams params");
	return statIllegal;
}
///////////////////////////////////////////////////////////////////////////////////////////

EStat CUpdatePartyVideoRelayInitParams::UpdateVideoOutParams(const CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	//assert and do nothing need to use the relay type
	PTRACE(eLevelError, "CUpdatePartyVideoRelayInitParams::UpdateVideoOutParams for relay wrong function, need to send with pBridgePartyMediaParams params");
	return statIllegal;
}
///////////////////////////////////////////////////////////////////////////////////////////

 EStat CUpdatePartyVideoRelayInitParams::UpdateVideoInParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyVideoParams)

 {
 	 PTRACE2(eLevelInfoNormal,"CUpdatePartyVideoRelayInitParams::UpdateVideoInParams : Name - ",m_pPartyName);
 	 EStat responseStatus = statOK;
		if(!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "CUpdatePartyVideoRelayInitParams::UpdateVideoInParams : Internal Error receive invalid params");
		responseStatus = statIllegal;
	}
	else
	{
		*((CBridgePartyVideoRelayMediaParams*)m_UpdateInParams) = *pBridgePartyVideoParams;
	}

	return responseStatus;

 }
 ///////////////////////////////////////////////////////////////////////////////////////////
EStat CUpdatePartyVideoRelayInitParams::UpdateVideoOutParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyVideoParams)
{

	PTRACE2(eLevelInfoNormal,"CUpdatePartyVideoRelayInitParams::UpdateVideoOutParams : Name - ",m_pPartyName);
	EStat responseStatus = statOK;
	if(!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "CUpdatePartyVideoRelayInitParams::UpdateVideoOutParams : Internal Error receive invalid params");
		responseStatus = statIllegal;
	}
	else
	{
		*((CBridgePartyVideoRelayMediaParams*)m_UpdateOutParams) = *pBridgePartyVideoParams;
	}

	return responseStatus;
}


