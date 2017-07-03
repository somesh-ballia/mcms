// GlobalH323MonitoringFunc.cpp: Global functions for H323 monitoring

#include "NStream.h"
#include "Capabilities.h"
#include "IpRtpFeccRoleToken.h"
#include "IpCsContentRoleToken.h"
#include "ConfPartyH323Defines.h"
#include "H264.h"
#include "CapClass.h"
#include "IpCommonUtilTrace.h"



char* GetRoleStr(DWORD role)
{
	char* roleString;
	switch(role)
	{
		case kRolePeople:
			roleString = "";
			break;
		case kRoleContent:
			roleString = "People+Content Role: Content";
			break;
		case kRolePresentation:
			roleString = "H.239 Role: Presentation";
			break;
		case kRoleLive:
			roleString = "Role live";
			break;
		case kRoleLiveOrPresentation:
			roleString = "H.239 Role: Live or Presentation";
			break;
		default:
			roleString = "";
			break;
	}
	return roleString;
}


void PrintAnnexType(std::ostream& ostr, int annex)
{
	switch(annex)
	{
	case typeAnnexB:
		ostr << "Annex B";
		break;
	case typeAnnexD:
		ostr << "Annex D";
		break;
	case typeAnnexE:
		ostr << "Annex E";
		break;
	case typeAnnexF:
		ostr << "Annex F";
		break;
	case typeAnnexG:
		ostr << "Annex G";
		break;
	case typeAnnexH:
		ostr << "Annex H";
		break;
	case typeAnnexI:
		ostr << "Annex I";
		break;
	case typeAnnexJ:
		ostr << "Annex J";
		break;
	case typeAnnexK:
		ostr << "Annex K";
		break;
	case typeAnnexL:
		ostr << "Annex L";
		break;
	case typeAnnexM:
		ostr << "Annex M";
		break;
	case typeAnnexN:
		ostr << "Annex N";
		break;
	case typeAnnexO:
		ostr << "Annex O";
		break;
	case typeAnnexP:
		ostr << "Annex P";
		break;
	case typeAnnexQ:
		ostr << "Annex Q";
		break;
	case typeAnnexR:
		ostr << "Annex R";
		break;
	case typeAnnexS:
		ostr << "Annex S";
		break;
	case typeAnnexT:
		ostr << "Annex T";
		break;
	case typeAnnexU:
		ostr << "Annex U";
		break;
	case typeAnnexV:
		ostr << "Annex V";
		break;
	case typeAnnexW:
		ostr << "Annex W";
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////
void  PrintH323CustomOF(std::ostream& ostr, customPic_St * pCustomFormats, int index)
{
	int xMax, yMax, xMin, yMin;

	char * pChar = (char *)pCustomFormats->customPicPtr;
	pChar += index * sizeof(customPicFormatSt);
	customPicFormatSt * pSpecificCustomFormat = (customPicFormatSt *)pChar;

	xMax = pSpecificCustomFormat->maxCustomPictureWidth;
	yMax = pSpecificCustomFormat->maxCustomPictureHeight;
	xMin = pSpecificCustomFormat->minCustomPictureWidth;
	yMin = pSpecificCustomFormat->minCustomPictureHeight;

	if((xMax >= 256) && (yMax >= 192))
		ostr << "XGA - ";
	else if ((xMax >= 200) && (yMax >= 150))
		ostr << "SVGA - ";
	else if((xMax >= 176) && (yMax >= 120))
		ostr << "NTSC - ";
	else if((xMax >= 160) && (yMax >= 120))
		ostr << "VGA - ";
	else if((xMax >= 88) && (yMax >= 72))
		ostr << "PAL (CIF) - ";
	else if((xMax >= 88) && (yMax >= 60))
		ostr << "1/4 NTSC - ";

	if(xMax != xMin)
	{
		ostr << " max width: " <<  xMax;
		ostr << ", min width: " <<  xMin << ".";
	}
	if(yMax != yMin)
	{
		ostr << " max height: " <<  yMax;
		ostr << ", min height: " <<  yMin << ".";
	}
	else
	{
		if(pSpecificCustomFormat->standardMPI)
			ostr << " Standard MPI: " << (int)pSpecificCustomFormat->standardMPI;
		else
			ostr << " Custom MPI: " << (int)pSpecificCustomFormat->customMPI;
	}
}



/////////////////////////////////////////////////////////////////////////////
char* GetH264ProfileAsString(WORD profileValue)
{
	char* profileString;

	switch(profileValue)
	{
		case H264_Profile_BaseLine:
		{
			profileString = "BaseLine Profile";
			break;
		}
		case H264_Profile_Main:
		{
			profileString = "Main Profile";
			break;
		}
		case H264_Profile_Extended:
		{
			profileString = "Extended Profile";
			break;
		}
		case H264_Profile_High:
		{
			profileString = "High Profile";
			break;
		}
		default :
		{
			profileString = "Unknown Profile ";
			break;
		}
	}
	return profileString;
}

/////////////////////////////////////////////////////////////////////
char* GetH264LevelAsString(BYTE levelValue)
{
	char* levelString;

	switch(levelValue)
	{
		case H264_Level_1:
		{
			levelString = "Level 1";
			break;
		}
		case H264_Level_1_1:
		{
			levelString = "Level 1.1";
			break;
		}
		case H264_Level_1_2:
		{
			levelString = "Level 1.2";
			break;
		}
		case H264_Level_1_3:
		{
			levelString = "Level 1.3";
			break;
		}
		case H264_Level_2:
		{
			levelString = "Level 2";
			break;
		}
		case H264_Level_2_1:
		{
			levelString = "Level 2.1";
			break;
		}
		case H264_Level_2_2:
		{
			levelString = "Level 2.2";
			break;
		}
		case H264_Level_3:
		{
			levelString = "Level 3";
			break;
		}
		case H264_Level_3_1:
		{
			levelString = "Level 3.1";
			break;
		}
		case H264_Level_3_2:
		{
			levelString = "Level 3.2";
			break;
		}
		case H264_Level_4:
		{
			levelString = "Level 4";
			break;
		}
		case H264_Level_4_1:
		{
			levelString = "Level 4.1";
			break;
		}
		case H264_Level_4_2:
		{
			levelString = "Level 4.2";
			break;
		}
		case H264_Level_5:
		{
			levelString = "Level 5";
			break;
		}
		case H264_Level_5_1:
		{
			levelString = "Level 5.1";
			break;
		}

			default :
		{
			levelString = "Level X By Default";
			break;
		}

	}
	return levelString;
}

///////////////////////////////////////////////////////////////////////////////////////
void PrintCapDataType(std::ostream& ostr, ctCapStruct* pCap)
{

	switch(pCap->type)
	{
	case cmCapAudio:
		ostr << "AudioCap" << "\n";
		break;
	case cmCapVideo:
		ostr << "VideoCap" << "\n";
		break;
	case cmCapData:
		ostr << "DataCap" << "\n";
		break;
	case cmCapNonStandard:
		ostr << "NonStandardCap" << "\n";
		break;
	case cmCapUserInput:
		ostr << "UserInputCap" << "\n";
		break;
	case cmCapConference:
		ostr << "ConferenceCap" << "\n";
		break;
	case cmCapH235:
		ostr << "H235Cap" << "\n";
		break;
	case cmCapGeneric:
		ostr << "GenericCap" << "\n";
		break;
	case cmCapMultiplexedStream:
		ostr << "MultiplexedStreamCap" << "\n";
		break;
	case cmCapAudioTelephonyEvent:
		ostr << "AudioTelephonyEventCap" << "\n";
		break;
	case cmCapAudioTone:
		ostr << "AudioToneCap" << "\n";
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void PrintBfcpData(std::ostream& ostr, bfcpCapStruct *pCapStruct)
{
	if((eBfcpSetup)pCapStruct->setup != bfcp_setup_null) //to print non-empty bfcp cap
	{
		ostr << "\n setup         " << GetBfcpSetupStr((eBfcpSetup)pCapStruct->setup);
		ostr << "\n connection    " << GetBfcpConnectionStr((eBfcpConnection)pCapStruct->connection);
		ostr << "\n floor control " << GetBfcpFloorCtrlStr((eBfcpFloorCtrl)pCapStruct->floorctrl);
		ostr << "\n confid        " << pCapStruct->confid;
		ostr << "\n userid        " << pCapStruct->userid;

		if (pCapStruct->floorid_0.floorid[0])
		{
			ostr << "\n FloorID       " << pCapStruct->floorid_0.floorid;
			ostr << "\n Stream0       " << pCapStruct->floorid_0.m_stream_0;

			ostr << "\n FloorID       " << pCapStruct->floorid_1.floorid;
			ostr << "\n Stream1       " << pCapStruct->floorid_1.m_stream_1;

			ostr << "\n FloorID       " << pCapStruct->floorid_2.floorid;
			ostr << "\n Stream2       " << pCapStruct->floorid_2.m_stream_2;

			ostr << "\n FloorID       " << pCapStruct->floorid_3.floorid;
			ostr << "\n Stream3       " << pCapStruct->floorid_3.m_stream_3;
		}
		else
			ostr << "\n Empty FloorId Structure";

		ostr << "\n info enabled  " << (int)pCapStruct->xbfcp_info_enabled;
		ostr << "\n info time     " << (int)pCapStruct->xbfcp_info_time;
		ostr << "\n";
	}
}

//////////////////////////////////////////////////////////////////////////
// dump of H323 capabilities
void DumpH323Cap(std::ostream& ostr, WORD len,BYTE* h323CapArray)
{
	if(!len || !h323CapArray)
	{
		FPASSERT(h323CapArray == NULL);
		return;
	}
	WORD offsetRead = 0;
	capBuffer * pCapBuffer = (capBuffer *)h323CapArray;
    char* tempPtr;

	while(1)
	{
		switch(pCapBuffer->capTypeCode){
		case eG711Alaw64kCapCode :
		case eG711Ulaw64kCapCode :
		case eG711Alaw56kCapCode :
		case eG711Ulaw56kCapCode :
		case eG722_64kCapCode :
		case eG722_56kCapCode :
		case eG722_48kCapCode :
		case eG722Stereo_128kCapCode :
		case eG728CapCode : {
			simpleAudioCapStruct * audioCap = (simpleAudioCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - " ;
			if (audioCap->value != -1)
				ostr <<  audioCap->value << " milli packet duration";
			//else
			//	ostr << "invalid FPP";
			if ((pCapBuffer->sipPayloadType >= 96) && (pCapBuffer->sipPayloadType <= 127))
				ostr << "\n  RTP payload type " << (int)pCapBuffer->sipPayloadType;
			ostr << "\n";
			break;
							}
		case eG729CapCode :
		case eG729AnnexACapCode :
		case eG7221_32kCapCode:
		case eG7221_24kCapCode:
		case eG7221_16kCapCode:
		case eSiren7_16kCapCode:
		case eSiren14_24kCapCode:
		case eSiren14_32kCapCode:
		case eSiren14_48kCapCode:
		case eG7221C_48kCapCode:
		case eG7221C_32kCapCode:
		case eG7221C_24kCapCode:
		case eSiren14Stereo_48kCapCode:
		case eSiren14Stereo_56kCapCode:
		case eSiren14Stereo_64kCapCode:
		case eSiren14Stereo_96kCapCode:
		case eSiren22Stereo_128kCapCode:
		case eSiren22Stereo_96kCapCode:
		case eSiren22Stereo_64kCapCode:
		case eSiren22_64kCapCode:
		case eSiren22_48kCapCode:
		case eSiren22_32kCapCode:
		case eG719_128kCapCode:
		case eG719_96kCapCode:
		case eG719_64kCapCode:
		case eG719_48kCapCode:
		case eG719_32kCapCode:
		case eG719Stereo_128kCapCode:
		case eG719Stereo_96kCapCode:
		case eG719Stereo_64kCapCode:
		case eAAC_LDCapCode:// TIP
	    case eiLBC_13kCapCode:
	    case eiLBC_15kCapCode:
		case eOpus_CapCode:
		case eOpusStereo_CapCode:
		{
			simpleAudioCapStruct * audioCap = (simpleAudioCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - " ;
			if (audioCap->value != -1)
			{
				if(audioCap->value == 1)
					ostr <<  audioCap->value << " frame per packet";
				else
					ostr <<  audioCap->value << " frames per packet";
			}
			//else
			//	ostr << "invalid FPP";
			if ((pCapBuffer->sipPayloadType >= 96) && (pCapBuffer->sipPayloadType <= 127))
				ostr << "\n  RTP payload type " << (int)pCapBuffer->sipPayloadType;
			ostr << "\n";
			break;
							}
		case eSirenLPR_32kCapCode:
		case eSirenLPR_48kCapCode:
		case eSirenLPR_64kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPRStereo_128kCapCode:
		{
			sirenLPR_CapStruct * audioCap = (sirenLPR_CapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - " ;
			if (audioCap->maxValue != -1)
			{
				if(audioCap->maxValue == 1)
					ostr <<  audioCap->maxValue << " frame per packet";
				else
					ostr <<  audioCap->maxValue << " frames per packet";
			}
			if ((pCapBuffer->sipPayloadType >= 96) && (pCapBuffer->sipPayloadType <= 127))
				ostr << "\n  RTP payload type " << (int)pCapBuffer->sipPayloadType;
			ostr << "\n";
			break;
							}
	    case    eSirenLPR_Scalable_32kCapCode:
	    case    eSirenLPR_Scalable_48kCapCode:
	    case    eSirenLPR_Scalable_64kCapCode:
	    case    eSirenLPRStereo_Scalable_64kCapCode:
	    case    eSirenLPRStereo_Scalable_96kCapCode:
	    case    eSirenLPRStereo_Scalable_128kCapCode:
	    	ostr << "SirenLPR_Scalable" << "todo";
	    	break;

	    case eG7221C_CapCode:	{
			g7221C_CapStruct * audioCap = (g7221C_CapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - " ;
			if (audioCap->maxValue != -1)
			{
				if(audioCap->maxValue == 1)
					ostr <<  audioCap->maxValue << " frame per packet";
				else
					ostr <<  audioCap->maxValue << " frames per packet";
			}
			ostr << "\n  Supported Rates are: ";
			if(audioCap->capBoolMask & g7221C_Mask_Rate48K)
				ostr << "\n    48K ";
			if(audioCap->capBoolMask & g7221C_Mask_Rate32K)
				ostr << "32K ";
			if(audioCap->capBoolMask & g7221C_Mask_Rate24K)
				ostr << "24K";
			//else
			//	ostr << "invalid FPP";
			if ((pCapBuffer->sipPayloadType >= 96) && (pCapBuffer->sipPayloadType <= 127))
				ostr << "\n  RTP payload type " << (int)pCapBuffer->sipPayloadType;
			ostr << "\n";
			break;
							}
		case eRfc2833DtmfCapCode:{
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - ";
			if ((pCapBuffer->sipPayloadType >= 96) && (pCapBuffer->sipPayloadType <= 127))
				ostr << "\n RTP payload type " << (int)pCapBuffer->sipPayloadType;
			ostr << "\n";
			break;
								}
		case eG7231CapCode :	{
			int maxAl_sduAudioFrames = ((g7231CapStruct *)pCapBuffer->dataCap)->maxAl_sduAudioFrames;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - ";
			if (maxAl_sduAudioFrames != -1)
			{
				if(maxAl_sduAudioFrames == 1)
					ostr <<  maxAl_sduAudioFrames << " frame per packet";
				else
					ostr <<  maxAl_sduAudioFrames << " frames per packet";
			}
			//else
			//	ostr << "invalid FPP";
			if ((pCapBuffer->sipPayloadType >= 96) && (pCapBuffer->sipPayloadType <= 127))
				ostr << "\n  RTP payload type " << (int)pCapBuffer->sipPayloadType;
			ostr << "\n";
			break;
							}
		case eG729wAnnexBCapCode :				// for future use
		case eG729AnnexAwAnnexBCapCode :	 	    // for future use
		case eIS11172AudioCapCode :			    // for future use
		case eIS13818CapCode :					// for future use
		case eGenericCapCode :					// for future use
		case eG7231AnnexCapCode :{				// for future use
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << "\n";
			break;
		}

		case eH261CapCode : {
    		h261CapStruct *h261Cap = (h261CapStruct *)pCapBuffer->dataCap;
				ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
				if(h261Cap->header.direction == cmCapTransmit)
					ostr << " (Transmit)";
				ostr << " - ";
				if (h261Cap->maxBitRate != -1)
					ostr << (h261Cap->maxBitRate)*100 << " bps";
				char* contentStr = GetRoleStr(h261Cap->header.roleLabel);
				if(strcmp("",contentStr)!=0)
					ostr << "\n  " << contentStr;
				if(h261Cap->cifMPI > 0)
					ostr << "\n  CIF at " << 30/(h261Cap->cifMPI) << " fps";
				if(h261Cap->qcifMPI > 0)
					ostr << "\n  QCIF at " << 30/(h261Cap->qcifMPI) << " fps";
				if (h261Cap->capBoolMask & h261_stillImageTransmission)
					ostr << "\n  Annex D";
				ostr << "\n";
			break;
						   }
		case eH263CapCode : {
			 h263CapStruct *h263Cap = (h263CapStruct *)pCapBuffer->dataCap;
			 ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			 if(h263Cap->header.direction == cmCapTransmit)
				 ostr << " (Transmit)";
			 ostr << " - ";

			 if (h263Cap->maxBitRate != -1)
				 ostr << (h263Cap->maxBitRate)*100 << " bps";
			 char* contentStr = GetRoleStr(h263Cap->header.roleLabel);
			if(strcmp("",contentStr)!=0)
				ostr << "\n  " << contentStr;
			 if((signed char)(h263Cap->cif16MPI) > 0)
				 ostr << "\n  16CIF at " << 30/(h263Cap->cif16MPI) << " fps";
			 if((signed char)(h263Cap->cif4MPI) > 0)
				 ostr << "\n  4CIF at " << 30/(h263Cap->cif4MPI) << " fps";
			 if((signed char)(h263Cap->cifMPI) > 0)
				 ostr << "\n  CIF at " << 30/(h263Cap->cifMPI) << " fps";
			 if((signed char)(h263Cap->qcifMPI) > 0)
				 ostr << "\n  QCIF at " << 30/(h263Cap->qcifMPI) << " fps";
			 if((signed char)(h263Cap->sqcifMPI) > 0)
				 ostr << "\n  SQCIF at " << 30/(h263Cap->sqcifMPI) << " fps";

			 // dump of the annexes:
			 if (!h263Cap->annexesMask.fds_bits[0])  // No annex at all.
			 {
			 	ostr << "\n";
			 	break;
			 }

			 h263OptionsStruct	* pH263OptionsSt = (h263OptionsStruct *)&h263Cap->annexesPtr;
			 char				* pChar = (char*)pH263OptionsSt;

			 int i;
			 for (i = 0; i<H263_Annexes_Number; i++)
			 {
				 if ( CAP_FD_ISSET(i, &h263Cap->annexesMask) )
				 {
				 	ostr << "\n  ";
				 	PrintAnnexType(ostr,(annexesListEn)i);

					 //annexI_NS has no body
					 if((annexesListEn)i != typeAnnexI_NS)
					 {
						 pChar += sizeof(h263OptionsStruct);
						 pH263OptionsSt = (h263OptionsStruct *)pChar;
					 }
				 }
			 }

			 // dump of the custom picture format
			 for (;i < H263_Annexes_Number + H263_Custom_Number; i++)
			 {
				 if( CAP_FD_ISSET(i, &h263Cap->annexesMask))
				 {
					 customPic_St * pH263CustomSt = (customPic_St *)pChar;
					 unsigned int j;
					 for ( j = 0; j < pH263CustomSt->numberOfCustomPic; j++)
					 {
					 	ostr << "\n  ";
						PrintH323CustomOF(ostr,pH263CustomSt,j);
					 }
					 break;
				 }
			 }
			 ostr << "\n";

			 break;
						   }

		case eH264CapCode :
		{
			h264CapStruct *h264Cap = (h264CapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			if(h264Cap->header.direction == cmCapTransmit)
				ostr << " (Transmit)";
			ostr << " - ";

			ostr << (h264Cap->maxBitRate)*100 << " bps";
			char* contentStr = GetRoleStr(h264Cap->header.roleLabel);
			if(strcmp("",contentStr)!=0)
				ostr << "\n  " << contentStr;

			ostr << "\n  "<< (h264Cap->maxFR) << " frame rate per second";

			if (h264Cap->H264mode == H264_standard)
				ostr << "\n  "<< "264Mode: standard";
			else // 2..
				ostr << "\n  "<< "264Mode: tipContent";

			ostr << "\n  " << GetH264ProfileAsString(h264Cap->profileValue)
				 << "\n  " << GetH264LevelAsString(h264Cap->levelValue);

			if (h264Cap->customMaxMbpsValue != -1)
				ostr << "\n  CustomMaxMBPS at " << h264Cap->customMaxMbpsValue << " (" << h264Cap->customMaxMbpsValue*CUSTOM_MAX_MBPS_FACTOR << " MB/s)";

			if (h264Cap->customMaxFsValue != -1)
				ostr << "\n  CustomMaxFS at " << h264Cap->customMaxFsValue << " (" << h264Cap->customMaxFsValue*CUSTOM_MAX_FS_FACTOR << " MBs)";

			if (h264Cap->customMaxDpbValue	 != -1)
				ostr << "\n  CustomMaxDPB at " << h264Cap->customMaxDpbValue << " (" << h264Cap->customMaxDpbValue*CUSTOM_MAX_DPB_FACTOR << " Bytes)";

			if (h264Cap->customMaxBrAndCpbValue != -1)
				ostr << "\n  customMaxBrAndCpbValue at " << h264Cap->customMaxBrAndCpbValue <<" (" << h264Cap->customMaxBrAndCpbValue*CUSTOM_MAX_BR_FACTOR << " bps)";

			if ((h264Cap->maxStaticMbpsValue != -1) && (h264Cap->maxStaticMbpsValue != 0))
				ostr << "\n  maxStaticMbpsValue at " << h264Cap->maxStaticMbpsValue <<" (" << h264Cap->maxStaticMbpsValue*CUSTOM_MAX_MBPS_FACTOR<< "  MB/s)";

			if (h264Cap->sampleAspectRatiosValue != -1)
				ostr << "\n  sampleAspectRatiosValue at " << h264Cap->sampleAspectRatiosValue <<" (" << h264Cap->sampleAspectRatiosValue << " H/V)";

			ostr << "\n";
			break;
		}

		case eRtvCapCode :
		{
			rtvCapStruct *RtvCap = (rtvCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			if(RtvCap->header.direction == cmCapTransmit)
				ostr << " (Transmit)";
			ostr << " - ";

			WORD numOfItems = RtvCap->numOfItems;
			ostr << "numOfItems  = " << numOfItems;

			for(int i =0;i < numOfItems;i++)
			{
				if((RtvCap->rtvCapItem[i]).capabilityID != 0)
				{
					ostr << "\nRtvItem                     = " << i;
					ostr << "\ncapabilityID                = " << (RtvCap->rtvCapItem[i]).capabilityID;
					ostr << "\nwidthVF                     = " << (RtvCap->rtvCapItem[i]).widthVF;
					ostr << "\nheightVF                    = " << (RtvCap->rtvCapItem[i]).heightVF;
					ostr << "\nfps                         = " << (RtvCap->rtvCapItem[i]).fps;
					ostr << "\nmaxBitrateInBps             = " << (RtvCap->rtvCapItem[i]).maxBitrateInBps <<
					" (" << ((RtvCap->rtvCapItem[i]).maxBitrateInBps)*100 << " bps)";
				}
			}
			ostr << "\n";
			break;
		}
		case eVP8CapCode :
		{
			vp8CapStruct *vp8Cap = (vp8CapStruct *)pCapBuffer->dataCap;
			//ostr << ::GetCapCodeNameStringOnly(pCapBuffer->capTypeCode); // N.A TO Fix
			if(vp8Cap->header.direction == cmCapTransmit)
				ostr << " (Transmit)";
			ostr << " - ";

			ostr << (vp8Cap->maxBitRate)*100 << " bps";
			char* contentStr = GetRoleStr(vp8Cap->header.roleLabel);
			if(strcmp("",contentStr)!=0)
				ostr << "\n  " << contentStr;

			ostr << "\n  "<< (vp8Cap->maxFR) << " frame rate per second";
			ostr << "\n  "<< (vp8Cap->maxFS) << " frame size";


			ostr << "\n";
			break;
		}
		case eH26LCapCode :
		case eGenericVideoCapCode:{
			genericVideoCapStruct *pGenericVideoCap = (genericVideoCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			if(pGenericVideoCap->header.direction == cmCapTransmit)
				ostr << " (Transmit)";
			ostr << " - ";
			ostr << (pGenericVideoCap->maxBitRate)*100 << " bps";
			if(pGenericVideoCap->genericCodeType == eH26LCode)
			{
				BYTE mpi = pGenericVideoCap->data[H26L_MPI_ONDATA_LOCATION];
				if(mpi >> 4)
					ostr << "\n  CIF at " << 30/(mpi >> 4) << " fps";
				if(mpi & 0xf)
					ostr << "\n  4CIF at " << 30/(mpi & 0xf) << " fps";
			}
			else
				ostr << ", Generic Video Algorithm";

			char* contentStr = GetRoleStr(pGenericVideoCap->header.roleLabel);
			if(strcmp("",contentStr)!=0)
				ostr << "\n  " << contentStr;

			ostr << "\n";
			break;
						   }
		case eH262CapCode :			// for future use
		case eIS11172VideoCapCode :  // for future use
		case eT120DataCapCode :
//CARMEL	case h224DataCapCode :
		{
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << "\n";
			break;
		}

		case eAnnexQCapCode:
		case eRvFeccCapCode:{
			dataCapStructBase *pDataCap = (dataCapStructBase *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - ";
			ostr << (pDataCap->maxBitRate)*100 << " bps" << "\n";
			break;
		}

        case ePeopleContentCapCode :
		case eRoleLabelCapCode :{
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			ostr << " - " << ((simpleAudioCapStruct *)pCapBuffer->dataCap)->value << "\n";
			break;
		}

		case eH239ControlCapCode:{
 			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			ostr << "\n";
			break;
		}
		case eDynamicPTRCapCode:{
 			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			ostr << "\n";
			break;
		}
		case eLPRCapCode:{
			lprCapStruct *pDataCap = (lprCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " -\n";
			ostr << "  versionID           " << pDataCap->versionID << "\n";
			ostr << "  minProtectionPeriod " << pDataCap->minProtectionPeriod << "\n";
			ostr << "  maxProtectionPeriod " << pDataCap->maxProtectionPeriod << "\n";
			ostr << "  maxRecoverySet      " << pDataCap->maxRecoverySet << "\n";
			ostr << "  maxRecoveryPackets  " << pDataCap->maxRecoveryPackets << "\n";
			ostr << "  maxPacketSize       " << pDataCap->maxPacketSize << "\n";
			break;
		}

		case eNonStandardCapCode : {
			//CARMEL
//			const NS_CAP_H323_VISUAL_CONCERT_PC	= 0x5A;
//			const NS_CAP_H323_VISUAL_CONCERT_FX	= 0x72;
//			const NS_CAP_H323_H263_QCIF_ANNEX_I	= 0x40;
//			const NS_CAP_H323_H263_CIF_ANNEX_I	= 0x41;
//			const NS_CAP_H323_H263_4CIF_ANNEX_I	= 0x42;
//			const NS_CAP_H323_H263_QCIF_ANNEX_T	= 0x44;
//			const NS_CAP_H323_H263_CIF_ANNEX_T	= 0x45;
//			const NS_CAP_H323_H263_4CIF_ANNEX_T	= 0x46;
//			const NS_CAP_ACCORD_SENDER			= 0x6C;
//			const NS_CAP_H323_HIGH_CAPACITY		= 0x54;
//			const NS_CAP_H323_VGA_800X600		= 0x61;
//			const NS_CAP_H323_VGA_1024X768		= 0x63;
//			const NS_CAP_H323_VGA_1280X1024		= 0x67;
//			const NS_CAP_H323_VIDEO_STREAMS_2	= 0x73;
			//CARMEL

			ctNonStandardCapStruct	*pNsCap = (ctNonStandardCapStruct*)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode) << " - ";

			ctNonStandardIdentifierSt	*pInfo = &(pNsCap->nonStandardData.info);

			if( !(pInfo->t35CountryCode == NS_T35COUNTRY_CODE_USA &&
				pInfo->t35Extension == NS_T35EXTENSION_USA) )
				// not USA
				ostr << "??";
			else if( !(pInfo->manufacturerCode == NS_MANUFACTURER_POLYCOM) )
				// not Polycom
				ostr << "??";
			else {
				for( int i=0; i<CT_NonStandard_Data_Size; i++ ) {
					switch( pNsCap->nonStandardData.data[i])
					{
						case NS_CAP_H323_VISUAL_CONCERT_PC : ostr << "(NonSt)VisualConcertPC" << "\n"; break;
						case NS_CAP_H323_VISUAL_CONCERT_FX : ostr << "(NonSt)VisualConcertFX" << "\n"; break;
						case NS_CAP_H323_H263_QCIF_ANNEX_I : ostr << "(NonSt)H263_QCif_AnnexI" << "\n"; break;
						case NS_CAP_H323_H263_CIF_ANNEX_I :  ostr << "(NonSt)H263_Cif_AnnexI" << "\n"; break;
						case NS_CAP_H323_H263_4CIF_ANNEX_I : ostr << "(NonSt)H263_4Cif_AnnexI" << "\n"; break;
						case NS_CAP_H323_H263_QCIF_ANNEX_T : ostr << "(NonSt)H263_QCif_AnnexT" << "\n"; break;
						case NS_CAP_H323_H263_CIF_ANNEX_T :  ostr << "(NonSt)H263_Cif_AnnexT" << "\n"; break;
						case NS_CAP_H323_H263_4CIF_ANNEX_T : ostr << "(NonSt)H263_4Cif_AnnexT" << "\n"; break;
						case NS_CAP_ACCORD_SENDER :          ostr << "(NonSt)Polycom' MCU sender" << "\n"; break;
						case NS_CAP_H323_HIGH_CAPACITY :     ostr << "(NonSt)High Video Capacity" << "\n"; break;
						case NS_CAP_H323_VGA_800X600 :       ostr << "(NonSt)VGA 800x600 Capacity" << "\n"; break;
						case NS_CAP_H323_VGA_1024X768 :      ostr << "(NonSt)VGA 1024x768 Capacity" << "\n"; break;
						case NS_CAP_H323_VGA_1280X1024 :     ostr << "(NonSt)VGA 1280x1024 Capacity" << "\n"; break;
						case NS_CAP_H323_VIDEO_STREAMS_2 :   ostr << "(NonSt)VideoStreams2 Capacity" << "\n"; break;
						default :
							break;
					}
				}
			}
			break;
		}
		case eEncryptionCapCode:{
			encryptionCapStruct *pEncrCap = (encryptionCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			ostr << " entry = "<<pEncrCap->entry << " \n";
			break;
		}

		case eSdesCapCode:
		{
			sdesCapStruct *pSdesCap = (sdesCapStruct *)pCapBuffer->dataCap;
			ostr << " SRTP: ";
			if(pSdesCap) {
				switch(pSdesCap->cryptoSuite)
				{
				case eAes_Cm_128_Hmac_Sha1_80:
					ostr << "AES_CM_128_HMAC_SHA1_80 \n ";
					break;
				case eAes_Cm_128_Hmac_Sha1_32:
					ostr << "AES_CM_128_HMAC_SHA1_32 \n ";
					break;
				case eF8_Cm_128_Hmac_Sha1_80:
					ostr << "F8_128_HMAC_SHA1_80 \n ";
					break;
				default:
					ostr << " Unknown crypto suite \n ";
				}
			}

			break;
		}
		case eBFCPCapCode:{
			bfcpCapStruct *pDataCap = (bfcpCapStruct *)pCapBuffer->dataCap;
			ostr << CapEnumToString((CapEnum)pCapBuffer->capTypeCode);
			//Anna
			PrintBfcpData(ostr, pDataCap);

			break;
		}
		case eUnknownAlgorithemCapCode :{
			break;
									   }
		default: {
			PrintCapDataType(ostr, (ctCapStruct *)pCapBuffer->dataCap);
		break;
			}
		}

		offsetRead += sizeof(capBufferBase) + pCapBuffer->capLength;
		if(offsetRead >= len)
			break;

		tempPtr = (char*)pCapBuffer;
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}



