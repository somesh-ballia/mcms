#include "LibsCommonHelperFuncs.h"
#include "Trace.h"
#include "TraceStream.h"




/////////////////////////////////////////////////////////////////////////////
WORD CLibsCommonHelperFuncs::GetNumbSubImg(const char layoutType)
{
  WORD result = 0;
  switch (layoutType)
  {
    case E_VIDEO_LAYOUT_DUMMY:
    {
      result = 0;
      break;
    }

    case E_VIDEO_LAYOUT_1X1:
    {
      result = 1;
      break;
    }

    case E_VIDEO_LAYOUT_1X2VER:
    case E_VIDEO_LAYOUT_1X2HOR:
    case E_VIDEO_LAYOUT_1X2:
    case E_VIDEO_LAYOUT_2X1:
    case E_VIDEO_LAYOUT_FLEX_1X2:
    {
      result = 2;
      break;
    }

    case E_VIDEO_LAYOUT_1P3HOR:
    case E_VIDEO_LAYOUT_1P3HOR_UP:
    case E_VIDEO_LAYOUT_1P3VER:
    case E_VIDEO_LAYOUT_2X2:
    case E_VIDEO_LAYOUT_FLEX_2X2_UP_RIGHT:
    case E_VIDEO_LAYOUT_FLEX_2X2_UP_LEFT:
    case E_VIDEO_LAYOUT_FLEX_2X2_DOWN_RIGHT:
    case E_VIDEO_LAYOUT_FLEX_2X2_DOWN_LEFT:
    case E_VIDEO_LAYOUT_FLEX_2X2_RIGHT:
    case E_VIDEO_LAYOUT_FLEX_2X2_LEFT:
    {
      result = 4;
      break;
    }

    case E_VIDEO_LAYOUT_1P8CENT:
    case E_VIDEO_LAYOUT_1P8UP:
    case E_VIDEO_LAYOUT_1P8HOR_UP:
    case E_VIDEO_LAYOUT_3X3:
    case E_VIDEO_LAYOUT_1TOP_LEFT_P8:
    {
      result = 9;
      break;
    }

    case E_VIDEO_LAYOUT_1P5:
    {
      result = 6;
      break;
    }

    case E_VIDEO_LAYOUT_1P7:
    {
      result = 8;
      break;
    }

    case E_VIDEO_LAYOUT_1P2HOR:
    case E_VIDEO_LAYOUT_1P2HOR_UP:
    case E_VIDEO_LAYOUT_1P2VER:
    case E_VIDEO_LAYOUT_FLEX_1P2HOR_RIGHT:
    case E_VIDEO_LAYOUT_FLEX_1P2HOR_LEFT:
    case E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_RIGHT:
    case E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_LEFT:
    {
      result = 3;
      break;
    }

    case E_VIDEO_LAYOUT_1P4HOR:
    case E_VIDEO_LAYOUT_1P4HOR_UP:
    case E_VIDEO_LAYOUT_1P4VER:
    {
      result = 5;
      break;
    }

    case E_VIDEO_LAYOUT_4X4:
    {
      result = 16;
      break;
    }

    case E_VIDEO_LAYOUT_2P8:
    case E_VIDEO_LAYOUT_2TOP_P8:
    {
      result = 10;
      break;
    }

    case E_VIDEO_LAYOUT_1P12:
    {
      result = 13;
      break;
    }
	
	case E_VIDEO_LAYOUT_OVERLAY_1P1:
	{
		result = 2;
		break;
	}

	case E_VIDEO_LAYOUT_OVERLAY_1P2:
	case E_VIDEO_LAYOUT_OVERLAY_ITP_1P2:
	{
		result = 3;
		break;
	}

	case E_VIDEO_LAYOUT_OVERLAY_1P3:
	case E_VIDEO_LAYOUT_OVERLAY_ITP_1P3:
	{
		result = 4;
		break;
	}

	case E_VIDEO_LAYOUT_OVERLAY_ITP_1P4:
	{
		result = 5;
		break;
	}
	
    default:
    {
      if (layoutType != 0)
      {
        DBGFPASSERT(layoutType);
      }
      else
      {
        DBGFPASSERT(1001);
      }
    }
  } // switch

  return result;
}

