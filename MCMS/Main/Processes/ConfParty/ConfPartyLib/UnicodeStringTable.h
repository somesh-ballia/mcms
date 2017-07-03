/*
 * UnicodeStringTable.h
 *
 *  Created on: Oct 12, 2009
 *      Author: bguelfand
 */

#ifndef _UNICODESTRINGTABLE_H_
#define _UNICODESTRINGTABLE_H_

#include <vector>
#include <iconv.h>
#include <errno.h>
#include "ConfPartyApiDefines.h"
#include "UnicodeDefines.h"

#define LANGUAGES_NUMBER	7

enum STRINGTABLE_INDEX {
  STR_ORGANIZER = 0,
  STR_TIME_CALL,
  STR_AUDIO_PARTS,
  STR_VIDEO_PARTS,
  STR_ACCESS_NO,
  STR_DURATION,
  STR_MINUTES,
  STR_HOUR,
  STR_HOURS,
  STR_DEC_DELIMITER,
  STR_PERMANENT,
  STRINGS_NUMBER,
};

class CUnicodeStringTable
{
public:
	CUnicodeStringTable();
	~CUnicodeStringTable();

	ELanguges	GetCurrentLanguage() const;
	void		SetCurrentLanguage(ELanguges eLang);

	// UTF-8 to UCS-2LE conversion
	int			TranslateUTF8ToUCS2(const char* utf8_str,char* ucs2_site_name_buffer,WORD ucs2_buffer_len);
	WORD		GetString ( int iStrID, char* buff );
	WORD		GetBytes ( int iStrID, char* buff );

	void		DumpBuffer ( const char* buff, WORD nBuffLen, char* buffOut, WORD nBuffOutLen );

private:
	void ClearTable();
	ELanguges	m_eCurrentLanguage;
	std::vector<unsigned char>	m_table[STRINGS_NUMBER];

	void 		TranslateString(std::vector<unsigned char>* pVector, const char* pszSource);

	static char* m_pHexStrings[LANGUAGES_NUMBER][STRINGS_NUMBER];


// UTF-8 to UCS-2LE conversion
	WORD		ConvertStingEncoding(const char* from_buffer,DWORD from_buffer_len,char* to_buffer,DWORD to_buffer_len);
	DWORD		AllocateIconv(const char* toEncoding = VIDEO_UNIT_SITE_NAMES_ENCODE_TYPE/*UCS-2LE*/,const char* fromEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE/*UTF-8*/);
	DWORD		DeallocatIconv();

	iconv_t m_pIconv;

};

#endif /* _UNICODESTRINGTABLE_H_ */
