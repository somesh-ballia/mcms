#ifndef _CVideoBridgeInitParams_H_
#define _CVideoBridgeInitParams_H_

#include "BridgeInitParams.h"
#include "Conf.h"
#include "VisualEffectsParams.h"
#include "LectureModeParams.h"
#include "COP_ConfParty_Defs.h"
#include "ConfPartyDefines.h"
#include "VideoBridgePartyCntl.h"

typedef std::map <eXcodeRsrcType, CBridgePartyVideoParams*> XCODE_INIT_PARAMS_LIST;
////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeInitParams
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeInitParams : public CBridgeInitParams
{
  CLASS_TYPE_1(CVideoBridgeInitParams, CBridgeInitParams)

public:
                          CVideoBridgeInitParams();
                          CVideoBridgeInitParams(const CVideoBridgeInitParams&);
                          CVideoBridgeInitParams(const CConf* pConf,
                                                 const char* pConfName,
                                                 ConfRsrcID confRsrcId,
                                                 const EBridgeImplementationTypes eBridgeImplementationType,
                                                 CVisualEffectsParams* pVisualEffects,
                                                 CLectureModeParams* pLectureMode,
                                                 BYTE byIsSameLayout,
                                                 BYTE byIsAutoLayout,
                                                 eVideoQuality videoQuality,
                                                 BYTE byIsVideoClarityEnabled,
                                                 CCommConf* m_pCommConf,
                                                 BYTE byIsSiteNamesEnabled,
                                                 BYTE byisMuteAllVideoButLeader);

  virtual                ~CVideoBridgeInitParams();

  CVideoBridgeInitParams& operator =(const CVideoBridgeInitParams& rVideoBridgeInitParams);
  virtual const char*     NameOf() const                                    {return "CVideoBridgeInitParams";}
  BYTE                    GetLectureModeType() const;
  WORD                    GetLectureModeTimerInterval() const;
  BYTE                    GetLectureModeAudioActivated() const;
  const char*             GetLecturerName() const;
  BYTE                    GetLectureModeTimerOnOff() const;
  CLectureModeParams*     GetLectureModeParams() const                      {return m_pLectureMode;}
  CVisualEffectsParams*   GetVisualEffectsParams() const                    {return m_pVisualEffects;}
  BYTE                    GetIsSameLayout() const                           {return m_isSameLayout;}
  BYTE                    GetIsAutoLayout() const                           {return m_isAutoLayout;}
  eVideoQuality           GetVideoQuality() const                           {return m_videoQuality;}
  CCommConf*              GetCommConf() const                               {return m_pCommConf;}
  BYTE                    GetIsVideoClarityEnabled() const                  {return m_isVideoClarityEnabled;}
  BYTE                    GetIsSiteNamesEnabled() const                     {return m_isSiteNamesEnabled;}
  BYTE                    GetIsMuteAllVideoButLeader() const                {return m_isMuteAllVideoButLeader;}

  // COP_VIDEO_BRIDGE
  CopRsrcsParams*         GetCopRsrcsParams() const                         {return m_pCopRsrcs;}
  void                    SetCopRsrcsParams(CopRsrcsParams* pCopRsrcParams) {*m_pCopRsrcs = *pCopRsrcParams;}

  //VIDEO_CONTENT_XCODE_BRIDGE
   XCODE_INIT_PARAMS_LIST* GetXCodeInitParamsList(){return m_XCodeInitParamsList;}
   XCODE_RESOURCES_MAP*    GetXCodeResourcesMap(){return m_pContentXcodeRsrcs;}
   CVideoBridgePartyCntlContent* GetVideoBridgeContentDecoder() const{ return m_pVideoBridgeContentDecoder;}
   void SetVideoBridgeContentDecoder(CVideoBridgePartyCntlContent* pVideoBridgeContentDecoder){m_pVideoBridgeContentDecoder = pVideoBridgeContentDecoder;}
private:
  CVisualEffectsParams*   m_pVisualEffects;
  CLectureModeParams*     m_pLectureMode;
  CCommConf*              m_pCommConf;
  BYTE                    m_isSameLayout;
  BYTE                    m_isAutoLayout;
  eVideoQuality           m_videoQuality;
  BYTE                    m_isVideoClarityEnabled;
  BYTE                    m_isSiteNamesEnabled;
  BYTE                    m_isMuteAllVideoButLeader;

  // COP_VIDEO_BRIDGE
  CopRsrcsParams*         m_pCopRsrcs;
  //VIDEO_CONTENT_XCODE_BRIDGE
  XCODE_RESOURCES_MAP*      m_pContentXcodeRsrcs;
  XCODE_INIT_PARAMS_LIST*   m_XCodeInitParamsList;
  CVideoBridgePartyCntlContent* m_pVideoBridgeContentDecoder;
};

#endif // _CVideoBridgeInitParams_H_
