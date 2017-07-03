#ifndef __KEY_CODE_SAVE_LOADER_H__
#define __KEY_CODE_SAVE_LOADER_H__

#include "VersionStruct.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

class KeyCodeSaveLoader
{
public:
	bool SaveKeyCode(string const & keycode, VERSION_S const & ver);
	bool LoadKeyCode(string & keycode, VERSION_S const & ver);
	bool ResetKeyCode(VERSION_S const & ver);
	
protected:
	class  VerKeyRec {
	public:
		VERSION_S  ver;
		string   key;
	};

	bool SaveKeyListToFile();
	bool LoadKeyListFromFile();

	vector<VerKeyRec> m_keyList;
};


#endif

