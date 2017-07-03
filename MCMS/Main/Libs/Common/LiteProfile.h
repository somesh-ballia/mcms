// profile.h

#ifndef _profile_h_
#define _profile_h_

using namespace std;

#include <map>
#include <string>

class CProfile
{
public:
    CProfile();
    virtual ~CProfile();

    bool Open(const char* pszFileName);
    bool Create(const char* pszFileName);
    bool Close();
	bool Flush();

    const char* GetString(const char* pszSectionName, const char* pszEntryName, const char *defStr="");
    void SetString(const char* pszSectionName, const char* pszEntryName, const char* pszValue);
    
	int GetInt(const char* pszSectionName, const char* pszEntryName,int defInt=0);
    void SetInt(const char* pszSectionName, const char* pszEntryName, int value);
    
	unsigned int GetUInt(const char* pszSectionName, const char* pszEntryName,unsigned int defInt=0);
    void SetUInt(const char* pszSectionName, const char* pszEntryName, unsigned int value);
    
	unsigned int GetHex(const char* pszSectionName, const char* pszEntryName,unsigned int defInt=0);
    void SetHex(const char* pszSectionName, const char* pszEntryName, unsigned int value);

private:
    typedef map<string,string>	    CSection;
    typedef map<string,CSection>    CMap;

    CMap    m_map;
    bool    m_bModified;
	string	m_filename;
};

#endif
