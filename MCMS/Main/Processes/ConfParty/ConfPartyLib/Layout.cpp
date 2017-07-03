//+========================================================================+
//                            Layout.CPP                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       Layout.CPP                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//+========================================================================+

#include "Layout.h"
#include "VideoLayout.h"
#include "CommConfDB.h"
#include "Conf.h"
#include "Party.h"
#include "SysConfigKeys.h"
#include "VideoBridge.h"

class TaskApp;

WORD GetNumbSubImg(const LayoutType layoutType)
{
  WORD result = 0;
  switch (layoutType)
  {
    case CP_NO_LAYOUT:
    {
      result = 0;
      break;
    }

    case CP_LAYOUT_1X1:
    {
      result = 1;
      break;
    }

    case CP_LAYOUT_1x2VER:
    case CP_LAYOUT_1x2HOR:
    case CP_LAYOUT_1X2:
    case CP_LAYOUT_2X1:
    case CP_LAYOUT_1X2_FLEX:
    {
      result = 2;
      break;
    }

    case CP_LAYOUT_1P3HOR:
    case CP_LAYOUT_1P3HOR_UP:
    case CP_LAYOUT_1P3VER:
    case CP_LAYOUT_2X2:
    case CP_LAYOUT_2X2_UP_RIGHT_FLEX:
    case CP_LAYOUT_2X2_UP_LEFT_FLEX:
    case CP_LAYOUT_2X2_DOWN_RIGHT_FLEX:
    case CP_LAYOUT_2X2_DOWN_LEFT_FLEX:
    case CP_LAYOUT_2X2_RIGHT_FLEX:
    case CP_LAYOUT_2X2_LEFT_FLEX:
    {
      result = 4;
      break;
    }

    case CP_LAYOUT_1P8CENT:
    case CP_LAYOUT_1P8UP:
    case CP_LAYOUT_1P8HOR_UP:
    case CP_LAYOUT_3X3:
    case CP_LAYOUT_1TOP_LEFT_P8:
    {
      result = 9;
      break;
    }

    case CP_LAYOUT_1P5:
    {
      result = 6;
      break;
    }

    case CP_LAYOUT_1P7:
    {
      result = 8;
      break;
    }

    case CP_LAYOUT_1P2HOR:
    case CP_LAYOUT_1P2HOR_UP:
    case CP_LAYOUT_1P2VER:
    case CP_LAYOUT_1P2HOR_RIGHT_FLEX:
    case CP_LAYOUT_1P2HOR_LEFT_FLEX:
    case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX:
    case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX:
    {
      result = 3;
      break;
    }

    case CP_LAYOUT_1P4HOR:
    case CP_LAYOUT_1P4HOR_UP:
    case CP_LAYOUT_1P4VER:
    {
      result = 5;
      break;
    }

    case CP_LAYOUT_4X4:
    {
      result = 16;
      break;
    }

    case CP_LAYOUT_2P8:
    case CP_LAYOUT_2TOP_P8:
    {
      result = 10;
      break;
    }

    case CP_LAYOUT_1P12:
    {
      result = 13;
      break;
    }

	case CP_LAYOUT_OVERLAY_1P1:
	{
		result = 2;
		break;
	}

	case CP_LAYOUT_OVERLAY_1P2:
	case CP_LAYOUT_OVERLAY_ITP_1P2:
	{
		result = 3;
		break;
	}

	case CP_LAYOUT_OVERLAY_1P3:
	case CP_LAYOUT_OVERLAY_ITP_1P3:
	{
		result = 4;
		break;
	}

	case CP_LAYOUT_OVERLAY_ITP_1P4:
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


////////////////////////////////////////////////////////////////////////////
//                            CLayout
////////////////////////////////////////////////////////////////////////////
CLayout::CLayout(LayoutType layoutType, const char* pConfName) : m_layoutType(layoutType)
{
  if (NULL != pConfName)
  {
    strncpy(m_confName, pConfName, H243_NAME_LEN-1);
    m_confName[H243_NAME_LEN -1] = '\0';
  }

  m_numberOfSubImages = GetNumbSubImg(layoutType);
  if (m_numberOfSubImages == 0)
    PASSERT(layoutType+1);

  for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
    m_layout[i] = NIL(CVidSubImage);

  for (int j = 0; j < m_numberOfSubImages; j++)
  {
    m_layout[j] = new CVidSubImage();
    m_layout[j]->SetLayoutTypeAndSizes(layoutType, j);
  }

  initIndications();

  m_isActive              = NO;

  SetStartsForImages();
}

////////////////////////////////////////////////////////////////////////////
CLayout::CLayout(const CLayout& oldLayout, const LayoutType layoutType) : m_layoutType(layoutType)
{
  strncpy(m_confName, oldLayout.m_confName, H243_NAME_LEN-1);
  m_confName[H243_NAME_LEN -1] = '\0';

  m_numberOfSubImages = GetNumbSubImg(layoutType);
  for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
    m_layout[i] = NIL(CVidSubImage);

  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    m_layout[i] = new CVidSubImage();
    m_layout[i]->SetLayoutTypeAndSizes(m_layoutType, i);
  }

  initIndications();

  m_isActive              = NO;

  SetStartsForImages();
}

////////////////////////////////////////////////////////////////////////////
CLayout::~CLayout(void)
{
  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    if (CPObject::IsValidPObjectPtr(m_layout[i]))
      POBJDELETE(m_layout[i]);
  }
}

////////////////////////////////////////////////////////////////////////////
CLayout::CLayout(const CLayout& rhs)
  : CPObject(rhs), m_layoutType(rhs.GetLayoutType())
{
  strncpy(m_confName, rhs.m_confName, H243_NAME_LEN-1);
  m_confName[H243_NAME_LEN -1] = '\0';

  m_numberOfSubImages     = rhs.GetNumberOfSubImages();
  m_isActive              = rhs.isActive();

  for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
    m_layout[i] = NIL(CVidSubImage);

  if(m_numberOfSubImages <= MAX_SUB_IMAGES_IN_LAYOUT)
  {
    for (int i = 0; i < m_numberOfSubImages; i++)
      m_layout[i] = new CVidSubImage(*(rhs[i]));
  }
  else
    PASSERTMSG(1, "m_numberOfSubImages exceed MAX_SUB_IMAGES_IN_LAYOUT");
}

////////////////////////////////////////////////////////////////////////////
CVidSubImage* CLayout::operator[](const WORD size)
{
  if (size >= MAX_SUB_IMAGES_IN_LAYOUT || size+1 > m_numberOfSubImages)
    return NIL(CVidSubImage);
  else
    return m_layout[size];
}

////////////////////////////////////////////////////////////////////////////
const CVidSubImage* CLayout::operator[](const WORD size) const
{
  PASSERTSTREAM_AND_RETURN_VALUE(size >= MAX_SUB_IMAGES_IN_LAYOUT,
    "size has invalid value " << size, NIL(CVidSubImage));

  if (size+1 > m_numberOfSubImages)
    return NIL(CVidSubImage);
  else
    return m_layout[size];
}

////////////////////////////////////////////////////////////////////////////
void CLayout::SetStartsForImages()
{
  switch (m_layoutType)
  {
    case CP_LAYOUT_1X1:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 1);
      m_layout[0]->SetStart(0, 0);
      break;
    }

    case CP_LAYOUT_1X2:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 2);
      m_layout[0]->SetStart(0, QCIF_Y_SIZE/2);
      m_layout[1]->SetStart(QCIF_X_SIZE, QCIF_Y_SIZE/2);
      break;
    }

    case CP_LAYOUT_2X1:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 2);
      m_layout[0]->SetStart(QCIF_X_SIZE/2, 0);
      m_layout[1]->SetStart(QCIF_X_SIZE/2, QCIF_Y_SIZE);
      break;
    }

    case CP_LAYOUT_2X2:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(QCIF_X_SIZE, 0);
      m_layout[2]->SetStart(0, QCIF_Y_SIZE);
      m_layout[3]->SetStart(QCIF_X_SIZE, QCIF_Y_SIZE);
      break;
    }

    case CP_LAYOUT_3X3:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 9);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(CIF_X_SIZE/3, 0);
      m_layout[2]->SetStart((CIF_X_SIZE/3)*2, 0);
      m_layout[3]->SetStart(0, CIF_Y_SIZE/3);
      m_layout[4]->SetStart(CIF_X_SIZE/3, CIF_Y_SIZE/3);
      m_layout[5]->SetStart((CIF_X_SIZE/3)*2, CIF_Y_SIZE/3);
      m_layout[6]->SetStart(0, (CIF_Y_SIZE/3)*2);
      m_layout[7]->SetStart(CIF_X_SIZE/3, (CIF_Y_SIZE/3)*2);
      m_layout[8]->SetStart((CIF_X_SIZE/3)*2, (CIF_Y_SIZE/3)*2);
      break;
    }

    case CP_LAYOUT_1P5:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 6);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart((CIF_X_SIZE/3)*2, 0);
      m_layout[2]->SetStart((CIF_X_SIZE/3)*2, CIF_Y_SIZE/3);
      m_layout[3]->SetStart(0, (CIF_Y_SIZE/3)*2);
      m_layout[4]->SetStart(CIF_X_SIZE/3, (CIF_Y_SIZE/3)*2);
      m_layout[5]->SetStart((CIF_X_SIZE/3)*2, (CIF_Y_SIZE/3)*2);
      break;
    }

    case CP_LAYOUT_1P7:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 8);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart((CIF_X_SIZE/4)*3, 0);
      m_layout[2]->SetStart((CIF_X_SIZE/4)*3, CIF_Y_SIZE/4);
      m_layout[3]->SetStart((CIF_X_SIZE/4)*3, CIF_Y_SIZE*2/4);
      m_layout[4]->SetStart(CIF_X_SIZE*3/4, (CIF_Y_SIZE/4)*3);
      m_layout[5]->SetStart(CIF_X_SIZE*2/4, (CIF_Y_SIZE/4)*3);
      m_layout[6]->SetStart(CIF_X_SIZE/4, (CIF_Y_SIZE/4)*3);
      m_layout[7]->SetStart(0, (CIF_Y_SIZE/4)*3);
      break;
    }

    case CP_LAYOUT_1x2VER:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 2);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart((CIF_X_SIZE/2), 0);
      break;
    }

    case CP_LAYOUT_1x2HOR:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 2);
      m_layout[0]->SetStart(0, GetNegativeCoord(LAY_1P2HOR_UP_Y_OFFSET));
      m_layout[1]->SetStart(0, LAY_1P2HOR_CROP_Y_OFFSET);
      break;
    }

    case CP_LAYOUT_1P2VER:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(CIF_X_SIZE/2, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE/2);
      break;
    }

    case CP_LAYOUT_1P2HOR_UP:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, GetNegativeCoord(LAY_1P2HOR_UP_Y_OFFSET));
      m_layout[1]->SetStart(0, CIF_Y_SIZE/2);
      m_layout[2]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE/2);
      break;
    }

    case CP_LAYOUT_1P2HOR:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, LAY_1P2HOR_CROP_Y_OFFSET); // the reason is cropping
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/2, 0);
      break;
    }

    case CP_LAYOUT_1P3HOR_UP:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, GetNegativeCoord(LAY_1P3HOR_UP_Y_OFFSET));
      m_layout[1]->SetStart(0, CIF_Y_SIZE*2/3);
      m_layout[2]->SetStart(CIF_X_SIZE/3, CIF_Y_SIZE*2/3);
      m_layout[3]->SetStart(CIF_X_SIZE*2/3, CIF_Y_SIZE*2/3);
      break;
    }

    case CP_LAYOUT_1P3HOR:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, LAY_1P3HOR_CROP_Y_OFFSET); // the reason is cropping
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/3, 0);
      m_layout[3]->SetStart(CIF_X_SIZE*2/3, 0);
      break;
    }

    case CP_LAYOUT_1P3VER:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(2*CIF_X_SIZE/3, 0);
      m_layout[2]->SetStart(2*CIF_X_SIZE/3, CIF_Y_SIZE/3);
      m_layout[3]->SetStart(2*CIF_X_SIZE/3, CIF_Y_SIZE*2/3);
      break;
    }

    case CP_LAYOUT_1P4HOR:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 5);
      m_layout[0]->SetStart(0, LAY_1P4HOR_CROP_Y_OFFSET); // the reason is cropping
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[3]->SetStart(CIF_X_SIZE*2/4, 0);
      m_layout[4]->SetStart(CIF_X_SIZE*3/4, 0);
      break;
    }

    case CP_LAYOUT_1P4HOR_UP:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 5);
      m_layout[0]->SetStart(0, GetNegativeCoord(LAY_1P4HOR_UP_Y_OFFSET)); // the reason is cropping
      m_layout[1]->SetStart(0, CIF_Y_SIZE*3/4);
      m_layout[2]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE*3/4);
      m_layout[3]->SetStart(CIF_X_SIZE*2/4, CIF_Y_SIZE*3/4);
      m_layout[4]->SetStart(CIF_X_SIZE*3/4, CIF_Y_SIZE*3/4);
      break;
    }

    case CP_LAYOUT_1P4VER:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 5);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(3*CIF_X_SIZE/4, 0);
      m_layout[2]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE/4);
      m_layout[3]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE*2/4);
      m_layout[4]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE*3/4);
      break;
    }

    case CP_LAYOUT_1P8CENT:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 9);
      m_layout[0]->SetStart(0, CIF_Y_SIZE/4);
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[3]->SetStart(CIF_X_SIZE/2, 0);
      m_layout[4]->SetStart(3*CIF_X_SIZE/4, 0);
      m_layout[5]->SetStart(3*CIF_X_SIZE/4, 3*CIF_Y_SIZE/4);
      m_layout[6]->SetStart(CIF_X_SIZE/2, 3*CIF_Y_SIZE/4);
      m_layout[7]->SetStart(CIF_X_SIZE/4, 3*CIF_Y_SIZE/4);
      m_layout[8]->SetStart(0, 3*CIF_Y_SIZE/4);
      break;
    }

    case CP_LAYOUT_1P8HOR_UP:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 9);
      m_layout[0]->SetStart(0, GetNegativeCoord(LAY_1P8HOR_UP_Y_OFFSET));
      m_layout[1]->SetStart(0, CIF_Y_SIZE/2);
      m_layout[2]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE/2);
      m_layout[3]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE/2);
      m_layout[4]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE/2);
      m_layout[5]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE*3/4);
      m_layout[6]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE*3/4);
      m_layout[7]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE*3/4);
      m_layout[8]->SetStart(0, CIF_Y_SIZE*3/4);
      break;
    }

    case CP_LAYOUT_1P8UP:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 9);
      m_layout[0]->SetStart(0, CIF_Y_SIZE/2);
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[3]->SetStart(CIF_X_SIZE/2, 0);
      m_layout[4]->SetStart(3*CIF_X_SIZE/4, 0);
      m_layout[5]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE/4);
      m_layout[6]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE/4);
      m_layout[7]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE/4);
      m_layout[8]->SetStart(0, CIF_Y_SIZE/4);
      break;
    }

    case CP_LAYOUT_1TOP_LEFT_P8:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 9);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart((CIF_X_SIZE/4)*2, 0);
      m_layout[2]->SetStart((CIF_X_SIZE/4)*3, 0);
      m_layout[3]->SetStart((CIF_X_SIZE/4)*2, CIF_Y_SIZE/3);
      m_layout[4]->SetStart((CIF_X_SIZE/4)*3, CIF_Y_SIZE/3);
      m_layout[5]->SetStart(0, (CIF_Y_SIZE/3)*2);
      m_layout[6]->SetStart((CIF_X_SIZE/4), (CIF_Y_SIZE/3)*2);
      m_layout[7]->SetStart((CIF_X_SIZE/4)*2, (CIF_Y_SIZE/3)*2);
      m_layout[8]->SetStart((CIF_X_SIZE/4)*3, (CIF_Y_SIZE/3)*2);
      break;
    }

    case CP_LAYOUT_4X4:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 16);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[2]->SetStart((CIF_X_SIZE/4)*2, 0);
      m_layout[3]->SetStart((CIF_X_SIZE/4)*3, 0);
      m_layout[4]->SetStart(0, CIF_Y_SIZE/4);
      m_layout[5]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE/4);
      m_layout[6]->SetStart((CIF_X_SIZE/4)*2, CIF_Y_SIZE/4);
      m_layout[7]->SetStart((CIF_X_SIZE/4)*3, CIF_Y_SIZE/4);
      m_layout[8]->SetStart(0, (CIF_Y_SIZE/4)*2);
      m_layout[9]->SetStart(CIF_X_SIZE/4, (CIF_Y_SIZE/4)*2);
      m_layout[10]->SetStart((CIF_X_SIZE/4)*2, (CIF_Y_SIZE/4)*2);
      m_layout[11]->SetStart((CIF_X_SIZE/4)*3, (CIF_Y_SIZE/4)*2);
      m_layout[12]->SetStart(0, (CIF_Y_SIZE/4)*3);
      m_layout[13]->SetStart((CIF_X_SIZE/4), (CIF_Y_SIZE/4)*3);
      m_layout[14]->SetStart((CIF_X_SIZE/4)*2, (CIF_Y_SIZE/4)*3);
      m_layout[15]->SetStart((CIF_X_SIZE/4)*3, (CIF_Y_SIZE/4)*3);
      break;
    }

    case CP_LAYOUT_2P8:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 10);
      m_layout[0]->SetStart(0, CIF_Y_SIZE/4);
      m_layout[1]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE/4);
      m_layout[2]->SetStart(0, 0);
      m_layout[3]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[4]->SetStart(CIF_X_SIZE/2, 0);
      m_layout[5]->SetStart(3*CIF_X_SIZE/4, 0);
      m_layout[6]->SetStart(0, 3*CIF_Y_SIZE/4);
      m_layout[7]->SetStart(CIF_X_SIZE/4, 3*CIF_Y_SIZE/4);
      m_layout[8]->SetStart(CIF_X_SIZE/2, 3*CIF_Y_SIZE/4);
      m_layout[9]->SetStart(3*CIF_X_SIZE/4, 3*CIF_Y_SIZE/4);
      break;
    }

    case CP_LAYOUT_2TOP_P8:
    {
       DBGPASSERT_AND_RETURN(m_numberOfSubImages != 10);
       m_layout[0]->SetStart(0, 0);
       m_layout[1]->SetStart((CIF_X_SIZE/4)*2, 0);
       m_layout[2]->SetStart(0, (CIF_Y_SIZE/4)*2);
       m_layout[3]->SetStart(CIF_X_SIZE/4, (CIF_Y_SIZE/4)*2);
       m_layout[4]->SetStart((CIF_X_SIZE/4)*2, (CIF_Y_SIZE/4)*2);
       m_layout[5]->SetStart((CIF_X_SIZE/4)*3, (CIF_Y_SIZE/4)*2);
       m_layout[6]->SetStart(0, (CIF_Y_SIZE/4)*3);
       m_layout[7]->SetStart(CIF_X_SIZE/4, (CIF_Y_SIZE/4)*3);
       m_layout[8]->SetStart((CIF_X_SIZE/4)*2, (CIF_Y_SIZE/4)*3);
       m_layout[9]->SetStart((CIF_X_SIZE/4)*3, (CIF_Y_SIZE/4)*3);
       break;
    }

    case CP_LAYOUT_1P12:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 13);
      m_layout[0]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE/4);
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[3]->SetStart(2*CIF_X_SIZE/4, 0);
      m_layout[4]->SetStart(3*CIF_X_SIZE/4, 0);
      m_layout[5]->SetStart(0, CIF_Y_SIZE/4);
      m_layout[6]->SetStart(3*CIF_X_SIZE/4, CIF_Y_SIZE/4);
      m_layout[7]->SetStart(0, 2*CIF_Y_SIZE/4);
      m_layout[8]->SetStart(3*CIF_X_SIZE/4, 2*CIF_Y_SIZE/4);
      m_layout[9]->SetStart(0, 3*CIF_Y_SIZE/4);
      m_layout[10]->SetStart(CIF_X_SIZE/4, 3*CIF_Y_SIZE/4);
      m_layout[11]->SetStart(CIF_X_SIZE/2, 3*CIF_Y_SIZE/4);
      m_layout[12]->SetStart(3*CIF_X_SIZE/4, 3*CIF_Y_SIZE/4);
      break;
    }

    case CP_LAYOUT_1X2_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 2);
      m_layout[0]->SetStart(CIF_X_SIZE/8, CIF_Y_SIZE/6);
      m_layout[1]->SetStart(CIF_X_SIZE/2, CIF_Y_SIZE/6);
      break;
    }

    case CP_LAYOUT_1P2HOR_RIGHT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, CIF_Y_SIZE/2);
      m_layout[1]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[2]->SetStart(5*CIF_X_SIZE/8, 0);
      break;
    }

    case CP_LAYOUT_1P2HOR_LEFT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, CIF_Y_SIZE/2);
      m_layout[1]->SetStart(0, 0);
      m_layout[2]->SetStart(3*CIF_X_SIZE/8, 0);
      break;
    }

    case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(CIF_X_SIZE/4, CIF_Y_SIZE/2);
      m_layout[2]->SetStart(5*CIF_X_SIZE/8, CIF_Y_SIZE/2);
      break;
    }

    case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(0, CIF_Y_SIZE/2);
      m_layout[2]->SetStart(3*CIF_X_SIZE/8, CIF_Y_SIZE/2);
      break;
    }

    case CP_LAYOUT_2X2_UP_RIGHT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, QCIF_Y_SIZE);
      m_layout[1]->SetStart(QCIF_X_SIZE, QCIF_Y_SIZE);
      m_layout[2]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[3]->SetStart(5*CIF_X_SIZE/8, 0);
      break;
    }

    case CP_LAYOUT_2X2_UP_LEFT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, QCIF_Y_SIZE);
      m_layout[1]->SetStart(QCIF_X_SIZE, QCIF_Y_SIZE);
      m_layout[2]->SetStart(0, 0);
      m_layout[3]->SetStart(3*CIF_X_SIZE/8, 0);
      break;
    }

    case CP_LAYOUT_2X2_DOWN_RIGHT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(QCIF_X_SIZE, QCIF_Y_SIZE);
      m_layout[2]->SetStart(CIF_X_SIZE/4, QCIF_Y_SIZE);
      m_layout[3]->SetStart(5*CIF_X_SIZE/8, QCIF_Y_SIZE);
      break;
    }

    case CP_LAYOUT_2X2_DOWN_LEFT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(QCIF_X_SIZE, QCIF_Y_SIZE);
      m_layout[2]->SetStart(0, QCIF_Y_SIZE);
      m_layout[3]->SetStart(3*CIF_X_SIZE/8, QCIF_Y_SIZE);
      break;
    }

    case CP_LAYOUT_2X2_RIGHT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(CIF_X_SIZE/4, 0);
      m_layout[1]->SetStart(5*CIF_X_SIZE/8, 0);
      m_layout[2]->SetStart(CIF_X_SIZE/4, QCIF_Y_SIZE);
      m_layout[3]->SetStart(5*CIF_X_SIZE/8, QCIF_Y_SIZE);
      break;
    }

    case CP_LAYOUT_2X2_LEFT_FLEX:
    {
      DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
      m_layout[0]->SetStart(0, 0);
      m_layout[1]->SetStart(3*CIF_X_SIZE/8, 0);
      m_layout[2]->SetStart(0, QCIF_Y_SIZE);
      m_layout[3]->SetStart(3*CIF_X_SIZE/8, QCIF_Y_SIZE);
      break;
    }

	case CP_LAYOUT_OVERLAY_1P1:
	{
		DBGPASSERT_AND_RETURN(m_numberOfSubImages != 2);
		m_layout[0]->SetStart(0,0);
		m_layout[1]->SetStart(3*CIF_X_SIZE/8,3*CIF_Y_SIZE/4);
		break;
	}

	case CP_LAYOUT_OVERLAY_1P2:
	{
		DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
		m_layout[0]->SetStart(0,0);
		m_layout[1]->SetStart(3*CIF_X_SIZE/16,3*CIF_Y_SIZE/4);
		m_layout[2]->SetStart(9*CIF_X_SIZE/16,3*CIF_Y_SIZE/4);
		break;
	}

	case CP_LAYOUT_OVERLAY_1P3:
	{
		DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
		m_layout[0]->SetStart(0,0);
		m_layout[1]->SetStart(CIF_X_SIZE/16,3*CIF_Y_SIZE/4);
		m_layout[2]->SetStart(3*CIF_X_SIZE/8,3*CIF_Y_SIZE/4);
		m_layout[3]->SetStart(5*CIF_X_SIZE/8,3*CIF_Y_SIZE/4);
		break;
	}

	case CP_LAYOUT_OVERLAY_ITP_1P2:
	{
		DBGPASSERT_AND_RETURN(m_numberOfSubImages != 3);
		m_layout[0]->SetStart(0,0);
		m_layout[1]->SetStart(3*CIF_X_SIZE/8,3*CIF_Y_SIZE/4);
		m_layout[2]->SetStart(CIF_X_SIZE/2,3*CIF_Y_SIZE/4);
		break;
	}

	case CP_LAYOUT_OVERLAY_ITP_1P3:
	{
		DBGPASSERT_AND_RETURN(m_numberOfSubImages != 4);
		m_layout[0]->SetStart(0,0);
		m_layout[1]->SetStart(CIF_X_SIZE/4,3*CIF_Y_SIZE/4);
		m_layout[2]->SetStart(9*CIF_X_SIZE/16,3*CIF_Y_SIZE/4);
		m_layout[3]->SetStart(7*CIF_X_SIZE/8,3*CIF_Y_SIZE/4);
		break;
	}

	case CP_LAYOUT_OVERLAY_ITP_1P4:
	{
		DBGPASSERT_AND_RETURN(m_numberOfSubImages != 5);
		m_layout[0]->SetStart(0,0);
		m_layout[1]->SetStart(3*CIF_X_SIZE/16,3*CIF_Y_SIZE/4);
		m_layout[2]->SetStart(3*CIF_X_SIZE/8,3*CIF_Y_SIZE/4);
		m_layout[3]->SetStart(CIF_X_SIZE/2,3*CIF_Y_SIZE/4);
		m_layout[4]->SetStart(13*CIF_X_SIZE/16,3*CIF_Y_SIZE/4);
		break;
	}
	
    default:
    {
      DBGPASSERT(m_layoutType);
      break;
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
// return number of first set cell or 0xff if not found
BYTE CLayout::isSetInOtherCell(const char* szPartyName) const
{
  BYTE res = 0xFF;
  if (szPartyName == NIL(const char))
  {
    PASSERT(1);
    return res;
  }

  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i]->isForced())
    {
      if (!strncmp(m_layout[i]->GetPartyForce(), szPartyName, H243_NAME_LEN))
      {
        res = i;
        break;
      }
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayout::isLayoutEmpty() const
{
  for (WORD i = 0; i < m_numberOfSubImages; i++)
    if (m_layout[i]->GetImageId() != 0)
      return FALSE;

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CLayout::SetLayoutType(LayoutType layoutType)
{
  if (m_layoutType == layoutType)
    return;

  m_layoutType        = layoutType;
  m_numberOfSubImages = GetNumbSubImg(layoutType);

  for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
    POBJDELETE(m_layout[i]);

  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    m_layout[i] = new CVidSubImage();
    m_layout[i]->SetLayoutTypeAndSizes(m_layoutType, i);
  }

  m_isActive              = NO;

  SetStartsForImages();
}

////////////////////////////////////////////////////////////////////////////
void CLayout::SetLayoutFromRes(CVideoLayout& operLayout, Force_Level forceLevel)
{
  if (::GetOldLayoutType(m_layoutType) != operLayout.GetScreenLayout() )
  {
    PASSERT(1);
    return;
  }

  if ( forceLevel != CONF_lev && forceLevel != PARTY_lev )
  {
    PASSERT(forceLevel);
    return;
  }

  if (operLayout.m_numb_of_cell == 0)
  {
    PTRACE(eLevelInfoNormal, "CLayout::SetLayoutFromRes : empty layout");
    return;
  }

  char              partyName[H243_NAME_LEN];
  CVideoCellLayout* cellLayout;
  CCommConfDB*      pCommConfDB = ::GetpConfDB();

  PASSERT_AND_RETURN(!pCommConfDB);

  BYTE cellNumb = 0;
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    cellLayout = operLayout.GetCurrentCell(i+1);
    if (cellLayout == NIL(CVideoCellLayout))
    {
      // in case of CP the cell can be zero - that mean that the cell is audio activation
      continue;
    }

    cellNumb = cellLayout->GetCellId() - 1;
    PASSERT_AND_RETURN( (cellLayout->GetCellId() - 1) < 0 || cellNumb >= MAX_SUB_IMAGES_IN_LAYOUT);

    if (cellLayout->IsBlank())
    {
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, BLANK_PARTY_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, BLANK_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, BLANK_CONF_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
    }
    else if (cellLayout->IsAutoScan())        // case AutoScan
    { // if there is no cell in layout that is already set for auto scan
      if (GetAutoScanCell() >= cellNumb) // ???
      {
        // check if this condition is correct
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, AUTO_SCAN_Active);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        PASSERTMSG(cellNumb+1, "CLayout::SetLayoutFromRes more than one cell in layout is configured to auto_scan");
      }
    }
    else if (!cellLayout->IsAudioActivated()) // case force
    {
      PASSERT_AND_RETURN(cellLayout->GetName() == NIL(char));
      strncpy(partyName, cellLayout->GetName(), sizeof(partyName) - 1);
      partyName[sizeof(partyName) -1] = '\0';
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_PARTY_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PARTY_Activ);
          m_layout[cellNumb]->SetPartyForceName(partyName);
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
          m_layout[cellNumb]->SetPartyForceName(partyName);
        }
      }
    }
    else                                      // audio activated
    {
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_PARTY_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
    }
  } // end for
}

////////////////////////////////////////////////////////////////////////////
void CLayout::SetLayout(CVideoLayout& operLayout, Force_Level forceLevel)
{
  WORD wOldLayoutType = GetOldLayoutType(m_layoutType);
  BYTE btScreenLayout = operLayout.GetScreenLayout();

  std::ostringstream str;
  str << "  oldLayoutType       :" << CVideoLayout::LayoutTypeToString(wOldLayoutType) << endl;
  str << "  newLayoutTpe        :" << CVideoLayout::LayoutTypeToString(btScreenLayout) << endl;
  str << "  forceLevel          :" << (int)forceLevel << (forceLevel == CONF_lev ? " (ConferenceLevel)" : " (PartyLevel)") << endl;

  string s;
  operLayout.ToString(s);
  str << "VIDEO_LAYOUT:\n" << s.c_str();
  PTRACE2(eLevelInfoNormal, "CLayout::SetLayout:\n", str.str().c_str());

  if (::GetOldLayoutType(m_layoutType) != operLayout.GetScreenLayout() ) {
    PASSERT_AND_RETURN(1);
  }

  if ( forceLevel != CONF_lev && forceLevel != PARTY_lev ) {
    PASSERT_AND_RETURN(forceLevel);
  }

  if (operLayout.m_numb_of_cell == 0) {
    PASSERT_AND_RETURN(2);
  }

  char              partyName[H243_NAME_LEN];
  DWORD             partyId;
  CVideoCellLayout* cellLayout;
  CCommConfDB*      pCommConfDB = ::GetpConfDB();

  PASSERT_AND_RETURN(!pCommConfDB);

  BYTE cellNumb = 0;
  // the bug: you can set sophisticated layout ( from 1+5 family ), in small cell set force ,
  // apply .Next step to set Audio active in this cell and force to the same part in an other
  // All forces are removed
  // Fixing strategy : when copying operator ( a new one ) layout , first of all copy empty
  // cells , and then forces
  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    cellLayout = operLayout.GetCurrentCell(i+1);
    if (cellLayout == NIL(CVideoCellLayout))
    {
      // in case of CP the cell can be zero - that mean that the cell is
      // audio activation
      continue;
    }

    cellNumb = cellLayout->GetCellId() - 1;
    if (((cellLayout->GetCellId() - 1) < 0 || cellNumb >= MAX_SUB_IMAGES_IN_LAYOUT) ||
		cellLayout->IsBlank() ||
		!(cellLayout->IsAudioActivated() || cellLayout->IsAutoScan()) ||
		m_layout[cellNumb]->GetRequestPriority() == CHAIRMAN_Prior)
    {
      continue;
    }
    else   // audio activated
    {
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_PARTY_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
    }
  }

  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    cellLayout = operLayout.GetCurrentCell(i+1);
    if (cellLayout == NIL(CVideoCellLayout))
    {
      // in case of CP the cell can be zero - that means that the cell is
      // audio activation
      continue;
    }

    cellNumb = cellLayout->GetCellId() - 1;
    PASSERT_AND_RETURN ((cellLayout->GetCellId() - 1) < 0 || cellNumb >= MAX_SUB_IMAGES_IN_LAYOUT);

    if (cellLayout->IsBlank())
    {
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, BLANK_PARTY_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, BLANK_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, BLANK_CONF_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
    }
    else if (cellLayout->IsAutoScan())        // case AutoScan
    { // if there is no cell in layout that is already set for auto scan
      if (GetAutoScanCell() >= cellNumb)      // ???
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, AUTO_SCAN_Active);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        PASSERTMSG(cellNumb+1, "CLayout::SetLayout more than one cell in layout is configured to auto_scan");
      }
    }
    else if (!cellLayout->IsAudioActivated()) // case force
    {
      partyId = cellLayout->GetForcedPartyId();
      const char* pDBpartyName = pCommConfDB->GetPartyName(m_confName, partyId);
      PASSERT_AND_RETURN(pDBpartyName == NIL(char));
      strncpy(partyName, pDBpartyName, sizeof(partyName) - 1);

      partyName[sizeof(partyName) - 1] = '\0';

      BYTE isExist = isSetInOtherCell(partyName);
      if (isExist == 0xFF)
      {
        // In this case the forced party does not appear in any other sub-image and therefore
        // we will act according to the existing logic and set the force if it can be setted according
        // to the relation table.
        if (forceLevel == PARTY_lev)
        {
          if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_PARTY_Activ))
          {
            m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PARTY_Activ);
            m_layout[cellNumb]->SetPartyForceName(partyName);
          }
        }
        else
        {
          if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_CONF_Activ))
          {
            m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
            m_layout[cellNumb]->SetPartyForceName(partyName);
          }
        }
      }
      else
      {
        if (isExist != cellNumb && isExist < MAX_SUB_IMAGES_IN_LAYOUT)              // We do not need to perform any thing if the force doesn't change.
        {
          // This means that the requesting forced party is already forced in the existing reservation
          // layout and in this case:
          // We will create a virtual sub-image (using operator new) and set the current request priority and video activity
          // (From the current reservation layout) as the current state of this virtual sub-image.
          // We will use the CanBeSetTo function with the new requested layout request priority and video activity
          // in order to determine if it is possible to switch forces according to the relation table,
          // and if it's possible we will perform the switch.
          CVidSubImage* pSubImage = new CVidSubImage;
          pSubImage->SetForceAttributes(m_layout[isExist]->GetRequestPriority(), m_layout[isExist]->GetVideoActivities());
          if (forceLevel == PARTY_lev)
          {
            if (pSubImage->CanBeSetTo(OPERATOR_Prior, FORCE_PARTY_Activ))
            {
              m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PARTY_Activ);
              m_layout[cellNumb]->SetPartyForceName(partyName);
              m_layout[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
              m_layout[isExist]->RemovePartyForceName();
            }
          }
          else
          {
            if (pSubImage->CanBeSetTo(OPERATOR_Prior, FORCE_CONF_Activ))
            {
              m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
              m_layout[cellNumb]->SetPartyForceName(partyName);
              m_layout[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
              m_layout[isExist]->RemovePartyForceName();
            }
          }

          POBJDELETE(pSubImage);
        }
      }
    }
  } // end for
}

////////////////////////////////////////////////////////////////////////////
void CLayout::SetLayout(CVideoLayout& operLayout, Force_Level forceLevel, WORD isPrivate)
{
  if (::GetOldLayoutType(m_layoutType) != operLayout.GetScreenLayout() )
  {
    PASSERT(1);
    return;
  }

  if ( forceLevel != CONF_lev && forceLevel != PARTY_lev )
  {
    PASSERT(forceLevel);
    return;
  }

  if (operLayout.m_numb_of_cell == 0)
  {
    PTRACE(eLevelInfoNormal, "CLayout::SetLayout : empty layout");
    return;
  }

  char              partyName[H243_NAME_LEN];
  DWORD             partyId;
  CVideoCellLayout* cellLayout;
  CCommConfDB*      pCommConfDB = ::GetpConfDB();

  PASSERT_AND_RETURN(!pCommConfDB);

  BYTE cellNumb = 0;
  // the bug: you can set sophisticated layout ( from 1+5 family ), in small cell set force ,
  // apply .Next step to set Audio active in this cell and force to the same part in an other
  // All forces are removed
  // Fixing strategy : when copying operator ( a new one ) layout , first of all copy empty
  // cells , and then forces
  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    cellLayout = operLayout.GetCurrentCell(i+1);
    if (cellLayout == NIL(CVideoCellLayout))
    {
      // in case of CP the cell can be zero - that mean that the cell is audio activation
      continue;
    }

    cellNumb = cellLayout->GetCellId() - 1;
    if (((cellLayout->GetCellId() - 1) < 0 || cellNumb >= MAX_SUB_IMAGES_IN_LAYOUT) ||
		cellLayout->IsBlank() ||
		!(cellLayout->IsAudioActivated() || cellLayout->IsAutoScan()) ||
		m_layout[cellNumb]->GetRequestPriority() == CHAIRMAN_Prior)
    {
      continue;
    }
    else   // audio activated
    {
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_PARTY_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
    }
  }

  for (int i = 0; i < m_numberOfSubImages; i++)
  {
    cellLayout = operLayout.GetCurrentCell(i+1);
    if (cellLayout == NIL(CVideoCellLayout))
    {
      // in case of CP the cell can be zero - that mean that the cell is
      // audio activation
      continue;
    }

    cellNumb = cellLayout->GetCellId() - 1;
    if ((cellLayout->GetCellId() - 1) < 0 || cellNumb >= MAX_SUB_IMAGES_IN_LAYOUT)
    	continue;

    if (cellLayout->IsBlank())
    {
      if (forceLevel == PARTY_lev)
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, BLANK_PRIVATE_PARTY_Active))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, BLANK_PRIVATE_PARTY_Active);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, BLANK_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, BLANK_CONF_Activ);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
    }
    else if (cellLayout->IsAutoScan())        // case AutoScan
    { // if there is no cell in layout that is already set for auto scan
      if (GetAutoScanCell() >= cellNumb) // ???
      {
        // check if this condition is correct - ToDo later Eitan!!!!
        if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
        {
          m_layout[cellNumb]->SetForceAttributes(AUTO_Prior, AUTO_SCAN_Active);
          m_layout[cellNumb]->RemovePartyForceName();
        }
      }
      else
      {
        PASSERTMSG(cellNumb+1, "CLayout::SetLayout more than one cell in layout is configured to auto_scan");
      }
    }
    else if (!cellLayout->IsAudioActivated()) // case force
    {
      partyId = cellLayout->GetForcedPartyId();
      const char* pDBpartyName = pCommConfDB->GetPartyName(m_confName, partyId);
      if (pDBpartyName == NIL(char))
      {
    	  PASSERT_AND_RETURN(1000+partyId);
//        if (partyId == 0)
//          PASSERT_AND_RETURN(partyId+1);
//        else
//          PASSERT_AND_RETURN(partyId);
      }

      // Being run over in the line below ???: strncpy(partyName,pDBpartyName,H243_NAME_LEN);
      strcpy_safe(partyName, pDBpartyName);
      BYTE isExist = isSetInOtherCell(partyName);
      if (isExist == 0xFF)
      {
        // In this case the forced party does not appear in any other sub-image and therefore
        // we will act according to the existing logic and set the force if it can be setted according
        // to the relation table.

        if (forceLevel == PARTY_lev)
        {
          if (isPrivate != 1)                 // Conference layout
          {
            if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_PARTY_Activ))
            {
              m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PARTY_Activ);
              m_layout[cellNumb]->SetPartyForceName(partyName);
            }
          }
          else                                // Private layout //
          {
            if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_PRIVATE_PARTY_Active))
            {
              m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PRIVATE_PARTY_Active);
              m_layout[cellNumb]->SetPartyForceName(partyName);
            }
            else
            {
              if (m_layout[cellNumb]->CanBeSetTo(PARTY_Prior, FORCE_PRIVATE_PARTY_Active))
              {
                m_layout[cellNumb]->SetForceAttributes(PARTY_Prior, FORCE_PRIVATE_PARTY_Active);
                m_layout[cellNumb]->SetPartyForceName(partyName);
              }
            }
          }
        }
        else
        {
          if (m_layout[cellNumb]->CanBeSetTo(OPERATOR_Prior, FORCE_CONF_Activ))
          {
            m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
            m_layout[cellNumb]->SetPartyForceName(partyName);
          }
        }
      }
      else
      {
        if (isExist != cellNumb && isExist < MAX_SUB_IMAGES_IN_LAYOUT)              // We do not need to perform any thing if the force doesn't change.
        {
          // This means that the requesting forced party is already forced in the existing reservation
          // layout and in this case:
          // We will create a virtual sub-image (using operator new) and set the current request priority and video activity
          // (From the current reservation layout) as the current state of this virtual sub-image.
          // We will use the CanBeSetTo function with the new requested layout request priority and video activity
          // in order to determine if it is possible to switch forces according to the relation table,
          // and if it's possible we will perform the switch.
          CVidSubImage* pSubImage = new CVidSubImage;
          pSubImage->SetForceAttributes(m_layout[isExist]->GetRequestPriority(), m_layout[isExist]->GetVideoActivities());
          if (forceLevel == PARTY_lev)
          {
            if (isPrivate != 1)               // Conference layout
            {
              if (pSubImage->CanBeSetTo(OPERATOR_Prior, FORCE_PARTY_Activ))
              {
                m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PARTY_Activ);
                m_layout[cellNumb]->SetPartyForceName(partyName);
                m_layout[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
                m_layout[isExist]->RemovePartyForceName();
              }
            }
            else                              // Private layout //
            {
              if (pSubImage->CanBeSetTo(OPERATOR_Prior, FORCE_PRIVATE_PARTY_Active))
              {
                m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_PRIVATE_PARTY_Active);
                m_layout[cellNumb]->SetPartyForceName(partyName);
                m_layout[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
                m_layout[isExist]->RemovePartyForceName();
              }
              else
              {
                if (pSubImage->CanBeSetTo(PARTY_Prior, FORCE_PRIVATE_PARTY_Active))
                {
                  m_layout[cellNumb]->SetForceAttributes(PARTY_Prior, FORCE_PRIVATE_PARTY_Active);
                  m_layout[cellNumb]->SetPartyForceName(partyName);
                  m_layout[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
                  m_layout[isExist]->RemovePartyForceName();
                }
              }
            }
          }
          else
          {
            if (pSubImage->CanBeSetTo(OPERATOR_Prior, FORCE_CONF_Activ))
            {
              m_layout[cellNumb]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
              m_layout[cellNumb]->SetPartyForceName(partyName);
              m_layout[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
              m_layout[isExist]->RemovePartyForceName();
            }
          }

          POBJDELETE(pSubImage);
        }
      }
    }
  } // end for
}

////////////////////////////////////////////////////////////////////////////
// returns 0 in case of success , 1 otherwise
BYTE CLayout::Serialize(Force_Level inf_level, CSegment* pSeg) const
{
	PASSERT_AND_RETURN_VALUE(!pSeg, 1);
	PASSERT_AND_RETURN_VALUE(inf_level != CONF_lev && inf_level != PARTY_lev, 1);

	WORD numb_sub_img = GetNumberOfSubImages();
	*pSeg << (BYTE)inf_level;
	*pSeg << numb_sub_img;
	*pSeg << (BYTE)GetLayoutType();
	if (inf_level == CONF_lev)
		*pSeg << (BYTE)m_isActive;

	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	for (int i = 0; i < numb_sub_img && i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
	{
		CVidSubImage* pCurrSubImg = m_layout[i];
		PASSERTSTREAM_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pCurrSubImg), "Invalid sub-image, index:" << i, 1);

		*pSeg << (BYTE)NO;
		*pSeg << (BYTE)pCurrSubImg->GetVideoActivities();
		*pSeg << (BYTE)pCurrSubImg->GetRequestPriority();
		if (pCurrSubImg->GetPartyForce())
			*pSeg << pCurrSubImg->GetPartyForce();
		else
			*pSeg << "";

		if (inf_level == PARTY_lev)
		{
			const CTaskApp* pVideoSource = NULL;
			DWORD partyRscId = pCurrSubImg->GetImageId();
			if (partyRscId)
			{
				CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
				PASSERTSTREAM(!pImage, "CLayout::Serialize - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

				if(NULL != pImage){
				  pVideoSource = pImage->GetVideoSource();
				}
			}
			*pSeg << (DWORD)pVideoSource;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
BYTE operator==(const CLayout& first, const CLayout& second)
{
  BYTE rval = YES;

  if (first.GetLayoutType() != second.GetLayoutType())
  {
    rval = NO;
    return rval;
  }

  for (WORD i = 0; i < first.m_numberOfSubImages; i++)
  {
    if (first.m_layout[i] != NIL(CVidSubImage) && second.m_layout[i] != NIL(CVidSubImage))
    {
      if (!(*(first.m_layout[i]) == *(second.m_layout[i])))
      {
        rval = NO;
        break;
      }
    }
    else
    {
      FPASSERT(1);
      rval = NO;
      break;
    }
  }

  return rval;
}

////////////////////////////////////////////////////////////////////////////
BYTE operator!=(const CLayout& first, const CLayout& second)
{
  return (!(first == second));
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayout::operator==(CVideoLayout& operLayout) const
{
  BYTE  result = YES;
  char  partyName[H243_NAME_LEN];
  DWORD partyId;

  if (::GetOldLayoutType(m_layoutType) != operLayout.GetScreenLayout())
    result = NO;

  if (result == YES && isActive() != operLayout.IsActive())
    result = NO;

  if (result)
  {
    CCommConfDB* pCommConfDB = ::GetpConfDB();
    PASSERT_AND_RETURN_VALUE(!pCommConfDB, NO);

    CVideoCellLayout* cellLayout;
    for (WORD i = 0; i < m_numberOfSubImages; i++)
    {
      cellLayout = operLayout.GetCurrentCell(i+1);
      if (cellLayout == NIL(CVideoCellLayout))
        continue;

      int cellNumb = cellLayout->GetCellId() - 1;
      DBGPASSERT(cellNumb >= m_numberOfSubImages);

      if (cellNumb < 0 || cellNumb >= MAX_SUB_IMAGES_IN_LAYOUT)
    	  continue;

      if (cellLayout->IsBlank())
      {
        if (m_layout[cellNumb]->isBlanked())
          continue;
        else
        {
          result = NO;
          break;
        }
      }
      else      // case when cell is forced
      {
        const RequestPriority rp = m_layout[cellNumb]->GetRequestPriority();
        const VideoActivities va = m_layout[cellNumb]->GetVideoActivities();
        if (!cellLayout->IsAudioActivated())
        {
          if (!cellLayout->IsAutoScan())
          {
            // here we know that the new cell if forced
            if (m_layout[cellNumb]->isForced())
            {
              partyId = cellLayout->GetForcedPartyId();
              const char* forcedPartyNameFromOper = pCommConfDB->GetPartyName(m_confName, partyId);
              if (forcedPartyNameFromOper == NULL)
              {
                PASSERT(1);
                return NO;
              }

              strncpy(partyName, forcedPartyNameFromOper, sizeof(partyName) - 1);
              partyName[sizeof(partyName) - 1] = '\0';
              if (strcmp(m_layout[cellNumb]->GetPartyForce(), partyName) == 0)
              {
                continue;
              }
              else
              {
                result = NO;
                break;
              }
            }
            else
            {
              result = NO;
              break;
            }
          }
          else  // case received the cell auto_scan
          {
            if (rp == AUTO_Prior && va == AUTO_SCAN_Active)
            {
              continue;
            }
            else
            {
              result = NO;
              break;
            }
          } // auto scan
        } // not audio activate
        else    // case received the cell aud activate
        {
          if (rp == AUTO_Prior && va == DEFAULT_Activ)
            continue;
          else
          {
            result = NO;
            break;
          }
        }
      }
    } // end for
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////
void CLayout::RemovePartyForce(const char* ppartyName, RequestPriority who_asked)
{
  PASSERT_AND_RETURN(ppartyName == NIL(char));

  WORD numOfSubImages = GetNumberOfSubImages();
  if(numOfSubImages <= MAX_SUB_IMAGES_IN_LAYOUT)
  {
    for (WORD j = 0; j < GetNumberOfSubImages(); j++)
    {
      CVidSubImage* pSumImage = m_layout[j];
      PASSERT_AND_RETURN(pSumImage == NIL(CVidSubImage));
 
      const char* pPartyForce = pSumImage->GetPartyForce();
      if (pPartyForce != NIL(char) && strcmp(ppartyName, pPartyForce) == 0)
        pSumImage->ClearForce(who_asked);
    }
  }
  else
  	PASSERTMSG(1, "GetNumberOfSubImages() exceed MAX_SUB_IMAGES_IN_LAYOUT");
}

////////////////////////////////////////////////////////////////////////////
void CLayout::ClearCurrentView(void)
{
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i] != NIL(CVidSubImage))
    {
      m_layout[i]->ClearForce();
      m_layout[i]->SetImageId(0);
    }
    else
    {
      PASSERT(1);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayout::NumberOfCellWherePartyForced(const CTaskApp* pParty) const
{
  BYTE result = AUTO; // mean is not forced
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i] != NIL(CVidSubImage))
    {
      if (m_layout[i]->isForced())
      {
        if (strncmp(m_layout[i]->GetPartyForce(), ((CParty*)pParty)->GetName(), H243_NAME_LEN-1) == 0)
        {
          result = i;
          break;
        }
      }
    }
    else
      PASSERT(1);
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////
void CLayout::CleanUp(void)
{
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i] != NIL(CVidSubImage))
      m_layout[i]->CleanUp();
    else
      PASSERT(1);
  }
}

////////////////////////////////////////////////////////////////////////////
void CLayout::ClearAllImageSources(void)
{
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i] != NIL(CVidSubImage))
      m_layout[i]->SetImageId(0);
    else
      PASSERT(1);
  }
}

////////////////////////////////////////////////////////////////////////////
void CLayout::ClearAllForces(void)
{
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i] != NIL(CVidSubImage))
      m_layout[i]->ClearForce();
    else
      PASSERT(1);
  }
}

////////////////////////////////////////////////////////////////////////////
WORD CLayout::FindFirstUnUsedCell(void) const
{
  for (WORD i = 0; i < m_numberOfSubImages; i++)
    if (m_layout[i]->noImgSet())
      return i;
  return AUTO;
}
////////////////////////////////////////////////////////////////////////////
BYTE CLayout::IsAutoScanSet() const
{
  return (GetAutoScanCell() != AUTO) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CLayout::GetAutoScanCell() const
{
  WORD result = AUTO;
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    if (m_layout[i]->GetVideoActivities() == AUTO_SCAN_Active)
    {
      result = i;
      break;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////
WORD CLayout::FindImagePlaceInLayout(DWORD partyRscId) const
{
  PASSERT_AND_RETURN_VALUE(!partyRscId, AUTO);

  for (WORD i = 0; i < m_numberOfSubImages; i++)
    if (partyRscId == m_layout[i]->GetImageId())
      return i;
  return AUTO;
}

////////////////////////////////////////////////////////////////////////////
CVidSubImage* CLayout::GetSubImageOfForcedParty(const char* ForcedParty, WORD& SubImageNumber)
{
  for (SubImageNumber = 0; SubImageNumber < m_numberOfSubImages; SubImageNumber++)
  {
    if (m_layout[SubImageNumber] != NIL(CVidSubImage))
      if (m_layout[SubImageNumber]->isForced())
        if (strcmp(m_layout[SubImageNumber]->GetPartyForce(), ForcedParty) == 0)
          return m_layout[SubImageNumber];
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
void CLayout::DumpForces(COstrStream& msg)const
{
	msg << "Dump forces for layout: ";
	msg << LayoutTypeAsString[m_layoutType] << "\n";
  msg << "Number of SubImages is: " << m_numberOfSubImages <<"\n";
  const CVidSubImage* pVidSubImage;
  for (WORD i = 0; i < m_numberOfSubImages; i++)
  {
    pVidSubImage = m_layout[i];
    if (pVidSubImage == NIL(CVidSubImage))
      break;

    msg << "	SubImage# " << i << "\n";
    pVidSubImage->DumpForces(msg);
    msg << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////
void CLayout::Dump(std::ostream& msg) const
{
	msg << "\nLayoutType  :" << LayoutTypeAsString[m_layoutType];
	msg << "\nSubImages # :" << m_numberOfSubImages;

	for (WORD i = 0; i < m_numberOfSubImages; i++)
	{
		const CVidSubImage* pVidSubImage = m_layout[i];
		if (pVidSubImage == NIL(CVidSubImage))
		{
			msg << "\nSubImg numb :" << i << " - Invalid";
			continue;
		}

		msg << "\nSubImg numb :" << i << " - ";
		pVidSubImage->Dump(msg);
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayout::SetVswLayout(DWORD partyRscId, const char* pConfName)
{
	PASSERT_AND_RETURN(!partyRscId);

	strcpy_safe(m_confName, pConfName);

	m_numberOfSubImages = 1;

	if (m_layout[0] == NULL)
		m_layout[0] = new CVidSubImage();

	m_layout[0]->SetLayoutTypeAndSizes(CP_LAYOUT_1X1, 0);
	m_layout[0]->SetImageId(partyRscId);

	m_isActive = YES;

	SetStartsForImages();
}

////////////////////////////////////////////////////////////////////////////
CLayout& CLayout::operator=(const CLayout& other)
{
  if (&other == this)
    return *this;

  strncpy(m_confName, other.m_confName, H243_NAME_LEN-1);
  m_confName[H243_NAME_LEN -1] = '\0';

  m_layoutType        = other.GetLayoutType();
  m_numberOfSubImages = other.GetNumberOfSubImages();

  for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
  {
    POBJDELETE(m_layout[i]);
    if (i < m_numberOfSubImages)
      m_layout[i] = new CVidSubImage(*(other[i]));
  }

  m_isActive = other.isActive();
  return *this;
}

////////////////////////////////////////////////////////////////////////////
CVidSubImage* CLayout::GetSubImageNum(WORD pos)
{
  if (pos < MAX_SUB_IMAGES_IN_LAYOUT)
    return m_layout[pos];
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
void CLayout::initIndications()
{
	const CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	WORD selfLocation = 0;
	DWORD cellLocation = 0;

	std::string szCellLocation;

	if (pSysConfig)
	{

		pSysConfig->GetDataByKey(CFG_KEY_CELL_IND_LOCATION,szCellLocation);		
		cellLocation = TranslateSysConfigStringToIconLocation(szCellLocation);
		//PTRACE2INT(eLevelInfoNormal,"CLayout::initIndications - cellLocation = ",cellLocation);
	}

	CCommConfDB*      pCommConfDB = ::GetpConfDB();
	PASSERT_AND_RETURN(!pCommConfDB);
	CCommConf* pCommConf = pCommConfDB->GetCurrentConf(m_confName);
	if(pCommConf)
	{
		selfLocation = pCommConf->GetIconDisplayPosition();

	
		BOOL  isEnableRecordingIcon= pCommConf->GetEnableRecordingIcon();
		BOOL  isEnableSelfNetworkQualityIcon= pCommConf->GetEnableSelfNetworkQualityIcon();
		BOOL  isEnableAudioParticipantsIcon= pCommConf->GetEnableAudioParticipantsIcon();
		WORD  stripIndexArray[MAX_NUM_TYPES_OF_ICON];
		WORD  index = 0;
		memset(stripIndexArray,0,sizeof(stripIndexArray));
	
		//indication order: Recording, audio priticipants, self network quality indication.
		if(isEnableRecordingIcon)
			stripIndexArray[eIconRecording] = index++;
		if(isEnableAudioParticipantsIcon)
			stripIndexArray[eIconAudioPartiesCount] = index++;
		if(isEnableSelfNetworkQualityIcon)	
			stripIndexArray[eIconNetworkQuality] = index++;

		indications().locations(static_cast<iconLocationEnum>(selfLocation), static_cast<iconLocationEnum>(cellLocation),stripIndexArray);
	}
}


/////////////////////////////////////////////////////////////////////////////////////
DWORD CLayout::TranslateSysConfigStringToIconLocation(std::string szLocation)
{
	DWORD iLocation = 0;

	if(szLocation == "TOP")
	{
		iLocation = 0;
	}
	else if(szLocation == "BOTTOM")
	{
		iLocation = 1;
	}
  	else if(szLocation == "LEFT")
	{
		iLocation = 2;
	}
	else if(szLocation == "RIGHT")
	{
		iLocation = 3;
	}
	else if(szLocation == "TOP_LEFT")
	{
		iLocation = 4;
	}
	else if(szLocation == "TOP_RIGHT")
	{
		iLocation = 5;
	}
	else if(szLocation == "BOTTOM_LEFT")
	{
		iLocation = 6;
	}
	else if(szLocation == "BOTTOM_RIGHT")
	{
		iLocation = 7;
	}
	else
	{
		iLocation = 0;	
	}
	return iLocation;
}


/*//////////////////////////////////////////////////////////////////////////////////////
//Change Layout Improvement - Conf Layout (CL-CL)
BOOL CLayout::IsThereAnyConfForcesInLayout()
{
	for (int i = 0; i < m_numberOfSubImages; i++)
		if (m_layout[i] && (m_layout[i]->isForcedInConfLevel()))
			return TRUE;

	return FALSE;
}
*/

