//+========================================================================+
//                            ObjString.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ObjString.h                                                 |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 27.08.96   |                                                      |
//+========================================================================+

#ifndef _COBJSTRINGAPP
#define _COBJSTRINGAPP


#include <string>
#include "PObject.h"
#include "DefinesGeneral.h"
#include "WrappersCSBase.h"
class CStructTm;

#define MAX_NUMBER_OF_DIGITS_IN_TYPES 	20
#define MAX_FORMAT						30

class CObjString : public CPObject
{
CLASS_TYPE_1(CObjString,CPObject)
public:
// Constructors

	//void* operator new(size_t);
	CObjString(const char* const str="",DWORD allocate_size=0, BOOL IsDynamic = YES);
	CObjString(char ch, WORD nRepeat = 1, BOOL IsDynamic = YES);
	CObjString(const CObjString& stringSrc, BOOL IsDynamic = YES);
	virtual const char* NameOf() const { return "CObjString";}

// overloaded assignment
	char operator[](DWORD nIndex) const;
    char& operator [] (DWORD n);


	const CObjString& operator=(const CObjString& stringSrc);
	const CObjString& operator=(char ch);
	const CObjString& operator=(const char* lpsz);


	// string concatenation
	const CObjString& operator+=(const CObjString& string);
	const CObjString& operator+=(char ch);
	const CObjString& operator+=(const char* lpsz);

	 friend CObjString operator+( const CObjString& s1, const CObjString& s2);
	 friend CObjString operator+( const CObjString& string, char ch);
	 friend CObjString operator+(char ch,  const CObjString& string);
	 friend CObjString operator+( const CObjString& string,  char* lpsz);
	 friend CObjString operator+( char* lpsz,  const CObjString& string);

	void 	SetFormat(const char *str);

 	void    Serialize(WORD format, std::ostream  &m_ostr);
    void    DeSerialize(WORD format, std::istream &m_istr);

	CObjString& operator<<(BYTE by);
	CObjString& operator<<(int num);
	CObjString& operator<<(WORD wd);
	CObjString& operator<<(DWORD dw);
	CObjString& operator<<(long lg);
	CObjString& operator<<(short sh);
	CObjString& operator<<(float fl);
    CObjString& operator<<(const std::string & stdString);
    CObjString& operator<<(ULONGLONG ull);
	CObjString& operator<<(char ch);
	CObjString& operator<<(const CObjString& string1);
	CObjString& operator<<(const char* lpsz);
	CObjString& operator<<(const CStructTm& time);
	CObjString& operator<<(const CBaseWrapper& basewrapper);

	friend BYTE operator==(const CObjString&, const CObjString&);
    friend BYTE operator==(const CObjString&, const char*);
    friend BYTE operator==(const char*, const CObjString&);
    friend BYTE operator!=(const CObjString&, const CObjString&);
    friend BYTE operator!=(const CObjString&, const char*);
    friend BYTE operator!=(const char*, const CObjString&);
    friend BYTE operator<(const CObjString&, const CObjString&);
    friend BYTE operator<(const CObjString&, const char*);
    friend BYTE operator<(const char*, const CObjString&);
    friend BYTE operator>(const CObjString&, const CObjString&);
    friend BYTE operator>(const CObjString&, const char*);
    friend BYTE operator>(const char*, const CObjString&);
    friend BYTE operator<=(const CObjString&, const CObjString&);
    friend BYTE operator<=(const CObjString&, const char*);
    friend BYTE operator<=(const char*, const CObjString&);
    friend BYTE operator>=(const CObjString&, const CObjString&);
    friend BYTE operator>=(const CObjString&, const char*);
    friend BYTE operator>=(const char*, const CObjString&);

	virtual ~CObjString();
	const char * GetString() const { return m_pString; }

	WORD GetAllocLength() const	{return m_allocatedBufflength;}
	WORD GetStringLength() const	{return m_stringLength;}
	BYTE IsEmpty() const;
	const char* GetFormat() const { return m_pFormat; }
    int Find(const char* str) const;

	void Clear();

	BOOL IsDynamic() const {return m_bIsDynamic;}
    void ToUpper();
    void ToLower();
    bool IsNumeric();
    void ReplaceChar(const char from, const char to);
    void ReplaceChar(bool fromCond(const char ch), const char to);
    void Reverse();
    void RemoveChars(const char *toRemove);
    void ForceAppendStr(const char *strToAppend);

    static void ToUpper(char * str);
    static void ToLower(char * str);
    static bool IsNumeric(const char * str);
    static bool IsContainsSpecialChars(const char* str);
    static eStringValidityStatus IsLegalAsciiString(char *bufferToCheck,
                                                    const int bufferLenToCheck,
                                                    bool isToCheckNullTermination = true
                                                    ,bool isToCheckGreaterLower=false);
    static bool TestCharRange(const char *strToCheck, BYTE downBound, BYTE upperBound);

    static void ReplaceChar(char *strSource, const char from, const char to);
    static void ReplaceChar(char *str, bool fromCond(const char ch), const char to);
    static void ReplaceByte(unsigned char *str, int strLen, unsigned char from, unsigned char to);
    static void Reverse(char *strSource);
    static void RemoveChars(char *strSource, const char *toRemove);
    void ForceAppendStr(char *strSource, const char *strToAppend);


protected:
	char* 	m_pString;   			// pointer to ref counted string data
	DWORD 	m_allocatedBufflength;	// allocated buffer length
	DWORD 	m_stringLength;
	char* 	m_pFormat;				// for formatting functions
	BOOL 	m_bIsDynamic;			// Is the string dynamically allocated

	void concat	(const CObjString & added_str);
	void concat	(const char* str, DWORD len);
	void Init	(const char* str);
	void Init	(const char ch);
	void Init	(const CObjString& s1, const WORD formatLen);

	template <WORD MAX_LEN> friend class CStaticString;

};

template <WORD MAX_LEN>
class CStaticString: public CObjString
{
	CLASS_TYPE_1(CStaticString, CObjString)
public:
	CStaticString(const char * const str="");
	CStaticString(char ch, WORD nRepeat = 1);
	CStaticString(const CObjString& stringSrc);
protected:
	char m_string[MAX_LEN];		// Static string buffer
	char m_format[MAX_FORMAT];	// Static formatting buffer
};
//typedef CStaticString<ONE_LINE_BUFFER_LEN> CSmallString;
//typedef CStaticString<TEN_LINE_BUFFER_LEN> CMedString;
//typedef CStaticString<FIFTY_LINE_BUFFER_LEN> CLargeString;
class CSmallString :  public CStaticString<ONE_LINE_BUFFER_LEN>
{
CLASS_TYPE_1(CSmallString, CStaticString<ONE_LINE_BUFFER_LEN>)
public:
	CSmallString(const char * const str="");
	CSmallString(const CObjString& stringSrc);
	virtual const char* NameOf() const { return "CSmallString";}
};


class CMedString :  public CStaticString<TEN_LINE_BUFFER_LEN>
{
CLASS_TYPE_1(CMedString, CStaticString<TEN_LINE_BUFFER_LEN>)
public:
	CMedString(const char * const str="");
	CMedString(const CObjString& stringSrc);
	virtual const char* NameOf() const { return "CMedString";}

private:
};


class CLargeString :  public CStaticString<FIFTY_LINE_BUFFER_LEN>
{
CLASS_TYPE_1(CLargeString, CStaticString<FIFTY_LINE_BUFFER_LEN>)
public:
	CLargeString(const char * const str="");
	CLargeString(const CObjString& stringSrc);
	virtual const char* NameOf() const { return "CLargeString";}
private:
};

#define HUNDRED_LINE_BUFFER_LEN	(10000*2) // should be moved to SharedDefines.h near FIFTY_LINE_BUFFER_LEN
											//N.A. DEBUG VP8
class CSuperLargeString :  public CStaticString<HUNDRED_LINE_BUFFER_LEN>
{
CLASS_TYPE_1(CSuperLargeString, CStaticString<HUNDRED_LINE_BUFFER_LEN>)
public:
	CSuperLargeString(const char * const str="");
	CSuperLargeString(const CObjString& stringSrc);
	virtual const char* NameOf() const { return "CSuperLargeString";}
private:
};

class CManDefinedString :  public CObjString
{
CLASS_TYPE_1(CManDefinedString, CObjString)
public:
	CManDefinedString(DWORD allocate_size): CObjString("", allocate_size) {}
//	CManDefinedString(const CObjString& stringSrc);
	virtual const char* NameOf() const { return "CManDefinedString";}
private:
};
#endif
