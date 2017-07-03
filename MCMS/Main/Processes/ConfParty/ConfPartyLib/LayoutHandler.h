#ifndef _LAYOUT_HANDLER_H_
#define _LAYOUT_HANDLER_H_

#include <algorithm>
#include <sstream>
#include "PObject.h"
#include "VideoBridge.h"
#include "VideoBridgeVsw.h"
#include "VideoBridgePartyCntl.h"
#include "Party.h"

class CTaskApp;
class CLayout;
class CVideoBridge;
class CVideoBridgePartyCntl;
class CAutoScanOrder;
class ScreenCpLayoutInfo;

typedef std::map<EScreenPosition, ScreenCpLayoutInfo*> CRoomScreensLayoutMap;

////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandler
////////////////////////////////////////////////////////////////////////////
class CLayoutHandler : public CPObject
{
CLASS_TYPE_1(CLayoutHandler, CPObject)
public:
                              CLayoutHandler(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);
                              CLayoutHandler(const CLayoutHandler* layout);
  virtual                    ~CLayoutHandler();
  virtual const char*         NameOf() const { return "CLayoutHandler"; }

  virtual bool                BuildLayout();
  virtual BYTE                IsLayoutWithSpeakerPicture(const LayoutType layoutType);
  WORD                        GetAutoScanTimeOut() {return m_AutoScanTimeOut;}
  virtual const CVideoBridge* GetpVideoBridge()     { return m_pVideoCntl;      }

  virtual void                FillAutoScanImageInLayout(CLayout& rResultLayout,BYTE needToStartTimer = 1);
  virtual void                SetAutoScanOrder(CAutoScanOrder* pAutoScanOrder);
  void                        DumpImagesVectorAutoScan(std::ostringstream& msg);

  void                        BuildLayoutCOP(BYTE &rIsLayoutChanged);
  void                        FillLayoutCOP();
  void                        CreateVectorOfImagesReadyForSettingCOP(CPartyImageVector& rImagesReadyForSetting);
  bool                        BuildLayoutIndications(CLayout& oLayout);
  DWORD                       GetImagesVectorAutoScanSize();

protected:
  virtual void                RemoveOldPartForcesAndBlankes();
  virtual void                RemoveOldConfForcesAndBlankes();

  virtual void                FillLayout();
  virtual void                FillSameLayout();

  virtual void                CreateVectorOfImagesReadyForSetting(CPartyImageVector& vectorOfImagesReadyForSetting);
  virtual void                SetForcedPartiesInLayout(CPartyImageVector& vectorOfImagesReadyForSetting, WORD &rNumCellsLeftToFill);
  virtual void                RemoveSelfImageFromImagesReadyForSetting(CPartyImageVector& vectorOfImagesReadyForSetting);
  virtual void                SetSpeakerImageToBigCellInLayout(CPartyImageVector& vectorOfImagesReadyForSetting, WORD &rNumCellsLeftToFill);
  virtual void                SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(CPartyImageVector &rImagesReadyForSetting, CLayout *pPreviouselySeenLayout, WORD &rNumCellsLeftToFill);
  virtual void                SetImagesToPreviouseSeenCellsInLayout(CPartyImageVector &rImagesReadyForSetting, CLayout* pPreviouselySeenLayout, WORD &rNumCellsLeftToFill);
  virtual void                SetImagesToAnyUnusedCell(CPartyImageVector &rImagesReadyForSetting);
  virtual void                SetConfForces(void);
  virtual void                SetPartyForces(void);
  virtual void                SetForcesForListener(void);
  virtual void                SetSelfView(void);
  virtual BYTE                IsContentImageNeedToBeAdded();
  BOOL                        IsBlankCelINeedToBeForcedInITPLayout(CLayout*pCurrLayout,const char* partyName,WORD cellId,BOOL bIsPrivate);
  BOOL                        IsBridgePartyCntlValid(CBridgePartyCntl*pPartyCntl);
  BOOL                        IsImageInSameRoom(DWORD partyID);
  bool                        IsVectorImagesReadyForSettingContainsAVMCULinksOnly(CPartyImageVector& rImagesReadyForSetting);

  const CVideoBridge*         m_pVideoCntl;
  CVideoBridgePartyCntl*      m_pPartyCntl;
  CVideoBridgeAutoScanParams* m_pAutoScanParams;
  WORD                        m_AutoScanTimeOut;
};


////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerVSW
////////////////////////////////////////////////////////////////////////////
class CLayoutHandlerVSW : public CLayoutHandler
{
CLASS_TYPE_1(CLayoutHandlerVSW, CLayoutHandler)
public:
	                            CLayoutHandlerVSW(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);
	                            CLayoutHandlerVSW(const CLayoutHandlerVSW* layout);
	virtual                    ~CLayoutHandlerVSW(void) { }
	virtual const char*         NameOf() const { return "CLayoutHandlerVSW"; }

	virtual bool                BuildLayout();
};


////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerLegacy
////////////////////////////////////////////////////////////////////////////
class CLayoutHandlerLegacy : public CLayoutHandler
{
CLASS_TYPE_1(CLayoutHandlerLegacy, CLayoutHandler)
public:
	                     CLayoutHandlerLegacy(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);
	                     CLayoutHandlerLegacy(const CLayoutHandlerLegacy* layout);
	virtual             ~CLayoutHandlerLegacy(void) { }
	virtual const char*  NameOf() const { return "CLayoutHandlerLegacy"; }

	virtual void         SetSpeakerImageToBigCellInLayout(CPartyImageVector& rImagesReadyForSetting, WORD &rNumCellsLeftToFill);
	virtual void         FillLayout();
	virtual void         SetForcesForListener(void);
	virtual BYTE         IsLayoutWithSpeakerPicture(const LayoutType layoutType);
	virtual BYTE         IsContentImageNeedToBeAdded();
	virtual void         CreateVectorOfImagesReadyForSetting(CPartyImageVector& vectorOfImagesReadyForSetting);
};


////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerTelepresence
////////////////////////////////////////////////////////////////////////////



class CLayoutHandlerTelepresence : public CLayoutHandler
{
CLASS_TYPE_1(CLayoutHandlerTelepresence, CLayoutHandler)
public:
	                          CLayoutHandlerTelepresence(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl) : CLayoutHandler(videoCntl, partyCntl) {}
	                          CLayoutHandlerTelepresence(const CLayoutHandlerTelepresence* layout) : CLayoutHandler(layout) {}
	virtual                  ~CLayoutHandlerTelepresence(void) { }
	virtual const char*       NameOf() const { return "CLayoutHandlerTelepresence"; }

	virtual bool              BuildLayout() = 0;
	virtual void              FillLayout() = 0;
	void                      SetBlackScreen(CVideoBridgePartyCntl* pPartyCntl);
	void                      SetBlackScreen(CLayout* pCurrLayout);
};



class CLayoutHandlerTelepresenceRoomSwitch : public CLayoutHandlerTelepresence
{
	CLASS_TYPE_1(CLayoutHandlerTelepresenceRoomSwitch, CLayoutHandlerTelepresence)
public:
	                               CLayoutHandlerTelepresenceRoomSwitch(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);
	                               CLayoutHandlerTelepresenceRoomSwitch (const CLayoutHandlerTelepresenceRoomSwitch* layout);
	virtual                       ~CLayoutHandlerTelepresenceRoomSwitch(void) { }
	virtual const char*            NameOf() const { return "CLayoutHandlerTelepresenceRoomSwitch"; }

	virtual bool                   BuildLayout();
	virtual void                   FillLayout();

	TELEPRESENCE_LAYOUT_INDEXES_S* GetTelepresenceLayoutIndexesStruct(CPartyImageVector& rImagesReadyForSetting);
	BOOL                           CheckIfAllImagesReadyForSettingAreInTheSameRoom(CPartyImageVector& imagesReadyForSetting);
	void                           RemoveSelfImageFromImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting);
	void                           SetImagesByIndexesInLayout(TELEPRESENCE_LAYOUT_INDEXES_S* pTelepresenceLayoutIndexesStruct, CPartyImageVector& rImagesReadyForSetting);
};



class CLayoutHandlerTelepresenceCP : public CLayoutHandlerTelepresence
{
	CLASS_TYPE_1(CLayoutHandlerTelepresenceCP, CLayoutHandlerTelepresence)
	                               CLayoutHandlerTelepresenceCP(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);
	virtual                       ~CLayoutHandlerTelepresenceCP();
	virtual const char*            NameOf() const { return "CLayoutHandlerTelepresenceCP"; }

	virtual bool                   BuildLayout();

	WORD                           CreateVectorOfRoomImagesReadyForSetting(CRoomSpeakerVector& vectorOfImagesReadyForSetting, ostringstream& ostr);

protected:

	typedef enum
	{
		eFillAllGrids, // Fill all existed grids, and then filmstrips
		eFillUpTo3x3,  // Use grid cells (up to 3x3) before filling the filmstrip cells
		eFillOnly4x4   // Fill the filmstrip first and then 4x4 grid
	} eFillGridPolicy;

	WORD                           RemoveSelfImagesFromRoomImagesReadyForSetting(RoomID id, CRoomSpeakerVector& rImagesReadyForSetting);
	void                           SetBlackScreen(RoomID id, bool send_change_layout_to_sublinks = false);
	void                           CollectRoomScreenLayoutsInfo(CRoomInfo& rLocalRoomInfo, TelepresenceCpModeLayout& rLayoutMapPerRoom,
                                                                CRoomScreensLayoutMap& retRoomLayoutInfo, ostringstream& ostr);
	void                           OccupyCellByPartyImage(WORD cellToBeSeen, ScreenCpLayoutInfo* pScreenLayoutInfo,
                                                          std::pair<PartyRsrcID, bool>& rPartySpeakerImage, bool isFilmstripCell, ostringstream& ostr);
	DWORD                          CountRemainingImage(CRoomSpeakerVector& vectorOfImagesReadyForSetting);
	void                           SendRoomChangeLayout(CRoomScreensLayoutMap& currRoomLayoutDB, ostringstream& ostr);
	bool                           FindSpeakerInGridLayout(std::pair<RoomID, CPartyImageInfoVector>& rSpeakerImages,
                                                           CRoomScreensLayoutMap& roomLayoutInfo,
                                                           CVidSubImage* pCurrScreenNewCell,
                                                           ostringstream& ostr);
	BYTE                           FindSpaceInGridLayout(const ScreenCpLayoutInfo& pGridLayoutInfo, WORD numSpeakerImages, ostringstream& ostr);
	void                           FillGridLayout(TelepresenceCpModeLayout& rLayoutMapPerRoom,
	                                              CRoomSpeakerVector& vectorOfImagesReadyForSetting,
	                                              CRoomScreensLayoutMap& rCurrRoomLayoutDB,
	                                              ostringstream& ostr, eFillGridPolicy fillPolicy = eFillAllGrids);
	bool                           FillGridLayoutByDefinedSpeakers(TelepresenceCpModeLayout& rLayoutMapPerRoom,
	                                                               CRoomSpeakerVector& vectorOfImagesReadyForSetting,
	                                                               WORD currSpeakerRoomIndex, WORD lastSpeakerRoomIndex,
	                                                               CRoomScreensLayoutMap& rCurrRoomLayoutDB,
	                                                               ostringstream& ostr, eFillGridPolicy fillPolicy);
	EScreenPosition                GetScreenPositionForActiveSpeaker(CRoomInfo& rLocalRoomInfo)const;
	BYTE                           GetSuitableCellIndexOfImageVector(size_t numSpeakerImages,BYTE indexSpeakerImages);
	EScreenPosition                LookForBestGridScreen(TelepresenceCpModeLayout& rLayoutMapPerRoom,
                                                         CRoomScreensLayoutMap& rCurrRoomLayoutDB,
                                                         std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages,
                                                         BYTE& bestGridCellPosition,
                                                         ostringstream& ostr, eFillGridPolicy fillPolicy = eFillAllGrids);
	WORD                           GetNumGridCellsOnAllScreens(TelepresenceCpModeLayout& rLayoutMapPerRoom);
	void                           SwapLayoutReservedAndGridScreensIfNeeded(CRoomScreensLayoutMap& rCurrRoomLayoutDB, ostringstream& ostr);
	RoomID                         GetRoomId(PartyRsrcID partyId);
	void                           ResetAllScreensForSwapping(CRoomScreensLayoutMap& rCurrRoomLayoutDB);
	void                           MarkScreensNotAllowedToSwap(CRoomScreensLayoutMap& rCurrRoomLayoutDB);

	struct IsNotMutedImage : public std::unary_function<std::pair<PartyRsrcID, bool>, bool>
	{
		IsNotMutedImage()
		{
			pPartyImageLookupTable = GetPartyImageLookupTable();
		}
		bool operator()(const std::pair<PartyRsrcID, bool>& imagePartyId) const
		{
			PartyRsrcID partyId = imagePartyId.first;
			if (!partyId)
				return false;
			CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyId);
			return (pImage && !pImage->isMuted());
		}

		CPartyImageLookupTable* pPartyImageLookupTable;
	};

	struct IsRoomMuted : public std::unary_function<std::pair<RoomID, CPartyImageInfoVector>, bool>
	{
		IsRoomMuted(RoomID localRoomId)
		{
			m_roomId = localRoomId;
		}
		bool operator()(const std::pair<RoomID, CPartyImageInfoVector>& imagePartyVector) const
		{
			if (imagePartyVector.first == m_roomId)
				return false;
			CPartyImageInfoVector::const_iterator it = std::find_if(imagePartyVector.second.begin(), imagePartyVector.second.end(), IsNotMutedImage());
			return (it == imagePartyVector.second.end());
		}

		RoomID m_roomId;
	};

	struct IsNotUsedImage : public std::unary_function<std::pair<PartyRsrcID, bool>, bool>
	{
		bool operator()(const std::pair<PartyRsrcID, bool>& imagePartyId) const
		{
			return (!imagePartyId.second);
		}
	};

};


class CLayoutHandlerTelepresenceSpeakerModeCP : public CLayoutHandlerTelepresenceCP
{
	CLASS_TYPE_1(CLayoutHandlerTelepresenceSpeakerModeCP, CLayoutHandlerTelepresenceCP)
public:
	                                    CLayoutHandlerTelepresenceSpeakerModeCP(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);

	virtual                            ~CLayoutHandlerTelepresenceSpeakerModeCP();
	virtual const char*                 NameOf() const { return "CLayoutHandlerTelepresenceSpeakerModeCP"; }

	virtual void                        FillLayout();

private:
	bool                                IfSpeakerRoomDisplayedInBestResolutionBefore(std::pair<RoomID, CPartyImageInfoVector>& rSpeakerImages,
                                                                                     CRoomScreensLayoutMap& roomLayoutInfo,
                                                                                     ScreenCpLayoutInfo* currScreenInfo,
                                                                                     WORD indexCurrCell,
                                                                                     ostringstream& ostr);
	bool                                FindImagePlaceInRoomLayout(std::pair<PartyRsrcID, bool>& rPartySpeakerImage,
                                                                   CRoomScreensLayoutMap& roomLayoutInfo,
                                                                   ScreenCpLayoutInfo* currScreenInfo,
                                                                   WORD indexCurrCell,
                                                                   ostringstream& ostr);
	WORD                                GetImagePlaceInRoomLayout(PartyRsrcID         imgPartyId,
	                                                              ScreenCpLayoutInfo* pPrevScreenInfo,
	                                                              ScreenCpLayoutInfo* pCurrScreenInfo,
	                                                              WORD                indexCurrCell,
	                                                              ostringstream&      ostr);
	void                                FillLayoutFilmstrip(CRoomSpeakerVector& vectorOfImagesReadyForSetting, CRoomScreensLayoutMap& roomLayoutInfo, ostringstream& ostr);
	WORD                                GetReservedScreenCellIndex(LayoutType layoutType, BYTE indexFirstCell, BYTE indexSpeakerImages, WORD numSpeakerImages);
	BYTE                                GetReservedScreenSpeakerImageIndex(EScreenPosition linkNum, LayoutType layoutType, BYTE indexSpeakerImages, WORD numSpeakerImages);
	WORD                                GetReservedScreenFilmstripCellIndex(WORD numSpeakerImages, BYTE indexFirstCell, BYTE indexSpeakerImages);
	TelepresenceCpModeLayout::iterator  GetScreenLayoutByPosition(TelepresenceCpModeLayout& rLayoutMap, EScreenPosition index, CRoomInfo& rLocalRoomInfo, WORD numSpeakerImages);
};


class CLayoutHandlerTelepresencePartyModeCP : public CLayoutHandlerTelepresenceCP
{
CLASS_TYPE_1(CLayoutHandlerTelepresencePartyModeCP, CLayoutHandlerTelepresenceCP)
public:
	CLayoutHandlerTelepresencePartyModeCP(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl);

	virtual                       ~CLayoutHandlerTelepresencePartyModeCP();
	virtual const char*            NameOf() const { return "CLayoutHandlerTelepresencePartyModeCP"; }

	virtual void                   FillLayout();
};


#endif //_LAYOUT_HANDLER_H_
