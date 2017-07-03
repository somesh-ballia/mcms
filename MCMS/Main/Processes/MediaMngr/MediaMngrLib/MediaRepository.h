#ifndef MEDIAREPOSITORY_H_
#define MEDIAREPOSITORY_H_

#include <map>
#include "PObject.h"
#include "DataTypes.h"
#include "DtmfAlgDB.h"
#include "MediaMngr.h"

using namespace std;

class CMediaDB;
class CMediaElement;


///////////////////////////
// Media Repository
///////////////////////////


class CMediaRepository : public CPObject
{
	CLASS_TYPE_1(CMediaRepository,CPObject)

public:
	CMediaRepository();
	virtual ~CMediaRepository();
	
	void Init();
	
	virtual const char*	NameOf () const{return "CMediaRepository";}
	
	
	CDtmfAlgDB* GetDtmfDB() { return m_pDtmfAlgDB;}
	CMediaDB* GetVideoDB() { return m_pVideoDB;}
	CMediaDB* GetContentDB() { return m_pContentDB;}
	CMediaDB* GetAudioDB() { return m_pAudioDB;}
	
	void PrintMediaRepository();
	
	
private:
	CDtmfAlgDB* m_pDtmfAlgDB;
	CMediaDB* m_pVideoDB;
	CMediaDB* m_pContentDB;
	CMediaDB* m_pAudioDB;
};



///////////////////////////
// Media DB
///////////////////////////
typedef map<string, CMediaElement*> MediaMap;

class CMediaDB : public CPObject
{
	CLASS_TYPE_1(CMediaDB,CPObject)
public:
	CMediaDB(EDBMedia dbMediaType);
	virtual ~CMediaDB();
	
	virtual const char*	NameOf () const{return "CMediaDB";}
	
	void Init();
	void PrintMediaDB();
	
	CMediaElement* GetMediaElement(const string mediaName);
	void AddMediaElement(CMediaElement* mediaElement);
	
		
protected:
	
	EDBMedia m_eDBMediaType;
	
	DWORD m_cacheSize; 		   // in Mbytes
	DWORD m_maxCacheFiles;
	DWORD m_maxCacheFileSize;  // in Mbytes
	DWORD m_maxCacheFilesSize; // in Mbytes
	
		
	MediaMap m_mediaMap;
};



///////////////////////////
// Media Element
///////////////////////////

class CMediaElement : public CPObject
{
	CLASS_TYPE_1(CMediaElement,CPObject)
	
public:
	CMediaElement();
	virtual ~CMediaElement();
	
	virtual const char*	NameOf () const{return "CMediaElement";}
	
	int LoadFile(string fileName);
	
	string GetName();
	DWORD GetSize();
	DWORD GetRefCounter();
	DWORD GetLastUsed();
	
	BYTE* GetDataBuffer();
	
	void IncRefCounter();
	void DecRefCounter();
		
protected:
	
	string m_fileName;
	DWORD m_fileSize; // in bytes
	DWORD m_refCounter;
	DWORD m_lastUsed;
	
	BYTE* m_dataBuffer;
};


#endif /*MEDIAREPOSITORY_H_*/
