#ifndef BOARD_H_
#define BOARD_H_

#include <set>
#include "PObject.h"
#include "CardsStructs.h"
#include "AllocateStructs.h"
#include "EnumsAndDefines.h"
#include "InnerStructs.h"

class CUnitMFA;
class CSpanRTM;
class CRtmChannelIds;

typedef std::set<CUnitMFA> CMediaUnitsList;
typedef std::set<CSpanRTM> CSpansList;
typedef std::set<CVideoPreview> CVideoPreviewList;

////////////////////////////////////////////////////////////////////////////
//                        CBoard
////////////////////////////////////////////////////////////////////////////
class CBoard : public CPObject
{
	CLASS_TYPE_1(CBoard, CPObject)
public:
	CBoard(WORD OneBasedBoardId);
	virtual ~CBoard();
	const char*      NameOf() const                                        { return "CBoard"; }

	CMediaUnitsList* GetMediaUnitsList()                                   { return m_pMediaUnitslist; }
	CSpansList*      GetSpansRTMlist()                                     { return m_pSpansRTMlist; }
	CRtmChannelIds*  GetRTMChannelIds()                                    { return m_pChannelIdsList; }
	WORD             GetOneBasedBoardId()                                  { return m_OneBasedBoardId; };
	const CUnitMFA*  GetMFA(WORD UnitId);
	const CSpanRTM*  GetRTM(WORD UnitId);

	void             InitializeChannelsIdsList(WORD subBid);
	void             RemoveChannelsIdsList();

	WORD             GetUtilizablePQUnitId()                               { return m_UtilizablePQUnitId; };
	void             SetUtilizablePQUnitId(WORD utilizablePQUnitId)        { m_UtilizablePQUnitId = utilizablePQUnitId; };

	WORD             GetDisplayBoardId()                                   { return m_DisplayBoardId; };
	void             SetDisplayBoardId(WORD displayBoardId)                { m_DisplayBoardId = displayBoardId; };

	WORD             GetCardsStartup(int condition)                        { return m_CardsStartup[condition]; };
	void             SetCardsStartup(int condition, WORD status)           { m_CardsStartup[condition] = status; };

	eCardType        GetCardType()                                         { return m_CardType; };
	void             SetCardType(eCardType cardType)                       { m_CardType = cardType; };

	float            CalculateNeededPromilles(ePortType type, DWORD artCapacity, BOOL isSvcOnly = FALSE, BOOL isAudioOnly = FALSE, eVideoPartyType videoPartyType = eVideo_party_type_dummy);
	WORD             CalculateNumberOfPortsPerUnit(ePortType type, DWORD artCapacity = 0);

	void             FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole = eParty_Role_regular_party, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode = eHdVswNon);
	void             FillVideoUnitsListCOP(eSessionType sessnType, eLogicalResourceTypes encType, AllocData& videoAllocData);

	WORD             GetAudioControllerUnitId() const                      { return m_AudioControllerUnitId; }
	void             SetAudioControllerUnitId( WORD audioControllerUnitId) { m_AudioControllerUnitId = audioControllerUnitId; }

	WORD             GetRecoveryReservedArtUnitId() const                  { return m_ArtReserveRecoveryUnitId; }
	void             SetRecoveryReservedArtUnitId( WORD artId)             { m_ArtReserveRecoveryUnitId = artId; }

	ECntrlType       GetACType() const                                     { return m_ACType; }
	void             SetACType( ECntrlType ACType)                         { m_ACType = ACType; }

	int              ChangeUnits(int numUnitsToChange, eUnitType newUnitType, BOOL is_Enabled = FALSE); //returns the number of units actually changed

	void             CountUnits(DWORD& audio, DWORD& video, BOOL bCountDisabledToo);

	void             DivideArtAndVideoUnitsAccordingToProportion(DWORD alreadyConfiguredArtUnits, DWORD alreadyConfiguredVideoUnits, DWORD& numArtUnitsPerBoard, DWORD& numVideoUnitsPerBoard);

	void             AddOneVideoParticipantsWithVideoOnThisBoard();
	void             AddOneVideoParticipantsWithArtOnThisBoard();
	void             RemoveOneVideoParticipantsWithVideoOnThisBoard();
	void             RemoveOneVideoParticipantsWithArtOnThisBoard();
	WORD             GetNumVideoParticipantsWithVideoOnThisBoard();
	WORD             GetNumVideoParticipantsWithArtOnThisBoard();
	void             AddOneCOPConfOnThisBoard();
	void             RemoveOneCOPConfOnThisBoard();
	WORD             GetNumCOPConfOnThisBoard();

	MenuID           AllocatePcmMenuId(PartyRsrcID partyId);
	void             DeallocatePcmMenuId(PartyRsrcID partyId);
	bool             IsPcmMenuIdExist(PartyRsrcID partyId) const;

	STATUS           AddVideoPreviewOnThisBoard(DWORD confID, DWORD partyID, WORD direction);
	void             RemoveVideoPreviewOnThisBoard(DWORD confID, DWORD partyID, WORD direction = CVideoPreview::ePREVIEW_NONE);

	BOOL             IsThereRoomLeftForArtOfHeavyParty();
	BOOL             CanAllocateHeavyPartyOnThisArtUnit(const CUnitMFA* pUnitMFA);

	void             UnregisterStateMachine();

	void             FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode = eHdVswNon);
	void             SetHighUsageCPUstate(bool onOff)                      { m_isHighUsageCPU = onOff; }
	bool             GetHighUsageCPUstate() const                          { return m_isHighUsageCPU; }

private:
	CMediaUnitsList*   m_pMediaUnitslist;
	CSpansList*        m_pSpansRTMlist;
	CRtmChannelIds*    m_pChannelIdsList; // Pool of channel Ids per board (actually per subBoard)

	//the unit id of a utilizable unit of PQ (0xFFFF means no such unit).
	WORD               m_UtilizablePQUnitId;

	//startup resources enabled, mfa complete, ivr check
	WORD               m_CardsStartup[COND_NUM];

	WORD               m_OneBasedBoardId;

	WORD               m_DisplayBoardId;

	eCardType          m_CardType;

	WORD               m_AudioControllerUnitId;
	WORD               m_ArtReserveRecoveryUnitId;

	ECntrlType         m_ACType;
	WORD               m_NumOfVideoParticipantsWithVideoOnThisBoard;
	WORD               m_NumOfVideoParticipantsWithArtOnThisBoard;
	WORD               m_NumOfCOPConfOnThisBoard;

	PartyRsrcID        m_PcmMenuId[MAX_NUM_PCM_MENU_PER_BOARD_MPMX];

	CVideoPreviewList* m_videoPreviewList;

	bool               m_isHighUsageCPU;
};

class CBreezeHelper
{
public:
	static float     CalculateNeededPromilles(ePortType type, DWORD artCapacity, BOOL isSvcOnly = FALSE, BOOL isAudioOnly = FALSE, eVideoPartyType videoPartyType = eVideo_party_type_dummy);
	static float     artCapacityToPromilles(DWORD artCapacity, BOOL isSvcOnly = FALSE, BOOL isAudioOnly = FALSE, eVideoPartyType videoPartyType = eVideo_party_type_dummy);
	static void      FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode);
	static void      FillVideoUnitsListCOP(eSessionType sessnType, eLogicalResourceTypes encType, AllocData& videoAllocData);
	static void      FillVideoUnitsListVSW(eSessionType sessionType, AllocData& videoAllocData);
	static void      FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode);
};

class CMpmRxHelper
{
public:
	static float     CalculateNeededPromilles(ePortType type, DWORD artCapacity, BOOL isSvcOnly = FALSE, BOOL isAudioOnly = FALSE);
	static float     artCapacityToPromilles(DWORD artCapacity, BOOL isSvcOnly = FALSE, BOOL isAudioOnly = FALSE);
	static void      FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode);
	static void      FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode);
};


class CSoftMpmxHelper //OLGA - SoftMCU
{
public:
	static float     PortTypeToPromilles(ePortType type);
	static void      FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode);
	static void      FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode);
};

class CSoftMfwHelper //OLGA - SoftMCU - IBM
{
public:
	static float     PortTypeToPromilles(ePortType type);
	static void      FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData);
};

class CSoftNinjaHelper         //OLGA - SoftMCU - Ninja
{
public:
	static float     PortTypeToPromilles(ePortType type);
};

class CSoftCallGeneratorHelper // SoftMCU - Call Generator
{
public:
	static float     PortTypeToPromilles(ePortType type);
	static void      FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole);
};

#endif /*BOARD_H_*/
