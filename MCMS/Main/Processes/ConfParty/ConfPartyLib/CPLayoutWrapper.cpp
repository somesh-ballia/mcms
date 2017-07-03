#include <fstream>

#include "CPLayoutWrapper.h"
#include "OsFileIF.h"
/*
#include "Macros.h"
#include "Trace.h"*/
#include "TraceStream.h"
#include "PrettyTable.h"
#include "ReservedScreensTableEntry.h"


void CPLayoutWrapper::LoadGridReservedMapAndCreateXMLIfNeeded(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
															std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap,
															const std::string& xmlFileName)
{

	std::string fullFilePath = (std::string)getenv("PWD") + std::string("/Cfg/")+ xmlFileName;

	TRACEINTO << "fullFilePath: " << fullFilePath;

	std::ifstream ifs;
	ifs.open(fullFilePath.c_str(), std::ios::in);

	//File exists
	if (ifs.good())
		LoadMapFromXML(reservedMap,gridMap,ifs,fullFilePath);
	else //File not exit
		BuildXmlFileToDisk(fullFilePath.c_str(), reservedMap,gridMap);

	ifs.close();

	//PrintReservedMap(reservedMap);
	//PrintGridMap(gridMap);

}

int CPLayoutWrapper::LoadMapFromXML(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
										std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap,
										std::ifstream& ifs,const std::string& fullFilePath)
{
	char* buffer = NULL;
	size_t fileSize = GetFileSize(fullFilePath.c_str());
	buffer = new char[fileSize+1];
	memset(buffer, 0, fileSize+1);
	ifs.read(buffer, fileSize);
	buffer[fileSize] = '\0';

	CPLayout::TelepresenceLayoutsLogic xmlParsedFile;

	if (xmlParsedFile.ReadFromXmlStream(buffer,fileSize+1))//In case file is good we fill the maps
	{
		delete [] buffer;
		FillMap(reservedMap,gridMap, xmlParsedFile);
		return 0;
	}
	else//In case file is corrupted we build a new file based on the DB
	{
		delete [] buffer;
		TRACEINTO << "Bad file content filling the map from DB";
		ifs.close();
		BuildXmlFileToDisk(fullFilePath, reservedMap,gridMap);
		return 1;
	}
}

int CPLayoutWrapper::BuildXmlFileToDisk(const std::string& filePath, std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
								std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap)
{
	//Build the map from DB
	reservedMap.clear();
	gridMap.clear();
	CTelepresenceCpLayoutLogic::CreateGridLayoutMap();
	CTelepresenceSpeakerModeLayoutLogic::CreateReservedLayoutMap();

	//PrintReservedMap(reservedMap);
	//PrintGridMap(gridMap);

	CPLayout::TelepresenceLayoutsLogic xmlFile;

	//Building the reserve map
	xmlFile.m_reservedScreensLogicTable.m_name = "Speaker_Priorety";

	std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >::iterator reserveTableIter = reservedMap.begin();
	std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >::iterator reserveTableIterEnd = reservedMap.end();

	for (;reserveTableIter !=reserveTableIterEnd;++reserveTableIter)
	{
		std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout>::iterator EntryIter =  reserveTableIter->second.begin();
		std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout>::iterator EntryIterEnd =  reserveTableIter->second.end();

		CPLayout::ReservedScreensTableEntry tableEntryObj;
		CPLayout::SpeakersParamsEntry speakersParamsEntryObj;

		FillParamFromReserveKey(reserveTableIter->first,tableEntryObj.m_destNumOfScreens,
						tableEntryObj.m_maxCamerasInConf,
						speakersParamsEntryObj.m_numOfScreensOfSpeaker1,
						speakersParamsEntryObj.m_numOfScreensOfSpeaker2,
						speakersParamsEntryObj.m_numOfScreensOfSpeaker3);

		for (;EntryIter !=EntryIterEnd;++EntryIter)
		{
			/*
			TRACEINTO << "LayoutType: " << GetLayoutTypeString(EntryIter->reservedLayoutType)
					<< "screenPosition: "  << GetScreenPositionString(EntryIter->screenPosition)
					<< "screenType: " << GetScreenTypeString(EntryIter->screenType);
*/
			CPLayout::ReservedLayout obj;
			obj.m_layoutType = GetLayoutTypeXML(EntryIter->reservedLayoutType);
			obj.m_screenIndex = GetScreenPositionXML(EntryIter->screenPosition);
			obj.m_screenType = GetScreenTypeXML(EntryIter->screenType);

			speakersParamsEntryObj.m_dstScreens.push_back(obj);
		}

		tableEntryObj.m_speakersParamsEntry.push_back(speakersParamsEntryObj);
		xmlFile.m_reservedScreensLogicTable.m_reservedScreensTableEntry.push_back(tableEntryObj);
	}

	//Building the grid map
	xmlFile.m_gridScreensLogicTable.m_name = "Speaker_Priorety";

	std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>::iterator gridTableIter = gridMap.begin();
	std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>::iterator gridTableIterEnd = gridMap.end();


	for (;gridTableIter !=gridTableIterEnd;++gridTableIter)
	{
		CPLayout::GridScreensTableEntry gridScreensTableEntryObj;
		CPLayout::GridScreensLogicTable gridScreensLogicTable;

		FillParamFromGridKey(gridTableIter->first,gridScreensTableEntryObj.m_filmsrtipNumOfScreens,
							gridScreensTableEntryObj.m_maxCamerasInConf,
							gridScreensTableEntryObj.m_numOfGridScreens);

		gridScreensTableEntryObj.m_condition = GetGridLayoutCalcFunc(gridTableIter->second.funcId);
		gridScreensTableEntryObj.m_remainingCellsToDisplayFrom = gridTableIter->second.rangeFrom;
		gridScreensTableEntryObj.m_remainingCellsToDisplayTo = gridTableIter->second.rangeTo;

		for (size_t i=0; i<MAX_NUM_GRID_SCREENS_SPEAKER_MODE;++i)
		{
			CPLayout::ScreenLayout screenLayout;
			screenLayout.m_screenIndex = i;
			screenLayout.m_gridLayoutTypeIndex = gridTableIter->second.gridLayoutSpeakerMode[i];

			gridScreensTableEntryObj.m_screensLayoutList.push_back(screenLayout);
		}

		xmlFile.m_gridScreensLogicTable.m_gridScreensTableEntry.push_back(gridScreensTableEntryObj);
	}

	//Creating the XML file on disk
	ofstream newCreatedFile;
	newCreatedFile.open(filePath.c_str());
	newCreatedFile << xmlFile;
	newCreatedFile.close();

	//TRACEINTO << "Result of objects "<< xmlFile;

	return 0;
}

DWORD CPLayoutWrapper::GetDestNumOfScreens(DWORD destNumOfScreens)
{
	switch(destNumOfScreens)
	{
		case 1: return DEST_NUM_OF_SCREENS_1;
		case 2: return DEST_NUM_OF_SCREENS_2;
		case 3: return DEST_NUM_OF_SCREENS_3;
		case 4: return DEST_NUM_OF_SCREENS_4;
		default: PASSERTSTREAM(1, "CPLayoutWrapper::GetDestNumOfScreens - Invaild destination number of screens: " << destNumOfScreens);
				 break;
	}
	return 0;
}

DWORD CPLayoutWrapper::GetMaxCamerasInConf(DWORD maxCamerasInConf)
{
	switch(maxCamerasInConf)
	{
		case 1: return MAX_NUM_CAMERAS_IN_CONF_1;
		case 2: return MAX_NUM_CAMERAS_IN_CONF_2;
		case 3: return MAX_NUM_CAMERAS_IN_CONF_3;
		case 4: return MAX_NUM_CAMERAS_IN_CONF_4;
		default: PASSERTSTREAM(1, "CPLayoutWrapper::GetMaxCamerasInConf - Invaild max cameras in conference: " << maxCamerasInConf);
				 break;
	}
	return 0;
}

DWORD CPLayoutWrapper::GetSpeakerNumOfScreens(DWORD speakerNum, DWORD numOfScreensOfSpeaker)
{
	switch(speakerNum)
	{
		case 1:
			switch(numOfScreensOfSpeaker)
			{
				case 1: return SPEAKER_NUM_OF_SCREENS_1;
				case 2: return SPEAKER_NUM_OF_SCREENS_2;
				case 3:	return SPEAKER_NUM_OF_SCREENS_3;
				case 4: return SPEAKER_NUM_OF_SCREENS_4;
				default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetSpeakerNumOfScreens - Invaild speaker number of screens: " << numOfScreensOfSpeaker,0);
			}
		break;
		case 2:
			switch(numOfScreensOfSpeaker)
			{
				case 0: return 0;
				case 1: return SPEAKER2_NUM_OF_SCREENS_1;
				case 2: return SPEAKER2_NUM_OF_SCREENS_2;
				case 3:	return SPEAKER2_NUM_OF_SCREENS_3;
				case 4: return SPEAKER2_NUM_OF_SCREENS_4;
				default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetSpeakerNumOfScreens - Invaild speaker number of screens: " << numOfScreensOfSpeaker,0);
			}
		break;
		case 3:
			switch(numOfScreensOfSpeaker)
			{
				case 0: return 0;
				case 1: return SPEAKER3_NUM_OF_SCREENS_1;
				case 2: return SPEAKER3_NUM_OF_SCREENS_2;
				case 3:	return SPEAKER3_NUM_OF_SCREENS_3;
				case 4: return SPEAKER3_NUM_OF_SCREENS_4;
				default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetSpeakerNumOfScreens - Invaild speaker number of screens: " << numOfScreensOfSpeaker,0);
			}
		break;
			default: PASSERTSTREAM_AND_RETURN_VALUE(1,"CPLayoutWrapper::GetSpeakerNumOfScreens - Invaild speaker number: " << speakerNum,0);
	}
}

DWORD CPLayoutWrapper::BuildReserveKey(DWORD destNumOfScreens, DWORD maxCamerasInConf, DWORD numOfScreensOfSpeaker1,
								DWORD numOfScreensOfSpeaker2, DWORD numOfScreensOfSpeaker3)
{
	DWORD rsrvScreenNumKey = GetDestNumOfScreens(destNumOfScreens) | GetMaxCamerasInConf(maxCamerasInConf) |
							 GetSpeakerNumOfScreens(1, numOfScreensOfSpeaker1) | GetSpeakerNumOfScreens(2, numOfScreensOfSpeaker2)|
							 GetSpeakerNumOfScreens(3, numOfScreensOfSpeaker3);
	
	return rsrvScreenNumKey;
}

void CPLayoutWrapper::FillMap(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap,
							  std::multimap<DWORD,  CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap,
		 	 	 	 	 	  CPLayout::TelepresenceLayoutsLogic& xmlParsedFile)
{
	if (reservedMap.size())
	{
		//PrintReservedMap(reservedMap);
		reservedMap.clear();
	}

	if (gridMap.size())
	{
		//PrintGridMap(gridMap);
		gridMap.clear();
	}

	//Building the reserved map
	CPLayout::ReservedScreensLogicTable::ReservedScreensTableEntryContainer& reservedScreensLogicTable = xmlParsedFile.m_reservedScreensLogicTable.m_reservedScreensTableEntry;

	for (CPLayout::ReservedScreensLogicTable::ReservedScreensTableEntryContainer::iterator reservedScreensLogicTableIter = reservedScreensLogicTable.begin();
			reservedScreensLogicTableIter!= reservedScreensLogicTable.end();
			++reservedScreensLogicTableIter)
	{
		/*TRACEINTO << "destNumOfScreens: " << reservedScreensLogicTableIter->m_destNumOfScreens
				 << "maxCamerasInConf: " << reservedScreensLogicTableIter->m_maxCamerasInConf;*/

		for (CPLayout::ReservedScreensTableEntry::SpeakersParamsEntryContainer::iterator speakersListIter = reservedScreensLogicTableIter->m_speakersParamsEntry.begin();
													speakersListIter!=reservedScreensLogicTableIter->m_speakersParamsEntry.end();
													speakersListIter++)
		{
			/*
				TRACEINTO << "speakersListIter->m_numOfScreensOfSpeaker1: " << speakersListIter->m_numOfScreensOfSpeaker1
						<< "speakersListIter->m_numOfScreensOfSpeaker2: " << speakersListIter->m_numOfScreensOfSpeaker2
						<< "speakersListIter->m_numOfScreensOfSpeaker3: " << speakersListIter->m_numOfScreensOfSpeaker3;
			 */
				DWORD reservedTableKey = BuildReserveKey(reservedScreensLogicTableIter->m_destNumOfScreens,
																		reservedScreensLogicTableIter->m_maxCamerasInConf,
																		speakersListIter->m_numOfScreensOfSpeaker1,
																		speakersListIter->m_numOfScreensOfSpeaker2,
																		speakersListIter->m_numOfScreensOfSpeaker3);

				std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> layoutVector;

				for (CPLayout::SpeakersParamsEntry::DstScreensContainer::iterator screensListIter = speakersListIter->m_dstScreens.begin();
																	screensListIter!=speakersListIter->m_dstScreens.end();
																	++screensListIter)
				{
					/*
					TRACEINTO << "layoutType: " << screensListIter->m_layoutType
							<< "screenIndex: " << screensListIter->m_screenIndex
							<< "screenType: " << screensListIter->m_screenType;
					*/
					layoutVector.push_back(BuidLayoutPerScreen(screensListIter->m_screenIndex,screensListIter->m_screenType,screensListIter->m_layoutType));
				}
				reservedMap.insert(std::make_pair(reservedTableKey, layoutVector));
		}
	}

	//PrintReservedMap(reservedMap);

	//Building the grid map
	CPLayout::GridScreensLogicTable::GridScreensTableEntryContainer& gridScreensLogicTable = xmlParsedFile.m_gridScreensLogicTable.m_gridScreensTableEntry;

	for (CPLayout::GridScreensLogicTable::GridScreensTableEntryContainer::iterator gridScreensLogicTableIter = gridScreensLogicTable.begin();
			gridScreensLogicTableIter!= gridScreensLogicTable.end();
			++gridScreensLogicTableIter)
	{
/*
		TRACEINTO << "gridScreensLogicTableIter->m_numOfGridScreens: " << gridScreensLogicTableIter->m_numOfGridScreens
				<< "gridScreensLogicTableIter->m_maxCamerasInConf: " << gridScreensLogicTableIter->m_filmsrtipNumOfScreens
				<< "gridScreensLogicTableIter->m_filmsrtipNumOfScreens: " << gridScreensLogicTableIter->m_filmsrtipNumOfScreens;
*/
		DWORD gridTableKey = BuildGridKey(gridScreensLogicTableIter->m_numOfGridScreens,
											gridScreensLogicTableIter->m_maxCamerasInConf,
											gridScreensLogicTableIter->m_filmsrtipNumOfScreens);

		CTelepresenceCpLayoutLogic::TelepresenceGridLayout newObj;

		newObj.funcId = GetGridLayoutCalcFunc(gridScreensLogicTableIter->m_condition);
		newObj.key = gridTableKey;
		newObj.rangeFrom = gridScreensLogicTableIter->m_remainingCellsToDisplayFrom;
		newObj.rangeTo = gridScreensLogicTableIter->m_remainingCellsToDisplayTo;

		CPLayout::GridScreensTableEntry::ScreensLayoutListContainer::iterator screensListIterEnd = gridScreensLogicTableIter->m_screensLayoutList.end();
		CPLayout::GridScreensTableEntry::ScreensLayoutListContainer::iterator screensListIter = gridScreensLogicTableIter->m_screensLayoutList.begin();

		for (;screensListIter != screensListIterEnd; screensListIter++)
		{
			//TRACEINTO << "screensListIter->m_gridLayoutTypeIndex: " << screensListIter->m_gridLayoutTypeIndex << " m_screenIndex: " <<  screensListIter->m_screenIndex;
			newObj.gridLayoutSpeakerMode[screensListIter->m_screenIndex] = screensListIter->m_gridLayoutTypeIndex;
		}

		gridMap.insert(std::make_pair(newObj.key,newObj));
	}

	//PrintGridMap(gridMap);
}

void CPLayoutWrapper::PrintReservedMap(std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >& reservedMap)
{
	std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >::iterator TableIter = reservedMap.begin();
	std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> >::iterator TableIterEnd = reservedMap.end();

	CPrettyTable<const char*, const char*, const char*>
	tbl ("LayoutType", "ScreenPosition", "ScreenType");

	ostringstream msg;

	for (;TableIter !=TableIterEnd;++TableIter)
	{
		std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout>::iterator EntryIter =  TableIter->second.begin();
		std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout>::iterator EntryIterEnd =  TableIter->second.end();

		msg <<  "\nKey: " << TableIter->first;

		for (;EntryIter !=EntryIterEnd;++EntryIter)
		{
			const char* screenPosition = GetScreenPositionString(EntryIter->screenPosition);
			const char* screenType = GetScreenTypeString(EntryIter->screenType);
			const char* layoutType = GetLayoutTypeString(EntryIter->reservedLayoutType);

			tbl.Add(layoutType, screenPosition, screenType);
 		}

		msg << tbl.Get() << std::endl;
		tbl.Clear();
	}

	TRACEINTO << msg.str();
}


CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout CPLayoutWrapper::BuidLayoutPerScreen(size_t screenIndex,CPLayout::ScreenType screenType,CPLayout::LayoutType layoutType)
{

	CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout layout = { GetScreenPosition(screenIndex),GetScreenType(screenType), GetLayoutType(layoutType) };
	return layout;
}


CTelepresenceSpeakerModeLayoutLogic::EReservedLayoutType CPLayoutWrapper::GetLayoutType(CPLayout::LayoutType layoutType)
{
	switch(layoutType)
	{
		case CPLayout::eLayoutType_CPLAYOUTOVERLAYITP1P:
			return CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_1P;
		case CPLayout::eLayoutType_CPLAYOUTOVERLAYITP2P:
			return CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_2P;
		case CPLayout::eLayoutType_CPLAYOUTOVERLAYITP3P:
			return CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_3P;
		case CPLayout::eLayoutType_CPLAYOUTOVERLAYITP4P:
			return CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_4P;
		case CPLayout::eLayoutType_ECPLAYOUTNONE:
			return CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_NONE;

		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetLayoutType - WrongLayoutType: " << layoutType,CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_NONE);
	}
}

CPLayout::LayoutType CPLayoutWrapper::GetLayoutTypeXML(CTelepresenceSpeakerModeLayoutLogic::EReservedLayoutType layoutType)
{
	switch(layoutType)
	{
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_1P:
			return CPLayout::eLayoutType_CPLAYOUTOVERLAYITP1P;
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_2P:
			return CPLayout::eLayoutType_CPLAYOUTOVERLAYITP2P;
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_3P:
			return CPLayout::eLayoutType_CPLAYOUTOVERLAYITP3P;
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_4P:
			return CPLayout::eLayoutType_CPLAYOUTOVERLAYITP4P;
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_NONE:
			return CPLayout::eLayoutType_ECPLAYOUTNONE;

		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetLayoutType - WrongLayoutType: " << layoutType,CPLayout::eLayoutType_ECPLAYOUTNONE);
	}
}


EScreenType CPLayoutWrapper::GetScreenType(CPLayout::ScreenType screenType)
{
	switch(screenType)
	{
		case CPLayout::eScreenType_RESERVED:
			return eReserved;
		case CPLayout::eScreenType_GRID:
			return eGrid;
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetScreenType - WrongScreenType: " << screenType,(EScreenType)-1);

	}
}

CPLayout::ScreenType CPLayoutWrapper::GetScreenTypeXML(EScreenType screenType)
{
	switch(screenType)
	{
		case eReserved:
			return CPLayout::eScreenType_RESERVED;
		case eGrid :
			return CPLayout::eScreenType_GRID;
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetScreenType - WrongScreenType: " << screenType,CPLayout::eScreenType_LAST);
	}
}

EScreenPosition CPLayoutWrapper::GetScreenPosition(size_t index)
{
	switch(index)
	{
		case 1:
			return ePos1;
		case 2:
			return ePos2;
		case 3:
			return ePos3;
		case 4:
			return ePos4;
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetScreenType - WrongScreenPosition: " << index,ePosNone);
	}
}

size_t CPLayoutWrapper::GetScreenPositionXML(EScreenPosition index)
{
	switch(index)
	{
		case ePos1:
			return 1;
		case ePos2:
			return 2;
		case ePos3:
			return 3;
		case ePos4:
			return 4;
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetScreenType - WrongScreenPosition: " << index,ePosNone);
	}
}
const char* CPLayoutWrapper::GetScreenPositionString(size_t index)
{
	switch(index)
	{
		case 0:
			return "ePos1";
		case 1:
			return "ePos2";
		case 2:
			return "ePos3";
		case 3:
			return "ePos4";
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetScreenType - WrongScreenPosition: " << index,"ePosNone");
	}
}

const char* CPLayoutWrapper::GetLayoutTypeString(CTelepresenceSpeakerModeLayoutLogic::EReservedLayoutType layoutType)
{
	switch(layoutType)
	{
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_1P:
			return "eCP_LAYOUT_OVERLAY_1P";
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_2P:
			return "eCP_LAYOUT_OVERLAY_2P";
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_3P:
			return "eCP_LAYOUT_OVERLAY_3P";
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_OVERLAY_4P:
			return "eCP_LAYOUT_OVERLAY_4P";
		case CTelepresenceSpeakerModeLayoutLogic::eCP_LAYOUT_NONE:
			return "eCP_LAYOUT_NONE";

		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetLayoutType - WrongLayoutType: " << layoutType,"eCP_LAYOUT_NONE");
	}

}

const char* CPLayoutWrapper::GetScreenTypeString(EScreenType screenType)
{
	switch(screenType)
	{
		case eReserved:
			return "eReserved";
		case eGrid:
			return "eGrid";
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetScreenType - WrongScreenType: " << screenType,"eNone");

	}
}

void CPLayoutWrapper::FillParamFromReserveKey(size_t key,
									size_t& destNumOfScreens,
									size_t& maxCamerasInConf,
									size_t& numOfScreensOfSpeaker1,
									size_t& numOfScreensOfSpeaker2,
									size_t& numOfScreensOfSpeaker3)
{
	destNumOfScreens = GetNumberFromMask(key & 0x0000F);//DEST_NUM_OF_SCREENS_1 0x0001, DEST_NUM_OF_SCREENS_2 0x0002, DEST_NUM_OF_SCREENS_3 0x0004, DEST_NUM_OF_SCREENS_4 0x0008
	maxCamerasInConf = GetNumberFromMask((key & 0x000F0) >> 4); //MAX_NUM_CAMERAS_IN_CONF_1 0x0010, MAX_NUM_CAMERAS_IN_CONF_2 0x0020, MAX_NUM_CAMERAS_IN_CONF_3 0x0040, MAX_NUM_CAMERAS_IN_CONF_4 0x0080
	numOfScreensOfSpeaker1 = GetNumberFromMask((key & 0x00F00) >>8); //SPEAKER_NUM_OF_SCREENS_1 0x0100, SPEAKER_NUM_OF_SCREENS_2 0x0200, SPEAKER_NUM_OF_SCREENS_3 0x0400, SPEAKER_NUM_OF_SCREENS_4 0x0800
	numOfScreensOfSpeaker2 = GetNumberFromMask((key & 0x0F000) >> 12); //SPEAKER2_NUM_OF_SCREENS_1 0x1000, SPEAKER2_NUM_OF_SCREENS_2 0x2000, SPEAKER2_NUM_OF_SCREENS_3 0x4000, SPEAKER2_NUM_OF_SCREENS_4 0x8000
	numOfScreensOfSpeaker3 = GetNumberFromMask((key & 0xF0000) >> 16); //SPEAKER3_NUM_OF_SCREENS_1 0x10000, SPEAKER3_NUM_OF_SCREENS_2 0x20000, SPEAKER3_NUM_OF_SCREENS_3 0x40000, SPEAKER3_NUM_OF_SCREENS_4 0x80000
}

size_t CPLayoutWrapper::GetNumberFromMask(size_t number)
{
	switch(number)
	{
		case 0:
		case 1:
		case 2:
			return number;
		case 4:
			return 3;
		case 8:
			return 4;
		default: PASSERTSTREAM_AND_RETURN_VALUE(1, "CPLayoutWrapper::GetNumberFromMask - WrongNumber: " << number ,-1);
	}

}

void CPLayoutWrapper::PrintGridMap(std::multimap<DWORD,CTelepresenceCpLayoutLogic::TelepresenceGridLayout>& gridMap)
{
	std::multimap<DWORD,CTelepresenceCpLayoutLogic::TelepresenceGridLayout>::iterator TableIter = gridMap.begin();
	std::multimap<DWORD,CTelepresenceCpLayoutLogic::TelepresenceGridLayout>::iterator TableIterEnd = gridMap.end();

	CPrettyTable<size_t, size_t, size_t, size_t,size_t,size_t,size_t>
	tbl ("key","rangeFrom","rangeTo","funcId","FirstScreenValue","SecondScreenValue","ThirdScreenValue");

	ostringstream msg;

	for (;TableIter !=TableIterEnd;++TableIter)
	{

		tbl.Add(TableIter->second.key, TableIter->second.rangeFrom, TableIter->second.rangeTo, TableIter->second.funcId,
				TableIter->second.gridLayoutSpeakerMode[0], TableIter->second.gridLayoutSpeakerMode[1], TableIter->second.gridLayoutSpeakerMode[2]);

		msg << tbl.Get() << std::endl;
		tbl.Clear();
	}

	TRACEINTO << msg.str();
}

DWORD CPLayoutWrapper::BuildGridKey(DWORD numOfGridScreens,DWORD maxCamerasInConf,DWORD filmsrtipNumOfScreens)
{
	return GetNumOfGridScreens(numOfGridScreens) | GetMaxCamerasInConf(maxCamerasInConf) | GetNumOfFilmStrip(filmsrtipNumOfScreens);
}

DWORD CPLayoutWrapper::GetNumOfGridScreens(DWORD numOfGridScreens)
{
	switch(numOfGridScreens)
	{
		case 1: return GRID_NUM_OF_SCREENS_1;
		case 2: return GRID_NUM_OF_SCREENS_2;
		case 3: return GRID_NUM_OF_SCREENS_3;
		default: PASSERTSTREAM(1, "CPLayoutWrapper::GetNumOfGridScreens - Invaild number of grid screens: " << numOfGridScreens);
		break;
	}
	return 0;
}

DWORD CPLayoutWrapper::GetNumOfFilmStrip(DWORD numOfFilmStrip)
{
	switch(numOfFilmStrip)
	{
		case 0: return 0;
		case 1: return FILMSTRIP_NUM_0;
		case 2: return FILMSTRIP_NUM_1;
		case 3: return FILMSTRIP_NUM_2;
		case 4: return FILMSTRIP_NUM_3;
		default: PASSERTSTREAM(1, "CPLayoutWrapper::GetNumOfFilmStrip - Invaild number of film strip: " << numOfFilmStrip);
		break;
	}
	return 0;
}

CTelepresenceCpLayoutLogic::EGridLayoutCalcFunc CPLayoutWrapper::GetGridLayoutCalcFunc(CPLayout::Condition condition)
{
	switch(condition)
	{
		case CPLayout::eCondition_EFuncDefault:
			return CTelepresenceCpLayoutLogic::eFuncDefault;
		case CPLayout::eCondition_EFuncUpTo3RoomsOf3Cam:
			return CTelepresenceCpLayoutLogic::eFuncUpTo3RoomsOf3Cam;
		case CPLayout::eCondition_EFuncMore3RoomsOf3Cam:
			return CTelepresenceCpLayoutLogic::eFuncMore3RoomsOf3Cam;
		case CPLayout::eCondition_EFuncUpTo4RoomsOf3And4Cam:
			return CTelepresenceCpLayoutLogic::eFuncUpTo4RoomsOf3And4Cam;
		case CPLayout::eCondition_EFuncUpTo4RoomsOf4CamAndUpTo3Rooms3:
			return CTelepresenceCpLayoutLogic::eFuncUpTo4RoomsOf4CamAndUpTo3Rooms3;
		case CPLayout::eCondition_EFuncMore4RoomsOf4Cam:
			return CTelepresenceCpLayoutLogic::eFuncMore4RoomsOf4Cam;
		case CPLayout::eCondition_EFuncUpTo2RoomsOf2Cam:
			return CTelepresenceCpLayoutLogic::eFuncUpTo2RoomsOf2Cam;
		case CPLayout::eCondition_EFuncMore2RoomsOf2Cam:
			return CTelepresenceCpLayoutLogic::eFuncMore2RoomsOf2Cam;
		default: PASSERTSTREAM(CTelepresenceCpLayoutLogic::eFuncDefault, "CPLayoutWrapper::GetGridLayoutCalcFunc - Invaild condition: " << condition);
				break;
	}
	return CTelepresenceCpLayoutLogic::eFuncDefault;
}

CPLayout::Condition CPLayoutWrapper::GetGridLayoutCalcFunc(CTelepresenceCpLayoutLogic::EGridLayoutCalcFunc condition)
{
	switch(condition)
	{
		case CTelepresenceCpLayoutLogic::eFuncDefault:
			return CPLayout::eCondition_EFuncDefault;
		case CTelepresenceCpLayoutLogic::eFuncUpTo3RoomsOf3Cam:
			return CPLayout::eCondition_EFuncUpTo3RoomsOf3Cam;
		case CTelepresenceCpLayoutLogic::eFuncMore3RoomsOf3Cam:
			return CPLayout::eCondition_EFuncMore3RoomsOf3Cam;
		case CTelepresenceCpLayoutLogic::eFuncUpTo4RoomsOf3And4Cam:
			return CPLayout::eCondition_EFuncUpTo4RoomsOf3And4Cam;
		case CTelepresenceCpLayoutLogic::eFuncUpTo4RoomsOf4CamAndUpTo3Rooms3:
			return CPLayout::eCondition_EFuncUpTo4RoomsOf4CamAndUpTo3Rooms3;
		case CTelepresenceCpLayoutLogic::eFuncMore4RoomsOf4Cam:
			return CPLayout::eCondition_EFuncMore4RoomsOf4Cam;
		case CTelepresenceCpLayoutLogic::eFuncUpTo2RoomsOf2Cam:
			return CPLayout::eCondition_EFuncUpTo2RoomsOf2Cam;
		case CTelepresenceCpLayoutLogic::eFuncMore2RoomsOf2Cam:
			return CPLayout::eCondition_EFuncMore2RoomsOf2Cam;
		default: PASSERTSTREAM(CPLayout::eCondition_EFuncDefault, "CPLayoutWrapper::GetGridLayoutCalcFunc - Invaild condition: " << condition);
				break;
	}
	return CPLayout::eCondition_EFuncDefault;
}

void CPLayoutWrapper::FillParamFromGridKey(size_t key, size_t& filmsrtipNumOfScreens, size_t& maxCamerasInConf, size_t& numOfGridScreens)
{
	numOfGridScreens = GetNumberFromMask(key & 0x0000F); //GRID_NUM_OF_SCREENS_1 0x00000001 GRID_NUM_OF_SCREENS_2  0x00000002 GRID_NUM_OF_SCREENS_3  0x00000004
	maxCamerasInConf = GetNumberFromMask((key & 0x000F0) >> 4); //MAX_NUM_CAMERAS_IN_CONF_1 0x0010, MAX_NUM_CAMERAS_IN_CONF_2 0x0020, MAX_NUM_CAMERAS_IN_CONF_3 0x0040, MAX_NUM_CAMERAS_IN_CONF_4 0x0080
	filmsrtipNumOfScreens = GetNumberFromMask((key & 0xF00) >> 8); //FILMSTRIP_NUM_0 0x00000100 FILMSTRIP_NUM_1 0x00000200 FILMSTRIP_NUM_2 0x00000400 FILMSTRIP_NUM_3 0x00000800
}
