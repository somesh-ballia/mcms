#include <sstream>
#include "SvcToAvcParams.h"
#include "Image.h"
#include "Trace.h"
#include "TraceStream.h"
#include "H264.h"
#include "ConfPartyGlobals.h"


#define SVC_TO_AVC_REQUIRED_TRANSLATED_LAYER_ID 3



////////////////////////////////////////////////
///             CSvcToAvcParams              ///
////////////////////////////////////////////////
CSvcToAvcParams::CSvcToAvcParams()
{
	m_bIsSupportSvcAvcTranslate = false;
	m_pTranslatedVideoRelayMediaStream = NULL;
	m_pVideoOperationPointsSet = NULL;//a pointer to the conf video operation point, here we dont allocate
	m_videoRelayInchannelHandle = INVALID;
	m_bIsAvcInConf = false;
	m_maxAllowedLayerId = 0;
	m_maxAllowedLayerIdHighRes = 0;
	m_bIsHighResStreamEnable = FALSE;
}
//----------------------------------------------
CSvcToAvcParams::~CSvcToAvcParams ()
{
  POBJDELETE(m_pTranslatedVideoRelayMediaStream);
}
//----------------------------------------------
CSvcToAvcParams::CSvcToAvcParams(const CSvcToAvcParams& svcToAvcParams): CPObject(svcToAvcParams)
{
	m_pTranslatedVideoRelayMediaStream = NULL;
	m_pVideoOperationPointsSet = NULL;
	m_videoRelayInchannelHandle = INVALID;
	m_maxAllowedLayerId = 0;
	m_maxAllowedLayerIdHighRes = 0;
	m_bIsHighResStreamEnable = FALSE;
	
	m_bIsSupportSvcAvcTranslate = svcToAvcParams.m_bIsSupportSvcAvcTranslate;
	if(IsValidPObjectPtr(svcToAvcParams.m_pTranslatedVideoRelayMediaStream))
	{
		m_pTranslatedVideoRelayMediaStream = new CVideoRelayMediaStream(*(svcToAvcParams.m_pTranslatedVideoRelayMediaStream));
	}
	else
		m_pTranslatedVideoRelayMediaStream = NULL;
	if(IsValidPObjectPtr(svcToAvcParams.m_pVideoOperationPointsSet))
	{
		m_pVideoOperationPointsSet = svcToAvcParams.m_pVideoOperationPointsSet;
	}
	else
		m_pVideoOperationPointsSet = NULL;
	m_videoRelayInchannelHandle = svcToAvcParams.m_videoRelayInchannelHandle;
	m_bIsAvcInConf = svcToAvcParams.m_bIsAvcInConf;
}
//----------------------------------------------
void CSvcToAvcParams::Dump() const
{
	std::ostringstream str;

	str << "CSvcToAvcParams:";
	str << "\n m_bIsSupportSvcAvcTranslate  = " << m_bIsSupportSvcAvcTranslate;
	str << "\n m_maxAllowedLayerId            = " << (DWORD)m_maxAllowedLayerId;
	str << "\n m_maxAllowedLayerIdHighRes     = " << (DWORD)m_maxAllowedLayerIdHighRes;
	str << "\n m_videoRelayInchannelHandle  = " << m_videoRelayInchannelHandle;
	str << "\n m_bIsAvcInConf               = " << m_bIsAvcInConf;
	if(m_pTranslatedVideoRelayMediaStream)
	{
		str << "\n m_pTranslatedVideoRelayMediaStream: ";
		str << "\n\t m_ssrc		       : " << m_pTranslatedVideoRelayMediaStream->GetSsrc();
		str << "\n\t m_layerId         : " << m_pTranslatedVideoRelayMediaStream->GetLayerId();
		str << "\n\t m_resolutionHeight: " << m_pTranslatedVideoRelayMediaStream->GetResolutionHeight();
		str << "\n\t m_resolutionWidth : " << m_pTranslatedVideoRelayMediaStream->GetResolutionWidth();
	}
	else
	{
		str << "\n m_pTranslatedVideoRelayMediaStream not valid ";

	}

	PTRACE(eLevelInfoNormal,str.str().c_str());

}

//----------------------------------------------
bool CSvcToAvcParams::IsValidParams()
{
	bool isValid = true;
	if(m_bIsSupportSvcAvcTranslate)
	{
		if(!IsValidPObjectPtr(m_pVideoOperationPointsSet))
		{
			PTRACE(eLevelInfoNormal, "CSvcToAvcParams::IsValidParams m_pVideoOperationPointsSet not valid");
			isValid = false;

		}
		else
		{
			int maxLayerIds = m_pVideoOperationPointsSet->GetOperationPointsList()->size();
			if(m_maxAllowedLayerId>=maxLayerIds)
			{
				PTRACE(eLevelInfoNormal, "CSvcToAvcParams::IsValidParams m_maxAllowedLayerId not valid");
				isValid = false;
			}
			if (m_maxAllowedLayerIdHighRes >= maxLayerIds)	// illegal
			{
				if (maxLayerIds > 0)
					m_maxAllowedLayerIdHighRes = maxLayerIds-1;
				else
					m_maxAllowedLayerIdHighRes = 0;
			}
		}
		if(!IsValidPObjectPtr(m_pTranslatedVideoRelayMediaStream))
		{
			PTRACE(eLevelInfoNormal, "CSvcToAvcParams::IsValidParams m_pTranslatedVideoRelayMediaStream not valid");
			isValid = false;
		}
		if(m_videoRelayInchannelHandle==INVALID)
		{
			PTRACE(eLevelInfoNormal, "CSvcToAvcParams::IsValidParams m_videoRelayInchannelHandle not valid");
			isValid = false;
		}
	}
	return isValid;

}
//----------------------------------------------
void CSvcToAvcParams::UpdateTranslatedVideoRelayMediaStream(CVideoRelayMediaStream * pVideoRelayMediaStream)
{
	TRACEINTO << " ";

	if(pVideoRelayMediaStream)//if(IsValidPObjectPtr(pVideoRelayMediaStream))
	{
		if(m_pTranslatedVideoRelayMediaStream!=NULL)
		{
			*m_pTranslatedVideoRelayMediaStream = *pVideoRelayMediaStream;
		}
		else
		{
			m_pTranslatedVideoRelayMediaStream = new CVideoRelayMediaStream(*pVideoRelayMediaStream);
		}
	}
	else
	{
		TRACEINTO << " - pVideoRelayMediaStream==NULL, deleting the internal MediaStream";
		POBJDELETE(m_pTranslatedVideoRelayMediaStream);
	}
}

//----------------------------------------------
void CSvcToAvcParams::SetMaxAllowedLayerId( int maxAllowedLayerId )
{
	m_maxAllowedLayerId = maxAllowedLayerId;
}

//----------------------------------------------
void CSvcToAvcParams::SetMaxAllowedLayerIdHighRes( int maxAllowedLayerIdHighRes )
{
	m_maxAllowedLayerIdHighRes = maxAllowedLayerIdHighRes;
}

int  CSvcToAvcParams::GetAllowedLayerId()const
{
	if (FALSE == m_bIsHighResStreamEnable)
		return m_maxAllowedLayerId;
	else
		return m_maxAllowedLayerIdHighRes;
}




