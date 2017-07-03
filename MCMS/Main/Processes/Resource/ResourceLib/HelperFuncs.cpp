#include "HelperFuncs.h"
#include "Trace.h"
#include "TraceStream.h"
#include "ConnToCardManager.h"
#include "SystemResources.h"
#include "ConfResources.h"
#include "ResourceProcess.h"
#include "ResourceManager.h"
#include "ProcessSettings.h"
#include "Reservator.h"
#include "MoveManager.h"
#include "SystemFunctions.h"
#include "ProfilesDB.h"
#include "SysConfigKeys.h"

/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsAudioParty(eVideoPartyType videoPartyType)
{
  return (videoPartyType == eVideo_party_type_none || eVoice_relay_party_type == videoPartyType);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVideoParty(eVideoPartyType videoPartyType)
{
  if (IsMode2C())
    return (!IsAudioParty(videoPartyType) && (videoPartyType != eCOP_party_type));

  return !IsAudioParty(videoPartyType);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsAudioContentParty(eVideoPartyType videoPartyType)
{
  return (videoPartyType == eVSW_Content_for_CCS_Plugin_party_type);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsISDNParty(eNetworkPartyType networkPartyType)
{
  return (networkPartyType == eISDN_network_party_type);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsIPParty(eNetworkPartyType networkPartyType)
{
  return (networkPartyType == eIP_network_party_type);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVideoBasicParty(eVideoPartyType videoPartyType)
{
  //Tsahi TODO: check if need to add h264CIF
  switch (videoPartyType)
  {
    case eVSW_video_party_type:
    //case eCP_H261_H263_H264_upto_CIF_video_party_type:
      return TRUE;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVideo2CParty(eVideoPartyType videoPartyType)
{
  switch (videoPartyType)
  {
    case eCOP_party_type:
    case eVSW_video_party_type:
    case eVideo_party_type_none:
      return TRUE;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVideoRelayParty(eVideoPartyType videoPartyType, bool includingRelayVSW)
{
	switch(videoPartyType)
	{
		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		  return TRUE;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	if (includingRelayVSW)
		return CHelperFuncs::IsVswRelayParty(videoPartyType);

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVswRelayParty(eVideoPartyType videoPartyType)
{
	switch(videoPartyType)
	{
		case eVSW_relay_CIF_party_type:
		case eVSW_relay_SD_party_type:
		case eVSW_relay_HD720_party_type:
		case eVSW_relay_HD1080_party_type:
			return TRUE;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsNeedAllocateVideo(eVideoPartyType videoPartyType, eConfModeTypes confModeType, eProductType prodType)
{
	if (CHelperFuncs::IsAudioParty(videoPartyType) || eVSW_Content_for_CCS_Plugin_party_type == videoPartyType)
		return FALSE;
	if (IsVideoRelayParty(videoPartyType) && ((eNonMix == confModeType) || eProductTypeSoftMCUMfw == prodType))
		return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsLegalVideoParty(eVideoPartyType videoPartyType)
{
	eResourceAllocationTypes allocType = GetResourceAllocationType();

	switch (videoPartyType)
	{
		case eVSW_video_party_type:
		case eCP_VP8_upto_CIF_video_party_type:
		case eCP_VP8_upto_SD30_video_party_type:
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_SD30_video_party_type:
		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_CIF_video_party_type:
		case eCP_H261_H263_upto_CIF_video_party_type:
		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
		case eVSW_Content_for_CCS_Plugin_party_type:
		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		case eVSW_relay_CIF_party_type:
        case eVSW_relay_SD_party_type:
        case eVSW_relay_HD720_party_type:
        case eVSW_relay_HD1080_party_type:
			return TRUE;

		case eCP_Content_for_Legacy_Decoder_video_party_type:        // auto mode only
		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type: // auto mode only
		case eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type: // auto mode only
			return (IsAutoModeAllocationType() ? TRUE : FALSE);

		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:  // MPMx mode only
			return (eAutoBreezeMode == allocType) ? TRUE : FALSE;

		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:   // MPM-Rx mode only
			return (eAutoMpmRxMode == allocType || eAutoMixedMode == allocType) ? TRUE : FALSE;

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsTIPSlavePartyType(ETipPartyTypeAndPosition partyTypeTIP) //TIP Cisco
{
  return (eTipSlaveLeft == partyTypeTIP || eTipSlaveRigth == partyTypeTIP || eTipSlaveAux == partyTypeTIP);
}
/////////////////////////////////////////////////////////////////////////////
eResourceAllocationTypes CHelperFuncs::GetResourceAllocationType()
{
  CSystemResources* pSystemResources = GetSystemResources();
  FPASSERT_AND_RETURN_VALUE(!pSystemResources, eAutoMpmRxMode);

  return pSystemResources->GetResourceAllocationType();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsEnhancedCardsAllocationType()
{
	eResourceAllocationTypes resourceAllocationTypes = GetResourceAllocationType();
	switch (resourceAllocationTypes)
	{
		case eAutoMpmRxMode:
		case eFixedMpmRxMode:
			return FALSE;
		case eFixedBreezeMode:
		case eAutoBreezeMode:
		case eAutoMixedMode:
			return TRUE;
		default:
			FPASSERTSTREAM_AND_RETURN_VALUE(1, "ResourceAllocationType:" << resourceAllocationTypes, FALSE);
			break;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsLegalCardsAllocationType()
{
	eResourceAllocationTypes resourceAllocationTypes = GetResourceAllocationType();
	switch (resourceAllocationTypes)
	{
		case eFixedBreezeMode:
		case eAutoBreezeMode:
		case eFixedMpmRxMode:
		case eAutoMpmRxMode:
		case eAutoMixedMode:
			return TRUE;
		default:
			FPASSERTSTREAM_AND_RETURN_VALUE(1, "ResourceAllocationType:" << resourceAllocationTypes, FALSE);
			break;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsAutoModeAllocationType()
{
	eResourceAllocationTypes resourceAllocationTypes = GetResourceAllocationType();
	switch (resourceAllocationTypes)
	{
		case eFixedBreezeMode:
		case eFixedMpmRxMode:
			return FALSE;
		case eAutoBreezeMode:
		case eAutoMpmRxMode:
		case eAutoMixedMode:
			return TRUE;
		default:
			FPASSERTSTREAM_AND_RETURN_VALUE(1, "ResourceAllocationType:" << resourceAllocationTypes, FALSE);
			break;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsFixedModeAllocationType()
{
	eResourceAllocationTypes resourceAllocationTypes = GetResourceAllocationType();
	switch (resourceAllocationTypes)
	{
		case eAutoBreezeMode:
		case eAutoMpmRxMode:
		case eAutoMixedMode:
			return FALSE;
		case eFixedBreezeMode:
		case eFixedMpmRxMode:
			return TRUE;
		default:
			FPASSERTSTREAM_AND_RETURN_VALUE(1, "ResourceAllocationType:" << resourceAllocationTypes, FALSE);
			break;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
eVideoPartyType CHelperFuncs::GetNextLowerVideoPartyType(eVideoPartyType videoPartyType)
{
	CSystemResources* pSystemResources = GetSystemResources();
	BOOL isNinja = FALSE;

	if (pSystemResources && eProductTypeNinja == pSystemResources->GetProductType())
	{
		isNinja = TRUE;
	}

	switch (videoPartyType)
	{
		case eCP_H264_upto_SD30_video_party_type:
			return eCP_H264_upto_CIF_video_party_type;
		case eCP_H261_H263_upto_CIF_video_party_type:
			return eCP_H264_upto_CIF_video_party_type; // Tsahi - check this in MPM-Rx!
		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type://olga - breeze mode
		  return eCP_H264_upto_SD30_video_party_type;
		case eCP_Content_for_Legacy_Decoder_video_party_type:
			return eVideo_party_type_none; //in this case we can't downgrade within video (olga)
		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
		case eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type:
		case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
			return eVideo_party_type_none;
		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
			return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
			return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
			return eCP_VP8_upto_SD30_video_party_type;
		case eCP_VP8_upto_SD30_video_party_type:
			return eCP_VP8_upto_CIF_video_party_type;
		default:
			return eVideo_party_type_none;
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVideoISDNParty(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType)
{
	return (IsISDNParty(networkPartyType) && IsVideoParty(videoPartyType));
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsPSTNParty(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType)
{
	return (IsISDNParty(networkPartyType) && IsAudioParty(videoPartyType));
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsDialOutParty(ISDN_SPAN_PARAMS_S* pIsdnParams)
{
	//pIsdnParams.board_id and pIsdnParams.span_id should be 0xFFFF in case of dial out
	return (DUMMY_BOARD_ID == pIsdnParams->board_id || DUMMY_SPAN_ID == pIsdnParams->span_id);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsMasterType(ECntrlType cntrl_type)
{
	return (E_VIDEO_MASTER_LB_ONLY == cntrl_type || E_VIDEO_MASTER_SPLIT_ENCODER == cntrl_type || E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP == cntrl_type);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsSlaveType(ECntrlType cntrl_type)
{
	return (E_VIDEO_SLAVE_FULL_ENCODER == cntrl_type || E_VIDEO_SLAVE_SPLIT_ENCODER == cntrl_type || E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP == cntrl_type);
}
/////////////////////////////////////////////////////////////////////////////
float CHelperFuncs::GetNumPhysicalHD720Ports(eVideoPartyType videoPartyType)
{
	// SoftMCU Call Generator - count all ports as 1
	CSystemResources* pSystemResources = GetSystemResources();
	if (pSystemResources && eProductTypeCallGeneratorSoftMCU == pSystemResources->GetProductType())
	{
		return 1;
	}

	bool isMFW = (pSystemResources && eProductTypeSoftMCUMfw == pSystemResources->GetProductType());

	BOOL bLimitCifSdPortsPerMpmxCard = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_LIMIT_CIF_SD_PORTS_PER_MPMX_CARD, bLimitCifSdPortsPerMpmxCard);

	switch (GetResourceAllocationType())
	{
		case eAutoMpmRxMode:
		case eFixedMpmRxMode:
		{
			switch (videoPartyType)
			{
				case eVSW_Content_for_CCS_Plugin_party_type:
					return 0;
				case eVSW_video_party_type:
					return 1;
				case eVideo_party_type_none:
					//return 0; Tsahi - in MPM-Rx, Audio Only party use same resource as CIF
				case eVoice_relay_party_type:
				case eVideo_relay_CIF_party_type:
				case eVideo_relay_SD_party_type:
				case eVideo_relay_HD720_party_type:
				case eVideo_relay_HD1080_party_type:
				case eVSW_relay_CIF_party_type:
				case eVSW_relay_SD_party_type:
				case eVSW_relay_HD720_party_type:
				case eVSW_relay_HD1080_party_type:
					return 0.5;
				case eCP_H264_upto_CIF_video_party_type:
				case eCP_VP8_upto_CIF_video_party_type:
				case eCP_H264_upto_SD30_video_party_type:
				case eCP_VP8_upto_SD30_video_party_type:
					return 0.5;
				case eCP_H261_H263_upto_CIF_video_party_type:
				case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
				case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
				case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
					return 1;
				case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
				case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
				case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
					return 2;
				case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
					return 4;
				default:
					FPASSERT(1 + videoPartyType);
					return 0;
			}
			break;
		}
		case eFixedBreezeMode:
		case eAutoBreezeMode:
		{
			switch (videoPartyType)
			{
				case eVideo_party_type_none:
				case eVoice_relay_party_type:
				case eVSW_Content_for_CCS_Plugin_party_type:
					return 0;
				case eVSW_video_party_type:
					return 0.25;
				case eCP_H264_upto_CIF_video_party_type:
				case eCP_VP8_upto_CIF_video_party_type:
					return (bLimitCifSdPortsPerMpmxCard) ? 0.6666 : 0.333;
				case eVSW_relay_CIF_party_type:
				case eVSW_relay_SD_party_type:
				case eVSW_relay_HD720_party_type:
				case eVSW_relay_HD1080_party_type:
					return 0.333;
				case eVideo_relay_CIF_party_type:    //MFW only: 15 SVC CIF resources consume 1 resource (resource unit is AVC HD720p30)
					return (isMFW ? PORT_WEIGHT_SVC_CIF_SOFT_MFW : 0.333);
				case eVideo_relay_SD_party_type:     //MFW only: 7.5 SVC SD resources consume 1 resource (resource unit is AVC HD720p30)
					return PORT_WEIGHT_SVC_SD30_SOFT_MFW;
				case eVideo_relay_HD720_party_type:  //MFW only: 3 SVC HD720 resources consume 1 resource (resource unit is AVC HD720p30)
					return PORT_WEIGHT_SVC_HD720_SOFT_MFW;
				case eVideo_relay_HD1080_party_type: //MFW only: ?
					return PORT_WEIGHT_SVC_HD1080_SOFT_MFW;
				case eCP_H264_upto_SD30_video_party_type:
				case eCP_VP8_upto_SD30_video_party_type:
				case eCP_H261_H263_upto_CIF_video_party_type:
					return (bLimitCifSdPortsPerMpmxCard) ? 0.6666 : 0.5;
				case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
				case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
				case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
					return 1;
				case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
				case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
				case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
					return 2;
				case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
					return 3;
				default:
					FPASSERT(1 + videoPartyType);
					return 0;
			}
			break;
		}
		case eAutoMixedMode: // In Mixed Mode we will take the worst case between MPMx and MPM-Rx
		{
			switch (videoPartyType)
			{
				case eVoice_relay_party_type:
				case eVideo_party_type_none:
				case eVSW_Content_for_CCS_Plugin_party_type:
					return 0;
				case eVSW_video_party_type:
					return 1;
				case eVideo_relay_CIF_party_type:
				case eVideo_relay_SD_party_type:
				case eVideo_relay_HD720_party_type:
				case eVideo_relay_HD1080_party_type:
				case eVSW_relay_CIF_party_type:
				case eVSW_relay_SD_party_type:
				case eVSW_relay_HD720_party_type:
				case eVSW_relay_HD1080_party_type:
				case eCP_H264_upto_CIF_video_party_type:
				case eCP_VP8_upto_CIF_video_party_type:
				case eCP_H264_upto_SD30_video_party_type:
				case eCP_VP8_upto_SD30_video_party_type:
					return 0.5;
				case eCP_H261_H263_upto_CIF_video_party_type:
				case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
				case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
				case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
					return 1;
				case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
				case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
				case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
					return 2;
				case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
					return 4;
				default:
					FPASSERT(1 + videoPartyType);
					return 0;
			}
			break;
		}
		default:
		{
			FPASSERT(1 + GetResourceAllocationType());
			return 0;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
WORD CHelperFuncs::GetNumArtChannels(PartyDataStruct& partyData)
{
	WORD numArtChannels = 0;

	// soft mcu - don't limit 1 soft ART by art channels
	CSystemResources* pSyst = GetSystemResources();
	if (pSyst && IsSoftMCU(pSyst->GetProductType()))
	{
		numArtChannels = NUM_ART_CHANNELS_FOR_SOFT_MCU_PARTY;
	}
	else if (partyData.m_networkPartyType == eISDN_network_party_type)
	{
		FPASSERT_AND_RETURN_VALUE(!partyData.m_pIsdn_Params_Request, 0);
		numArtChannels = partyData.m_pIsdn_Params_Request->num_of_isdn_ports;
	}
	else
	{
		// Not in use for IP parties.
		numArtChannels = 0;
	}

	return numArtChannels;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsBreeze(eCardType cardType)
{
  return (cardType == eMpmx_80 || cardType == eMpmx_40 || cardType == eMpmx_20);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsSoftCard(eCardType cardType) //OLGA - SoftMCU
{
	CSystemResources* pSyst = GetSystemResources();
	if( pSyst )
		return ( IsSoftMCU(pSyst->GetProductType()) && (cardType == eMpmx_Soft_Half || cardType == eMpmx_Soft_Full) );

	return (cardType == eMpmx_Soft_Half || cardType == eMpmx_Soft_Full);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsSoftMCU(eProductType productType) //OLGA - SoftMCU
{
	return ( eProductFamilySoftMcu == ::ProductTypeToProductFamily(productType) );
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsMpmRx(eCardType cardType)
{
	return (cardType == eMpmRx_Half || cardType == eMpmRx_Full);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsMpmRxOrNinja(eCardType cardType)
{
	return (cardType == eMpmRx_Half || cardType == eMpmRx_Full || cardType == eMpmRx_Ninja);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsValidBoardId(WORD bId)
{
  return (bId > 0 && bId <= BOARDS_NUM);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsEventMode()
{
  CSystemResources* pSystemResources = GetSystemResources();
  FPASSERT_AND_RETURN_VALUE(!pSystemResources, FALSE);

  return pSystemResources->IsEventMode();
}
/////////////////////////////////////////////////////////////////////////////
CResourceManager* CHelperFuncs::GetResourceManager()
{
  CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
  if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "Manager") != 0)
  {
    FPASSERT(1); //trying to get table from other task
    return NULL;
  }
  return (CResourceManager*)pTaskApp;
}
/////////////////////////////////////////////////////////////////////////////
CConnToCardManager* CHelperFuncs::GetConnToCardManager()
{
  CResourceManager* pResourceManager = GetResourceManager();
  FPASSERT_AND_RETURN_VALUE(!pResourceManager, NULL);

  return pResourceManager->GetConnToCardManager();
}
/////////////////////////////////////////////////////////////////////////////
CConfRsrcDB* CHelperFuncs::GetConfRsrcDB()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CConfRsrcDB* pConfRsrcDB = pProcess->GetConfRsrcDB();
  FPASSERT_AND_RETURN_VALUE(!pConfRsrcDB, NULL);

  return pConfRsrcDB;
}
/////////////////////////////////////////////////////////////////////////////
CMoveManager* CHelperFuncs::GetMoveManager()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CMoveManager* pMoveManager = pProcess->GetMoveManager();
  FPASSERT_AND_RETURN_VALUE(!pMoveManager, NULL);

  return pMoveManager;
}
/////////////////////////////////////////////////////////////////////////////
CNetServicesDB* CHelperFuncs::GetNetServicesDB()
{
  CSystemResources* pProcess = GetSystemResources();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CNetServicesDB* pNetServicesDB = pProcess->GetNetServicesDB();
  FPASSERT_AND_RETURN_VALUE(!pNetServicesDB, NULL);

  return pNetServicesDB;
}
/////////////////////////////////////////////////////////////////////////////
CSystemResources* CHelperFuncs::GetSystemResources()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CSystemResources* pSystemResources = pProcess->GetSystemResources();
  FPASSERT_AND_RETURN_VALUE(!pSystemResources, NULL);

  return pSystemResources;
}
/////////////////////////////////////////////////////////////////////////////
eProcessStatus CHelperFuncs::GetProcessStatus()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, eProcessInvalid);

  return pProcess->GetProcessStatus();
}
/////////////////////////////////////////////////////////////////////////////
CProcessSettings* CHelperFuncs::GetProcessSettings()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  return pProcess->GetProcessSettings();
}
/////////////////////////////////////////////////////////////////////////////
CReservator* CHelperFuncs::GetReservator()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CReservator* pReservator = pProcess->GetReservator();
  FPASSERT_AND_RETURN_VALUE(!pReservator, NULL);

  return pReservator;
}
/////////////////////////////////////////////////////////////////////////////
CRsrvDB* CHelperFuncs::GetRsrvDB()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CRsrvDB* pRsrvDB = pProcess->GetRsrvDB();
  FPASSERT_AND_RETURN_VALUE(!pRsrvDB, NULL);

  return pRsrvDB;
}
/////////////////////////////////////////////////////////////////////////////
SleepingConferences* CHelperFuncs::GetSleepingConferences()
{
	CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

	SleepingConferences* pSleepingConferences = pProcess->GetSleepingConferences();
	FPASSERT_AND_RETURN_VALUE(!pSleepingConferences, NULL);

	return pSleepingConferences;
}
/////////////////////////////////////////////////////////////////////////////
ReservedConferences* CHelperFuncs::GetConfRsrvRsrcs()
{
	CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

	ReservedConferences* pConfRsrvRsrc = pProcess->GetConfRsrvRsrcs();
	FPASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, NULL);

	return pConfRsrvRsrc;
}
/////////////////////////////////////////////////////////////////////////////
CProfilesDB* CHelperFuncs::GetProfilesDB()
{
  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN_VALUE(!pProcess, NULL);

  CProfilesDB* pProfilesDB = pProcess->GetProfilesDB();
  FPASSERT_AND_RETURN_VALUE(!pProfilesDB, NULL);

  return pProfilesDB;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsLogicalVideoEncoderType(eLogicalResourceTypes type)
{
  switch (type)
  {
    case eLogical_video_encoder:
    case eLogical_video_encoder_content:
    case eLogical_COP_CIF_encoder:
    case eLogical_COP_VSW_encoder:
    case eLogical_COP_PCM_encoder:
    case eLogical_COP_HD720_encoder:
    case eLogical_COP_HD1080_encoder:
    case eLogical_COP_4CIF_encoder:
    case eLogical_relay_video_encoder:  //OLGA - Soft MCU
    case eLogical_relay_avc_to_svc_video_encoder_1:
    case eLogical_relay_avc_to_svc_video_encoder_2:
      return TRUE;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsLogicalSoftMixVideoEncoderType(eLogicalResourceTypes type)
{
	return (eLogical_relay_avc_to_svc_video_encoder_1 == type || eLogical_relay_avc_to_svc_video_encoder_2 == type);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsLogicalRTPtype(eLogicalResourceTypes type)
{
	return (eLogical_rtp == type ||	eLogical_relay_avc_to_svc_rtp == type || eLogical_relay_avc_to_svc_rtp_with_audio_encoder == type);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsLogicalVideoDecoderType(eLogicalResourceTypes type)
{
  switch (type)
  {
    case eLogical_video_decoder:
    case eLogical_COP_Dynamic_decoder:
    case eLogical_COP_VSW_decoder:
    case eLogical_COP_LM_decoder:
      return TRUE;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsMode2C()
{
  CReservator* pReservator = CHelperFuncs::GetReservator();
  if (pReservator)
    return pReservator->IsRMode2C();
  else
    return CHelperFuncs::IsEventMode();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsSessionTypeCOP(eSessionType sessionType)
{
  switch (sessionType)
  {
    case eCOP_HD1080_session:
    case eCOP_HD720_50_session:
    case eVSW_28_session:
    case eVSW_56_session:
      return TRUE;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
WORD CHelperFuncs::GetMaxUnitsNeededForVideo()
{
  CSystemResources* pSystemResources = GetSystemResources();
  FPASSERT_AND_RETURN_VALUE(!pSystemResources, MAX_UNITS_NEEDED_FOR_VIDEO);

  return pSystemResources->GetMaxUnitsNeededForVideo();
}
/////////////////////////////////////////////////////////////////////////////
WORD CHelperFuncs::GetMaxRequiredPortsPerMediaUnit()
{
  CSystemResources* pSystemResources = GetSystemResources();
  FPASSERT_AND_RETURN_VALUE(!pSystemResources, MAX_REQUIRED_PORTS_PER_MEDIA_UNIT);

  return pSystemResources->GetMaxRequiredPortsPerMediaUnit();
}
/////////////////////////////////////////////////////////////////////////////
STATUS CHelperFuncs::CheckAllReservationsSysMode()
{
  CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
  FPASSERT_AND_RETURN_VALUE(!pRsrvDB, STATUS_FAIL);

  return pRsrvDB->CheckAllReservationsSysMode();
}
/////////////////////////////////////////////////////////////////////////////
eSystemCardsMode CHelperFuncs::GetSystemCardsModeFromCardType(eCardType cardType)
{
	eSystemCardsMode ret_mode = eSystemCardsMode_illegal;

	switch (cardType)
	{
		case eMfa_26:
		case eMfa_13:
		{
			ret_mode = eSystemCardsMode_mpm;
			break;
		}
		case eMpmPlus_20:
		case eMpmPlus_40:
		case eMpmPlus_80:
		{
			ret_mode = eSystemCardsMode_mpm_plus;
			break;
		}
		case eMpmPlus_MezzanineA:
		case eMpmPlus_MezzanineB:
		case eMpmx_40:
		case eMpmx_80:
		case eMpmx_20:
		case eMpmx_Soft_Full: //temp
		case eMpmx_Soft_Half:
		{
			ret_mode = eSystemCardsMode_breeze;
			break;
		}
		case eMpmRx_Half:
		case eMpmRx_Full:
		case eMpmRx_Ninja:
		{
			ret_mode = eSystemCardsMode_mpmrx;
			break;
		}
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return ret_mode;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CHelperFuncs::CalcIpServicePart(DWORD total, float service_factor, BOOL round_up)
{
	if (service_factor < 0 || service_factor > 1)
	{
		FPASSERT((DWORD)(service_factor * 10));
		return total;
	}

	DWORD ipServicePart = (DWORD)(total * service_factor);
	if (round_up)
	{
		if ((total * service_factor) > floor(total * service_factor))
			ipServicePart = ipServicePart + 1;
	}

	return ipServicePart;
}
/////////////////////////////////////////////////////////////////////////////
float CHelperFuncs::CalcIpServicePart(float total, float service_factor, BOOL round_up)
{
	if (service_factor < 0 || service_factor > 1)
	{
		FPASSERT((DWORD)(service_factor * 10));
		return total;
	}

	float ipServicePart = total * service_factor;
	if (round_up)
	{
		if (ipServicePart > floor(ipServicePart))
			ipServicePart = ipServicePart + 1;
	}
	return ipServicePart;
}
/////////////////////////////////////////////////////////////////////////////
const char* CHelperFuncs::GetIpServiceName(DWORD service_id)
{
	return GetSystemResources()->GetIpServiceName(service_id);
}
/////////////////////////////////////////////////////////////////////////////
DWORD CHelperFuncs::GetIpServiceId(const char* service_name)
{
	return GetSystemResources()->GetIpServiceId(service_name);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CHelperFuncs::IsMultipleService()
{
	return GetSystemResources()->GetMultipleIpServices();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CHelperFuncs::IsVideoSwitchParty(eVideoPartyType videoPartyType)
{
	return (videoPartyType == eVSW_video_party_type) ? TRUE : FALSE;
}
/////////////////////////////////////////////////////////////////////////////
float CHelperFuncs::GetArtPromilsBreeze()
{
	string sTmp;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("MPMX_MAX_ART_PORTS_PER_UNIT", sTmp);
	WORD num_ports_per_unit = atoi(sTmp.c_str());
	WORD mpmx_art_promils = ART_PROMILLES_BREEZE;

	if (num_ports_per_unit == 6)
	{
		mpmx_art_promils = 166;
	}
	else if (num_ports_per_unit != 9)
	{
		FPTRACE2(eLevelInfoNormal, "CHelperFuncs::GetArtPromilsBreeze - illegal system.cfg MPMX_MAX_ART_PORTS_PER_UNIT value = ", sTmp.c_str());
	}
	return mpmx_art_promils;
}
/////////////////////////////////////////////////////////////////////////////
void CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(eVideoPartyType videoPartyType, BOOL& isNeeded, BOOL& isSvc, BOOL& isHD)
{
	// init results
	isNeeded = FALSE;
	isSvc = FALSE;
	isHD = FALSE;

	switch (videoPartyType)
	{
		// no Avc to Svc video resources required
		case eVideo_party_type_dummy:
		case eVideo_party_type_none: // voice
		case eCP_Content_for_Legacy_Decoder_video_party_type: // no legacy content in svc
		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type: // no legacy content in svc
		case eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type: // no legacy content in svc
		case eCOP_party_type: // no cop to svc
		case eVSW_Content_for_CCS_Plugin_party_type: // not video party
		case eVSW_relay_CIF_party_type:
		case eVSW_relay_SD_party_type:
		case eVSW_relay_HD720_party_type:
		case eVSW_relay_HD1080_party_type:
		{
			break;
		}
			// Svc to Avc resources
		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		{
			isNeeded = TRUE;
			isSvc = TRUE;
			break;
		}
			// Avc to Svc HD resources (encoder 360p+180p)
		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
		{
			isNeeded = TRUE;
			isHD = TRUE;
			break;
		}
			// Avc to Svc SD resources (encoder 180p)
		case eCP_H264_upto_CIF_video_party_type:
		case eCP_VP8_upto_CIF_video_party_type:
		case eCP_H264_upto_SD30_video_party_type:
		case eCP_VP8_upto_SD30_video_party_type:
		case eCP_H261_H263_upto_CIF_video_party_type:
		case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		case eVSW_video_party_type:
		{
			isNeeded = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
eConfModeTypes CHelperFuncs::GetConferenceMode(ConfMonitorID confId)
{
	// general logic - party is allocated at mixed when:
	// conf is defined as mixed mode
	// not EQ
	// and:
	// dynamic allocation is off (all Avc-Svc resources allocated from advance)
	// dynamic allocation is on, and conf already upgraded to mix mode.

	bool isSoftMcu = false;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		isSoftMcu = CHelperFuncs::IsSoftMCU(pSystemResources->GetProductType());

	std::ostringstream msg;
	msg << "MonitorConfId:" << confId << ", IsSoftMcu:" << (WORD)isSoftMcu;

	// 1) check if conf configured as mixed mode
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	const CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(confId) : NULL;
	FTRACECOND_AND_RETURN_VALUE(!pConfRsrc, msg.str().c_str() << ", RC:eNonMix - Failed to find conference", eNonMix);

	eConfMediaType conf_media_type = pConfRsrc->GetConfMediaType();
	msg << ", ConfMediaType:" << ConfMediaTypeToString(conf_media_type);

	FTRACECOND_AND_RETURN_VALUE((eMixAvcSvc != conf_media_type && eMixAvcSvcVsw != conf_media_type), msg.str().c_str() << ", RC:eNonMix - Conference is not set as mixed AVC SVC", eNonMix);

	// 2) check system cfg flag
	bool bIsAvcSvcDymamicAllocation = CHelperFuncs::IsDynamicMixedAvcSvcAllocationMode();

	// 3) check for EQ
	if (STANDALONE_CONF_ID == pConfRsrc->GetRsrcConfId())
	{
		FTRACECOND_AND_RETURN_VALUE(!bIsAvcSvcDymamicAllocation, msg.str().c_str() << ", RC:eMix - EQ conference, fixed mixed mode", eMix);

		FTRACEINTO << msg.str().c_str() << ", RC:eNonMix - EQ conference, dynamic allocation";
		return eNonMix;
	}

	// 4) check for regular conf
	if (!bIsAvcSvcDymamicAllocation)
	{
		FTRACEINTO << msg.str().c_str() << ", RC:eMix - Fixed mixed mode";
		return eMix;
	}

	if (bIsAvcSvcDymamicAllocation && eMediaStateMixAvcSvc == pConfRsrc->GetConfMediaState())
	{
		FTRACEINTO << msg.str().c_str() << ", RC:eMix - Dynamic mode, conference already upgraded";
		return eMix;
	}

	FTRACEINTO << msg.str().c_str() << ", RC:eNonMix";
	return eNonMix;
}
/////////////////////////////////////////////////////////////////////////////
bool CHelperFuncs::IsDynamicMixedAvcSvcAllocationMode()
{
	bool bIsAvcSvcDymamicAllocation = false;
	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
	if (pSystResources && eProductTypeSoftMCUMfw == pSystResources->GetProductType())
		bIsAvcSvcDymamicAllocation = TRUE;
	else
	{
		// 2) check system cfg flag
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		if (pSysConfig)
			pSysConfig->GetBOOLDataByKey("MIX_AVC_SVC_DYNAMIC_ALLOCATION", bIsAvcSvcDymamicAllocation);
	}
	return bIsAvcSvcDymamicAllocation;
}
