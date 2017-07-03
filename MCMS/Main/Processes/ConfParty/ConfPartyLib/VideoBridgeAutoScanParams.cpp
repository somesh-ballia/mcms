#include "VideoBridgeAutoScanParams.h"
#include "Party.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeAutoScanParams
////////////////////////////////////////////////////////////////////////////
CVideoBridgeAutoScanParams::CVideoBridgeAutoScanParams(const CVideoBridge* pVideoBridge, WORD timerInterval/* = 20 */)
	: m_timerInterval(timerInterval)
	, m_nextImageIndex(0)
	, m_pVideoBridge(pVideoBridge)
{
}

//--------------------------------------------------------------------------
CVideoBridgeAutoScanParams::~CVideoBridgeAutoScanParams()
{
	m_AutoScanImages.clear();
}

//--------------------------------------------------------------------------
void CVideoBridgeAutoScanParams::RemoveImageFromAutoScanVector(DWORD partyRscId)
{
	CPartyImageVector::iterator _ii = std::find(m_AutoScanImages.begin(), m_AutoScanImages.end(), partyRscId);
	if (_ii != m_AutoScanImages.end())
		m_AutoScanImages.erase(_ii);
}

//--------------------------------------------------------------------------
void CVideoBridgeAutoScanParams::InitScanOrder(CAutoScanOrder* scanOrder)
{
	m_AutoScanOrder.clear();
	AutoScanMap* scanOrderMap = scanOrder->GetAutoScanOrderMap();
	for (DWORD i = 0; i < scanOrderMap->size(); i++)
	{
		if (scanOrderMap->find(i) != scanOrderMap->end())
			m_AutoScanOrder.push_back((*scanOrderMap)[i]);
		else
			PASSERT(1);
	}
}

//--------------------------------------------------------------------------
DWORD CVideoBridgeAutoScanParams::GetNextPartyImageId()
{
	DWORD size = m_AutoScanImages.size();
	DWORD partyRscId = 0;

	if (m_nextImageIndex < size)
	{
		partyRscId = m_AutoScanImages[m_nextImageIndex];
		m_nextImageIndex = (m_nextImageIndex + 1) % size;
	}
	return partyRscId;
}

//--------------------------------------------------------------------------
BYTE CVideoBridgeAutoScanParams::FillAutoScanImages(CPartyImageVector& imagesReadyForSetting, WORD& numCellsLeft)
{
	std::ostringstream msg1;
	msg1 << "CVideoBridgeAutoScanParams::FillAutoScanImages - AutoScanImagesVector before:";
	DumpImagesVector(m_AutoScanImages, msg1);
	PTRACE(eLevelInfoNormal, msg1.str().c_str());

	std::ostringstream msg2;
	msg2 << "CVideoBridgeAutoScanParams::FillAutoScanImages - imagesReadyForSetting:";
	DumpImagesVector(imagesReadyForSetting, msg2);
	PTRACE(eLevelInfoNormal, msg2.str().c_str());

	// find currently seen image in order to start auto scan from it if it exists in new vector
	DWORD currentlySeenImageId = 0;
	BYTE searchForOldImageInNewVector = TRUE;
	WORD oldSize = m_AutoScanImages.size();
	if (oldSize)
	{
		m_nextImageIndex = m_nextImageIndex ? ((m_nextImageIndex - 1) % oldSize) : oldSize - 1;
		currentlySeenImageId = GetNextPartyImageId();
	}
	else
	{
		// old vector is empty - don't search for it in new vector
		searchForOldImageInNewVector = FALSE;
	}

	m_AutoScanImages.clear();
	m_nextImageIndex = 0;

	//VNGR-22765,return directly to avoid numAvailableImages get a very large positive value.
	if (imagesReadyForSetting.size() <= numCellsLeft)
	{
		return m_AutoScanImages.size();
	}

	// the size of the list from EMA (0 is auto)
	WORD orderedListSize = m_AutoScanOrder.size();
	// the num of available images for auto scan (all images - the images that would be set in other cells)
	WORD numAvailableImages = imagesReadyForSetting.size() - numCellsLeft + 1;

	// the num of the images to scan
	WORD numAutoScanImages = orderedListSize ? min(orderedListSize, numAvailableImages) : numAvailableImages;

	WORD numSelectedImagesToScan = m_AutoScanImages.size(); // 0
	WORD numIterations = orderedListSize ? orderedListSize : numAvailableImages;
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	for (WORD i = 0; i < numIterations && numSelectedImagesToScan < numAutoScanImages; i++)
	{
		for (int j = imagesReadyForSetting.size() - 1; j >= 0; j--)
		{
			DWORD partyRscId = imagesReadyForSetting[j];

			if (orderedListSize)
			{
				CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
				PASSERTSTREAM(!pImage, "CVideoBridgeAutoScanParams::FillAutoScanImages - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

				DWORD monitorPartyID = pImage ? m_pVideoBridge->GetMonitorPartyId(pImage->GetVideoSource()) : INVALID;

				if (monitorPartyID == (DWORD)m_AutoScanOrder[i])
				{
					m_AutoScanImages.push_back(partyRscId);
					// remove auto scan image from imagesReadyForSetting vector
					imagesReadyForSetting.erase(imagesReadyForSetting.begin() + j);
					break;
				}
			}
			else
			{
				m_AutoScanImages.insert(m_AutoScanImages.begin(), partyRscId);
				// remove auto scan image from imagesReadyForSetting vector
				imagesReadyForSetting.erase(imagesReadyForSetting.begin() + j);
				break;
			}
		}

		numSelectedImagesToScan = m_AutoScanImages.size();
	}

	if (m_AutoScanImages.size() > 0)
	{
		--numCellsLeft;

		if (searchForOldImageInNewVector == TRUE && currentlySeenImageId != 0)
		{
			// find if currently seen image exists in new vector and start the scan from it
			DWORD autoScanImagesSize = m_AutoScanImages.size();
			for (DWORD k = 0; k < autoScanImagesSize; k++)
			{
				if (m_AutoScanImages[k] == currentlySeenImageId)
				{
					m_nextImageIndex = k;
					PTRACE2INT(eLevelInfoNormal, "old image found at new vector in index: ", k);
					break;
				}
			}
		}
	} // if

	std::ostringstream msg3;
	msg3 << "CVideoBridgeAutoScanParams::FillAutoScanImages - AutoScanImagesVector after:";
	DumpImagesVector(m_AutoScanImages, msg3);
	PTRACE(eLevelInfoNormal, msg3.str().c_str());

	return m_AutoScanImages.size();
}

//--------------------------------------------------------------------------
void CVideoBridgeAutoScanParams::DumpImagesVector(const CPartyImageVector& images, std::ostringstream& msg)
{
	msg << "\n  vector size    :" << images.size();
	msg << "\n  vector dump    :<";

	WORD size = images.size();
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	for (WORD i = 0; i < size; ++i)
	{
		if (i > 0)
			msg << ", ";

		DWORD partyRscId = images[i];
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
		PASSERTSTREAM(!pImage, "CVideoBridgeAutoScanParams::DumpImagesVector - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

		if (pImage)
			msg << pImage->GetVideoSourceName() << "(" << partyRscId << ")";
	}

	msg << ">";
	msg << "\n  next index     :" << m_nextImageIndex;
}

//--------------------------------------------------------------------------
void CVideoBridgeAutoScanParams::DumpImagesVector(std::ostringstream& msg)
{
	DumpImagesVector(m_AutoScanImages, msg);
}
