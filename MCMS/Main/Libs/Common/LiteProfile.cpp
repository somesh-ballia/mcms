#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "LiteProfile.h"

CProfile::CProfile() :
    m_bModified(false)
{
}

CProfile::~CProfile()
{
    Close();
}

bool CProfile::Open(const char* pszFileName)
{
	m_filename=pszFileName;
	m_map.clear();
    m_bModified = false;

    FILE *fp = fopen(pszFileName, "r");
    if (NULL == fp)
        return false;

    // process lines
    char        szSectionName[256] = "";
    CSection    section;
    char        szLine[2048];

    while (NULL != fgets(szLine, sizeof(szLine) - 1, fp))
    {
        if ('[' == *szLine)
        {       // found the beginning of new section, add current section to map
            if (0 != *szSectionName)
				m_map[szSectionName] = section;
            char *p = strchr(szLine, ']');
			if (NULL == p) {
				fclose(fp);
				return false;
			}
            *p = 0;
            strncpy(szSectionName, szLine + 1,sizeof(szSectionName)-1);
            szSectionName[256-1]='\0';
			section.clear();
        }
        else
        {       //      add an entry to current section
            char *p = strchr(szLine, '=');
            if (NULL == p)
				continue;   // skip empty lines
            *p++ = 0;
            char szEntryName[256]={0};
            strncpy(szEntryName, szLine,sizeof(szEntryName)-1);

            char *q = strchr(p, '\n');
            if (NULL != q)
				*q = 0;
            char szValue[1024]={0};
            strncpy(szValue, p,sizeof(szValue)-1);

            section[szEntryName] = szValue;
        }
    }

    m_map[szSectionName] = section;

	fclose(fp);
    return true;
}

bool CProfile::Create(const char* pszFileName)
{
	m_filename=pszFileName;
    m_map.clear();
    m_bModified = false;
	FILE *fp = fopen(pszFileName, "w");
	bool ret=fp!=NULL;
	if(fp!=NULL)
		fclose(fp);
    return ret;
}

bool CProfile::Close()
{
	return Flush();
}

bool CProfile::Flush()
{
	if (!m_bModified)
		return true;

	FILE *fp=fopen(m_filename.c_str(),"w");
    if (NULL == fp)
        return false;

    for (CMap::const_iterator i = m_map.begin(); i != m_map.end(); i++)
    {
        const char* pszSectionName = i->first.c_str();
        if (0 != *pszSectionName)
            fprintf(fp, "[%s]\n", pszSectionName);
        const CSection& section = i->second;
        for (CSection::const_iterator k = section.begin(); k != section.end(); k++)
            fprintf(fp, "%s=%s\n", k->first.c_str(), k->second.c_str());
        fprintf(fp, "\n");
    }

    fclose(fp);
	m_bModified=false;
    return true;
}

const char* CProfile::
GetString(const char* pszSectionName, const char* pszEntryName, const char* defStr)
{
    CMap::const_iterator itrt1 = m_map.find(pszSectionName);
    if (itrt1 == m_map.end())
        return defStr;
    const CSection& section = itrt1->second;
    CSection::const_iterator itrt2 = section.find(pszEntryName);
    return (itrt2 == section.end()) ? defStr : (itrt2->second).c_str();
}

void CProfile::
SetString(const char* pszSectionName, const char* pszEntryName, const char* pszValue)
{
    CSection& section = m_map[pszSectionName];
	section[pszEntryName] = pszValue;
	m_bModified = true;
}

int CProfile::
GetInt(const char* pszSectionName, const char* pszEntryName, int defInt)
{
    CMap::const_iterator itrt1 = m_map.find(pszSectionName);
    if (itrt1 == m_map.end())
        return defInt;
    const CSection& section = itrt1->second;
    CSection::const_iterator itrt2 = section.find(pszEntryName);
	if (itrt2 == section.end())
		return defInt;
    return (int)strtol(itrt2->second.c_str(),NULL,0);
}

void CProfile::
SetInt(const char* pszSectionName, const char* pszEntryName, int value)
{
    char s[32];
    sprintf(s, "%d", value);
    SetString(pszSectionName, pszEntryName, s);
}

unsigned int CProfile::
GetUInt(const char* pszSectionName, const char* pszEntryName, unsigned int defInt)
{
    CMap::const_iterator itrt1 = m_map.find(pszSectionName);
    if (itrt1 == m_map.end())
        return defInt;
    const CSection& section = itrt1->second;
    CSection::const_iterator itrt2 = section.find(pszEntryName);
	if (itrt2 == section.end())
		return defInt;
    return (unsigned int)strtoul(itrt2->second.c_str(),NULL,0);
}

void CProfile::
SetUInt(const char* pszSectionName, const char* pszEntryName, unsigned int value)
{
    char s[32];
    sprintf(s, "%d", value);
    SetString(pszSectionName, pszEntryName, s);
}

unsigned int CProfile::
GetHex(const char* pszSectionName, const char* pszEntryName, unsigned int defInt)
{
	return GetInt(pszSectionName, pszEntryName, defInt);
}

void CProfile::
SetHex(const char* pszSectionName, const char* pszEntryName, unsigned int value)
{
    char s[32];
    sprintf(s, "0x%08x", value);
    SetString(pszSectionName, pszEntryName, s);
}
