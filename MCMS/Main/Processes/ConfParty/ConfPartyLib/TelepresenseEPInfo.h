/*
 * TelepresenseEPInfo.h
 *
 *  Created on: Jul 9, 2012
 *      Author: sshafrir
 */

#ifndef TELEPRESENSEEPINFO_H_
#define TELEPRESENSEEPINFO_H_

#include <map>
#include "SerializeObject.h"
#include "ConfPartyApiDefines.h"
#include "ConfPartySharedDefines.h"
#include "VideoDefines.h"
#include "VideoStructs.h"
#include "Segment.h"
#include <string>


#ifndef EMB_MLA_PRINTOUTS
#define EMB_MLA_PRINTOUTS 1
#endif



class CTelepresenseEPInfo : public CPObject

{ //abstract class

CLASS_TYPE_1(CTelepresenseEPInfo,CPObject)

public:
    // Constructors
    CTelepresenseEPInfo();
    CTelepresenseEPInfo( BYTE linkRole, DWORD roomID, DWORD numOfLinks,
                         DWORD linkNum, eTelePresencePartyType EPtype, DWORD partyMonitoringId);
    CTelepresenseEPInfo(const CTelepresenseEPInfo& other):CPObject(other) { *this = other; }
    virtual ~CTelepresenseEPInfo();
    virtual const char*           NameOf() const                               { return "CTelepresenseEPInfo"; }
    virtual void                Serialize(WORD format, CSegment& seg) const;
    virtual void                DeSerialize(WORD format, CSegment& seg);
    // Initializations:
    CTelepresenseEPInfo& operator=(const CTelepresenseEPInfo& other);
    // Get/Set:
    void SetLinkRole (BYTE linkRole) {m_linkRole = linkRole;}
    BYTE GetLinkRole () const {return m_linkRole;}

    //void SetIsActive (BYTE isActive) {m_isActive = isActive;}
    //BYTE GetIsActive () {return m_isActive;}

    void   SetRoomID (RoomID roomID) {m_roomID = roomID;}
    RoomID GetRoomID () const {return m_roomID;}

    void  SetNumOfLinks (DWORD numOfLinks) {m_numOfLinks = numOfLinks;}
    DWORD GetNumOfLinks () const {return m_numOfLinks;}

    void  SetLinkNum (DWORD linkNum) {m_linkNum = linkNum;}
    DWORD GetLinkNum () const {return m_linkNum;}

    void  SetEPtype (eTelePresencePartyType EPtype) {m_EPtype = EPtype;}
    eTelePresencePartyType GetEPtype () const {return m_EPtype;}

    void  SetPartyMonitorID (DWORD partyMonitorID) {m_partyMonitorID = partyMonitorID;}
    DWORD GetPartyMonitorID () const {return m_partyMonitorID;}

    void SetWaitForUpdate (BOOL bWaitForUpdate) {m_bWaitForUpdate = bWaitForUpdate;}
    BOOL GetWaitForUpdate () const {return m_bWaitForUpdate;}

    void Dump(std::ostringstream& msg, bool print_to_log = false) const;
    void Dump() const;

    void TestValidityAndCorrectParams();
    void UpdateLinkNumFromName( const char *mainLinkName );

protected:

    // Attributes

    BYTE       m_linkRole;  //0=regular , 1-link
    RoomID     m_roomID;
    DWORD      m_numOfLinks; //is updated just in MainLink..
    DWORD      m_linkNum;
    DWORD      m_partyMonitorID;
    //BYTE       m_isActive; //in confPaty already
    eTelePresencePartyType  m_EPtype;
    BOOL       m_bWaitForUpdate; //_e_m_
};



////////////////////////////////////////////////////////////////////////////
//                        CTelepresenceLayoutLogic
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

#define BLANK_CELL	(BYTE)(-1)

enum eRoomScreensNumber
{
	NO_SCREEN  = 0,
	ONE_SCREEN,
	TWO_SCREEN,
	THREE_SCREEN,
	FOUR_SCREEN
};

typedef enum _TelepresenceCellBorders
{
	eTelepresenceBordersNone = 0,
	eTelepresenceBordersStandalone,
	eTelepresenceBordersMiddleCell,
	eTelepresenceBordersLeftCell,
	eTelepresenceBordersRightCell,
	eTelepresenceBordersNotSpecified,
	eTelepresenceBordersLast

} eTelepresenceCellBorders;


static std::string CellBorderTypeStrings[] = { "No Border border type", "Standalone border type", "Middle Cell border type", "Left Cell border type", "Right Cell border type", "Unspecified Cell border type" };
bool operator==(BORDER_PARAM_S& first, const BORDER_PARAM_S& second);


static BORDER_PARAM_S gTelepresenceCellBorders[eTelepresenceBordersLast] =
{
		{0,0,0,0},	// no borders
		{1,1,1,1},	// all borders
		{0,0,1,1},	// middle screen - only top and bottom
		{0,1,1,1},	// left-most screen
		{1,0,1,1},	// right-most screen
		{0xFF,0xFF,0xFF,0xFF}	// right-most screen
};

bool GetTelepresenceBordersByTPTypeAndScreenNum(eTelePresencePartyType EPtype, eRoomScreensNumber maxScreensInRoom, WORD linkNum, eTelepresenceCellBorders& eBorderTypeResult, BORDER_PARAM_S& result);

void DumpTelepresenceBordersInfo(std::ostringstream& msg, const BORDERS_PARAM_S* pTelepresenceBorderParams);

typedef enum
{
	eReserved,
	eGrid
} EScreenType;

typedef enum
{
	ePos1 = 0,
	ePos2,
	ePos3,
	ePos4,
	ePosNone = 0xFF
} EScreenPosition;

typedef struct
{
	LayoutType  layoutType;
	BYTE 		IndexArr[MAX_SUB_IMAGES_IN_LAYOUT];

} TELEPRESENCE_LAYOUT_INDEXES_S;

typedef struct
{
	LayoutType   layout;
	EScreenType  screenType;

} TELEPRESENCE_SPEAKER_MODE_LAYOUT_S;


typedef std::map<WORD, TELEPRESENCE_LAYOUT_INDEXES_S*>::iterator TelepresenceLayoutIndexes;
typedef std::map<EScreenPosition, TELEPRESENCE_SPEAKER_MODE_LAYOUT_S> TelepresenceCpModeLayout;

/******************* COMMON **********************/
#define DEST_NUM_OF_SCREENS_1 0x0001
#define DEST_NUM_OF_SCREENS_2 0x0002
#define DEST_NUM_OF_SCREENS_3 0x0004
#define DEST_NUM_OF_SCREENS_4 0x0008

#define SPEAKER_NUM_OF_SCREENS_1 0x0100
#define SPEAKER_NUM_OF_SCREENS_2 0x0200
#define SPEAKER_NUM_OF_SCREENS_3 0x0400
#define SPEAKER_NUM_OF_SCREENS_4 0x0800

/******************* ROOM SWITCH ******************/
#define DEST_SCREEN_POSITION_1 0x0010
#define DEST_SCREEN_POSITION_2 0x0020
#define DEST_SCREEN_POSITION_3 0x0040
#define DEST_SCREEN_POSITION_4 0x0080


/************** SPEAKER MODE - RESERVED SCREEN *************/
#define MAX_NUM_CAMERAS_IN_CONF_1 0x0010
#define MAX_NUM_CAMERAS_IN_CONF_2 0x0020
#define MAX_NUM_CAMERAS_IN_CONF_3 0x0040
#define MAX_NUM_CAMERAS_IN_CONF_4 0x0080

#define SPEAKER2_NUM_OF_SCREENS_1 0x1000
#define SPEAKER2_NUM_OF_SCREENS_2 0x2000
#define SPEAKER2_NUM_OF_SCREENS_3 0x4000
#define SPEAKER2_NUM_OF_SCREENS_4 0x8000

#define SPEAKER3_NUM_OF_SCREENS_1 0x10000
#define SPEAKER3_NUM_OF_SCREENS_2 0x20000
#define SPEAKER3_NUM_OF_SCREENS_3 0x40000
#define SPEAKER3_NUM_OF_SCREENS_4 0x80000

/************** SPEAKER MODE - GRID SCREEN ****************/
#define GRID_NUM_OF_SCREENS_1  0x00000001
#define GRID_NUM_OF_SCREENS_2  0x00000002
#define GRID_NUM_OF_SCREENS_3  0x00000004

#define FILMSTRIP_NUM_0        0x00000100
#define FILMSTRIP_NUM_1        0x00000200
#define FILMSTRIP_NUM_2        0x00000400
#define FILMSTRIP_NUM_3        0x00000800

#define MAX_NUM_GRID_SCREENS_SPEAKER_MODE   3
#define MAX_NUM_GRID_SCREENS_PARTY_MODE     4
/*********************************************************/


class CTelepresenceLayoutLogic : public CSerializeObject //CPObject
{
	CLASS_TYPE_1(CTelepresenceLayoutLogic, CSerializeObject)
public:

	                               CTelepresenceLayoutLogic();
	virtual                       ~CTelepresenceLayoutLogic();
	virtual const char*            NameOf() const { return "CTelepresenceLayoutLogic"; }

	void                           Create();
	virtual WORD                   CreateScreenNumbersKey(DWORD roomNumOfScreens, DWORD screenPosition, DWORD speakerRoomNumOfScreens) {return 0;}
	TELEPRESENCE_LAYOUT_INDEXES_S* GetScreenLayoutEntity(WORD screenNumbers);

	//CSerializeObject overrides
	virtual CSerializeObject*      Clone() = 0;
    virtual void                   SerializeXml(CXMLDOMElement*& thisNode ) const;
    virtual int                    DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);

    static bool                    IsValidRoomLinksNum(BYTE num);

protected:
	virtual void                   CreateTelepresenceLayoutMap() = 0;
	void                           AddScreenLayoutEntity(WORD screenNumbers, TELEPRESENCE_LAYOUT_INDEXES_S* pTelepresenceLayoutIndexes);
	void                           SetAndAddScreenLayoutEntity1x1(WORD screenNumKey, TELEPRESENCE_LAYOUT_INDEXES_S *pTelepresenceLayoutIndexes, BYTE indexForCell0 );

private:
	std::map<WORD, TELEPRESENCE_LAYOUT_INDEXES_S*>    m_TelepresenceLayoutMap;
};

class CTelepresenceRoomSwitchLayoutLogic : public CTelepresenceLayoutLogic
{
	CLASS_TYPE_1(CTelepresenceRoomSwitchLayoutLogic, CTelepresenceLayoutLogic)

public:
	                            CTelepresenceRoomSwitchLayoutLogic();
	virtual                    ~CTelepresenceRoomSwitchLayoutLogic();
	virtual const char*         NameOf() const { return "CTelepresenceRoomSwitchLayoutLogic"; }

	WORD                        CreateScreenNumbersKey(DWORD roomNumOfScreens, DWORD screenPosition, DWORD speakerRoomNumOfScreens);
    virtual CSerializeObject*   Clone() { return new CTelepresenceRoomSwitchLayoutLogic; };

protected:
	virtual void                CreateTelepresenceLayoutMap();
};


class CTelepresenceCpLayoutLogic : public CTelepresenceLayoutLogic
{
	CLASS_TYPE_1(CTelepresenceSpeakerModeLayoutLogic, CTelepresenceLayoutLogic)
public:
	enum EGridLayoutCalcFunc
	{
		eFuncDefault = 0,
		eFuncUpTo3RoomsOf3Cam,
		eFuncMore3RoomsOf3Cam,
		eFuncUpTo4RoomsOf3And4Cam,
		eFuncUpTo4RoomsOf4CamAndUpTo3Rooms3,
		eFuncMore4RoomsOf4Cam,
		eFuncUpTo2RoomsOf2Cam,
		eFuncMore2RoomsOf2Cam
	};

	struct TelepresenceGridLayout
	{
		DWORD               key;
		BYTE                rangeFrom;  //from-to:  range of remaining cells
		BYTE                rangeTo;
		EGridLayoutCalcFunc funcId;
		BYTE                gridLayoutSpeakerMode[MAX_NUM_GRID_SCREENS_SPEAKER_MODE]; // array of indexes for speaker mode
		BYTE                gridLayoutPartyMode[MAX_NUM_GRID_SCREENS_PARTY_MODE];     // array of indexes for participants priority mode
		                                                                              // each index value defines some grid type from the global array: gGridLayoutArr
	};

	struct GridScreenCalcParams
	{
		void Init() { numRemainCells=0; twoCamCount=0; threeCamCount=0; fourCamCount=0; }
		WORD numRemainCells;
		WORD twoCamCount;
		WORD threeCamCount;
		WORD fourCamCount;
	};

public:
	                            CTelepresenceCpLayoutLogic() {}
	virtual                    ~CTelepresenceCpLayoutLogic() {}
	virtual const char*         NameOf() const { return "CTelepresenceCpLayoutLogic"; }

	virtual bool                GetRoomLayout(RoomID id, CRoomSpeakerVector& pSpeakersVect, TelepresenceCpModeLayout& rLayoutResult) { return true;}
	virtual bool                GetGridLayout(RoomID id, CRoomSpeakerVector& pSpeakersVect, TelepresenceCpModeLayout& rLayoutResult); // pass here the result of GetRoomLayout

	bool                        IsOverlayLayoutType(LayoutType type);

	static const char*          ScreenTypeAsString(EScreenType type);

protected:

	virtual WORD                GetMaxGridScreens()const = 0;
	virtual BYTE                GetGridTypeIndex(const TelepresenceGridLayout* gridL, WORD i)const = 0;

	DWORD                       CreateGridScreensKey(WORD gridNumOfScreens, WORD maxCamOfRemaining, WORD filmstripNumOfScreens=0xFFFF);

	bool                        CheckFuncCondition(EGridLayoutCalcFunc efunc, GridScreenCalcParams& param);
	bool                        GetGridLayoutByKey(DWORD key, GridScreenCalcParams& param, TelepresenceCpModeLayout& layoutResult);
	void                        CalcSpeakersParams(RoomID id, CRoomSpeakerVector& pSpeakersVect,
                                                   WORD& localRoomNumOfScreens,
                                                   WORD& maxCamerasWithoutLocal,
                                                   GridScreenCalcParams& params);

public:
	static void                 CreateGridLayoutMap();
	static std::multimap<DWORD, TelepresenceGridLayout>  m_gridScreenLayoutMap;
};



class CTelepresenceSpeakerModeLayoutLogic : public CTelepresenceCpLayoutLogic
{
	CLASS_TYPE_1(CTelepresenceSpeakerModeLayoutLogic, CTelepresenceCpLayoutLogic)

public:

	enum EReservedLayoutType
	{
		eCP_LAYOUT_NONE,
		eCP_LAYOUT_OVERLAY_1P,
		eCP_LAYOUT_OVERLAY_2P,
		eCP_LAYOUT_OVERLAY_3P,
		eCP_LAYOUT_OVERLAY_4P
	};

	struct SpeakerModeLayout
	{
		EScreenPosition     screenPosition; //  for example in local room with 4cam: | 3 | 1 | 0 | 2 |
		EScreenType         screenType;
		EReservedLayoutType reservedLayoutType;
	};


public:
	                            CTelepresenceSpeakerModeLayoutLogic();
	virtual                    ~CTelepresenceSpeakerModeLayoutLogic();
	virtual const char*         NameOf() const { return "CTelepresenceSpeakerModeLayoutLogic"; }

	virtual CSerializeObject*   Clone() { return new CTelepresenceSpeakerModeLayoutLogic; };

	bool                        GetRoomLayout(RoomID id, WORD localRoomNumOfScreens, CRoomSpeakerVector& pSpeakersVect, TelepresenceCpModeLayout& rLayoutResult);

protected:
	virtual void                CreateTelepresenceLayoutMap();
	virtual WORD                GetMaxGridScreens() const { return MAX_NUM_GRID_SCREENS_SPEAKER_MODE; }
	virtual BYTE                GetGridTypeIndex(const TelepresenceGridLayout* gridL, WORD i)const;

	DWORD                       CreateReservedScreensKey(WORD roomNumOfScreens, WORD maxConfCameras, WORD speaker, WORD prevSpeaker=0xFFFF, WORD thirdSpeaker=0xFFFF);

	void                        CalcSpeakersParams(RoomID id, CRoomSpeakerVector& pSpeakersVect,
                                                   WORD& localRoomNumOfScreens,
                                                   WORD& maxCamerasWithoutLocal,
                                                   WORD& speakerNumOfScreens,
                                                   WORD& prevSpeaker,
                                                   WORD& thirdSpeaker);

	std::map<EReservedLayoutType,LayoutType>               m_reservedLayoutTypeConversionMap;

public:
	static std::map<DWORD, std::vector<SpeakerModeLayout> >       m_reservedScreenLayoutMap;
	static void                 CreateReservedLayoutMap();
};


class CTelepresencePartyModeLayoutLogic : public CTelepresenceCpLayoutLogic
{
	CLASS_TYPE_1(CTelepresencePartyModeLayoutLogic, CTelepresenceCpLayoutLogic)

public:
	                            CTelepresencePartyModeLayoutLogic();
	virtual                    ~CTelepresencePartyModeLayoutLogic() {}
	virtual const char*         NameOf() const { return "CTelepresencePartyModeLayoutLogic"; }

	virtual CSerializeObject*   Clone() { return new CTelepresencePartyModeLayoutLogic; };

	bool                        GetRoomLayout(RoomID id, CRoomSpeakerVector& pSpeakersVect, TelepresenceCpModeLayout& rLayoutResult);

protected:
	virtual void                CreateTelepresenceLayoutMap();
	virtual WORD                GetMaxGridScreens()const { return MAX_NUM_GRID_SCREENS_PARTY_MODE; }
	virtual BYTE                GetGridTypeIndex(const TelepresenceGridLayout* gridL, WORD i)const;
};



#endif /* TELEPRESENSEEPINFO_H_ */
