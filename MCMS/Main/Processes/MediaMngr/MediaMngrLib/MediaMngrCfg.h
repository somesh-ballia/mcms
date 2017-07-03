#ifndef MEDIAMNGRCFG_H_
#define MEDIAMNGRCFG_H_


#include <string>
#include <stdio.h>
#include "SerializeObject.h"
#include "IpCommonDefinitions.h"

using namespace std;


#define	MEDIA_MNGR_CONFIG_FILE_NAME	"Cfg/MEDIA_MNGR_CFG.XML"

#define	MAX_MEDIA_FILES_IN_LIBRARY	10


class CNicCfg;
class CMediaLibrary; 

class  CMediaMngrCfg : public CSerializeObject
{
CLASS_TYPE_1(CMediaMngrCfg, CSerializeObject)

public:

	// Constructors
	CMediaMngrCfg();
	virtual ~CMediaMngrCfg();
	CMediaMngrCfg(const CMediaMngrCfg& other);
	
	// Initializations
	void Init();

	// Operations
	CMediaMngrCfg& operator =(const CMediaMngrCfg& reference);
	const char* NameOf() const {return "CMediaMngrCfg";}
	virtual CSerializeObject* Clone() { return NULL; }

	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& rSegment);
	
	int IsEqual(const CMediaMngrCfg& rOther) const;
	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(MEDIA_MNGR_CONFIG_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(MEDIA_MNGR_CONFIG_FILE_NAME, "MEDIA_MNGR_CFG"); }
	void WriteXmlFile(string name) { CSerializeObject::WriteXmlFile(name.c_str(), "MEDIA_MNGR_CFG"); }

	
	CNicCfg*  GetNicCfg( const WORD index );
	void SetNicCfg( const WORD index, CNicCfg* nicCfg);
	void ClearNicCfg( const WORD index );
	
	//Primary media read path
	string GetPrimaryAudioFileReadPath() { return m_sPrimaryAudioFileReadPath;}
	string GetPrimaryVideoFileReadPath() { return m_sPrimaryVideoFileReadPath;}
	string GetPrimaryContentFileReadPath() { return m_sPrimaryContentFileReadPath;}
	string GetPrimaryDtmfFileReadPath() { return m_sPrimaryDtmfFileReadPath;}
	
	//Installation media read path
	string GetInstallationAudioFileReadPath() { return m_sInstallationAudioFileReadPath;}
	string GetInstallationVideoFileReadPath() { return m_sInstallationVideoFileReadPath;}
	string GetInstallationContentFileReadPath() { return m_sInstallationContentFileReadPath;}
	string GetInstallationDtmfFileReadPath() { return m_sInstallationDtmfFileReadPath;}
	
	//Write path
	string GetMediaFileWritePath() { return m_sMediaFileWritePath;}

	//maximize number of speaker
	int GetMaxNumerSpeaker() { return m_nMaxNumberSpeaker;}
	//switch interval
	int GetSwitchAudioInterval() { return m_nSwitchAudioInterval;}
	//Enable or disable the audio switch
	int GetStateSwitchAudio() { return m_nSwitchAudio;}
	
	//Video params
	int GetIntraDelayTimeResponse() { return m_intraDelayTimeResponse;}
	
	//Media Library
	CMediaLibrary* GetAudioLibrary() { return m_audioLibrary;}
	CMediaLibrary* GetVideoLibrary() { return m_videoLibrary;}
	CMediaLibrary* GetContentLibrary() { return m_contentLibrary;}
	
	void SetAudioLibrary(CMediaLibrary* audioLibrary);
	void SetVideoLibrary(CMediaLibrary* videoLibrary);
	void SetContentLibrary(CMediaLibrary* contentLibrary);
		

protected:

	// Attributes
	
	//Primary media read path
	string m_sPrimaryAudioFileReadPath;
	string m_sPrimaryVideoFileReadPath;
	string m_sPrimaryContentFileReadPath;
	string m_sPrimaryDtmfFileReadPath;
	
	//Installation media read path
	string m_sInstallationAudioFileReadPath;
	string m_sInstallationVideoFileReadPath;
	string m_sInstallationContentFileReadPath;
	string m_sInstallationDtmfFileReadPath;
	
	//Write path
	string m_sMediaFileWritePath;

	//maximize number of speaker
	int m_nMaxNumberSpeaker;
	//switch interval
	int m_nSwitchAudioInterval;	
	//Enable or disable the audio switch
	int m_nSwitchAudio;
	
	//Video params
	int m_intraDelayTimeResponse;
	
	
	//Media Library
	CMediaLibrary* m_audioLibrary;
	CMediaLibrary* m_videoLibrary;
	CMediaLibrary* m_contentLibrary;
	
	
	//Nic List
	WORD		m_wMaxNicSlots;
	CNicCfg**	m_ppNicArr;
	
};




class CMediaLibrary: public CSerializeObject
{
CLASS_TYPE_1(CMediaLibrary, CSerializeObject)

public:
	
	// Constructors
	CMediaLibrary();
	CMediaLibrary(channelMediaType mediaType);
	virtual ~CMediaLibrary();
	CMediaLibrary(const CMediaLibrary& other);
	CMediaLibrary& operator= (const CMediaLibrary& other);

	// Initializations

	// Operations
	const char*NameOf() const {return "CMediaLibrary";}
	CMediaLibrary* CreateCopy();
	

	// Utils
	channelMediaType GetMediaType() const { return m_eMediaType; }
	void SetMediaType(channelMediaType mediaType) { m_eMediaType = mediaType; }
	WORD GetFilesNum() const { return m_wFilesNum; }
	WORD GetCurrentFileIndex() const { return m_wCurrentFileIndex; }
	string GetNextMediaItemName();
	int GetMaxMediaFilesInLibrary() { return MAX_MEDIA_FILES_IN_LIBRARY;}

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	virtual CSerializeObject* Clone() { return NULL; }
	
	//void Dump();

protected:
	
	// Operations

	// Attributes
	channelMediaType m_eMediaType;
	WORD	m_wFilesNum;
	WORD	m_wCurrentFileIndex;
	string  m_strMediaList[MAX_MEDIA_FILES_IN_LIBRARY];
	
};




class CNicCfg : public CSerializeObject
{
CLASS_TYPE_1(CNicCfg, CSerializeObject)

public:
	
	// Constructors
	CNicCfg();
	virtual ~CNicCfg();
	CNicCfg(const CNicCfg& other);
	CNicCfg& operator= (const CNicCfg& other);

	// Initializations

	// Operations
	const char*NameOf() const {return "CNicCfg";}
	CNicCfg* CreateCopy();
	

	// Utils
	WORD	GetNicId() const      { return m_wNicId; }
	const char* GetNicIpAddress() const	{ return (const char*)m_szNicIp; }
	BOOL GetIsActive() { return m_bIsActive; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	virtual CSerializeObject* Clone() { return NULL; }

protected:
	
	// Operations

	// Attributes
	WORD	m_wNicId;
	char	m_szNicIp[IP_ADDRESS_STR_LEN];
	BOOL	m_bIsActive;
};




#endif /*MEDIAMNGRCFG_H_*/
