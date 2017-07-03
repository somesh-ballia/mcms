// CCH323StrCap.cpp: implementation of the CH323StrCap class.
//
//////////////////////////////////////////////////////////////////////
#include "H323StrCap.h"
#include "ConfPartyH323Defines.h"
#include "Macros.h"
#include "CapClass.h"
#include "Segment.h"

extern void DumpH323Cap(std::ostream& ostr, WORD len,BYTE* h323CapArray);


// CLASS CH323StrCap
/////////////////////////////////////////////////////////////////////////////
//cap dump of H323 BAS capabilits vector
void  CH323StrCap::Dump(std::ostream &m_ostr)
{
  if(m_size != 0)
	DumpH323Cap(m_ostr, m_size, m_pStr);
}

/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::ConverCustomFormat(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize)
{	
	
	customPic_St      *pCustomFormats = (customPic_St*)pStruct;

	int err = 0;
	BYTE	*pCustom522	= new BYTE[sizeof(customPic_StBaseApi522)];
	customPic_StBaseApi522 *pCustomFormat522 = (customPic_StBaseApi522*)pCustom522;

	pCustomFormat522->customPictureClockFrequency.clockConversionCode	= pCustomFormats->customPictureClockFrequency.clockConversionCode;
	pCustomFormat522->customPictureClockFrequency.clockDivisor			= pCustomFormats->customPictureClockFrequency.clockDivisor;
	pCustomFormat522->customPictureClockFrequency.sqcifMPI				= pCustomFormats->customPictureClockFrequency.sqcifMPI;
	pCustomFormat522->customPictureClockFrequency.qcifMPI				= pCustomFormats->customPictureClockFrequency.qcifMPI;
	pCustomFormat522->customPictureClockFrequency.cifMPI				= pCustomFormats->customPictureClockFrequency.cifMPI;
	pCustomFormat522->customPictureClockFrequency.cif4MPI				= pCustomFormats->customPictureClockFrequency.cif4MPI;
	pCustomFormat522->customPictureClockFrequency.cif16MPI				= pCustomFormats->customPictureClockFrequency.cif16MPI;

	pCustomFormat522->numberOfCustomPic	= pCustomFormats->numberOfCustomPic;

	int size = sizeof(customPic_StBaseApi522);
	newSize += size;

	err = CopyStructToOldformArray(0, size, tempArr, pCustom522, IndexArr);
	
	PDELETEA(pCustom522);
	if(err == -1)
		return -1;

	int	numberOfCustomPic = pCustomFormats->numberOfCustomPic;

	customPicFormatSt	*pCurrentCustomFormat = (customPicFormatSt *)pCustomFormats->customPicPtr;
	char				*pChar = (char *)pCustomFormats->customPicPtr;

	for(int j=0; j < numberOfCustomPic; j++)
	{		
		if(pCurrentCustomFormat)
		{
			BYTE	*pCurrCust522	= new BYTE[sizeof(customPicFormatStApi522)];
			customPicFormatStApi522 *pCurrentCustomFormat522 = (customPicFormatStApi522*)pCurrCust522;

			pCurrentCustomFormat522->maxCustomPictureWidth	= pCurrentCustomFormat->maxCustomPictureWidth;
			pCurrentCustomFormat522->maxCustomPictureHeight = pCurrentCustomFormat->maxCustomPictureHeight;
			pCurrentCustomFormat522->minCustomPictureWidth	= pCurrentCustomFormat->minCustomPictureWidth;
			pCurrentCustomFormat522->minCustomPictureHeight = pCurrentCustomFormat->minCustomPictureHeight;

			pCurrentCustomFormat522->mPI.standardMPI					= pCurrentCustomFormat->standardMPI;
			pCurrentCustomFormat522->mPI.customPCF.clockConversionCode	= pCurrentCustomFormat->clockConversionCode;
			pCurrentCustomFormat522->mPI.customPCF.clockDivisor			= pCurrentCustomFormat->clockDivisor;
			pCurrentCustomFormat522->mPI.customPCF.customMPI			= pCurrentCustomFormat->customMPI;

			pCurrentCustomFormat522->pixelAspectInformation.pixelAspectCode[0] = pCurrentCustomFormat->pixelAspectCode[0];
		
			size = sizeof(customPicFormatStApi522);
			newSize += size;
			
			err = CopyStructToOldformArray(0, size, tempArr, pCurrCust522, IndexArr);
			
			PDELETEA(pCurrCust522);
			if(err == -1)
				return -1;
		}
					
		pChar += sizeof(customPicFormatSt);
		pCurrentCustomFormat = (customPicFormatSt *)pChar;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::ConvertAnnex(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize, BYTE **ppAnnexPtr)
{	
	h263CapStruct	*pCapStruct = (h263CapStruct*)pStruct;
	BYTE			*pAnnexPtr	= (BYTE *)pCapStruct->annexesPtr;
	*ppAnnexPtr					= pAnnexPtr;
	int				size		= 0;
	int				err			= 0;
	
	annexesListEn annex;
	for (annex = typeAnnexB; (annex < H263_Annexes_Number) && (err == 0); annex++)
	{
		if (CAP_FD_ISSET(annex, &(pCapStruct->annexesMask))) 
		{
			size = sizeof(h263OptionsStructApi522);
			newSize += size;
			
			switch(annex)
			{
			case typeAnnexB:
			case typeAnnexE:
			case typeAnnexF:
			case typeAnnexG:
			case typeAnnexH:
			case typeAnnexO:
			case typeAnnexV:
			case typeAnnexW:
				{
					annexBEFGHO_St			*pCapStruct			= ((annexBEFGHO_St*)pAnnexPtr);
					BYTE					*pAnnexStruct		= new BYTE[size];
					annexBEFGHO_StApi522	*pAnnexStructApi522	= (annexBEFGHO_StApi522 *)pAnnexStruct;
					
					pAnnexStructApi522->dummy = pCapStruct->dummy;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexStruct, IndexArr);
					
					PDELETEA(pAnnexStruct);

					break;
				}
			case typeAnnexD:
				{
					annexD_St		*pCapStruct				= ((annexD_St*)pAnnexPtr);
					BYTE			*pAnnexDStruct			= new BYTE[size];
					annexD_StApi522	*pAnnexDStructApi522	= (annexD_StApi522 *)pAnnexDStruct;
					
					pAnnexDStructApi522->unlimitedMotionVectors = (pCapStruct->annexBoolMask & annexD_unlimitedMotionVectors ? 1:0);

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexDStruct, IndexArr);
					
					PDELETEA(pAnnexDStruct);

					break;
				}
				
			case typeAnnexI:
				{
					annexI_St		*pCapStruct				= ((annexI_St*)pAnnexPtr);
					BYTE			*pAnnexIStruct			= new BYTE[size];
					annexI_StApi522	*pAnnexIStructApi522	= (annexI_StApi522 *)pAnnexIStruct;
					
					pAnnexIStructApi522->advancedIntraCodingMode = (pCapStruct->annexBoolMask & annexI_advancedIntraCodingMode ? 1:0);								

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexIStruct, IndexArr);
					
					PDELETEA(pAnnexIStruct);
					
					break;
				}
			case typeAnnexJ:
				{
					annexJ_St		*pCapStruct				= ((annexJ_St*)pAnnexPtr);
					BYTE			*pAnnexJStruct			= new BYTE[size];
					annexJ_StApi522	*pAnnexJStructApi522	= (annexJ_StApi522 *)pAnnexJStruct;
					
					pAnnexJStructApi522->deblockingFilterMode = (pCapStruct->annexBoolMask & annexJ_deblockingFilterMode ? 1:0);

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexJStruct, IndexArr);
					
					PDELETEA(pAnnexJStruct);
					break;
				}
			case typeAnnexK:
				{
					annexK_St		*pCapStruct				= ((annexK_St*)pAnnexPtr);
					BYTE			*pAnnexKStruct			= new BYTE[size];
					annexK_StApi522	*pAnnexKStructApi522	= (annexK_StApi522 *)pAnnexKStruct;
					
					pAnnexKStructApi522->slicesInOrder_NonRect	= (pCapStruct->annexBoolMask & annexK_slicesInOrder_NonRect ? 1:0);
					pAnnexKStructApi522->slicesInOrder_Rect		= (pCapStruct->annexBoolMask & annexK_slicesInOrder_Rect ? 1:0);
					pAnnexKStructApi522->slicesNoOrder_NonRect	= (pCapStruct->annexBoolMask & annexK_slicesNoOrder_NonRect ? 1:0);
					pAnnexKStructApi522->slicesNoOrder_Rect		= (pCapStruct->annexBoolMask & annexK_slicesNoOrder_Rect ? 1:0);
										
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexKStruct, IndexArr);
					
					PDELETEA(pAnnexKStruct);
					
					break;
				}
			case typeAnnexL:
				{

					annexL_St		*pCapStruct				= ((annexL_St*)pAnnexPtr);
					BYTE			*pAnnexLStruct			= new BYTE[size];
					annexL_StApi522	*pAnnexLStructApi522	= (annexL_StApi522 *)pAnnexLStruct;
										
					pAnnexLStructApi522->fullPictureFreeze					= (pCapStruct->annexBoolMask & annexL_fullPictureFreeze ? 1:0);
					pAnnexLStructApi522->partialPictureFreezeAndRelease		= (pCapStruct->annexBoolMask & annexL_partialPictureFreezeAndRelease ? 1:0);
					pAnnexLStructApi522->resizingPartPicFreezeAndRelease	= (pCapStruct->annexBoolMask & annexL_resizingPartPicFreezeAndRelease ? 1:0);
					pAnnexLStructApi522->fullPictureSnapshot				= (pCapStruct->annexBoolMask & annexL_fullPictureSnapshot ? 1:0);
					pAnnexLStructApi522->partialPictureSnapshot				= (pCapStruct->annexBoolMask & annexL_partialPictureSnapshot ? 1:0);
					pAnnexLStructApi522->videoSegmentTagging				= (pCapStruct->annexBoolMask & annexL_videoSegmentTagging ? 1:0);
					pAnnexLStructApi522->progressiveRefinement				= (pCapStruct->annexBoolMask & annexL_progressiveRefinement ? 1:0);
					
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexLStruct, IndexArr);
					
					PDELETEA(pAnnexLStruct);
					
					break;
				}
			case typeAnnexM:
				{
					annexM_St		*pCapStruct				= ((annexM_St*)pAnnexPtr);
					BYTE			*pAnnexMStruct			= new BYTE[size];
					annexM_StApi522	*pAnnexMStructApi522	= (annexM_StApi522 *)pAnnexMStruct;

					pAnnexMStructApi522->improvedPBFramesMode = (pCapStruct->annexBoolMask & annexM_improvedPBFramesMode ? 1:0);
					
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexMStruct, IndexArr);
					
					PDELETEA(pAnnexMStruct);

					break;
				}
			case typeAnnexN:
			case typeAnnexU:
				{
					annexN_St		*pCapStruct				= ((annexN_St*)pAnnexPtr);
					BYTE			*pAnnexNStruct			= new BYTE[size];
					
					refPictureSelectionStructApi522		*pRefPicApi522 = &((annexN_StApi522*)pAnnexNStruct)->refPictureSelection;
					additionalPictureMemoryStructApi522	*pAddPicApi522 = &pRefPicApi522->additionalPictureMemory;
					
					refPictureSelectionStruct			*pRefPic = &pCapStruct->refPictureSelection;
					additionalPictureMemoryStruct		*pAddPic = &pRefPic->additionalPictureMemory;
					
					pAddPicApi522->sqcifAdditionalPictureMemory = pAddPic->sqcifAdditionalPictureMemory;
					pAddPicApi522->qcifAdditionalPictureMemory	= pAddPic->qcifAdditionalPictureMemory;
					pAddPicApi522->cifAdditionalPictureMemory	= pAddPic->cifAdditionalPictureMemory;
					pAddPicApi522->cif4AdditionalPictureMemory	= pAddPic->cif4AdditionalPictureMemory;
					pAddPicApi522->cif16AdditionalPictureMemory	= pAddPic->cif16AdditionalPictureMemory;
					pAddPicApi522->bigCpfAdditionalPictureMemory= pAddPic->bigCpfAdditionalPictureMemory;
					
					pRefPicApi522->videoMux				= (pRefPic->annexBoolMask & refPic_videoMux ? 1:0);
					pRefPicApi522->videoBackChannelSend	= pRefPic->videoBackChannelSend;
					pRefPicApi522->mpuHorizMBs			= pRefPic->mpuHorizMBs;
					pRefPicApi522->mpuVertMBs			= pRefPic->mpuVertMBs;
										
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexNStruct, IndexArr);
					
					PDELETEA(pAnnexNStruct);									
					
					break;
				}
			case typeAnnexP:
				{
					annexP_St		*pCapStruct				= ((annexP_St*)pAnnexPtr);
					BYTE			*pAnnexPStruct			= new BYTE[size];
					annexP_StApi522	*pAnnexPStructApi522	= (annexP_StApi522 *)pAnnexPStruct;

					
					pAnnexPStructApi522->dynamicPictureResizingByFour		= (pCapStruct->annexBoolMask & annexP_dynamicPictureResizingByFour ? 1:0);
					pAnnexPStructApi522->dynamicPictureResizingSixteenthPel = (pCapStruct->annexBoolMask & annexP_dynamicPictureResizingSixteenthPel ? 1:0);
					pAnnexPStructApi522->dynamicWarpingHalfPel				= (pCapStruct->annexBoolMask & annexP_dynamicWarpingHalfPel ? 1:0);
					pAnnexPStructApi522->dynamicWarpingSixteenthPel			= (pCapStruct->annexBoolMask & annexP_dynamicWarpingSixteenthPel ? 1:0);
										
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexPStruct, IndexArr);
					
					PDELETEA(pAnnexPStruct);									
					
					break;
				}
			case typeAnnexQ:
				{
					annexQ_St		*pCapStruct				= ((annexQ_St*)pAnnexPtr);
					BYTE			*pAnnexQStruct			= new BYTE[size];
					annexQ_StApi522	*pAnnexQStructApi522	= (annexQ_StApi522 *)pAnnexQStruct;

					pAnnexQStructApi522->reducedResolutionUpdate = (pCapStruct->annexBoolMask & annexQ_reducedResolutionUpdate ? 1:0);

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexQStruct, IndexArr);
					
					PDELETEA(pAnnexQStruct);									

					break;
				}
			case typeAnnexR:
				{
					annexR_St		*pCapStruct				= ((annexR_St*)pAnnexPtr);
					BYTE			*pAnnexRStruct			= new BYTE[size];
					annexR_StApi522	*pAnnexRStructApi522	= (annexR_StApi522 *)pAnnexRStruct;

					pAnnexRStructApi522->independentSegmentDecoding = (pCapStruct->annexBoolMask & annexR_independentSegmentDecoding ? 1:0);

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexRStruct, IndexArr);
					
					PDELETEA(pAnnexRStruct);									

					break;
				}
			case typeAnnexS:
				{
					annexS_St		*pCapStruct				= ((annexS_St*)pAnnexPtr);
					BYTE			*pAnnexSStruct			= new BYTE[size];
					annexS_StApi522	*pAnnexSStructApi522	= (annexS_StApi522 *)pAnnexSStruct;

					pAnnexSStructApi522->alternateInterVLCMode = (pCapStruct->annexBoolMask & annexS_alternateInterVLCMode ? 1:0);

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexSStruct, IndexArr);
					
					PDELETEA(pAnnexSStruct);									

					break;
				}
			case typeAnnexT:
				{
					annexT_St		*pCapStruct				= ((annexT_St*)pAnnexPtr);
					BYTE			*pAnnexTStruct			= new BYTE[size];
					annexT_StApi522	*pAnnexTStructApi522	= (annexT_StApi522 *)pAnnexTStruct;

					pAnnexTStructApi522->modifiedQuantizationMode = (pCapStruct->annexBoolMask & annexT_modifiedQuantizationMode ? 1:0);

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexTStruct, IndexArr);
					
					PDELETEA(pAnnexTStruct);									

					break;
				}
			default:
				break;
			}//switch

			pAnnexPtr += sizeof(h263OptionsStruct);
		} //if								
	}//for

	return err;
}
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::GetFullSizeOf263Struct(char *pBuffer)
{
	int size						= sizeof(h263CapStructBaseApi522);
	capBuffer		*pCapBuffer		= (capBuffer *) pBuffer;
	h263CapStruct	*pCapStruct		= ((h263CapStruct*)(pCapBuffer->dataCap));
	BYTE			*pAnnexPtr		= (BYTE *)pCapStruct->annexesPtr;

	if(pCapStruct->annexesMask.fds_bits[0])
	{
		for (annexesListEn annex = typeAnnexB; annex < H263_Annexes_Number; annex++)
		{
			if (CAP_FD_ISSET(annex, &(pCapStruct->annexesMask))) 		
			{
				size += sizeof(h263OptionsStructApi522);
				pAnnexPtr += sizeof(h263OptionsStruct);
			}		
		} //end for

		if(pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
		{
			customPic_St     *pCustomFormats = (customPic_St*)pAnnexPtr;

			size +=	sizeof(customPic_StBaseApi522);
			size +=	pCustomFormats->numberOfCustomPic * sizeof(customPicFormatStApi522);
		}
	}
	
	return size;
}
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::ConvertCapBuffAndHeader(char *pBuffer,WORD *tempArr,int &IndexArr,int size)
{

	BYTE				*pBufBase		= new BYTE[sizeof(capBufferBaseApi522)];	
	capBufferBaseApi522 *pBufBaseApi522 = (capBufferBaseApi522 *)pBufBase;
	capBuffer			*pCapBuffer		= (capBuffer *) pBuffer;
	int					err = 0;
	
	pBufBaseApi522->capLength	= size;
	pBufBaseApi522->capTypeCode	= pCapBuffer->capTypeCode;
	
	size = sizeof(capBufferBaseApi522);
	err = CopyStructToOldformArray(0, size, tempArr, pBufBase, IndexArr);
	
	PDELETEA(pBufBase);
	if(err == -1)
		return err;
	
	ctCapStruct	*pHeaderStruct	= ((ctCapStruct*)pCapBuffer->dataCap);
	BYTE		*pHeaderApi522	= new BYTE[sizeof(capStructHeader)];
	capStructHeader *pHeader522 = (capStructHeader *)pHeaderApi522;
		
	pHeader522->type		= pHeaderStruct->type;
	pHeader522->direction	= pHeaderStruct->direction;
	pHeader522->roleLabel	= pHeaderStruct->roleLabel;
	
	size = sizeof(capStructHeader);
	err = CopyStructToOldformArray(0, size, tempArr, pHeaderApi522, IndexArr);
	
	PDELETEA(pHeaderApi522);
	
	return err;
}
/////////////////////////////////////////////////////////////////////////////
void CH323StrCap::Serialize(WORD format, std::ostream &m_ostr)
{
	// assuming format = OPERATOR_MCMS
	
	int i/*,err*/ = 0;
/*//CARMEL
	if(apiNum <= 522)
	{
		if(m_size > 0)
		{
			int			newSize		= 0;
			int			size		= 0;
			int			oldSize		= m_size;
			int			sizeHeader	= sizeof(capStructHeader);
			int			noCapsCount = 0;

			WORD		*tempArr	= new WORD[SIZE_STREAM];
			int			IndexArr	= 0;
			
			capBuffer	*pCapBuffer = (capBuffer *) m_pStr;
			char		*pTempPtr	= (char*)pCapBuffer;

			while((oldSize > 0) && (err == 0))
			{
				noCapsCount++;
				switch(pCapBuffer->capTypeCode)
				{
				case g711Alaw64kCapCode : 
				case g711Ulaw64kCapCode : 
				case g711Alaw56kCapCode : 
				case g711Ulaw56kCapCode : 
				case g722_64kCapCode : 
				case g722_56kCapCode : 
				case g722_48kCapCode : 
				case g728CapCode : 
				case g729CapCode :
				case g729AnnexACapCode :
				case g729wAnnexBCapCode :				
				case g729AnnexAwAnnexBCapCode :	 	    
				case g7221_32kCapCode:
				case g7221_24kCapCode:	
				case siren14_24kCapCode:
				case siren14_32kCapCode:
				case siren14_48kCapCode:
				case PeopleContentCapCode : 
				case RoleLabelCapCode :
					{
						size	= sizeof(simpleAudioCapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;

						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);		
						
						simpleAudioCapStruct		*pCapStruct			= ((simpleAudioCapStruct*)(pCapBuffer->dataCap));
						BYTE						*pAudStruct			= new BYTE[size];
						simpleAudioCapStructApi522	*pAudStructApi522	= (simpleAudioCapStructApi522 *)pAudStruct;
						
						pAudStructApi522->value	= pCapStruct->value;

						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, pAudStruct, IndexArr);
						
						PDELETEA(pAudStruct);

						break;
					}   
				case g7231CapCode :	
					{
						size	= sizeof(g7231CapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;

						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);	
						
						g7231CapStruct		*pCapStruct			= ((g7231CapStruct*)(pCapBuffer->dataCap));
						BYTE				*p7231Struct		= new BYTE[size];
						g7231CapStructApi522*p7231StructApi522	= (g7231CapStructApi522 *)p7231Struct;

						p7231StructApi522->maxAl_sduAudioFrames = pCapStruct->maxAl_sduAudioFrames;;
						p7231StructApi522->silenceSuppression	= (pCapStruct->capBoolMask & g7231_silenceSuppression ? 1:0);
					
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, p7231Struct, IndexArr);
						
						PDELETEA(p7231Struct);
						
						break;
					}   			
				case IS11172AudioCapCode :			    // for future use
				case IS13818CapCode :					// for future use
				case G7231AnnexCapCode :				// for future use
					{
						size								= sizeof(capBufferBaseApi522);
						BYTE				*pBufBase		= new BYTE[size];	
						capBufferBaseApi522 *pBufBaseApi522 = (capBufferBaseApi522 *)pBufBase;
																
						pBufBaseApi522->capLength	= size;
						pBufBaseApi522->capTypeCode	= pCapBuffer->capTypeCode;
											
						if(err == 0)
							err = CopyStructToOldformArray(0, size, tempArr, pBufBase, IndexArr);
						
						PDELETEA(pBufBase);
						
						break;
					}
					
				case h261CapCode : 
					{
						size	= sizeof(h261CapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;

						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);					
						
						h261CapStruct		*pCapStruct			= ((h261CapStruct*)(pCapBuffer->dataCap));
						BYTE				*p261Struct			= new BYTE[size];
						h261CapStructApi522	*p261StructApi522	= (h261CapStructApi522 *)p261Struct;
						
						p261StructApi522->qcifMPI		= pCapStruct->qcifMPI;
						p261StructApi522->cifMPI		= pCapStruct->cifMPI;
						p261StructApi522->maxBitRate	= pCapStruct->maxBitRate;
						
						p261StructApi522->temporalSpatialTradeOffCapability	= (pCapStruct->capBoolMask & h261_temporalSpatialTradeOffCapability ? 1:0);
						p261StructApi522->stillImageTransmission			= (pCapStruct->capBoolMask & h261_stillImageTransmission ? 1:0);
						
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, p261Struct, IndexArr);
						
						PDELETEA(p261Struct);
						
						break;
					}							    
				case h263CapCode : 
					{
						
						size = sizeof(h263CapStructBaseApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;
						
						int fullSize	= GetFullSizeOf263Struct((char *)pCapBuffer);
						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,fullSize);										
						
						h263CapStruct		*pCapStruct		= ((h263CapStruct*)(pCapBuffer->dataCap));
						BYTE				*p263Struct		= new BYTE[size];
						h263CapStructApi522 *p263StructApi522 = (h263CapStructApi522 *)p263Struct;
						
						p263StructApi522->maxBitRate	= pCapStruct->maxBitRate;
						p263StructApi522->hrd_B			= pCapStruct->hrd_B;
						p263StructApi522->bppMaxKb		= pCapStruct->bppMaxKb;
						p263StructApi522->slowSqcifMPI	= pCapStruct->slowSqcifMPI;
						p263StructApi522->slowQcifMPI	= pCapStruct->slowQcifMPI;
						p263StructApi522->slowCifMPI	= pCapStruct->slowCifMPI;
						p263StructApi522->slowCif4MPI	= pCapStruct->slowCif4MPI;
						p263StructApi522->slowCif16MPI	= pCapStruct->slowCif16MPI;
						p263StructApi522->sqcifMPI		= pCapStruct->sqcifMPI;
						p263StructApi522->qcifMPI		= pCapStruct->qcifMPI;
						p263StructApi522->cifMPI		= pCapStruct->cifMPI;
						p263StructApi522->cif4MPI		= pCapStruct->cif4MPI;
						p263StructApi522->cif16MPI		= pCapStruct->cif16MPI;
						p263StructApi522->annexesMask	= pCapStruct->annexesMask;					
						
						p263StructApi522->unrestrictedVector	= (pCapStruct->capBoolMask & h263_unrestrictedVector ? 1:0);
						p263StructApi522->arithmeticCoding		= (pCapStruct->capBoolMask & h263_arithmeticCoding ? 1:0);
						p263StructApi522->advancedPrediction	= (pCapStruct->capBoolMask & h263_advancedPrediction ? 1:0);
						p263StructApi522->pbFrames				= (pCapStruct->capBoolMask & h263_pbFrames ? 1:0);				
						p263StructApi522->errorCompensation		= (pCapStruct->capBoolMask & h263_errorCompensation ? 1:0);
						p263StructApi522->temporalSpatialTradeOffCapability	= (pCapStruct->capBoolMask & h263_temporalSpatialTradeOffCapability ? 1:0);
											
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, p263Struct, IndexArr);
						
						BYTE *pEndAnnex = (BYTE *)pCapStruct->annexesMask.fds_bits[0];
						
						if(pCapStruct->annexesMask.fds_bits[0] && (err == 0))				
							err = ConvertAnnex((char *)pCapStruct,tempArr,IndexArr,newSize, &pEndAnnex);
						
						if((pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK) && (err == 0))
							err = ConverCustomFormat((char *)pEndAnnex,tempArr,IndexArr,newSize);
						
						PDELETEA(p263Struct);
						
						break;
					}
							
				case h264CapCode : 
				case H239ControlCapCode:
					{//this doesn't exist in old operator
						break;
					}		

				case h26LCapCode :
				case genericVideoCapCode:
					{
						size	= sizeof(genericVideoCapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;
						
						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);					
						
						genericVideoCapStruct	*pCapStruct			= ((genericVideoCapStruct*)(pCapBuffer->dataCap));
						BYTE					*pGenericStruct		= new BYTE[size];					
						genericVideoCapStructApi522	*pGenVidStruct	= (genericVideoCapStructApi522*)pGenericStruct;
						
						pGenVidStruct->h221NoneStand.h221NonStand.t35CountryCode	= NS_T35COUNTRY_CODE_USA;
						pGenVidStruct->h221NoneStand.h221NonStand.t35Extension		= NS_T35EXTENSION_USA;
						pGenVidStruct->h221NoneStand.h221NonStand.manufacturerCode	= NS_MANUFACTURER_PICTURETEL;
						pGenVidStruct->h221NoneStand.data[H26L_ONDATA_LOCATION]		= Polycom_H26L;
						pGenVidStruct->ParameterType								= kNonCollapsing;
						pGenVidStruct->Parameter.pNonCollapsimg.h221NoneStand.h221NonStand.t35CountryCode	= NS_T35COUNTRY_CODE_USA;
						pGenVidStruct->Parameter.pNonCollapsimg.h221NoneStand.h221NonStand.t35Extension	= NS_T35EXTENSION_USA;
						pGenVidStruct->Parameter.pNonCollapsimg.h221NoneStand.h221NonStand.manufacturerCode= NS_MANUFACTURER_PICTURETEL;
						pGenVidStruct->Parameter.pNonCollapsimg.h221NoneStand.data[H26L_PROFILE_ONDATA_LOCATION]	= Polycom_H26L_Profile;
						pGenVidStruct->Parameter.pNonCollapsimg.octetString[H26L_PROFILE_ONDATA_LOCATION]  = pCapStruct->data[H26L_PROFILE_ONDATA_LOCATION];
						pGenVidStruct->Parameter.pNonCollapsimg.octetString[H26L_MPI_ONDATA_LOCATION]	= pCapStruct->data[H26L_MPI_ONDATA_LOCATION];
						pGenVidStruct->Parameter.pNonCollapsimg.octetString[H26L_DUAL_MPI_ONDATA_LOCATION] = 0;// we don't support dual stream h26L
						pGenVidStruct->Parameter.pNonCollapsimg.octetString[H26L_DUAL_MPI_ONDATA_LOCATION+1] = '\0';// end the octet string
						pGenVidStruct->maxBitRate	= pCapStruct->maxBitRate;
						
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, pGenericStruct, IndexArr);
						
						PDELETEA(pGenericStruct);
						
						break;
					}							    		
				case h262CapCode :			// for future use
				case IS11172VideoCapCode :  // for future use
				case t120DataCapCode : 			
					{
						size	= sizeof(t120DataCapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;

						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);					

						BYTE	*pCapStruct		= ((BYTE*)(pCapBuffer->dataCap));
						
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, pCapStruct, IndexArr);
						
						break;
					}
				case h224DataCapCode : 
					{
						size	= sizeof(h224DataCapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;

						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);					

						BYTE	*pCapStruct		= ((BYTE*)(pCapBuffer->dataCap));
						
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, pCapStruct, IndexArr);
						
						break;
					}
				case genericCapCode:
				case nonStandardCapCode : 
					{

						size	= sizeof(nonStandardCapStructApi522);
						newSize	+= sizeof(capBufferBaseApi522) + size;

						err = ConvertCapBuffAndHeader((char *)pCapBuffer,tempArr,IndexArr,size);	
						
						ctNonStandardCapStruct		*pCapStruct			= ((ctNonStandardCapStruct*)(pCapBuffer->dataCap));
						BYTE						*pNonStanStruct		= new BYTE[size];					
						nonStandardCapStructApi522	*pNonStruct522		= (nonStandardCapStructApi522*)pNonStanStruct;

						pNonStruct522->nonStandard.nonStandardIdentifier.h221NonStandard.t35CountryCode		= pCapStruct->nonStandardData.info.t35CountryCode;
						pNonStruct522->nonStandard.nonStandardIdentifier.h221NonStandard.t35Extension		= pCapStruct->nonStandardData.info.t35Extension;
						pNonStruct522->nonStandard.nonStandardIdentifier.h221NonStandard.manufacturerCode	= pCapStruct->nonStandardData.info.manufacturerCode;

						memset(pNonStruct522->nonStandard.data,0,NonStandard_Data_Size);
						memcpy(pNonStruct522->nonStandard.data,pCapStruct->nonStandardData.data,CT_NonStandard_Data_Size);
						
						if(err == 0)
							err = CopyStructToOldformArray(sizeHeader, size, tempArr, pNonStanStruct, IndexArr);

						PDELETEA(pNonStanStruct);
						
						break;
					}
				case UnknownAlgorithemCapCode :
					break;
				default: 
					
					break;
				}

				size		= sizeof(capBufferBase) + pCapBuffer->capLength;
				oldSize		-= size;
				pTempPtr	+= size; 
				pCapBuffer	= (capBuffer*)pTempPtr;
			}


			m_size = newSize;
			m_ostr <<  m_size << "\n";
			
			PDELETEA(m_pStr);
			m_pStr = new BYTE[m_size];

			if(m_pStr == NULL)
			{
				PASSERT(!m_pStr);
				PDELETEA( tempArr );
				return;
			}
			
			for (i=0; i<m_size; i++)
				m_ostr << tempArr[i] << "\n";

			memcpy(m_pStr,(char*)tempArr,m_size); 


			PDELETEA( tempArr );
		}
		else
		{
			m_ostr <<  m_size << "\n";
			for (i=0; i<m_size; i++)
				m_ostr <<  (WORD)m_pStr[i] << "\n";
			
			return; //in m_size==0 there are no details so we need to get out from the function.
		}
	}
	else if(apiNum <= 599)
	{
		int			newSize		= 0;
		int			size		= 0;
		int			oldSize		= m_size;
		int			sizeHeader	= sizeof(ctCapStruct);
		
		WORD		*tempArr	= new WORD[SIZE_STREAM];
		int			IndexArr	= 0;
		
		capBuffer	*pCapBuffer	= (capBuffer *) m_pStr;
		char		*pTempPtr	= (char*)pCapBuffer;
		
		memset(tempArr,0,SIZE_STREAM);
		while((oldSize > 0) && (err == 0))
		{
			if(pCapBuffer->capTypeCode == h263CapCode)
			{
				size = sizeof(capBufferBase) + sizeof(h263CapStructBase);
				newSize	+= size;
				
				h263CapStruct	*pCapStruct		= ((h263CapStruct*)(pCapBuffer->dataCap));
				
				if(pCapStruct->annexesMask.fds_bits[0])
				{
					//In version 7 in place H263_Annexes_Number there is typeAnnexI_NS - in case it exist we should remove it from the
					//mask only - annexI_NS has no bodey.					
					if(CAP_FD_ISSET(typeAnnexI_NS, &(pCapStruct->annexesMask)))
						CAP_FD_CLR(typeAnnexI_NS, &(pCapStruct->annexesMask));
				}
				
				err = CopyStructToOldformArray(0, size, tempArr,(BYTE *)pCapBuffer, IndexArr);
				
				BYTE *pStartAnnex = (BYTE *)pCapStruct->annexesPtr;
				BYTE *pEndAnnex = (BYTE *)pCapStruct->annexesPtr;
				
				if(pCapStruct->annexesMask.fds_bits[0] && (err == 0))	
				{
					int annexesSize = GetSizeOfAnnexes((char *)pCapStruct, &pEndAnnex);
					err = CopyStructToOldformArray(0, annexesSize, tempArr,(BYTE *)pStartAnnex, IndexArr);												
					newSize += annexesSize;
				}
				
				if(pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
				{
					int customSize = GetSizeOfCustoms((char *)pEndAnnex);
					err = CopyStructToOldformArray(0, customSize, tempArr,(BYTE *)pEndAnnex, IndexArr);
					newSize += customSize;
				}

			}
			else
			{			
				size	= sizeof(capBufferBase) + pCapBuffer->capLength;
				newSize	+=  size;
				err = CopyStructToOldformArray(0, size, tempArr, (BYTE *)pCapBuffer, IndexArr);										
			}

			size		= sizeof(capBufferBase) + pCapBuffer->capLength;
			oldSize		-= size;
			pTempPtr	+= size; 
			pCapBuffer	= (capBuffer*)pTempPtr;
			
		} //while
		
		m_size = newSize;
		m_ostr <<  m_size << "\n";
		
		PDELETEA(m_pStr);
		m_pStr = new BYTE[m_size];
		
		if(m_pStr == NULL)
		{
			PASSERT(!m_pStr);
			PDELETEA( tempArr );
			return;
		}
		
		for (i=0; i<m_size; i++)
			m_ostr << tempArr[i] << "\n";
		
		memcpy(m_pStr,(char*)tempArr,m_size); 
				
		PDELETEA( tempArr );
		
	}
	else
	{*/ //CARMEL		
		m_ostr <<  m_size << "\n";
		for (i=0; i<m_size; i++)
			m_ostr <<  (BYTE)m_pStr[i] << "\n";
	//}

}

/////////////////////////////////////////////////////////////////////////////        
void  CH323StrCap::Serialize(WORD format, CSegment  *pSeg)
{                        
    WORD    i;
	
	*pSeg <<  m_size << "\n";
	for (i=0; i<m_size; i++)
		*pSeg <<  (BYTE)m_pStr[i] << "\n";
}

/////////////////////////////////////////////////////////////////////////////
//Description
//Receive old custom format structure and convert it to new structure of custom format
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::ConverCustomFormatOld(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize)
{	
	
	customPic_StApi522      *pCustomFormatsApi522 = (customPic_StApi522*)pStruct;

	int err;
	int		size					= sizeof(customPic_StBase);
	BYTE	*pCustom				= new BYTE[size];
	customPic_StBase *pCustomFormat = (customPic_StBase*)pCustom;

	pCustomFormat->customPictureClockFrequency.clockConversionCode	= (APIS8)pCustomFormatsApi522->customPictureClockFrequency.clockConversionCode;
	pCustomFormat->customPictureClockFrequency.clockDivisor			= (APIS8)pCustomFormatsApi522->customPictureClockFrequency.clockDivisor;
	pCustomFormat->customPictureClockFrequency.sqcifMPI				= (APIS16)pCustomFormatsApi522->customPictureClockFrequency.sqcifMPI;
	pCustomFormat->customPictureClockFrequency.qcifMPI				= (APIS16)pCustomFormatsApi522->customPictureClockFrequency.qcifMPI;
	pCustomFormat->customPictureClockFrequency.cifMPI				= (APIS16)pCustomFormatsApi522->customPictureClockFrequency.cifMPI;
	pCustomFormat->customPictureClockFrequency.cif4MPI				= (APIS16)pCustomFormatsApi522->customPictureClockFrequency.cif4MPI;
	pCustomFormat->customPictureClockFrequency.cif16MPI				= (APIS16)pCustomFormatsApi522->customPictureClockFrequency.cif16MPI;

	pCustomFormat->numberOfCustomPic	= pCustomFormatsApi522->numberOfCustomPic;

	newSize += size;
	err = CopyStructToOldformArray(0, size, tempArr, pCustom, IndexArr);

	PDELETEA(pCustom);
	if(err == -1)
		return -1;

	int	numberOfCustomPic = pCustomFormatsApi522->numberOfCustomPic;

	customPicFormatStApi522	*pCurrentCustomFormat522 = (customPicFormatStApi522 *)pCustomFormatsApi522->customPicPtr;
	char					*pChar = (char *)pCustomFormatsApi522->customPicPtr;

	for(int j=0; j < numberOfCustomPic; j++)
	{		
		if(pCurrentCustomFormat522)
		{
			size										= sizeof(customPicFormatSt);
			BYTE				*pCurrCust				= new BYTE[size];
			customPicFormatSt	*pCurrentCustomFormat	= (customPicFormatSt*)pCurrCust;
			
			pCurrentCustomFormat->maxCustomPictureWidth		= (APIU16)pCurrentCustomFormat522->maxCustomPictureWidth;
			pCurrentCustomFormat->maxCustomPictureHeight	= (APIU16)pCurrentCustomFormat522->maxCustomPictureHeight;
			pCurrentCustomFormat->minCustomPictureWidth		= (APIU16)pCurrentCustomFormat522->minCustomPictureWidth;
			pCurrentCustomFormat->minCustomPictureHeight	= (APIU16)pCurrentCustomFormat522->minCustomPictureHeight;
			pCurrentCustomFormat->customMPI					= (APIU16)pCurrentCustomFormat522->mPI.customPCF.customMPI;
			pCurrentCustomFormat->clockConversionCode		= (APIU16)pCurrentCustomFormat522->mPI.customPCF.clockConversionCode;
			pCurrentCustomFormat->clockDivisor				= (APIS8)pCurrentCustomFormat522->mPI.customPCF.clockDivisor;
			pCurrentCustomFormat->standardMPI				= (APIU8)pCurrentCustomFormat522->mPI.standardMPI;
			pCurrentCustomFormat->pixelAspectCode[0]		= (APIU8)pCurrentCustomFormat522->pixelAspectInformation.pixelAspectCode[0];
									
			newSize += size;
			err = CopyStructToOldformArray(0, size, tempArr, pCurrCust, IndexArr);
			
			PDELETEA(pCurrCust);
			if(err == -1)
				return -1;
		}
					
		pChar += sizeof(customPicFormatStApi522);
		pCurrentCustomFormat522 = (customPicFormatStApi522 *)pChar;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
//Description
//Receive old Annexes structure and return the size of aneexes
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::GetSizeOfAnnexes(char *pStruct, BYTE **ppAnnexPtr)
{
	h263CapStruct	*pCapStruct = (h263CapStruct*)pStruct;
	BYTE			*pAnnexPtr		= (BYTE *)pCapStruct->annexesPtr;
	int				size			= 0;

	for (annexesListEn annex = typeAnnexB; annex < H263_Annexes_Number; annex++)
	{
		if (CAP_FD_ISSET(annex, &(pCapStruct->annexesMask))) 
		{
			size += sizeof(h263OptionsStruct);
			pAnnexPtr += sizeof(h263OptionsStruct);
		}
	}
	
	*ppAnnexPtr						= pAnnexPtr;

	return size;
}
/////////////////////////////////////////////////////////////////////////////
//Description
//Receive old Annexes structure and return the size of customs
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::GetSizeOfCustoms(char *pStruct)
{
	customPic_St    *pCustomFormats = (customPic_St*)pStruct;
	int				size			= sizeof(customPic_StBase);
	int	numberOfCustomPic			= pCustomFormats->numberOfCustomPic;

	customPicFormatSt	*pCurrentCustomFormat = (customPicFormatSt *)pCustomFormats->customPicPtr;
	char				*pChar = (char *)pCustomFormats->customPicPtr;

	for(int j=0; j < numberOfCustomPic; j++)
	{		
		if(pCurrentCustomFormat)
			size += sizeof(customPicFormatSt);
					
		pChar += sizeof(customPicFormatSt);
		pCurrentCustomFormat = (customPicFormatSt *)pChar;
	}
	return size;
}
/////////////////////////////////////////////////////////////////////////////
//Description
//Receive old Annexes structure and convert it to new structure of Annexes
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::ConvertAnnexOld(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize, BYTE **ppAnnexPtr)
{	
	h263CapStructApi522	*pCapStruct522 = (h263CapStructApi522*)pStruct;
	BYTE				*pAnnexPtr		= (BYTE *)pCapStruct522->annexesPtr;
	*ppAnnexPtr							= pAnnexPtr;
	int					size			= 0;
	int					err				= 0;
	
	for (annexesListEn annex = typeAnnexB; (annex < H263_Annexes_Number) && (err == 0); annex++)
	{
		if (CAP_FD_ISSET(annex, &(pCapStruct522->annexesMask))) 
		{
			size = sizeof(h263OptionsStruct);
			newSize += size;
			
			switch(annex)
			{
			case typeAnnexB:
			case typeAnnexE:
			case typeAnnexF:
			case typeAnnexG:
			case typeAnnexH:
			case typeAnnexO:
			case typeAnnexV:
			case typeAnnexW:
				{
					annexBEFGHO_StApi522	*pCapStruct522		= ((annexBEFGHO_StApi522*)pAnnexPtr);
					BYTE					*pAnnexStruct		= new BYTE[size];
					annexBEFGHO_St			*pAnnexStructNew	= (annexBEFGHO_St *)pAnnexStruct;
					
					pAnnexStructNew->dummy = (APIU8)pCapStruct522->dummy;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexStruct, IndexArr);
					
					PDELETEA(pAnnexStruct);

					break;
				}
			case typeAnnexD:
				{
					annexD_StApi522	*pCapStruct522		= ((annexD_StApi522*)pAnnexPtr);
					BYTE			*pAnnexDStruct		= new BYTE[size];
					annexD_St		*pAnnexDStructNew	= (annexD_St *)pAnnexDStruct;
					
					pAnnexDStructNew->annexBoolMask = 0;

					if(pCapStruct522->unlimitedMotionVectors > 0)
						pAnnexDStructNew->annexBoolMask |= annexD_unlimitedMotionVectors;
						
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexDStruct, IndexArr);
					
					PDELETEA(pAnnexDStruct);

					break;
				}
				
			case typeAnnexI:
				{
					annexI_StApi522	*pCapStruct522		= ((annexI_StApi522*)pAnnexPtr);
					BYTE			*pAnnexIStruct		= new BYTE[size];
					annexI_St		*pAnnexIStructNew	= (annexI_St *)pAnnexIStruct;
					
					pAnnexIStructNew->annexBoolMask = 0;

					if(pCapStruct522->advancedIntraCodingMode > 0)
						pAnnexIStructNew->annexBoolMask |= annexI_advancedIntraCodingMode;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexIStruct, IndexArr);
					
					PDELETEA(pAnnexIStruct);
					
					break;
				}
			case typeAnnexJ:
				{
					annexJ_StApi522	*pCapStruct522		= ((annexJ_StApi522*)pAnnexPtr);
					BYTE			*pAnnexJStruct		= new BYTE[size];
					annexJ_St		*pAnnexJStructNew	= (annexJ_St *)pAnnexJStruct;

					pAnnexJStructNew->annexBoolMask = 0;

					if(pCapStruct522->deblockingFilterMode > 0)
						pAnnexJStructNew->annexBoolMask |= annexJ_deblockingFilterMode;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexJStruct, IndexArr);
					
					PDELETEA(pAnnexJStruct);
					break;
				}
			case typeAnnexK:
				{

					annexK_StApi522	*pCapStruct522		= ((annexK_StApi522*)pAnnexPtr);
					BYTE			*pAnnexKStruct		= new BYTE[size];
					annexK_St		*pAnnexKStructNew	= (annexK_St *)pAnnexKStruct;

					pAnnexKStructNew->annexBoolMask = 0;

					if(pCapStruct522->slicesInOrder_NonRect > 0)
						pAnnexKStructNew->annexBoolMask |= annexK_slicesInOrder_NonRect;

					if(pCapStruct522->slicesInOrder_Rect > 0)
						pAnnexKStructNew->annexBoolMask |= annexK_slicesInOrder_Rect;

					if(pCapStruct522->slicesNoOrder_NonRect > 0)
						pAnnexKStructNew->annexBoolMask |= annexK_slicesNoOrder_NonRect;

					if(pCapStruct522->slicesNoOrder_Rect > 0)
						pAnnexKStructNew->annexBoolMask |= annexK_slicesNoOrder_Rect;
															
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexKStruct, IndexArr);
					
					PDELETEA(pAnnexKStruct);
					
					break;
				}
			case typeAnnexL:
				{

					annexL_StApi522	*pCapStruct522		= ((annexL_StApi522*)pAnnexPtr);
					BYTE			*pAnnexLStruct		= new BYTE[size];
					annexL_St		*pAnnexLStructNew	= (annexL_St *)pAnnexLStruct;

					pAnnexLStructNew->annexBoolMask = 0;

					if(pCapStruct522->fullPictureFreeze > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_fullPictureFreeze;

					if(pCapStruct522->partialPictureFreezeAndRelease > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_partialPictureFreezeAndRelease;

					if(pCapStruct522->resizingPartPicFreezeAndRelease > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_resizingPartPicFreezeAndRelease;

					if(pCapStruct522->fullPictureSnapshot > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_fullPictureSnapshot;

					if(pCapStruct522->partialPictureSnapshot > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_partialPictureSnapshot;

					if(pCapStruct522->videoSegmentTagging > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_videoSegmentTagging;

					if(pCapStruct522->progressiveRefinement > 0)
						pAnnexLStructNew->annexBoolMask |= annexL_progressiveRefinement;
					
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexLStruct, IndexArr);
					
					PDELETEA(pAnnexLStruct);
					
					break;
				}
			case typeAnnexM:
				{
					annexM_StApi522	*pCapStruct522		= ((annexM_StApi522*)pAnnexPtr);
					BYTE			*pAnnexMStruct		= new BYTE[size];
					annexM_St		*pAnnexMStructNew	= (annexM_St *)pAnnexMStruct;

					pAnnexMStructNew->annexBoolMask = 0;

					if(pCapStruct522->improvedPBFramesMode > 0)
						pAnnexMStructNew->annexBoolMask |= annexM_improvedPBFramesMode;
					
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexMStruct, IndexArr);
					
					PDELETEA(pAnnexMStruct);

					break;
				}
			case typeAnnexN:
			case typeAnnexU:
				{					
					annexN_StApi522	*pCapStruct522			= ((annexN_StApi522*)pAnnexPtr);
					BYTE			*pAnnexNStruct			= new BYTE[size];

					refPictureSelectionStruct		*pRefPic = &((annexN_St*)pAnnexNStruct)->refPictureSelection;
					additionalPictureMemoryStruct	*pAddPic = &pRefPic->additionalPictureMemory;
					
					refPictureSelectionStructApi522		*pRefPic522 = &pCapStruct522->refPictureSelection;
					additionalPictureMemoryStructApi522	*pAddPic522 = &pRefPic522->additionalPictureMemory;
					
					pAddPic->sqcifAdditionalPictureMemory	= (APIU8)pAddPic522->sqcifAdditionalPictureMemory;
					pAddPic->qcifAdditionalPictureMemory	= (APIU8)pAddPic522->qcifAdditionalPictureMemory;
					pAddPic->cifAdditionalPictureMemory		= (APIU8)pAddPic522->cifAdditionalPictureMemory;
					pAddPic->cif4AdditionalPictureMemory	= (APIU8)pAddPic522->cif4AdditionalPictureMemory;
					pAddPic->cif16AdditionalPictureMemory	= (APIU8)pAddPic522->cif16AdditionalPictureMemory;
					pAddPic->bigCpfAdditionalPictureMemory	= (APIU8)pAddPic522->bigCpfAdditionalPictureMemory;
					pRefPic->videoBackChannelSend			= (APIU8)pRefPic522->videoBackChannelSend;
					pRefPic->mpuHorizMBs					= (APIU8)pRefPic522->mpuHorizMBs;
					pRefPic->mpuVertMBs						= (APIU8)pRefPic522->mpuVertMBs;

					pRefPic->annexBoolMask					= 0;

					if(pRefPic522->videoMux > 0)
						pRefPic->annexBoolMask |= refPic_videoMux;
										
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexNStruct, IndexArr);
					
					PDELETEA(pAnnexNStruct);									
					
					break;
				}
			case typeAnnexP:
				{

					annexP_StApi522	*pCapStruct522		= ((annexP_StApi522*)pAnnexPtr);
					BYTE			*pAnnexPStruct		= new BYTE[size];
					annexP_St		*pAnnexPStructNew	= (annexP_St *)pAnnexPStruct;

					pAnnexPStructNew->annexBoolMask = 0;

					if(pCapStruct522->dynamicPictureResizingByFour > 0)
						pAnnexPStructNew->annexBoolMask |= annexP_dynamicPictureResizingByFour;

					if(pCapStruct522->dynamicPictureResizingSixteenthPel > 0)
						pAnnexPStructNew->annexBoolMask |= annexP_dynamicPictureResizingSixteenthPel;
					
					if(pCapStruct522->dynamicWarpingHalfPel > 0)
						pAnnexPStructNew->annexBoolMask |= annexP_dynamicWarpingHalfPel;
					
					if(pCapStruct522->dynamicWarpingSixteenthPel > 0)
						pAnnexPStructNew->annexBoolMask |= annexP_dynamicWarpingSixteenthPel;
															
					err = CopyStructToOldformArray(0, size, tempArr, pAnnexPStruct, IndexArr);
					
					PDELETEA(pAnnexPStruct);									
					
					break;
				}
			case typeAnnexQ:
				{
					annexQ_StApi522	*pCapStruct522		= ((annexQ_StApi522*)pAnnexPtr);
					BYTE			*pAnnexQStruct		= new BYTE[size];
					annexQ_St		*pAnnexQStructNew	= (annexQ_St *)pAnnexQStruct;

					pAnnexQStructNew->annexBoolMask = 0;

					if(pCapStruct522->reducedResolutionUpdate > 0)
						pAnnexQStructNew->annexBoolMask |= annexQ_reducedResolutionUpdate;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexQStruct, IndexArr);
					
					PDELETEA(pAnnexQStruct);									

					break;
				}
			case typeAnnexR:
				{
					annexR_StApi522	*pCapStruct522		= ((annexR_StApi522*)pAnnexPtr);
					BYTE			*pAnnexRStruct		= new BYTE[size];
					annexR_St		*pAnnexRStructNew	= (annexR_St *)pAnnexRStruct;

					pAnnexRStructNew->annexBoolMask = 0;

					if(pCapStruct522->independentSegmentDecoding > 0)
						pAnnexRStructNew->annexBoolMask |= annexR_independentSegmentDecoding;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexRStruct, IndexArr);
					
					PDELETEA(pAnnexRStruct);									

					break;
				}
			case typeAnnexS:
				{
					annexS_StApi522	*pCapStruct522		= ((annexS_StApi522*)pAnnexPtr);
					BYTE			*pAnnexSStruct		= new BYTE[size];
					annexS_St		*pAnnexSStructNew	= (annexS_St *)pAnnexSStruct;

					pAnnexSStructNew->annexBoolMask = 0;

					if(pCapStruct522->alternateInterVLCMode > 0)
						pAnnexSStructNew->annexBoolMask |= annexS_alternateInterVLCMode;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexSStruct, IndexArr);
					
					PDELETEA(pAnnexSStruct);									

					break;
				}
			case typeAnnexT:
				{
					annexT_StApi522	*pCapStruct522		= ((annexT_StApi522*)pAnnexPtr);
					BYTE			*pAnnexTStruct		= new BYTE[size];
					annexT_St		*pAnnexTStructNew	= (annexT_St *)pAnnexTStruct;

					pAnnexTStructNew->annexBoolMask = 0;

					if(pCapStruct522->modifiedQuantizationMode > 0)
						pAnnexTStructNew->annexBoolMask |= annexT_modifiedQuantizationMode;

					err = CopyStructToOldformArray(0, size, tempArr, pAnnexTStruct, IndexArr);
					
					PDELETEA(pAnnexTStruct);									

					break;
				}
			default:
				break;
			}//switch

			pAnnexPtr += sizeof(h263OptionsStructApi522);
		} //if								
	}//for

	//Because of the typeAnnexI_NS that there is in version 7 but does not include in version 6 we should add 
	//as empty struct to the capabilities we reecieve from version 6.
	size = sizeof(h263OptionsStruct);
	BYTE	*pTempStruct	= new BYTE[size];
	err = CopyStructToOldformArray(0, size, tempArr, pTempStruct, IndexArr);
	PDELETEA(pTempStruct);
	
	pAnnexPtr += sizeof(h263OptionsStructApi522);
	newSize += size;

	return err;
}
/////////////////////////////////////////////////////////////////////////////
//Description
//Receive old 263 structure and return the size of the 263 new structure.
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::GetFullSizeOf263StructNew(char *pBuffer)
{
	int size							= sizeof(h263CapStructBase);
	capBufferApi522		*pCapBuffer		= (capBufferApi522 *) pBuffer;
	h263CapStructApi522	*pCapStruct		= ((h263CapStructApi522*)(pCapBuffer->dataCap));
	BYTE				*pAnnexPtr		= (BYTE *)pCapStruct->annexesPtr;

	if(pCapStruct->annexesMask.fds_bits[0])
	{
		for (annexesListEn annex = typeAnnexB; annex < H263_Annexes_Number; annex++)
		{
			if (CAP_FD_ISSET(annex, &(pCapStruct->annexesMask))) 		
			{
				size += sizeof(h263OptionsStruct);
				pAnnexPtr += sizeof(h263OptionsStructApi522);
			}		
		} //end for

		if(pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
		{
			customPic_StApi522     *pCustomFormats = (customPic_StApi522*)pAnnexPtr;

			size +=	sizeof(customPic_StBase);
			size +=	pCustomFormats->numberOfCustomPic * sizeof(customPicFormatSt);
		}
	}
	//Because of the typeAnnexI_NS that there is in version 7 but does not include in version 6 we should add 
	//as empty struct to the capabilities we reecieve from version 6.
	size += sizeof(h263OptionsStruct);
	pAnnexPtr += sizeof(h263OptionsStructApi522);
	
	return size;
}
/////////////////////////////////////////////////////////////////////////////
//Description
//Convert old capBuffer and header structures to new one
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::ConvertCapBuffAndHeaderOld(char *pBuffer,WORD *tempArr,int &IndexArr,int size)
{

	BYTE			*pBufBase			= new BYTE[sizeof(capBufferBase)];	
	capBufferBase	*pBufBaseNew		= (capBufferBase *)pBufBase;
	capBufferApi522	*pCapBufferApi522	= (capBufferApi522 *) pBuffer;
	int					err = 0;
	
	pBufBaseNew->capLength		= (APIU8)size;
	pBufBaseNew->capTypeCode	= (APIU16)pCapBufferApi522->capTypeCode;
	
	size = sizeof(capBufferBase);
	err = CopyStructToOldformArray(0, size, tempArr, pBufBase, IndexArr);
	
	PDELETEA(pBufBase);
	if(err == -1)
		return err;
	

	size = sizeof(ctCapStruct);

	capStructHeader *pHeaderApi522	= ((capStructHeader *)pCapBufferApi522->dataCap);
	BYTE			*pHeader		= new BYTE[size];
	ctCapStruct		*pHeaderNew		= (ctCapStruct *)pHeader;
			
	pHeaderNew->type		= pHeaderApi522->type;
	pHeaderNew->direction	= pHeaderApi522->direction;
	pHeaderNew->roleLabel	= pHeaderApi522->roleLabel;

	err = CopyStructToOldformArray(0, size, tempArr, pHeader, IndexArr);
	
	PDELETEA(pHeader);
	
	return err;
}

/////////////////////////////////////////////////////////////////////////////
//Because of the typeAnnexI_NS that there is in version 7 but does not include in version 6 we should shift the bits
//in the mask one to the left.
void CH323StrCap::HandleAnnexesMask(long *pAnnexMask)
{
	annexes_fd_set newMask;
	annexes_fd_set oldMask;
	oldMask.fds_bits[0] = *pAnnexMask;
	newMask.fds_bits[0] = 0;

	int i;
	for(i = 0; i<H263_Annexes_Number-1; i++)
	{
		if(CAP_FD_ISSET(i, &oldMask))		
			CAP_FD_SET(i,&newMask);		
	}

	for(i=H263_Annexes_Number + H263_Custom_Number-1; i>=H263_Annexes_Number-1; i--)
	{
		if(CAP_FD_ISSET(i, &oldMask))		
			CAP_FD_SET(i+1,&newMask);		
	}

	*pAnnexMask = newMask.fds_bits[0];
}

/////////////////////////////////////////////////////////////////////////////
void CH323StrCap::DeSerialize(WORD format, std::istream &m_istr)
{
// assuming format = OPERATOR_MCMS
	int i, err = 0;
	m_istr >> m_size;            
	WORD tmp;
	m_pStr = new BYTE[m_size];
	for (i=0; i<m_size; i++) 
	{
		m_istr >>  tmp;
		m_pStr[i]=(BYTE)tmp;
	}
	/*//CARMEL
	if(apiNum <= 522) //we received string from old mcu (lowest then version 6) and need to get string for version 6
	{
		int			newSize		= 0;
		int			size		= 0;
		int			oldSize		= m_size;
		int			sizeHeader	= sizeof(ctCapStruct);
		
		WORD		*tempArr	= new WORD[SIZE_STREAM];
		int			IndexArr	= 0;
		
		capBufferApi522	*pCapBuffer522	= (capBufferApi522 *) m_pStr;
		char			*pTempPtr522	= (char*)pCapBuffer522;
		
		while((oldSize > 0) && (err == 0))
		{
			switch(pCapBuffer522->capTypeCode)
			{
			case eG711Alaw64kCapCode : 
			case eG711Ulaw64kCapCode : 
			case eG711Alaw56kCapCode : 
			case eG711Ulaw56kCapCode : 
			case eG722_64kCapCode : 
			case eG722_56kCapCode : 
			case eG722_48kCapCode : 
			case eG728CapCode : 
			case eG729CapCode :
			case eG729AnnexACapCode :
			case eG729wAnnexBCapCode :				
			case eG729AnnexAwAnnexBCapCode :	 	    
			case eG7221_32kCapCode:
			case eG7221_24kCapCode:	
			case eSiren14_24kCapCode:
			case eSiren14_32kCapCode:
			case eSiren14_48kCapCode:
			case ePeopleContentCapCode : 
			case eRoleLabelCapCode :
				{
					size	= sizeof(simpleAudioCapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);		
		
					simpleAudioCapStructApi522	*pCapStruct522		= ((simpleAudioCapStructApi522*)(pCapBuffer522->dataCap));
					BYTE						*pAudStruct		= new BYTE[size];
					simpleAudioCapStruct		*pAudStructNew	= (simpleAudioCapStruct	*)pAudStruct;
										
					pAudStructNew->value	= pCapStruct522->value;
					
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, pAudStruct, IndexArr);
					
					PDELETEA(pAudStruct);
					
					break;
				}   
			case g7231CapCode :	
				{
					size	= sizeof(g7231CapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);	

					g7231CapStructApi522*pCapStruct522			= ((g7231CapStructApi522*)(pCapBuffer522->dataCap));
					BYTE				*p7231Struct		= new BYTE[size];
					g7231CapStruct		*p7231StructNew		= (g7231CapStruct *)p7231Struct;
										
					p7231StructNew->maxAl_sduAudioFrames = (INT16)pCapStruct522->maxAl_sduAudioFrames;
					p7231StructNew->capBoolMask			= 0;
					
					if(pCapStruct522->silenceSuppression > 0)
						p7231StructNew->capBoolMask |= g7231_silenceSuppression;
					
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, p7231Struct, IndexArr);
					
					PDELETEA(p7231Struct);
					
					break;
				}   			
			case IS11172AudioCapCode :			    // for future use
			case IS13818CapCode :					// for future use
			case G7231AnnexCapCode :				// for future use
				{
					size							= sizeof(capBufferBase);
					BYTE			*pBufBase		= new BYTE[size];	
					capBufferBase	 *pBufBaseNew	= (capBufferBase *)pBufBase;
					
					pBufBaseNew->capLength			= (UINT8)size;
					pBufBaseNew->capTypeCode		= (UINT16)pCapBuffer522->capTypeCode;
					
					if(err == 0)
						err = CopyStructToOldformArray(0, size, tempArr, pBufBase, IndexArr);
					
					PDELETEA(pBufBase);
					
					break;
				}
				
			case h261CapCode : 
				{
					size	= sizeof(h261CapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);					

					h261CapStructApi522	*pCapStruct522			= ((h261CapStructApi522*)(pCapBuffer522->dataCap));
					BYTE				*p261Struct			= new BYTE[size];
					h261CapStruct		*p261StructNew		= (h261CapStruct *)p261Struct;
					
					p261StructNew->qcifMPI		= (INT8)pCapStruct522->qcifMPI;
					p261StructNew->cifMPI		= (INT8)pCapStruct522->cifMPI;
					p261StructNew->maxBitRate	= (INT16)pCapStruct522->maxBitRate;
					p261StructNew->capBoolMask	= 0;

					if(pCapStruct522->temporalSpatialTradeOffCapability > 0)
						p261StructNew->capBoolMask |= h261_temporalSpatialTradeOffCapability;

					if(pCapStruct522->stillImageTransmission > 0)
						p261StructNew->capBoolMask |= h261_stillImageTransmission;
										
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, p261Struct, IndexArr);
					
					PDELETEA(p261Struct);
					
					break;
				}							    
			case h263CapCode : 
				{
					
					size = sizeof(h263CapStructBase);
					newSize	+= sizeof(capBufferBase) + size;
					
					int fullSize	= GetFullSizeOf263StructNew((char *)pCapBuffer522);
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,fullSize);										

					h263CapStructApi522	*pCapStruct522		= ((h263CapStructApi522*)(pCapBuffer522->dataCap));
					BYTE				*p263Struct		= new BYTE[size];
					h263CapStruct		*p263StructNew	= (h263CapStruct *)p263Struct;
						
					p263StructNew->maxBitRate	= pCapStruct522->maxBitRate;
					p263StructNew->hrd_B		= pCapStruct522->hrd_B;
					p263StructNew->bppMaxKb		= (INT16)pCapStruct522->bppMaxKb;
					p263StructNew->slowSqcifMPI	= (INT16)pCapStruct522->slowSqcifMPI;
					p263StructNew->slowQcifMPI	= (INT16)pCapStruct522->slowQcifMPI;
					p263StructNew->slowCifMPI	= (INT16)pCapStruct522->slowCifMPI;
					p263StructNew->slowCif4MPI	= (INT16)pCapStruct522->slowCif4MPI;
					p263StructNew->slowCif16MPI	= (INT16)pCapStruct522->slowCif16MPI;
					p263StructNew->sqcifMPI		= (INT8)pCapStruct522->sqcifMPI;
					p263StructNew->qcifMPI		= (INT8)pCapStruct522->qcifMPI;
					p263StructNew->cifMPI		= (INT8)pCapStruct522->cifMPI;
					p263StructNew->cif4MPI		= (INT8)pCapStruct522->cif4MPI;
					p263StructNew->cif16MPI		= (INT8)pCapStruct522->cif16MPI;
					p263StructNew->annexesMask	= pCapStruct522->annexesMask;					
					
					p263StructNew->capBoolMask	= 0;

					if(pCapStruct522->unrestrictedVector > 0)
						p263StructNew->capBoolMask |= h263_unrestrictedVector;

					if(pCapStruct522->arithmeticCoding > 0)
						p263StructNew->capBoolMask |= h263_arithmeticCoding;

					if(pCapStruct522->advancedPrediction > 0)
						p263StructNew->capBoolMask |= h263_advancedPrediction;
					
					if(pCapStruct522->pbFrames > 0)
						p263StructNew->capBoolMask |= h263_pbFrames;

					if(pCapStruct522->errorCompensation > 0)
						p263StructNew->capBoolMask |= h263_errorCompensation;

					if(pCapStruct522->temporalSpatialTradeOffCapability > 0)
						p263StructNew->capBoolMask |= h263_temporalSpatialTradeOffCapability;
					
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, p263Struct, IndexArr);
					
					BYTE *pEndAnnex = (BYTE *)pCapStruct522->annexesMask.fds_bits[0];
					
					if(pCapStruct522->annexesMask.fds_bits[0] && (err == 0))				
						err = ConvertAnnexOld((char *)pCapStruct522,tempArr,IndexArr,newSize, &pEndAnnex);
					
					if(pCapStruct522->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
						err = ConverCustomFormatOld((char *)pEndAnnex,tempArr,IndexArr,newSize);
					
					PDELETEA(p263Struct);
					
					break;
				}	
				
			case h264CapCode: 
			case H239ControlCapCode:
			{//this doesn't exist in old operator
				break;
			}	

			case h26LCapCode :
			case genericVideoCapCode:
				{
					size	= sizeof(genericVideoCapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);					
					
					genericVideoCapStructApi522	*pCapStruct522			= ((genericVideoCapStructApi522*)(pCapBuffer522->dataCap));
					BYTE						*pGenericStruct		= new BYTE[size];					
					genericVideoCapStruct		*pGenVidStructNew	= (genericVideoCapStruct*)pGenericStruct;

					pGenVidStructNew->genericCodeType					= H26LCode;
					pGenVidStructNew->maxBitRate						= pCapStruct522->maxBitRate;
					pGenVidStructNew->data[H26L_PROFILE_ONDATA_LOCATION]= pCapStruct522->Parameter.pNonCollapsimg.h221NoneStand.data[H26L_PROFILE_ONDATA_LOCATION];
					pGenVidStructNew->data[H26L_MPI_ONDATA_LOCATION]	= pCapStruct522->Parameter.pNonCollapsimg.octetString[H26L_MPI_ONDATA_LOCATION];
					pGenVidStructNew->data[H26L_DUAL_MPI_ONDATA_LOCATION] = 0;
					pGenVidStructNew->data[H26L_DUAL_MPI_ONDATA_LOCATION+1] = '\0';
					
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, pGenericStruct, IndexArr);
					
					PDELETEA(pGenericStruct);
					
					break;
				}							    		
			case h262CapCode :			// for future use
			case IS11172VideoCapCode :  // for future use
			case t120DataCapCode : 			
				{
					size	= sizeof(t120DataCapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);					
					
					BYTE	*pCapStruct522		= ((BYTE*)(pCapBuffer522->dataCap));
					
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, pCapStruct522, IndexArr);
					
					break;
				}
			case h224DataCapCode : 
				{
					size	= sizeof(h224DataCapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);					
					
					BYTE	*pCapStruct522		= ((BYTE*)(pCapBuffer522->dataCap));
					
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, pCapStruct522, IndexArr);					

					break;
				}
			case genericCapCode:
			case nonStandardCapCode : 
				{
					
					size	= sizeof(ctNonStandardCapStruct);
					newSize	+= sizeof(capBufferBase) + size;
					
					err = ConvertCapBuffAndHeaderOld((char *)pCapBuffer522,tempArr,IndexArr,size);	

					nonStandardCapStructApi522	*pCapStruct522			= ((nonStandardCapStructApi522*)(pCapBuffer522->dataCap));
					BYTE						*pNonStanStruct		= new BYTE[size];					
					ctNonStandardCapStruct		*pNonStructNew		= (ctNonStandardCapStruct*)pNonStanStruct;

					pNonStructNew->nonStandardData.info.t35CountryCode		= (UINT8)pCapStruct522->nonStandard.nonStandardIdentifier.h221NonStandard.t35CountryCode;
					pNonStructNew->nonStandardData.info.t35Extension		= (UINT8)pCapStruct522->nonStandard.nonStandardIdentifier.h221NonStandard.t35Extension;
					pNonStructNew->nonStandardData.info.manufacturerCode	= (UINT16)pCapStruct522->nonStandard.nonStandardIdentifier.h221NonStandard.manufacturerCode;

					memset(pNonStructNew->nonStandardData.data,0,CT_NonStandard_Data_Size);					
					memcpy(pNonStructNew->nonStandardData.data,pCapStruct522->nonStandard.data,CT_NonStandard_Data_Size);
										
					if(err == 0)
						err = CopyStructToOldformArray(sizeHeader, size, tempArr, pNonStanStruct, IndexArr);					
					
					PDELETEA(pNonStanStruct);
					
					break;
				}
			case UnknownAlgorithemCapCode :
				break;
			default: 
				
				break;
			}
			
			size		= sizeof(capBufferBaseApi522) + pCapBuffer522->capLength;
			oldSize		-= size;
			pTempPtr522	+= size; 
			pCapBuffer522	= (capBufferApi522*)pTempPtr522;
			
		} //while
		
		
		m_size = newSize;  
		PDELETEA(m_pStr);		
		m_pStr = new BYTE[m_size];
		PASSERT(!m_pStr);

		for (i=0; i<m_size; i++) 
			m_pStr[i]=(BYTE)tempArr[i];
				
		PDELETEA( tempArr );	
	}
	else if(apiNum <= 599)
	{
		int			newSize		= 0;
		int			size		= 0;
		int			oldSize		= m_size;
		int			sizeHeader	= sizeof(ctCapStruct);
		
		BYTE		*tempArr	= new BYTE[SIZE_STREAM];
		int			IndexArr	= 0;
		
		capBuffer	*pCapBuffer	= (capBuffer *) m_pStr;
		char		*pTempPtr	= (char*)pCapBuffer;
		
		memset(tempArr,0,SIZE_STREAM);
		while((oldSize > 0) && (err == 0))
		{
			if(pCapBuffer->capTypeCode == h263CapCode)
			{
				size = sizeof(capBufferBase) + sizeof(h263CapStructBase);
				newSize	+= size;
				
				h263CapStruct	*pCapStruct		= ((h263CapStruct*)(pCapBuffer->dataCap));
				
				if(pCapStruct->annexesMask.fds_bits[0])
					HandleAnnexesMask(&pCapStruct->annexesMask.fds_bits[0]);					
				
				err = CopyStruct(0, size, tempArr,(BYTE *)pCapBuffer, IndexArr);
				
				BYTE *pStartAnnex = (BYTE *)pCapStruct->annexesPtr;
				BYTE *pEndAnnex = (BYTE *)pCapStruct->annexesPtr;
				
				if(pCapStruct->annexesMask.fds_bits[0] && (err == 0))	
				{
					int annexesSize = GetSizeOfAnnexes((char *)pCapStruct, &pEndAnnex);
					err = CopyStruct(0, annexesSize, tempArr,(BYTE *)pStartAnnex, IndexArr);												
					newSize += annexesSize;
				}
				
				if(pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
				{
					int customSize = GetSizeOfCustoms((char *)pEndAnnex);
					err = CopyStruct(0, customSize, tempArr,(BYTE *)pEndAnnex, IndexArr);
					newSize += customSize;
				}
			}
			else
			{
				size	= sizeof(capBufferBase) + pCapBuffer->capLength;
				newSize	+= size;
				err = CopyStruct(0, size, tempArr, (BYTE *)pCapBuffer, IndexArr);										
			}

			size		= sizeof(capBufferBase) + pCapBuffer->capLength;
			oldSize		-= size;
			pTempPtr	+= size; 
			pCapBuffer	= (capBuffer*)pTempPtr;
			
		} //while
		
		
		m_size = newSize;  
		PDELETEA(m_pStr);		
		m_pStr = new BYTE[m_size];
		PASSERT(!m_pStr);

		for (i=0; i<m_size; i++) 
			m_pStr[i]=(BYTE)tempArr[i];
				
		PDELETEA( tempArr );	


	}*///CARMEL
}


/////////////////////////////////////////////////////////////////////////////
void  CH323StrCap::DeSerialize(WORD format, CSegment *pSeg)
{
	int i, err = 0;
	*pSeg >> m_size;            
	BYTE tmp;
	m_pStr = new BYTE[m_size];
	for (i=0; i<m_size; i++) 
	{
		if (pSeg->EndOfSegment())
		{
			//TODO : FIX THIS ASSERT !!!
			//PASSERTMSG(m_size, "CH323StrCap::DeSerialize over reading from segment 111");
			return; // SAGI - this funxtion caused assert in voip connection
		}
		*pSeg >>  tmp;
		m_pStr[i]=(BYTE)tmp;
	}
}


/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::CopyStruct(int startPoint, int toatlSize,BYTE *pToArray,BYTE *pFromArray,int &IndexArr)
{
	int i = 0;
	memcpy(&pToArray[IndexArr],pFromArray,toatlSize);

	IndexArr += toatlSize;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int CH323StrCap::CopyStructToOldformArray(int startPoint, int toatlSize,WORD *pToArray,BYTE *pFromArray,int &IndexArr)
{
	int i = 0;
	for (i=startPoint; i < toatlSize; i++)
	{
		if(IndexArr >= SIZE_STREAM)
		{
			PASSERT(IndexArr);
			return -1;
		}
		pToArray[IndexArr++] = pFromArray[i];
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CLASS CH323STRCOM
/////////////////////////////////////////////////////////////////////////////
//comm mode dump of H323 BAS commands vector
void  CH323strCom::Dump(std::ostream& ostr)
{
	//PASSERTMSG(1, "Not implemented");
  if(m_size != 0)
     DumpH323Cap(ostr, m_size,m_pStr);
}
