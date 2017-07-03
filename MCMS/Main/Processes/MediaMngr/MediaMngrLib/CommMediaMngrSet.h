#ifndef COMMMEDIAMNGRSET_H_
#define COMMMEDIAMNGRSET_H_

#include "SerializeObject.h"

#include "MediaMngrCfg.h" 

#define MEDIA_FILE_NAME_LEN 	128
#define ENDPOINT_LIST_LEN		128
#define CHANNEL_TYPE_LEN		128



class CCommSetEPMediaParam : public CSerializeObject
{
public:
	CCommSetEPMediaParam();
	CCommSetEPMediaParam(const CCommSetEPMediaParam& other);
	virtual ~CCommSetEPMediaParam();
	CCommSetEPMediaParam& operator =(const CCommSetEPMediaParam& other);
	virtual const char*  NameOf() const { return "CCommSetEPMediaParam"; }
	virtual CSerializeObject* Clone() { return new CCommSetEPMediaParam; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	
	//methods
	DWORD GetMonitorPartyId() const { return m_monitorPartyId; }
	DWORD GetMonitorConfId() const { return m_monitorConfId; }
	
	const char* GetVideoFileName() { return m_szVideoFileName; }
	const char* GetAudioFileName() { return m_szAudioFileName; }
	const char* GetContentFileName() { return m_szContentFileName; }
	
	const char* GetSaveVideoFileName() { return m_szSaveVideoFileName; }
	const char* GetSaveAudioFileName() { return m_szSaveAudioFileName; }
	const char* GetSaveContentFileName() { return m_szSaveContentFileName; }
	
private:
	
	DWORD m_monitorPartyId;
	DWORD m_monitorConfId;
	
	char m_szVideoFileName[MEDIA_FILE_NAME_LEN];
	char m_szAudioFileName[MEDIA_FILE_NAME_LEN];
	char m_szContentFileName[MEDIA_FILE_NAME_LEN];
	
	char m_szSaveVideoFileName[MEDIA_FILE_NAME_LEN];
	char m_szSaveAudioFileName[MEDIA_FILE_NAME_LEN];
	char m_szSaveContentFileName[MEDIA_FILE_NAME_LEN];
};






class CCommResetChannelOut : public CSerializeObject
{
public:
	CCommResetChannelOut();
	CCommResetChannelOut(const CCommResetChannelOut& other);
	virtual ~CCommResetChannelOut();
	CCommResetChannelOut& operator =(const CCommResetChannelOut& other);
	virtual const char*  NameOf() const { return "CCommResetChannelOut"; }
	virtual CSerializeObject* Clone() { return new CCommResetChannelOut; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	
	//methods
	const char* GetEndPointList() { return m_szEndPointList; }
	const char* GetChannelType() { return m_szChannelType; }
	
private:	

	char m_szEndPointList[ENDPOINT_LIST_LEN];
	char m_szChannelType[CHANNEL_TYPE_LEN];
};





class CCommMediaLibrary : public CSerializeObject
{
public:
	CCommMediaLibrary();
	CCommMediaLibrary(const CCommMediaLibrary& other);
	virtual ~CCommMediaLibrary();
	CCommMediaLibrary& operator =(const CCommMediaLibrary& other);
	virtual const char*  NameOf() const { return "CCommMediaLibrary"; }
	virtual CSerializeObject* Clone() { return new CCommMediaLibrary; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	
	//methods
	CMediaLibrary* GetAudioLibrary() { return m_audioLibrary;}
	CMediaLibrary* GetVideoLibrary() { return m_videoLibrary;}
	CMediaLibrary* GetContentLibrary() { return m_contentLibrary;}
	
private:	

	//Media Library
	CMediaLibrary* m_audioLibrary;
	CMediaLibrary* m_videoLibrary;
	CMediaLibrary* m_contentLibrary;
};




#endif /*COMMMEDIAMNGRSET_H_*/
