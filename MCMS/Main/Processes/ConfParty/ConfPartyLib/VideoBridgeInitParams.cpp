#include "VideoBridgeInitParams.h"
#include "LectureModeParams.h"


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeInitParams
////////////////////////////////////////////////////////////////////////////
CVideoBridgeInitParams::CVideoBridgeInitParams()
{
  m_pVisualEffects        = NULL;
  m_pLectureMode          = NULL;
  m_isSameLayout          = FALSE;
  m_isAutoLayout          = FALSE;
  m_videoQuality          = eVideoQualitySharpness; // Sharpness is the default video quality
  m_isVideoClarityEnabled = FALSE;
  m_pCommConf             = NULL;
  m_isSiteNamesEnabled    = YES;
  m_pCopRsrcs             = new CopRsrcsParams;
  m_XCodeInitParamsList   = new  XCODE_INIT_PARAMS_LIST;
  m_pContentXcodeRsrcs    = new  XCODE_RESOURCES_MAP;
  memset(m_pCopRsrcs, 0, sizeof(CopRsrcsParams));
  m_pVideoBridgeContentDecoder = NULL;
  m_isMuteAllVideoButLeader = NO;
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeInitParams::CVideoBridgeInitParams (const CVideoBridgeInitParams& rVideoBridgeInitParams) :
  CBridgeInitParams(rVideoBridgeInitParams)
{
  if (rVideoBridgeInitParams.GetVisualEffectsParams())
    m_pVisualEffects = new CVisualEffectsParams(*rVideoBridgeInitParams.GetVisualEffectsParams());
  else
    m_pVisualEffects = NULL;

  if (rVideoBridgeInitParams.GetLectureModeParams())
    m_pLectureMode = new CLectureModeParams(*rVideoBridgeInitParams.GetLectureModeParams());
  else
    m_pLectureMode = NULL;

  m_isSameLayout          = rVideoBridgeInitParams.GetIsSameLayout();
  m_isAutoLayout          = rVideoBridgeInitParams.GetIsAutoLayout();
  m_videoQuality          = rVideoBridgeInitParams.GetVideoQuality();
  m_isVideoClarityEnabled = rVideoBridgeInitParams.GetIsVideoClarityEnabled();
  m_pCommConf             = rVideoBridgeInitParams.GetCommConf();
  m_isSiteNamesEnabled    = rVideoBridgeInitParams.GetIsSiteNamesEnabled();
  m_pCopRsrcs             = new CopRsrcsParams;
  m_XCodeInitParamsList   = new  XCODE_INIT_PARAMS_LIST;
  m_pContentXcodeRsrcs    = new  XCODE_RESOURCES_MAP;
  m_pVideoBridgeContentDecoder = rVideoBridgeInitParams.GetVideoBridgeContentDecoder();
  m_isMuteAllVideoButLeader = rVideoBridgeInitParams.GetIsMuteAllVideoButLeader();

  memset(m_pCopRsrcs, 0, sizeof(CopRsrcsParams));
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeInitParams::CVideoBridgeInitParams (const CConf* pConf,
                                                const char* pConfName,
                                                ConfRsrcID confRsrcId,
                                                const EBridgeImplementationTypes eBridgeImplementationType,
                                                CVisualEffectsParams* pVisualEffects,
                                                CLectureModeParams* pLectureMode,
                                                BYTE byIsSameLayout,
                                                BYTE byIsAutoLayout,
                                                eVideoQuality videoQuality,
                                                BYTE byIsVideoClarityEnabled,
                                                CCommConf* pCommConf,
                                                BYTE byIsSiteNamesEnabled, BYTE byisMuteAllVideoButLeader) :

  CBridgeInitParams(pConf, pConfName, confRsrcId, eBridgeImplementationType)
{
  m_pVisualEffects        = new CVisualEffectsParams(*pVisualEffects);
  m_pLectureMode          = new CLectureModeParams(*pLectureMode);
  m_isSameLayout          = byIsSameLayout;
  m_isAutoLayout          = byIsAutoLayout;
  m_videoQuality          = videoQuality;
  m_isVideoClarityEnabled = byIsVideoClarityEnabled;
  m_pCommConf             = pCommConf;
  m_isSiteNamesEnabled    = byIsSiteNamesEnabled;
  m_pCopRsrcs             = new CopRsrcsParams;
  m_XCodeInitParamsList   = new  XCODE_INIT_PARAMS_LIST;
  m_pContentXcodeRsrcs    = new  XCODE_RESOURCES_MAP;
  m_isMuteAllVideoButLeader = byisMuteAllVideoButLeader;

  memset(m_pCopRsrcs, 0, sizeof(CopRsrcsParams));
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeInitParams::~CVideoBridgeInitParams ()
{
  POBJDELETE(m_pVisualEffects);
  POBJDELETE(m_pLectureMode);
  POBJDELETE(m_pCopRsrcs);
  for(XCODE_INIT_PARAMS_LIST::iterator _ii = m_XCodeInitParamsList->begin(); _ii != m_XCodeInitParamsList->end(); _ii++)
  {
	 POBJDELETE(_ii->second);
  }
  delete m_XCodeInitParamsList;

  for(XCODE_RESOURCES_MAP::iterator _ii = m_pContentXcodeRsrcs->begin(); _ii != m_pContentXcodeRsrcs->end(); _ii++)
  {
	  delete _ii->second;
  }
  delete m_pContentXcodeRsrcs;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeInitParams::GetLectureModeType() const
{
  return (m_pLectureMode) ? m_pLectureMode->GetLectureModeType() : NO;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeInitParams::GetLectureModeTimerInterval() const
{
  return (m_pLectureMode) ? m_pLectureMode->GetLectureTimeInterval() : NO;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeInitParams::GetLectureModeAudioActivated() const
{
  return (m_pLectureMode) ? m_pLectureMode->GetAudioActivated() : NO;
}

////////////////////////////////////////////////////////////////////////////
const char* CVideoBridgeInitParams::GetLecturerName() const
{
  return (m_pLectureMode) ? m_pLectureMode->GetLecturerName() : NULL;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeInitParams::GetLectureModeTimerOnOff() const
{
  return (m_pLectureMode) ? m_pLectureMode->GetTimerOnOff() : NO;
}


