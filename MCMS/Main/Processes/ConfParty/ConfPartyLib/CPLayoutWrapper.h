/*
 * CPLayoutWrapper.h
 *
 *  Created on: Jul 27, 2014
 *      Author: nsalameh
 */

#ifndef CPLAYOUTWRAPPER_H_
#define CPLAYOUTWRAPPER_H_
//#include "DataTypes.h"
#include "PObject.h"
#include "TelepresenceLayoutsLogic.h"
#include "TelepresenseEPInfo.h"
#include <map>
#include <vector>

class CPLayoutWrapper: public CPObject
{

CLASS_TYPE_1(CPLayoutWrapper, CPObject)

public:

	virtual const char* NameOf() const { return "CPLayoutWrapper";}
public:
	void PrintReservedMap(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap);
	void PrintGridMap(std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap);
	void LoadGridReservedMapAndCreateXMLIfNeeded(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
												std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap,
												const std::string& xmlFileName = "TelepresenceLayoutsMap.xml");

private:

	int LoadMapFromXML(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
										std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap,
										std::ifstream& ifs, const std::string& filePath);

	int BuildXmlFileToDisk(const std::string& filePath, std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
						std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap);
	DWORD BuildReserveKey(DWORD destNumOfScreens, DWORD maxCamerasInConf, DWORD numOfScreensOfSpeaker1, DWORD numOfScreensOfSpeaker2, DWORD numOfScreensOfSpeaker3);
	DWORD BuildGridKey(DWORD numOfGridScreens,DWORD maxCamerasInConf,DWORD filmsrtipNumOfScreens);
	DWORD GetDestNumOfScreens(DWORD destNumOfScreens);
	DWORD GetMaxCamerasInConf(DWORD maxCamerasInConf);
	DWORD GetNumOfGridScreens(DWORD numOfGridScreens);
	DWORD GetSpeakerNumOfScreens(DWORD speakerNum, DWORD numOfScreensOfSpeaker);
	DWORD GetNumOfFilmStrip(DWORD numOfFilmStrip);
	CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout BuidLayoutPerScreen(size_t screenIndex,CPLayout::ScreenType screenType,CPLayout::LayoutType LayoutType);

	void FillMap(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
				 std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap,
				 CPLayout::TelepresenceLayoutsLogic& xmlParsedFile);

	//Screen info
	CTelepresenceSpeakerModeLayoutLogic::EReservedLayoutType GetLayoutType(CPLayout::LayoutType layoutType);
	EScreenType GetScreenType(CPLayout::ScreenType screenType);
	EScreenPosition GetScreenPosition(size_t index);
	const char* GetScreenPositionString(size_t index);
	const char* GetLayoutTypeString(CTelepresenceSpeakerModeLayoutLogic::EReservedLayoutType layoutType);
	const char* GetScreenTypeString(EScreenType screenType);

	CPLayout::LayoutType GetLayoutTypeXML(CTelepresenceSpeakerModeLayoutLogic::EReservedLayoutType layoutType);
	size_t GetScreenPositionXML(EScreenPosition index);
	CPLayout::ScreenType GetScreenTypeXML(EScreenType screenType);

	void FillParamFromReserveKey(size_t key,
						size_t& destNumOfScreens,
						size_t& maxCamerasInConf,
						size_t& numOfScreensOfSpeaker1,
						size_t& numOfScreensOfSpeaker2,
						size_t& numOfScreensOfSpeaker3);

	void FillParamFromGridKey(size_t key,
						size_t& filmsrtipNumOfScreens,
						size_t& maxCamerasInConf,
						size_t& numOfGridScreens);

	size_t GetNumberFromMask(size_t number);
	CTelepresenceCpLayoutLogic::EGridLayoutCalcFunc GetGridLayoutCalcFunc(CPLayout::Condition condition);
	CPLayout::Condition GetGridLayoutCalcFunc(CTelepresenceCpLayoutLogic::EGridLayoutCalcFunc condition);
};

#endif /* CPLAYOUTWRAPPER_H_ */
