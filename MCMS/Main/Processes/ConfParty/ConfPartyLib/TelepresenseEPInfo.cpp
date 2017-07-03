/*
 * TelepresenseEPInfo.cpp
 *
 *  Created on: Jul 9, 2012
 *      Author: sshafrir
 */
#include <string>
#include <stdlib.h>
#include <sstream>
#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "TelepresenseEPInfo.h"
#include "ConfPartySharedDefines.h"
#include "ObjString.h"
#include "StatusesGeneral.h"
#include "CPLayoutWrapper.h"

std::map<DWORD, std::vector<CTelepresenceSpeakerModeLayoutLogic::SpeakerModeLayout> > CTelepresenceSpeakerModeLayoutLogic::m_reservedScreenLayoutMap;
std::multimap<DWORD, CTelepresenceCpLayoutLogic::TelepresenceGridLayout>  CTelepresenceCpLayoutLogic::m_gridScreenLayoutMap;

extern const char* TelePresencePartyTypeToString(eTelePresencePartyType telePresencePartyType); //_e_m_

const char* CTelepresenceCpLayoutLogic::ScreenTypeAsString(EScreenType type)
{
	return (eGrid == type) ? "eGrid" : (eReserved == type ? "eReserved" : "eNone");
}


TELEPRESENCE_SPEAKER_MODE_LAYOUT_S gGridLayoutArr[] =
{
    {CP_LAYOUT_1X1,eGrid},
    {CP_LAYOUT_2X2,eGrid},
    {CP_LAYOUT_3X3,eGrid},
    {CP_LAYOUT_4X4,eGrid},
    {CP_NO_LAYOUT ,eGrid}
};


#define gridScreenKey_1x1    GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_1
#define gridScreenKey_1x1x0  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_1 | FILMSTRIP_NUM_0
#define gridScreenKey_1x1x3  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_1 | FILMSTRIP_NUM_3


CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid1Max1[] =
{
	{ gridScreenKey_1x1,    0,   1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {0, 4, 4}, {0, 4, 4, 4}},
	{ gridScreenKey_1x1,    2,   4, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 4, 4}, {1, 4, 4, 4}},
	{ gridScreenKey_1x1,    5,   9, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}},
	{ gridScreenKey_1x1,   10,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}}, //except case when there is no filmstrip
	{ gridScreenKey_1x1x0, 10,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}},
	{ gridScreenKey_1x1,   14,  21, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}, //except case when there are 3 filmstrip
	{ gridScreenKey_1x1x3, 14,  21, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}},
	{ gridScreenKey_1x1,   22,  -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}
};


#define gridScreenKey_1x2    GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_2
#define gridScreenKey_1x2x0  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_2 | FILMSTRIP_NUM_0
#define gridScreenKey_1x2x3  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_2 | FILMSTRIP_NUM_3

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid1Max2[] =
{
	{ gridScreenKey_1x2,    0,   4, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 4, 4}, {1, 4, 4, 4}},
	{ gridScreenKey_1x2,    5,   9, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}},
	{ gridScreenKey_1x2,   10,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}}, //except case when there is no filmstrip
	{ gridScreenKey_1x2x0, 10,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}},
	{ gridScreenKey_1x2,   14,  21, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}, //except case when there are 3 filmstrip
	{ gridScreenKey_1x2x3, 14,  21, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}},
	{ gridScreenKey_1x2,   22,  -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}
};


#define gridScreenKey_1x3    GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_3
#define gridScreenKey_1x3x0  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_3 | FILMSTRIP_NUM_0
#define gridScreenKey_1x3x3  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_3 | FILMSTRIP_NUM_3

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid1Max3[] =
{
	{ gridScreenKey_1x3,    0,   9, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}},
	{ gridScreenKey_1x3,   10,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}}, //except case when there is no filmstrip
	{ gridScreenKey_1x3x0, 10,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}},
	{ gridScreenKey_1x3,   14,  21, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}, //except case when there are 3 filmstrip
	{ gridScreenKey_1x3x3, 14,  21, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 4, 4}, {2, 4, 4, 4}},
	{ gridScreenKey_1x3,   22,  -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}
};


#define gridScreenKey_1x4  GRID_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_4

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid1Max4[] =
{
	{ gridScreenKey_1x4, 0, -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 4, 4}, {3, 4, 4, 4}}
};

#define gridScreenKey_2x1    GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_1
#define gridScreenKey_2x1x0  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_1 | FILMSTRIP_NUM_0
#define gridScreenKey_2x1x1  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_1 | FILMSTRIP_NUM_1
#define gridScreenKey_2x1x2  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_1 | FILMSTRIP_NUM_2

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid2Max1[] =
{
	{ gridScreenKey_2x1,    0,   2, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {0, 0, 4}, {0, 0, 4, 4}},
	{ gridScreenKey_2x1,    3,   5, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 0, 4}, {0, 1, 4, 4}},
	{ gridScreenKey_2x1,    6,   8, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 1, 4}, {1, 1, 4, 4}},
	{ gridScreenKey_2x1,    9,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 1, 4}, {1, 2, 4, 4}},
	{ gridScreenKey_2x1,   14,  18, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 4}, {2, 2, 4, 4}},

	{ gridScreenKey_2x1x0, 19,  22, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x1,   19,  22, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 4}, {2, 2, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x1x1, 23,  26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x1x2, 23,  26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 4}, {2, 2, 4, 4}},
	{ gridScreenKey_2x1,   23,  26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 1 or 2 filmstrip

	{ gridScreenKey_2x1x0, 27,  29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}},
	{ gridScreenKey_2x1,   27,  29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x1x2, 30,  33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x1,   30,  33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 2 filmstrip

	{ gridScreenKey_2x1,   34,  -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}}
};


#define gridScreenKey_2x2    GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_2
#define gridScreenKey_2x2x0  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_2 | FILMSTRIP_NUM_0
#define gridScreenKey_2x2x1  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_2 | FILMSTRIP_NUM_1
#define gridScreenKey_2x2x2  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_2 | FILMSTRIP_NUM_2

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid2Max2[] =
{
	{ gridScreenKey_2x2,    0,   5, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 0, 4}, {0, 1, 4, 4}},
	{ gridScreenKey_2x2,    6,   8, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 1, 4}, {1, 1, 4, 4}},
	{ gridScreenKey_2x2,    9,  13, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 1, 4}, {1, 2, 4, 4}},
	{ gridScreenKey_2x2,   14,  18, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 4}, {2, 2, 4, 4}},

	{ gridScreenKey_2x2x0, 19,  22, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x2,   19,  22, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 4}, {2, 2, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x2x1, 23,  26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x2x2, 23,  26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 4}, {2, 2, 4, 4}},
	{ gridScreenKey_2x2,   23,  26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 1 or 2 filmstrip

	{ gridScreenKey_2x2x0, 27,  29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}},
	{ gridScreenKey_2x2,   27,  29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x2x2, 30,  33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x2,   30,  33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 2 filmstrip

	{ gridScreenKey_2x2,   34,  -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 4}, {3, 3, 4, 4}}
};


#define gridScreenKey_2x3    GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3
#define gridScreenKey_2x3x0  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3 | FILMSTRIP_NUM_0
#define gridScreenKey_2x3x1  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3 | FILMSTRIP_NUM_1
#define gridScreenKey_2x3x2  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3 | FILMSTRIP_NUM_2

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid2Max3[] =
{
	{ gridScreenKey_2x3,    0,  3, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 0, 4}, {0, 2, 4, 4}},
	{ gridScreenKey_2x3,    4, 10, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 0, 4}, {0, 2, 4, 4}},
	{ gridScreenKey_2x3,   11, 13, CTelepresenceSpeakerModeLayoutLogic::eFuncUpTo3RoomsOf3Cam, {2, 1, 4}, {1, 2, 4, 4}}, // case when there are up to 3 rooms of 3 camera
	{ gridScreenKey_2x3,   11, 13, CTelepresenceSpeakerModeLayoutLogic::eFuncMore3RoomsOf3Cam, {2, 2, 4}, {2, 2, 4, 4}}, // case when there are more than 3 rooms of 3 camera
	{ gridScreenKey_2x3,   14, 18, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 2, 4}, {2, 2, 4, 4}},

	{ gridScreenKey_2x3x0, 19, 22, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x3,   19, 22, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 2, 4}, {2, 2, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x3x1, 23, 26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x3x2, 23, 26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 2, 4}, {2, 2, 4, 4}},
	{ gridScreenKey_2x3,   23, 26, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 1 or 2 filmstrip

	{ gridScreenKey_2x3x0, 27, 29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 4}, {3, 3, 4, 4}},
	{ gridScreenKey_2x3,   27, 29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 2, 4}, {2, 3, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x3x2, 30, 33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x3,   30, 33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 2 filmstrip

	{ gridScreenKey_2x3,   34, -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 4}, {3, 3, 4, 4}},
};


#define gridScreenKey_2x4    GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4
#define gridScreenKey_2x4x0  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | FILMSTRIP_NUM_0
#define gridScreenKey_2x4x2  GRID_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | FILMSTRIP_NUM_2

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid2Max4[] =
{
	{ gridScreenKey_2x4,    0, 17, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,                        {3, 0, 4}, {0, 3, 4, 4}},

	{ gridScreenKey_2x4,   18, 20, CTelepresenceSpeakerModeLayoutLogic::eFuncUpTo4RoomsOf3And4Cam,           {3, 1, 4}, {1, 3, 4, 4}}, //up to 4 rooms of 4/3 camera – one grid of 4x4, and one of 2x2
	{ gridScreenKey_2x4,   18, 20, CTelepresenceSpeakerModeLayoutLogic::eFuncUpTo4RoomsOf4CamAndUpTo3Rooms3, {3, 2, 4}, {2, 3, 4, 4}}, //up to 4 rooms of 4 and up to 3 rooms of 3 use 4x4 and 3x3 grids
	{ gridScreenKey_2x4,   18, 20, CTelepresenceSpeakerModeLayoutLogic::eFuncMore4RoomsOf4Cam,               {3, 3, 4}, {3, 3, 4, 4}}, //more than 4 rooms of 4 camera both grids are 4x4

	{ gridScreenKey_2x4,   21, 26, CTelepresenceSpeakerModeLayoutLogic::eFuncUpTo4RoomsOf4CamAndUpTo3Rooms3, {3, 2, 4}, {2, 3, 4, 4}}, //up to 4 rooms of 4 and up to 3 rooms of 3 use 4x4 and 3x3 grids
	{ gridScreenKey_2x4,   21, 26, CTelepresenceSpeakerModeLayoutLogic::eFuncMore4RoomsOf4Cam,               {3, 3, 4}, {3, 3, 4, 4}}, //more than 4 rooms of 4 camera both grids are 4x4

	{ gridScreenKey_2x4x0, 27, 29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,                        {3, 3, 4}, {3, 3, 4, 4}},
	{ gridScreenKey_2x4,   27, 29, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,                        {3, 2, 4}, {2, 3, 4, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_2x4x2, 30, 33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,                        {3, 2, 4}, {2, 3, 4, 4}},
	{ gridScreenKey_2x4,   30, 33, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,                        {3, 3, 4}, {3, 3, 4, 4}}, //except case when there are 2 filmstrip

	{ gridScreenKey_2x4,   34, -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,                        {3, 3, 4}, {3, 3, 4, 4}},
};

#define gridScreenKey_3x1    GRID_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_1
#define gridScreenKey_3x1x0  GRID_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_1 | FILMSTRIP_NUM_0

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid3Max1[] =
{
	{ gridScreenKey_3x1,    0,  3, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {0, 0, 0}, {0, 0, 0, 4}},
	{ gridScreenKey_3x1,    4,  6, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 0, 0}, {0, 1, 0, 4}},
	{ gridScreenKey_3x1,    7,  9, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 1, 0}, {0, 1, 1, 4}},
	{ gridScreenKey_3x1,   10, 12, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {1, 1, 1}, {1, 1, 1, 4}},
	{ gridScreenKey_3x1,   13, 17, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 1, 1}, {1, 2, 1, 4}},
	{ gridScreenKey_3x1,   18, 23, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 1}, {1, 2, 2, 4}},
	{ gridScreenKey_3x1,   24, 27, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 2}, {2, 2, 2, 4}},

	{ gridScreenKey_3x1x0, 28, 31, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 2}, {2, 3, 2, 4}},
	{ gridScreenKey_3x1,   28, 31, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {2, 2, 2}, {2, 2, 2, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_3x1x0, 32, 38, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 2}, {2, 3, 3, 4}},
	{ gridScreenKey_3x1,   32, 38, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 2, 2}, {2, 3, 2, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_3x1x0, 39, 45, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 3}, {3, 3, 3, 4}},
	{ gridScreenKey_3x1,   39, 45, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 2}, {2, 3, 3, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_3x1,   46, -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault, {3, 3, 3}, {3, 3, 3, 4}},
};


#define gridScreenKey_3x2    GRID_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_2
#define gridScreenKey_3x2x0  GRID_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_2 | FILMSTRIP_NUM_0

CTelepresenceCpLayoutLogic::TelepresenceGridLayout gLayoutGrid3Max2[] =
{
	{ gridScreenKey_3x2,    0,  6, CTelepresenceSpeakerModeLayoutLogic::eFuncUpTo2RoomsOf2Cam, {1, 0, 0}, {0, 1, 0, 4}},// case when there are up to 2 rooms of 2 camera
	{ gridScreenKey_3x2,    0,  6, CTelepresenceSpeakerModeLayoutLogic::eFuncMore3RoomsOf3Cam, {1, 1, 0}, {0, 1, 1, 4}},// if three 2-camera

	{ gridScreenKey_3x2,    7,  9, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {1, 1, 0}, {0, 1, 1, 4}},
	{ gridScreenKey_3x2,   10, 12, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {1, 1, 1}, {1, 1, 1, 4}},
	{ gridScreenKey_3x2,   13, 17, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 1, 1}, {1, 2, 1, 4}},
	{ gridScreenKey_3x2,   18, 23, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 2, 1}, {1, 2, 2, 4}},
	{ gridScreenKey_3x2,   24, 27, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 2, 2}, {2, 2, 2, 4}},

	{ gridScreenKey_3x2x0, 28, 31, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 2, 2}, {2, 3, 2, 4}},
	{ gridScreenKey_3x2,   28, 31, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {2, 2, 2}, {2, 2, 2, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_3x2x0, 32, 38, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 2}, {2, 3, 3, 4}},
	{ gridScreenKey_3x2,   32, 38, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 2, 2}, {2, 3, 2, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_3x2x0, 39, 45, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 3}, {3, 3, 3, 4}},
	{ gridScreenKey_3x2,   39, 45, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 2}, {2, 3, 3, 4}}, //except case when there is no filmstrip

	{ gridScreenKey_3x2,   46, -1, CTelepresenceSpeakerModeLayoutLogic::eFuncDefault,          {3, 3, 3}, {3, 3, 3, 4}}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTelepresenseEPInfo::CTelepresenseEPInfo()
{
    m_linkRole   = 0 ;  //0=regular , 1-link
    m_roomID     = (RoomID)-1;
    m_numOfLinks = 1;
    m_linkNum    = 1;
    m_partyMonitorID   = (DWORD)-1;
    m_EPtype     = eTelePresencePartyNone;
    m_bWaitForUpdate = FALSE; //_e_m_
}

/////////////////////////////////////////////////////////////////////////////
CTelepresenseEPInfo::CTelepresenseEPInfo( BYTE linkRole, DWORD roomID, DWORD numOfLinks, DWORD linkNum, eTelePresencePartyType EPtype, DWORD partyMonitoringId)
{
    m_linkRole   = linkRole;
    m_roomID     = roomID;
    m_numOfLinks = numOfLinks;
    m_linkNum    = linkNum;
    m_EPtype     = EPtype;
    m_partyMonitorID = partyMonitoringId;
    m_bWaitForUpdate = FALSE; //_e_m_
}

/////////////////////////////////////////////////////////////////////////////
CTelepresenseEPInfo& CTelepresenseEPInfo::operator=(const CTelepresenseEPInfo& other)
{
  if (&other == this)
    return *this;


  m_linkRole   = other.m_linkRole ;  //0=regular , 1-link
  m_roomID     = other.m_roomID;
  m_numOfLinks = other.m_numOfLinks;
  m_linkNum    = other.m_linkNum;
  m_partyMonitorID = other.m_partyMonitorID;
  m_EPtype     	   = other.m_EPtype;
  m_bWaitForUpdate = other.m_bWaitForUpdate; //_e_m_

  return *this;

}

/////////////////////////////////////////////////////////////////////////////
CTelepresenseEPInfo::~CTelepresenseEPInfo() // destructor
{
}

void CTelepresenseEPInfo::Serialize(WORD format, CSegment& seg) const
{
	seg << (BYTE)m_linkRole;  //0=regular , 1-link
	seg << (DWORD)m_roomID;
	seg << (DWORD)m_numOfLinks;
	seg << (DWORD)m_linkNum;
	seg << (DWORD)m_EPtype;
	seg << (BYTE)m_bWaitForUpdate; //_e_m_
}
void CTelepresenseEPInfo::DeSerialize(WORD format, CSegment& seg)
{
	seg >> (BYTE&)m_linkRole;  //0=regular , 1-link
	seg >> (DWORD&)m_roomID;
	seg >> (DWORD&)m_numOfLinks;
	seg >> (DWORD&)m_linkNum;
	DWORD tmp;
	seg >> (DWORD&)tmp;
	m_EPtype = (eTelePresencePartyType)tmp;
	seg >> (BYTE&)m_bWaitForUpdate;  //_e_m_
}

/////////////////////////////////////////////////////////////////////////////
void CTelepresenseEPInfo::Dump(std::ostringstream& msg, bool print_to_log) const 	//_e_m_
{
  msg << " m_roomID = " << m_roomID << ", m_linkNum = " << m_linkNum << ", m_numOfLinks = " << m_numOfLinks << " ,m_partyMonitorID = " << m_partyMonitorID << " , m_linkRole = ";
  if(0 == m_linkRole){
    msg << "regular";
  }else if(1 == m_linkRole){
    msg << "link";
  }else{
    msg << "unknown";
  }

  msg << " ,m_EPtype = " << TelePresencePartyTypeToString(m_EPtype);
  msg << " ,m_bWaitForUpdate = " << (WORD)m_bWaitForUpdate;

  if(print_to_log){
    TRACEINTO << msg.str().c_str();
  }
}
/////////////////////////////////////////////////////////////////////////////
void CTelepresenseEPInfo::Dump() const	 //_e_m_
{
  std::ostringstream msg;
  Dump(msg,true);
}
/////////////////////////////////////////////////////////////////////////////
void CTelepresenseEPInfo::TestValidityAndCorrectParams()
{
  if(eTelePresencePartyNone == m_EPtype &&  FALSE == m_bWaitForUpdate && m_linkRole == 0){
    if(m_numOfLinks != 1 || m_linkNum != 1){
      TRACEINTO << " m_numOfLinks = " << m_numOfLinks << " , m_linkNum = " << m_linkNum << " - Set to 1";
      m_numOfLinks = 1;
      m_linkNum = 0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// duplication of CTelepresenceLayoutMngr::GetLinkPartyIndex
void CTelepresenseEPInfo::UpdateLinkNumFromName( const char *mainLinkName )
{
	PASSERTMSG_AND_RETURN(mainLinkName == NULL, "CTelepresenseEPInfo::UpdateLinkNumFromName: null party name");
	WORD ch = strlen(mainLinkName);
	PASSERTMSG_AND_RETURN(ch==0, "CTelepresenseEPInfo::UpdateLinkNumFromName: Empty party name");
	TRACEINTO << "Received name " << mainLinkName << " ,which has length " << ch;

	WORD index = 0;
	ch--;
	if (mainLinkName[ch]>='0' && mainLinkName[ch]<='9')
	{
		index = (WORD)atoi(&mainLinkName[ch]);
		if (index == 0 || index > MAX_CASCADED_LINKS_NUMBER)
		{
			CSmallString str;
			str << "CTelepresenseEPInfo::UpdateLinkNumFromName: Invalid party name index " << mainLinkName;
			PASSERTMSG_AND_RETURN(1,str.GetString());
		}
		else
			m_linkNum = --index;
	}
	else
	{
		CSmallString str;
		str << "CTelepresenseEPInfo::UpdateLinkNumFromName: Invalid format of party name " << mainLinkName;
		PASSERTMSG_AND_RETURN(1,str.GetString());
	}
}
/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//                        CTelepresenceLayoutLogic
////////////////////////////////////////////////////////////////////////////
CTelepresenceLayoutLogic::CTelepresenceLayoutLogic()
{
}
////////////////////////////////////////////////////////////////////////////
CTelepresenceLayoutLogic::~CTelepresenceLayoutLogic()
{
	TelepresenceLayoutIndexes it;

	while(!m_TelepresenceLayoutMap.empty())
	{
		it = m_TelepresenceLayoutMap.begin();
		POBJDELETE(it->second);
		m_TelepresenceLayoutMap.erase(it);
	}
}

////////////////////////////////////////////////////////////////////////////
void CTelepresenceLayoutLogic::Create()
{
	CreateTelepresenceLayoutMap();
}
////////////////////////////////////////////////////////////////////////////

void CTelepresenceLayoutLogic::SerializeXml(CXMLDOMElement*& thisNode ) const
{
}
////////////////////////////////////////////////////////////////////////////

int CTelepresenceLayoutLogic::DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action)
{
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////
bool CTelepresenceLayoutLogic::IsValidRoomLinksNum(BYTE num) //EMB_MLA_OLGA
{
	return (num>0 && num<=MAX_CASCADED_LINKS_NUMBER);
}
////////////////////////////////////////////////////////////////////////////
void CTelepresenceLayoutLogic::AddScreenLayoutEntity(WORD screenNumbers, TELEPRESENCE_LAYOUT_INDEXES_S* pTelepresenceLayoutIndexes)
{
	std::pair<std::map<WORD, TELEPRESENCE_LAYOUT_INDEXES_S*>::iterator, bool> rc = m_TelepresenceLayoutMap.insert(std::make_pair(screenNumbers, pTelepresenceLayoutIndexes));
	PASSERTSTREAM_AND_RETURN(!rc.second, "CTelepresenceLayoutLogic::AddScreenLayoutEntity - Failed, Element already exists in map, key:" << screenNumbers);
}

////////////////////////////////////////////////////////////////////////////
TELEPRESENCE_LAYOUT_INDEXES_S* CTelepresenceLayoutLogic::GetScreenLayoutEntity(WORD screenNumbers)
{
	std::map<WORD, TELEPRESENCE_LAYOUT_INDEXES_S*>::const_iterator _ii = m_TelepresenceLayoutMap.find(screenNumbers);
	if (_ii == m_TelepresenceLayoutMap.end())
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, "Failed, The map doesn't have an element, key:" << screenNumbers, NULL);
	}
	return _ii->second;
}

////////////////////////////////////////////////////////////////////////////
void CTelepresenceLayoutLogic::SetAndAddScreenLayoutEntity1x1(	WORD screenNumKey,
																TELEPRESENCE_LAYOUT_INDEXES_S *pTelepresenceLayoutIndexes,
																BYTE indexForCell0 )
{
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X1;
	pTelepresenceLayoutIndexes->IndexArr[0] = indexForCell0;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CTelepresenceRoomSwitchLayoutLogic::CTelepresenceRoomSwitchLayoutLogic() : CTelepresenceLayoutLogic()
{
}
////////////////////////////////////////////////////////////////////////////
CTelepresenceRoomSwitchLayoutLogic::~CTelepresenceRoomSwitchLayoutLogic()
{
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
WORD CTelepresenceRoomSwitchLayoutLogic::CreateScreenNumbersKey(DWORD roomNumOfScreens, DWORD screenPosition, DWORD speakerRoomNumOfScreens)
{
	WORD destNumOfScreens = 0, destScreenPosition = 0, speakerNumOfScreens = 0;

	switch(roomNumOfScreens)
	{
	case 1:
		destNumOfScreens = DEST_NUM_OF_SCREENS_1;
		break;

	case 2:
		destNumOfScreens = DEST_NUM_OF_SCREENS_2;
		break;

	case 3:
		destNumOfScreens = DEST_NUM_OF_SCREENS_3;
		break;

	case 4:
		destNumOfScreens = DEST_NUM_OF_SCREENS_4;
		break;

	default:
		PASSERTSTREAM_AND_RETURN_VALUE(1, "CTelepresenceLayoutLogic::CreateScreenNumbersKey - invalid destination number of screens: " << roomNumOfScreens, 0);
	}

	switch(screenPosition)
	{
	case 0:
		destScreenPosition = DEST_SCREEN_POSITION_1;
		break;

	case 1:
		destScreenPosition = DEST_SCREEN_POSITION_2;
		break;

	case 2:
		destScreenPosition = DEST_SCREEN_POSITION_3;
		break;

	case 3:
		destScreenPosition = DEST_SCREEN_POSITION_4;
		break;

	default:
		PASSERTSTREAM_AND_RETURN_VALUE(1, "CTelepresenceLayoutLogic::CreateScreenNumbersKey - invalid destination screen position: " << screenPosition, 0);
	}

	switch(speakerRoomNumOfScreens)
	{
	case 1:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_1;
		break;

	case 2:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_2;
		break;

	case 3:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_3;
		break;

	case 4:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_4;
		break;

	default:
		PASSERTSTREAM_AND_RETURN_VALUE(1, "CTelepresenceLayoutLogic::CreateScreenNumbersKey - invalid speaker number of screens: " << speakerRoomNumOfScreens, 0);
	}

	WORD screenNumKey = destNumOfScreens | destScreenPosition | speakerNumOfScreens;

	return screenNumKey;
}
////////////////////////////////////////////////////////////////////////////
void CTelepresenceRoomSwitchLayoutLogic::CreateTelepresenceLayoutMap()
{
	WORD screenNumKey = 0;
	TELEPRESENCE_LAYOUT_INDEXES_S* pTelepresenceLayoutIndexes = NULL;

	// Destination: 1 screen:

	// Destination: screen #1 out of 1 screen; Speaker: 1 screen
	screenNumKey = DEST_NUM_OF_SCREENS_1 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #1 out of 1 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_1 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_2;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - from SRS: Note: special 1x2 4:3 layout when RPX is talker
	pTelepresenceLayoutIndexes->IndexArr[0] = 2;
	pTelepresenceLayoutIndexes->IndexArr[1] = 1;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #1 out of 1 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_1 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_3;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_3X3;
	pTelepresenceLayoutIndexes->IndexArr[0] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[1] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[2] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[3] = 2;
	pTelepresenceLayoutIndexes->IndexArr[4] = 1;
	pTelepresenceLayoutIndexes->IndexArr[5] = 3;
	pTelepresenceLayoutIndexes->IndexArr[6] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[7] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[8] = BLANK_CELL;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #1 out of 1 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_1 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_4;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1P8UP;
	pTelepresenceLayoutIndexes->IndexArr[0] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[1] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[2] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[3] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[4] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[5] = 3;
	pTelepresenceLayoutIndexes->IndexArr[6] = 1;
	pTelepresenceLayoutIndexes->IndexArr[7] = 2;
	pTelepresenceLayoutIndexes->IndexArr[8] = 4;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);


	// Destination: 2 screens:

	// Destination: screen #1 out of 2 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 2 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);

	// Destination: screen #1 out of 2 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 2 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 2);

	// Destination: screen #1 out of 2 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_3;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = 1;
	pTelepresenceLayoutIndexes->IndexArr[1] = 3;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #2 out of 2 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_3;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[1] = 2;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #1 out of 2 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_4;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = 1;
	pTelepresenceLayoutIndexes->IndexArr[1] = 3;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #2 out of 2 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_2 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_4;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = 4;
	pTelepresenceLayoutIndexes->IndexArr[1] = 2;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);


	// Destination: 3 screens:

	// Destination: screen #1 out of 3 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 3 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);

	// Destination: screen #3 out of 3 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);


	// Destination: screen #1 out of 3 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 3 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 2);

	// Destination: screen #3 out of 3 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);


	// Destination: screen #1 out of 3 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 3 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 2);

	// Destination: screen #3 out of 3 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 3);


	// Destination: screen #1 out of 3 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_4;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = 2;
	pTelepresenceLayoutIndexes->IndexArr[1] = 1;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #2 out of 3 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_4;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = BLANK_CELL;
	pTelepresenceLayoutIndexes->IndexArr[1] = 4;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);

	// Destination: screen #3 out of 3 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_3 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_4;
	pTelepresenceLayoutIndexes = new TELEPRESENCE_LAYOUT_INDEXES_S;
	pTelepresenceLayoutIndexes->layoutType = CP_LAYOUT_1X2_FLEX;	// ****TBD - is this the right layout type?
	pTelepresenceLayoutIndexes->IndexArr[0] = 3;
	pTelepresenceLayoutIndexes->IndexArr[1] = BLANK_CELL;
	AddScreenLayoutEntity(screenNumKey, pTelepresenceLayoutIndexes);


	// Destination: 4 screens:

	// Destination: screen #1 out of 4 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 4 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);

	// Destination: screen #3 out of 4 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);

	// Destination: screen #4 out of 4 screens; Speaker: 1 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_4 | SPEAKER_NUM_OF_SCREENS_1;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);


	// Destination: screen #1 out of 4 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 4 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 2);

	// Destination: screen #3 out of 4 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);

	// Destination: screen #4 out of 4 screens; Speaker: 2 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_4 | SPEAKER_NUM_OF_SCREENS_2;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);


	// Destination: screen #1 out of 4 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 3);

	// Destination: screen #2 out of 4 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #3 out of 4 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, BLANK_CELL);

	// Destination: screen #4 out of 4 screens; Speaker: 3 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_4 | SPEAKER_NUM_OF_SCREENS_3;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 2);


	// Destination: screen #1 out of 4 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_1 | SPEAKER_NUM_OF_SCREENS_4;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 1);

	// Destination: screen #2 out of 4 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_2 | SPEAKER_NUM_OF_SCREENS_4;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 2);

	// Destination: screen #3 out of 4 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_3 | SPEAKER_NUM_OF_SCREENS_4;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 3);

	// Destination: screen #4 out of 4 screens; Speaker: 4 screens
	screenNumKey = DEST_NUM_OF_SCREENS_4 | DEST_SCREEN_POSITION_4 | SPEAKER_NUM_OF_SCREENS_4;
	SetAndAddScreenLayoutEntity1x1(screenNumKey, pTelepresenceLayoutIndexes, 4);

}

bool GetTelepresenceBordersByTPTypeAndScreenNum(eTelePresencePartyType EPtype, eRoomScreensNumber maxScreensInRoom, WORD linkNum, eTelepresenceCellBorders& eBorderTypeResult, BORDER_PARAM_S& result)
{
	bool isValid = true;
	// default value- border not specified
	eBorderTypeResult = eTelepresenceBordersNotSpecified;
	switch (maxScreensInRoom)
	{
	case ONE_SCREEN:
	{
		switch (linkNum)
		{
		case 0:
			eBorderTypeResult = eTelepresenceBordersStandalone;
			break;
		default:
			isValid=false;
		}
		break;
	}
	case TWO_SCREEN:
	{
		switch (linkNum)
		{
		case 0:
			eBorderTypeResult = eTelepresenceBordersRightCell;
			break;
		case 1:
			eBorderTypeResult = eTelepresenceBordersLeftCell;
			break;
		default:
			isValid=false;
		}
		break;
	}
	case THREE_SCREEN:
	{
		switch (linkNum)
		{
		case 0:
			eBorderTypeResult = eTelepresenceBordersMiddleCell;
			break;
		case 1:
			eBorderTypeResult = eTelepresenceBordersLeftCell;
			break;
		case 2:
			eBorderTypeResult = eTelepresenceBordersRightCell;
			break;
		default:
			isValid=false;
		}
		break;
	}
	case FOUR_SCREEN:
	{
		switch (linkNum)
		{
		case 0:
			eBorderTypeResult = eTelepresenceBordersMiddleCell;
			break;
		case 1:
			eBorderTypeResult = eTelepresenceBordersMiddleCell;
			break;
		case 2:
			eBorderTypeResult = eTelepresenceBordersRightCell;
			break;
		case 3:
			eBorderTypeResult = eTelepresenceBordersLeftCell;
			break;
		default:
			isValid=false;
		}
		break;
	}
	case NO_SCREEN:
	default:
		isValid=false;
	}

	result = gTelepresenceCellBorders[eBorderTypeResult];



	//	switch (EPtype)
	//	{
	//	case eTelePresencePartyNone:
		//		break;
		//	  eTelePresencePartyRPX:
		//		break;
		//	  eTelePresencePartyFlex:
	//		break;
	//	  eTelePresencePartyMaui:
	//		break;
	//	  eTelePresencePartyInactive:
	//		break;
	//	default:
	//		isValid=false
	//	}
	return isValid;
}

bool operator==(BORDER_PARAM_S& first, const BORDER_PARAM_S& second)
{
	return first.ucDown == second.ucDown &&
			first.ucUp == second.ucUp &&
			first.ucLeft == second.ucLeft &&
			first.ucRight == second.ucRight;
}


void DumpTelepresenceBordersInfo(std::ostringstream& msg, const BORDERS_PARAM_S* pTelepresenceBorderParams)
{
	BORDER_PARAM_S unspecified = gTelepresenceCellBorders[eTelepresenceBordersNotSpecified];
	msg << " - DumpTelepresenceBordersInfo:";
	if (pTelepresenceBorderParams == NULL)
	{
		msg << " No specific border params present";
	}
	else
	{
		msg << " border params present-";
		bool found_specified = false;
		for (int counter=0; counter<MAX_NUMBER_OF_CELLS_IN_LAYOUT; ++counter)
		{
			BORDER_PARAM_S cell_borders = pTelepresenceBorderParams->tBorderEdges[counter];
			if (!(cell_borders == unspecified))
			{
				msg << "\nborders specified for cell " << counter << ": ";
				msg << " up " << (int)cell_borders.ucUp << ","
				<< " down " << (int)cell_borders.ucDown << ", "
				<< " left " << (int)cell_borders.ucLeft << ", "
				<< " right " << (int)cell_borders.ucRight;
				found_specified = true;
			}
		}
		if (!found_specified)
			msg << " borders not specified";
	}
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void CTelepresenceCpLayoutLogic::CreateGridLayoutMap()
{
	/************ 1 grid screen - max cameras of remaining cells is 1 ******************/

	size_t gridTableSize = sizeof(gLayoutGrid1Max1)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid1Max1[i].key, gLayoutGrid1Max1[i]));

	/************ 1 grid screen - max cameras of remaining cells is 2 ******************/

	gridTableSize = sizeof(gLayoutGrid1Max2)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid1Max2[i].key, gLayoutGrid1Max2[i]));

	/************ 1 grid screen - max cameras of remaining cells is 3 ******************/

	gridTableSize = sizeof(gLayoutGrid1Max3)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid1Max3[i].key, gLayoutGrid1Max3[i]));

	/************ 1 grid screen - max cameras of remaining cells is 4 ******************/

	gridTableSize = sizeof(gLayoutGrid1Max4)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid1Max4[i].key, gLayoutGrid1Max4[i]));

	/************ 2 grid screen - max cameras of remaining cells is 1 ******************/

	gridTableSize = sizeof(gLayoutGrid2Max1)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid2Max1[i].key, gLayoutGrid2Max1[i]));

	/************ 2 grid screen - max cameras of remaining cells is 2 ******************/

	gridTableSize = sizeof(gLayoutGrid2Max2)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid2Max2[i].key, gLayoutGrid2Max2[i]));

	/************ 2 grid screen - max cameras of remaining cells is 3 ******************/

	gridTableSize = sizeof(gLayoutGrid2Max3)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid2Max3[i].key, gLayoutGrid2Max3[i]));

	/************ 2 grid screen - max cameras of remaining cells is 4 ******************/

	gridTableSize = sizeof(gLayoutGrid2Max4)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid2Max4[i].key, gLayoutGrid2Max4[i]));

	/************ 3 grid screen - max cameras of remaining cells is 1 ******************/

	gridTableSize = sizeof(gLayoutGrid3Max1)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid3Max1[i].key, gLayoutGrid3Max1[i]));

	/************ 3 grid screen - max cameras of remaining cells is 2 ******************/

	gridTableSize = sizeof(gLayoutGrid3Max2)/sizeof(TelepresenceGridLayout);
	for(WORD i=0; i<gridTableSize; ++i)
		m_gridScreenLayoutMap.insert(std::make_pair(gLayoutGrid3Max2[i].key, gLayoutGrid3Max2[i]));
}

////////////////////////////////////////////////////////////////////////////
bool CTelepresenceCpLayoutLogic::IsOverlayLayoutType(LayoutType type)
{
	switch(type)
	{
//		case CP_LAYOUT_OVERLAY_ITP_1P2:
//		case CP_LAYOUT_OVERLAY_ITP_1P3:
		case CP_LAYOUT_OVERLAY_ITP_1P4:
			return true;
		default:
			return false;
	}
//	return false;
}
////////////////////////////////////////////////////////////////////////////

DWORD CTelepresenceCpLayoutLogic::CreateGridScreensKey(WORD gridNum, WORD maxCamNum, WORD filmstripNum)
{
	DWORD gridNumOfScreens = 0, maxCamOfRemaining = 0, filmstripNumOfScreens = 0;
	switch(gridNum)
	{
		case ONE_SCREEN:
			gridNumOfScreens = GRID_NUM_OF_SCREENS_1;
			break;

		case TWO_SCREEN:
			gridNumOfScreens = GRID_NUM_OF_SCREENS_2;
			break;

		case THREE_SCREEN:
			gridNumOfScreens = GRID_NUM_OF_SCREENS_3;
			break;
		default:
			TRACESTRFUNC(eLevelWarn) << "invalid grid screen number " << gridNum;
			return 0;
	}

	switch(maxCamNum)
	{
		case ONE_SCREEN:
			maxCamOfRemaining = MAX_NUM_CAMERAS_IN_CONF_1;
			break;

		case TWO_SCREEN:
			maxCamOfRemaining = MAX_NUM_CAMERAS_IN_CONF_2;
			break;

		case THREE_SCREEN:
			maxCamOfRemaining = MAX_NUM_CAMERAS_IN_CONF_3;
			break;

		case FOUR_SCREEN:
			maxCamOfRemaining = MAX_NUM_CAMERAS_IN_CONF_4;
			break;

		default:
			TRACESTRFUNC(eLevelWarn) << "invalid max num cameras " << maxCamNum;
			return 0;
	}
	if (filmstripNum != 0xFFFF)
	{
		switch(filmstripNum)
		{
			case NO_SCREEN:
				filmstripNumOfScreens = FILMSTRIP_NUM_0;
				break;

			case ONE_SCREEN:
				filmstripNumOfScreens = FILMSTRIP_NUM_1;
				break;

			case TWO_SCREEN:
				filmstripNumOfScreens = FILMSTRIP_NUM_2;
				break;

			case THREE_SCREEN:
				filmstripNumOfScreens = FILMSTRIP_NUM_3;
				break;
			default:
				TRACESTRFUNC(eLevelWarn) << "invalid filmstrip screen number " << filmstripNum;
				return 0;
		}
	}

	DWORD screenNumKey = gridNumOfScreens | maxCamOfRemaining | filmstripNumOfScreens;
	return screenNumKey;
}
////////////////////////////////////////////////////////////////////////////

bool CTelepresenceCpLayoutLogic::GetGridLayoutByKey(DWORD key, GridScreenCalcParams& params, TelepresenceCpModeLayout& rLayoutResult)
{
	bool gridLayoutFound = false;

	std::pair<std::multimap<DWORD, TelepresenceGridLayout>::const_iterator,
		  std::multimap<DWORD, TelepresenceGridLayout>::const_iterator> iGridItRange  = m_gridScreenLayoutMap.equal_range(key);

	size_t gridTableSize = sizeof(gGridLayoutArr)/sizeof(TELEPRESENCE_SPEAKER_MODE_LAYOUT_S);

	TelepresenceCpModeLayout::iterator iiEnd = rLayoutResult.end(), ii;
	if (iGridItRange.first != iGridItRange.second)
	{
		std::multimap<DWORD, TelepresenceGridLayout>::const_iterator theKeyIter;
		for (theKeyIter = iGridItRange.first; theKeyIter != iGridItRange.second;  ++theKeyIter)
		{
			const TelepresenceGridLayout* gridL = &theKeyIter->second;
//			ostr << "\n Grid : range=" << (WORD)gridL->rangeFrom << " to " << (WORD)gridL->rangeTo << ", numCells=" << numCells;
			if (params.numRemainCells >= gridL->rangeFrom && params.numRemainCells <= gridL->rangeTo && CheckFuncCondition(gridL->funcId, params))
			{
//				ostr << "\n Grid found: range from " << (WORD)gridL->rangeFrom << " to " << (WORD)gridL->rangeTo;
				gridLayoutFound = true;
				WORD maxNumScreens = GetMaxGridScreens();
				for (WORD i=0; i<maxNumScreens; i++)
				{
					BYTE gridTypeIndex = GetGridTypeIndex(gridL, i);
					if (gridTypeIndex < gridTableSize && gGridLayoutArr[gridTypeIndex].layout != CP_NO_LAYOUT)
					{
						for (ii = rLayoutResult.begin(); ii != iiEnd; ++ii)
						{
							if (eGrid == ii->second.screenType && CP_NO_LAYOUT == ii->second.layout)
							{
								ii->second.layout = gGridLayoutArr[gridTypeIndex].layout;
								break;
							}
						}
					}
				}
			}
		}
	}
	return gridLayoutFound;
}
////////////////////////////////////////////////////////////////////////////

bool CTelepresenceCpLayoutLogic::CheckFuncCondition(EGridLayoutCalcFunc efunc, GridScreenCalcParams& params)
{
	switch (efunc)
	{
		case eFuncDefault:
			return true;

		case eFuncUpTo3RoomsOf3Cam: // case when there are up to 3 rooms of 3 camera
			return (params.threeCamCount <= 3);

		case eFuncMore3RoomsOf3Cam: // case when there are more than 3 rooms of 3 camera
			return (params.threeCamCount > 3);

		case eFuncUpTo4RoomsOf3And4Cam: // up to 4 rooms of 4/3 camera – one grid of 4x4, and one of 2x2
			return ((params.threeCamCount+ params.fourCamCount) <= 4);

		case eFuncUpTo4RoomsOf4CamAndUpTo3Rooms3: // up to 4 rooms of 4 and up to 3 rooms of 3 use 4x4 and 3x3 grids
			return (params.threeCamCount <= 3 && params.fourCamCount <= 4);

		case eFuncMore4RoomsOf4Cam: //	more than 4 rooms of 4 camera both grids are 4x4
			return (params.fourCamCount > 4);

		case eFuncUpTo2RoomsOf2Cam: // if up to two 2-camera
			return (params.twoCamCount <= 2);

		case eFuncMore2RoomsOf2Cam: // if more than two 2-camera
			return (params.twoCamCount > 2);

		default:
			return true;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////

void CTelepresenceCpLayoutLogic::CalcSpeakersParams(RoomID id, CRoomSpeakerVector& pSpeakersVect,
													WORD& localRoomNumOfScreens,
													WORD& maxCamerasWithoutLocal,
													GridScreenCalcParams& params)
{
	params.Init();

	size_t speakersSize = pSpeakersVect.size();
	for(WORD i=0; i<speakersSize; ++i)
	{
		std::pair<RoomID, CPartyImageInfoVector> ii = pSpeakersVect.at(i);

		size_t numParties = ii.second.size();
		if (!numParties || ii.second.front().second != 0) // don't take in account already used images???
			continue;
		if (id == ii.first)
			localRoomNumOfScreens = numParties;
		else
		{
			if (IsValidRoomLinksNum(numParties))
			{
				switch(numParties)
				{
					case TWO_SCREEN:	params.twoCamCount += 1;	break;
					case THREE_SCREEN:	params.threeCamCount += 1;	break;
					case FOUR_SCREEN:	params.fourCamCount += 1;	break;
				}
				params.numRemainCells += numParties;
			}
			else
				TRACESTRFUNC(eLevelWarn) << "EMB_MLA_OLGA: room:" << ii.first << ", invalid number of room parties: " << numParties;
		}
	}

	maxCamerasWithoutLocal = ((params.fourCamCount>0)  ? FOUR_SCREEN  :
							 ((params.threeCamCount>0) ? THREE_SCREEN :
							 ((params.twoCamCount>0) ? TWO_SCREEN : ONE_SCREEN)));

}
////////////////////////////////////////////////////////////////////////////

bool CTelepresenceCpLayoutLogic::GetGridLayout(RoomID id, CRoomSpeakerVector& pSpeakersVect,
                                               TelepresenceCpModeLayout& rLayoutResult)
{
	WORD localRoomNumOfScreens = 0, maxCamerasWithoutLocal = 0;
	GridScreenCalcParams params;

	WORD gridCount = 0, filmCount = 0;
	TelepresenceCpModeLayout::iterator ii    = rLayoutResult.begin();
	TelepresenceCpModeLayout::iterator iiEnd = rLayoutResult.end();
	for (; ii != iiEnd; ++ii)
	{
		if (IsOverlayLayoutType(ii->second.layout))
			++filmCount;
		else if (eGrid == ii->second.screenType)
			++gridCount;
	}

	CTelepresenceCpLayoutLogic::CalcSpeakersParams(id, pSpeakersVect, localRoomNumOfScreens, maxCamerasWithoutLocal, params);

	ostringstream ostr;
	ostr << "EMB_MLA_OLGA:\nCreate grids screen key by the following parameters - RoomID:" << id << ", gridCount:" << gridCount
		 << ", filmCount:" << filmCount	<< ", localRoomNumOfScreens:" << localRoomNumOfScreens
		 << ", maxCamerasWithoutLocal:" << maxCamerasWithoutLocal << ", numRemainCells:" << params.numRemainCells;

	DWORD key = CreateGridScreensKey(gridCount, maxCamerasWithoutLocal, filmCount);

	ostr << "\n (1) create Grid Screens Key:" << key;

	bool gridLayoutFound = GetGridLayoutByKey(key, params, rLayoutResult);

	if (!gridLayoutFound)
	{
		key = CreateGridScreensKey(gridCount, maxCamerasWithoutLocal);
		ostr << "\n (2) create Grid Screens Key:" << key;
		gridLayoutFound = GetGridLayoutByKey(key, params, rLayoutResult);
	}
	TRACECOND_AND_RETURN_VALUE (!gridLayoutFound, ostr.str().c_str(), gridLayoutFound);

	TelepresenceCpModeLayout::iterator iDcsEnd   = rLayoutResult.end();
	for (TelepresenceCpModeLayout::iterator iDcs = rLayoutResult.begin(); iDcs != iDcsEnd; ++iDcs)
		ostr << "\nscreenPosition:" << iDcs->first << ", screenType: " << ScreenTypeAsString(iDcs->second.screenType) << ",  " << LayoutTypeAsString[iDcs->second.layout];

#ifdef EMB_MLA_PRINTOUTS
	TRACEINTO << ostr.str().c_str();//TEMP for debugging
#endif

	return gridLayoutFound;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CTelepresenceSpeakerModeLayoutLogic::CTelepresenceSpeakerModeLayoutLogic() : CTelepresenceCpLayoutLogic()
{
	m_reservedLayoutTypeConversionMap.insert(std::make_pair(eCP_LAYOUT_OVERLAY_1P, CP_LAYOUT_OVERLAY_ITP_1P4));
	m_reservedLayoutTypeConversionMap.insert(std::make_pair(eCP_LAYOUT_OVERLAY_2P, CP_LAYOUT_1X2_FLEX));
	m_reservedLayoutTypeConversionMap.insert(std::make_pair(eCP_LAYOUT_OVERLAY_3P, CP_LAYOUT_3X3));
	m_reservedLayoutTypeConversionMap.insert(std::make_pair(eCP_LAYOUT_OVERLAY_4P, CP_LAYOUT_4X4));
}

////////////////////////////////////////////////////////////////////////////
CTelepresenceSpeakerModeLayoutLogic::~CTelepresenceSpeakerModeLayoutLogic()
{
}

////////////////////////////////////////////////////////////////////////////
void CTelepresenceSpeakerModeLayoutLogic::CalcSpeakersParams(RoomID id, CRoomSpeakerVector& pSpeakersVect,
															 WORD& localRoomNumOfScreens,
															 WORD& maxCamerasWithoutLocal,
															 WORD& speakerNumOfScreens,
															 WORD& prevSpeaker,
															 WORD& thirdSpeaker)
{
	WORD index=0;

	std::vector<WORD> countSitesByScreensNum(MAX_CASCADED_LINKS_NUMBER+1, 0);

	size_t speakersSize = pSpeakersVect.size();
	for(WORD i=0; i < speakersSize; ++i)
	{
		std::pair<RoomID, CPartyImageInfoVector> ii= pSpeakersVect.at(i);
		size_t numParties = ii.second.size();
		if (id == ii.first)
			localRoomNumOfScreens = numParties;
		else
		{
			if (0 == index)
				speakerNumOfScreens = numParties;
			else if (1 == index)
				prevSpeaker = numParties;
			else if (2 == index)
				thirdSpeaker = numParties;

			++index;

			if (IsValidRoomLinksNum(numParties))
				++countSitesByScreensNum[numParties];
		}
	}

	maxCamerasWithoutLocal = ((countSitesByScreensNum.at(FOUR_SCREEN)>0)  ? FOUR_SCREEN  :
                             ((countSitesByScreensNum.at(THREE_SCREEN)>0) ? THREE_SCREEN :
                             ((countSitesByScreensNum.at(TWO_SCREEN)>0)   ? TWO_SCREEN : ONE_SCREEN)));

	if (FOUR_SCREEN == maxCamerasWithoutLocal && (THREE_SCREEN == localRoomNumOfScreens || FOUR_SCREEN == localRoomNumOfScreens))
	{
		maxCamerasWithoutLocal = (countSitesByScreensNum.at(THREE_SCREEN)>0) ? THREE_SCREEN : FOUR_SCREEN; //by SRS: 3 reserved screens only if there are 3 cameras else 2
	}
}

////////////////////////////////////////////////////////////////////////////
bool CTelepresenceSpeakerModeLayoutLogic::GetRoomLayout(RoomID id, WORD localRoomNumOfScreens, CRoomSpeakerVector& pSpeakersVect, TelepresenceCpModeLayout& rLayoutResult)
{
	ostringstream ostr;

	std::vector<WORD> countSitesByScreensNum(MAX_CASCADED_LINKS_NUMBER+1, 0);

	WORD localRoomScreens = 0, maxCamerasWithoutLocal = 0, speakerNumOfScreens = 0, prevSpeaker = 0, thirdSpeaker = 0, index=0;

	CalcSpeakersParams(id, pSpeakersVect, localRoomScreens, maxCamerasWithoutLocal, speakerNumOfScreens, prevSpeaker, thirdSpeaker);

	ostr << "EMB_MLA_OLGA:\n Create reserved screen key by the following parameters - RoomID:" << id << ", localRoomNumOfScreens:" << localRoomNumOfScreens
		 << ", maxCamerasWithoutLocal:" << maxCamerasWithoutLocal << ", speakerNumOfScreens:" << speakerNumOfScreens
		 << ", prevSpeakerNumOfScreens:" << prevSpeaker << ", thirdSpeakerNumOfScreens:" << thirdSpeaker;

	if (0==localRoomNumOfScreens)
	{
		TRACESTRFUNC(eLevelWarn) << "localRoomNumOfScreens:0 - return false - " << ostr.str().c_str();
		return false;
	}

	// decide about reserved screens
	std::map<DWORD, std::vector<SpeakerModeLayout> >::iterator iRsvEnd = m_reservedScreenLayoutMap.end();
	std::map<DWORD, std::vector<SpeakerModeLayout> >::iterator iRsvScreensByKey = iRsvEnd;
	DWORD key = 0;

	if (prevSpeaker != 0 && thirdSpeaker != 0)
	{
		key = CreateReservedScreensKey(localRoomNumOfScreens, maxCamerasWithoutLocal, speakerNumOfScreens, prevSpeaker, thirdSpeaker);
		ostr << "\n (1) create Reserved Screens Key:" << key;
		iRsvScreensByKey = m_reservedScreenLayoutMap.find(key);
	}
	if (iRsvScreensByKey == iRsvEnd && prevSpeaker != 0)
	{
		key = CreateReservedScreensKey(localRoomNumOfScreens, maxCamerasWithoutLocal, speakerNumOfScreens, prevSpeaker);
		ostr << "\n (2) create Reserved Screens Key:" << key;
		iRsvScreensByKey = m_reservedScreenLayoutMap.find(key);
	}
	if (iRsvScreensByKey == iRsvEnd)
	{
		key = CreateReservedScreensKey(localRoomNumOfScreens, maxCamerasWithoutLocal, speakerNumOfScreens);
		ostr << "\n (3) create Reserved Screens Key:" << key;
		iRsvScreensByKey = m_reservedScreenLayoutMap.find(key);
	}
	if (iRsvScreensByKey == iRsvEnd && 0 == thirdSpeaker)
	{
		thirdSpeaker = 1;
		key = CreateReservedScreensKey(localRoomNumOfScreens, maxCamerasWithoutLocal, speakerNumOfScreens, prevSpeaker, thirdSpeaker);
		ostr << "\n (4) create Reserved Screens Key:" << key;
		iRsvScreensByKey = m_reservedScreenLayoutMap.find(key);
	}
	if (iRsvScreensByKey == iRsvEnd && 0 == prevSpeaker)
	{
		prevSpeaker = 1;
		key = CreateReservedScreensKey(localRoomNumOfScreens, maxCamerasWithoutLocal, speakerNumOfScreens, prevSpeaker, thirdSpeaker);
		ostr << "\n (5) create Reserved Screens Key:" << key;
		iRsvScreensByKey = m_reservedScreenLayoutMap.find(key);
	}
	if (iRsvScreensByKey == iRsvEnd)
	{
		TRACESTRFUNC(eLevelWarn) << "ASSERT: key not found" << endl << ostr.str().c_str();
		return false;
	}
	else
	{
		ostr << "\n Screens layout entry is found";
		std::map<EReservedLayoutType,LayoutType>::iterator iiType;
		WORD gridCount = 0, filmCount = 0;

		std::vector<SpeakerModeLayout>           foundLayoutVect = iRsvScreensByKey->second;
		std::vector<SpeakerModeLayout>::iterator iFoundLayoutEnd = foundLayoutVect.end();

		for(std::vector<SpeakerModeLayout>::iterator it = foundLayoutVect.begin(); it != iFoundLayoutEnd; ++it)
		{
			ostr << "\nscreenType: " << ScreenTypeAsString(it->screenType);
			if (eGrid == it->screenType)
			{
				++gridCount;
				TELEPRESENCE_SPEAKER_MODE_LAYOUT_S screen = {CP_NO_LAYOUT, it->screenType};
				rLayoutResult.insert(make_pair(it->screenPosition, screen));
			}
			else
			{
				iiType = m_reservedLayoutTypeConversionMap.find(it->reservedLayoutType);
				if (iiType == m_reservedLayoutTypeConversionMap.end())
				{
					TRACESTRFUNC(eLevelWarn) << "EMB_MLA: invalid reserved Layout Type: " << it->reservedLayoutType;
					continue;
				}
				ostr << ", layoutType:" << LayoutTypeAsString[iiType->second];

				if (IsOverlayLayoutType(iiType->second))
					++filmCount;

				TELEPRESENCE_SPEAKER_MODE_LAYOUT_S screen = {iiType->second, it->screenType};
				rLayoutResult.insert(make_pair(it->screenPosition, screen));
			}
		}

		ostr << "\n\n Layout Decision vector size:" << rLayoutResult.size() << ", filmCount:" << filmCount << ", gridCount:" << gridCount;

		TelepresenceCpModeLayout::iterator iDcs = rLayoutResult.begin();
		TelepresenceCpModeLayout::iterator iDcsEnd = rLayoutResult.end();
		for (; iDcs != iDcsEnd; ++iDcs)
			ostr << "\nscreenPosition:"<< iDcs->first << ", screenType: " << ScreenTypeAsString(iDcs->second.screenType) << ",  " << LayoutTypeAsString[iDcs->second.layout];

#ifdef EMB_MLA_PRINTOUTS
		TRACEINTO << ostr.str().c_str(); //TEMP for debugging
#endif
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////
void CTelepresenceSpeakerModeLayoutLogic::CreateTelepresenceLayoutMap()
{
}
////////////////////////////////////////////////////////////////////////////

void CTelepresenceSpeakerModeLayoutLogic::CreateReservedLayoutMap()
{
	DWORD rsrvScreenNumKey = 0;
	SpeakerModeLayout rsrvOverlay1p = { ePos1, eReserved, eCP_LAYOUT_OVERLAY_1P };
	SpeakerModeLayout rsrvOverlay2p = { ePos1, eReserved, eCP_LAYOUT_OVERLAY_2P };
	SpeakerModeLayout rsrvOverlay3p = { ePos1, eReserved, eCP_LAYOUT_OVERLAY_3P };
	SpeakerModeLayout rsrvOverlay4p = { ePos1, eReserved, eCP_LAYOUT_OVERLAY_4P };
	SpeakerModeLayout gridStruct    = { ePos2, eGrid,     eCP_LAYOUT_NONE };

	std::vector<SpeakerModeLayout> layoutVectorOverlay1p, layoutVectorOverlay2p, layoutVectorOverlay3p, layoutVectorOverlay4p;
	layoutVectorOverlay1p.push_back(rsrvOverlay1p);
	layoutVectorOverlay2p.push_back(rsrvOverlay2p);
	layoutVectorOverlay3p.push_back(rsrvOverlay3p);
	layoutVectorOverlay4p.push_back(rsrvOverlay4p);

	/***********************************************************************************/
	/************************************ 1 local screen *******************************/
	/***********************************************************************************/

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_1 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));


	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay2p));


	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay2p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay3p));


	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay2p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay3p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_1 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay4p));

	/***********************************************************************************/
	/************************************ 2 local screen *******************************/
	/***********************************************************************************/

	layoutVectorOverlay1p.push_back(gridStruct);
	layoutVectorOverlay2p.push_back(gridStruct);
	layoutVectorOverlay3p.push_back(gridStruct);
	layoutVectorOverlay4p.push_back(gridStruct);

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_1 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));


	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay2p));


	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay1p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay2p));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay3p));

	// Local=2, max=4, speaker=1 ====> 2 reserved screens

	SpeakerModeLayout rsrvOverlay1p_2nd = { ePos2, eReserved, eCP_LAYOUT_OVERLAY_1P };
	SpeakerModeLayout rsrvOverlay2p_2nd = { ePos2, eReserved, eCP_LAYOUT_OVERLAY_2P };
	SpeakerModeLayout rsrvOverlay3p_2nd = { ePos2, eReserved, eCP_LAYOUT_OVERLAY_3P };
	SpeakerModeLayout rsrvOverlay4p_2nd = { ePos2, eReserved, eCP_LAYOUT_OVERLAY_4P };

	std::vector<SpeakerModeLayout> layoutVector;
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay3p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay4p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=2, max=4, speaker=2

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));


	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));


	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay3p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVectorOverlay4p.clear();
	layoutVectorOverlay4p.push_back(rsrvOverlay2p);
	layoutVectorOverlay4p.push_back(rsrvOverlay4p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay4p));

	// Local=2, max=4, speaker=3

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(rsrvOverlay3p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(rsrvOverlay4p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=2, max=4, speaker=4
	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_2 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	/***********************************************************************************/
	/************************************ 3 local screen *******************************/
	/***********************************************************************************/

	SpeakerModeLayout gridStruct_2nd = { ePos3, eGrid, eCP_LAYOUT_NONE };

	// Local=3, max=1

	layoutVector.clear();
	layoutVector.push_back(gridStruct);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_1 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=2

	layoutVectorOverlay2p.clear();
	layoutVectorOverlay2p.push_back(gridStruct);
	layoutVectorOverlay2p.push_back(rsrvOverlay2p);
	layoutVectorOverlay2p.push_back(gridStruct_2nd);

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVectorOverlay2p));

	// Local=3, max=3, speaker=1
	SpeakerModeLayout rsrvOverlay1p_3rd = { ePos3, eReserved, eCP_LAYOUT_OVERLAY_1P };
	SpeakerModeLayout rsrvOverlay2p_3rd = { ePos3, eReserved, eCP_LAYOUT_OVERLAY_2P };
	SpeakerModeLayout rsrvOverlay3p_3rd = { ePos3, eReserved, eCP_LAYOUT_OVERLAY_3P };
	SpeakerModeLayout rsrvOverlay4p_3rd = { ePos3, eReserved, eCP_LAYOUT_OVERLAY_4P };

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay2p_3rd);

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=3, speaker=2

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=3, speaker=3

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=3, speaker=4 special case for max=4 when there are 3 cameras

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay2p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay3p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(rsrvOverlay4p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=4, speaker=1 (in case there are no other 3 camera)

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=4, speaker=2

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=4, speaker=3

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(rsrvOverlay1p_3rd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=3, max=4, speaker=4

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_3 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	/***********************************************************************************/
	/************************************ 4 local screen *******************************/
	/***********************************************************************************/

	SpeakerModeLayout gridStruct_3rd    = { ePos4, eGrid,     eCP_LAYOUT_NONE };
	SpeakerModeLayout gridStruct_1st    = { ePos1, eGrid,     eCP_LAYOUT_NONE };

	// Local=4, max=1

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(gridStruct_1st);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_1 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=2

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(gridStruct_1st);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_2 | SPEAKER_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=3

	SpeakerModeLayout rsrvOverlay1p_4th = { ePos4, eReserved, eCP_LAYOUT_OVERLAY_1P };
	SpeakerModeLayout rsrvOverlay2p_4th = { ePos4, eReserved, eCP_LAYOUT_OVERLAY_2P };
	SpeakerModeLayout rsrvOverlay3p_4th = { ePos4, eReserved, eCP_LAYOUT_OVERLAY_3P };
	SpeakerModeLayout rsrvOverlay4p_4th = { ePos4, eReserved, eCP_LAYOUT_OVERLAY_4P };

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4 | SPEAKER3_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=3, speaker=3
	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=3, speaker=4  special case for local=4, max=4 and there are 3camera
	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay2p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay3p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay4p_4th);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_3 | SPEAKER_NUM_OF_SCREENS_4 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=4, speaker=1

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);

	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_1 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=4, speaker=2

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_1;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_2;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay3p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay4p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_2 | SPEAKER2_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=4, speaker=3

	layoutVector.clear();
	layoutVector.push_back(rsrvOverlay1p_4th);
	layoutVector.push_back(rsrvOverlay1p_2nd);
	layoutVector.push_back(rsrvOverlay1p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_3;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));

	// Local=4, max=4, speaker=4

	layoutVector.clear();
	layoutVector.push_back(gridStruct_3rd);
	layoutVector.push_back(rsrvOverlay2p_2nd);
	layoutVector.push_back(rsrvOverlay2p);
	layoutVector.push_back(gridStruct_2nd);
	rsrvScreenNumKey = DEST_NUM_OF_SCREENS_4 | MAX_NUM_CAMERAS_IN_CONF_4 | SPEAKER_NUM_OF_SCREENS_4;
	m_reservedScreenLayoutMap.insert(std::make_pair(rsrvScreenNumKey, layoutVector));
}

DWORD CTelepresenceSpeakerModeLayoutLogic::CreateReservedScreensKey(WORD roomNumOfScreens, WORD maxConfCameras, WORD speakerRoomNumOfScreens, WORD prevSpeaker, WORD thirdSpeaker)
{
	DWORD destNumOfScreens = 0, maxCameras = 0, speakerNumOfScreens = 0, speakerSecondNumOfScreens = 0, speakerThirdNumOfScreens = 0;

	switch(roomNumOfScreens)
	{
	case ONE_SCREEN:
		destNumOfScreens = DEST_NUM_OF_SCREENS_1;
		break;

	case TWO_SCREEN:
		destNumOfScreens = DEST_NUM_OF_SCREENS_2;
		break;

	case THREE_SCREEN:
		destNumOfScreens = DEST_NUM_OF_SCREENS_3;
		break;

	case FOUR_SCREEN:
		destNumOfScreens = DEST_NUM_OF_SCREENS_4;
		break;

	default:
		TRACESTRFUNC(eLevelWarn) << "invalid local number of screens: " << roomNumOfScreens;
		return 0;
	}

	switch(maxConfCameras)
	{
	case ONE_SCREEN:
		maxCameras = MAX_NUM_CAMERAS_IN_CONF_1;
		break;

	case TWO_SCREEN:
		maxCameras = MAX_NUM_CAMERAS_IN_CONF_2;
		break;

	case THREE_SCREEN:
		maxCameras = MAX_NUM_CAMERAS_IN_CONF_3;
		break;

	case FOUR_SCREEN:
		maxCameras = MAX_NUM_CAMERAS_IN_CONF_4;
		break;

	default:
		TRACESTRFUNC(eLevelWarn) << "invalid max cameras in conference" << maxConfCameras;
		return 0;
	}

	switch(speakerRoomNumOfScreens)
	{
	case ONE_SCREEN:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_1;
		break;

	case TWO_SCREEN:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_2;
		break;

	case THREE_SCREEN:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_3;
		break;

	case FOUR_SCREEN:
		speakerNumOfScreens = SPEAKER_NUM_OF_SCREENS_4;
		break;

	default:
		TRACESTRFUNC(eLevelWarn) << "invalid speaker number of screens: " << speakerRoomNumOfScreens;
		return 0;
	}

	if (prevSpeaker != 0xFFFF)// && prevSpeaker != 0)
	{
		switch(prevSpeaker)
		{
			case ONE_SCREEN:
				speakerSecondNumOfScreens = SPEAKER2_NUM_OF_SCREENS_1;
				break;

			case TWO_SCREEN:
				speakerSecondNumOfScreens = SPEAKER2_NUM_OF_SCREENS_2;
				break;

			case THREE_SCREEN:
				speakerSecondNumOfScreens = SPEAKER2_NUM_OF_SCREENS_3;
				break;

			case FOUR_SCREEN:
				speakerSecondNumOfScreens = SPEAKER2_NUM_OF_SCREENS_4;
				break;

			default:
				TRACESTRFUNC(eLevelWarn) << "invalid second speaker number of screens: " << speakerSecondNumOfScreens;
				return 0;
		}
	}
	if (thirdSpeaker != 0xFFFF)// && thirdSpeaker != 0)
	{
		switch(thirdSpeaker)
		{
			case ONE_SCREEN:
				speakerThirdNumOfScreens = SPEAKER3_NUM_OF_SCREENS_1;
				break;

			case TWO_SCREEN:
				speakerThirdNumOfScreens = SPEAKER3_NUM_OF_SCREENS_2;
				break;

			case THREE_SCREEN:
				speakerThirdNumOfScreens = SPEAKER3_NUM_OF_SCREENS_3;
				break;

			case FOUR_SCREEN:
				speakerThirdNumOfScreens = SPEAKER3_NUM_OF_SCREENS_4;
				break;

			default:
				TRACESTRFUNC(eLevelWarn) << "invalid third speaker number of screens: " << speakerThirdNumOfScreens;
				return 0;
		}
	}

	DWORD screenNumKey = destNumOfScreens | maxCameras | speakerNumOfScreens | speakerSecondNumOfScreens | speakerThirdNumOfScreens;
//	TRACEINTO << destNumOfScreens << ", " << maxCameras << ", " << speakerNumOfScreens << ", " << speakerSecondNumOfScreens << ", " << speakerThirdNumOfScreens << ", screenNumKey:" << screenNumKey;
	return screenNumKey;
}


BYTE CTelepresenceSpeakerModeLayoutLogic::GetGridTypeIndex(const TelepresenceGridLayout* gridL, WORD i)const
{
	return gridL ? gridL->gridLayoutSpeakerMode[i] : 0;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CTelepresencePartyModeLayoutLogic::CTelepresencePartyModeLayoutLogic() : CTelepresenceCpLayoutLogic()
{}

////////////////////////////////////////////////////////////////////////////

bool  CTelepresencePartyModeLayoutLogic::GetRoomLayout(RoomID id, CRoomSpeakerVector& pSpeakersVect, TelepresenceCpModeLayout& rLayoutResult)
{
	CRoomPartyVectorIter iend = pSpeakersVect.end();
	for (CRoomPartyVectorIter ii = pSpeakersVect.begin(); ii != iend; ++ii)
	{
		if (ii->first == id)
		{
			CPartyImageInfoVector& rPartyImages = ii->second;
			size_t numSpeakerImages = rPartyImages.size();
			for (BYTE indexSpeakerImages=0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
			{
				std::pair<PartyRsrcID, bool>& rCurrSpeakerImage = rPartyImages.at(indexSpeakerImages);

				TELEPRESENCE_SPEAKER_MODE_LAYOUT_S screen = {CP_NO_LAYOUT, eGrid};
				rLayoutResult.insert(make_pair((EScreenPosition)indexSpeakerImages, screen));
			}
			break;
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////
void CTelepresencePartyModeLayoutLogic::CreateTelepresenceLayoutMap()
{
	CreateGridLayoutMap();
}
////////////////////////////////////////////////////////////////////////////
BYTE CTelepresencePartyModeLayoutLogic::GetGridTypeIndex(const TelepresenceGridLayout* gridL, WORD i)const
{
	return gridL ? gridL->gridLayoutPartyMode[i] : 0;
}
