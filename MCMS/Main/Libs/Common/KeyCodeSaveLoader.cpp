#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include "KeyCodeSaveLoader.h"
#include "LiteProfile.h"
#include "McmsCfgFileInfo.h"
#include "DefinesGeneral.h"
#include <string.h>

using namespace std;

#define MAX_KEY_NUM 100
std::string g_keyCodeFileName;

bool KeyCodeSaveLoader::SaveKeyListToFile()
{
    CProfile pf;
    char keyNum[32];
    char versionNum[32];
    if (g_keyCodeFileName=="")
    	g_keyCodeFileName = RMX_KEYCODE_FILE;

    if (!pf.Create(g_keyCodeFileName.c_str()))
    {
        return false;
    }

    vector<VerKeyRec>::const_iterator const end = m_keyList.end();
    vector<VerKeyRec>::const_iterator pos = m_keyList.begin();
    unsigned int i = 0;

    for (; pos!=end; ++pos, ++i)
    {
        sprintf(keyNum, "%d", i);
        pf.SetString(keyNum, "KeyCode", (*pos).key.c_str());
        pf.SetUInt(keyNum, "ver_major", (*pos).ver.ver_major);
        pf.SetUInt(keyNum, "ver_minor", (*pos).ver.ver_minor);
        pf.SetUInt(keyNum, "ver_release", (*pos).ver.ver_release);
        pf.SetUInt(keyNum, "ver_internal", (*pos).ver.ver_internal);
    }

    return true;
}

bool KeyCodeSaveLoader::LoadKeyListFromFile()
{
    CProfile pf;
    char keyNum[32];
    m_keyList.clear();

    if (g_keyCodeFileName=="")
    	g_keyCodeFileName = RMX_KEYCODE_FILE;

    if (!pf.Open(g_keyCodeFileName.c_str()))
    {
        return false;
    }

    unsigned int i = 0;
    for(; i < MAX_KEY_NUM; ++i)
    {
        VerKeyRec rec;
    	  sprintf(keyNum, "%d", i);

        rec.key = pf.GetString(keyNum, "KeyCode", "");
        if(0 == rec.key.length()) continue;
		
        rec.ver.ver_major = pf.GetUInt(keyNum, "ver_major", 0);
        rec.ver.ver_minor = pf.GetUInt(keyNum, "ver_minor", 0);
        rec.ver.ver_release = pf.GetUInt(keyNum, "ver_release", 1);
        rec.ver.ver_internal = pf.GetUInt(keyNum, "ver_internal", 1);
	  m_keyList.push_back(rec);
    }

    return true;
}

bool KeyCodeSaveLoader::SaveKeyCode(string const & keycode, VERSION_S const & ver)
{
    bool found = false;
    LoadKeyListFromFile();

    vector<VerKeyRec>::iterator end = m_keyList.end();
    vector<VerKeyRec>::iterator pos = m_keyList.begin();
    for (; pos!=end; ++pos)
    {
        if((*pos).ver.ver_major == ver.ver_major && (*pos).ver.ver_minor == ver.ver_minor)
	 {
		found = true;
		break;
	 }
    }

    if(found)
    {
        m_keyList.erase(pos);
    }

    VerKeyRec rec;
    rec.ver = ver;
    rec.key = keycode;
    if(m_keyList.size() >= MAX_KEY_NUM)
    {
         m_keyList.erase(m_keyList.begin());
    }
    m_keyList.push_back(rec);
    
    return SaveKeyListToFile();
}

bool KeyCodeSaveLoader::ResetKeyCode(VERSION_S const & ver)
{
    //remove all saved keycode when restore factory.
    m_keyList.clear();
    return SaveKeyListToFile();
}

bool KeyCodeSaveLoader::LoadKeyCode(string & keycode, VERSION_S const & ver)
{
    LoadKeyListFromFile();
	
    vector<VerKeyRec>::const_iterator const end = m_keyList.end();
    vector<VerKeyRec>::const_iterator pos=m_keyList.begin();
    for (; pos!=end; ++pos)
    {
        if((*pos).ver.ver_major == ver.ver_major && (*pos).ver.ver_minor == ver.ver_minor)
	 {
		keycode = (*pos).key;
		return true;
	 }
    }

    return false;
}

