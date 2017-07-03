#include "NStream.h"
#include "DecoderResolutionTable.h"
#include "RsrcParams.h"
#include "Macros.h"
#include "VideoHardwareInterface.h"

#define CELL_ON   1
#define CELL_OFF  0
#define CELL_NA  (DWORD)-1

CDecoderResolutionTable::CDecoderResolutionTable()
{
	for (int i=0;i<NUM_ROWS;i++)
		for (int j=0;j<NUM_COLUMNS;j++)
			for (int k=0;k<TABLE_DEPTH;k++)
				REQUIRED_RESOLUTION_TABLE[i][j][k] = CELL_NA;
	
	for (int l=0;l<STATUS_VECTOR_LEN;l++)
		STATUS_VECTOR[l] = 0;
	
	for (int m=0;m<NUM_ROWS;m++)
		DECODERS_RSRC_PARAMS_VECTOR[m] = (CRsrcParams*)NULL;
		
	numEncoders = 0;
	m_useDecoderTable = TRUE;
}
////////////////////////////////////////////////////////////////
CDecoderResolutionTable::~CDecoderResolutionTable()
{
	for (int i=0;i<NUM_ROWS;i++)
		POBJDELETE(DECODERS_RSRC_PARAMS_VECTOR[i]);
}
////////////////////////////////////////////////////////////////
//
//	Adds a local copy of the rsrc params of the specified decoder to 
//  the first available row in column 0 of the table (the row header)
//
void CDecoderResolutionTable::AddDecoderToTable(CRsrcParams* pRsrcParams)
{
	if (!CPObject::IsValidPObjectPtr(pRsrcParams)) {
		PASSERT_AND_RETURN(1);
	}
	
	DWORD decoderConnectionId = pRsrcParams->GetConnectionId();
	PTRACE2INT(eLevelInfoNormal,"CDecoderResolutionTable::AddDecoderToTable, DecoderId: ", decoderConnectionId);
	int i, decoderIndex;
	
	CSmallString cstr;
	if (FindDecoderInTable(decoderConnectionId, decoderIndex))
	{
		cstr << "CDecoderResolutionTable::AddDecoderToTable , DecoderId: " << decoderConnectionId << "is already in the table in row: " << decoderIndex;
		PTRACE(eLevelError,cstr.GetString());
		PASSERT_AND_RETURN(2);
	}
	BYTE ans = FALSE;	
	for (i=1;i<NUM_ROWS;i++)
	{
		if(REQUIRED_RESOLUTION_TABLE[i][0][0] == CELL_NA)
		{			
			AddRsrcParamsToRsrcVector(pRsrcParams,i);
			REQUIRED_RESOLUTION_TABLE[i][0][0] = decoderConnectionId;
			ans = TRUE;
			break;
		}
	}
				
	if (!ans)
	{
		cstr << "CDecoderResolutionTable::AddDecoderToTable - table is full!!!";
		PTRACE(eLevelError,cstr.GetString());
		PASSERT_AND_RETURN(3);
	}
	
	DWORD statusIndex = i*MAX_DECODER_RESOLUTIONS;
	for (int j = 0; j<MAX_DECODER_RESOLUTIONS && statusIndex+j<STATUS_VECTOR_LEN; j++)
		STATUS_VECTOR[statusIndex+j] = 1;
	
}
////////////////////////////////////////////////////////////////
//
//	Adds the encoder connectionId to 
//  the first available column in row 0 of the table (the column header)
//
void CDecoderResolutionTable::AddEncoderToTable(DWORD encoderConnectionId)
{
	PTRACE2INT(eLevelInfoNormal,"CDecoderResolutionTable::AddEncoderToTable, EncoderId: ", encoderConnectionId);
	CSmallString cstr;
	int encoderIndex;
	if (FindDecoderInTable(encoderConnectionId, encoderIndex))
	{
		cstr << "CDecoderResolutionTable::AddEncoderToTable , EncoderId: " << encoderConnectionId << "is already in the table in column: " << encoderIndex;
		PTRACE(eLevelError,cstr.GetString());
		PASSERT_AND_RETURN(2);
	}
	BYTE ans = FALSE;
	for (int i=1;i<NUM_COLUMNS;i++)
		if(REQUIRED_RESOLUTION_TABLE[0][i][0] == CELL_NA)
		{
			REQUIRED_RESOLUTION_TABLE[0][i][0] = encoderConnectionId;
			ans = TRUE;
			numEncoders++;
			break;
		}
	
	if (!ans)
	{
		cstr << "CDecoderResolutionTable::AddEncoderToTable - table is full!!!";
		PTRACE(eLevelError,cstr.GetString());
		PASSERT_AND_RETURN(3);
	}
}
////////////////////////////////////////////////////////////////
//
//	Removes the row header
//	deletes the rsrc params local copy
//	update the table 
void CDecoderResolutionTable::RemoveDecoderFromTable(DWORD decoderConnectionId)
{
	PTRACE2INT(eLevelInfoNormal,"CDecoderResolutionTable::RemoveDecoderFromTable, DecoderId: ", decoderConnectionId);
	int decoderIndex = 0;
	BYTE ans = FindDecoderInTable(decoderConnectionId, decoderIndex);
	
	if (ans)
	{
		RemoveRow(decoderIndex);
		RemoveRsrcParamsFromRsrcVector(decoderIndex);
		CheckConsistency();
	}
				
	if(!ans)
		PTRACE2INT(eLevelInfoNormal,"CDecoderResolutionTable::RemoveDecoderFromTable - did not find decoderId in table - ",decoderConnectionId);
}
////////////////////////////////////////////////////////////////
//  Removes the column header
//	update the table

void CDecoderResolutionTable::RemoveEncoderFromTable(DWORD encoderConnectionId)
{
	PTRACE2INT(eLevelInfoNormal,"CDecoderResolutionTable::RemoveEncoderFromTable, EncoderId: ", encoderConnectionId);
	int encoderIndex = 0;
	BYTE ans = FindEncoderInTable(encoderConnectionId, encoderIndex);
	
	if (ans)
	{
		REQUIRED_RESOLUTION_TABLE[0][encoderIndex][0] = CELL_NA;
		RemoveColumn(encoderIndex);
		numEncoders--;
		CheckConsistency();
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CDecoderResolutionTable::RemoveEncoderFromTable - did not find encoderId in table - " ,encoderConnectionId);
		PASSERT(4);
	}
}
////////////////////////////////////////////////////////////////
//
//  Update the table according to the change layout req
//	1) Find the right column according to encoderId
//	2) go over all active decoders
//	3) update their required resolution according to CardManager func
//	4) check if the update changes the status of the row
//	5) if it does -> send update decoder resolution to the decoder
//
void CDecoderResolutionTable::UpdateTable(const CHANGE_LAYOUT_S& changeLayoutReq, DWORD encoderConnectionId)
{
	PTRACE(eLevelInfoNormal,"CDecoderResolutionTable::UpdateTable");
	CSmallString cstr;
	int encoderIndex = -1;
	if (!FindEncoderInTable(encoderConnectionId, encoderIndex))
	{
		cstr << "CDecoderResolutionTable::UpdateTable , did not find encoderId in table - " << encoderConnectionId;
		PTRACE(eLevelError,cstr.GetString());
		PASSERT_AND_RETURN(4);
	}
			
	// update all active decoders
	for (int j = 1; j < NUM_ROWS;j++)
	{
		if (REQUIRED_RESOLUTION_TABLE[j][0][0] == CELL_NA)
			continue;
		DWORD decoderConnectionId = REQUIRED_RESOLUTION_TABLE[j][0][0];
		BYTE requiredResolution = 0xFF;
		int imageIndex = -1;
		if (IsDecoderInLayout(decoderConnectionId, changeLayoutReq.atImageParam,imageIndex) == TRUE)
		{
			if (imageIndex == -1) {
				PASSERT_AND_RETURN(1);
			}

			requiredResolution = CalculateDecoderResolution(changeLayoutReq.nLayoutType, changeLayoutReq.nEncoderResolutionRatio, changeLayoutReq.atImageParam[imageIndex].nDecoderResolutionRatio, changeLayoutReq.atImageParam[imageIndex].nDecoderSizeInLayout);
		}
		
		cstr << "party (encoderId): " << encoderConnectionId << " will see party (DecoderId) :" << decoderConnectionId << " in resolution: " << requiredResolution <<"\n";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		cstr.Clear();
		FillResolutionForEncoderDecoder(j,encoderIndex,requiredResolution);
		if (UpdateStatusIfNeeded(j))
			CreateHardwareInterfaceFillStructAndSendUpdateDecoder(j);
	}
	CheckConsistency();
}
////////////////////////////////////////////////////////////////
//
//	update the status for a given decoder 
//	returns TRUE if the required resolution for the specific decoder has changed
//
BYTE CDecoderResolutionTable::UpdateStatusIfNeeded(DWORD DecoderIndex) 
{
	PTRACE(eLevelInfoNormal,"CDecoderResolutionTable::UpdateStatusIfNeeded");
	
	BYTE UpdateQcif = UpdateRowStatus(DecoderIndex, DECODER_1_16_RESOLUTION);
	BYTE UpdateCif = UpdateRowStatus(DecoderIndex, DECODER_1_4_RESOLUTION);
	BYTE UpdateSD = UpdateRowStatus(DecoderIndex, DECODER_FULL_RESOLUTION);
	
	// for debug
	// DumpTable(TRUE,numEncoders);
	
	return (UpdateQcif || UpdateCif || UpdateSD);
}
////////////////////////////////////////////////////////////////
//
//	update the status for a given decoder and a given resolution
//	returns TRUE if the specific resolution for the specific decoder has changed
//
BYTE CDecoderResolutionTable::UpdateRowStatus(DWORD index, DWORD resolution)
{
	BYTE updatedStatus = 0;
	
	for (int i=1;i<NUM_COLUMNS;i++)
	{
		if (REQUIRED_RESOLUTION_TABLE[index][i][resolution] != CELL_NA)
			updatedStatus |= REQUIRED_RESOLUTION_TABLE[index][i][resolution];
	}
	
	DWORD statusIndex = index*MAX_DECODER_RESOLUTIONS + resolution;
	if (updatedStatus != STATUS_VECTOR[statusIndex])
	{
		STATUS_VECTOR[statusIndex] = updatedStatus;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}
////////////////////////////////////////////////////////////////
BYTE CDecoderResolutionTable::RemoveRow(DWORD index)
{
	
	for (int i=0;i<NUM_COLUMNS;i++)
		for(int j=0;j<TABLE_DEPTH;j++)
	{
		REQUIRED_RESOLUTION_TABLE[index][i][j] = CELL_NA;
	}
	
	DWORD statusIndex = index*MAX_DECODER_RESOLUTIONS;
	for (int k = 0;k < MAX_DECODER_RESOLUTIONS;k++)
		STATUS_VECTOR[statusIndex+k] = 0;
	
	return TRUE;
	
}
////////////////////////////////////////////////////////////////
BYTE CDecoderResolutionTable::RemoveColumn(DWORD index)
{
	BYTE needToUpdateStatus;
	for (int i=0;i<NUM_ROWS;i++)
	{
		needToUpdateStatus = FALSE;
		if (REQUIRED_RESOLUTION_TABLE[i][0][0] == CELL_NA)
			continue;
		
		for(int j=0;j<TABLE_DEPTH;j++)
		{
			if (REQUIRED_RESOLUTION_TABLE[i][index][j] != CELL_NA)
			{
				REQUIRED_RESOLUTION_TABLE[i][index][j] = CELL_OFF;
				needToUpdateStatus = TRUE;
			}
		}
		
		if (needToUpdateStatus == TRUE)
			UpdateStatusIfNeeded(i);
		
		for(int j=0;j<TABLE_DEPTH;j++)
		{
			REQUIRED_RESOLUTION_TABLE[i][index][j] = CELL_NA;
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////
void CDecoderResolutionTable::CreateHardwareInterfaceFillStructAndSendUpdateDecoder(DWORD decoderIndex)
{
	CRsrcParams* pRsrcParams =  DECODERS_RSRC_PARAMS_VECTOR[decoderIndex];
	if (!CPObject::IsValidPObjectPtr(pRsrcParams)) {
		PASSERT_AND_RETURN(1);
	}
	
	DWORD statusIndex = decoderIndex*MAX_DECODER_RESOLUTIONS;// + resolution;
	DWORD OnOffResolution0 = STATUS_VECTOR[statusIndex+DECODER_1_16_RESOLUTION];
	DWORD OnOffResolution1 = STATUS_VECTOR[statusIndex+DECODER_1_4_RESOLUTION];
	DWORD OnOffResolution4 = STATUS_VECTOR[statusIndex+DECODER_FULL_RESOLUTION];
	
	CVideoHardwareInterface tempVideoHardwareInterface(pRsrcParams->GetConnectionId(),pRsrcParams->GetPartyRsrcId(),pRsrcParams->GetConfRsrcId(),pRsrcParams->GetLogicalRsrcType());
	if (GetUseDecoderTableTrueFalse() == TRUE)
		tempVideoHardwareInterface.SendUpdateDecoderResolution(pRsrcParams->GetConnectionId(),OnOffResolution0,OnOffResolution1,OnOffResolution4);
}
////////////////////////////////////////////////////////////////
BYTE CDecoderResolutionTable::IsDecoderInLayout(DWORD decoderConnectionId, const IMAGE_PARAM_S* imagesVector, int& imageIndex)
{
	BYTE ans = FALSE;
	for (int i = 0;i<MAX_NUMBER_OF_CELLS_IN_LAYOUT;i++)
		if (imagesVector[i].tDecoderPhysicalId.connection_id == decoderConnectionId)
		{
			imageIndex = i;
			return TRUE;
		}
	
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////
void CDecoderResolutionTable::FillResolutionForEncoderDecoder(DWORD decoderIndex,DWORD encoderIndex,BYTE requiredResolution)
{
	REQUIRED_RESOLUTION_TABLE[decoderIndex][encoderIndex][DECODER_1_16_RESOLUTION] = CELL_OFF;
	REQUIRED_RESOLUTION_TABLE[decoderIndex][encoderIndex][DECODER_1_4_RESOLUTION] = CELL_OFF;
	REQUIRED_RESOLUTION_TABLE[decoderIndex][encoderIndex][DECODER_FULL_RESOLUTION] = CELL_OFF;
	
	if (requiredResolution != 0xFF)
		REQUIRED_RESOLUTION_TABLE[decoderIndex][encoderIndex][requiredResolution] = CELL_ON;
}
///////////////////////////////////////////////////////////////////////////////////////
BYTE CDecoderResolutionTable::FindDecoderInTable(DWORD decoderConnectionId,int& decoderIndex)
{
	BYTE ans = FALSE;
	for (int i = 1; i < NUM_ROWS;i++)
	{
		if (REQUIRED_RESOLUTION_TABLE[i][0][0] == CELL_NA)
			continue;
		DWORD tempEncoderConnectionId = REQUIRED_RESOLUTION_TABLE[i][0][0];
		if (tempEncoderConnectionId ==  decoderConnectionId)
		{
			ans = TRUE;
			decoderIndex = i;
			break;
		}
	}
	return ans;
	
}
///////////////////////////////////////////////////////////////////////////////////////
BYTE CDecoderResolutionTable::FindEncoderInTable(DWORD encoderConnectionId, int& encoderIndex)
{
	BYTE ans = FALSE;
	for (int i = 1; i < NUM_COLUMNS;i++)
	{
		if (REQUIRED_RESOLUTION_TABLE[0][i][0] == CELL_NA)
			continue;
		DWORD tempEncoderConnectionId = REQUIRED_RESOLUTION_TABLE[0][i][0];
		if (tempEncoderConnectionId == encoderConnectionId)
		{
			ans = TRUE;
			encoderIndex = i;
			break;
		}
	}
	return ans;
}
///////////////////////////////////////////////////////////////////////////////////////
//
//	The decision func - should be the same as in Card Manager!
//
BYTE CDecoderResolutionTable::CalculateDecoderResolution (int nLayoutType, int nEncResRatio, int nDecResRatio, int nDecSizeInLayout)
{
	int unRes;
	CMedString cstr;
	BYTE ans;
	
	cstr << "CDecoderResolutionTable::CalculateDecoderResolution \n";
	
	if ((ERelativeSizeOfImageInLayout)nDecSizeInLayout == E_NOT_IN_LAYOUT)
	{
		cstr << "(ERelativeSizeOfImageInLayout)nDecSizeInLayout == E_NOT_IN_LAYOUT --> return (DECODER_DUMMY_RESOLUTION)\n";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		return (DECODER_DUMMY_RESOLUTION);
	}
	
	//we don't need SD streams for those places in layout.
	if (nEncResRatio == RESOLUTION_RATIO_16)//HD (Greater then SD)
	{
		if ( (nDecSizeInLayout == E_QUARTER_SCREEN) && ((nLayoutType == E_VIDEO_LAYOUT_1P5) || (nLayoutType == E_VIDEO_LAYOUT_3X3)) )
		{
			if (nDecResRatio > RESOLUTION_RATIO_1)
				nDecResRatio = RESOLUTION_RATIO_1;
		}
	}
	//end of patch
	//--------------------------
	unRes = (nEncResRatio * nDecSizeInLayout) >> 4;
	unRes = MIN_(unRes,(nDecResRatio));
	cstr << "unRes is: " << unRes << " Decoder resolution: ";
	if (unRes < 1)
	{
		cstr << "DECODER_1_16_RESOLUTION";
		ans = DECODER_1_16_RESOLUTION;
	}
	else if (unRes == 1)
	{
		cstr << "DECODER_1_4_RESOLUTION";
		ans = DECODER_1_4_RESOLUTION;
	}
	else if (unRes == 4)
	{
		cstr << "DECODER_FULL_RESOLUTION";
		ans = DECODER_FULL_RESOLUTION;	
	}
	/*else if (unRes == 16) //HD_patch
	{
		return (E_REQUIRED_DECODER_RESOLUTION_HD);	
	}
	*/
	else 
	{
		PASSERT(100);
		ans = DECODER_DUMMY_RESOLUTION;
	}
	
	PTRACE(eLevelInfoNormal,cstr.GetString());
	return ans;
	
}
////////////////////////////////////////////////////////////////
void CDecoderResolutionTable::DumpTable(BYTE onlyValidEntries,int numColumnsToPrint)
{
	CManDefinedString *pStr = new CManDefinedString(NUM_ROWS * NUM_COLUMNS * ONE_LINE_BUFFER_LEN);
	CMedString line;
	int numValidLines = 0;
	
	if (numColumnsToPrint > MAX_CIF_PARTIES_FOR_SYSTEM)
		numColumnsToPrint = MAX_CIF_PARTIES_FOR_SYSTEM;
	
	
	line << "+-----------+-----+";
	for (int lineLength = 1;lineLength <=numColumnsToPrint;lineLength++)
	{
		if (REQUIRED_RESOLUTION_TABLE[0][lineLength][0] == CELL_NA && onlyValidEntries)
			continue;
		line << "-----------+";
	}
	
	line << "----------------+\n";
	// Header
	*pStr << line;
	*pStr << "| DecoderId | Res |";
	for (int j=1;j<=numColumnsToPrint;j++)
	{
		if(REQUIRED_RESOLUTION_TABLE[0][j][0] != CELL_NA)
		{
			DWORD encoderId = REQUIRED_RESOLUTION_TABLE[0][j][0];
			if (encoderId < 10)
				*pStr << "     " << encoderId << "     |";
			else if(encoderId < 100)
				*pStr << "    " << encoderId << "     |";
			else if(encoderId < 1000)
				*pStr << "    " << encoderId << "    |";
			else if(encoderId < 10000)
				*pStr << "   " << encoderId << "    |";
			else if(encoderId < 100000)
				*pStr << "   " << encoderId << "   |";
			else if(encoderId < 1000000)
				*pStr << "  " << encoderId << "   |";
			else
				*pStr << "  " << encoderId << "  |";
		}
		else
			if (onlyValidEntries)
				continue;
			else
			{
				//*pStr << "     NA     |";
				if (j < 10)
				{
					*pStr << "    NA (" << j <<") |";
				}
				else
				{
					*pStr << "   NA (" << j <<") |";
				}
			}
	}
	*pStr << "     STATUS     |\n";
	*pStr << line;
	
	for (int i=1;i<NUM_ROWS;i++)
	{
		if(REQUIRED_RESOLUTION_TABLE[i][0][0] == CELL_NA && onlyValidEntries)
			continue;
			for (int k=0;k<TABLE_DEPTH;k++)
			{
				numValidLines++;
				for (int j=0;j<=numColumnsToPrint;j++)
				{
					
					// Column 0 - decoder Id
					if (j == 0)
					{
						if (k != 1)
						{
							*pStr << "|           |";
						}
						else
						{
							if(REQUIRED_RESOLUTION_TABLE[i][0][0] != CELL_NA)
							{
								DWORD conId = REQUIRED_RESOLUTION_TABLE[i][0][0];
								if (conId < 10)
									*pStr << "|     " << conId << "     |";
								else if(conId < 100)
									*pStr << "|    " << conId << "     |";
								else if(conId < 1000)
									*pStr << "|    " << conId << "    |";
								else if(conId < 10000)
									*pStr << "|   " << conId << "    |";
								else if(conId < 100000)
									*pStr << "|   " << conId << "   |";
								else if(conId < 1000000)
									*pStr << "|  " << conId << "   |";
								else
									*pStr << "|  " << conId << "  |";
							}
							else
							{
								//*pStr << "|    NA     |";
								if ( i < 10)
									*pStr << "|    NA (" << i <<") |";
								else
									*pStr << "|   NA (" << i <<") |";
							}
						
						}
						*pStr << "  " << k << "  |";
					}
					else
					{
						if(REQUIRED_RESOLUTION_TABLE[0][j][0] == CELL_NA && onlyValidEntries)
							continue;
						switch (REQUIRED_RESOLUTION_TABLE[i][j][k])
						{
							case(CELL_ON):
							{
								*pStr << "     ON    |";
								break;
							}
							case(CELL_OFF):
							{
								*pStr << "     OFF   |";
								break;
							}
							default:
							{
								*pStr << "     NA    |";
								break;
							}
						}//switch
					}//if
				}//for j(column)
				DWORD statusIndex = i*MAX_DECODER_RESOLUTIONS + k;
				*pStr <<	"      " << STATUS_VECTOR[statusIndex] << "         |\n";
			}//for k (depth)
		*pStr << line;
		if (numValidLines % 4 == 0)
		{
			numValidLines = 0;
			if(pStr->GetString())
				PTRACE(eLevelInfoNormal,pStr->GetString());
			else
				PASSERTMSG(1, "GetString() return NULL"); 
			pStr->Clear();
		}
			
				
	}//for i (row)
	
	if (numValidLines > 0)
	{
		if(pStr->GetString())
			PTRACE(eLevelInfoNormal,pStr->GetString());
		else
			PASSERTMSG(1, "GetString() return NULL"); 
	}
	
	POBJDELETE(pStr);
	
		
}
///////////////////////////////////////////////////////////////////////////////
void CDecoderResolutionTable::CheckConsistency()
{
	for (int i = 3; i < STATUS_VECTOR_LEN;i++)
	{
		DWORD resolution = i%3;
		DWORD rowIndex = i/3;
		DWORD tempStatus = CELL_NA;
		// check just non-empty rows (only active decoders)
		if (REQUIRED_RESOLUTION_TABLE[rowIndex][0][0] != CELL_NA)
		{	
			for (int j = 1;j < NUM_COLUMNS;j++)
			{
				if (REQUIRED_RESOLUTION_TABLE[rowIndex][j][resolution] != CELL_NA) {
					if (tempStatus == CELL_NA)
						tempStatus = REQUIRED_RESOLUTION_TABLE[rowIndex][j][resolution];
					else
						tempStatus |= REQUIRED_RESOLUTION_TABLE[rowIndex][j][resolution];
				}
			}
			// a new decoder that added to conf, all its cells are still empty, but its status should be 1 
			if (tempStatus == CELL_NA)
				tempStatus = 2;
		}
		
		DWORD connectionId = REQUIRED_RESOLUTION_TABLE[rowIndex][0][0];
		// the status of non active decoder should be 0 
		if (tempStatus == CELL_NA)
			tempStatus = 0;
			
		if (tempStatus == 2)
		{
			// can be a new decoder in conf, and then the status should be 1
			// or a decoder X that has been seen by encoder Y, and now encoder Y has disconnected, so the status is 0
			CSmallString cstr;
			cstr << "CDecoderResolutionTable::CheckConsistency - can not determine if DecoderId:" << connectionId << ", resolution:" << resolution << " is consistant - All row is NA";
			PTRACE (eLevelInfoNormal,cstr.GetString());
			continue;
		}
		if (tempStatus != STATUS_VECTOR[i])
		{
			 CMedString str;
			 str << "CDecoderResolutionTable::CheckConsistency FAILED!\n" << "Line " << rowIndex << " in the table is inconsistant - ";
			 str << "DecoderId:" << connectionId << ", resolution:" << resolution << " status is:" << tempStatus << " while in status vector it apears as: " << STATUS_VECTOR[i];
			 
			 PASSERTMSG(rowIndex,str.GetString());
			 
			 DumpTable();
		}
				 
	}
	
}
///////////////////////////////////////////////////////////////////////////////
void CDecoderResolutionTable::AddRsrcParamsToRsrcVector(CRsrcParams* pRsrcParams,DWORD index)
{
	//create local copy and insert to table
	CRsrcParams* m_pRsrcParams = new CRsrcParams(*pRsrcParams);
	DECODERS_RSRC_PARAMS_VECTOR[index] = m_pRsrcParams;
}
///////////////////////////////////////////////////////////////////////////////
void CDecoderResolutionTable::RemoveRsrcParamsFromRsrcVector(DWORD index)
{
	POBJDELETE(DECODERS_RSRC_PARAMS_VECTOR[index]);	
}
///////////////////////////////////////////////////////////////////////////////
