#ifndef VIDEOAPIDEFINITIONSSTRINGS_H_
#define VIDEOAPIDEFINITIONSSTRINGS_H_

static const char* EVideoEncoderTypeNames[] =
{
  "E_VIDEO_ENCODER_DUMMY",
  "E_VIDEO_ENCODER_NORMAL",
  "E_VIDEO_ENCODER_MASTER_LB_ONLY",
  "E_VIDEO_ENCODER_SLAVE_FULL_ENCODER",
  "E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER",
  "E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER",
  "E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER_HALF_DSP",
  "E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER_HALF_DSP",
  "E_VIDEO_ENCODER_PCM",
  "E_VIDEO_ENCODER_LAST"
};


static const char* EVideoDecoderTypeNames[] =
{
  "E_VIDEO_DECODER_DUMMY",
  "E_VIDEO_DECODER_NORMAL",
  "E_VIDEO_DECODER_CONTENT",
  "E_VIDEO_DECODER_LAST"
};

static const char* EVideoProtocolNames[] =
{
  "E_VIDEO_PROTOCOL_DUMMY",
  "E_VIDEO_PROTOCOL_H261",
  "E_VIDEO_PROTOCOL_DUMMY2",
  "E_VIDEO_PROTOCOL_H263",
  "E_VIDEO_PROTOCOL_H264",
  "E_VIDEO_PROTOCOL_RTV",
  "E_VIDEO_PROTOCOL_SVC",
  "E_VIDEO_PROTOCOL_MS_SVC",
  "E_VIDEO_PROTOCOL_VP8"
};

static const char* EVideoResolutionNames[] =
{
  "E_VIDEO_RES_DUMMY",
  "E_VIDEO_RES_QCIF",
  "E_VIDEO_RES_CIF",
  "E_VIDEO_RES_VGA",
  "E_VIDEO_RES_4CIF",
  "E_VIDEO_RES_525SD",
  "E_VIDEO_RES_625SD",
  "E_VIDEO_RES_SVGA",
  "E_VIDEO_RES_XGA",
  "E_VIDEO_RES_HD",
  "E_VIDEO_RES_16CIF",
  "E_VIDEO_RES_SIF",
  "E_VIDEO_RES_QVGA",
  "E_VIDEO_RES_QSIF",
  "E_VIDEO_RES_QVGA"
};


static const char* EVideoFrameRateNames[] =
{
  "E_VIDEO_FPS_DUMMY",
  "E_VIDEO_60_FPS",
  "E_VIDEO_50_FPS",
  "E_VIDEO_30_FPS",
  "E_VIDEO_25_FPS",
  "E_VIDEO_20_FPS",
  "E_VIDEO_15_FPS",
  "E_VIDEO_12_5_FPS",
  "E_VIDEO_10_FPS",
  "E_VIDEO_7_5_FPS",
  "E_VIDEO_6_FPS",
  "E_VIDEO_5_FPS",
  "E_VIDEO_3_FPS"
};


static const char* ELayoutTypeNames[] =
{
  "E_VIDEO_LAYOUT_DUMMY",
  "E_VIDEO_LAYOUT_1X1",
  "E_VIDEO_LAYOUT_1X2",
  "E_VIDEO_LAYOUT_2X1",
  "E_VIDEO_LAYOUT_2X2",
  "E_VIDEO_LAYOUT_3X3",
  "E_VIDEO_LAYOUT_4X4",
  "E_VIDEO_LAYOUT_1P5",
  "E_VIDEO_LAYOUT_1P7",
  "E_VIDEO_LAYOUT_1X2HOR",
  "E_VIDEO_LAYOUT_1X2VER",
  "E_VIDEO_LAYOUT_1P2VER",
  "E_VIDEO_LAYOUT_1P2HOR",
  "E_VIDEO_LAYOUT_1P3HOR_UP",
  "E_VIDEO_LAYOUT_1P3VER",
  "E_VIDEO_LAYOUT_1P4HOR",
  "E_VIDEO_LAYOUT_1P4VER",
  "E_VIDEO_LAYOUT_1P8CENT",
  "E_VIDEO_LAYOUT_1P8UP",
  "E_VIDEO_LAYOUT_1P8HOR_UP",
  "E_VIDEO_LAYOUT_1P2HOR_UP",
  "E_VIDEO_LAYOUT_1P3HOR",
  "E_VIDEO_LAYOUT_1P4HOR_UP",
  "E_VIDEO_LAYOUT_1P12",
  "E_VIDEO_LAYOUT_2P8",
  "E_VIDEO_LAYOUT_1X1_QCIF",
  "E_VIDEO_LAYOUT_FLEX_1X2",
  "E_VIDEO_LAYOUT_FLEX_1P2HOR_RIGHT",
  "E_VIDEO_LAYOUT_FLEX_1P2HOR_LEFT",
  "E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_RIGHT",
  "E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_LEFT",
  "E_VIDEO_LAYOUT_FLEX_2X2_UP_RIGHT",
  "E_VIDEO_LAYOUT_FLEX_2X2_UP_LEFT",
  "E_VIDEO_LAYOUT_FLEX_2X2_DOWN_RIGHT",
  "E_VIDEO_LAYOUT_FLEX_2X2_DOWN_LEFT",
  "E_VIDEO_LAYOUT_FLEX_2X2_RIGHT",
  "E_VIDEO_LAYOUT_FLEX_2X2_LEFT",
  "E_VIDEO_LAYOUT_OVERLAY_1P1",
  "E_VIDEO_LAYOUT_OVERLAY_1P2",
  "E_VIDEO_LAYOUT_OVERLAY_1P3",
  "E_VIDEO_LAYOUT_OVERLAY_ITP_1P2",
  "E_VIDEO_LAYOUT_OVERLAY_ITP_1P3",
  "E_VIDEO_LAYOUT_OVERLAY_ITP_1P4",
  "E_VIDEO_LAYOUT_1TOP_LEFT_P8",
  "E_VIDEO_LAYOUT_2TOP_P8",
  "E_VIDEO_NO_LAYOUT"
};

static const char* ERelativeSizeOfImageInLayoutNames[] =
{
  "E_NOT_IN_LAYOUT",
  "E_QQUARTER_SCREEN",
  "UNKNOWN",
  "UNKNOWN",
  "E_QUARTER_SCREEN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "UNKNOWN",
  "E_FULL_SCREEN"
};

static const char* EVideoQualityTypeNames[] =
{
  "E_VIDEO_QUALITY_DUMMY",
  "E_VIDEO_QUALITY_SHARPNESS",
  "E_VIDEO_QUALITY_MOTION",
  "E_VIDEO_QUALITY_LAST"
};

static const char* EVideoConfTypeNames[] =
{
  "E_VIDEO_CONF_TYPE_DUMMY",
  "E_VIDEO_CONF_TYPE_CP",
  "E_VIDEO_CONF_TYPE_VSW",
  "E_VIDEO_CONF_TYPE_COP_HD1080_25FPS",
  "E_VIDEO_CONF_TYPE_COP_HD720_50FPS",
  "E_VIDEO_CONF_TYPE_CP_CONTENT_XCODE",
  "E_VIDEO_CONF_TYPE_LAST"
};
static const char* EVideoProfileTypeNames[] =
{
  "E_PROFILE_DUMMY",
  "E_PROFILE_BASELINE",
  "E_PROFILE_HIGH",
  "E_PROFILE_RTV",
  "E_PROFILE_MAIN",
  "E_PROFILE_LAST"
};
static const char* EIconRecordingTypeNames[] =
{
  "E_ICON_DUMMY",
  "E_ICON_REC_OFF",
  "E_ICON_REC_ON",
  "E_ICON_REC_PAUSE",
  "E_ICON_REC_LAST"
};


#endif /*VIDEOAPIDEFINITIONSSTRINGS_H_*/
