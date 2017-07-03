// StrArray.h: interface for the CStringArray class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _STR_ARRAY_
#define _STR_ARRAY_


#include "DataTypes.h"


class ZlibEngine;

class CStrArray  
{
public:
	CStrArray();
	virtual ~CStrArray();
	//int AllocateBuffers(DWORD strLength); //returns the length of the last buffer
	char* operator[] (int index); //no validity checking!!!!
	int GetNumOfAllocatedBuffers();
	DWORD GetTotalAllocatedLength() {return m_totalAllocatedLength;}
	//void SetTotalAllocatedLength(int totalAllocatedLength) {m_totalAllocatedLength=totalAllocatedLength;}
	int Add(char *pString,int size);
	void Reset();
	void Finalize();
	WORD  GetCompressionLevel();
	void SetCompressionLevel(WORD CompressionLevel);


protected:
	char**  m_strArray;
	DWORD	m_RowIndex;
	DWORD	m_totalAllocatedLength;
	DWORD   m_index;
	ZlibEngine *m_pZlibEngine;
	int		m_bZip;
	int		m_bZipWhenMoreThenOneBuffer;
	char*	m_strToZip;
	DWORD   m_ZipIndex;
	WORD	m_CompressionLevel;
	DWORD   m_nSize;
	void FreeAllocatedBuffers();
	void Init();
};

#endif // !defined(AFX_STRARRAY_H__F0597FAC_665D_4A7B_A0AA_49598C554E6D__INCLUDED_)
