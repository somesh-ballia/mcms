/*
 * Media.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: bguelfand
 */

#include <stdio.h>
#include "Media.h"
#include "Segment.h"
#include "psosxml.h"
#include "StlUtils.h"
#include "TraceStream.h"
#include "ConfPartyDefines.h"

void MEDIA_DATA::Serialize(CSegment* pSeg) const
{
	pSeg->Put((BYTE*)this, sizeof(MEDIA_DATA));
}
void MEDIA_DATA::DeSerialize(CSegment* pSeg)
{
	pSeg->Get((BYTE*)this, sizeof(MEDIA_DATA));
}

CMedia::CMedia()
{
	m_dwID = 0;
	m_dwSsrcID = 0;

	m_eMediaRole = eMediaRoleUndefined;
	m_eCodec = eCodecSubTypeUndefined;
	m_eAvailable = eMediaAvailableUndefined;

	m_eMediaStatus = eMediaStatusUndefined;
	m_eMediaType = eMediaTypeUndefined;
	m_bStereo = false;

	m_bMediaDeleted = false;
	m_bReportedToEventPkgOnMediaDeleted = false;
	m_bUrgent = false;
}

CMedia::CMedia(DWORD dwID, DWORD dwSsrcID)
{
	m_dwID = dwID;
	m_dwSsrcID = dwSsrcID;

	m_eMediaRole = eMediaRoleUndefined;
	m_eCodec = eCodecSubTypeUndefined;
	m_eAvailable = eMediaAvailableUndefined;

	m_eMediaStatus = eMediaStatusUndefined;
	m_eMediaType = eMediaTypeUndefined;
	m_bStereo = false;

	m_bMediaDeleted = false;
	m_bReportedToEventPkgOnMediaDeleted = false;
	m_bUrgent = false;
}

CMedia& CMedia::operator=(const CMedia &other)
{
	m_dwID = other.m_dwID;
	m_dwSsrcID = other.m_dwSsrcID;

	if (other.m_eMediaRole != eMediaRoleUndefined)
		m_eMediaRole = other.m_eMediaRole;
	if (other.m_eCodec != eCodecSubTypeUndefined)
		m_eCodec = other.m_eCodec;
	if (other.m_eAvailable != eMediaAvailableUndefined)
		m_eAvailable = other.m_eAvailable;
	if (other.m_eMediaStatus != eMediaStatusUndefined)
		m_eMediaStatus = other.m_eMediaStatus;
	if (other.m_eMediaType != eMediaTypeUndefined)
		m_eMediaType = other.m_eMediaType;

	if (other.m_maxResolution.m_iWidth >= 0)
		m_maxResolution.m_iWidth = other.m_maxResolution.m_iWidth;
	if (other.m_maxResolution.m_iHeight >= 0)
		m_maxResolution.m_iHeight = other.m_maxResolution.m_iHeight;

	if (other.m_actualResolution.m_iWidth >= 0)
		m_actualResolution.m_iWidth = other.m_actualResolution.m_iWidth;
	if (other.m_actualResolution.m_iHeight >= 0)
		m_actualResolution.m_iHeight = other.m_actualResolution.m_iHeight;

	if (other.m_frameRate.m_iMax >= 0)
		m_frameRate.m_iMax = other.m_frameRate.m_iMax;
	if (other.m_frameRate.m_iActual >= 0)
		m_frameRate.m_iActual = other.m_frameRate.m_iActual;

	if (other.m_bitRate.m_iMax >= 0)
		m_bitRate.m_iMax = other.m_bitRate.m_iMax;
	if (other.m_bitRate.m_iActual >= 0)
		m_bitRate.m_iActual = other.m_bitRate.m_iActual;
	m_bStereo = other.m_bStereo;

	m_bMediaDeleted = other.m_bMediaDeleted;
	m_bReportedToEventPkgOnMediaDeleted = other.m_bReportedToEventPkgOnMediaDeleted;
	m_bUrgent = other.m_bUrgent;

	return *this;
}
bool CMedia::operator==(const CMedia &other)
{
	return (this->m_dwID == other.m_dwID &&
			this->m_dwSsrcID == other.m_dwSsrcID &&
			this->m_eMediaRole == other.m_eMediaRole &&
			this->m_eCodec == other.m_eCodec &&
			this->m_eAvailable == other.m_eAvailable &&
			this->m_eMediaStatus == other.m_eMediaStatus &&
			this->m_eMediaType == other.m_eMediaType &&

			this->m_maxResolution.m_iWidth == other.m_maxResolution.m_iWidth &&
			this->m_maxResolution.m_iHeight == other.m_maxResolution.m_iHeight &&

			this->m_actualResolution.m_iWidth == other.m_actualResolution.m_iWidth &&
			this->m_actualResolution.m_iHeight == other.m_actualResolution.m_iHeight &&

			this->m_frameRate.m_iActual == other.m_frameRate.m_iActual &&
			this->m_frameRate.m_iMax == other.m_frameRate.m_iMax &&

			this->m_bitRate.m_iActual == other.m_bitRate.m_iActual &&
			this->m_bitRate.m_iMax == other.m_bitRate.m_iMax &&

			this->m_bMediaDeleted == other.m_bMediaDeleted &&
			this->m_bReportedToEventPkgOnMediaDeleted == other.m_bReportedToEventPkgOnMediaDeleted
	);
}

void CMedia::SerializeMediaTypeXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName)
{
	if (m_eMediaType > eMediaTypeUndefined)
	{
		std::string sMediaType;
		switch(m_eMediaType)
		{
			case(eMediaVideo):
				sMediaType = "video";
				break;
			case(eMediaAudio):
			default:
				sMediaType = "audio";
				break;
		}
		pParentNode->AddChildNodeWithNsPrefix(pszAttributeName, pszNameSpacePrefix, sMediaType.c_str());
	}
}

void CMedia::SerializeMediaTypeXmlApi(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName)
{
	if (m_eMediaType > eMediaTypeUndefined)
	{
		std::string sMediaType;
		switch(m_eMediaType)
		{
			case(eMediaVideo):
				sMediaType = "VIDEO";
				break;
			case(eMediaAudio):
			default:
				sMediaType = "AUDIO";
				break;
		}
		pParentNode->AddChildNodeWithNsPrefix(pszAttributeName, pszNameSpacePrefix, sMediaType.c_str());
	}
}

void CMedia::SerializeMediaStatusXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName)
{
	if (m_eMediaStatus > eMediaStatusUndefined)
	{
		std::string sMediaStatus;
		switch(m_eMediaStatus)
		{
			case(eMediaStatusSendOnly):
				sMediaStatus = "sendonly";
				break;
			case(eMediaStatusRecvOnly):
				sMediaStatus = "recvonly";
				break;
			case(eMediaStatusSendRecv):
				sMediaStatus = "sendrecv";
				break;
			case(eMediaStatusInactive):
			default:
				sMediaStatus = "inactive";
				break;
		}
		pParentNode->AddChildNodeWithNsPrefix(pszAttributeName, pszNameSpacePrefix, sMediaStatus.c_str());
	}
}
void CMedia::SerializeMediaRoleXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName)
{
	if (m_eMediaRole > eMediaRoleUndefined)
	{
		std::string sMediaRole;
		switch(m_eMediaType)
		{
			case(eMediaRoleContent):
				sMediaRole = "content";
				break;
			case(eMediaRolePeople):
			default:
				sMediaRole = "people";
				break;
		}
		pParentNode->AddChildNodeWithNsPrefix(pszAttributeName, pszNameSpacePrefix, sMediaRole.c_str());
	}
}

void CMedia::SerializeMediaCodecXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, const char* pszAttributeName)
{
	if (m_eCodec > eCodecSubTypeUndefined && m_eCodec < eCodecSubTypeMaxNumOfValues)
	{
		//eH264, eH264SVC, eSAC, eG711, eG722, eG722_1
		std::string sCodec;
		switch(m_eCodec)
		{
			case(eH264):
				sCodec = "H264";
				break;
			case(eH264SVC):
				sCodec = "H264SVC";
				break;
			case(eSAC):
				sCodec = "SAC";
				break;
			case(eG711):
				sCodec = "G711";
				break;
			case(eG722):
				sCodec = "G722";
				break;
			case(eG722_1):
				sCodec = "G722_1";
				break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
		pParentNode->AddChildNodeWithNsPrefix(pszAttributeName, pszNameSpacePrefix, sCodec.c_str());
	}
}

void CMedia::SerializeXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, bool bFull /*= true*/)
{
	if (! bFull && ! m_bMediaDeleted && ! IsModified())
		return;
	if (bFull && m_bMediaDeleted)
		return;
	if (m_bReportedToEventPkgOnMediaDeleted)
		return;

	CXMLDOMElement*  pFeatureNode = NULL;

	pFeatureNode = pParentNode->AddChildNodeWithNsPrefix("media", pszNameSpacePrefix);
	char buff[20];
	snprintf(buff, sizeof(buff), "%d", m_dwID);
	pFeatureNode->AddAttribute("id", buff);
	if (m_bMediaDeleted)
	{
		pFeatureNode->AddAttribute("state", "deleted");
	}
	else
	{
		pFeatureNode->AddAttribute("state", bFull ? "full" : "partial");

		if (bFull || m_stModification.eMediaType)
			SerializeMediaTypeXml(pFeatureNode, "", "type");

		pFeatureNode->AddChildNodeWithNsPrefix("src-id", "", m_dwSsrcID);
		CXMLDOMAttribute* pAttribute = NULL;
		if (m_eMediaType == eMediaAudio)
		{
			if (bFull)
				pFeatureNode->AddChildNodeWithNsPrefix("stereo", "mrc:", "false");
			if (bFull || m_stModification.eCodec)
				SerializeMediaCodecXml(pFeatureNode, "mrc:", "codec");
		}
		else if (m_eMediaType == eMediaVideo)
		{
			if ((bFull || m_stModification.maxResolutionWidth || m_stModification.maxResolutionHeight) && (m_maxResolution.m_iWidth >= 0 || m_maxResolution.m_iHeight >= 0))
			{
				CXMLDOMElement*  pMaxResNode = pFeatureNode->AddChildNodeWithNsPrefix("max-resolution", "mrc:");
				if ((bFull || m_stModification.maxResolutionWidth) && m_maxResolution.m_iWidth >= 0)
				{
					snprintf(buff, sizeof(buff), "%d", m_maxResolution.m_iWidth);
					pMaxResNode->AddAttribute("width", buff);
				}
				if ((bFull || m_stModification.maxResolutionHeight) && m_maxResolution.m_iHeight >= 0)
				{
					snprintf(buff, sizeof(buff), "%d", m_maxResolution.m_iHeight);
					pMaxResNode->AddAttribute("height", buff);
				}
			}

			if ((bFull || m_stModification.actualResolutionWidth || m_stModification.actualResolutionHeight) && (m_actualResolution.m_iWidth >= 0 || m_actualResolution.m_iHeight >= 0))
			{
				CXMLDOMElement*  pActResNode = pFeatureNode->AddChildNodeWithNsPrefix("actual-resolution", "mrc:");
				if ((bFull || m_stModification.actualResolutionWidth) && m_actualResolution.m_iWidth >= 0)
				{
					snprintf(buff, sizeof(buff), "%d", m_actualResolution.m_iWidth);
					pActResNode->AddAttribute("width", buff);
				}
				if ((bFull || m_stModification.actualResolutionHeight) && m_actualResolution.m_iHeight >= 0)
				{
					snprintf(buff, sizeof(buff), "%d", m_actualResolution.m_iHeight);
					pActResNode->AddAttribute("height", buff);
				}
			}

			if ((bFull || m_stModification.frameRateActual || m_stModification.frameRateMax ) && (m_frameRate.m_iActual >= 0 || m_frameRate.m_iMax >= 0))
			{
				CXMLDOMElement*  pFrameRateNode = pFeatureNode->AddChildNodeWithNsPrefix("frame-rate", "mrc:");
				if ((bFull || m_stModification.frameRateActual) && m_frameRate.m_iActual >= 0)
				{
					snprintf(buff, sizeof(buff), "%d", m_frameRate.m_iActual);
					pFrameRateNode->AddAttribute("actual", buff);
				}
				if ((bFull || m_stModification.frameRateMax) && m_frameRate.m_iMax >= 0)
				{
					snprintf(buff, sizeof(buff), "%d", m_frameRate.m_iMax);
					pFrameRateNode->AddAttribute("max", buff);
				}
			}

			if (bFull || m_stModification.eMediaRole)
				SerializeMediaRoleXml(pFeatureNode, "mrc:", "media-role");
			if (bFull || m_stModification.eCodec)
				SerializeMediaCodecXml(pFeatureNode, "mrc:", "codec");
		}
		if (bFull)
			pFeatureNode->AddChildNodeWithNsPrefix("available", "mrc:", "true");

		if ((bFull || m_stModification.bitRateActual || m_stModification.bitRateMax) && (m_bitRate.m_iActual >= 0 || m_bitRate.m_iMax >= 0))
		{
			CXMLDOMElement*  pBitrateNode = pFeatureNode->AddChildNodeWithNsPrefix("bit-rate", "mrc:");
			if ((bFull || m_stModification.bitRateActual) && m_bitRate.m_iActual >= 0)
			{
				snprintf(buff, sizeof(buff), "%d", m_bitRate.m_iActual);
				pBitrateNode->AddAttribute("actual", buff);
			}
			if ((bFull || m_stModification.bitRateMax) && m_bitRate.m_iMax >= 0)
			{
				snprintf(buff, sizeof(buff), "%d", m_bitRate.m_iMax);
				pBitrateNode->AddAttribute("max", buff);
			}
		}
		if(bFull || (m_eMediaType == eMediaAudio && m_stModification.bStereo))
			CXMLDOMElement*  pStereoNode = pFeatureNode->AddChildNodeWithNsPrefix("stereo", "mrc:", (m_bStereo ? "true" : "false"));

		if (bFull || m_stModification.eMediaStatus)
			SerializeMediaStatusXml(pFeatureNode, "", "status");
	}
}

bool CMedia::IsModified()
{
	return m_stModification.IsModified();
}
void CMedia::CheckModification(const CMedia& other)
{
	m_stModification.maxResolutionWidth = int(m_maxResolution.m_iWidth != other.m_maxResolution.m_iWidth);
	m_stModification.maxResolutionHeight = int(m_maxResolution.m_iHeight != other.m_maxResolution.m_iHeight);
	m_stModification.actualResolutionWidth = int(m_actualResolution.m_iWidth != other.m_actualResolution.m_iWidth);
	m_stModification.actualResolutionHeight = int(m_actualResolution.m_iHeight != other.m_actualResolution.m_iHeight);
	m_stModification.frameRateMax = int(m_frameRate.m_iMax != other.m_frameRate.m_iMax);
	m_stModification.frameRateActual = int(m_frameRate.m_iActual != other.m_frameRate.m_iActual);
	m_stModification.bitRateMax = int(m_bitRate.m_iMax != other.m_bitRate.m_iMax);
	m_stModification.bitRateActual = int(m_bitRate.m_iActual != other.m_bitRate.m_iActual);
	m_stModification.eMediaRole = int(m_eMediaRole != other.m_eMediaRole);
	m_stModification.eCodec = int(m_eCodec != other.m_eCodec);
	m_stModification.eAvailable = int(m_eAvailable != other.m_eAvailable);
	m_stModification.eMediaStatus = int(m_eMediaStatus != other.m_eMediaStatus);
	m_stModification.eMediaType = int(m_eMediaType != other.m_eMediaType);
	m_stModification.bStereo = int(m_bStereo != other.m_bStereo);
	m_stModification.bDeleted = int(m_bMediaDeleted != other.m_bMediaDeleted);
}

void CMedia::Merge(const CMedia& other)
{
	if (m_bMediaDeleted != other.m_bMediaDeleted)
	{
		if (m_bMediaDeleted == true)
			m_stModification.Set();
		m_bMediaDeleted = other.m_bMediaDeleted;
		m_stModification.bDeleted = true;
	}
//	if (other.m_bReportedToEventPkgOnMediaDeleted)
//		m_bReportedToEventPkgOnMediaDeleted = true;
	m_bReportedToEventPkgOnMediaDeleted = other.m_bReportedToEventPkgOnMediaDeleted;

	if (m_maxResolution.m_iWidth != other.m_maxResolution.m_iWidth)
	{
		m_maxResolution.m_iWidth = other.m_maxResolution.m_iWidth;
		m_stModification.maxResolutionWidth = 1;
	}
	if (m_maxResolution.m_iHeight != other.m_maxResolution.m_iHeight)
	{
		m_maxResolution.m_iHeight = other.m_maxResolution.m_iHeight;
		m_stModification.maxResolutionHeight = 1;
	}
	if (m_actualResolution.m_iWidth != other.m_actualResolution.m_iWidth)
	{
		m_actualResolution.m_iWidth = other.m_actualResolution.m_iWidth;
		m_stModification.actualResolutionWidth = 1;
	}
	if (m_actualResolution.m_iHeight != other.m_actualResolution.m_iHeight)
	{
		m_actualResolution.m_iHeight = other.m_actualResolution.m_iHeight;
		m_stModification.actualResolutionHeight = 1;
	}
	if (m_frameRate.m_iMax != other.m_frameRate.m_iMax)
	{
		m_frameRate.m_iMax = other.m_frameRate.m_iMax;
		m_stModification.frameRateMax = 1;
	}
	if (m_frameRate.m_iActual != other.m_frameRate.m_iActual)
	{
		m_frameRate.m_iActual = other.m_frameRate.m_iActual;
		m_stModification.frameRateActual = 1;
	}
	if (m_bitRate.m_iMax != other.m_bitRate.m_iMax)
	{
		m_bitRate.m_iMax = other.m_bitRate.m_iMax;
		m_stModification.bitRateMax = 1;
	}
	if (m_bitRate.m_iActual != other.m_bitRate.m_iActual)
	{
		m_bitRate.m_iActual = other.m_bitRate.m_iActual;
		m_stModification.bitRateActual = 1;
	}
	if (m_eMediaRole != other.m_eMediaRole)
	{
		m_eMediaRole = other.m_eMediaRole;
		m_stModification.eMediaRole = 1;
	}
	if (m_eCodec != other.m_eCodec)
	{
		m_eCodec = other.m_eCodec;
		m_stModification.eCodec = 1;
	}
	if (m_eAvailable != other.m_eAvailable)
	{
		m_eAvailable = other.m_eAvailable;
		m_stModification.eAvailable = 1;
	}
	if (m_eMediaStatus != other.m_eMediaStatus)
	{
		m_eMediaStatus = other.m_eMediaStatus;
		m_stModification.eMediaStatus = 1;
	}
	if (m_eMediaType != other.m_eMediaType)
	{
		m_eMediaType = other.m_eMediaType;
		m_stModification.eMediaStatus = 1;
	}
	if (m_bStereo != other.m_bStereo)
	{
		m_bStereo = other.m_bStereo;
		m_stModification.bStereo = 1;
	}
}
void CMedia::SetMediaUpdated()
{
	m_stModification.Reset();
	m_bUrgent = false;
}

void CMedia::SerializeXmlApi(CXMLDOMElement* pParentNode, bool bFull /*= true*/)
{
	CXMLDOMElement*  pFeatureNode = NULL;

//	if(bFull || eNoChange != m_state)
	{
		pFeatureNode = pParentNode->AddChildNode("RELAY_MEDIA");

		pFeatureNode->AddChildNode("SRC_ID", m_dwSsrcID);

		if ( /*! bFull &&*/ m_bMediaDeleted)
		{
			pFeatureNode->AddChildNode("STATE","deleted");
			return;
		}
		pFeatureNode->AddChildNode("STATE",(bFull ? "full" : "partial"));

		SerializeMediaTypeXmlApi(pFeatureNode, "", "TYPE");

		SerializeMediaCodecXml(pFeatureNode, "", "CODEC");

		if (m_eMediaType == eMediaAudio)
		{
			// pFeatureNode->AddChildNode("STEREO", "false");
		}
		else if (m_eMediaType == eMediaVideo)
		{
			if (m_maxResolution.m_iWidth >= 0 && m_maxResolution.m_iHeight >= 0)
			{
				CXMLDOMElement*  pMaxResNode = pFeatureNode->AddChildNode("RELAY_MAX_RESOLUTION");
				pMaxResNode->AddChildNode("RESOLUTION_WIDTH", m_maxResolution.m_iWidth);
				pMaxResNode->AddChildNode("RESOLUTION_HEIGHT", m_maxResolution.m_iHeight);
			}

			if (m_frameRate.m_iMax  >= 0)
			{
				pFeatureNode->AddChildNode("RELAY_MAX_FRAME_RATE", m_frameRate.m_iMax);
			}

			SerializeMediaRoleXml(pFeatureNode, "", "MEDIA_ROLE");
		}

		pFeatureNode->AddChildNode("AVAILABLE", "true");

		if (m_bitRate.m_iMax >= 0)
		{
			pFeatureNode->AddChildNode("RELAY_MAX_BIT_RATE", m_bitRate.m_iMax);
		}

		SerializeMediaStatusXml(pFeatureNode, "", "MEDIA_STATUS");
	}
}


std::string CMedia::ToString() const
{
	std::string sRetVal = "Media: ";
	char buff[50] = "";
	snprintf(buff, sizeof(buff), "MediaID = %d", m_dwID);
	sRetVal.append(buff);
	snprintf(buff, sizeof(buff), ", SsrcID = %d", m_dwSsrcID);
	sRetVal.append(buff);
	snprintf(buff, sizeof(buff), ", Frame-rate = %d/%d", m_frameRate.m_iActual, m_frameRate.m_iMax);
	sRetVal.append(buff);
	snprintf(buff, sizeof(buff), ", Bit-rate = %d/%d", m_bitRate.m_iActual, m_bitRate.m_iMax);
	sRetVal.append(buff);
	snprintf(buff, sizeof(buff), ", IsDeleted = %s", m_bMediaDeleted ? "true" : "false");
	sRetVal.append(buff);
	snprintf(buff, sizeof(buff), ", IsReportedToEventPkgOnMediaDeleted = %s", m_bReportedToEventPkgOnMediaDeleted ? "true" : "false");
	sRetVal.append(buff);
	if(m_eMediaType == eMediaAudio)
	{
		snprintf(buff, sizeof(buff), ", Stereo = %d", (int)m_bStereo);
		sRetVal.append(buff);
	}
	sRetVal.append("\n");
	return sRetVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

CMediaList::CMediaList()
{
	m_bNewList = false;
}

int CMediaList::GetListMedia(std::list<CMedia>& listMedia)
{
	listMedia = m_listMedia;
	return listMedia.size();
}

void CMediaList::SetMedia(const CMediaList* pMediaList)
{
	m_listMedia = pMediaList->m_listMedia;
	m_bNewList = true;
}

void CMediaList::SetMedia(CMediaList& mediaList)
{
	mediaList.SetMedia(m_listMedia);
	m_bNewList = true;
}

void CMediaList::SetMedia(std::list<CMedia>& listMedia)
{
	m_listMedia = listMedia;
	m_bNewList = true;
}

bool CMediaList::Merge(const CMediaList* pMediaList)
{
	if (m_listMedia.empty())
	{
		SetMedia(pMediaList);
		
		for (std::list<CMedia>::iterator it = m_listMedia.begin(); it != m_listMedia.end(); ++it)
			m_mapMediaState[it->m_dwID] = MEDIA_NEW;
	}
	else
	{
		const std::list<CMedia>* pListNew = pMediaList->GetListMedia();

		for (std::list<CMedia>::const_iterator itNew = pListNew->begin(); itNew != pListNew->end(); ++itNew)
		{
			bool bNew = true;
			
			for (std::list<CMedia>::iterator it = m_listMedia.begin(); it != m_listMedia.end(); ++it)
			{
				if (it->m_dwID == itNew->m_dwID)
				{
					if (itNew->m_bMediaDeleted)
					{
						if (it->m_bMediaDeleted == false)
						{
							it->m_bMediaDeleted = true;
							m_mapMediaState[it->m_dwID] = MEDIA_DELETED;
						}
					}
					else
					{
						it->Merge(*itNew);
						if (it->IsModified())
							m_mapMediaState[it->m_dwID] = MEDIA_MODIFIED;
					}
					
					bNew = false;
					
					break;
				}
			}
			
			if (bNew)
			{
				m_listMedia.push_back(*itNew);
				m_listMedia.back().m_stModification.Set();
				m_mapMediaState[m_listMedia.back().m_dwID] = MEDIA_NEW;
			}
		}
	}
	
	return m_mapMediaState.size() > 0;
}

int	 CMediaList::GetVideoStreamsCount()
{
	int n = 0;
	std::list<CMedia>::iterator it = m_listMedia.begin();
	for ( ; it != m_listMedia.end(); ++it )
	{
		if (it->m_eMediaType == eMediaVideo)
			++n;
	}
	return n;
}

bool CMediaList::operator==(const CMediaList& other)
{
	if (GetSize() != other.GetSize())
		return false;
	return true;
}

void CMediaList::ReplaceMedia(CMediaList& listMedia)
{
	const std::list<CMedia>* pListNew = listMedia.GetListMedia();
	std::list<CMedia>::const_iterator itNew = pListNew->begin();
	for ( ; itNew != pListNew->end(); ++itNew )
	{
		std::list<CMedia>::iterator itMedia = m_listMedia.begin();
		for ( ; itMedia != m_listMedia.end(); ++itMedia )
		{
			if (itMedia->m_dwID == itNew->m_dwID)
			{
				m_listMedia.erase(itMedia);
				break;
			}
		}
	}
	m_listMedia.insert(m_listMedia.end(), pListNew->begin(), pListNew->end());
}

void CMediaList::ReplaceMedia(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	std::list<CMedia>::iterator itNew = listMedia.begin();
	for ( ; itNew != listMedia.end(); ++itNew )
	{
		itNew->m_bUrgent = itNew->m_bUrgent || bUrgent;
		std::list<CMedia>::iterator itMedia = m_listMedia.begin();
		for ( ; itMedia != m_listMedia.end(); ++itMedia )
		{
			if (itMedia->m_dwID == itNew->m_dwID)
			{
				m_listMedia.erase(itMedia);
				break;
			}
		}
	}
	m_listMedia.insert(m_listMedia.end(), listMedia.begin(), listMedia.end());
	m_bNewList = false;
}

void CMediaList::ReplaceMediaVideoIn(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	ReplaceMedia(listMedia, eMediaStatusSendOnly, eMediaVideo, eMediaRolePeople, bUrgent);
}
void CMediaList::ReplaceMediaVideoOut(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	ReplaceMedia(listMedia, eMediaStatusRecvOnly, eMediaVideo, eMediaRolePeople, bUrgent);
}
void CMediaList::ReplaceMediaAudioIn(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	ReplaceMedia(listMedia, eMediaStatusSendOnly, eMediaAudio, eMediaRoleUndefined, bUrgent);
}
void CMediaList::ReplaceMediaAudiOut(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	ReplaceMedia(listMedia, eMediaStatusRecvOnly, eMediaAudio, eMediaRoleUndefined, bUrgent);
}
void CMediaList::ReplaceMediaContentVideoIn(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	ReplaceMedia(listMedia, eMediaStatusSendOnly, eMediaVideo, eMediaRoleContent, bUrgent);
}
void CMediaList::ReplaceMediaContentVideoOut(std::list<CMedia>& listMedia, bool bUrgent /*= false*/)
{
	ReplaceMedia(listMedia, eMediaStatusRecvOnly, eMediaVideo, eMediaRoleContent, bUrgent);
}
void CMediaList::SetMediaDeleted(std::list<unsigned int> listMediaID, bool bUrgent /*= false*/)
{
	std::list<unsigned int>::iterator it = listMediaID.begin();
	for ( ; it != listMediaID.end(); ++it )
	{
		std::list<CMedia>::iterator itMedia = m_listMedia.begin();
		for ( ; itMedia != m_listMedia.end(); ++itMedia )
		{
			if (itMedia->m_dwID == *it)
			{
				itMedia->m_bMediaDeleted = true;
				itMedia->m_bUrgent = bUrgent;
				break;
			}
		}
	}
}
void CMediaList::RemoveDeletedMedia()
{
	std::list<CMedia>::iterator it = m_listMedia.begin();
	while (it != m_listMedia.end())
	{
		it->m_bUrgent = false;
		if (it->m_bMediaDeleted)
		{
			it->m_bReportedToEventPkgOnMediaDeleted = true;
			++it;
			//m_listMedia.erase(it++);
		}
		else
			++it;
	}
}
void CMediaList::SetMediaUpdated()
{
	std::list<CMedia>::iterator it = m_listMedia.begin();
	while (it != m_listMedia.end())
	{
		if (it->m_bMediaDeleted)
		{
			it->m_bReportedToEventPkgOnMediaDeleted = true;
			++it;
			//m_listMedia.erase(it++);
		}
		else
		{
			it->SetMediaUpdated();
			++it;
		}
	}
	m_mapMediaState.clear();
}

void CMediaList::ReplaceMedia(std::list<CMedia>& listMedia,	EMediaStatus eMediaDirectionType, EMediaType eMediaType, EMediaRole eMediaRoleType, bool bUrgent)
{
	std::list<CMedia>::iterator itMedia = m_listMedia.begin();
	for ( ; itMedia != m_listMedia.end(); ++itMedia )
	{
		if (itMedia->m_eMediaStatus == eMediaDirectionType && itMedia->m_eMediaType == eMediaType && itMedia->m_eMediaRole == eMediaRoleType)
		{
			itMedia->m_bMediaDeleted = true;
			itMedia->m_bUrgent = bUrgent;
		}
	}
	std::list<CMedia>::iterator itNew = listMedia.begin();
	for ( ; itNew != listMedia.end(); ++itNew )
	{
		itNew->m_bUrgent = bUrgent;
		std::list<CMedia>::iterator itMedia = m_listMedia.begin();
		for ( ; itMedia != m_listMedia.end(); ++itMedia )
		{
			if (itMedia->m_dwID == itNew->m_dwID)
			{
				m_listMedia.erase(itMedia);
				break;
			}
		}
	}
	m_listMedia.insert(m_listMedia.end(), listMedia.begin(), listMedia.end());
	m_bNewList = false;
}

bool CMediaList::IsUrgent() const
{
	bool bUrgent = false;
	
	for (std::list<CMedia>::const_iterator it = m_listMedia.begin(); it != m_listMedia.end(); ++it)
	{
		if (it->m_bUrgent)
		{
			bUrgent = true;
			
			break;
		}
	}
	
	return bUrgent;
}

void CMediaList::SerializeXml(CXMLDOMElement* pParentNode, const char* pszNameSpacePrefix, bool bFull, bool bNew)
{
//	TRACEINTO << "CMedia::SerializeXml - bFull = "
//			<< (bFull ? "true" : "false") << ", "<< (bNew ? "true" : "false")
//			<< "\n" << ToString().c_str();
	std::list<CMedia>::iterator it = m_listMedia.begin();
	for ( ; it != m_listMedia.end(); ++it )
	{
		bool bAdded = false;
		std::map<DWORD, BYTE>::const_iterator itStateNew = m_mapMediaState.find(it->m_dwID);
		if (itStateNew != m_mapMediaState.end())
			bAdded = (itStateNew->second == MEDIA_NEW);
		it->SerializeXml(pParentNode, "", bFull || bNew || bAdded);
	}
	RemoveDeletedMedia();
}

void CMediaList::SerializeXmlApi(CXMLDOMElement* pParentNode, bool bFull /*= true*/)
{
	//TRACEINTO << "bFull = " << (bFull ? "true" : "false") << "\n" << ToString().c_str();
	std::list<CMedia>::iterator it = m_listMedia.begin();
	for ( ; it != m_listMedia.end(); ++it )
		it->SerializeXmlApi(pParentNode, bFull);
}

void CMediaList::Serialize(CSegment* pSeg) const
{
	std::list<CMedia>::const_iterator it = m_listMedia.begin();
	for ( ; it != m_listMedia.end(); ++it )
		it->Serialize(pSeg);
	DWORD dw = m_bNewList ? 1 :0;
	pSeg->Put(dw);
}
void CMediaList::DeSerialize(CSegment* pSeg)
{
	m_listMedia.clear();
	DWORD dwSize = pSeg->GetWrtOffset();
	int n = dwSize / sizeof(MEDIA_DATA);
	for (int i=0; i<n; ++i)
	{
		CMedia* pMedia = new CMedia;
		MEDIA_DATA* pData = (MEDIA_DATA*)pMedia;
		pData->DeSerialize(pSeg);
		m_listMedia.push_back(*pMedia);
		delete pMedia;
	}
	DWORD dw;
	pSeg->Get(dw);
	m_bNewList = (dw == 0) ? false : true;
}

std::string CMediaList::ToString() const
{
	std::string sRetVal = "CMediaList: this = ";
	sRetVal.append(CStlUtils::ValueToString((unsigned long)this));
	sRetVal.append("\n");
	std::list<CMedia>::const_iterator it = m_listMedia.begin();
	for ( ; it != m_listMedia.end(); ++it )
		sRetVal.append(it->ToString());
	if ( ! m_mapMediaState.empty() )
	{
		sRetVal.append("\nModifications:");
		for (std::map<DWORD, BYTE>::const_iterator itMap = m_mapMediaState.begin(); itMap != m_mapMediaState.end(); ++itMap)
		{
			sRetVal.append("\n");
			sRetVal.append(CStlUtils::ValueToString(itMap->first));
			switch (itMap->second)
			{
				case MEDIA_NEW:
					sRetVal.append(" - new");
					break;
				case MEDIA_MODIFIED:
					sRetVal.append(" - modified");
					break;
				case MEDIA_DELETED:
					sRetVal.append(" - deleted");
					break;
				default:
					break;
			}
		}
	}
	return sRetVal;
}

int CMediaList::GetListMediaSize()
{
	return m_listMedia.size();
}


