/*
 * Media.h
 *
 *  Created on: Apr 23, 2012
 *      Author: bguelfand
 */

#ifndef MEDIA_H_
#define MEDIA_H_

#include "PObject.h"
#include <list>
#include <map>
#include "ConfPartyApiDefines.h"

#define	MEDIA_NEW		1
#define	MEDIA_MODIFIED	2
#define	MEDIA_DELETED	3

struct ResolutionElementType
{
	ResolutionElementType() { m_iWidth = -1; m_iHeight = -1; }
	ResolutionElementType(int iWidth, int iHeight) { m_iWidth = iWidth; m_iHeight = iHeight; }
	int m_iWidth;
	int m_iHeight;
};

struct RateElementType
{
	RateElementType() { m_iMax = -1; m_iActual = -1; }
	RateElementType(int iMax, int iActual) { m_iMax = iMax; m_iActual = iActual; }
	int m_iMax;
	int m_iActual;
};

enum EMediaRole
{
	eMediaRoleUndefined,

	eMediaRolePeople,
	eMediaRoleContent,

	eMediaRoleMaxNumOfValues
};

enum EMediaStatus
{
	eMediaStatusUndefined,

	eMediaStatusSendOnly,
	eMediaStatusRecvOnly,
	eMediaStatusSendRecv,
	eMediaStatusInactive,

	eMediaStatusMaxNumOfValues
};
enum EMediaType
{
	eMediaTypeUndefined,

	eMediaAudio,
	eMediaVoip,
	eMediaVideo,
	eMediaData,

	eMediaTypeMaxNumOfValues
};

enum EMediaAvailable
{
	eMediaAvailableUndefined,

	eAvailableFalse,
	eAvailableTrue,

	eMediaAvailableMaxNumOfValues
};

class CSegment;

struct MEDIA_DATA
{
	DWORD					m_dwID;
	DWORD					m_dwSsrcID;
	ResolutionElementType	m_maxResolution;
	ResolutionElementType	m_actualResolution;
	RateElementType			m_frameRate;
	RateElementType			m_bitRate;
	EMediaRole				m_eMediaRole;
	ECodecSubType			m_eCodec;
	EMediaAvailable			m_eAvailable;

//	DWORD m_idAttribute;
	EMediaStatus			m_eMediaStatus;
	EMediaType				m_eMediaType;
	bool					m_bStereo;

	bool					m_bMediaDeleted;
	bool					m_bReportedToEventPkgOnMediaDeleted;
	bool					m_bUrgent;


	void Serialize(CSegment* pSeg) const;
	void DeSerialize(CSegment* pSeg);
};

class CXMLDOMElement;

struct MODIFICATION
{
	int maxResolutionWidth		: 1;
	int maxResolutionHeight		: 1;
	int actualResolutionWidth	: 1;
	int actualResolutionHeight	: 1;
	int frameRateMax			: 1;
	int frameRateActual			: 1;
	int bitRateMax				: 1;
	int bitRateActual			: 1;
	int eMediaRole				: 1;
	int eCodec					: 1;
	int eAvailable				: 1;
	int eMediaStatus			: 1;
	int eMediaType				: 1;
	int bStereo					: 1;
	int bDeleted				: 1;

	void Set()
	{
		maxResolutionWidth		= 1;
		maxResolutionHeight		= 1;
		actualResolutionWidth	= 1;
		actualResolutionHeight	= 1;
		frameRateMax			= 1;
		frameRateActual			= 1;
		bitRateMax				= 1;
		bitRateActual			= 1;
		eMediaRole				= 1;
		eCodec					= 1;
		eAvailable				= 1;
		eMediaStatus			= 1;
		eMediaType				= 1;
		bStereo					= 1;
		bDeleted				= 1;
	}
	void Reset()
	{
		maxResolutionWidth		= 0;
		maxResolutionHeight		= 0;
		actualResolutionWidth	= 0;
		actualResolutionHeight	= 0;
		frameRateMax			= 0;
		frameRateActual			= 0;
		bitRateMax				= 0;
		bitRateActual			= 0;
		eMediaRole				= 0;
		eCodec					= 0;
		eAvailable				= 0;
		eMediaStatus			= 0;
		eMediaType				= 0;
		bStereo					= 0;
		bDeleted				= 0;
	}
	MODIFICATION()
	{
		Reset();
	}

	bool IsModified()
	{
		return	maxResolutionWidth		||
				maxResolutionHeight		||
				actualResolutionWidth	||
				actualResolutionHeight	||
				frameRateMax			||
				frameRateActual			||
				bitRateMax				||
				bitRateActual			||
				eMediaRole				||
				eCodec					||
				eAvailable				||
				eMediaStatus			||
				eMediaType				||
				bStereo					||
				bDeleted;
	}
};

class CMedia : public MEDIA_DATA, public CPObject
{
	CLASS_TYPE_1(CMedia, CPObject )

public:
	CMedia();
	CMedia(DWORD dwID, DWORD dwSsrcID);
	virtual const char* NameOf() const { return "CMedia"; }

	CMedia& operator=(const CMedia &other);
	bool	operator==(const CMedia &other);
	void	SerializeXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, bool bFull = true);
	std::string ToString() const;
	bool	IsModified();
	void	CheckModification(const CMedia& other);
	void	Merge(const CMedia& other);
	void	SetMediaUpdated();
	void SerializeXmlApi(CXMLDOMElement* pParentNode, bool bFull = true);

protected:
	void SerializeMediaTypeXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName);
	void SerializeMediaTypeXmlApi(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName);
	void SerializeMediaStatusXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName);
	void SerializeMediaRoleXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName);
	void SerializeMediaCodecXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName);

public:
	MODIFICATION m_stModification;
};

class CMediaList : public CPObject
{
	CLASS_TYPE_1(CMediaList, CPObject )

public:
	CMediaList();
	virtual const char* NameOf() const { return "CMediaList"; }

	int	GetListMedia(std::list<CMedia>& listMedia);
	const std::list<CMedia>* GetListMedia() const { return &m_listMedia; }
	void SetMedia(const CMediaList* pMediaList);
	void SetMedia(CMediaList& mediaList);
	void SetMedia(std::list<CMedia>& listMedia);
	void ReplaceMedia(CMediaList& listMedia);
	void ReplaceMedia(std::list<CMedia>& listMedia, bool bUrgent = false);
	void ReplaceMediaVideoIn(std::list<CMedia>& listMedia, bool bUrgent = false);
	void ReplaceMediaVideoOut(std::list<CMedia>& listMedia, bool bUrgent = false);
	void ReplaceMediaAudioIn(std::list<CMedia>& listMedia, bool bUrgent = false);
	void ReplaceMediaAudiOut(std::list<CMedia>& listMedia, bool bUrgent = false);
	void ReplaceMediaContentVideoIn(std::list<CMedia>& listMedia, bool bUrgent = false);
	void ReplaceMediaContentVideoOut(std::list<CMedia>& listMedia, bool bUrgent = false);
	void SetMediaDeleted(std::list<unsigned int> listMediaID, bool bUrgent = false);
	void RemoveDeletedMedia();
	void SetMediaUpdated();
	bool IsUrgent() const;
	bool IsNewList() { return m_bNewList; }
	bool Merge(const CMediaList* pMediaList);
	int	 GetSize() const { return m_listMedia.size(); }
	int	 GetVideoStreamsCount();

	bool operator==(const CMediaList &other);

	void SerializeXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, bool bFull, bool bNew);
	void Serialize(CSegment* pSeg) const;
	void DeSerialize(CSegment* pSeg);
	std::string ToString() const;
	int GetListMediaSize();
	void SerializeXmlApi(CXMLDOMElement* pParentNode, bool bFull = true);


protected:
	void ReplaceMedia(std::list<CMedia>& listMedia,
			EMediaStatus eMediaDirectionType, EMediaType eMediaType, EMediaRole eMediaRoleType, bool bUrgent);

	bool				m_bNewList;
	std::list<CMedia>	m_listMedia;
	std::map<DWORD, BYTE>	m_mapMediaState;

private:
	CMediaList& operator=(const CMediaList& other){return *this;}
};

#endif /* MEDIA_H_ */
