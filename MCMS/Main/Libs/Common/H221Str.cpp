// CH221Str.cpp: implementation of the CH221Str class.
//
//////////////////////////////////////////////////////////////////////

#include <ostream>
#include <istream>

#include "H221Str.h"
#include "Macros.h"
#include "StatusesGeneral.h"

using namespace std;


//extern void DumpH323Cap(std::ostream& ostr, WORD len,BYTE* h323CapArray);

/////////////////////////////////////////////////////////////////////////////
// CLASS CH221Str
/////////////////////////////////////////////////////////////////////////////

CH221Str::CH221Str()       // constructor
{                                         
	m_size = 0;
	m_pStr = NULL;
	m_bDump=TRUE;
}


/////////////////////////////////////////////////////////////////////////////
CH221Str::CH221Str(const CH221Str &other):CPObject(other)          // copy constructor
{                                         
	m_size = other.m_size;  
	
	if ( m_size == 0 )
		m_pStr = NULL;  
	else {
		m_pStr = new BYTE[m_size];
    	
		if(m_pStr)
    		memcpy(m_pStr,other.m_pStr,m_size);  
		else
			PASSERT(1);
    }
	m_bDump=other.m_bDump;
	
}
	  
	  
/////////////////////////////////////////////////////////////////////////////
CH221Str::~CH221Str()      // destructor
{ 
	if ( m_pStr )  {
    delete [] m_pStr;
    m_pStr = NULL;
  }      
}

/////////////////////////////////////////////////////////////////////////////                   
void  CH221Str::Dump(std::ostream  &ostr)
{
    DumpHex(ostr);
}
/////////////////////////////////////////////////////////////////////////////                   
/////////////////////////////////////////////////////////////////////////////                   
DWORD  CH221Str::GetLen() const  
{    
	return  m_size;
}
/////////////////////////////////////////////////////////////////////////////                   
BYTE*  CH221Str::GetPtr() const 
{
    return m_pStr;   
}
/////////////////////////////////////////////////////////////////////////////                      
CH221Str&  CH221Str::operator=(const CH221Str &other)
{
	if ( &other == this ) return *this;
	if ( m_pStr != NULL ) PDELETEA(m_pStr);
					  
	m_size        = other.m_size;
	
	m_pStr = new BYTE[m_size];  
	memcpy(m_pStr,other.m_pStr,m_size);
	m_bDump=other.m_bDump;
	return *this;                                     
}

/////////////////////////////////////////////////////////////////////////////
bool CH221Str::operator==(const CH221Str &other)
{
	if(&other == this)
	{
		return true;
	}

	bool res = (m_size 	== other.m_size		&& 
				m_bDump == other.m_bDump 	&&
				0 == memcmp(m_pStr, m_pStr, m_size));
	return res;
}

/////////////////////////////////////////////////////////////////////////////
void CH221Str::DumpHex(std::ostream  &ostr) {
    
  BYTE cap;
  WORD i=0,j=0;

      ostr << "H221 STRING HEX DUMP :\n";
      ostr << "----------------------\n";
      
      while ( i < m_size )  {
        cap = m_pStr[i++];
        ostr << (hex) << cap << "  ";
        if ( j > 7) {
            ostr << "\n";
            j = 0;
        }
        else
            j++;
  }
}

/////////////////////////////////////////////////////////////////////////////
void CH221Str::Serialize(WORD format, std::ostream  &m_ostr)
{
  // assuming format = OPERATOR_MCMS
  int i;
  m_ostr <<  m_size << "\n";
  for (i=0; i<m_size; i++)
  {
  	// should be WORD, BYTE 0 == '\0'.
  	WORD tmp = m_pStr[i];
    m_ostr <<  tmp << "\n";
  }
}

void CH221Str::SetDump(BYTE bDump)
{
	m_bDump=bDump;
}

BYTE CH221Str::IsDump()
{
	return m_bDump;
}

/////////////////////////////////////////////////////////////////////////////
void CH221Str::Serialize(WORD format, std::ostream  &m_ostr, BYTE billing)
{
  // assuming format = OPERATOR_MCMS
  int i;
  m_ostr <<  m_size << ",";
  for (i=0; i<m_size; i++)
  {
  	// should be WORD, BYTE 0 == '\0'.
  	WORD tmp = m_pStr[i];
    m_ostr <<  tmp << ",";
  }
}
/////////////////////////////////////////////////////////////////////////////
void CH221Str::Serialize(WORD format, std::ostream  &m_ostr, WORD fullformat)
{
  // assuming format = OPERATOR_MCMS
  int i;
 // m_ostr <<  m_size << ",";
  for (i=0; i<m_size; i++)
  {
  	// should be WORD, BYTE 0 == '\0'.
  	WORD tmp = m_pStr[i];
    m_ostr <<"remote comm mode:"<<  tmp << ",";
  }
}

/////////////////////////////////////////////////////////////////////////////
void CH221Str::DeSerialize(WORD format, std::istream  &m_istr)
{
  int i;
  m_istr >> m_size;            

  m_pStr = new BYTE[m_size];
  for (i=0; i<m_size; i++) 
  {
  	WORD tmp = 0;
    m_istr >> tmp;
    m_pStr[i] = (BYTE)tmp;
  }
}
/////////////////////////////////////////////////////////////////////////////
void CH221Str::DeSerialize(WORD format, std::istream  &m_istr, BYTE bilflag)
{
  int i;
  m_istr >> m_size;  
  m_istr.ignore(1);
  m_pStr = new BYTE[m_size];
  for (i = 0; i < m_size; i++) 
  {
  		WORD tmp = 0;
    	m_istr >> tmp;
    	m_pStr[i] = (BYTE)tmp;
    
    	m_istr.ignore(1);
  }
}

/////////////////////////////////////////////////////////////////////////////
void CH221Str::SetH221FromString(const DWORD strLen,const char* pszCH221String)
{
	int nStatus = STATUS_OK;

	PDELETEA(m_pStr);

	m_size = strLen + 1;
	m_pStr = new BYTE[m_size];
	memset(m_pStr,'\0',m_size);
	memcpy(m_pStr,pszCH221String,m_size-1);
}




