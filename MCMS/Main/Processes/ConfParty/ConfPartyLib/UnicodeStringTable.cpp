/*
 * UnicodeStringTable.cpp
 *
 *  Created on: Oct 12, 2009
 *      Author: bguelfand
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "UnicodeStringTable.h"

char* CUnicodeStringTable::m_pHexStrings[LANGUAGES_NUMBER][STRINGS_NUMBER] =
{ {//English
    "4f 00 72 00 67 00 61 00 6e 00 69 00 7a 00 65 00 72 00",                                                      //STR_ORGANIZER
    "54 00 69 00 6d 00 65 00 20 00 69 00 6e 00 20 00 63 00 61 00 6c 00 6c 00",                                    //STR_TIME_CALL
    "41 00 75 00 64 00 69 00 6f 00 20 00 70 00 61 00 72 00 74 00 69 00 63 00 69 00 70 00 61 00 6e 00 74 00 73 00",//STR_AUDIO_PARTS
    "56 00 69 00 64 00 65 00 6f 00 20 00 70 00 61 00 72 00 74 00 69 00 63 00 69 00 70 00 61 00 6e 00 74 00 73 00",//STR_VIDEO_PARTS
    "41 00 63 00 63 00 65 00 73 00 73 00 20 00 6e 00 75 00 6d 00 62 00 65 00 72 00 73 00 3a 00",                  //STR_ACCESS_NO
    "44 00 75 00 72 00 61 00 74 00 69 00 6f 00 6e 00",                                                            //STR_DURATION
    "6d 00 69 00 6e 00 75 00 74 00 65 00 73 00",                                                                  //STR_MINUTES
    "68 00 6f 00 75 00 72 00",                                                                                    //STR_HOUR
    "68 00 6f 00 75 00 72 00 73 00",                                                                              //STR_HOURS
    "2e 00",                                                                                                      //STR_DEC_DELIMITER
    "70 00 65 00 72 00 6D 00 61 00 6E 00 65 00 6E 00 74 00"                                                       //STR_PERMANENT
},{//German
    "4f 00 72 00 67 00 61 00 6e 00 69 00 73 00 61 00 74 00 6f 00 72 00",                                          //STR_ORGANIZER
    "44 00 69 00 65 00 20 00 5a 00 65 00 69 00 74 00 20 00 69 00 6e 00 20 00 43 00 61 00 6c 00 6c 00",            //STR_TIME_CALL
    "41 00 75 00 64 00 69 00 6f 00 2d 00 54 00 65 00 69 00 6c 00 6e 00 65 00 68 00 6d 00 65 00 72 00",            //STR_AUDIO_PARTS
    "56 00 69 00 64 00 65 00 6f 00 2d 00 54 00 65 00 69 00 6c 00 6e 00 65 00 68 00 6d 00 65 00 72 00",            //STR_VIDEO_PARTS
    "41 00 63 00 63 00 65 00 73 00 73 00 2d 00 4e 00 75 00 6d 00 6d 00 65 00 72 00",                              //STR_ACCESS_NO
    "44 00 61 00 75 00 65 00 72 00",                                                                              //STR_DURATION
    "6d 00 69 00 6e 00 75 00 74 00 65 00 73 00",                                                                  //STR_MINUTES
    "68 00 6f 00 75 00 72 00",                                                                                    //STR_HOUR
    "68 00 6f 00 75 00 72 00 73 00",                                                                              //STR_HOURS
    "2c 00",                                                                                                      //STR_DEC_DELIMITER
    "70 00 65 00 72 00 6D 00 61 00 6E 00 65 00 6E 00 74 00 65 00"                                                 //STR_PERMANENT(permanente)
},{//Spanish SA
    "4f 00 72 00 67 00 61 00 6e 00 69 00 7a 00 61 00 64 00 6f 00 72 00",
    "45 00 6c 00 20 00 74 00 69 00 65 00 6d 00 70 00 6f 00 20 00 65 00 6e 00 20 00 6c 00 61 00 20 00 6c 00 6c 00 61 00 6d 00 61 00 64 00 61 00",
    "50 00 61 00 72 00 74 00 69 00 63 00 69 00 70 00 61 00 6e 00 74 00 65 00 20 00 41 00 75 00 64 00 69 00 6f 00",
    "50 00 61 00 72 00 74 00 69 00 63 00 69 00 70 00 61 00 6e 00 74 00 65 00 20 00 56 00 ed 00 64 00 65 00 6f 00",
    "45 00 6c 00 20 00 6e 00 fa 00 6d 00 65 00 72 00 6f 00 20 00 64 00 65 00 20 00 61 00 63 00 63 00 65 00 73 00 6f 00",
    "44 00 75 00 72 00 61 00 63 00 69 00 f3 00 6e 00",
    "6d 00 69 00 6e 00 75 00 74 00 65 00 73 00",
    "68 00 6f 00 75 00 72 00",
    "68 00 6f 00 75 00 72 00 73 00",
    "2e 00",
    "70 00 65 00 72 00 6D 00 61 00 6E 00 65 00 6E 00 74 00 65 00 73 00"                                           //STR_PERMANENT(permanentes)
},{//French
    "4f 00 72 00 67 00 61 00 6e 00 69 00 73 00 61 00 74 00 65 00 75 00 72 00",
    "4c 00 65 00 20 00 74 00 65 00 6d 00 70 00 73 00 20 00 64 00 61 00 6e 00 73 00 20 00 6c 00 27 00 61 00 70 00 70 00 65 00 6c 00",
    "50 00 61 00 72 00 74 00 69 00 63 00 69 00 70 00 61 00 6e 00 74 00 20 00 41 00 75 00 64 00 69 00 6f 00",
    "50 00 61 00 72 00 74 00 69 00 63 00 69 00 70 00 61 00 6e 00 74 00 20 00 56 00 69 00 64 00 e9 00 6f 00",
    "4e 00 75 00 6d 00 e9 00 72 00 6f 00 20 00 64 00 27 00 61 00 63 00 63 00 e8 00 73 00",
    "44 00 75 00 72 00 e9 00 65 00", "6d 00 69 00 6e 00 75 00 74 00 65 00 73 00",
    "68 00 6f 00 75 00 72 00",
    "68 00 6f 00 75 00 72 00 73 00",
    "2c 00",
    "70 00 65 00 72 00 6D 00 61 00 6E 00 65 00 6E 00 74 00 65 00"                                                 //STR_PERMANENT(permanente)
},{//Japanese
    "3b 4e ac 50 05 80",                                                                                          //STR_ORGANIZER
    "42 66 93 95 6e 30 7c 54 73 30 fa 51 57 30 67 30",                                                            //STR_TIME_CALL
    "F3 97 F0 58 C2 53 A0 52 05 80",                                                                              //STR_AUDIO_PARTS
    "D3 30 C7 30 AA 30 C2 53 A0 52 05 80",                                                                        //STR_VIDEO_PARTS
    "A2 30 AF 30 BB 30 B9 30 6A 75 F7 53",                                                                        //STR_ACCESS_NO
    "1A 4F 70 8B 42 66 93 95",                                                                                    //STR_DURATION
    "06 52",                                                                                                      //STR_MINUTES
    "42 66 93 95",                                                                                                //STR_HOUR
    "42 66 93 95",                                                                                                //STR_HOURS
    "2e 00",                                                                                                      //STR_DEC_DELIMITER
    "38 6C 45 4E"                                                                                                 //STR_PERMANENT
}, {//Korean
    "6C AD 31 C1 90 C7",                                                                                          //STR_ORGANIZER
    "dc c2 04 ac 20 00 b5 d1 54 d6 d0 c5 1c c1",                                                                  //STR_TIME_CALL
    "24 C6 14 B5 24 C6 20 00 38 CC 00 AC 90 C7",                                                                  //STR_AUDIO_PARTS
    "44 BE 14 B5 24 C6 20 00 38 CC 00 AC 90 C7",                                                                  //STR_VIDEO_PARTS
    "61 C5 38 C1 A4 C2 20 00 88 BC 38 D6",                                                                        //STR_ACCESS_NO
    "C0 C9 8D C1 20 00 DC C2 04 AC",                                                                              //STR_DURATION
    "84 BD",                                                                                                      //STR_MINUTES
    "DC C2 04 AC",                                                                                                //STR_HOUR
    "DC C2 04 AC",                                                                                                //STR_HOURS
    "2e 00",                                                                                                      //STR_DEC_DELIMITER
    "01 C6 6C AD"                                                                                                 //STR_PERMANENT
},{//Chinese Simple
    "C4 7E C7 7E 05 80",                                                                                          //STR_ORGANIZER
    "c4 7e c7 7e 05 80",                                                                                          //STR_TIME_CALL
    "F3 97 91 98 C2 53 0E 4E 05 80",                                                                              //STR_AUDIO_PARTS
    "C6 89 91 98 C2 53 0E 4E 05 80",                                                                              //STR_VIDEO_PARTS
    "A5 63 65 51 F7 53 01 78",                                                                                    //STR_ACCESS_NO
    "01 63 ED 7E F6 65 F4 95",                                                                                    //STR_DURATION
    "06 52 9F 94",                                                                                                //STR_MINUTES
    "0F 5C F6 65",                                                                                                //STR_HOUR
    "0F 5C F6 65",                                                                                                //STR_HOURS
    "2e 00",                                                                                                      //STR_DEC_DELIMITER
    "38 6C 45 4E"                                                                                                 //STR_PERMANENT
}};

CUnicodeStringTable::CUnicodeStringTable()
{
	m_eCurrentLanguage = eEnglish;
	SetCurrentLanguage ( m_eCurrentLanguage );

	AllocateIconv();
}

CUnicodeStringTable::~CUnicodeStringTable()
{
	DeallocatIconv();
}

ELanguges CUnicodeStringTable::GetCurrentLanguage() const
{
	return m_eCurrentLanguage;
}

void CUnicodeStringTable::ClearTable()
{
	for (int i=0; i<STRINGS_NUMBER; ++i)
	{
		m_table[i].clear();
	}
}

void CUnicodeStringTable::SetCurrentLanguage(ELanguges eLang)
{
	m_eCurrentLanguage = eLang;

	for ( int i=0; i < STRINGS_NUMBER; ++i )
	{
		TranslateString ( &(m_table[i]), m_pHexStrings[eLang][i] );
	}
}

void CUnicodeStringTable::TranslateString(std::vector<unsigned char>* pVector, const char* pszSource)
{
	int n = strlen ( pszSource );
	char sTmp[3] = "";
	pVector->clear();
	for ( int i=0,j=0; i<n; ++i)
	{
		if ( pszSource[i] == ' ' )
			j = 0;
		else if ( j == 0 || j == 1 )
		{
			sTmp[j] = pszSource[i];
			if ( j == 1 )
			{
				unsigned char ch = strtoul ( sTmp, NULL, 16 );
				pVector->push_back(ch);
			}
			else
				++j;
		}
	}
}

WORD CUnicodeStringTable::GetString ( int iStrID, char* buff )
{
	WORD n = GetBytes(iStrID, buff);
	buff[n] = 0;
	buff[n+1] = 0;
	return n+2;
}
WORD CUnicodeStringTable::GetBytes ( int iStrID, char* buff )
{
	std::vector<unsigned char>* pvStr = &(m_table[iStrID]);
	WORD n = pvStr->size();
	for (int i=0; i<n; ++i)
	{
		buff[i] = pvStr->at(i);
	}
	return n;
}

void CUnicodeStringTable::DumpBuffer ( const char* buffIn, WORD nBuffInLen, char* buffOut, WORD nBuffOutLen )
{
	char pTmp[3] = "";
	int  j = 0;
	for ( int i=0; i < nBuffInLen; ++i )
	{
		if ( j >= nBuffOutLen - 4 )
		{
			buffOut[j] = '\0';
			return;
		}
		sprintf(pTmp, "%02x", (unsigned char)(buffIn[i]));
		buffOut[j++] = pTmp[0];
		buffOut[j++] = pTmp[1];
		buffOut[j++] = ' ';
	}
	if (j > 0 && buffOut[j-1] == '\0')
		buffOut[j-1] = '\0';
	else
		buffOut[j] = '\0';
}

DWORD CUnicodeStringTable::AllocateIconv(const char* toEncoding ,const char* fromEncoding )
{
	m_pIconv = iconv_open(toEncoding,fromEncoding);

	if(m_pIconv == (iconv_t)-1)
	{
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------------------------
DWORD CUnicodeStringTable::DeallocatIconv()
{
	int ret_val = iconv_close(m_pIconv);
	if(ret_val == -1)
	{
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------------------------
WORD CUnicodeStringTable::ConvertStingEncoding(const char* from_buffer,DWORD from_buffer_len,char* to_buffer,DWORD to_buffer_len)
{
	if(m_pIconv == (iconv_t)-1)
	{
		return 0;
	}
	if(from_buffer == NULL || from_buffer_len ==0)
	{
		return 0;
	}
	if(to_buffer == NULL || to_buffer_len ==0)
	{
		return 0;
	}

	char**  read_buffer = (char**)&from_buffer;
	size_t in_char_len = from_buffer_len;
	size_t* in_char_count = &in_char_len;
	char**  write_buffer = &to_buffer;
	size_t  out_char_len = to_buffer_len;
	size_t* out_char_count = &out_char_len;
	size_t res = iconv(m_pIconv,read_buffer,in_char_count,write_buffer,out_char_count);
	if(res == (size_t)-1)
	{
		return 0;
	}

	WORD num_of_char_wrote = to_buffer_len - *out_char_count;

    return num_of_char_wrote;

}
//-----------------------------------------------------------------------------------------------------
int CUnicodeStringTable::TranslateUTF8ToUCS2(const char* utf8_str,char* ucs2_site_name_buffer,WORD ucs2_buffer_len)
{
     // we don't know how much it can grow. so *10 could be good guess.

	WORD unicode_buffer_size = strlen(utf8_str)+1;
	WORD max_ucs2_buffer_size = unicode_buffer_size * 2;

	char* ucs2_site_name = new char[max_ucs2_buffer_size];
	memset(ucs2_site_name,0,max_ucs2_buffer_size);

	WORD chars_wrote = ConvertStingEncoding(utf8_str,unicode_buffer_size,ucs2_site_name,max_ucs2_buffer_size);

	WORD num_of_bytes_to_copy = ucs2_buffer_len;
	if(max_ucs2_buffer_size < ucs2_buffer_len)
	{
		num_of_bytes_to_copy = max_ucs2_buffer_size;
	}
	for(WORD byte_index=0;byte_index<num_of_bytes_to_copy;byte_index++)
	{
		ucs2_site_name_buffer[byte_index] = ucs2_site_name[byte_index];
	}

	delete [] ucs2_site_name;
       return chars_wrote;
}
