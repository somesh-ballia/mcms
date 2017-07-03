//+========================================================================+
//                            SIPInternals.cpp                             |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPInternals.cpp                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#include <stdarg.h>
#include <ostream>
#include <stdlib.h>

#include "Macros.h"
#include "Trace.h"
#include "Segment.h"
#include "NStream.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "StringsLen.h"
#include "IpScm.h"
//#include "IpCommonTypes.h"
#include "DataTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "Capabilities.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"

#include "CapClass.h"
#include "SipCaps.h"
#include "SIPInternals.h"
#include "SysConfigKeys.h"

///////////////////////////////////////////////////////////////////////////////////////
//Retrieve current video rate and looking if asked to
//reduce the rate in system.cfg. If YES return fixed rate
//If NOT no change return same video rate
DWORD SipGetVideoFlowControlRateFix(DWORD wVideoRate)
{
	DWORD nRetRate = wVideoRate;
	char *cFixRates = NULL, *pComma = NULL, *pTemp, cValue[10] = {0};
	DWORD nConfRatesNum = 7;
	DWORD VideoRates[] = {64, 128, 192, 256, 384, 512, 768};
	ALLOCBUFFER(cLog, 256);
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string sKey;
	std::string sRates;

	//getting system.cfg rates adjustment values
	sKey = "SIP_USER_AGENT_FLOW_CONTROL_RATE";
	sysConfig->GetDataByKey(sKey, sRates);
	if ( strlen(cFixRates = (char*) sRates.c_str()) > 0 )
	{
		pComma = cFixRates;

		FPTRACE2(eLevelInfoNormal, "SipGetVideoFlowControlRateFix Rates =", cFixRates);

		//loop over confs rates
		for ( DWORD i = 0; i < nConfRatesNum; i++)
		{
			//processing only relevant video rate
			//if current video less or equal for rate in VideoRates array
			//we arrive to relevant location in cFixRates string too
			if ( wVideoRate <= VideoRates[i])
			{
				pTemp = strstr(pComma, ",");
				if (!pTemp) pTemp = cFixRates + strlen(cFixRates);

				//getting fix video rate value for current upper limit from system.cfg string
				int maxStrLenForKW = min((int)sizeof(cValue) - 1, pTemp - pComma);
				strncpy(cValue, pComma, maxStrLenForKW);
				cValue[maxStrLenForKW] = 0;

				DWORD nFixRate = atoi(cValue);

				//if fix video rate is different from current video rate
				if ( nFixRate != VideoRates[i] )
				{
					nRetRate = nFixRate;
					sprintf(cLog, " Setting Rates: new [%d]Kb old [%d]Kb video limit [%d]Kb", nRetRate, wVideoRate, VideoRates[i]);
				}
				else
				{
					sprintf(cLog, " No rate change for video in limit [%d]Kb current rate [%d]Kb", VideoRates[i], wVideoRate);
				}

				break;
			}

			//go to next rate value in system.cfg string
			pTemp = strstr(pComma, ",");
			if (!pTemp)
			{
				pComma = cFixRates + strlen(cFixRates);
			}
			else
			{
				pComma = pTemp + 1;
			}
		}
	}
	else
	{
		sprintf(cLog, " Not defined fix for rates");
	}


	FPTRACE2(eLevelInfoNormal, "SipGetVideoFlowControlRateFix ", cLog);
	DEALLOCBUFFER(cLog);

	return nRetRate;
}

///////////////////////////////////////////////////////////////////////////////////////
// The goal is alignment call rate in case
// if it doesn't defined by remote (or doesn't recognized by card) with rates in caps
void SDPRateAlignment(sipSdpAndHeadersSt* sdp)
{
	ALLOCBUFFER(cLog, 256);

	//in case if whole call rate does not defined
	if ( 0xFFFFFFFF == sdp->callRate )
	{
		if (sdp->sipMediaLinesLength)
		{
			sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &sdp->capsAndHeaders[sdp->sipMediaLinesOffset];
			int mediaLinePos = 0;

			CSipCaps* pSDPCaps = new CSipCaps;

			for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

				sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
				mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

				const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
				const BYTE* pTemp = (const BYTE*) pCapBuffer;

				for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
				{
					CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
					cmCapDataType eType = capInfo.GetCapType();
					pSDPCaps->AddCapSet(eType, pCapBuffer);

					pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
					pCapBuffer = (capBuffer*) pTemp;
				}
			}

			sdp->callRate = pSDPCaps->GetTotalRate();

			sprintf(cLog, "Call rate calculated is [%d]", sdp->callRate);
			POBJDELETE(pSDPCaps);
		}
		else
		{
			sprintf(cLog, "No SDP provided");
		}
	}
	else
	{
		sprintf(cLog, "No alignment. Call rate is [%d]", sdp->callRate);
	}

	FPTRACE2(eLevelInfoNormal, "SDPRateAlignment ", cLog);
	DEALLOCBUFFER(cLog);
}

///////////////////////////////////////////////////////////////////////////////////////
void DumpMediaIp(CObjString* pMsgStr,const mcTransportAddress & mediaIpSip)
{
	*pMsgStr << "Media IP Address: ";
	CLargeString trace;
	int i = 0;
	if (mediaIpSip.ipVersion == eIpVersion4)
	{
		// Case IpV4
		char szIP[20];
		::SystemDWORDToIpString(mediaIpSip.addr.v4.ip, szIP);
		*pMsgStr << szIP;
		*pMsgStr << "::" << (WORD)mediaIpSip.port << '(';
		char str[30];
		sprintf(str,"%x",mediaIpSip.addr.v4.ip);
		*pMsgStr << "Ip: 0x" << str;
	}
	else
	{
		// Case IpV6
		*pMsgStr << "\n" << "Scope Id: " << mediaIpSip.addr.v6.scopeId << '\n';
		char szIP[64];
		::ipToString(mediaIpSip,szIP,1);
		*pMsgStr << "Ip: " << szIP;
		*pMsgStr << "::" << (WORD)mediaIpSip.port << '(';
	}

	if (mediaIpSip.ipVersion == eIpVersion4)
		*pMsgStr << " V4)\n";
	else
		*pMsgStr << " V6)\n";
}

///////////////////////////////////////////////////////////////////////////////////////
void DumpMediaLineEntry(sipMediaLinesEntrySt *pMediaLinesEntry,std::ostream &ostr)
{
	int mediaLinePos = 0;
	unsigned int i, j;
	FPTRACE2INT(eLevelInfoNormal,"DumpMediaLineEntry pMediaLinesEntry->numberOfMediaLines ",pMediaLinesEntry->numberOfMediaLines);

	for (j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

		FPTRACE2INT(eLevelInfoNormal, "mediaLinePos :",mediaLinePos);
		sipMediaLineSt *pMediaLine = (sipMediaLineSt *)(((APIS8*) &pMediaLinesEntry->mediaLines[0])+mediaLinePos);
		mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

		FPTRACE2INT(eLevelInfoNormal, "lenOfDynamicSection :",pMediaLine->lenOfDynamicSection);

		capBuffer* pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
		BYTE*      pTemp	  = (BYTE *)pCapBuffer;

		unsigned int numOfCaps = pMediaLine->numberOfCaps;
		FPTRACE2INT(eLevelInfoNormal, "numberOfCaps :",pMediaLine->numberOfCaps);
		for (i=0 ; i<numOfCaps; i++)
		{
			CBaseCap * pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
			if (pCap)
			{
				COstrStream msg;
				pCap->Dump(msg);
				ostr << msg.str().c_str();
				ostr <<"\n";
				pTemp += (sizeof(capBufferBase) + pCapBuffer->capLength);
				pCapBuffer = (capBuffer *)pTemp;
				FPTRACE2(eLevelInfoNormal, "In Cap  str - ",msg.str().c_str());

			}
			else
				FPTRACE(eLevelInfoNormal, "No Cap!!!");

			POBJDELETE(pCap);
		}
	}
}

void DumpSdpAndHeadersToStream(CObjString* pMsgStr,const sipSdpAndHeadersSt& sdpAndHeaders)
{
	*pMsgStr << "\nSDP Dynamic Length: " << sdpAndHeaders.lenOfDynamicSection << "\n";
	*pMsgStr << "SDP Media Lines Offset: " << sdpAndHeaders.sipMediaLinesOffset << "\n";
	*pMsgStr << "SDP Media Lines Length: " << sdpAndHeaders.sipMediaLinesLength << "\n";
	*pMsgStr << "SDP Headers Offset: " << sdpAndHeaders.sipHeadersOffset << "\n";
	*pMsgStr << "SDP Headers Length: " << sdpAndHeaders.sipHeadersLength << "\n";
	if (sdpAndHeaders.lenOfDynamicSection) // if there is a dynamic section
	{
		// print headers
		if (sdpAndHeaders.sipHeadersLength) // if there are headers
		{
			sipMessageHeaders* pHeaders = (sipMessageHeaders *)(sdpAndHeaders.capsAndHeaders + sdpAndHeaders.sipHeadersOffset);

			//FSN-671 MFW-Core, BRIDGE-18254
			if((*pHeaders).numOfHeaders>100){
				FPTRACE(eLevelError, "DumpSdpAndHeadersToStream headers.numOfHeaders > 100 !!!!!!!!!!!!!!! Don't create and print sip headers");
				FPASSERT((*pHeaders).numOfHeaders);
				return;
			}

			CSipHeaderList headers(*pHeaders);
			headers.DumpToStream(pMsgStr);
		}

		// print sdp
		if (sdpAndHeaders.sipMediaLinesLength) // if there are capabilities
		{
			sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &sdpAndHeaders.capsAndHeaders[sdpAndHeaders.sipMediaLinesOffset];
			int mediaLinePos = 0;
			unsigned int i, j, k;

			*pMsgStr << "Call  Rate	: " << (int)sdpAndHeaders.callRate  << " (bits per second: " << 100 * (int)sdpAndHeaders.callRate << ")\n";
			*pMsgStr << "CNAME string	: " << sdpAndHeaders.cCname << "\n";
			*pMsgStr << "msVideoRateTx: " << (int)sdpAndHeaders.msVideoRateTx << "\n";
			*pMsgStr << "msVideoRateRx: " << (int)sdpAndHeaders.msVideoRateRx << "\n";
			*pMsgStr << "msAudioRateTx: " << (int)sdpAndHeaders.msAudioRateTx << "\n";
			*pMsgStr << "msAudioRateRx: " << (int)sdpAndHeaders.msAudioRateRx << "\n";
			if(sdpAndHeaders.rssConnectionType)
			{
				*pMsgStr << "rssConnectionType: " << (int)sdpAndHeaders.rssConnectionType << "\n";
			}
			*pMsgStr << "Media Lines Entry Dynamic Length: " << pMediaLinesEntry->lenOfDynamicSection << "\n";
			*pMsgStr << "Number of media lines: " << pMediaLinesEntry->numberOfMediaLines << "\n";
			*pMsgStr << "Main Media Ip Present: " << pMediaLinesEntry->bMainMediaIpPresent<< "\n";
			DumpMediaIp(pMsgStr, pMediaLinesEntry->mainMediaIp.transAddr);


			for (j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

				sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
				mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

				*pMsgStr << "Media Line #: " << j << "\n";

				*pMsgStr << "Address params; Type: " << pMediaLine->mediaIp.unionProps.unionType << ", Size: " << pMediaLine->mediaIp.unionProps.unionSize << "\n";
				DumpMediaIp(pMsgStr, pMediaLine->mediaIp.transAddr);
				*pMsgStr << "Rtcp port: " << (int)pMediaLine->rtcpPort << "\n";
				*pMsgStr << "Index	: " << (int)pMediaLine->index  << "\n";
				*pMsgStr << "type	: " << (int)pMediaLine->type  << "\n";
				*pMsgStr << "subType	: " << GetMediaLineSubTypeStr((eMediaLineSubType)pMediaLine->subType)  << "\n";
				if (pMediaLine->subType == eMediaLineTypeNotSupported)
						*pMsgStr << "notSupportedSubType	: " <<(int)pMediaLine->notSupportedSubType  << "\n";
				*pMsgStr << "intType	: " << (int)pMediaLine->internalType  << "\n";
				*pMsgStr << "content	: " << (int)pMediaLine->content  << "\n";
				*pMsgStr << "label	: " << (char*)pMediaLine->label  << "\n";
				*pMsgStr << "msi    : " << (ULONGLONG)pMediaLine->msi  << "\n";
				*pMsgStr << "rtcp_rsize    : " << (ULONGLONG)pMediaLine->rtcp_rsize  << "\n";
				*pMsgStr << "ssrcrange    : " << (ULONGLONG)pMediaLine->ssrcrange[0] << " - " << (ULONGLONG)pMediaLine->ssrcrange[1]  << "\n";				
				if(sdpAndHeaders.rssConnectionType)
				{
					*pMsgStr << "rssVideoLayout	: " << (int)pMediaLine->rssVideoLayout  << "\n";
				}
				*pMsgStr << "mid  	: " << (int)pMediaLine->mid  << "\n";  //added for ANAT
				*pMsgStr << "midAnatGroup  : ";
				for (k=0; k<MAX_GROUP_ANAT_MEMBER_NUM; k++)
				{
					*pMsgStr << (int)pMediaLine->midAnatGroup[k]<< " "; 
				}
				*pMsgStr << "\n"; 
				if (pMediaLine->intraFlag)
					*pMsgStr << "Intra flag: true\n";
				*pMsgStr << "Num of caps	: " << (int)pMediaLine->numberOfCaps  << "\n";
				*pMsgStr << "Media Line Dynamic Length: " << pMediaLine->lenOfDynamicSection << "\n";

				capBuffer* pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
				BYTE*      pTemp	  = (BYTE *)pCapBuffer;

				// in comment until the compilation error will be resolved
				unsigned int numOfCaps = pMediaLine->numberOfCaps;
				for (i=0 ; i<numOfCaps; i++)
				{
					*pMsgStr << i+1 <<"\tCap - - - - - - - -"  << "  "<<(CapEnum)pCapBuffer->capTypeCode ;//N.A. DEBUG VP8
					CBaseCap * pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
					if (pCap)
					{
						COstrStream msg;
						pCap->Dump(msg);
						*pMsgStr << msg.str().c_str();
						*pMsgStr << "Payload Type					  = " << (int)pCapBuffer->sipPayloadType << "\n";
						pTemp += (sizeof(capBufferBase) + pCapBuffer->capLength);
						pCapBuffer = (capBuffer *)pTemp;
					}
					POBJDELETE(pCap);
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void DumpXMLAndHeadersToStream(CObjString* pMsgStr, sipSdpAndHeadersSt& sdpAndHeaders)
{
	*pMsgStr << "\nSDP Dynamic Length: " << sdpAndHeaders.lenOfDynamicSection << "\n";
	*pMsgStr << "SDP Media Lines Offset: " << sdpAndHeaders.sipMediaLinesOffset << "\n";
	*pMsgStr << "SDP Media Lines Length: " << sdpAndHeaders.sipMediaLinesLength << "\n";
	*pMsgStr << "SDP Headers Offset: " << sdpAndHeaders.sipHeadersOffset << "\n";
	*pMsgStr << "SDP Headers Length: " << sdpAndHeaders.sipHeadersLength << "\n";
	if (sdpAndHeaders.lenOfDynamicSection) // if there is a dynamic section
	{
		// print headers
		if (sdpAndHeaders.sipHeadersLength) // if there are headers
		{
			sipMessageHeaders* pHeaders = (sipMessageHeaders *)(sdpAndHeaders.capsAndHeaders + sdpAndHeaders.sipHeadersOffset);
			CSipHeaderList headers(*pHeaders);
			headers.DumpToStream(pMsgStr);
		}

		int size = sdpAndHeaders.sipMediaLinesLength;
		char* DynamicSection = (char*)&sdpAndHeaders.capsAndHeaders[sdpAndHeaders.sipMediaLinesOffset];
		char *pXMLBuffer = new char[size+1];

		memset(pXMLBuffer,'\0', size+1);
		memcpy(pXMLBuffer,DynamicSection,size);

		*pMsgStr << "XML Buffer: " << pXMLBuffer;

		POBJDELETE(pXMLBuffer);
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void DumpXMLToStream(std::ostream &ostr, sipSdpAndHeadersSt* sdpAndHeaders)
{
	int size = sdpAndHeaders->sipMediaLinesLength;
		char* DynamicSection = (char*)&sdpAndHeaders->capsAndHeaders[sdpAndHeaders->sipMediaLinesOffset];
		char *pXMLBuffer = new char[size+1];
		memset(pXMLBuffer,'\0', size+1);
		memcpy(pXMLBuffer,DynamicSection,size);
		ostr << "XML Buffer: " << pXMLBuffer;

		POBJDELETE(pXMLBuffer);


}

//////////////////////////////////////////////////////////////////////////////////////
enMediaDirection GetMediaDirectionAttribute(sipSdpAndHeadersSt *pSdpAndHeaders, cmCapDataType eMediaType, ERoleLabel eRole)
{
	if ( pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength )
	{
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		int mediaLinePos = 0;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if ( (eMediaType == cmCapAudio && pMediaLine->internalType != kMediaLineInternalTypeAudio) ||
				 (eMediaType == cmCapVideo && eRole == kRolePeople && pMediaLine->internalType != kMediaLineInternalTypeVideo) ||
				 (eMediaType == cmCapVideo && eRole == kRolePresentation && pMediaLine->internalType != kMediaLineInternalTypeContent) ||
				 (eMediaType == cmCapData && pMediaLine->internalType != kMediaLineInternalTypeFecc))
				continue;

			const capBuffer* pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
			const BYTE*	pTemp = (const BYTE*)pCapBuffer;

			for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
			{
				BaseCapStruct *pBaseCap  = (BaseCapStruct*)pCapBuffer->dataCap;
				cmCapDataType eSdpType = (cmCapDataType)pBaseCap->header.type;

				if(eSdpType == eMediaType)
				{
					enMediaDirection eSdpDirection = (enMediaDirection)pBaseCap->header.direction;
					return eSdpDirection;
				}

				pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)pTemp;
			}
		}
	}

	return kInactive;
}

//////////////////////////////////////////////////////////////////////////////////////
void SetMediaLineDirectionAttribute(sipMediaLineSt *pMediaLine, cmCapDirection eDirection)
{
	const capBuffer* pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
	const BYTE*	pTemp = (const BYTE*)pCapBuffer;

	for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
	{
		BaseCapStruct *pBaseCap  = (BaseCapStruct*)pCapBuffer->dataCap;

		CapEnum capCodeType = (CapEnum)pBaseCap->header.capTypeCode;

		if(capCodeType != eSdesCapCode)
			pBaseCap->header.direction = eDirection;
		else
			pBaseCap->header.direction = cmCapReceiveAndTransmit;


		pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTemp;
	}
}
void SetDirectionAttributesForAVMCU(sipSdpAndHeadersSt *pSettingSdpAndHeaders)
{
	if ( pSettingSdpAndHeaders && pSettingSdpAndHeaders->sipMediaLinesLength )
	{
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSettingSdpAndHeaders->capsAndHeaders[pSettingSdpAndHeaders->sipMediaLinesOffset];
		int mediaLinePos = 0;
		BYTE isFirstVideoMLine = YES;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++)
		{
			sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			if (pMediaLine->internalType == kMediaLineInternalTypeVideo)
			{
				if(!isFirstVideoMLine)
					SetMediaLineDirectionAttribute(pMediaLine, cmCapReceive);
				else isFirstVideoMLine = NO;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////
BYTE SetMediaDirectionAttribute(sipSdpAndHeadersSt *pSettingSdpAndHeaders, BYTE eInfoSourceMediaDirection, cmCapDataType eMediaType, BYTE bForceUnmute, ERoleLabel eRole)
{
	BYTE rVal = STATUS_OK;

	if ( pSettingSdpAndHeaders && pSettingSdpAndHeaders->sipMediaLinesLength )
	{
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSettingSdpAndHeaders->capsAndHeaders[pSettingSdpAndHeaders->sipMediaLinesOffset];
		int mediaLinePos = 0;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			const capBuffer* pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
			const BYTE*	pTemp = (const BYTE*)pCapBuffer;

			if ((eMediaType == cmCapAudio && pMediaLine->internalType == kMediaLineInternalTypeAudio) ||
				(eMediaType == cmCapVideo && eRole == kRolePeople && pMediaLine->internalType == kMediaLineInternalTypeVideo) ||
				(eMediaType == cmCapVideo && eRole == kRolePresentation && pMediaLine->internalType == kMediaLineInternalTypeContent) ||
				(eMediaType == cmCapData  && pMediaLine->internalType == kMediaLineInternalTypeFecc))
			{
				for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
				{
					BaseCapStruct *pBaseCap  = (BaseCapStruct*)pCapBuffer->dataCap;
				//	cmCapDataType eSdpType = (cmCapDataType)pBaseCap->header.type;

					CapEnum capCodeType = (CapEnum)pBaseCap->header.capTypeCode;

					if(capCodeType != eSdesCapCode)
					{
						enMediaDirection eSdpDirection = (enMediaDirection)pBaseCap->header.direction;

						CLargeString traceStr;
						traceStr << "(!sdes before, line: " << j << ", cap: " << i << "): direction: " << (int)eSdpDirection << ", mediaType: " << (int)eMediaType << ", Role: " << (int)eRole;
					//	FPTRACE( eLevelInfoNormal, traceStr.GetString() );

						if(eSdpDirection != kInactive || bForceUnmute)
						{
							pBaseCap->header.direction = ::CalcOppositeDirection((cmCapDirection)eInfoSourceMediaDirection);

							CLargeString traceStr1;
							traceStr1 << "(!sdes after, line: " << j << ", cap: " << i << "): direction: " << (int)(pBaseCap->header.direction) << ", mediaType: " << (int)eMediaType << ", Role: " << (int)eRole;
						//	FPTRACE( eLevelInfoNormal, traceStr1.GetString() );
						}
					}
					else
					{
						pBaseCap->header.direction = cmCapReceiveAndTransmit;

						CLargeString traceStr2;
						traceStr2 << "(sdes, line: " << j << ", cap: " << i << "): direction: " << (int)(pBaseCap->header.direction) << ", mediaType: " << (int)eMediaType << ", Role: " << (int)eRole;
						FPTRACE( eLevelInfoNormal, traceStr2.GetString() );
					}

					pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
					pCapBuffer = (capBuffer*)pTemp;
				}
			}
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
BYTE SetTheDirectionAttribute(sipSdpAndHeadersSt *pSettingSdpAndHeaders, BYTE eAudioDirection, BYTE eVideoDirection, BYTE eDataDirection, BYTE eContentDirection, BYTE eBfcpDirection, BYTE bForceUnmute)
{
	CLargeString traceStr;
	traceStr << "SetTheDirectionAttribute - eAudioDirection: " << (int)eAudioDirection << ", eVideoDirection: " << (int)eVideoDirection
			 << ", eDataDirection: " << (int)eDataDirection << ", eContentDirection: " << (int)eContentDirection << ", eBfcpDirection: " << (int)eBfcpDirection;
	FPTRACE( eLevelInfoNormal, traceStr.GetString() );

	BYTE rVal = STATUS_OK;
	rVal |= SetMediaDirectionAttribute(pSettingSdpAndHeaders, eAudioDirection, cmCapAudio,bForceUnmute);
	rVal |= SetMediaDirectionAttribute(pSettingSdpAndHeaders, eVideoDirection, cmCapVideo,bForceUnmute);
	rVal |= SetMediaDirectionAttribute(pSettingSdpAndHeaders, eDataDirection, cmCapData,bForceUnmute);
	rVal |= SetMediaDirectionAttribute(pSettingSdpAndHeaders, eContentDirection, cmCapVideo,bForceUnmute,kRolePresentation);
	rVal |= SetMediaDirectionAttribute(pSettingSdpAndHeaders, eBfcpDirection, cmCapBfcp,bForceUnmute);

	return rVal;
}


///////////////////////////////////////////////////
WORD IsValidIpV4Address(const char* pIP)
{
	char *pFront, *pRear;
	char sTmp[IP_ADDRESS_LEN];
	int i, val;
	WORD bIsValid = YES;
	BYTE val255Counter = 0;

	if ( strlen(pIP) > IP_ADDRESS_LEN )
	{
		//This condition also insures that all fields are smaller then sTmp size.
		return NO;
	}

	pRear = (char*) pIP;

	for( i = 0; i < 4; i++ )
	{
		if ( i == 3 )
		{ //last field
		  strncpy(sTmp, pRear, sizeof(sTmp)-1);
		  sTmp[sizeof(sTmp)-1] = '\0';
			val = atoi(sTmp);
		}
		else if ( (pFront = strchr(pRear, '.')) )
		{
			strncpy(&sTmp[0], pRear, pFront - pRear);
			sTmp[pFront - pRear] = '\0';
			val = atoi(sTmp);

			pRear = (pFront + 1);
			if ( strlen(pRear) == 0 )
			{
				bIsValid = NO; //string that end by '.'
				break;
			}
		}
		else //not found next "."
		{
			bIsValid = NO;
			break;
		}

		//retrieved value does not valid
		if ( (val < 0) || (val > 255) )
		{
			bIsValid = NO;
			break;
		}
		else if((val == 255) || (val == 0))
		{
			val255Counter++;
			if(val255Counter == 4)
			{
				bIsValid = NO;
				break;
			}
		}
	}

	return bIsValid;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
 	void SetBfcpInSipPartyScm(CIpComMode* pPartyScm,BOOL IsOfferer, enTransportType transportType)
 	{
 		enTransportType ToSetTransportType = eUnknownTransportType;

 		CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
 		std::string dialOutMode;

 		if(IsOfferer)
 		{
			sysConfig->GetDataByKey(CFG_KEY_SIP_BFCP_DIAL_OUT_MODE, dialOutMode);

			if(dialOutMode.compare("UDP")==0)
				ToSetTransportType = eTransportTypeUdp;
			else if(dialOutMode.compare("TCP")==0)
				ToSetTransportType = eTransportTypeTcp;
 		}
 		else // Answerer - In this case we want to set the Transport Type to that of the Remote b/c Remote is offerer
 		{
 			ToSetTransportType = transportType;
 			FPTRACE2INT(eLevelInfoNormal,"SetBfcpInSipPartyScm - Answerer Case Set to Remote BFCP Transport Type", ToSetTransportType);
 		}

 		FPTRACE2INT(eLevelInfoNormal,"SetBfcpInSipPartyScm - new transportType according to flag:", ToSetTransportType);

 		pPartyScm->SetBfcp(ToSetTransportType);
 	}

