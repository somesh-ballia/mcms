//+========================================================================+
//							 ObjString.cpp                                 |
//			Copyright 1995 Pictel Technologies Ltd.                        |
//				   All Rights Reserved.                                    |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:	   ObjString.cpp                                               |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date	   | Description                                           |
//-------------------------------------------------------------------------|
//	 |			|													       |
//+========================================================================+
#include <stdio.h>
#include <string.h>
#include <ostream>
#include <istream>
#include <iostream>
//#include <map>
//#include <algorithm>
using namespace std;


#include "ObjString.h"
#include "SharedDefines.h"
#include "Macros.h"
#include "NStream.h"
#include "StructTm.h"

/*unsigned maxFormat = 3;
class INT
{
public:
	INT(): _i(0) {}
	INT(unsigned i): _i(i) {}
	operator unsigned()
	{
		return _i;
	}

private:
	unsigned _i;
};

typedef map<unsigned, INT> tAllocStats;
typedef pair<const unsigned, INT> tAllocPair;
typedef tAllocStats::iterator tAllocIter;
tAllocStats allocStats;

static void print(tAllocPair displayPair)
{
	printf("\nCOBJSTRING: SIZE %u COUNT %u", displayPair.first, (unsigned) displayPair.second);
}

void* operator new(size_t size, char, char)
{
//	int			percentage;
//	static 	clock_t initial = clock();
//	static 	double	commulative = 0;
//	commulative += current;
//	percentage = (int) ((commulative*100.0) / ((double) (clock() - initial)));
//	unsigned comm = (int) commulative;
//	unsigned total =  clock() - initial;
//	printf("\nCurrent %u size %u commulative %u initial %u clock() %u total %u percentage %u", current, size, comm, initial,
//			clock(), total, percentage);

	static 	int newCount = 0;
	//	clock_t 	current = clock();
	void*		allocPtr = malloc(size);
	//	current = clock() - current;

	allocStats[size] = allocStats[size] + 1;

	if (!(++newCount % 1000))
	{
		printf("\n**************************************************************");
		printf("\n**************************************************************");
//		printf("\n Current running percentage is %d", percentage);
		printf("\n Max format is %u", maxFormat);
		for_each(allocStats.begin(), allocStats.end(), print);
		printf("\n\n");
	}

	return allocPtr;
}
void* operator new[](size_t size, char, char)
{
	return operator new(size,0,0);
}
*/
// constructor
//////////////////////////////////////////////////////////////////////////////////
void CObjString::Init(const char* str)
{
	//========================================
	// Initial string content initialization
	//========================================
	memcpy(m_pFormat, "%d", 2);
	m_pFormat[2]='\0';
	if(str==NULL || str[0] == '\0')  // when using operator = to NULL
	{
		//===============
		// Empty string
		//===============
		m_pString[0]='\0';
		m_stringLength = 0;

	}
	else
	{
		//========================
		// Copying source string
		//========================
		strncpy(m_pString, str, m_allocatedBufflength - 1);
		m_pString[m_allocatedBufflength - 1] = '\0';
		m_stringLength = strlen(m_pString);
	}
}

CObjString::CObjString(const char * const str, DWORD allocate_size, BOOL IsDynamic): m_bIsDynamic(IsDynamic)
{
	m_stringLength = 0;
	m_allocatedBufflength = 0;	
	m_pFormat = NULL;
	m_pString = NULL;

	//==============================
	// Work buffers initialization
	//==============================
	if (IsDynamic)
	{
		//==============================
		// Determining allocation size
		//==============================
		if (allocate_size == 0 && str) 	m_allocatedBufflength = strlen(str) + 1;
		else							m_allocatedBufflength = allocate_size;

		//====================
		// Allocating string
		//====================
		m_pString = new char[m_allocatedBufflength];
		m_pFormat = new char[3];

		//========================================
		// Initial string content initialization
		//========================================
		Init(str);
	}
	// else will be handled by derivatives
}

template <WORD MAX_LEN>
CStaticString<MAX_LEN>::CStaticString(const char * const str): CObjString(str, sizeof(m_string), NO)
{
	//========================================
	// Connecting pointers to static buffers
	//========================================
	m_allocatedBufflength = sizeof(m_string);
	m_pString = m_string;
	m_pFormat = m_format;

	//========================================
	// Initial string content initialization
	//========================================
	Init(str);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::Init(char ch)
{
	//======================
	// Initializing string
	//======================
	memset(m_pString, ch, m_stringLength);
	m_pString[m_stringLength] = '\0';
	strncpy(m_pFormat, "%d", 2);
	m_pFormat[2]='\0';

}

CObjString::CObjString(char ch, WORD nRepeat, BOOL IsDynamic): m_bIsDynamic(IsDynamic)
{
	m_stringLength = 0;
	m_allocatedBufflength = 0;	
	m_pFormat = NULL;
	m_pString = NULL;

	//==============================
	// Work buffers initialization
	//==============================
	if (IsDynamic)
	{
		m_stringLength = nRepeat;
		m_allocatedBufflength = nRepeat+1;
		m_pString = new char[m_allocatedBufflength];
		m_pFormat = new char [3];

		//======================
		// Initializing string
		//======================
		Init(ch);
	}
	// else will be handled by derivatives
}

template<WORD MAX_LEN>
CStaticString<MAX_LEN>::CStaticString(char ch, WORD nRepeat): CObjString(ch, nRepeat, NO)
{
	//==============================
	// Work buffers initialization
	//==============================
	m_pString = m_string;
	m_allocatedBufflength = sizeof(m_string);
	m_stringLength = min((DWORD) nRepeat, m_allocatedBufflength - 1);
	m_pFormat = m_format;

	//======================
	// Initializing string
	//======================
	Init(ch);
}


// copy constructor
//////////////////////////////////////////////////////////////////////////////////
void CObjString::Init(const CObjString& s1, const WORD formatLen)
{
	//================================
	// Initializing string  & format
	//================================
	strncpy(m_pString, s1.m_pString, m_stringLength);
	m_pString[m_stringLength] = '\0';
	if(s1.m_pFormat!=NULL)
	{
		strncpy(m_pFormat,s1.m_pFormat,formatLen);
		m_pFormat[formatLen]='\0';
	}
}

CObjString::CObjString(const CObjString & s1, BOOL IsDynamic): CPObject(s1), m_bIsDynamic(IsDynamic)
{
	m_stringLength = 0;
	m_allocatedBufflength = 0;	
	m_pFormat = NULL;
	m_pString = NULL;

	//==============================
	// Work buffers initialization
	//==============================
	if (IsDynamic)
	{
		WORD formatLen = 0;
		//			if (formatLen >= maxFormat) maxFormat = formatLen + 1;

		//=====================
		// String buffer init
		//=====================
		m_allocatedBufflength = s1.GetAllocLength();
	    m_pString = new char[m_allocatedBufflength];
		m_stringLength = s1.GetStringLength();

		//=====================
		// Format buffer init
		//=====================
		if(s1.m_pFormat!=NULL)
		{
			formatLen = strlen(s1.m_pFormat);  // after testing can be limited to "min((size_t) MAX_FORMAT - 1, strlen(s1.m_pFormat));"
			m_pFormat = new char[formatLen+1];
		}

		//================================
		// Initializing string  & format
		//================================
		Init(s1, formatLen);
	}
	// else will be handled by derivatives
}


template <WORD MAX_LEN>
CStaticString<MAX_LEN>::CStaticString(const CObjString & s1): CObjString(s1, NO)
{
	const WORD formatLen = min((size_t)MAX_FORMAT - 1, strlen(s1.m_pFormat));

	//=====================
	// String buffer init
	//=====================
	m_pString = m_string;
	m_allocatedBufflength = sizeof(m_string);
	m_stringLength = min((DWORD)s1.GetStringLength(), m_allocatedBufflength - 1);

	//=====================
	// Format buffer init
	//=====================
	m_pFormat = m_format;

	//================================
	// Initializing string  & format
	//================================
	Init(s1, formatLen);
}


// destructor
//////////////////////////////////////////////////////////////////////////////////
CObjString::~CObjString()
{
	if (IsDynamic())
	{
		PDELETEA(m_pString);
		PDELETEA(m_pFormat);
	}
}

///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Serialize/Deserialize //////////////////////////
void CObjString::Serialize(WORD format, std::ostream &m_ostr)
{
  m_ostr << m_stringLength    << "\n";
  m_ostr << m_pString         << "\n";
  m_ostr << strlen(m_pFormat) << "\n";
  m_ostr << m_pFormat         << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CObjString::DeSerialize(WORD format, std::istream& m_istr)
{
  DWORD temp_stringLength = 0;
  m_istr >> temp_stringLength;
  if (m_allocatedBufflength - 1 > temp_stringLength)//= is ok, since the size is always m_allocatedBufflength for '/0'
  {
    m_stringLength = temp_stringLength;
    m_istr.ignore(1);
    m_istr.getline(m_pString, m_stringLength+1, '\n');  //VNGFE-5462, DK

    WORD temp_formatLength = 0;
    m_istr >> temp_formatLength;
    if (IsDynamic())
    {
      PDELETEA(m_pFormat);
      m_pFormat = new char[temp_formatLength + 1]; // allocate temp_formatLength+1 for '\0'
    }
    else
    {
      temp_formatLength = min(temp_formatLength, (WORD) (MAX_FORMAT - 2));
    }
    m_istr.ignore(1);
    m_istr.getline(m_pFormat, temp_formatLength+1, '\n');

  }
}

//////////////////////////////////////////////////////////////////////////////////
int CObjString::Find(const char* str) const
{
    char *pStr = strstr(m_pString,str);
    if (pStr == NULL)
        return 0;
    if (pStr[0]=='\0')
        return 0;
    return 1;
}
//////////////////////////////////////////////////////////////////////////////////
bool CObjString::IsContainsSpecialChars(const char* str)
{
	if (str==NULL)
		return false;

	/* too slow
	CObjString strTemp(str);
	if (strTemp.Find("<") ||strTemp.Find(">") || strTemp.Find("\'")
		|| strTemp.Find("\"") || strTemp.Find("&") )
		return true;
	*/
	while (*str)
	{
		if ( '<' == *str || '>' == *str || '\"' == *str || '\'' == *str || '&' == *str )
			return true;
		++str;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////
BYTE CObjString::IsEmpty() const
{
	return (!m_pString || m_pString[0] == 0);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::Clear()
{
	m_pString[0] = '\0';
	m_stringLength = 0;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator = ///////////////////////////////////
const CObjString& CObjString::operator=(const CObjString & s1)
{
	if (this == &s1)
		return *this;

    CObjString::operator=(s1.GetString());

	if(s1.m_pFormat!=NULL)
	{
		DWORD len;
		if (IsDynamic())
		{
			len = strlen(s1.m_pFormat);
		}
		else
		{
			len = min(strlen(s1.m_pFormat), (size_t)MAX_FORMAT - 1);
		}

		if(m_pFormat)
		{
			if(strncmp(m_pFormat, s1.m_pFormat, len + 1))  // +1 for comparison of the null, otherwise one string may be the subset of the other, len is validated above to be small enough
			{
				if (IsDynamic() && (!m_pFormat || len != strlen(m_pFormat)))
				{
					if (m_pFormat) delete [] m_pFormat;
					m_pFormat = new char[len+1];
				}
				strncpy(m_pFormat,s1.m_pFormat,len);
				m_pFormat[len]='\0';
			}
		}
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
const CObjString& CObjString::operator=(char ch)
{
	if(2>m_allocatedBufflength)
		return *this;
	m_pString[0] = ch;
	m_pString[1] = '\0';
	m_stringLength = 1;
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
const CObjString& CObjString::operator=(const char *str)
{
	if (m_pString==str)
		return *this;

    strncpy(m_pString, str, m_allocatedBufflength - 1);
    m_pString[m_allocatedBufflength - 1] = '\0';
    m_stringLength = strlen(m_pString);


// 	if(strlen(str)>m_allocatedBufflength)
// 		return *this;
// 	strncpy(m_pString,str, strlen(str));
// 	m_pString[strlen(str)]='\0';
// 	m_stringLength = strlen(str);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator [] ///////////////////////////////////
char & CObjString::operator[](DWORD offset)
{
	if (offset >= m_allocatedBufflength)
		return m_pString[m_allocatedBufflength-1];
	else
	{
		if(m_stringLength<=offset)
		{
			for(DWORD i=m_stringLength;i<offset;i++)
				m_pString[i]=' ';
			m_stringLength=offset+1;
			m_pString[m_stringLength]='\0';
		}
		return m_pString[offset];
	}
}

//////////////////////////////////////////////////////////////////////////////////
char CObjString::operator[](DWORD offset) const
{
	if (offset > m_allocatedBufflength)
		return m_pString[m_allocatedBufflength-1];
    else
        return m_pString[offset];
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator + ///////////////////////////////////
void CObjString::concat(const CObjString & added_str)
{
	DWORD len = added_str.GetStringLength();
	if(len)
	{
		if(m_stringLength + len >= m_allocatedBufflength)
			return;
		memcpy(m_pString+m_stringLength ,added_str.GetString(), len);
		m_stringLength=m_stringLength + len;
		m_pString[m_stringLength]='\0';
	}
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::concat(const char* str, DWORD len)
{
	if(len)
	{
		if(m_stringLength + len >= m_allocatedBufflength)
			return;
		memcpy(m_pString+m_stringLength ,str, len);
		m_stringLength += len;
		m_pString[m_stringLength]='\0';
	}
}

//////////////////////////////////////////////////////////////////////////////////
CObjString operator+( const CObjString& s1, const CObjString& s2 )
{
	CObjString temp(s1.GetString(), s1.GetStringLength() + s2.GetStringLength());
	temp.concat(s2);
    return temp;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString operator+( const CObjString & s1, char *new_str)
{
	CObjString temp(s1.GetString(), (s1.GetStringLength() + strlen(new_str)));
	temp.concat(new_str, strlen(new_str));
    return temp;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString operator+( char *new_str,  const CObjString & s2)
{
	CObjString temp(new_str, strlen(new_str) + s2.GetStringLength());
	temp.concat(s2);
    return temp;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString operator+( const CObjString & s1, char ch)
{
	CObjString temp(s1.GetString(),s1.GetStringLength()+1);
	temp.concat(ch);//, 1);
    return temp;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString operator+(char ch,  const CObjString & s2)
{
	CObjString temp(ch,s2.GetStringLength()+1);
	temp.concat(s2);
    return temp;
}


//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator += ///////////////////////////////////
const CObjString& CObjString::operator+=(const CObjString& str)
{
	this->concat(str);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
const CObjString& CObjString::operator+=(char ch)
{
	this->concat(&ch, 1);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
const CObjString& CObjString::operator+=(const char *str)
{
	this->concat(str, strlen(str));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator << ///////////////////////////////////
CObjString& CObjString::operator<<(const CObjString& str)
{
	this->concat(str);
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(const CStructTm& time)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
	time.DumpToBuffer(temp);
	this->concat(temp, strlen(temp));
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(char ch)
{
	this->concat(&ch, 1);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(const char *str)
{
	if(str)
	{
		DWORD len = strlen(str);

		this->concat(str, len);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(BYTE bt)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
	sprintf(temp, m_pFormat,bt);
	this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(int num)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
	sprintf(temp, m_pFormat,num);
	this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(WORD wd)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
	sprintf(temp, m_pFormat,wd);
	this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(DWORD dw)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	memset(&temp,'\0',MAX_NUMBER_OF_DIGITS_IN_TYPES);
	sprintf(temp,"%u",dw);
	this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(long lg)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
	sprintf(temp, m_pFormat,lg);
	this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(short sh)
{
	char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
	sprintf(temp,m_pFormat,sh);
	this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(ULONGLONG ull)
{
    char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
    snprintf(temp,MAX_NUMBER_OF_DIGITS_IN_TYPES, "%qd", ull);
    this->concat(temp, strlen(temp));
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(float fl)
{
    char temp[MAX_NUMBER_OF_DIGITS_IN_TYPES];
	temp[0]='\0';
    snprintf(temp,MAX_NUMBER_OF_DIGITS_IN_TYPES, "%.3f", fl);
    this->concat(temp, strlen(temp));
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(const std::string & stdString)
{
    this->operator<<(stdString.c_str());
    return *this;
}
//////////////////////////////////////////////////////////////////////////////////
CObjString& CObjString::operator<<(const CBaseWrapper& basewrapper)
{
	 COstrStream ostr;

	 basewrapper.Dump(ostr);
	 this->operator<<(ostr.str().c_str());

	 return *this;
}
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator == ///////////////////////////////////
BYTE operator==(const CObjString& s1, const CObjString& s2)
{
	return (strcmp(s1.GetString(), s2.GetString())==0);
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator==(const CObjString& s1, const char *str)
{
	if(str==NULL)
	{
		if(s1.GetStringLength()==0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return (strcmp(s1.GetString(), str)==0);
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator ==( const char *str, const CObjString& s2)
{
	return s2==str;
}


//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator != ///////////////////////////////////
BYTE operator !=(const CObjString& s1, const CObjString& s2)
{
	return(!(s1==s2));
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator !=(const CObjString& s1, const char *str)
{
	return(!(s1==str));
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator !=(const char *str, const CObjString& s2)
{
	return(!(str==s2));
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator < ///////////////////////////////////
BYTE operator <(const CObjString& s1, const CObjString& s2)
{
	return (strcmp(s1.GetString(), s2.GetString())<0);
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator <(const CObjString& s1, const char *str)
{
	if(str==NULL)
		return FALSE;

	return (strcmp(s1.GetString(), str)<0);
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator <( const char *str, const CObjString& s2)
{
	return ((!(s2<str))&&(!(s2==str)));
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator > ///////////////////////////////////
BYTE operator >(const CObjString& s1, const CObjString& s2)
{
	return (strcmp(s1.GetString(), s2.GetString())>0);
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator >(const CObjString& s1, const char *str)
{
	return (str<s1);
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator >( const char *str, const CObjString& s2)
{
	if(str==NULL)
		return FALSE;

	return (strcmp(str, s2.GetString())>0);
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator <= ///////////////////////////////////
BYTE operator <=(const CObjString& s1, const CObjString& s2)
{
	return (!(s1.GetString()>s2.GetString()));
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator <=(const CObjString& s1, const char *str)
{
	return (!(s1.GetString()>str));
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator <=( const char *str, const CObjString& s2)
{
	return (!(str>s2.GetString()));
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// operator >= ///////////////////////////////////
BYTE operator >=(const CObjString& s1, const CObjString& s2)
{
	return (!(s1.GetString()<s2.GetString()));
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator >=(const CObjString& s1, const char *str)
{
	return (!(s1.GetString()<str));
}

//////////////////////////////////////////////////////////////////////////////////
BYTE operator >=( const char *str, const CObjString& s2)
{
	return (!(str<s2.GetString()));
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::SetFormat(const char *str)
{
	if(str && str[0]=='%')
	{
		DWORD len = strlen(str);
		//			if (len >= maxFormat) maxFormat = len + 1;
		if(len != strlen(m_pFormat))
		{
			if (IsDynamic())
			{
				delete [] m_pFormat;
				m_pFormat = new char[len+1];
			}
			else if (len >= MAX_FORMAT)
			{
				len = MAX_FORMAT - 1;
			}
		}
		memcpy(m_pFormat,str,len);
		m_pFormat[len]='\0';
	}
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ToUpper()
{
    ToUpper(m_pString);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ToLower()
{
    ToLower(m_pString);
}

//////////////////////////////////////////////////////////////////////////////////
bool CObjString::IsNumeric()
{
    return IsNumeric(m_pString);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ReplaceChar(const char from, const char to)
{
    ReplaceChar(m_pString, from, to);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ReplaceChar(bool fromCond(const char ch), const char to)
{
    ReplaceChar(m_pString, fromCond, to);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::Reverse()
{
    return Reverse(m_pString);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::RemoveChars(const char *toRemove)
{
    RemoveChars(m_pString, toRemove);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ForceAppendStr(const char *strToAppend)
{
	ForceAppendStr(m_pString, strToAppend);
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ReplaceChar(char *strSource, const char from, const char to)
{
	while (*strSource)
	{
		if (from == *strSource)
			*strSource = to;
		++strSource;
	}
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ReplaceByte(unsigned char *buff, int buffLen, unsigned char from, unsigned char to)
{
    for(int i = 0 ; i < buffLen ; i++)
    {
        if(from == buff[i])
        {
            buff[i] = to;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ReplaceChar(char *strSource, bool fromCond(const char ch), const char to)
{
	while (*strSource)
	{
		if (fromCond(*strSource))
			*strSource = to;
		++strSource;
	}
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ToUpper(char *str)
{
    int strLen = strlen(str);
    int delta = ('a' - 'A');
    for(int i = 0 ; i < strLen ; i++)
    {
        if('a' <= str[i] && str[i] <= 'z')
        {
            str[i] -= delta;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ToLower(char *str)
{
    int strLen = strlen(str);
    int delta = ('a' - 'A');
    for(int i = 0 ; i < strLen ; i++)
    {
        if('A' <= str[i] && str[i] <= 'Z')
        {
            str[i] += delta;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
bool CObjString::IsNumeric(const char *str)
{
	if(NULL == str)
	{
		return false;
	}

  	bool result = false;
    int i = 0;

    // negative numbers are still numbers : -1
    if('-' == str[0])
    {
        i++;
    }

	for(; '\0' != str[i] ; i++)
	{
		result = (0 != isdigit(str[i]));
		if(false == result)
		{
			break;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::Reverse(char *strSource)
{
    const int bufferLen = strlen(strSource);
    const int halfBufferLen = bufferLen / 2;
    for(int i = 0 ; i < halfBufferLen ; i++)
    {
        const int j = bufferLen - i - 1;

        char tmp = strSource[i];
        strSource[i] = strSource[j];
        strSource[j] = tmp;
    }
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::RemoveChars(char *strSource, const char *toRemove)
{
    char *ptrToRemove = strstr(strSource, toRemove);
    while(NULL != ptrToRemove)
    {
        const char *ptrAfterRemove = ptrToRemove + strlen(toRemove);
        if('\0' == *ptrAfterRemove)
        {
            *ptrToRemove = '\0';
            return;
        }

        int strLen = strlen(ptrAfterRemove) + 1;
        for(int i = 0 ; i < strLen; i++)
        {
            ptrToRemove[i] = ptrAfterRemove[i];
        }

        ptrToRemove = strstr(ptrToRemove, toRemove);
    }
}

//////////////////////////////////////////////////////////////////////////////////
void CObjString::ForceAppendStr(char *strSource, const char *strToAppend)
// Appending strToAppend to strSource.
//    If the appended string is too long, then strSource is shortened, so the appended string will be added anyway.
{
	DWORD lenToAppend = strlen(strToAppend);

	if ( (TRUE == IsEmpty()) || (lenToAppend >= m_allocatedBufflength) )	// no need or impossible to append
	{
		return;
	}

	else if (m_stringLength+lenToAppend < m_allocatedBufflength)			// no problem to append
	{
		memcpy(m_pString+m_stringLength, strToAppend, lenToAppend);
		m_stringLength = m_stringLength + lenToAppend;
		m_pString[m_stringLength] = '\0';
	}
	
	else																	// strSource should be shortened
	{
		int whereToStart = m_allocatedBufflength - lenToAppend - 1;
		memcpy(m_pString+whereToStart, strToAppend, lenToAppend);
		m_stringLength = whereToStart + lenToAppend;
		m_pString[m_allocatedBufflength] = '\0';
	}
}


// 1) legal ASCII should be as the following pattern,
// 2) lenToCheck should contain the NULL terminator
// [... ,x1,x2,\0,x3,x4, ...] , 32 < x1, x2 < 126 (we don't care about x3, x4)
//////////////////////////////////////////////////////////////////////////////////
eStringValidityStatus CObjString::IsLegalAsciiString(char *bufferToCheck,
                                                     const int bufferLenToCheck,
                                                     bool isToCheckNullTermination/*=true*/,bool isToCheckGreaterLower/*=false*/)
{
	eStringValidityStatus retStrStatus = eStringNotNullTerminated;
	if (false == isToCheckNullTermination)
		retStrStatus = eStringValid;

    for(int i = 0 ; i < bufferLenToCheck ; i++)
	{
		const char curChar = bufferToCheck[i];

		// legal ascii string should be NULL terminated
		if (curChar == '\0')
		{
			retStrStatus = eStringValid;
			break;
		}

		// legal ascii values: 32-126
		if( (curChar < 32) || (126 < curChar)  )
		{
			retStrStatus = eStringInvalidChar;
			break;
		}
		if (isToCheckGreaterLower==true &&  ((60 == curChar) || (62 == curChar)) )
		{
			retStrStatus = eStringInvalidChar;
			bufferToCheck[i] ='\0';    //VNGR-26890
			break;
		}
	}


	return retStrStatus;
}

//////////////////////////////////////////////////////////////////////////////////
bool CObjString::TestCharRange(const char *strToCheck, BYTE downBound, BYTE upperBound)
{
    const int strToCheckLen = strlen(strToCheck);
    for(int i = 0 ; i < strToCheckLen ; i++)
    {
        if(strToCheck[i] < downBound || upperBound < strToCheck[i])
        {
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
CSmallString::CSmallString(const char*const str) : CStaticString<ONE_LINE_BUFFER_LEN>(str)
{
}

// copy constructor
//////////////////////////////////////////////////////////////////////////////////
CSmallString::CSmallString (const CObjString & s1) : CStaticString<ONE_LINE_BUFFER_LEN>(s1)
{
}

//////////////////////////////////////////////////////////////////////////////////
CMedString::CMedString(const char*const str) : 	CStaticString<TEN_LINE_BUFFER_LEN>(str)
{
}

// copy constructor
//////////////////////////////////////////////////////////////////////////////////
CMedString::CMedString (const CObjString & s1) : CStaticString<TEN_LINE_BUFFER_LEN>(s1)
{
}

//////////////////////////////////////////////////////////////////////////////////
CLargeString::CLargeString(const char*const str) : 	CStaticString<FIFTY_LINE_BUFFER_LEN>(str)
{
}

// copy constructor
//////////////////////////////////////////////////////////////////////////////////
CLargeString::CLargeString (const CObjString & s1) : CStaticString<FIFTY_LINE_BUFFER_LEN>(s1) //: m_pString(strcpy(new char[s1.m_allocatedBufflength+1],s1.GetString())) : m_allocatedBufflength(s1.m_allocatedBufflength)
{
}
/*
//////////////////////////////////////////////////////////////////////////////////
CManDefinedString::CManDefinedString(DWORD allocate_size) : CObjString("",allocate_size)
{
}

//////////////////////////////////////////////////////////////////////////////////
CManDefinedString::CManDefinedString (const CObjString & s1): CObjString(s1)
{
	if(m_pString)
	{
		delete [] m_pString;
	}

	m_stringLength = s1.GetStringLength();
	m_allocatedBufflength = s1.GetAllocLength();
	m_pString = new (0,0) char[m_allocatedBufflength+1];
	strncpy(m_pString,(s1.GetString()),m_stringLength);
	m_pString[m_stringLength] = '\0';

	if(m_pFormat)
	{
		delete [] m_pFormat;
	}

	if(s1.GetFormat()!=NULL)
	{
		WORD formatLen = strlen(s1.GetFormat());
		if (formatLen >= maxFormat) maxFormat = formatLen + 1;
		m_pFormat = new (0,0) char[formatLen+1];
		strncpy(m_pFormat,s1.GetFormat(),formatLen);
		m_pFormat[formatLen]='\0';
	}
}


*/

CSuperLargeString::CSuperLargeString(const char*const str) : 	CStaticString<HUNDRED_LINE_BUFFER_LEN>(str)
{
}

// copy constructor
//////////////////////////////////////////////////////////////////////////////////
CSuperLargeString::CSuperLargeString (const CObjString & s1) : CStaticString<HUNDRED_LINE_BUFFER_LEN>(s1) //: m_pString(strcpy(new char[s1.m_allocatedBufflength+1],s1.GetString())) : m_allocatedBufflength(s1.m_allocatedBufflength)
{
}
