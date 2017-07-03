
#include "COP_Layout_definitions.h"
#include "ObjString.h"
#include "ConfPartyGlobals.h"

//======================================================================================================================================================================//
// Tables - static data members
//======================================================================================================================================================================//
sCopCellResolutionDef CCopLayoutResolutionTable::g_CopDecodersResolutionTableHD1080p25MPMPlus[TOTAL_NUM_OF_CELLS] = {    // {layout,cell_index,max_decoder_resolution,active_decoder_index}
  // 1 cell layout
  {CP_LAYOUT_1X1,0,COP_decoder_resolution_HD720p25,0},
  // 2 cells layout
  {CP_LAYOUT_1X2,0,COP_decoder_resolution_HD720p25,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_1X2,1,COP_decoder_resolution_HD720p25,6}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_2X1,0,COP_decoder_resolution_HD720p25,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_2X1,1,COP_decoder_resolution_HD720p25,6}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_1x2HOR,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1x2HOR,1,COP_decoder_resolution_HD720p25,6},

  {CP_LAYOUT_1x2VER,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1x2VER,1,COP_decoder_resolution_HD720p25,6},

  // 3 cells layout
  {CP_LAYOUT_1P2VER,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P2VER,1,COP_decoder_resolution_W4CIF25,6},
  {CP_LAYOUT_1P2VER,2,COP_decoder_resolution_W4CIF25,12},

  {CP_LAYOUT_1P2HOR,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P2HOR,1,COP_decoder_resolution_W4CIF25,6},
  {CP_LAYOUT_1P2HOR,2,COP_decoder_resolution_W4CIF25,12},

  {CP_LAYOUT_1P2HOR_UP,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P2HOR_UP,1,COP_decoder_resolution_W4CIF25,6},
  {CP_LAYOUT_1P2HOR_UP,2,COP_decoder_resolution_W4CIF25,12},

  // 4 cells layout
  {CP_LAYOUT_2X2,0,COP_decoder_resolution_W4CIF25,0},
  {CP_LAYOUT_2X2,1,COP_decoder_resolution_W4CIF25,6},
  {CP_LAYOUT_2X2,2,COP_decoder_resolution_W4CIF25,7},
  {CP_LAYOUT_2X2,3,COP_decoder_resolution_W4CIF25,12},

  {CP_LAYOUT_1P3HOR,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P3HOR,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P3HOR,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P3HOR,3,COP_decoder_resolution_4CIF25,12},

  {CP_LAYOUT_1P3VER,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P3VER,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P3VER,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P3VER,3,COP_decoder_resolution_4CIF25,12},

  {CP_LAYOUT_1P3HOR_UP,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P3HOR_UP,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P3HOR_UP,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P3HOR_UP,3,COP_decoder_resolution_4CIF25,12},


  // 5+ cells layout
  // 5 cells
  {CP_LAYOUT_1P4HOR,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P4HOR,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P4HOR,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P4HOR,3,COP_decoder_resolution_4CIF25,12},
  {CP_LAYOUT_1P4HOR,4,COP_decoder_resolution_4CIF25,13},

  {CP_LAYOUT_1P4VER,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P4VER,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P4VER,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P4VER,3,COP_decoder_resolution_4CIF25,12},
  {CP_LAYOUT_1P4VER,4,COP_decoder_resolution_4CIF25,13},

  {CP_LAYOUT_1P4HOR_UP,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P4HOR_UP,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P4HOR_UP,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P4HOR_UP,3,COP_decoder_resolution_4CIF25,12},
  {CP_LAYOUT_1P4HOR_UP,4,COP_decoder_resolution_4CIF25,13},

  // 6 cells
  {CP_LAYOUT_1P5,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P5,1,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_1P5,2,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_1P5,3,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_1P5,4,COP_decoder_resolution_4CIF25,12},
  {CP_LAYOUT_1P5,5,COP_decoder_resolution_4CIF25,13},

  // 8 cells
  {CP_LAYOUT_1P7,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P7,1,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P7,2,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P7,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P7,4,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P7,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P7,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P7,7,COP_decoder_resolution_CIF25,14},

  // 9 cells
  {CP_LAYOUT_3X3,0,COP_decoder_resolution_4CIF25,0},
  {CP_LAYOUT_3X3,1,COP_decoder_resolution_4CIF25,1},
  {CP_LAYOUT_3X3,2,COP_decoder_resolution_4CIF25,2},
  {CP_LAYOUT_3X3,3,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_3X3,4,COP_decoder_resolution_4CIF25,7},
  {CP_LAYOUT_3X3,5,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_3X3,6,COP_decoder_resolution_4CIF25,12},
  {CP_LAYOUT_3X3,7,COP_decoder_resolution_4CIF25,13},
  {CP_LAYOUT_3X3,8,COP_decoder_resolution_4CIF25,14},

  {CP_LAYOUT_1P8CENT,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P8CENT,1,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8CENT,2,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P8CENT,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8CENT,4,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8CENT,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8CENT,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P8CENT,7,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P8CENT,8,COP_decoder_resolution_CIF25,15},

  {CP_LAYOUT_1P8UP,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P8UP,1,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8UP,2,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P8UP,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8UP,4,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8UP,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8UP,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P8UP,7,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P8UP,8,COP_decoder_resolution_CIF25,15},

  {CP_LAYOUT_1P8HOR_UP,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P8HOR_UP,1,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8HOR_UP,2,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P8HOR_UP,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8HOR_UP,4,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8HOR_UP,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8HOR_UP,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P8HOR_UP,7,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P8HOR_UP,8,COP_decoder_resolution_CIF25,15},
  // 10+ cells layout
  // 10 cells
  {CP_LAYOUT_2P8,0,COP_decoder_resolution_W4CIF25,6},
  {CP_LAYOUT_2P8,1,COP_decoder_resolution_W4CIF25,7},
  {CP_LAYOUT_2P8,2,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_2P8,3,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_2P8,4,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_2P8,5,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_2P8,6,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_2P8,7,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_2P8,8,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_2P8,9,COP_decoder_resolution_CIF25,15},

  // 13 cells
  {CP_LAYOUT_1P12,0,COP_decoder_resolution_W4CIF25,12},
  {CP_LAYOUT_1P12,1,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_1P12,2,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_1P12,3,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_1P12,4,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_1P12,5,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P12,6,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P12,7,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P12,8,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P12,9,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P12,10,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P12,11,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P12,12,COP_decoder_resolution_CIF25,11},

  // 16 cells
  {CP_LAYOUT_4X4,0,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_4X4,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_4X4,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_4X4,3,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_4X4,4,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_4X4,5,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_4X4,6,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_4X4,7,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_4X4,8,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_4X4,9,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_4X4,10,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_4X4,11,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_4X4,12,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_4X4,13,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_4X4,14,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_4X4,15,COP_decoder_resolution_CIF25,15}
};
//======================================================================================================================================================================//
sCopCellResolutionDef CCopLayoutResolutionTable::g_CopDecodersResolutionTableHD720p50MPMPlus[TOTAL_NUM_OF_CELLS] = {    // {layout,cell_index,max_decoder_resolution,active_decoder_index}
  // 1 cell layout
  {CP_LAYOUT_1X1,0,COP_decoder_resolution_HD720p50,0},
  // 2 cells layout
  {CP_LAYOUT_1X2,0,COP_decoder_resolution_HD720p50,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_1X2,1,COP_decoder_resolution_HD720p50,8}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_2X1,0,COP_decoder_resolution_HD720p50,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_2X1,1,COP_decoder_resolution_HD720p50,8}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_1x2HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1x2HOR,1,COP_decoder_resolution_HD720p50,8},

  {CP_LAYOUT_1x2VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1x2VER,1,COP_decoder_resolution_HD720p50,8},
  // 3 cells layout
  {CP_LAYOUT_1P2VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P2VER,1,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_1P2VER,2,COP_decoder_resolution_4CIF25,9},

  {CP_LAYOUT_1P2HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P2HOR,1,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_1P2HOR,2,COP_decoder_resolution_4CIF25,9},

  {CP_LAYOUT_1P2HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P2HOR_UP,1,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_1P2HOR_UP,2,COP_decoder_resolution_4CIF25,9},
  // 4 cells layout
  {CP_LAYOUT_2X2,0,COP_decoder_resolution_4CIF25,0},
  {CP_LAYOUT_2X2,1,COP_decoder_resolution_4CIF25,1},
  {CP_LAYOUT_2X2,2,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_2X2,3,COP_decoder_resolution_4CIF25,9},

  {CP_LAYOUT_1P3HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P3HOR,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P3HOR,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P3HOR,3,COP_decoder_resolution_CIF25,10},

  {CP_LAYOUT_1P3VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P3VER,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P3VER,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P3VER,3,COP_decoder_resolution_CIF25,10},

  {CP_LAYOUT_1P3HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P3HOR_UP,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P3HOR_UP,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P3HOR_UP,3,COP_decoder_resolution_CIF25,10},

  // 5+ cells layout
  // 5 cells
  {CP_LAYOUT_1P4HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P4HOR,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4HOR,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P4HOR,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P4HOR,4,COP_decoder_resolution_CIF25,11},

  {CP_LAYOUT_1P4VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P4VER,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4VER,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P4VER,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P4VER,4,COP_decoder_resolution_CIF25,11},

  {CP_LAYOUT_1P4HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P4HOR_UP,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4HOR_UP,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P4HOR_UP,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P4HOR_UP,4,COP_decoder_resolution_CIF25,11},
  // 6 cells
  {CP_LAYOUT_1P5,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P5,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P5,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P5,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P5,4,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P5,5,COP_decoder_resolution_CIF25,12},

  // 8 cells
  {CP_LAYOUT_1P7,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P7,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P7,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P7,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P7,4,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P7,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P7,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P7,7,COP_decoder_resolution_CIF25,14},

  // 9 cells
  {CP_LAYOUT_3X3,0,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_3X3,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_3X3,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_3X3,3,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_3X3,4,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_3X3,5,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_3X3,6,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_3X3,7,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_3X3,8,COP_decoder_resolution_CIF25,11},

  {CP_LAYOUT_1P8CENT,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P8CENT,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8CENT,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8CENT,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8CENT,4,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P8CENT,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8CENT,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P8CENT,7,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P8CENT,8,COP_decoder_resolution_CIF25,15},

  {CP_LAYOUT_1P8UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P8UP,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8UP,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8UP,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8UP,4,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P8UP,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8UP,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P8UP,7,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P8UP,8,COP_decoder_resolution_CIF25,15},

  {CP_LAYOUT_1P8HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P8HOR_UP,1,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8HOR_UP,2,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8HOR_UP,3,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8HOR_UP,4,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P8HOR_UP,5,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8HOR_UP,6,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P8HOR_UP,7,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P8HOR_UP,8,COP_decoder_resolution_CIF25,15},
  // 10+ cells layout
  // 10 cells
  {CP_LAYOUT_2P8,0,COP_decoder_resolution_4CIF25,0},
  {CP_LAYOUT_2P8,1,COP_decoder_resolution_4CIF25,1},
  {CP_LAYOUT_2P8,2,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_2P8,3,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_2P8,4,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_2P8,5,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_2P8,6,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_2P8,7,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_2P8,8,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_2P8,9,COP_decoder_resolution_CIF25,15},
  // 13 cells
  {CP_LAYOUT_1P12,0,COP_decoder_resolution_4CIF25,0},
  {CP_LAYOUT_1P12,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_1P12,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_1P12,3,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_1P12,4,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P12,5,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P12,6,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P12,7,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P12,8,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P12,9,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P12,10,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_1P12,11,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_1P12,12,COP_decoder_resolution_CIF25,15},

  // 16 cells
  {CP_LAYOUT_4X4,0,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_4X4,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_4X4,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_4X4,3,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_4X4,4,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_4X4,5,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_4X4,6,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_4X4,7,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_4X4,8,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_4X4,9,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_4X4,10,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_4X4,11,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_4X4,12,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_4X4,13,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_4X4,14,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_4X4,15,COP_decoder_resolution_CIF25,15}
};

//===============================MPMX tables============================================================================================================================//

sCopCellResolutionDef CCopLayoutResolutionTable::g_CopDecodersResolutionTableHD1080p25MPMX[TOTAL_NUM_OF_CELLS] = {    // {layout,cell_index,max_decoder_resolution,active_decoder_index}
  // 1 cell layout
  {CP_LAYOUT_1X1,0,COP_decoder_resolution_HD108030,0},
  // 2 cells layout
  {CP_LAYOUT_1X2,0,COP_decoder_resolution_HD108030,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_1X2,1,COP_decoder_resolution_HD108030,4}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_2X1,0,COP_decoder_resolution_HD108030,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_2X1,1,COP_decoder_resolution_HD108030,4}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_1x2HOR,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1x2HOR,1,COP_decoder_resolution_HD108030,4},

  {CP_LAYOUT_1x2VER,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1x2VER,1,COP_decoder_resolution_HD108030,4},

  // 3 cells layout
  {CP_LAYOUT_1P2VER,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P2VER,1,COP_decoder_resolution_W4CIF25,4},
  {CP_LAYOUT_1P2VER,2,COP_decoder_resolution_W4CIF25,5},

  {CP_LAYOUT_1P2HOR,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P2HOR,1,COP_decoder_resolution_W4CIF25,4},
  {CP_LAYOUT_1P2HOR,2,COP_decoder_resolution_W4CIF25,5},

  {CP_LAYOUT_1P2HOR_UP,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P2HOR_UP,1,COP_decoder_resolution_W4CIF25,4},
  {CP_LAYOUT_1P2HOR_UP,2,COP_decoder_resolution_W4CIF25,5},

  // 4 cells layout
  {CP_LAYOUT_2X2,0,COP_decoder_resolution_W4CIF25,0},
  {CP_LAYOUT_2X2,1,COP_decoder_resolution_W4CIF25,1},
  {CP_LAYOUT_2X2,2,COP_decoder_resolution_W4CIF25,4},
  {CP_LAYOUT_2X2,3,COP_decoder_resolution_W4CIF25,5},

  {CP_LAYOUT_1P3HOR,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P3HOR,1,COP_decoder_resolution_4CIF25,4},
  {CP_LAYOUT_1P3HOR,2,COP_decoder_resolution_4CIF25,5},
  {CP_LAYOUT_1P3HOR,3,COP_decoder_resolution_4CIF25,8},

  {CP_LAYOUT_1P3VER,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P3VER,1,COP_decoder_resolution_4CIF25,4},
  {CP_LAYOUT_1P3VER,2,COP_decoder_resolution_4CIF25,5},
  {CP_LAYOUT_1P3VER,3,COP_decoder_resolution_4CIF25,8},

  {CP_LAYOUT_1P3HOR_UP,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P3HOR_UP,1,COP_decoder_resolution_4CIF25,4},
  {CP_LAYOUT_1P3HOR_UP,2,COP_decoder_resolution_4CIF25,5},
  {CP_LAYOUT_1P3HOR_UP,3,COP_decoder_resolution_4CIF25,8},


  // 5+ cells layout
  // 5 cells
  {CP_LAYOUT_1P4HOR,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P4HOR,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P4HOR,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P4HOR,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4HOR,4,COP_decoder_resolution_CIF25,9},

  {CP_LAYOUT_1P4VER,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P4VER,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P4VER,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P4VER,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4VER,4,COP_decoder_resolution_CIF25,9},

  {CP_LAYOUT_1P4HOR_UP,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P4HOR_UP,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P4HOR_UP,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P4HOR_UP,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4HOR_UP,4,COP_decoder_resolution_CIF25,9},

  // 6 cells
  {CP_LAYOUT_1P5,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P5,1,COP_decoder_resolution_W4CIF25,4},
  {CP_LAYOUT_1P5,2,COP_decoder_resolution_W4CIF25,5},
  {CP_LAYOUT_1P5,3,COP_decoder_resolution_W4CIF25,8},
  {CP_LAYOUT_1P5,4,COP_decoder_resolution_W4CIF25,9},
  {CP_LAYOUT_1P5,5,COP_decoder_resolution_W4CIF25,12},

  // 8 cells
  {CP_LAYOUT_1P7,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_1P7,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P7,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P7,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P7,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P7,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P7,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P7,7,COP_decoder_resolution_CIF25,12},

  // 9 cells
  {CP_LAYOUT_3X3,0,COP_decoder_resolution_4CIF25,0},
  {CP_LAYOUT_3X3,1,COP_decoder_resolution_4CIF25,1},
  {CP_LAYOUT_3X3,2,COP_decoder_resolution_4CIF25,2},
  {CP_LAYOUT_3X3,3,COP_decoder_resolution_4CIF25,4},
  {CP_LAYOUT_3X3,4,COP_decoder_resolution_4CIF25,5},
  {CP_LAYOUT_3X3,5,COP_decoder_resolution_4CIF25,6},
  {CP_LAYOUT_3X3,6,COP_decoder_resolution_4CIF25,8},
  {CP_LAYOUT_3X3,7,COP_decoder_resolution_4CIF25,9},
  {CP_LAYOUT_3X3,8,COP_decoder_resolution_4CIF25,10},

  {CP_LAYOUT_1P8CENT,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P8CENT,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P8CENT,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P8CENT,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8CENT,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8CENT,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8CENT,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8CENT,7,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8CENT,8,COP_decoder_resolution_CIF25,14},

  {CP_LAYOUT_1P8UP,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P8UP,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P8UP,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P8UP,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8UP,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8UP,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8UP,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8UP,7,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8UP,8,COP_decoder_resolution_CIF25,14},

  {CP_LAYOUT_1P8HOR_UP,0,COP_decoder_resolution_HD108030,0},
  {CP_LAYOUT_1P8HOR_UP,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P8HOR_UP,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P8HOR_UP,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8HOR_UP,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8HOR_UP,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8HOR_UP,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8HOR_UP,7,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8HOR_UP,8,COP_decoder_resolution_CIF25,14},
  // 10+ cells layout
  // 10 cells
  {CP_LAYOUT_2P8,0,COP_decoder_resolution_HD720p25,0},
  {CP_LAYOUT_2P8,1,COP_decoder_resolution_HD720p25,12},
  {CP_LAYOUT_2P8,2,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_2P8,3,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_2P8,4,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_2P8,5,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_2P8,6,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_2P8,7,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_2P8,8,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_2P8,9,COP_decoder_resolution_CIF25,11},

  // 13 cells
  {CP_LAYOUT_1P12,0,COP_decoder_resolution_HD720p25,12},
  {CP_LAYOUT_1P12,1,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_1P12,2,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_1P12,3,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_1P12,4,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_1P12,5,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P12,6,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P12,7,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P12,8,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P12,9,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P12,10,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P12,11,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P12,12,COP_decoder_resolution_CIF25,11},

  // 16 cells
  {CP_LAYOUT_4X4,0,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_4X4,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_4X4,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_4X4,3,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_4X4,4,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_4X4,5,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_4X4,6,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_4X4,7,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_4X4,8,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_4X4,9,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_4X4,10,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_4X4,11,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_4X4,12,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_4X4,13,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_4X4,14,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_4X4,15,COP_decoder_resolution_CIF25,15}
};
//======================================================================================================================================================================//
sCopCellResolutionDef CCopLayoutResolutionTable::g_CopDecodersResolutionTableHD720p50MPMX[TOTAL_NUM_OF_CELLS] = {    // {layout,cell_index,max_decoder_resolution,active_decoder_index}
  // 1 cell layout
  {CP_LAYOUT_1X1,0,COP_decoder_resolution_HD720p50,0},
  // 2 cells layout
  {CP_LAYOUT_1X2,0,COP_decoder_resolution_HD720p50,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_1X2,1,COP_decoder_resolution_HD720p50,4}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_2X1,0,COP_decoder_resolution_HD720p50,0}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER
  {CP_LAYOUT_2X1,1,COP_decoder_resolution_HD720p50,4}, // COP_decoder_resolution_W4CIF changed to COP_decoder_resolution_HD720 to prevent re-cap from 1X2HOR / 1X2VER

  {CP_LAYOUT_1x2HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1x2HOR,1,COP_decoder_resolution_HD720p50,4},

  {CP_LAYOUT_1x2VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1x2VER,1,COP_decoder_resolution_HD720p50,4},
  // 3 cells layout
  {CP_LAYOUT_1P2VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P2VER,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P2VER,2,COP_decoder_resolution_4CIF50,8},

  {CP_LAYOUT_1P2HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P2HOR,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P2HOR,2,COP_decoder_resolution_4CIF50,8},

  {CP_LAYOUT_1P2HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P2HOR_UP,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P2HOR_UP,2,COP_decoder_resolution_4CIF50,8},
  // 4 cells layout
  {CP_LAYOUT_2X2,0,COP_decoder_resolution_4CIF50,0},
  {CP_LAYOUT_2X2,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_2X2,2,COP_decoder_resolution_4CIF50,8},
  {CP_LAYOUT_2X2,3,COP_decoder_resolution_4CIF50,12},

  {CP_LAYOUT_1P3HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P3HOR,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P3HOR,2,COP_decoder_resolution_4CIF50,8},
  {CP_LAYOUT_1P3HOR,3,COP_decoder_resolution_4CIF50,12},

  {CP_LAYOUT_1P3VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P3VER,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P3VER,2,COP_decoder_resolution_4CIF50,8},
  {CP_LAYOUT_1P3VER,3,COP_decoder_resolution_4CIF50,12},

  {CP_LAYOUT_1P3HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P3HOR_UP,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P3HOR_UP,2,COP_decoder_resolution_4CIF50,8},
  {CP_LAYOUT_1P3HOR_UP,3,COP_decoder_resolution_4CIF50,12},

  // 5+ cells layout
  // 5 cells
  {CP_LAYOUT_1P4HOR,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P4HOR,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P4HOR,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P4HOR,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4HOR,4,COP_decoder_resolution_CIF25,9},

  {CP_LAYOUT_1P4VER,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P4VER,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P4VER,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P4VER,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4VER,4,COP_decoder_resolution_CIF25,9},

  {CP_LAYOUT_1P4HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P4HOR_UP,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P4HOR_UP,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P4HOR_UP,3,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P4HOR_UP,4,COP_decoder_resolution_CIF25,9},
  // 6 cells
  {CP_LAYOUT_1P5,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P5,1,COP_decoder_resolution_4CIF50,4},
  {CP_LAYOUT_1P5,2,COP_decoder_resolution_4CIF50,5},
  {CP_LAYOUT_1P5,3,COP_decoder_resolution_4CIF50,8},
  {CP_LAYOUT_1P5,4,COP_decoder_resolution_4CIF50,9},
  {CP_LAYOUT_1P5,5,COP_decoder_resolution_4CIF50,12},

  // 8 cells
  {CP_LAYOUT_1P7,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P7,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P7,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P7,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P7,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P7,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P7,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P7,7,COP_decoder_resolution_CIF25,12},

  // 9 cells
  {CP_LAYOUT_3X3,0,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_3X3,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_3X3,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_3X3,3,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_3X3,4,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_3X3,5,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_3X3,6,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_3X3,7,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_3X3,8,COP_decoder_resolution_CIF25,10},

  {CP_LAYOUT_1P8CENT,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P8CENT,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P8CENT,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P8CENT,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8CENT,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8CENT,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8CENT,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8CENT,7,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8CENT,8,COP_decoder_resolution_CIF25,14},

  {CP_LAYOUT_1P8UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P8UP,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P8UP,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P8UP,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8UP,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8UP,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8UP,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8UP,7,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8UP,8,COP_decoder_resolution_CIF25,14},

  {CP_LAYOUT_1P8HOR_UP,0,COP_decoder_resolution_HD720p50,0},
  {CP_LAYOUT_1P8HOR_UP,1,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P8HOR_UP,2,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P8HOR_UP,3,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P8HOR_UP,4,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P8HOR_UP,5,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P8HOR_UP,6,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P8HOR_UP,7,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P8HOR_UP,8,COP_decoder_resolution_CIF25,14},
  // 10+ cells layout
  // 10 cells
  {CP_LAYOUT_2P8,0,COP_decoder_resolution_4CIF50,0},
  {CP_LAYOUT_2P8,1,COP_decoder_resolution_4CIF50,1},
  {CP_LAYOUT_2P8,2,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_2P8,3,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_2P8,4,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_2P8,5,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_2P8,6,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_2P8,7,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_2P8,8,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_2P8,9,COP_decoder_resolution_CIF25,14},
  // 13 cells
  {CP_LAYOUT_1P12,0,COP_decoder_resolution_4CIF50,0},
  {CP_LAYOUT_1P12,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_1P12,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_1P12,3,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_1P12,4,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_1P12,5,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_1P12,6,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_1P12,7,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_1P12,8,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_1P12,9,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_1P12,10,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_1P12,11,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_1P12,12,COP_decoder_resolution_CIF25,14},

  // 16 cells
  {CP_LAYOUT_4X4,0,COP_decoder_resolution_CIF25,0},
  {CP_LAYOUT_4X4,1,COP_decoder_resolution_CIF25,1},
  {CP_LAYOUT_4X4,2,COP_decoder_resolution_CIF25,2},
  {CP_LAYOUT_4X4,3,COP_decoder_resolution_CIF25,3},
  {CP_LAYOUT_4X4,4,COP_decoder_resolution_CIF25,4},
  {CP_LAYOUT_4X4,5,COP_decoder_resolution_CIF25,5},
  {CP_LAYOUT_4X4,6,COP_decoder_resolution_CIF25,6},
  {CP_LAYOUT_4X4,7,COP_decoder_resolution_CIF25,7},
  {CP_LAYOUT_4X4,8,COP_decoder_resolution_CIF25,8},
  {CP_LAYOUT_4X4,9,COP_decoder_resolution_CIF25,9},
  {CP_LAYOUT_4X4,10,COP_decoder_resolution_CIF25,10},
  {CP_LAYOUT_4X4,11,COP_decoder_resolution_CIF25,11},
  {CP_LAYOUT_4X4,12,COP_decoder_resolution_CIF25,12},
  {CP_LAYOUT_4X4,13,COP_decoder_resolution_CIF25,13},
  {CP_LAYOUT_4X4,14,COP_decoder_resolution_CIF25,14},
  {CP_LAYOUT_4X4,15,COP_decoder_resolution_CIF25,15}
};

//======================================================================================================================================================================//
// constructors
//======================================================================================================================================================================//
CCopLayoutResolutionTable::CCopLayoutResolutionTable()
{
}
//======================================================================================================================================================================//
CCopLayoutResolutionTable::~CCopLayoutResolutionTable()
{
}
//======================================================================================================================================================================//
// Public API
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDef(ECopEncoderMaxResolution encoder_max_resolution, LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found
  decoder_resolution = COP_decoder_resolution_CIF25;
  decoder_connection_id_index = 15;
  eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
  switch(systemCardsBasedMode)
  {
	  case(eSystemCardsMode_mpm_plus):
	  {
		  ret_value=GetCellResolutionDefMPMPlus(encoder_max_resolution,layout_type,cell_index,decoder_resolution,decoder_connection_id_index);
		  break;
	  }
	  case(eSystemCardsMode_breeze):
	  {
		  ret_value=GetCellResolutionDefMPMX(encoder_max_resolution,layout_type,cell_index,decoder_resolution,decoder_connection_id_index);
		  break;
	  }
	  default:
		  PASSERT(101);
		  PTRACE2INT(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDef wrong system mode: ",systemCardsBasedMode);
		  break;

  }
  return ret_value;
}//=========================================MPM+=============================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDefMPMPlus(ECopEncoderMaxResolution encoder_max_resolution, LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found
  decoder_resolution = COP_decoder_resolution_CIF25;
  decoder_connection_id_index = 15;

  if(encoder_max_resolution==COP_encoder_max_resolution_HD1080p25){
    ret_value = GetCellResolutionDefHD1080p25MPMPlus(layout_type,cell_index,decoder_resolution,decoder_connection_id_index);
  }else if(encoder_max_resolution==COP_encoder_max_resolution_HD720p50){
    ret_value = GetCellResolutionDefHD720p50MPMPlus(layout_type,cell_index,decoder_resolution,decoder_connection_id_index);
  }else{
    CSmallString sstr;
    sstr << " encoder_max_resolution = " << (WORD)encoder_max_resolution;
    PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDefMPMPlus faild to find layout/cell: ", sstr.GetString());
  }

  return ret_value;
}

//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDefHD1080p25MPMPlus(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found

  for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++){
    if(layout_type==g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].layout_type && cell_index==g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].cell_index){
      decoder_resolution = g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].decoder_resolution;
      decoder_connection_id_index = g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].decoder_connection_id_index;
      ret_value = line_index;
      break;
    }
  }
  if(ret_value == (WORD)-1){
    CSmallString sstr;
    sstr << " layout_type = " << (WORD)layout_type << ", cell_index = " << cell_index;
    PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDefHD1080p25MPMPlus faild to find layout/cell: ", sstr.GetString());
  }
  return ret_value;
}
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDefHD720p50MPMPlus(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found

  for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++){
    if(layout_type==g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].layout_type && cell_index==g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].cell_index){
      decoder_resolution = g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].decoder_resolution;
      decoder_connection_id_index = g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].decoder_connection_id_index;
      ret_value = line_index;
      break;
    }
  }
  if(ret_value == (WORD)-1){
    CSmallString sstr;
    sstr << " layout_type = " << (WORD)layout_type << ", cell_index = " << cell_index;
    PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDefHD720p50MPMPlus faild to find layout/cell: ", sstr.GetString());
  }
  return ret_value;
}
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDefMPMX(ECopEncoderMaxResolution encoder_max_resolution, LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found
  decoder_resolution = COP_decoder_resolution_CIF25;
  decoder_connection_id_index = 15;

  if(encoder_max_resolution==COP_encoder_max_resolution_HD1080p25){
    ret_value = GetCellResolutionDefHD1080p25MPMX(layout_type,cell_index,decoder_resolution,decoder_connection_id_index);
  }else if(encoder_max_resolution==COP_encoder_max_resolution_HD720p50){
    ret_value = GetCellResolutionDefHD720p50MPMX(layout_type,cell_index,decoder_resolution,decoder_connection_id_index);
  }else{
    CSmallString sstr;
    sstr << " encoder_max_resolution = " << (WORD)encoder_max_resolution;
    PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDefMPMX faild to find layout/cell: ", sstr.GetString());
  }

  return ret_value;
}
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDefHD1080p25MPMX(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found
  for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++){
    if(layout_type==g_CopDecodersResolutionTableHD1080p25MPMX[line_index].layout_type && cell_index==g_CopDecodersResolutionTableHD1080p25MPMX[line_index].cell_index){
      decoder_resolution = g_CopDecodersResolutionTableHD1080p25MPMX[line_index].decoder_resolution;
      decoder_connection_id_index = g_CopDecodersResolutionTableHD1080p25MPMX[line_index].decoder_connection_id_index;
      ret_value = line_index;
      break;
    }
  }
  if(ret_value == (WORD)-1){
    CSmallString sstr;
    sstr << " layout_type = " << (WORD)layout_type << ", cell_index = " << cell_index;
    PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDefHD1080p25MPMX failed to find layout/cell: ", sstr.GetString());
  }
  return ret_value;
}
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetCellResolutionDefHD720p50MPMX(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index)
{
  WORD ret_value = (WORD)-1; // not found

  for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++){
    if(layout_type==g_CopDecodersResolutionTableHD720p50MPMX[line_index].layout_type && cell_index==g_CopDecodersResolutionTableHD720p50MPMX[line_index].cell_index){
      decoder_resolution = g_CopDecodersResolutionTableHD720p50MPMX[line_index].decoder_resolution;
      decoder_connection_id_index = g_CopDecodersResolutionTableHD720p50MPMX[line_index].decoder_connection_id_index;
      ret_value = line_index;
      break;
    }
  }
  if(ret_value == (WORD)-1){
    CSmallString sstr;
    sstr << " layout_type = " << (WORD)layout_type << ", cell_index = " << cell_index;
    PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetCellResolutionDefHD720p50MPMX failed to find layout/cell: ", sstr.GetString());
  }
  return ret_value;
}

//======================================================================================================================================================================//

WORD CCopLayoutResolutionTable::IsLayoutSupportedInCop(LayoutType layout_type)
{
	WORD ret_val = FALSE;
	switch(layout_type){
	case CP_LAYOUT_1X1:
	case CP_LAYOUT_1X2:
	case CP_LAYOUT_2X1:
	case CP_LAYOUT_2X2:
	case CP_LAYOUT_3X3:
	case CP_LAYOUT_4X4:
	case CP_LAYOUT_1P5:
	case CP_LAYOUT_1P7:
	case CP_LAYOUT_1x2HOR:
	case CP_LAYOUT_1x2VER:
	case CP_LAYOUT_1P2VER:
	case CP_LAYOUT_1P2HOR:
	case CP_LAYOUT_1P3HOR_UP:
	case CP_LAYOUT_1P3VER:
	case CP_LAYOUT_1P4HOR:
	case CP_LAYOUT_1P4VER:
	case CP_LAYOUT_1P8CENT:
	case CP_LAYOUT_1P8UP:
	case CP_LAYOUT_1P8HOR_UP:
	case CP_LAYOUT_1P2HOR_UP:
	case CP_LAYOUT_1P3HOR:
	case CP_LAYOUT_1P4HOR_UP:
	case CP_LAYOUT_1P12:
	case CP_LAYOUT_2P8:{
		ret_val = TRUE;
		break;
    }
    // RPX layouts (flex) and 1X1_QCIF?not supported in COP
	/*CP_LAYOUT_1X2_FLEX,
		CP_LAYOUT_1P2HOR_RIGHT_FLEX,
		CP_LAYOUT_1P2HOR_LEFT_FLEX,

		CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX,
		CP_LAYOUT_1P2HOR_UP_LEFT_FLEX,

		CP_LAYOUT_2X2_UP_RIGHT_FLEX,
		CP_LAYOUT_2X2_UP_LEFT_FLEX,

		CP_LAYOUT_2X2_DOWN_RIGHT_FLEX,
		CP_LAYOUT_2X2_DOWN_LEFT_FLEX,

		CP_LAYOUT_2X2_RIGHT_FLEX,
		CP_LAYOUT_2X2_LEFT_FLEX,
	*/
    // 	case E_VIDEO_LAYOUT_1X1_QCIF:

  default:
    break;
  }
  return ret_val;
}
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetDecoderIndexInLayout(eVideoConfType video_conf_type, LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	WORD ret_value = (WORD)-1; // not found
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	switch(systemCardsBasedMode)
	{
	  case(eSystemCardsMode_mpm_plus):
	  {
		  ret_value=GetDecoderIndexInLayoutMPMPlus(video_conf_type,layout_type,decoder_connection_id_index,decoder_resolution);
		  break;
	  }
	  case(eSystemCardsMode_breeze):
	  {
		  ret_value=GetDecoderIndexInLayoutMPMX(video_conf_type,layout_type,decoder_connection_id_index,decoder_resolution);
		  break;
	  }
	  default:
		  PASSERT(101);
		  PTRACE2INT(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayout wrong system mode: ",systemCardsBasedMode);
			  break;

	}
	return ret_value;
}
//======================================================================================================================================================================//

WORD CCopLayoutResolutionTable::GetDecoderIndexInLayoutMPMPlus(eVideoConfType video_conf_type, LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	if (video_conf_type == eVideoConfTypeCopHD108025fps)
		return GetDecoderIndexInLayoutHD1080p25MPMPlus(layout_type,decoder_connection_id_index,decoder_resolution);
	else if (video_conf_type == eVideoConfTypeCopHD72050fps)
		return GetDecoderIndexInLayoutHD720p50MPMPlus(layout_type,decoder_connection_id_index,decoder_resolution);
	else
	{
		PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayoutMPMPlus illegal video conf type (not cop!) - ",VideoConfTypeAsString[video_conf_type]);
		return (WORD)-1;
	}
}
//======================================================================================================================================================================//
WORD  CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD1080p25MPMPlus(LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	WORD ret_value = (WORD)-1; // not found

	for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++)
	{
	    if(layout_type==g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].layout_type)
	    {
	    	while (layout_type==g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].layout_type)
	    	{
	    		if (decoder_connection_id_index == g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].decoder_connection_id_index)
	    		{
	    			ret_value = g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].cell_index;
	    			decoder_resolution = g_CopDecodersResolutionTableHD1080p25MPMPlus[line_index].decoder_resolution;
	    			break;
	    		}
	    		line_index++;
	    	}
	    	break;
	    }
	}

	if(ret_value == (WORD)-1)
	{
	    CSmallString sstr;
	    sstr << " layout_type = " << (WORD)layout_type << ", decoder_connection_id_index = " << decoder_connection_id_index;
	   // PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD1080p25MPMPlus failed to find layout/cell: ", sstr.GetString());
	 }

	return ret_value;
}
//======================================================================================================================================================================//
WORD  CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD720p50MPMPlus(LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	WORD ret_value = (WORD)-1; // not found

		for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++)
		{
		    if(layout_type==g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].layout_type)
		    {
		    	while (layout_type==g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].layout_type)
		    	{
		    		if (decoder_connection_id_index == g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].decoder_connection_id_index)
		    		{
		    			ret_value = g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].cell_index;
		    			decoder_resolution = g_CopDecodersResolutionTableHD720p50MPMPlus[line_index].decoder_resolution;
		    			break;
		    		}
		    		line_index++;
		    	}
		    	break;
		    }
		}

		if(ret_value == (WORD)-1)
		{
		    CSmallString sstr;
		    sstr << " layout_type = " << (WORD)layout_type << ", decoder_connection_id_index = " << decoder_connection_id_index;
		  //  PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD720p50MPMPlus failed to find layout/cell: ", sstr.GetString());
		 }

	return ret_value;
}
//======================================================================================================================================================================//
WORD CCopLayoutResolutionTable::GetDecoderIndexInLayoutMPMX(eVideoConfType video_conf_type, LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	if (video_conf_type == eVideoConfTypeCopHD108025fps)
		return GetDecoderIndexInLayoutHD1080p25MPMX(layout_type,decoder_connection_id_index,decoder_resolution);
	else if (video_conf_type == eVideoConfTypeCopHD72050fps)
		return GetDecoderIndexInLayoutHD720p50MPMX(layout_type,decoder_connection_id_index,decoder_resolution);
	else
	{
		PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayoutMPMX illegal video conf type (not cop!) - ",VideoConfTypeAsString[video_conf_type]);
		return (WORD)-1;
	}
}
//======================================================================================================================================================================//
WORD  CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD1080p25MPMX(LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	WORD ret_value = (WORD)-1; // not found

	for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++)
	{
	    if(layout_type==g_CopDecodersResolutionTableHD1080p25MPMX[line_index].layout_type)
	    {
	    	while (layout_type==g_CopDecodersResolutionTableHD1080p25MPMX[line_index].layout_type)
	    	{
	    		if (decoder_connection_id_index == g_CopDecodersResolutionTableHD1080p25MPMX[line_index].decoder_connection_id_index)
	    		{
	    			ret_value = g_CopDecodersResolutionTableHD1080p25MPMX[line_index].cell_index;
	    			decoder_resolution = g_CopDecodersResolutionTableHD1080p25MPMX[line_index].decoder_resolution;
	    			break;
	    		}
	    		line_index++;
	    	}
	    	break;
	    }
	}

	if(ret_value == (WORD)-1)
	{
	    CSmallString sstr;
	    sstr << " layout_type = " << (WORD)layout_type << ", decoder_connection_id_index = " << decoder_connection_id_index;
	   // PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD1080p25MPMX failed to find layout/cell: ", sstr.GetString());
	 }

	return ret_value;
}
//======================================================================================================================================================================//
WORD  CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD720p50MPMX(LayoutType layout_type,WORD decoder_connection_id_index, ECopDecoderResolution& decoder_resolution)
{
	WORD ret_value = (WORD)-1; // not found

		for(WORD line_index=0;line_index<TOTAL_NUM_OF_CELLS;line_index++)
		{
		    if(layout_type==g_CopDecodersResolutionTableHD720p50MPMX[line_index].layout_type)
		    {
		    	while (layout_type==g_CopDecodersResolutionTableHD720p50MPMX[line_index].layout_type)
		    	{
		    		if (decoder_connection_id_index == g_CopDecodersResolutionTableHD720p50MPMX[line_index].decoder_connection_id_index)
		    		{
		    			ret_value = g_CopDecodersResolutionTableHD720p50MPMX[line_index].cell_index;
		    			decoder_resolution = g_CopDecodersResolutionTableHD720p50MPMX[line_index].decoder_resolution;
		    			break;
		    		}
		    		line_index++;
		    	}
		    	break;
		    }
		}

		if(ret_value == (WORD)-1)
		{
		    CSmallString sstr;
		    sstr << " layout_type = " << (WORD)layout_type << ", decoder_connection_id_index = " << decoder_connection_id_index;
		  //  PTRACE2(eLevelInfoNormal,"CCopLayoutResolutionTable::GetDecoderIndexInLayoutHD720p50MPMX failed to find layout/cell: ", sstr.GetString());
		 }

	return ret_value;
}
//======================================================================================================================================================================//

//======================================================================================================================================================================//
const char* CCopLayoutResolutionTable::GetCopDecoderResolutionStr(ECopDecoderResolution copDecoderResolution)
{
	const char *name = ((DWORD)copDecoderResolution <= NUM_OF_DECODER_MODES
                       ?
                    	ECopDecoderResolutionString[copDecoderResolution] : "Invalid");
   return name;
}
