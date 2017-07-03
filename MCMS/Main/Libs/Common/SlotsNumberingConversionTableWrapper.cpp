// SlotsNumberingConversionTableWrapper.cpp: implementation of the CSlotsNumberingConversionTableWrapper class.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SlotsNumberingConversionTableWrapper.h"
#include <stdio.h>
#include "ObjString.h"
#include "TraceStream.h"



// ------------------------------------------------------------
CSlotsNumberingConversionTableWrapper::CSlotsNumberingConversionTableWrapper ()
{
	// first init (for ensuring that the basic slots numbering is ok even before the table arrives from ShelfMngr - VNGR-12409, VNGR-12456)
	m_struct.numOfBoardsInTable = MAX_NUM_OF_BOARDS;

	for (int i=0; i< MAX_NUM_OF_BOARDS; i++)
	{
		m_struct.conversionTable[i].boardId			= i;
		m_struct.conversionTable[i].subBoardId		= FIXED_CM_SUBBOARD_ID;
		m_struct.conversionTable[i].displayBoardId	= i;
	}
}


// ------------------------------------------------------------
CSlotsNumberingConversionTableWrapper::~CSlotsNumberingConversionTableWrapper ()
{
}


// ------------------------------------------------------------
/*
void  CSlotsNumberingConversionTableWrapper::Dump(ostream& msg)
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	int numOfBoardsInTable = (int)(m_struct.numOfBoardsInTable);

	msg << "\n\n"
		<< "SlotsNumberingConversionTableWrapper::Dump\n"
		<< "------------------------------------------\n";

	msg	<< "Num of boards in table: " << numOfBoardsInTable;

    if (MAX_NUM_OF_BOARDS >= numOfBoardsInTable)
    {
		for (int i=0; i<numOfBoardsInTable; i++)
		{
			msg	<< "\nBoardId "				<< m_struct.conversionTable[i].boardId
				<< ", SubBoardId "			<< m_struct.conversionTable[i].subBoardId
				<< " - displayBoardId: "	<< m_struct.conversionTable[i].displayBoardId;
		}
    } // end numOfBoardsInTable==ok
    
    else // illegal numOfBoardsInTable
    {
    	msg	<< "\nIllegal num of boards (must not be greater than " << MAX_NUM_OF_BOARDS << ")!";
    }
}
*/

// ------------------------------------------------------------
CSlotsNumberingConversionTableWrapper& CSlotsNumberingConversionTableWrapper::operator = (const CSlotsNumberingConversionTableWrapper &other)
{
	memcpy( &m_struct,
		    &(other.m_struct),
			sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S) );

	return *this;
}


// ------------------------------------------------------------
SLOTS_NUMBERING_CONVERSION_TABLE_S* CSlotsNumberingConversionTableWrapper::GetStruct()
{
	return &m_struct;
}


// ------------------------------------------------------------
void CSlotsNumberingConversionTableWrapper::SetStruct(const SLOTS_NUMBERING_CONVERSION_TABLE_S* pStruct)
{
	memcpy( &m_struct,
			pStruct,
			sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S) );
}


// ------------------------------------------------------------
DWORD CSlotsNumberingConversionTableWrapper::GetDisplayBoardId(const DWORD boardId, const DWORD subBoardId)
{
	DWORD retId = 0;

	for (int i=0; i< MAX_NUM_OF_BOARDS; i++)
	{
		if ( (boardId		== m_struct.conversionTable[i].boardId) &&
			 (subBoardId	== m_struct.conversionTable[i].subBoardId) )
		{
			retId = m_struct.conversionTable[i].displayBoardId;
			break;
		}
	}

	return retId;
}


// ------------------------------------------------------------
DWORD CSlotsNumberingConversionTableWrapper::GetBoardId(const DWORD displayBoardId, const DWORD subBoardId)
{
	DWORD retId = 0;

	for (int i=0; i< MAX_NUM_OF_BOARDS; i++)
	{
		if ( (displayBoardId	== m_struct.conversionTable[i].displayBoardId) &&
			 (subBoardId		== m_struct.conversionTable[i].subBoardId) )
		{
			retId = m_struct.conversionTable[i].boardId;
			break;
		}
	}

	return retId;
}

// ------------------------------------------------------------
void  CSlotsNumberingConversionTableWrapper::PrintData(const string theCaller)
{
	string retStr = "\nCSlotsNumberingConversionTableWrapper::PrintData (caller: ";
	retStr += theCaller;
	retStr += ")";
	retStr += "\n------------------------------------------------";
	
	int numOfBoardsInTable = (int)(m_struct.numOfBoardsInTable);

	char tmpBuf[ONE_LINE_BUFFER_LEN];
	memset(tmpBuf, 0, ONE_LINE_BUFFER_LEN);

	sprintf( tmpBuf, "\nNum of boards in table: %d", numOfBoardsInTable);
	retStr += tmpBuf;

	if (MAX_NUM_OF_BOARDS >= numOfBoardsInTable)
    {

    	for (int i=0; i<numOfBoardsInTable; i++)
		{
			memset(tmpBuf, 0, ONE_LINE_BUFFER_LEN);
			sprintf( tmpBuf,
					"\nBoardId %d, SubBoardId %d - displayBoardId: %d",
					m_struct.conversionTable[i].boardId,
					m_struct.conversionTable[i].subBoardId,
					m_struct.conversionTable[i].displayBoardId );

			retStr += tmpBuf;
		}
    	
    } // end numOfBoardsInTable==ok
    
    else // illegal numOfBoardsInTable
    {
		memset(tmpBuf, 0, ONE_LINE_BUFFER_LEN);
		sprintf(tmpBuf, "\nIllegal num of boards (must not be greater than %d)!", MAX_NUM_OF_BOARDS);
		retStr += tmpBuf;
    }
	
	TRACESTR(eLevelInfoNormal) <<  retStr.c_str();
}
