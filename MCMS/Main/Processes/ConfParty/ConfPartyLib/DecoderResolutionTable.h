#ifndef DECODERRESOLUTIONTABLE_H_
#define DECODERRESOLUTIONTABLE_H_

#include "PObject.h"
#include "VideoStructs.h"
#include "VideoApiDefinitions.h"


#define MAX_SD30_PARTIES_FOR_SYSTEM		20
#define MAX_CIF_PARTIES_FOR_SYSTEM		80
#define MAX_DECODER_RESOLUTIONS			3 //qcif, cif, sd

#define DECODER_1_16_RESOLUTION			0	//up to qcif	
#define DECODER_1_4_RESOLUTION			1	//up to cif
#define DECODER_FULL_RESOLUTION			2	//up to SD

#define DECODER_DUMMY_RESOLUTION		0xFF



const WORD NUM_ROWS = MAX_SD30_PARTIES_FOR_SYSTEM + 1;
const WORD NUM_COLUMNS = MAX_CIF_PARTIES_FOR_SYSTEM + 1;
const WORD TABLE_DEPTH = MAX_DECODER_RESOLUTIONS;
const WORD STATUS_VECTOR_LEN = MAX_DECODER_RESOLUTIONS * NUM_ROWS;


class CRsrcParams;

class CDecoderResolutionTable : public CPObject
{
	CLASS_TYPE_1(CDecoderResolutionTable, CPObject )
public:
	CDecoderResolutionTable();
	~CDecoderResolutionTable();
	virtual const char*  NameOf()const{ return "CDecoderResolutionTable";}

	void AddDecoderToTable(CRsrcParams* pRsrcParams);
	void AddEncoderToTable(DWORD encoderConnectionId);
	void RemoveDecoderFromTable(DWORD decoderConnectionId);
	void RemoveEncoderFromTable(DWORD encoderConnectionID);
	void UpdateTable(const CHANGE_LAYOUT_S& changeLayoutReq,DWORD encoderConnectionId);
	
	
	void DumpTable(BYTE onlyValidEntries = FALSE,int numColumnsToPrint = NUM_COLUMNS);
	void CheckConsistency();
	
	void SetUseDecoderTableTrueFalse (BYTE useDecoderTable) {m_useDecoderTable = useDecoderTable;}
	BYTE GetUseDecoderTableTrueFalse () {return m_useDecoderTable;}
	
private:
	DWORD REQUIRED_RESOLUTION_TABLE [NUM_ROWS][NUM_COLUMNS][TABLE_DEPTH];
	BYTE  STATUS_VECTOR[STATUS_VECTOR_LEN];
	CRsrcParams* DECODERS_RSRC_PARAMS_VECTOR[NUM_ROWS];
	                                 
	BYTE UpdateStatusIfNeeded(DWORD DecoderIndex);
	BYTE UpdateRowStatus(DWORD index,DWORD resolution);
	void CreateHardwareInterfaceFillStructAndSendUpdateDecoder(DWORD decoderIndex);
	BYTE FindDecoderInTable(DWORD decoderConnectionId,int& dencoderIndex);
	BYTE FindEncoderInTable(DWORD encoderConnectionId,int& encoderIndex);
	BYTE RemoveRow(DWORD index);
	BYTE RemoveColumn(DWORD index);
	BYTE IsDecoderInLayout(DWORD decoderConnectionId, const IMAGE_PARAM_S* imagesVector,int& imageIndex);
	void FillResolutionForEncoderDecoder(DWORD decoderIndex,DWORD encoderIndex,BYTE requiredResolution = 0xFF);
	BYTE CalculateDecoderResolution (int nLayoutType, int nEncResRatio, int nDecResRatio, int nDecSizeInLayout);
	void AddRsrcParamsToRsrcVector(CRsrcParams* pRsrcParams,DWORD index);
	void RemoveRsrcParamsFromRsrcVector(DWORD index);
	
	
	int numEncoders;
	BYTE m_useDecoderTable;

	/*std::map<DWORD, CRsrcParams*> DecoderIndexMap;
	std::map<DWORD, DWORD> EncoderIndexMap;
	*/
	
};
#endif /*DECODERRESOLUTIONTABLE_H_*/
