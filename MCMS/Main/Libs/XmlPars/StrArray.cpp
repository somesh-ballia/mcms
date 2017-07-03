// StrArray.cpp: implementation of the CStrArray class.
//
//////////////////////////////////////////////////////////////////////



#include "psosxml.h"
#include "zlibengn.h"
#include "string.h"

#define   STR_ARRAY_SIZE 100

using namespace std;


//////////////////////////////////////////////////////////////////////
CStrArray::CStrArray()
{
	m_pZlibEngine=NULL;
	m_strToZip= new char[COMMPRESS_BUFFER];
	Init();
}

//////////////////////////////////////////////////////////////////////
CStrArray::~CStrArray()
{
	if(m_pZlibEngine)
		m_pZlibEngine->DeflateCleanup();
	FreeAllocatedBuffers();
	if(m_pZlibEngine)
		delete m_pZlibEngine;
	delete [] m_strToZip;
}

//////////////////////////////////////////////////////////////////////
void CStrArray::Init()
{
	m_strArray=NULL;
	m_strArray = new char*[STR_ARRAY_SIZE];
	m_index=0;
	m_RowIndex=0;
	for(int i=0;i<STR_ARRAY_SIZE;i++)
		m_strArray[i]=NULL;
	m_nSize=STR_ARRAY_SIZE;
	m_strArray[m_RowIndex]= new char[DUMP_BUF_SIZE];
	m_bZip=FALSE;
	m_bZipWhenMoreThenOneBuffer=FALSE;
	m_ZipIndex=0;
	m_totalAllocatedLength=0;

}

//////////////////////////////////////////////////////////////////////
void CStrArray::Reset()
{
	if(m_strArray!=NULL)
	{
		for (DWORD i=1;i<=m_RowIndex;i++)
		{
			if(m_strArray[i]!=NULL)
			{
				delete [] m_strArray[i];
				m_strArray[i]=NULL;
			}
			else
				break;
		}

	}
	m_index=0;
	m_RowIndex=0;
	//if(m_pZlibEngine)
	//	m_pZlibEngine->DeflateReset();
	m_bZip=FALSE;
	m_bZipWhenMoreThenOneBuffer=FALSE;
	m_ZipIndex=0;
	m_totalAllocatedLength=0;

}

//////////////////////////////////////////////////////////////////////
WORD  CStrArray::GetCompressionLevel()
{
	if(m_bZip)
		return m_CompressionLevel;
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////
void CStrArray::SetCompressionLevel(WORD CompressionLevel)
{
	if(CompressionLevel!=FALSE&&m_pZlibEngine==NULL)
	{
		m_pZlibEngine=new ZlibEngine(this);
		m_pZlibEngine->DeflateInit(Z_BEST_SPEED);
	}
	m_CompressionLevel=CompressionLevel;
	if(m_CompressionLevel>0)
		m_bZipWhenMoreThenOneBuffer=TRUE;
}

//////////////////////////////////////////////////////////////////////
int CStrArray::Add(char *pString,int size)
{
	
	if(size==0)
		return SEC_OK;
	if(m_bZip&&m_pZlibEngine)
	{
		int availbleLength = (COMMPRESS_BUFFER)-m_ZipIndex;

		memcpy(&(m_strToZip[m_ZipIndex]),pString,min(availbleLength,size));

		if(availbleLength>size)
		{
			//if there's enough space in the current buffer
			m_ZipIndex=m_ZipIndex+size;
		}
		else
		{
			//copy the remaining length into next buffer
			m_ZipIndex=0;
			pString+=availbleLength;
			m_bZip=FALSE; //m_bZip was temporary set to false ,the deflate adds the string to the m_strArray
			//add error code - TBD
			m_pZlibEngine->Deflate(COMMPRESS_BUFFER,m_strToZip);
			m_bZip=TRUE;//m_bZip set back to the original value
			m_ZipIndex=size - availbleLength;
			if(m_ZipIndex)
				memcpy(&(m_strToZip[0]),pString,m_ZipIndex);

		}
	}
	else
	{
		int availbleLength = (DUMP_BUF_SIZE)-m_index;
		memcpy(&(m_strArray[m_RowIndex][m_index]),pString,min(availbleLength,size));
		if(availbleLength>=size)
		{
			//if there's enough space in the current buffer
			m_index=m_index+size;
			m_totalAllocatedLength+=size;
			if(m_totalAllocatedLength>=COMMPRESS_BUFFER&&m_bZipWhenMoreThenOneBuffer)
			{
				if(m_pZlibEngine)
					m_pZlibEngine->DeflateReset();
				int StringSize=m_totalAllocatedLength;
				//copy the remaining length into next buffer
				m_totalAllocatedLength=0;
				m_index=0;

				m_ZipIndex=0;
				pString+=availbleLength;
				m_bZip=FALSE; //m_bZip was temporary set to false ,the deflate adds the string to the m_strArray
				//add error code - TBD
				if(m_pZlibEngine)
					m_pZlibEngine->Deflate(StringSize,m_strArray[0]);
				m_bZip=TRUE;//m_bZip set back to the original value
				m_bZipWhenMoreThenOneBuffer=FALSE;
				
			}
		}
		else
		{
			if(!m_bZipWhenMoreThenOneBuffer)
			{
				m_index=0;
				m_RowIndex++;
				if(m_RowIndex==m_nSize)
				{
					char **pStrArray= new char*[m_nSize+STR_ARRAY_SIZE];


					for (DWORD i=0;i<m_nSize;i++)
					{
						pStrArray[i]=m_strArray[i];
					}
					delete []m_strArray;
					m_strArray=pStrArray;
					m_nSize=m_nSize+STR_ARRAY_SIZE;
					

				}
				m_strArray[m_RowIndex]= new char[DUMP_BUF_SIZE];
				pString+=availbleLength;
				m_index=size - availbleLength;
				memcpy(&(m_strArray[m_RowIndex][0]),pString,m_index);
				m_totalAllocatedLength+=size;
			}


		}
		
	}
	return SEC_OK;
}

//////////////////////////////////////////////////////////////////////
void CStrArray::FreeAllocatedBuffers()
{
	if(m_strArray!=NULL)
	{
		for (DWORD i=0;i<=m_RowIndex;i++)
		{
			if(m_strArray[i]!=NULL)
			{
				delete  [] m_strArray[i];
			}
			else
				break;
		}

		delete []m_strArray;

	}
}

//////////////////////////////////////////////////////////////////////
char* CStrArray::operator[] (int index) //no validity checking!!!!
{
	return m_strArray[index];
}

//////////////////////////////////////////////////////////////////////
void CStrArray::Finalize()
{
	if(m_bZip&&m_pZlibEngine)
	{
		m_bZip=FALSE; //m_bZip was temporary set to false ,the deflate adds the string to the m_strArray
		if(m_ZipIndex!=COMMPRESS_BUFFER)
		{
			//add ettor code - TBD
			m_pZlibEngine->Deflate(m_ZipIndex,m_strToZip);
		}
		m_pZlibEngine->DeflateEnd();
		m_bZip=TRUE;//m_bZip set back to the original value

	}
}

//////////////////////////////////////////////////////////////////////
int CStrArray::GetNumOfAllocatedBuffers() 
{
	return m_RowIndex+1;
}

