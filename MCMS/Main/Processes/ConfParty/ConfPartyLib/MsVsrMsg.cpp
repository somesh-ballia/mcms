
#include "MsVsrMsg.h"
#include "TraceStream.h"
#include "Trace.h"
#include "MsSvcMode.h"
//#include "ConfPartyApiDefines.h"
#include <iostream>
#include <cstring>
#include <string>

#define VSR_ENTRIES_SELECT_DEBUG_PRINTS if(1)

//=========================================================================================//
// class CMsVsrEntry
//=========================================================================================//
CMsVsrEntry::CMsVsrEntry(const ST_VSR_PARAMS& vsrEntryParamsSt):m_VsrEntryParamsSt(vsrEntryParamsSt)
{
}
//=========================================================================================//
CMsVsrEntry::CMsVsrEntry(){
m_VsrEntryParamsSt.payload_type = 0;
m_VsrEntryParamsSt.aspect_ratio = 0;
m_VsrEntryParamsSt.max_width = 0;
m_VsrEntryParamsSt.max_height = 0;
m_VsrEntryParamsSt.frame_rate = 0;
m_VsrEntryParamsSt.min_bitrate = 0;
m_VsrEntryParamsSt.num_of_must = 0;
m_VsrEntryParamsSt.num_of_may = 0;
}
//=========================================================================================//
bool CMsVsrEntry::operator<(const CMsVsrEntry& second)const
{
	DWORD first_resolution = m_VsrEntryParamsSt.max_width*m_VsrEntryParamsSt.max_height;
	DWORD second_resolution = second.m_VsrEntryParamsSt.max_width*second.m_VsrEntryParamsSt.max_height;

	DWORD first_num_of_may = m_VsrEntryParamsSt.num_of_may;
	DWORD second_num_of_may = second.m_VsrEntryParamsSt.num_of_may;

	return (first_resolution<second_resolution || (first_resolution==second_resolution && first_num_of_may<second_num_of_may));
}
//=========================================================================================//
bool CMsVsrEntry::operator==(const CMsVsrEntry& other)const
{
	if(m_VsrEntryParamsSt.payload_type ==  other.m_VsrEntryParamsSt.payload_type &&
	m_VsrEntryParamsSt.aspect_ratio ==  other.m_VsrEntryParamsSt.aspect_ratio &&
	m_VsrEntryParamsSt.max_width ==  other.m_VsrEntryParamsSt.max_width &&
	m_VsrEntryParamsSt.max_height ==  other.m_VsrEntryParamsSt.max_height &&
	m_VsrEntryParamsSt.frame_rate ==  other.m_VsrEntryParamsSt.frame_rate &&
	m_VsrEntryParamsSt.min_bitrate ==  other.m_VsrEntryParamsSt.min_bitrate){
		return true;
	}
	return false;
}
//=========================================================================================//
bool operator<(const CMsVsrEntry& first,DWORD resolution)
{
	return (first.m_VsrEntryParamsSt.max_width*first.m_VsrEntryParamsSt.max_height < resolution);
}
//=========================================================================================//
bool operator<(DWORD resolution,const CMsVsrEntry& second)
{
	return (resolution < second.m_VsrEntryParamsSt.max_width*second.m_VsrEntryParamsSt.max_height);
}
//=========================================================================================//
bool operator<=(const CMsVsrEntry& first,DWORD resolution)
{
	return (first.m_VsrEntryParamsSt.max_width*first.m_VsrEntryParamsSt.max_height <= resolution);
}
//=========================================================================================//
bool operator<=(DWORD resolution,const CMsVsrEntry& second)
{
	return (resolution <= second.m_VsrEntryParamsSt.max_width*second.m_VsrEntryParamsSt.max_height);
}
//=========================================================================================//
DWORD CMsVsrEntry::GetNumOfMust()const
{
	return m_VsrEntryParamsSt.num_of_must;
}
//=========================================================================================//
DWORD CMsVsrEntry::GetNumOfMay()const
{
	return m_VsrEntryParamsSt.num_of_may;
}
//=========================================================================================//
DWORD CMsVsrEntry::GetResolution()const
{
	return (m_VsrEntryParamsSt.max_width*m_VsrEntryParamsSt.max_height);
}
//=========================================================================================//
DWORD CMsVsrEntry::GetPayloadType()const
{
	return m_VsrEntryParamsSt.payload_type;
}
//=========================================================================================//
DWORD CMsVsrEntry::GetMaxWidth()const
{
	return m_VsrEntryParamsSt.max_width;
}
//=========================================================================================//
DWORD CMsVsrEntry::GetMaxHight()const
{
	return m_VsrEntryParamsSt.max_height;
}
//=========================================================================================//
void  CMsVsrEntry::SetMaxWidth(DWORD maxWidth)
{
	m_VsrEntryParamsSt.max_width = maxWidth;
}
//=========================================================================================//
void  CMsVsrEntry::SetMaxHight(DWORD maxHight)
{
	m_VsrEntryParamsSt.max_height = maxHight;
}
//=========================================================================================//
void CMsVsrEntry::Dump(std::ostringstream& msg) const
{
	msg << "payload_type: ";
	switch(m_VsrEntryParamsSt.payload_type){
	case MS_SVC_PT_RTV:
		msg << "rtv";
		break;
	case MS_SVC_PT_H264:
		msg << "svc";
		break;
	default:
		msg << (DWORD)(m_VsrEntryParamsSt.payload_type);
		break;
	}
	msg << " , ";
	msg << "aspect_ratio: " << m_VsrEntryParamsSt.aspect_ratio << " , ";
	msg << "max_width x max_height: " << m_VsrEntryParamsSt.max_width << " x " << m_VsrEntryParamsSt.max_height << " , ";
	msg << "frame_rate: " << m_VsrEntryParamsSt.frame_rate << " , ";
	msg << "min_bitrate: " << m_VsrEntryParamsSt.min_bitrate << " , ";
	msg << "num_of_must: " << m_VsrEntryParamsSt.num_of_must << " , num_of_may: " << m_VsrEntryParamsSt.num_of_may << " , ";
	msg << "LYNC2013_FEC_RED:DEBUG Quality histogram: " ;
	for (int histNdx = 0; histNdx < VSR_NUM_OF_QUALITY_LEVELS; ++histNdx)
		msg << m_VsrEntryParamsSt.qualityReportHistogram[histNdx] << " ";
	msg << "\n";
}
//=========================================================================================//
void CMsVsrEntry::Dump() const
{
	std::ostringstream msg;
	Dump(msg);
	TRACEINTO << "\n" << msg.str().c_str();
}
//=========================================================================================//
EVideoResolutionType CMsVsrEntry::GetEncoderResolutionType()const
{
//	Eh264VideoModeType videoModeType = CMsSvcVideoMode::MinVideoModeForRes(m_VsrEntryParamsSt.max_width, m_VsrEntryParamsSt.max_height);
	Eh264VideoModeType videoModeType = CMsSvcVideoMode::GetBestVideoModeForVsrEntry(m_VsrEntryParamsSt.max_width, m_VsrEntryParamsSt.max_height);

	EVideoResolutionType videoResolutionType = eAuto_Res;
	if(eCIF30 == videoModeType){
		videoResolutionType = eCIF_Res;
	}else if(videoModeType < eHD720Asymmetric){
		videoResolutionType = eSD_Res;
	}else{
		videoResolutionType = eHD720_Res;
	}
	return videoResolutionType;
}

//=========================================================================================//
// class CMsVsrMsg
//=========================================================================================//

CMsVsrMsg::CMsVsrMsg(const ST_VSR_SINGLE_STREAM& vsrMsgParamsSt):m_VsrMsgParamsSt(vsrMsgParamsSt)
{
}
//=========================================================================================//
DWORD CMsVsrMsg::SelectBestEntries(const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap)
{
	VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "start (VSR_ENTRIES_SELECT)";
	VSR_ENTRIES_SELECT_DEBUG_PRINTS Dump();
	SelectMustEntries(encodersResolutionsSet,selectedEntriesPerEncodersMap);
	// if we have free encoders
	if(selectedEntriesPerEncodersMap.size()<encodersResolutionsSet.size()){
		SelectMayEntries(encodersResolutionsSet,selectedEntriesPerEncodersMap);
	}

	TRACEINTO;
    DumpSelection(encodersResolutionsSet,selectedEntriesPerEncodersMap);
    VSR_ENTRIES_SELECT_DEBUG_PRINTS TRACEINTO << "end (VSR_ENTRIES_SELECT)";
	return selectedEntriesPerEncodersMap.size();

}

//=========================================================================================//
// selecting must entries logic:
// iterate all must entries from lowest to highest entry resolution
// for each must entry select the encoder with lowest resolution that can decode the entry and free
DWORD CMsVsrMsg::SelectMustEntries(const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap)
{
	// 1) fill must entries set
	DWORD num_of_selected_entries = 0;
	MsVsrEntriesSet mustEntriesSet;
	FillMustEntriesSet(mustEntriesSet);
	DWORD num_of_must_entries = mustEntriesSet.size();
	// if no must entries, or no encoders - do nothing
	if(num_of_must_entries == 0 || encodersResolutionsSet.size() == 0){
		TRACEINTO << " return 0 : num of must entries = " << mustEntriesSet.size() <<  " , num of resolutions = " << encodersResolutionsSet.size();
		return num_of_selected_entries;
	}

	if(1 != num_of_must_entries){
		DBGPASSERT(num_of_must_entries);
		TRACEINTO << " received more them 1 must entry - expect only 1 must entry for RTV - take first must entry";
	}

	// 2) select encoder for (first) must entry
	MsVsrEntriesSet::iterator entriesIterator = mustEntriesSet.begin();
	SelectBestEncoderForVsrEntry(entriesIterator,encodersResolutionsSet,selectedEntriesPerEncodersMap);

	num_of_selected_entries = selectedEntriesPerEncodersMap.size();

	VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << " (VSR_ENTRIES_SELECT)";
	VSR_ENTRIES_SELECT_DEBUG_PRINTS	DumpSelection(encodersResolutionsSet,selectedEntriesPerEncodersMap);

	return num_of_selected_entries;

}
//=========================================================================================//
// may entries selection logic:
// 1) select SVC entry for lowest resolution
// 2) select SVC entries by "number of may":
//		for HD encoder - prefer HD entry , if no HD entry - by number of may
// 3) if free encoder left - select rtv entry
DWORD CMsVsrMsg::SelectMayEntries(const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap)
{
	VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << " start (VSR_ENTRIES_SELECT)";

	MsVsrEntriesSet mayEntriesSet;
	FillMayEntriesSet(mayEntriesSet); // fill may entries of svc paload only

	// empty sets or all encoders resolutions already assigned to entries
	if(mayEntriesSet.size() == 0 || encodersResolutionsSet.size() == 0 || selectedEntriesPerEncodersMap.size() >= encodersResolutionsSet.size()){
		TRACEINTO << " return 0 : num of may entries = " << mayEntriesSet.size() <<  " , num of encoders resolutions = " << encodersResolutionsSet.size() << " , num of in use encoders resolutions = " << selectedEntriesPerEncodersMap.size();
		return 0;
	}

	// 1) select SVC entry for lowest resolution
	MsVsrEntriesSet::iterator minEntriesIterator = mayEntriesSet.begin();
	bool found_encoder = SelectBestEncoderForVsrEntry(minEntriesIterator,encodersResolutionsSet,selectedEntriesPerEncodersMap);
	if(found_encoder){
		mayEntriesSet.erase(minEntriesIterator);
	}else{
		TRACEINTO << " return 0 : could not find encoder for lowest resolution";
		return 0;
	}

	VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "SVC entry for lowest resolution (VSR_ENTRIES_SELECT)";
	VSR_ENTRIES_SELECT_DEBUG_PRINTS	DumpSelection(encodersResolutionsSet,selectedEntriesPerEncodersMap);


	// 2) select may SVC entries
	// encoders resolutions are sorted in set sorted by resolution from low to high
	for(ResolutionsSet::iterator encoderResolutionsIterator = encodersResolutionsSet.begin(); encoderResolutionsIterator != encodersResolutionsSet.end(); ++encoderResolutionsIterator)
	{
		// entry is already assigned to this encoder resolution - skip
		if(selectedEntriesPerEncodersMap.find(*encoderResolutionsIterator)!=selectedEntriesPerEncodersMap.end())
		{
			continue;
		}
		bool selectedEntry = SelectBestEntryForEncoder(*encoderResolutionsIterator,mayEntriesSet,selectedEntriesPerEncodersMap,true);
	}
	// if we still have free encoders - means entries are not with the same resolutions
	if(selectedEntriesPerEncodersMap.size() < encodersResolutionsSet.size()){ // if we have free encoders
		if(mayEntriesSet.size() == 1){ // if only 1 entry left - choose the best encoder
			VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "1 entry left";
			MsVsrEntriesSet::iterator entryIterator = mayEntriesSet.begin();
			SelectBestEncoderForVsrEntry(entryIterator,encodersResolutionsSet,selectedEntriesPerEncodersMap);
		}else{
			// from High to low encoder - take best entry
			for(ResolutionsSet::reverse_iterator encoderResolutionsIterator = encodersResolutionsSet.rbegin(); encoderResolutionsIterator != encodersResolutionsSet.rend(); ++encoderResolutionsIterator)
			{
				// entry is already assigned to this encoder resolution - skip
				if(selectedEntriesPerEncodersMap.find(*encoderResolutionsIterator)!=selectedEntriesPerEncodersMap.end())
				{
					continue;
				}
				VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "select may entry - different resolutions";
				bool selectedEntry = SelectBestEntryForEncoder(*encoderResolutionsIterator,mayEntriesSet,selectedEntriesPerEncodersMap,false);
			}
		}
	}

	VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "SVC entry for may (VSR_ENTRIES_SELECT)";
	VSR_ENTRIES_SELECT_DEBUG_PRINTS	DumpSelection(encodersResolutionsSet,selectedEntriesPerEncodersMap);

	// 3) if free encoders - select may rtv entries - currently out of logic (low rtv takes HD encoder)
	/*
	if(selectedEntriesPerEncodersMap.size() < encodersResolutionsSet.size()){

		VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "selecting rtv entry for free encoder (VSR_ENTRIES_SELECT)";

		MsVsrEntriesSet rtvMayEntriesSet;
		FillMayEntriesSet(rtvMayEntriesSet,MS_SVC_PT_RTV); // fill may entries of svc paload only

		for(ResolutionsSet::iterator encoderResolutionsIterator = encodersResolutionsSet.begin(); encoderResolutionsIterator != encodersResolutionsSet.end(); ++encoderResolutionsIterator)
		{
			// entry is already assigned to this encoder resolution - skip
			if(selectedEntriesPerEncodersMap.find(*encoderResolutionsIterator)!=selectedEntriesPerEncodersMap.end())
			{
				continue;
			}

			bool selectedEntry = SelectBestEntryForEncoder(*encoderResolutionsIterator,rtvMayEntriesSet, selectedEntriesPerEncodersMap);
		}
	}
	*/

	VSR_ENTRIES_SELECT_DEBUG_PRINTS	TRACEINTO << "end (VSR_ENTRIES_SELECT)";

	return selectedEntriesPerEncodersMap.size();
}
//=========================================================================================//
bool CMsVsrMsg::SelectBestEncoderForVsrEntry(MsVsrEntriesSet::iterator& entriesIterator, const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap)
{
	bool encoderFound = false;
	// empty sets or all encoders resolutions already assigned to entries
	if(encodersResolutionsSet.size() == 0 || selectedEntriesPerEncodersMap.size() > encodersResolutionsSet.size()){
		TRACEINTO << " return 0 : num of encoders = " << encodersResolutionsSet.size() << " , num of in use encoders = " << selectedEntriesPerEncodersMap.size();
		return false;
	}

	EVideoResolutionType entryResolutionType = (*entriesIterator).GetEncoderResolutionType();
	// encoder sorted from low to high (CIF -> SD -> HD)
	// look for first encoder that can handle entry resolution
	for(ResolutionsSet::iterator encoderResolutionsIterator = encodersResolutionsSet.begin(); encoderResolutionsIterator != encodersResolutionsSet.end(); ++encoderResolutionsIterator)
	{
		if(selectedEntriesPerEncodersMap.find(*encoderResolutionsIterator)!=selectedEntriesPerEncodersMap.end())
		{
			continue;
		}
		// take first encoder that can handle entry resolution
		if((*encoderResolutionsIterator) >= entryResolutionType){
			selectedEntriesPerEncodersMap.insert(std::make_pair(*encoderResolutionsIterator,*entriesIterator));
			encoderFound = true;
			break;
		}
	}

	// if encoder with entry resolution not found - for example only CIF encoder in encodersResolutionsSet and all entries has SD/HD resoltion
	// take highest free encoder
	if(false == encoderFound){

		for(ResolutionsSet::reverse_iterator encoderReverseIterator = encodersResolutionsSet.rbegin(); encoderReverseIterator != encodersResolutionsSet.rend(); ++encoderReverseIterator)
		{
			if(selectedEntriesPerEncodersMap.find(*encoderReverseIterator)!=selectedEntriesPerEncodersMap.end())
			{
				continue;
			}
			// highest free encoder
			selectedEntriesPerEncodersMap.insert(std::make_pair(*encoderReverseIterator,*entriesIterator));
			encoderFound = true;
			break;
		}
	}
	return encoderFound;
}

//=========================================================================================//
DWORD CMsVsrMsg::FillMustEntriesSet(MsVsrEntriesSet& entriesSet)
{
	for(WORD entryIndex=0;entryIndex<GetNumOfEntries();entryIndex++)
	{
		if(m_VsrMsgParamsSt.st_vsrs_params[entryIndex].num_of_must > 0)
		{
			entriesSet.insert(CMsVsrEntry(m_VsrMsgParamsSt.st_vsrs_params[entryIndex]));
		}
	}
	return entriesSet.size();
}
//=========================================================================================//
DWORD CMsVsrMsg::FillMayEntriesSet(MsVsrEntriesSet& entriesSet,DWORD payload_type)
{
	for(WORD entryIndex=0;entryIndex<GetNumOfEntries();entryIndex++)
	{
		if(m_VsrMsgParamsSt.st_vsrs_params[entryIndex].num_of_may > 0 && m_VsrMsgParamsSt.st_vsrs_params[entryIndex].num_of_must == 0)
		{
			if(payload_type == m_VsrMsgParamsSt.st_vsrs_params[entryIndex].payload_type || ((DWORD)(-1)) == payload_type)
			//CMsVsrEntry entryCopy(m_VsrMsgParamsSt.st_vsrs_params[entryIndex]);
			// entryCopy.AlignToHd720();
			entriesSet.insert(m_VsrMsgParamsSt.st_vsrs_params[entryIndex]);
		}
	}
	return entriesSet.size();
}
//=========================================================================================//
bool CMsVsrMsg::SelectBestEntryForEncoder(EVideoResolutionType encoderResolutionType ,MsVsrEntriesSet& entiesSet, ResolutionsEntriesMap& selectedEntriesPerEncodersMap,bool same_resolution)
{
	bool found = false;
	if(0==entiesSet.size() || selectedEntriesPerEncodersMap.find(encoderResolutionType)!=selectedEntriesPerEncodersMap.end()){
		return false;
	}

	if(eHD720_Res == encoderResolutionType){
		// 1) look for the first entry with 720*1280 or higher resolution
		for(MsVsrEntriesSet::iterator entriesIterator = entiesSet.begin(); entriesIterator != entiesSet.end(); ++entriesIterator){
			if((*entriesIterator).GetMaxHight()>=720 && (*entriesIterator).GetMaxWidth()>=1280){
				selectedEntriesPerEncodersMap.insert(std::make_pair(encoderResolutionType,*entriesIterator));
				entiesSet.erase(entriesIterator);
				found = true;
				break;
			}
		}
	}

	if(!found){
		// 2) look for entry max num of may and with the same encoder resolution, if 2 entries with the same (highest) num of may - take the lower resolution
		MsVsrEntriesSet::iterator maxMayEntriesIterator = entiesSet.begin();
		for(MsVsrEntriesSet::iterator entriesIterator = entiesSet.begin(); entriesIterator != entiesSet.end(); ++entriesIterator){
			if(encoderResolutionType == (*entriesIterator).GetEncoderResolutionType() && (*entriesIterator).GetNumOfMay() > (*maxMayEntriesIterator).GetNumOfMay() ){
				maxMayEntriesIterator = entriesIterator;
				found = true;
			}
		}
		if(found){
			selectedEntriesPerEncodersMap.insert(std::make_pair(encoderResolutionType,*maxMayEntriesIterator));
			entiesSet.erase(maxMayEntriesIterator);
		}else if(!same_resolution){ // entry set contains only entries not of the encoder resolution
			// 3) select entry with highest resolution
			MsVsrEntriesSet::iterator lowerResolutionIterator = entiesSet.begin();
			MsVsrEntriesSet::iterator higherResolutionIterator = entiesSet.begin();
			DWORD lowerNumOfMay = 0;
			DWORD higherNumOfMay = 0;
			bool found_lower = false;
			bool found_higher = false;
			for(MsVsrEntriesSet::iterator entriesIterator = entiesSet.begin(); entriesIterator != entiesSet.end(); ++entriesIterator){
				if(encoderResolutionType > (*entriesIterator).GetEncoderResolutionType()){
					if((*entriesIterator).GetNumOfMay() >= lowerNumOfMay){
						lowerResolutionIterator = entriesIterator;
						lowerNumOfMay = (*entriesIterator).GetNumOfMay();
					}
					found_lower = true;
				}else{
					if((*entriesIterator).GetNumOfMay() >= higherNumOfMay){
						higherResolutionIterator = entriesIterator;
						higherNumOfMay = (*entriesIterator).GetNumOfMay();
					}
					found_higher = true;
					break;
				}
			}
			if(found_lower && !found_higher){
				selectedEntriesPerEncodersMap.insert(std::make_pair(encoderResolutionType,*lowerResolutionIterator));
				entiesSet.erase(lowerResolutionIterator);
			}else{
				selectedEntriesPerEncodersMap.insert(std::make_pair(encoderResolutionType,*higherResolutionIterator));
				entiesSet.erase(higherResolutionIterator);
			}
			found = true;
		}
	}
	return found;
}
//=========================================================================================//
/*
bool CMsVsrMsg::SelectBestEntryForEncoder(DWORD encoderResolution,DWORD previousEncoderResolution, MsVsrEntriesSet& entrySetPerEncoderResolution, stEntrySelectPolicy select_policy, ResolutionsEntriesMap& selectedEntriesPerEncodersMap)
{
	TRACEINTO << " VSR_FROM_AV_MCU_DEBUG: encoderResolution = " << encoderResolution << " , previousEncoderResolution = " << previousEncoderResolution << " , select_policy = " << select_policy;
	bool found = false;
	if(0==entrySetPerEncoderResolution.size()){
		return found;
	}
	switch(select_policy){
	case lowest_resolution:
	{

		std::set<CMsVsrEntry>::iterator minEntriesIterator = entrySetPerEncoderResolution.begin();
		TRACEINTO << " VSR_FROM_AV_MCU_DEBUG: lowest_resolution , minResolution = " <<  (*minEntriesIterator).GetResolution() << " , encoderResolution = "  << encoderResolution;
		if(*minEntriesIterator <= encoderResolution){
			selectedEntriesPerEncodersMap.insert(std::pair<DWORD,CMsVsrEntry>(encoderResolution,*minEntriesIterator));
			(*minEntriesIterator).Dump(); // VSR_FROM_AV_MCU_DEBUG
			entrySetPerEncoderResolution.erase(minEntriesIterator);
			found = true;
		}
		break;
	}
	case highest_resolution:
	{
		for(std::set<CMsVsrEntry>::reverse_iterator entriesIterator = entrySetPerEncoderResolution.rbegin(); entriesIterator != entrySetPerEncoderResolution.rend(); ++entriesIterator)
		{
			if(*entriesIterator <= encoderResolution){
				selectedEntriesPerEncodersMap.insert(std::pair<DWORD,CMsVsrEntry>(encoderResolution,*entriesIterator));
				entrySetPerEncoderResolution.erase(*entriesIterator);
				found = true;
				break;
			}
		}
		break;
	}
	case max_number_of_may:
	{

		std::set<CMsVsrEntry>::iterator maxMayEntriesIterator = entrySetPerEncoderResolution.begin();
		for(std::set<CMsVsrEntry>::iterator entriesIterator = entrySetPerEncoderResolution.begin(); entriesIterator != entrySetPerEncoderResolution.end(); ++entriesIterator)
		{
			if(encoderResolution < *entriesIterator){
				break;
			}
			if(*maxMayEntriesIterator <= previousEncoderResolution && previousEncoderResolution < *entriesIterator){
				maxMayEntriesIterator = entriesIterator;
			}else if((*entriesIterator).GetNumOfMay() > (*maxMayEntriesIterator).GetNumOfMay() ){
				maxMayEntriesIterator = entriesIterator;
			}
			found = true;
		}
		TRACEINTO << " VSR_FROM_AV_MCU_DEBUG: max_number_of_may , maxMayEntries = " <<  (*maxMayEntriesIterator).GetNumOfMay() << " , encoderResolution = "  << encoderResolution;
		if(found){
			selectedEntriesPerEncodersMap.insert(std::pair<DWORD,CMsVsrEntry>(encoderResolution,*maxMayEntriesIterator));
			(*maxMayEntriesIterator).Dump(); // VSR_FROM_AV_MCU_DEBUG
			entrySetPerEncoderResolution.erase(maxMayEntriesIterator);
		}
		break;
	}
	case max_number_of_must:
	{
		std::set<CMsVsrEntry>::iterator maxMustEntriesIterator = entrySetPerEncoderResolution.begin();
		for(std::set<CMsVsrEntry>::iterator entriesIterator = entrySetPerEncoderResolution.begin(); entriesIterator != entrySetPerEncoderResolution.end(); ++entriesIterator)
		{
			if(encoderResolution < *entriesIterator){
				break;
			}
			if(previousEncoderResolution < *entriesIterator && *entriesIterator <= encoderResolution){
				if(false == found){// first in range
					maxMustEntriesIterator = entriesIterator;
					found = true;
				}else{
					if((*entriesIterator).GetNumOfMust() > (*maxMustEntriesIterator).GetNumOfMust()){
						maxMustEntriesIterator = entriesIterator;
					}
				}
			}
		}
		selectedEntriesPerEncodersMap.insert(std::pair<DWORD,CMsVsrEntry>(encoderResolution,*maxMustEntriesIterator));
		entrySetPerEncoderResolution.erase(maxMustEntriesIterator);
		break;
	}
	default:
	{
		DBGPASSERT(select_policy);
		std::set<CMsVsrEntry>::iterator entriesIterator = entrySetPerEncoderResolution.begin();
		selectedEntriesPerEncodersMap.insert(std::pair<DWORD,CMsVsrEntry>(encoderResolution,*entriesIterator));
		entrySetPerEncoderResolution.erase(entriesIterator);
		break;
	}
	}

	return found;
}
*/
//=========================================================================================//
void CMsVsrMsg::Dump(std::ostringstream& msg) const
{
	msg 	<< "Vsr Message: \n"
			<< "partyId: " << m_VsrMsgParamsSt.partyId
			<< " , sender_ssrc: " << m_VsrMsgParamsSt.sender_ssrc
			<< " , msi: " << m_VsrMsgParamsSt.msi
			<< " , key_frame: " << m_VsrMsgParamsSt.key_frame
			<< " , num_of_entries: " << m_VsrMsgParamsSt.num_vsrs_params
			<< "\n";
	for(WORD entryIndex=0;entryIndex<GetNumOfEntries();++entryIndex)
	{
		CMsVsrEntry vsrEntry(m_VsrMsgParamsSt.st_vsrs_params[entryIndex]);
		vsrEntry.Dump(msg);
	}
}
//=========================================================================================//
void CMsVsrMsg::DumpEncodersResolutionsSet(std::ostringstream& msg,const ResolutionsSet& encodersResolutionsSet) const
{
	msg << "EncodersResolutionsSet: ";

	for(ResolutionsSet::iterator encoderResolutionsIterator = encodersResolutionsSet.begin(); encoderResolutionsIterator != encodersResolutionsSet.end(); ++encoderResolutionsIterator)
	{
		const char* resStr = NULL;
		BYTE res = CStringsMaps::GetDescription(RESOLUTION_SLIDER_ENUM, *encoderResolutionsIterator, &resStr);
		if (res){
			msg << resStr << "  ";
		}else{
			msg << " unkown ";
		}
	}
	msg << "\n";
}

//=========================================================================================//
void CMsVsrMsg::DumpResolutionsEntriesMap(std::ostringstream& msg,ResolutionsEntriesMap&  selectedEntriesPerEncodersMap) const
{
	msg << "SelectedEntriesPerEncodersMap: \n";
	for(ResolutionsEntriesMap::iterator mapItr = selectedEntriesPerEncodersMap.begin(); mapItr != selectedEntriesPerEncodersMap.end(); ++mapItr)
	{
		const char* resStr = NULL;
		BYTE res = CStringsMaps::GetDescription(RESOLUTION_SLIDER_ENUM, (*mapItr).first, &resStr);
		if (res){
			msg << resStr << ": ";
		}else{
			msg << " unkown: ";
		}
		(*mapItr).second.Dump(msg);

	}
}
//=========================================================================================//
void CMsVsrMsg::Dump() const
{
	std::ostringstream msg;
	Dump(msg);
	TRACEINTO << "\n" << msg.str().c_str();
}
//=========================================================================================//
void CMsVsrMsg::DumpSelection(const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap)const
{
	std::ostringstream msg;
	Dump(msg);
	DumpEncodersResolutionsSet(msg,encodersResolutionsSet);
	DumpResolutionsEntriesMap(msg,selectedEntriesPerEncodersMap);

	TRACEINTO << "\n" << msg.str().c_str();
}

//=========================================================================================//
void CMsVsrMsg::SetSingleEntry(CMsVsrEntry& entry)
{
	m_VsrMsgParamsSt.num_vsrs_params = 1;
	SetEntry(entry,0);
	for(WORD entryIndex=1;entryIndex<VSR_MAX_ENTRIES;++entryIndex)
	{
		ResetEntry(entryIndex);
	}
}
//=========================================================================================//
void CMsVsrMsg::SetEntry(CMsVsrEntry& entry,WORD entyIndex)
{
	if(entyIndex >= VSR_MAX_ENTRIES){
		PASSERT_AND_RETURN(entyIndex);
	}
	m_VsrMsgParamsSt.st_vsrs_params[entyIndex] = entry.GetEntryParamsSt();
}
//=========================================================================================//
void CMsVsrMsg::ResetEntry(WORD entyIndex)
{
	if(entyIndex >= VSR_MAX_ENTRIES){
		PASSERT_AND_RETURN(entyIndex);
	}
	CMsVsrEntry emptyEntry;
	m_VsrMsgParamsSt.st_vsrs_params[entyIndex] = emptyEntry.GetEntryParamsSt();
}
//=========================================================================================//
const ST_VSR_SINGLE_STREAM& CMsVsrMsg::GetVsrSingleStreamStruct()const
{
	return m_VsrMsgParamsSt;
}
//=========================================================================================//
WORD CMsVsrMsg::GetNumOfEntries()const
{
	WORD max_entries = m_VsrMsgParamsSt.num_vsrs_params;
	if(max_entries > VSR_MAX_ENTRIES){
		max_entries = VSR_MAX_ENTRIES;
	}
	return max_entries;
}
//=========================================================================================//
void  CMsVsrMsg::SetEmptyVsr()
{
                m_VsrMsgParamsSt.num_vsrs_params = 0;
                m_VsrMsgParamsSt.msi = 0xFFFFFFFF;
}

//=========================================================================================//

/*

EVideoResolutionType CMsVsrMsg::GetEncoderResolutionType(DWORD encoder_resolution)const
{
	EVideoResolutionType videoResolutionType= eCIF_Res;
	if(encoder_resolution > 320*320 && encoder_resolution <= 800*600){
		videoResolutionType = eSD_Res;
	}else{
		videoResolutionType = eHD720_Res;
	}
	return videoResolutionType;
}


////// VSR structure required by party, external API..
	typedef struct
	{
					DWORD 	payload_type; //SVC or RTV
					DWORD 	aspect_ratio; //from decision matrix
					DWORD 	max_width; //from decision matrix
					DWORD 	max_height; //from decision matrix
					DWORD 	frame_rate; // enum: by msft RTP
					DWORD 	min_bitrate; //msft RTP: take care of units
					WORD	no_of_may; // At the moment, ignored for outgoing
					WORD    no_of_must; // At the moment, ignored for outgoing
	} ST_VSR_PARAMS;

	typedef struct
	{
					DWORD sender_ssrc; // RA
					DWORD partyId;
					DWORD msi; // source any on P2P
					DWORD key_frame; // bool: default YES
					DWORD num_vsrs_params; //1 for svc/rtv only, 2 - (deafault) rtv or svc
					ST_VSR_PARAMS st_vsrs_params[VSR_MAX_ENTRIES];
	} ST_VSR_SINGLE_STREAM;


*/
