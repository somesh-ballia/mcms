#ifndef INNERSTRUCTS_H_
#define INNERSTRUCTS_H_

#include "AllocateStructs.h"
#include "EnumsAndDefines.h"

class CUnitMFA;
class CRsrcReport;
class CSharedRsrcConfReport;

#define NUM_OF_FPGAS_PER_BOARD                4
#define BANDWIDTH_PER_FPGA                    1248

#define MAX_UNITS_NEEDED_FOR_VIDEO            4 //3 // (bridge-6630) MAX_UNITS_NEEDED_FOR_VIDEO changed to 4 - asymetric 1080p60 + additional avc to svc =>  1 unit for decoder, 2 units for encoder, 1 unit for evc to svc sd+cif encoders
#define MAX_UNITS_NEEDED_FOR_VIDEO_COP        14
#define MAX_UNITS_NEEDED_FOR_VIDEO_VSW28      7

#define MAX_REQUIRED_PORTS_PER_MEDIA_UNIT     2
#define MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP 8

#define MAX_REQUIRED_PORTS_PER_MEDIA_SOFTMCU_UNIT 4
#define MAX_REQUIRED_PORTS_SOFTMCU_UNIT           600  // maximum number of ports supported in the VMP is 160 encoders + 300 decoders, so we need 300*2=600
#define MAX_REQUIRED_PORTS_SOFTMFW_UNIT           1000

#define TOTAL_MAX_VIDEO_RESOURCES             MAX_UNITS_NEEDED_FOR_VIDEO*MAX_REQUIRED_PORTS_PER_MEDIA_UNIT // actually it is not more than 4
#define TOTAL_MAX_VIDEO_RESOURCES_COP         MAX_UNITS_NEEDED_FOR_VIDEO_COP*MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP

#define MAX_PORTS_FOR_MOVING                  16
#define MAX_NUMBER_OF_BOARD_COMPARERS         4

#define MAX_NUM_OF_TIP_SCREENS_PER_ART        7
#define MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU        15  // According to Yiye: 15/ART * 4 ART/Board = 60/Board

#define MAX_ENCODER_WEIGHT_PER_UNIT                   12
#define ACCELERATORS_PER_UNIT_NETRA                   3
#define NUM_PORTS_PER_ACCELERATOR_NETRA               16
#define MAX_FREE_UNIT_CAPACITY                        1000
#define NETRA_PORTS_SPREADING_FREE_CAPACITY_THRESHOLD 1000

//////////////////////////////////////////////////////////////////////////////
//Structures connected to finding optimal boards for allocation
//////////////////////////////////////////////////////////////////////////////

//this is data from the party that we need to know in order to decide about the allocations
struct PartyDataStruct
{
	DWORD                    m_monitor_conf_id;
	eNetworkPartyType        m_networkPartyType;
	eVideoPartyType          m_videoPartyType;
	ePartyRole               m_partyRole;
	BOOL                     m_allowReconfiguration;
	WORD                     m_subServiceId;
	WORD                     m_room_id; // TIP Cisco
	WORD                     m_reqBoardId; //TIP or SVC
	WORD                     m_reqArtUnitId; //TIP or AvMcu
	ETipPartyTypeAndPosition m_partyTypeTIP;
	WORD                     m_tipNumOfScreens;
	ISDN_SPAN_PARAMS_S*      m_pIsdn_Params_Request;
	eConfModeTypes           m_confModeType; // temporary
	BOOL                     m_countPartyAsICEinMFW;
	DWORD                    m_artCapacity;
	eConfMediaType           m_confMediaType;	// Saving the media type in order to support traffic shaping/300 SVC only
	eHdVswTypesInMixAvcSvc   m_HdVswTypeInMixAvcSvcMode;
};

struct BasePartyDataStruct
{
	BasePartyDataStruct()
	{
		memset(this, 0, sizeof(*this));
	}
	BasePartyDataStruct(
			eVideoPartyType videoPartyType,
			ePartyRole partyRole,
			PartyRsrcID partyId,
			BOOL bIsISDNParty,
			WORD artBoardId,
			WORD videoBoardId,
			eConfModeTypes confModeType,
			BOOL countPartyAsICEinMFW,
			WORD ipServiceId,
			BOOL bAddAudioAsVideo) :
		m_videoPartyType(videoPartyType), m_resourcePartyType(e_Audio), m_partyRole(partyRole), m_partyId(partyId), m_bIsISDNParty(bIsISDNParty), m_artBoardId(artBoardId),
		m_videoBoardId(videoBoardId), m_confModeType(confModeType), m_countPartyAsICEinMFW(countPartyAsICEinMFW), m_ipServiceId(ipServiceId), m_bAddAudioAsVideo(bAddAudioAsVideo)  {}

	eVideoPartyType     m_videoPartyType;
	ePartyResourceTypes m_resourcePartyType; /*out*/
	ePartyRole          m_partyRole;
	PartyRsrcID         m_partyId;
	BOOL                m_bIsISDNParty;
	WORD                m_artBoardId;
	WORD                m_videoBoardId;
	eConfModeTypes      m_confModeType;
	BOOL                m_countPartyAsICEinMFW;
	WORD                m_ipServiceId;
	BOOL                m_bAddAudioAsVideo;
};

struct MediaPort
{
	DWORD                    m_needed_bandwidth_in;
	DWORD                    m_needed_bandwidth_out;
	float                    m_needed_encoder_weight;
	float                    m_needed_capacity_promilles;
	WORD                     m_acceleratorId;
	WORD                     m_portId;
	eLogicalResourceTypes    m_type;
	ECntrlType               m_cntrl_type;
	DWORD                    m_connId;
	DWORD                    m_rsrcEntityId;
	BOOL                     m_bNewEntryInSharedMemory;

	eResourceTypes           GetPhysicalResourceType();
};

struct MediaUnit
{
	WORD                     m_UnitId;
	MediaPort                m_MediaPortsList[MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP]; // Olga - max between COP and CP

	float                    GetTotalNeededCapacity();
	float                    GetTotalNeededEncoderCapacity();
	float                    GetTotalNeededDecoderCapacity();
	DWORD                    GetTotalNeededBandwidthIn();
	DWORD                    GetTotalNeededBandwidthOut();
	float                    GetTotalNeededEncoderWeight();
	WORD                     GetTotalNeededVideoPortsPerType(BOOL isEncoderType); // isEncoderType=TRUE for encoders count, FALSE for decoders count.
	void                     SetAcceleratorIdForAllPorts(WORD acceleratorId);
};

struct AllocData
{
	eConfModeTypes           m_confModeType;
	WORD                     m_boardId;
	MediaUnit                m_unitsList[MAX_UNITS_NEEDED_FOR_VIDEO_COP]; // Olga - max between COP and CP

	AllocData();
	MediaUnit*               GetUnitThatIncludesThisPort(eLogicalResourceTypes type, ECntrlType cntrl_type = E_NORMAL);
};

struct VideoDataPerBoardStruct
{
	AllocData                m_VideoAlloc;
	WORD                     m_numFreeVideoCapacity;
	WORD                     m_numFreeVideoPortsCapacity;
	DWORD                    m_freeBandwidth_in[NUM_OF_FPGAS_PER_BOARD];
	DWORD                    m_freeBandwidth_out[NUM_OF_FPGAS_PER_BOARD];

	BOOL                     m_bWasDownGraded;
	eVideoPartyType          m_DownGradedPartyType;

	BOOL                     m_bFragmentedUnit;
	WORD                     m_NumVideoUnitsToReconfigure;

	WORD                     m_NumVideoPartiesSameConf;
	BOOL                     m_bCanBeAllocated;
};

struct RTMDataPerBoardStruct
{
	WORD                     m_numFreeRTMPorts;

	BOOL                     m_bCanBePartiallyAllocated;
	BOOL                     m_bCanBeAllocated;
};

struct ARTDataPerBoardStruct
{
	WORD                     m_numFreeArtPorts;

	BOOL                     m_bCanBeAllocated;
	BOOL                     m_bCanAndShouldReconfigureVideoUnitToART;
};

struct DataPerBoardStruct
{
	VideoDataPerBoardStruct  m_VideoData;
	RTMDataPerBoardStruct    m_RTMData;
	ARTDataPerBoardStruct    m_ARTData;
};

struct BestAllocStruct
{
	// Necessary data for video allocation
	AllocData                m_VideoAlloc;
	WORD                     m_NumVideoUnitsToReconfigure;

	// Necessary data for ART allocation
	WORD                     m_ArtBoardId;
	WORD                     m_NumARTUnitsToReconfigure;

	// Necessary data for RTM allocation
	ISDN_PARTY_IND_PARAMS_S* m_pIsdn_Params_Response;
};

struct AllocRequirements
{
	                         AllocRequirements() { AllFalse(); }
	void                     AllFalse() { memset(this, 0, sizeof(*this)); }

	BOOL                     m_bCheckRTMFullAvailability;
	BOOL                     m_bCheckRTMPartialAvailability;

	BOOL                     m_bCheckARTAvailability;
	BOOL                     m_bAllowVideoReconfigurationToART;

	BOOL                     m_bCheckVideoAvailability;
	// means that if party is HD or SD, it will not be downgrade, and if it's CIF, then it's on a fragmented unit
	// also means that no unit has to be reconfigured
	BOOL                     m_bCheckVideoAvailabilityOptimalUnits;
};

enum eBoardComparer
{
	eBoardComparer_None,
	eBoardComparer_Rtm_Free_Ports,
	eBoardComparer_Art_Free_Ports,
	eBoardComparer_Video_Free,
	eBoardComparer_Video_Parties,
	eBoardComparer_Video_Free_Ports,
	eBoardComparer_Max
};

inline const char* to_string(eBoardComparer val)
{
	static const char* BoardComparerEnumNames[] =
	{
		"eBoardComparer_None",
		"eBoardComparer_Rtm_Free_Ports",
		"eBoardComparer_Art_Free_Ports",
		"eBoardComparer_Video_Free",
		"eBoardComparer_Video_Parties",
		"eBoardComparer_Video_Free_Ports"
	};
	return (eBoardComparer_None <= val && val < eBoardComparer_Max) ? BoardComparerEnumNames[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eBoardComparer val) { return os << to_string(val); }

enum eSpanAllocation
{
	eSpanAllocation_LoadBalancing,
	eSpanAllocation_FillFromStart,
	eSpanAllocation_FillFromEnd,
	eSpanAllocation_Max
};

inline const char* to_string(eSpanAllocation val)
{
	static const char* enumNames[] =
	{
		"eSpanAllocation_LoadBalancing",
		"eSpanAllocation_FillFromStart",
		"eSpanAllocation_FillFromEnd",
	};
	return (eSpanAllocation_LoadBalancing <= val && val < eSpanAllocation_Max) ? enumNames[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eSpanAllocation val) { return os << to_string(val); }


//////////////////////////////////////////////////////////////////////////////
//END ---- Structures connected to finding optimal boards for allocation
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//Structures connected to move
//////////////////////////////////////////////////////////////////////////////
struct MoveDataEntity
{
	                         MoveDataEntity() { m_pMoveTo = NULL; }

	AllocData                m_MoveFrom;
	CUnitMFA*                m_pMoveTo;
};

struct MoveData
{
	MoveDataEntity           m_MoveDataEntities[MAX_PORTS_FOR_MOVING];
};


////////////////////////////////////////////////////////////////////////////
//                        CPartiesResources
////////////////////////////////////////////////////////////////////////////
class CBoardsStatistics
{
public:
	CBoardsStatistics();
	CBoardsStatistics& operator=(const CBoardsStatistics &other);

	DWORD GetTotalCunfiguredAudioPortsOnCards() const;
	DWORD GetTotalCunfiguredVideoPortsOnCards() const;

	DWORD GetTotalNumOfEnabledArtUnits() const;
	DWORD GetTotalNumOfEnabledVideoUnits() const;

	DWORD m_NumConfiguredAudioPorts[BOARDS_NUM];
	DWORD m_NumConfiguredVideoPorts[BOARDS_NUM];

	DWORD m_NumOfEnabledArtUnits[BOARDS_NUM];
	DWORD m_NumOfEnabledVideoUnits[BOARDS_NUM];

	DWORD m_NumOfEnabledUnits[BOARDS_NUM];	// Tsahi - remove this?
	DWORD m_NumOfUnits[BOARDS_NUM];			// Tsahi - remove this?
};


////////////////////////////////////////////////////////////////////////////
//                        CPartiesResources
////////////////////////////////////////////////////////////////////////////
struct CPartiesResources
{
	               CPartiesResources();
	void           Init();
	void           DumpToTrace(char* calledFrom) const;
	BOOL           HasParties();
	void           CalculateHasParties();
	void           SetIpServiceFactor(float factor, bool round_up);

	WORD           m_physical_audio_ports;
	WORD           m_physical_hd720_ports;

	// these are floats because for example SD30 is 8/3 cif
	float          m_logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES]; //the array is of size NUM_OF_PARTY_RESOURCE_TYPES, because it's bigger than audio + video (2) for mixed/auto mode
	float          m_logical_COP_num_parties; //carmit-fix
	BOOL           m_bHasParties;
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoPreview
////////////////////////////////////////////////////////////////////////////
class CVideoPreview
{
public:
	enum { ePREVIEW_IN, ePREVIEW_OUT, ePREVIEW_NONE };

	               CVideoPreview();
	               CVideoPreview(DWORD confID, DWORD partyID, WORD direction);

	               friend WORD operator==(const CVideoPreview&, const CVideoPreview&);
	               friend bool operator<(const CVideoPreview&, const CVideoPreview&);

	DWORD          m_confID;
	DWORD          m_partyID; // is it better to use rsrcPartyId ?
	WORD           m_direction;
};


////////////////////////////////////////////////////////////////////////////
//                        ResourcesInterface
////////////////////////////////////////////////////////////////////////////
class ResourcesInterface
{
public:
	               ResourcesInterface() { m_dongleNumParties = 0; }
	               ResourcesInterface(const ResourcesInterface& other) { m_dongleNumParties = other.m_dongleNumParties; }
	virtual       ~ResourcesInterface() { }

	virtual void   CalculateResourceReport(CRsrcReport* pReport) = 0;
	virtual void   CalculateConfResourceReport(CSharedRsrcConfReport* pReport) = 0;
	virtual BOOL   CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                             BYTE i_rmxPortGaugeThreshold = FALSE, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE) = 0;
	virtual BOOL   CheckIfOneMorePartyCanBeAddedCOP(ALLOC_PARTY_IND_PARAMS_S* pResult) = 0;
	virtual BOOL   CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix) = 0;
	virtual void   AddParty(BasePartyDataStruct& rPartyData) = 0;
	virtual void   RemoveParty(BasePartyDataStruct& rPartyData) = 0;
	virtual BOOL   IsThereAnyParty() = 0;
	virtual WORD   GetMaxNumberOfParties() = 0;
	virtual DWORD  GetOccupiedNumberOfParties() = 0;
	virtual WORD   GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv) = 0; //ICE 4 ports
	virtual STATUS IsRsrcEnough(CBoardsStatistics* pBoardsStatistics) = 0;
	virtual void   InitDongleRestriction(DWORD& num_parties, BOOL bRecalculateReservationPartyResources = FALSE) = 0;
	virtual int    SetSvcDongleRestriction(DWORD num_parties) { return num_parties; }
	virtual void   FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf, eVideoPartyType maxVideoPartyTypeInConf, CPartiesResources& partiesResourcesToFill) = 0;
	virtual void   UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources = FALSE) = 0;
	virtual void   InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources = FALSE) = 0;
	virtual STATUS ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure = FALSE) = 0;
	virtual STATUS CanSetConfigurationNow() = 0;
	virtual void   OnLicenseExpired() = 0;

	// multiple service
	virtual BOOL   CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                                        float service_factor, BOOL round_up, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE) = 0;
	virtual BOOL   CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, float service_factor, BOOL round_up, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix) = 0;
	virtual        ResourcesInterface* NewCopy() const = 0;
	virtual float  CalculateAdditionalLicenseForMixMode(class CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty) { return 0; }
	virtual float  GetTotalAllocatedVideoParties() { return 0; }
	virtual void   GetLogicalPartyWeight(eVideoPartyType videoPartyType, float& nonMixWeight, float& mixWeight) { nonMixWeight = 0; mixWeight = 0; }

protected:
	DWORD          m_dongleNumParties;
};

///////////////////////////////////////////////////////////////////////////////
//END ---- Other Structures
//////////////////////////////////////////////////////////////////////////////

#endif /*INNERSTRUCTS_H_*/
