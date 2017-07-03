#ifndef _CVideoBridgeAutoScanParams_H_
#define _CVideoBridgeAutoScanParams_H_

#include "PObject.h"
#include "VideoDefines.h"
#include "VideoBridge.h"
#include "AutoScanOrder.h"

class CObjString;

using namespace std;

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeAutoScanParams
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeAutoScanParams : public CPObject
{
	CLASS_TYPE_1(CVideoBridgeAutoScanParams, CPObject)

public:
	                    CVideoBridgeAutoScanParams(const CVideoBridge* pVideoBridge, WORD timerInterval = 20);
	virtual            ~CVideoBridgeAutoScanParams();

	const char*         NameOf() const { return "CVideoBridgeAutoScanParams"; }

	void                InitScanOrder(CAutoScanOrder* scanOrder);
	BYTE                FillAutoScanImages(CPartyImageVector& imagesReadyForSetting, WORD& numCellsLeft);

	WORD                GetTimerInterval() const             { return m_timerInterval; }
	void                SetTimerInterval(WORD timerInterval) { m_timerInterval = timerInterval; }

	DWORD               GetNextPartyImageId();

	void                RemoveImageFromAutoScanVector(DWORD partyRscId);

	void                DumpImagesVector(std::ostringstream& msg);

	DWORD               GetAutoScanVectorSize() const        { return m_AutoScanImages.size(); }

private:
	void                DumpImagesVector(const CPartyImageVector& pImages, std::ostringstream& msg);

protected:
	WORD                m_timerInterval;
	WORD                m_nextImageIndex;
	CPartyImageVector   m_AutoScanImages;
	vector<int>         m_AutoScanOrder;

	const CVideoBridge* m_pVideoBridge;
};


#endif //_CVideoBridgeLectureModeParams_H_
