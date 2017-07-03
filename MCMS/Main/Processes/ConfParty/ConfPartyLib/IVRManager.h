#ifndef __IVRMANAGER_H__
#define __IVRMANAGER_H__

#include "StateMachine.h"
#include "IvrApiStructures.h"

class CAVmsgServiceList;

#define MAX_MUSIC_SOURCE_NUMBER 1   // max music sources in system
#define START_MUSIC_SOURCE_ID   100


////////////////////////////////////////////////////////////////////////////
//                        CIVRManager
////////////////////////////////////////////////////////////////////////////
class CIVRManager : public CStateMachine
{
	CLASS_TYPE_1(CIVRManager, CStateMachine)

public:
	                    CIVRManager();
	virtual            ~CIVRManager();
	virtual const char* NameOf() const { return GetCompileType(); }

	void                OnTimerCheckRollCallFiles(CSegment* pParam);
	int                 InitIVRConfig();
	void                GetMusicSources();
	void                AddAllMusicSources();
	void                DeleteIVRServices();

protected:

	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	void                FillSlidesList();

	STATUS              CreateIVRFolders(const char* szMainFolder);

	void                CleanRollCallFiles(WORD actionType);
	int                 FillIvrListFromXmlFile(const char* file_name);
	void                AddDefaultIvrToIVRListFromXmlFile(const char* file_name);
	void                UpdateTables(CAVmsgServiceList* pAVmsgServiceList);
	void                ImportNewIVREvents(CAVmsgServiceList* pDestServiceList, CAVmsgServiceList* pSourceServiceList);
	void                DeleteDefaultXMLFileFromDisk(const char* file_name);
	void                UpdateDefaultServices();
	void                SaveIvrServiceList();
	void                AddActiveAlarmForBadIVRList();
	void                AddFaultForBadIVRListFiles();
	void                TestCreateXmlFile();
	void                TestSaveListToDisk();
	void                TestCreateXmlFileOfMultipleServices();

	void                DeleteXMLFileFromDisk(const char* file_name);

protected:

	WORD                m_musicSourceNum;
	SIVRAddMusicSource* m_musicSourceList[MAX_MUSIC_SOURCE_NUMBER];

	PDECLAR_MESSAGE_MAP
};

#endif /* __IVRMANAGER_H__  */

