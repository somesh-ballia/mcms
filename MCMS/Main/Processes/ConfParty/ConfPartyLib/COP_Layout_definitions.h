
#ifndef _COP_LAYOUT_DEFINITIONS_H_
#define _COP_LAYOUT_DEFINITIONS_H_

#include  "PObject.h"
#include "VideoApiDefinitions.h"
#include "VideoDefines.h"
//#include "ProcessBase.h"
//#include "Trace.h"
//#include "ConfPartyDefines.h"

// Total number of cells in all layouts in table
// (1*1)+(4*2)+(3*3)+(4*4)+(3*5)+(1*6)+(1*8)+(4*9)+(1*10)+(1*13)+(1*16)

//bla bla bla

#define TOTAL_NUM_OF_CELLS 138

typedef enum
{
  COP_encoder_max_resolution_HD1080p25,
  COP_encoder_max_resolution_HD720p50
}ECopEncoderMaxResolution;

typedef enum
{
  COP_decoder_resolution_HD108030,
  COP_decoder_resolution_HD720p50,
  COP_decoder_resolution_HD720p25,
  COP_decoder_resolution_W4CIF25,
  COP_decoder_resolution_4CIF50,
  COP_decoder_resolution_4CIF25,
  COP_decoder_resolution_CIF25,
  COP_decoder_resolution_Last
}ECopDecoderResolution;

typedef enum
{
    eCop_DecoderParams = 0,
    eCop_EncoderIndex
}ECopChangeModeType;


#define NUM_OF_DECODER_MODES (DWORD)COP_decoder_resolution_Last

static const char* ECopDecoderResolutionString[] =
{
	"COP_decoder_resolution_HD108030",
	"COP_decoder_resolution_HD720p50",
	"COP_decoder_resolution_HD720p25",
	"COP_decoder_resolution_W4CIF25",
	"COP_decoder_resolution_4CIF50",
	"COP_decoder_resolution_4CIF25",
	"COP_decoder_resolution_CIF25",
	"COP_decoder_resolution_Last"
};


// {layout,cell_index,max_decoder_resolution,active_decoder_index}
typedef struct
{
  LayoutType layout_type;
  WORD cell_index;
  ECopDecoderResolution decoder_resolution;
  WORD decoder_connection_id_index;
}sCopCellResolutionDef;


class CCopLayoutResolutionTable : public CPObject
{
	CLASS_TYPE_1(CCopLayoutResolutionTable, CPObject)
public:
  // costructors
  CCopLayoutResolutionTable();
  virtual ~CCopLayoutResolutionTable();
  // CPObject
  virtual const char* NameOf() const { return "CCopLayoutResolutionTable"; };

  // API
  WORD GetCellResolutionDef(ECopEncoderMaxResolution encoder_max_resolution, LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD IsLayoutSupportedInCop(LayoutType layout_type);
  WORD GetDecoderIndexInLayout(eVideoConfType video_conf_type, LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);

  //MPM+
  WORD GetCellResolutionDefMPMPlus(ECopEncoderMaxResolution encoder_max_resolution, LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD GetCellResolutionDefHD1080p25MPMPlus(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD GetCellResolutionDefHD720p50MPMPlus(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD GetDecoderIndexInLayoutMPMPlus(eVideoConfType video_conf_type, LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);
  WORD GetDecoderIndexInLayoutHD1080p25MPMPlus(LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);
  WORD GetDecoderIndexInLayoutHD720p50MPMPlus(LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);

  //MPMX
  WORD GetCellResolutionDefMPMX(ECopEncoderMaxResolution encoder_max_resolution, LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD GetCellResolutionDefHD1080p25MPMX(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD GetCellResolutionDefHD720p50MPMX(LayoutType layout_type, WORD cell_index, ECopDecoderResolution& decoder_resolution, WORD& decoder_connection_id_index);
  WORD GetDecoderIndexInLayoutMPMX(eVideoConfType video_conf_type, LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);
  WORD GetDecoderIndexInLayoutHD1080p25MPMX(LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);
  WORD GetDecoderIndexInLayoutHD720p50MPMX(LayoutType layout_type,WORD decoder_connection_id_index,ECopDecoderResolution& decoder_resolution);



  const char* GetCopDecoderResolutionStr(ECopDecoderResolution copDecoderResolution);
private:

  static sCopCellResolutionDef g_CopDecodersResolutionTableHD1080p25MPMPlus[TOTAL_NUM_OF_CELLS];
  static sCopCellResolutionDef g_CopDecodersResolutionTableHD720p50MPMPlus[TOTAL_NUM_OF_CELLS];

  //MPMX
  static sCopCellResolutionDef g_CopDecodersResolutionTableHD1080p25MPMX[TOTAL_NUM_OF_CELLS];
  static sCopCellResolutionDef g_CopDecodersResolutionTableHD720p50MPMX[TOTAL_NUM_OF_CELLS];

};








#endif
