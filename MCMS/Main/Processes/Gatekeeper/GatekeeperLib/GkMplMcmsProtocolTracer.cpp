

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////				GK  Prints			  /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


#include "GkMplMcmsProtocolTracer.h"
#include "GkCsReq.h"
#include "GkCsInd.h"
#include "IpMngrOpcodes.h"
#include "GKManagerUtils.h"
#include "Macros.h"
#include "IpCommonUtilTrace.h"
#include "ObjString.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"


///////////////////////////////////////////////////////////////////////////////////////////
CGkMplMcmsProtocolTracer::CGkMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt)
:CMplMcmsProtocolTracer(mplMcmsProt)
{
}

///////////////////////////////////////////////////////////////////////////////////////////
CGkMplMcmsProtocolTracer::~CGkMplMcmsProtocolTracer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceContent(CObjString* pContentStr,eProcessType processType)
{
	OPCODE opcode = m_pMplMcmsProt->getCommonHeaderOpcode();
	switch (opcode)
	{

	// GK MANAGER -> CS:

	case H323_CS_RAS_GRQ_REQ: {
		TraceRasGRQReq(pContentStr);
		break;  }

  	case H323_CS_RAS_RRQ_REQ:{
		TraceRasRRQReq(pContentStr);
		break;  }
		
	case H323_CS_RAS_ARQ_REQ:{
		TraceRasARQReq(pContentStr);
		break;  }
		
	case H323_CS_RAS_DRQ_REQ:{
		TraceRasDRQReq(pContentStr);
		break;  }	
		
	case H323_CS_RAS_URQ_REQ: {
		TraceRasURQReq(pContentStr);
		break;	}
		
	case H323_CS_RAS_URQ_RESPONSE_REQ:{
		TraceRasURQResponseReq(pContentStr);
		break;	}
		
	case H323_CS_RAS_DRQ_RESPONSE_REQ:{
		TraceRasDRQResponseReq(pContentStr);
		break;	}
		
	case H323_CS_RAS_BRQ_REQ:{
		TraceRasBRQReq(pContentStr);
		break;	}
		
	case H323_CS_RAS_LRQ_RESPONSE_REQ:{
		TraceRasLRQResponseReq(pContentStr);
		break;	}


    case H323_CS_RAS_RAI_REQ :{
		TraceRasRAIReq(pContentStr);
		break;	}

	case H323_CS_RAS_IRR_RESPONSE_REQ:
	{
		TraceRasIrrResponseReq(pContentStr);
		break;
	}
		
	/*	case H323_CS_RAS_IRR_REQ:
	case H323_CS_RAS_IRR_RESPONSE_REQ:		
	case H323_CS_RAS_BRQ_RESPONSE_REQ:*/
		
	
	
	// CS -> GK MANAGER

	case H323_CS_RAS_GRQ_IND:	{
		TraceRasGRQInd(pContentStr);
		break;  }
		
	case H323_CS_RAS_RRQ_IND:	{				
		TraceRasRRQInd(pContentStr);
		break; }
		
	case H323_CS_RAS_ARQ_IND:	{				
		TraceRasARQInd(pContentStr);
		break; }

    case H323_CS_RAS_DRQ_IND:	{				
		TraceRasDRQInd(pContentStr);
		break; }

    case H323_CS_RAS_TIMEOUT_IND: {
	    TraceRasTimeoutInd(pContentStr);
		break; }
		
	case H323_CS_RAS_FAIL_IND:{
	    TraceRasFailInd(pContentStr);
		break; }

	case H323_CS_RAS_GKURQ_IND:	{
	    TraceRasGkURQInd(pContentStr);
		break; }

	case H323_CS_RAS_GKDRQ_IND:	{
	    TraceRasGkDRQInd(pContentStr);
		break; }
	
	case H323_CS_RAS_GKLRQ_IND:	{
	    TraceRasGkLRQInd(pContentStr);
		break; }
		
	case H323_CS_RAS_URQ_IND:	{
	    TraceRasURQInd(pContentStr);
		break; }	
						
	case H323_CS_RAS_BRQ_IND: {
	    TraceRasBRQInd(pContentStr);
		break; }	
									
/*	case H323_CS_RAS_GKBRQ_IND:				
	case H323_CS_RAS_GKIRQ_IND:*/
	
	
    default:	{
    	CMplMcmsProtocolTracer::TraceContent(pContentStr);
		break;	}   
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::PrintGkRejectInfo(rejectInfoSt* pRejectInfo, CObjString *pContentStr)
{
	if (pRejectInfo->bIsReject)
	{
		*pContentStr << "Reject reason				: ";
		::GetRasRejectReasonTypeName(pRejectInfo->rejectReason, *pContentStr);
		*pContentStr << "\n";
		if (pRejectInfo->altGkList.numOfAltGks != 0)
		{
			*pContentStr << "Alt Gk permanent   =  " << pRejectInfo->bAltGkPermanent << "\n";
			PrintAltGkInfo(&(pRejectInfo->altGkList), pContentStr);
		}
	}
}
	
///////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::PrintAltGkList(altGksListSt* pAltGkList, CObjString *pContentStr)
{
	if (pAltGkList->numOfAltGks != 0)
		PrintAltGkInfo(pAltGkList, pContentStr);		
}

////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::PrintGkFs(h460FsSt fs, CObjString *pContentStr)
{
	*pContentStr << "Need Fs    - Fs ID		= " << fs.needFs.fsId     << "\n"
				 << "Need Fs    - Sub Fs		= " << fs.needFs.subFs    << "\n"
				 << "Desire Fs  - Fs ID		= " << fs.desireFs.fsId   << "\n"
				 << "Desire Fs  - Sub Fs		= " << fs.desireFs.subFs  << "\n"
				 << "Support Fs - Fs ID		= " << fs.supportFs.fsId  << "\n"  
				 << "Support Fs - Sub Fs		= " << fs.supportFs.subFs << "\n"; 
}


////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::PrintAltGkInfo(altGksListSt* pListStFromGk, CObjString *pContentStr)
{
    int numOfAltGks = pListStFromGk->numOfAltGks;
    *pContentStr << "Num of alternate Gks	= " << numOfAltGks << "\n";
	if (numOfAltGks == 0)
		return;		
		
    alternateGkSt* pCurPtrInGkList = (alternateGkSt*)(&(pListStFromGk->altGkSt)); //first alt gk struct	
 	*pContentStr << "\nAlternate GK Info:";
	
	char *pChar;
 
	for (int i = 0; i < numOfAltGks; i++)
	{
		*pContentStr << "\n";		
		*pContentStr << "Is need to register		= " << pCurPtrInGkList->bNeedToRegister << "\n";
		
		*pContentStr << "Alternate GK RAS address:	\n";
		TraceTransportAddrSt(pContentStr, pCurPtrInGkList->rasAddress);
		*pContentStr << "Priority				= " << pCurPtrInGkList->priority << "\n";
				
		TraceGkIdent(pContentStr, pCurPtrInGkList->gkIdent, pCurPtrInGkList->gkIdentLength);

		pChar = (char*)pCurPtrInGkList;
		pChar += sizeof(alternateGkSt);
		pCurPtrInGkList = (alternateGkSt *)pChar;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceGkIdent(CObjString *pContentStr, const char* gatekeeperIdent, unsigned int gkIdentLength)
{
	*pContentStr << "Gk ident length			= " << gkIdentLength << "\n";
	if (gkIdentLength)
	{
		ALLOCBUFFER(buf, gkIdentLength + 1);
		ConvertIdFromBmpString(gatekeeperIdent, gkIdentLength, buf);
		buf[gkIdentLength] = '\0';
		*pContentStr << "GK Ident				= " << buf << "\n";
		DEALLOCBUFFER(buf); 
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceEpIdent(CObjString *pContentStr, const char* epIdent, unsigned int epIdentLength)
{
	*pContentStr << "EP ident length			= " << epIdentLength << "\n";
	if (epIdentLength)
	{
		ALLOCBUFFER(buf, epIdentLength + 1);
		ConvertIdFromBmpString(epIdent, epIdentLength, buf);
		buf[epIdentLength] = '\0';
		*pContentStr << "EP Ident				= " << buf << "\n";
		DEALLOCBUFFER(buf); 
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasGRQReq(CObjString *pContentStr)
{
	const gkReqRasGRQ *pSt = (const gkReqRasGRQ *)m_pMplMcmsProt->getpData();
	
	*pContentStr <<"\nCONTENT: H323_CS_RAS_GRQ_REQ:\n";
	
	*pContentStr << "RAS address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->rasAddress);
	*pContentStr << "\n";
	
	*pContentStr << "GK address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->gatekeeperAddress);
	*pContentStr << "\n";
	
	*pContentStr << "endpointType			= ";
	::GetEndpointTypeName(pSt->endpointType, *pContentStr);
	*pContentStr << "\n";
				
	*pContentStr << "bIsMulticast			= "
				 << pSt->bIsMulticast << "\n"
				
				 << "bSupportAltGk			= "
				 << pSt->bSupportAltGk << "\n";
		
	PrintGkFs(pSt->fs, pContentStr); 

	*pContentStr << "AliasesLength			= " << pSt->aliasesLength << "\n";
	if (pSt->aliasesLength)
	{
		char aliasStr[pSt->aliasesLength + 1];
		strncpy(aliasStr, pSt->aliasesList, pSt->aliasesLength + 1);
		aliasStr[pSt->aliasesLength] = '\0';
		*pContentStr << "Aliases List			= " << aliasStr << "\n";
	}
	
	*pContentStr << "Alias Types				= ";
	for(int i = 0 ; i < MaxNumberOfAliases; i++)
	{
		if (pSt->aliasesTypes[i])
		{
			::GetAliasTypeName(pSt->aliasesTypes[i], *pContentStr);
			*pContentStr << ", " ;
		}
	}

    *pContentStr << "\n";
    *pContentStr << "GK Identifier. Length: " << pSt->gkIdentLength << "GK_IDENT: ";

    for(APIU32 nS = 0; nS < pSt->gkIdentLength; nS++)
    {
        char szSimb[16]="";
        sprintf(szSimb, " 0x%X |",  pSt->gatekeeperIdent[nS]); 
        *pContentStr << szSimb;
    }

	*pContentStr << "\n";
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasGRQInd(CObjString *pContentStr)
{
	const gkIndRasGRQ *pSt = (const gkIndRasGRQ *)m_pMplMcmsProt->getpData(); 

	*pContentStr <<"\nCONTENT: H323_CS_RAS_GRQ_IND:\n";

	*pContentStr << "RAS address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->rasAddress);
	*pContentStr << "\n";	

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);	
	PrintGkFs(pSt->fs, pContentStr);
	
	mcRejectOrConfirmChoice* pRejOrConfirmChoice = (mcRejectOrConfirmChoice*)& (pSt->rejectOrConfirmCh);
    rejectInfoSt* pRejectInfo =  &(pRejOrConfirmChoice->choice.rejectInfo);
	altGksListSt* pAltGkList  =  &(pRejOrConfirmChoice->choice.altGkList);
	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	if (status != STATUS_OK)
		PrintGkRejectInfo(pRejectInfo, pContentStr);
	else
	    PrintAltGkList(pAltGkList, pContentStr);	
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasRRQReq(CObjString *pContentStr)
{
	const gkReqRasRRQ *pSt = (const gkReqRasRRQ *)m_pMplMcmsProt->getpData();
	
	*pContentStr <<"\nCONTENT: H323_CS_RAS_RRQ_REQ:\n";
	
	*pContentStr << "RAS address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->rasAddress);
	*pContentStr << "\n";
	
	*pContentStr << "Call signaling addresses:	\n";
	for(int i=0; i<MaxNumberOfEndpointIp; i++)
	{
		if( !(::isApiTaNull(&pSt->callSignalingAddress[i].transAddr)) )
		{
			TraceTransportAddrSt(pContentStr, pSt->callSignalingAddress[i]);
			*pContentStr << "\n";
		}
	}

	*pContentStr << "GK address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->gatekeeperAddress);
	*pContentStr << "\n";
		
	*pContentStr << "bIsMulticast			= "
				 << pSt->bIsMulticast << "\n"
				
				 << "bSupportAltGk			= "
				 << pSt->bSupportAltGk << "\n";
		
	PrintGkFs(pSt->fs, pContentStr); 

	*pContentStr << "AliasesLength			= " << pSt->aliasesLength << "\n";
	if (pSt->aliasesLength)
	{
		ALLOCBUFFER(aliasStr, pSt->aliasesLength + 1);
		strncpy(aliasStr, pSt->aliasesList, pSt->aliasesLength + 1);
		aliasStr[pSt->aliasesLength] = '\0';
		*pContentStr << "Aliases List			= " << aliasStr << "\n";
		DEALLOCBUFFER(aliasStr);		
	}

	*pContentStr << "Alias Types				= ";
	for(int i = 0 ; i < MaxNumberOfAliases; i++)
	{
		if (pSt->aliasesTypes[i])
		{
			::GetAliasTypeName(pSt->aliasesTypes[i], *pContentStr);
			*pContentStr << ", " ;
		}
	}
	*pContentStr << "\n";

	*pContentStr << "EP ident length			= " << pSt->epIdentLength << "\n";
	if (pSt->epIdentLength)
	{
		ALLOCBUFFER(buf, pSt->epIdentLength + 1);
		memcpy(buf, pSt->endpointIdent, pSt->epIdentLength + 1);
		buf[pSt->epIdentLength] = '\0';
		*pContentStr << "EP Ident				= " << buf << "\n";
		DEALLOCBUFFER(buf); 
	}

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);
	
	*pContentStr << "DiscoveryComplete		= " << pSt->dicoveryComplete << "\n"
				 << "IsKeepAlive				= " << pSt->bIsKeepAlive << "\n"
				 << "Time to live			= " << pSt->timeToLive << "\n";
  
	if (pSt->gwPrefix[0] != '\0')
		*pContentStr << "GwPrefix            =  " << pSt->gwPrefix << "\n";
	
	/* MCU Details */
	*pContentStr << "ManufacturerCode		= " << pSt->mcuDetails.info.manufacturerCode<< "\n";
	*pContentStr << "T35CountryCode			= " << (int)pSt->mcuDetails.info.t35CountryCode << "\n";
	*pContentStr << "T35Extension			= " << (int)pSt->mcuDetails.info.t35Extension  << "\n";
	
	ALLOCBUFFER(mcuDetails, CT_NonStandard_Data_Size);
	*pContentStr << "MCU details				= ";
	for (int j = 0; j < CT_NonStandard_Data_Size; j++)
	{	  
		sprintf(mcuDetails, "%c", pSt->mcuDetails.data[j]);
		*pContentStr << mcuDetails ;
	}
	*pContentStr << "\n" ;
	DEALLOCBUFFER(mcuDetails);
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasRRQInd(CObjString *pContentStr)
{
	const gkIndRasRRQ *pSt = (const gkIndRasRRQ *)m_pMplMcmsProt->getpData(); 

	*pContentStr <<"\nCONTENT: H323_CS_RAS_RRQ_IND:\n";

	*pContentStr << "Time to live			= " << pSt->timeToLive << "\n";

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);
	TraceEpIdent(pContentStr, pSt->endpointIdent, pSt->epIdentLength);
	PrintGkFs(pSt->fs, pContentStr); 
	
    mcRejectOrConfirmChoice* pRejOrConfirmChoice = (mcRejectOrConfirmChoice*)& (pSt->rejectOrConfirmCh);
    rejectInfoSt* pRejectInfo =  &(pRejOrConfirmChoice->choice.rejectInfo);
	altGksListSt* pAltGkList  =  &(pRejOrConfirmChoice->choice.altGkList);
	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	if (status != STATUS_OK)
		PrintGkRejectInfo(pRejectInfo, pContentStr);
	else
		PrintAltGkList(pAltGkList, pContentStr);	
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasARQReq(CObjString *pContentStr)
{
	const gkReqRasARQ *pSt = (const gkReqRasARQ *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_ARQ_REQ:\n";
	int i;

	*pContentStr << "GK address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->gatekeeperAddress);
	*pContentStr << "\n";

	*pContentStr << "Dest call signaling address:	\n";
	TraceTransportAddrSt(pContentStr,pSt->destCallSignalAddress);
	*pContentStr << "\n";
	
	*pContentStr << "Source Call Signaling Address:	\n";
	TraceTransportAddrSt(pContentStr,pSt->srcCallSignalAddress);
	*pContentStr << "\n";
			
	*pContentStr << "CID						= ";
	PrintHexNum(pContentStr, pSt->cid, Size16);
		
	*pContentStr << "Call Id					= ";
	PrintHexNum(pContentStr, pSt->callId, Size16);

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);
	TraceEpIdent(pContentStr, pSt->endpointIdent, pSt->epIdentLength);
	
	*pContentStr << "Call type				= ";
	::GetCallTypeName(pSt->callType, *pContentStr );
	*pContentStr << "\n";
	
	*pContentStr << "Call model				= ";
	::GetCallModelTypeName(pSt->callModel, *pContentStr );
	*pContentStr << "\n";
	
	*pContentStr << "Bandwidth				= " << pSt->bandwidth << "\n";
	*pContentStr << "IsDialIn				= " << pSt->bIsDialIn << "\n";
	*pContentStr << "CanMapAlias				= " << pSt->bCanMapAlias << "\n";
	*pContentStr << "Fs Id					= " << pSt->avfFeVndIdReq.fsId << "\n";

	*pContentStr << "Total dyn len			= " << pSt->totalDynLen << "\n";

	*pContentStr << "Src info length			= " << pSt->srcInfoLength;

	for (i = 0; i < MaxNumberOfAliases; i++)
	{
		if(pSt->srcInfoTypes[i])
		{
			if(i == 0)
				*pContentStr << "\nsrcInfoAliasTypes		= ";
			::GetAliasTypeName(pSt->srcInfoTypes[i], *pContentStr);
			*pContentStr << ", " ;
		}
	}
	*pContentStr << "\n" ;

	DWORD srcLength = pSt->srcInfoLength;
	ALLOCBUFFER(aliasesStr, srcLength+1);
	strncpy(aliasesStr, pSt->srcAndDestInfo, srcLength);
	aliasesStr[srcLength] = '\0';
	*pContentStr << "Src Info				= " << aliasesStr << "\n";
	DEALLOCBUFFER(aliasesStr);

	*pContentStr << "Dest info length		= " << pSt->destInfoLength;

	for (i = 0; i < MaxNumberOfAliases; i++)
	{
		if(pSt->destInfoTypes[i])
		{
			if(i == 0)
				*pContentStr << "\ndestInfoAliasTypes		= ";
			::GetAliasTypeName(pSt->destInfoTypes[i], *pContentStr);
			*pContentStr << ", " ;
		}
	}
	*pContentStr << "\n" ;
	
	DWORD destLength = pSt->destInfoLength;
	if (destLength)
	{
		BYTE* pointerToDestAliases = (BYTE*)pSt->srcAndDestInfo;
		pointerToDestAliases += srcLength;
		ALLOCBUFFER(aliasesStr, destLength+1);
		strncpy(aliasesStr, (char*)pointerToDestAliases, destLength);
		aliasesStr[destLength]='\0'; //B.S.: KW 1136
		*pContentStr << "Dest Info				= " << aliasesStr << "\n";
		DEALLOCBUFFER(aliasesStr);
	}
}


///////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasARQInd(CObjString *pContentStr)
{
	const gkIndRasARQ *pSt = (const gkIndRasARQ *)m_pMplMcmsProt->getpData(); 
 	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	
	*pContentStr <<"\nCONTENT: H323_CS_RAS_ARQ_IND:\n";
	if (status == STATUS_OK)
	{
		*pContentStr << "Dest call signaling address:	\n";
		TraceTransportAddrSt(pContentStr, pSt->destCallSignalAddress);
		*pContentStr << "\n";
			
		*pContentStr << "CRV						= " << pSt->crv << "\n";
		
		*pContentStr << "Call type				= ";
		::GetCallTypeName(pSt->callType, *pContentStr);
		*pContentStr << "\n";
		
		*pContentStr << "Call model				= ";
		::GetCallModelTypeName(pSt->callModel, *pContentStr);
		*pContentStr << "\n";
		
		*pContentStr << "Bandwidth				= " << pSt->bandwidth << "\n";
		*pContentStr << "Irr frequency			= " << pSt->irrFrequency << "\n";
		*pContentStr << "real fsId				= " << pSt->avfFeVndIdInd.fsId << "\n";
		*pContentStr << "real countryCode		= " << (int)pSt->avfFeVndIdInd.countryCode << "\n";
		*pContentStr << "real t35Extension		= " << (int)pSt->avfFeVndIdInd.t35Extension << "\n";
		*pContentStr << "real manfctrCode		= " << pSt->avfFeVndIdInd.manfctrCode << "\n";
		
		if (pSt->avfFeVndIdInd.productId[0] != '\0')
		{
			ALLOCBUFFER(tempBuf, H460_C_ProdIdMaxSize);
			strncpy(tempBuf, pSt->avfFeVndIdInd.productId, H460_C_ProdIdMaxSize);
			*pContentStr << "real productID  	=  " << tempBuf << "\n";
			DEALLOCBUFFER(tempBuf);
		}
		
		if (pSt->avfFeVndIdInd.versionId[0] != '\0')
		{
			ALLOCBUFFER(tempBuf, H460_C_VerIdMaxSize);
			strncpy(tempBuf, pSt->avfFeVndIdInd.versionId, H460_C_VerIdMaxSize);
			*pContentStr << "real versionId  	=  " << tempBuf << "\n";
			DEALLOCBUFFER(tempBuf);
		}
		
		if (pSt->avfFeVndIdInd.enterpriseNum[0] != '\0')
		{
			ALLOCBUFFER(tempBuf, H460_C_EntrpNumMaxSize);
			strncpy(tempBuf, pSt->avfFeVndIdInd.enterpriseNum, H460_C_EntrpNumMaxSize);
			*pContentStr << "enterprise num   	=  " << tempBuf << "\n";
			DEALLOCBUFFER(tempBuf);
		}
	 
	  //canMapAlias
		if (pSt->destExtraCallInfo[0] != '\0')
		{
			ALLOCBUFFER (destExtraCallInfo, MaxAddressListSize);
			strncpy (destExtraCallInfo, pSt->destExtraCallInfo, MaxAddressListSize);
			*pContentStr << "destExtraCallInfo  	=  " << destExtraCallInfo;
			DEALLOCBUFFER (destExtraCallInfo);
			for(int i = 0; i < MaxNumberOfAliases; i++ )
			{
				if(pSt->destExtraCallInfoTypes[i])
				{
					if(i == 0)
						*pContentStr << "\ndestExtraCallInfoTypes: ";
					::GetAliasTypeName(pSt->destExtraCallInfoTypes[i], *pContentStr);
					*pContentStr << ", " ;
				}
			}
			*pContentStr << "\n" ;
		}
	
		ALLOCBUFFER(destInfo, MaxAddressListSize);
		strncpy(destInfo, pSt->destInfo, MaxAddressListSize);
		*pContentStr << "Dest info				= " << destInfo << "\n";
		DEALLOCBUFFER(destInfo);
	 	
		if (pSt->remoteExtensionAddress[0] != '\0')
		{
			ALLOCBUFFER(remoteExtensionAddress, MaxAliasLength);
			strncpy( remoteExtensionAddress, pSt->remoteExtensionAddress, MaxAliasLength);
			*pContentStr << "remoteExtensionAddress   =  " << remoteExtensionAddress << "\n";
			DEALLOCBUFFER(remoteExtensionAddress);
		}
		
		*pContentStr << "Conference Id			= "; 
		PrintHexNum(pContentStr, pSt->conferenceId, MaxConferenceIdSize);
		*pContentStr << "Call Id					= ";
		PrintHexNum(pContentStr, pSt->callId, Size16);
	}
	else   // status != STATUS_OK
		PrintGkRejectInfo((rejectInfoSt*)&(pSt->rejectInfo), pContentStr);
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasDRQReq(CObjString *pContentStr)
{
	const gkReqRasDRQ *pSt = (const gkReqRasDRQ *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_DRQ_REQ:\n";

	*pContentStr << "GK address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->gatekeeperAddress);
	*pContentStr << "\n";

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);
	TraceEpIdent(pContentStr, pSt->endpointIdent, pSt->epIdentLength);

	*pContentStr << "Disengage reason	 =  " << pSt->disengageReason << "\n";
	*pContentStr << "Is dial in	         =  " << pSt->bIsDialIn << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasDRQInd(CObjString *pContentStr)
{
	const gkIndRasDRQ *pSt = (const gkIndRasDRQ *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_DRQ_IND:\n";
	
	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	if (status != STATUS_OK)
		PrintGkRejectInfo((rejectInfoSt*)&(pSt->rejectInfo), pContentStr);
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasURQReq(CObjString *pContentStr)
{
	const gkReqRasURQ *pSt = (const gkReqRasURQ *)m_pMplMcmsProt->getpData();
	
	*pContentStr <<"\nCONTENT: H323_CS_RAS_URQ_REQ:\n";

	*pContentStr << "GK address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->gatekeeperAddress);
	*pContentStr << "\n";

	*pContentStr << "Call signaling address:	\n";
	for(int i=0; i<MaxNumberOfEndpointIp; i++)
	{
		if( !(::isApiTaNull(&pSt->callSignalingAddress[i].transAddr)) )
		{
			TraceTransportAddrSt(pContentStr, pSt->callSignalingAddress[i]);
			*pContentStr << "\n";
		}
	}

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);
	TraceEpIdent(pContentStr, pSt->endpointIdent, pSt->epIdentLength);
	
	*pContentStr << "AliasesLength		 =  " << pSt->aliasesLength << "\n";
	if (pSt->aliasesLength)
	{
		ALLOCBUFFER(aliasStr, pSt->aliasesLength + 1);
		strncpy(aliasStr, pSt->sAliasesList, pSt->aliasesLength + 1);
		aliasStr[pSt->aliasesLength] = '\0';
		*pContentStr << "Aliases List	     =  " << aliasStr << "\n";
		DEALLOCBUFFER(aliasStr);		
	}
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasURQResponseReq(CObjString *pContentStr)
{
	const gkReqURQResponse *pSt = (const gkReqURQResponse *)m_pMplMcmsProt-> getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_URQ_RESPONSE_REQ:\n";

	*pContentStr << "HS Ras		 =  " << pSt->hsRas << "\n";
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasDRQResponseReq(CObjString *pContentStr)
{
	const gkReqDRQResponse *pSt = (const gkReqDRQResponse *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_DRQ_RESPONSE_REQ:\n";

	*pContentStr << "HS Ras		 =  " << pSt->hsRas << "\n";
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasLRQResponseReq(CObjString *pContentStr)
{
	const gkReqLRQResponse *pSt = (const gkReqLRQResponse *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_LRQ_RESPONSE_REQ:\n";

	*pContentStr << "HS Ras		 =  " << pSt->hsRas << "\n";

	*pContentStr << "Call signaling addresses:	\n";
	for(int i=0; i<MaxNumberOfEndpointIp; i++)
	{
		if( !(::isApiTaNull(&pSt->callSignalingAddress[i].transAddr)) )
		{
			TraceTransportAddrSt(pContentStr, pSt->callSignalingAddress[i]);
			*pContentStr << "\n";
		}
	}
		
	*pContentStr << "RAS address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->rasAddress);
	*pContentStr << "\n";	
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasBRQReq(CObjString *pContentStr)
{
	const gkReqRasBRQ *pSt = (const gkReqRasBRQ *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_BRQ_REQ:\n";

	TraceGkIdent(pContentStr, pSt->gatekeeperIdent, pSt->gkIdentLength);
	TraceEpIdent(pContentStr, pSt->endpointIdent, pSt->epIdentLength);
	
	*pContentStr << "Call Type	 =  ";
	::GetCallTypeName(pSt->callType, *pContentStr);
	*pContentStr << "\n";
	
	*pContentStr << "Bandwidth	 =  " << pSt->bandwidth << "\n";
	*pContentStr << "Is Dial In	 =  " << pSt->bIsDialIn << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasRAIReq(CObjString *pContentStr)
{
    const gkReqRasRAI *pSt = (const gkReqRasRAI *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: H323_CS_RAS_RAI_REQ:\n";
    *pContentStr << "GK address:	\n";
	TraceTransportAddrSt(pContentStr, pSt->gatekeeperAddress);
    *pContentStr << "bAlmostOutOfResources    =  " << pSt->bAlmostOutOfResources << "\n";
    *pContentStr << "maximumAudioCapacity     =  " << pSt->maximumAudioCapacity   << "\n";
    *pContentStr << "maximumVideoCapacity     =  " << pSt->maximumVideoCapacity   << "\n";
    *pContentStr << "currentAudioCapacity     =  " << pSt->currentAudioCapacity   << "\n";
    *pContentStr << "currentVideoCapacity     =  " << pSt->currentVideoCapacity   << "\n";

    *pContentStr << "\n";
    *pContentStr << "numOfSupportedProfiles   =  " << pSt->numOfSupportedProfiles << "\n";

    h460ConferenceProfileExt *pProf = (h460ConferenceProfileExt *) &pSt->profilesArray[0];
    for (int i=0; i<pSt->numOfSupportedProfiles; i++)
    {
        *pContentStr << "---- Profile Details #" << i+1 << "\n";
        *pContentStr << "profileE164ID               = " << pProf->h460ConfProfile.profileE164ID << "\n";
        *pContentStr << "minimumPorts                = " << pProf->h460ConfProfile.minimumPorts << "\n";
        *pContentStr << "partyCallRate               = " << pProf->h460ConfProfile.partyCallRate << "\n";
        *pContentStr << "numOfPortsAvailable         = " << pProf->h460ConfProfile.numOfPortsAvailable << "\n";
        *pContentStr << "maxNumOfPortsCapacity       = " << pProf->h460ConfProfile.maxNumOfPortsCapacity << "\n";
        *pContentStr << "numOfConferencesAvailable   = " << pProf->h460ConfProfile.numOfConferencesAvailable << "\n";
        *pContentStr << "maxNumofConferencesCapacity =  " << pProf->h460ConfProfile.maxNumofConferencesCapacity << "\n";
        *pContentStr << "videoBitRateType            =  " << pProf->h460ConfProfile.videoBitRateType << "\n";
        pProf ++;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasIrrResponseReq(CObjString *pContentStr)
{
	const gkReqRasIRR* pSt = (const gkReqRasIRR *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: H323_CS_RAS_IRR_RESPONSE_REQ:\n";

	//*pContentStr << "Bandwidth	 =  " << pSt->bandwidth << "\n";

}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasTimeoutInd(CObjString *pContentStr)
{
	const gkIndRasTimeout *pSt = (const gkIndRasTimeout *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_TIMEOUT_IND:\n";

	*pContentStr << "transaction		 =";
	switch(pSt->transaction)
	{
		case cmRASGatekeeper:  
			*pContentStr << "GRQ transaction\n";
			break;
		case cmRASRegistration:  
			*pContentStr << "RRQ transaction\n";
			break;
		case cmRASUnregistration:  
			*pContentStr << "URQ transaction\n";
			break;
		case cmRASAdmission:    
			*pContentStr << "ARQ transaction\n";
			break;
		case cmRASDisengage:   
			*pContentStr << "DRQ transaction\n";
			break;
		case cmRASBandwidth:  
       		*pContentStr << "BRQ transaction\n";
			break;
		case cmRASLocation:  
			*pContentStr << "LRQ transaction\n";
			break;
		case cmRASInfo:      
			*pContentStr << "IRQ transaction\n";
			break;
        case cmRASResourceAvailability:
            *pContentStr << "RAI transaction\n";
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasFailInd(CObjString *pContentStr)
{
	const gkIndRasFail *pSt = (const gkIndRasFail *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_FAIL_IND:\n";

	*pContentStr << "Fail Indication Opcode		 =" << pSt->FailIndicationOpcode << "\n";
	
	ALLOCBUFFER(message, MaxErrorMessageSize);
	memcpy(message, pSt->message, MaxErrorMessageSize);
	*pContentStr << "Fail Indication Message	 =" << message << "\n";
	DEALLOCBUFFER(message); 
}

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasGkURQInd(CObjString *pContentStr)
{
	const gkIndURQFromGk *pSt = (const gkIndURQFromGk *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_GKURQ_IND:\n";

	*pContentStr << "HS Ras					 =  " << pSt->hsRas << "\n";
	*pContentStr << "Unregister Reason		 =  " << pSt->unRegisterReason << "\n";

	PrintAltGkInfo( (altGksListSt*)&(pSt->altGkList), pContentStr);
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasGkDRQInd(CObjString *pContentStr)
{
	const gkIndDRQFromGk *pSt = (const gkIndDRQFromGk *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_GKDRQ_IND:\n";

	*pContentStr << "HS Ras					 =  " << pSt->hsRas << "\n";
	*pContentStr << "Disengage Reason		 =  " << pSt->disengageReason << "\n";
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasGkLRQInd(CObjString *pContentStr)
{
	const gkIndLRQFromGk *pSt = (const gkIndLRQFromGk *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_GKLRQ_IND:\n";
	
	ALLOCBUFFER(destInfo, MaxAddressListSize);
	strncpy(destInfo, pSt->destinationInfo, MaxAddressListSize);
	*pContentStr << "Dest info           =  " << destInfo << "\n";
	DEALLOCBUFFER(destInfo);
	
	ALLOCBUFFER(sourceInfo, MaxAddressListSize);
	strncpy(sourceInfo, pSt->sourceInfo, MaxAddressListSize);
	*pContentStr << "Source info         =  " << sourceInfo << "\n";
	DEALLOCBUFFER(sourceInfo); 	

	//	*pContentStr << "Call Id			 = "  << pSt->callId << "\n";
	*pContentStr << "HS Ras				 =  " << pSt->hsRas << "\n";	
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasURQInd(CObjString *pContentStr)
{
	const gkIndRasURQ *pSt = (const gkIndRasURQ *)m_pMplMcmsProt->getpData();
	
	*pContentStr <<"\nCONTENT: H323_CS_RAS_GKLRQ_IND:\n";
	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	if (status != STATUS_OK)
		PrintGkRejectInfo((rejectInfoSt*)&(pSt->rejectInfo), pContentStr);
}


////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasBRQInd(CObjString *pContentStr)
{
	const gkIndRasBRQ *pSt = (const gkIndRasBRQ *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_CS_RAS_BRQ_IND:\n";
	*pContentStr <<"Bandwidth            = " << pSt->bandwidth << "\n";
	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	if (status != STATUS_OK)
		PrintGkRejectInfo((rejectInfoSt*)&(pSt->rejectInfo), pContentStr);
}	

////////////////////////////////////////////////////////////////////////////////
void CGkMplMcmsProtocolTracer::TraceRasRACInd(CObjString *pContentStr)
{
    *pContentStr <<"\nCONTENT: H323_CS_RAS_RAC_IND:\n";
}
