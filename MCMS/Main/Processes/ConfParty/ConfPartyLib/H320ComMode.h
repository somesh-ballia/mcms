//+========================================================================+
//                            H320ComMode.H                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320ComMode.H                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 10.10.07   |                                                      |
//+========================================================================+


#ifndef _H320COMMODE_H
#define _H320COMMODE_H

#include "PObject.h"
#include "ConfPartyApiDefines.h"
#include "H320AudioCaps.h"
#include "H221StrCap.h"
#include "H264.h"
#include "H264Cap.h"
#include "ConfPartyDefines.h"
#include "TraceStream.h"

class CSegment;
class CCommConf;


#define   LSD_NONE                1
#define   LSD_DYNAMIC_6400_ONLY   2
#define   LSD_DYNAMIC             3
#define   LSD_FIXED               4
#define   LSD_DYNAMIC_4800_ONLY   5
#define   LSD_FIXED_4800_GW       6
#define   NOT_VALID_CONTENT_RATE  100

#define   H26L    VIDEO_PROTOCOL_H26L

const  int  B0 = 1;
const  int  H0 = 6;

const int LIVE_ROLE_LABEL_AMCOpenByte1			= 0x1 << 4;
const int PRESENTAION_ROLE_LABEL_AMCOpenByte1	= 0x2 << 4;
const int ID_ONLY_BIT_MASK						= (0x3F);
const int FOUR_LEAST_SIGNIFICANT_BITS			= (0x0F);
const int FOUR_MOST_SIGNIFICANT_BITS			= (0xF0);
const int SUB_TIME_SLOTS_IN_BO_CHANNEL			= 8;

class  CSegment;
class  CCapH320;
class  CDHKey;
class  CvidMode;
class  CH264VidMode;
class  CCapSetH264;

class CAudMode : public CPObject
{
CLASS_TYPE_1(CAudMode, CPObject)
public:
  // Constructors
  CAudMode();
  virtual ~CAudMode();

  // Initializations

  // Operations
  void  SetAudMode(WORD audMode);
  void  SetBitRate(WORD audMode);
  void  SetBitRateValue(WORD bitRate) { m_bitRate = bitRate; }
  WORD  GetAudMode() const;
  WORD  GetBitRate() const;

  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);
  WORD  operator<=(const CCapH320& cap) const;

  friend WORD  operator==(const CAudMode& rAud_1,const CAudMode& rAud_2);
  virtual const char*  NameOf() const;
  WORD  GetBitRateClass() const;

  WORD IsAudioOpen() const;

  void Dump (ostringstream& str)const;

protected:

  // Attributes
  WORD    m_audMode;
  WORD    m_bitRate;    // 0 - auto , 16 , 48 , 56
};


class CXferMode : public CPObject
{
CLASS_TYPE_1(CXferMode,CPObject )
public:
  // Constructors
  CXferMode();
  virtual ~CXferMode();

  // Initializations
  void  SetXferMode(WORD mode);
  WORD  GetXferMode() const;
  void  Zero();
  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);

  // Operations
  static WORD GetXferModeByNumChannels(WORD numChannels);

  WORD  GetNumChnl() const;
  WORD  GetChnlWidth() const;
  WORD  GetH221NetChnlWidth() const;
  WORD  operator<=(const CCapH320& cap) const;

  friend WORD  operator==(const CXferMode& rXferMode_1,const CXferMode& rXferMode_2);
  friend WORD  operator<=(const CXferMode& rXferMode_1,const CXferMode& rXferMode_2);
  virtual const char*  NameOf() const;

  void Dump (ostringstream& str)const;

protected:

  // Attributes
  WORD    m_xferMode;
};


//H.26L
class CNSVidMode : public CPObject
{
CLASS_TYPE_1(CNSVidMode, CPObject)
public:

  // Constructors
  CNSVidMode();
  virtual ~CNSVidMode();

  // Operations
  virtual const char*  NameOf() const;
  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);
  WORD  operator<=(const CCapH320& cap) const;

  void SetH26LMpi(WORD NumB0Chnl); //Currently, it used as auto mode only.
  void SetH26LCifMpi(WORD Mpi);
  void SetH26L4CifMpi(WORD Mpi);

  BYTE GetOctet0() const;
  BYTE GetOctet1() const;
  WORD GetH26LCifMpiForOneVideoStream() const;
  WORD GetH26L4CifMpiForOneVideoStream() const;

  protected:
  // Attributes

  //MPI of 1 indicates 30 fps for SIF/4SIF, 25 fps for CIF/4CIF.
  //MPI of 2 indicates 15 fps SIF/4SIF, 12.5 fps CIF/4CIF etc.

  BYTE octet0; //bit 7: must be 0.
               //bits 6-4: MPI for CIF/SIF, when one video stream is active.
               //bits 3-0: MPI for 4CIF/4SIF, when one video stream is active.

  BYTE octet1; //bit 7: must be 0.
               //bits 6-4: MPI for CIF/SIF, when two video streams are active.
               //bits 3-0: MPI for 4CIF/4SIF, when two video streams are active.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//H.264
class CH264VidMode : public CPObject
{
CLASS_TYPE_1(CH264VidMode,CPObject )
public:

  // Constructors
  CH264VidMode();
  virtual ~CH264VidMode();

  // Operations
  virtual const char*  NameOf() const;
  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);
  BYTE  operator<=(const CH264VidMode& vidMode) const;
  //BYTE  operator==(const CH264VidMode& vidMode) const;
  BYTE GetProfileValue() const;
  void SetProfileValue(BYTE profile);
  BYTE GetLevelValue() const;
  void SetLevelValue(BYTE level);
  DWORD GetMaxMBPS() const;
  WORD  GetMaxMBPSasCustomWord() const;
  //void  ConvertMaxMBPS(DWORD mbps);
  void  SetMaxMBPS(WORD mbps);
  void  SetMaxMBPS(DWORD mbps);
  DWORD GetMaxFS() const;
  WORD  GetMaxFSasCustomWord() const;
  //void  ConvertMaxFS(DWORD fs);
  void  SetMaxFS(WORD fs);
  void  SetMaxFS(DWORD fs);
  DWORD GetMaxDPB() const;
  WORD  GetMaxDPBasCustomWord() const;
  //void  ConvertMaxDPB(DWORD dpb);
  void  SetMaxDPB(WORD dpb);
  void  SetMaxDPB(DWORD dpb);
  DWORD GetMaxBR() const;
  WORD  GetMaxBRasCustomWord() const;
  //void  ConvertMaxBR(DWORD br);
  void  SetMaxBR(WORD br);
  void  SetMaxBR(DWORD br);
  DWORD GetMaxCPB() const;
  WORD  GetMaxCPBasCustomWord() const;
  //void  ConvertMaxCPB(DWORD cpb);
  void  SetMaxCPB(WORD cpb);
  void  SetMaxCPB(DWORD cpb);

  void  SetSAR(WORD sar);
  WORD  GetSAR() const;

  void  CalcAndSetCommonLevelAndParam(CH264VidMode vidMode);
  DWORD CalcCommonParamSameLevel(DWORD firstParam, DWORD secondParam);
  DWORD CalcCommonParamDiffLevel(DWORD firstParam,BYTE firstlevel,DWORD secondParam,BYTE secondlevel,
											BYTE customType);
  //DWORD GetDefaultParamForLevel(BYTE level, BYTE Type);
  //BYTE  IsParamSameAsDefaultForLevel(DWORD param,BYTE level, BYTE Type);
  DWORD GetCustomFactor(BYTE Type);
  char* GetLevelAsString() const;
  BYTE  IsLevelValueIsLegal() const;
  BYTE  IsDiffLevelOrParam(CH264VidMode vidMode)const;
  BYTE  IsDiffPLF(CH264VidMode vidMode)const;

//BYTE ConvertBitRateToLevel(BYTE confTransferRate);
//void SetCommonLevel(CCapH320* pCap);

  void Dump (ostringstream& str)const;
  BYTE IsCapableOfHD720_15fps();
  BYTE IsCapableOfHD1080_15fps();
  BYTE IsCapableOfHD720_50fps();
  eVideoPartyType GetCPH264ResourceVideoPartyType();

  protected:
  // Attributes

  BYTE m_profileValue;   // VideoConference bit 2 is 1 / Cable,DigitalTV,DVD...
  BYTE m_levelValue;

  //Aditional Parameters (Optional)
  DWORD  m_MaxMBPS;  // For higher processing rate capability(Rate in MBPS)
  DWORD  m_MaxFS;    // The decoder can decode larger picture(frame) sizes(Resolution MB)
						 // m_CustomMaxMBPS/m_customMaxFS --> FrameRate 1/sec
  DWORD m_MaxDPB;   // The decoder has additional decoded picture buffer memory.
  DWORD m_MaxBR; // The decoder can decode higher video bit rate and larger coded picture buffer
  DWORD m_MaxCPB; // The decoder can decode higher video bit rate and larger coded picture buffer

  WORD m_Sar;

};

class CVidMode : public CPObject
{
CLASS_TYPE_1(CVidMode, CPObject)
public:
  // Constructors
  CVidMode();
  virtual ~CVidMode();

  // Initializations
  void  Create(CCapH320* pCap,WORD mode /*= 0*/,WORD vidMode,WORD RequestedFormat = AUTO);
  void  Dump() const;
  void  DumpFullScm();
  void Intersect(CCapH320* pCap,WORD isAutoVidScm, CVidMode& IntersectVidMode);
  void  SetH261VidMode(WORD vidBitRate,WORD vidImageFormat,WORD qcifMpi,WORD cifMpi = 0);
  void  SetVidMode(WORD mode);
  void  SetH263VidMode(WORD vidBitRate,WORD h263VidImageFormat,WORD h263Mpi = 0);
  void  SetFreeBitRate(BYTE bOnOff = 1)  { m_isFreeBitRate = bOnOff; }
  void  SetVideoSwitchAutoVideoProtocol(WORD set_value = 1) { m_isVideoSwitchAutoVideoProtocol_H261_H263_H264 = set_value; }
  void  SetH263Mpi(WORD H263Mpi);
  void  SetVidImageFormat(WORD imageFormat) {m_vidImageFormat = imageFormat;}
  WORD  GetVidMode() const;
  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);
  WORD  GetVidFormat() const;
  WORD  GetCifMpi() const;
  WORD  GetQcifMpi() const;
  WORD  GetH263Mpi() const;
  WORD  DiffMpiFormat(const CVidMode& other) const;
  WORD  IsFreeBitRate() const { return m_isFreeBitRate; }
  WORD  IsAutoVidScm() const { return m_isAutoVidScm;  }
  WORD  IsVideoSwitchAutoVideoProtocol() const;
  void  SetH26LVidMode(WORD isAutoVidScm,WORD vidImageFormat,WORD NumB0Chnl);
  void  SetH26LVidMode(WORD cif4Mpi,WORD cifMpi);
  WORD  GetH26LMpi(WORD h320FormatType) const ;
  BYTE  GetOctet0() const;
  BYTE  GetOctet1() const;

  void  SetH264VidMode(WORD isAutoVidScm,
					   BYTE h264Level,
					   DWORD maxBR=0xFFFFFFFF,
					   DWORD maxMBPS=0xFFFFFFFF,
					   DWORD maxFS=0xFFFFFFFF,
					   DWORD maxDPB=0xFFFFFFFF,
					   BYTE maxSAR = H264_ALL_LEVEL_DEFAULT_SAR);

  void  SetH264DefaultVswVidMode(WORD isAutoVidScm = TRUE,
								 BYTE h264Level = MAX_H264_LEVEL_SUPPORTED_IN_VSW,
								 DWORD maxBR=MAX_BR_SUPPORTED_IN_VSW,
								 DWORD maxMBPS=MAX_MBPS_SUPPORTED_IN_VSW,
								 DWORD maxFS=MAX_FS_SUPPORTED_IN_VSW,
								 DWORD maxDPB=MAX_DPB_SUPPORTED_IN_VSW);

  void  UpdateH264VidMode(CH264VidMode newH264);
  BYTE  IsH264HigherThanQcifResolution();
  BYTE  IsH264CIFResolution();
  void  GetH264ResolutionFrameRateAndDPB(WORD& resolution, WORD& frameRate, DWORD& dpb );
  void  SetH264VidMode(const CCapSetH264& h264CapSet);
  WORD  IsDiffH264LevelOrParam(const CVidMode& other) const;

  CH264VidMode GetH264VidMode() {return m_H264VidMode;};


  void  SetIsAutoVidScm( WORD IsAutoVidScm) { m_isAutoVidScm = IsAutoVidScm; };
  // Operations

  WORD     operator<=(const CCapH320& cap) const;
  friend WORD  operator==(const CVidMode& rVidMode_1, const CVidMode& rVidMode_2);
  friend WORD  operator!=(const CVidMode& rVidMode_1, const CVidMode& rVidMode_2);
  virtual const char*  NameOf() const;
  void  SetCommonVidMode(const CVidMode& other);
  void  SetCommonVidFormat(const CVidMode& other);
  void  SetCommonMPI(CCapH320* pCap);

  void  SetCifMpi(WORD cifMpi) {m_cifMpi=cifMpi; }
  void  SetQcifMpi(WORD qcifMpi) {m_qcifMpi=qcifMpi; }

	// SIP options
  void  CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol);
  BYTE TranslateFormatValueFromH261toH263(BYTE h261VideoFormat);

	//PLF
  WORD  IsDiffPLFLevelOrParam(const CVidMode& other) const;

  BYTE IsCapableOfHD720_15fps();
  BYTE IsCapableOfHD1080_15fps();
  BYTE IsCapableOfHD720_50fps();

  void Dump (ostringstream& str)const;
  eVideoPartyType GetCPResourceVideoPartyType();

  CH264VidMode	m_H264VidMode;      // Class for H264 parameters
protected:

  BYTE  IsH263FormatAndMpiAreLegal();

  // Attributes
  WORD          m_vidMode;          // 0 = Video_off; 1 = H261; 2 = H263; H264
  WORD          m_isAutoVidScm;     // 1 = VSW Auto video mode
    // if "Auto" was selected in "Video Protocol" field of operator conference's
    // properties. For Video Switching conferences ONLY !!!
  WORD          m_isVideoSwitchAutoVideoProtocol_H261_H263_H264;
  WORD          m_vidImageFormat;
  WORD          m_cifMpi;           // h261 CIF mpi
  WORD          m_qcifMpi;          // h261 QCIF mpi
  WORD          m_isFreeBitRate;    // 0 = VSW ; 1 = CP
  WORD          m_h263Mpi;          // h263 mpi
  CNSVidMode	m_NSCVidMode;       // Class for non-standard video ( = H.26L)
};


class COtherMode : public CPObject
{
CLASS_TYPE_1(COtherMode,CPObject )
public:
  // Constructors
  COtherMode();
  virtual ~COtherMode();

  // Initializations
  void  SetH06BMode(WORD mode);
  void  SetEncrypMode(WORD mode);
  void  SetRestrictMode(WORD mode);
  void  SetLoopMode(WORD mode);


  void  SetMode(WORD mode);

  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);

  // accessors
  WORD GetH06BCompMode() const;
  WORD GetEncrypMode() const;
  WORD GetRestrictMode() const;
  WORD GetLoopMode() const;

  // Operations

  WORD     operator<=(const CCapH320& cap) const;
  friend WORD  operator==(const COtherMode& rOtherMode_1,const COtherMode& rOtherMode_2);
  virtual const char*  NameOf() const;

  void Dump(ostringstream& str)const;

protected:

  // Attributes
  WORD  m_H06Bcomp;
  WORD  m_encryption;
  WORD  m_restrict;
  WORD  m_loop;
};


class CLsdMlpMode : public CPObject
{
CLASS_TYPE_1(CLsdMlpMode, CPObject)
public:
  // Constructors
  CLsdMlpMode();
  virtual ~CLsdMlpMode();

  // Initializations
  void  SetLsdMode(WORD mode);
  void  SetMlpMode(WORD mode);
  void  SetlsdCap(WORD Cap);
  WORD  GetLsdMode() const;
  WORD  GetMlpMode() const;
  WORD  GetlsdCap() const;
  void  SetMode(WORD mode);

  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);

  // Operations

  WORD     operator<=(const CCapH320& cap) const;
  friend WORD  operator==(const CLsdMlpMode& rLsdMlpMode_1,const CLsdMlpMode& rLsdMlpMode_2);
  virtual const char*  NameOf() const;

protected:

  // Attributes
  WORD  m_lsd;
  WORD  m_mlp;
  WORD  m_lsdCap;
};


class CHsdHmlpMode : public CPObject
{
CLASS_TYPE_1(CHsdHmlpMode, CPObject)
public:
  // Constructors
  CHsdHmlpMode();
  virtual ~CHsdHmlpMode();

  // Initializations
  void  SetHsdMode(WORD mode);
  void  SetHmlpMode(WORD mode);
  WORD  GetHsdMode() const;
  WORD  GetHmlpMode() const;
  void  SetMode(WORD mode);

  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);

  // Operations

  WORD     operator<=(const CCapH320& cap) const;
  friend WORD  operator==(const CHsdHmlpMode& rHsdHmlpMode_1,const CHsdHmlpMode& rHsdHmlpMode_2);
  virtual const char*  NameOf() const;

protected:

  // Attributes
  WORD  m_hsd;
  WORD  m_hMlp;
};


class CNonStandardMode : public CPObject
{
CLASS_TYPE_1(CNonStandardMode, CPObject)

public:
	// Constructors
	CNonStandardMode();
	CNonStandardMode( const WORD isContentEnabled,
		const WORD isVisualConcertPc = 0, const WORD isVisualConcertFx = 0,
		const WORD isDuoVideo = 0 );
	virtual ~CNonStandardMode();

	// Initializations
	void SetPeopleContent(const WORD mode);
	void SetVisualConcertPC(const WORD newVisualConcertPcMode);
	void SetVisualConcertFX(const WORD newVisualConcertFxMode);
	void SetDuoVideo(const WORD newDuoVideoMode);

	void  DeSerialize(WORD format,CSegment& seg);
	void  Serialize(WORD format,CSegment& seg);

	WORD GetContentMode() const { return m_isContentEnabled; }

	WORD GetModeVisualConcertPC() const { return m_isVisualConcertPC; }
	WORD GetModeVisualConcertFX() const { return m_isVisualConcertFX; }
	WORD GetModeDuoVideo() const { return m_isDuoVideo; }

	// Operations
	friend WORD  operator==(const CNonStandardMode& rNsMode1,const CNonStandardMode& rNsMode2);
	virtual const char* NameOf() const;

protected:
	// Attributes
	WORD m_isContentEnabled;
	WORD m_isVisualConcertPC;
	WORD m_isVisualConcertFX;
	WORD m_isDuoVideo; // used only for H323 DuoVideo mode
};


class CAMSCMode : public CPObject
{
CLASS_TYPE_1(CAMSCMode,CPObject )
public:

	// Initializations
	virtual void Create(const BYTE opcode, const BYTE ControlID, const BYTE StartSubChannel,const BYTE EndSubChannel,
					    const BYTE mediaModeLen, const BYTE* pMediaMode)=0;

	virtual void SetOpcode(const BYTE opcode)=0;
    virtual	void SetControlID(const BYTE ControlID)=0;
	virtual void SetStartSubChannel(const BYTE StartSubChannel)=0;
	virtual void SetEndSubChannel(const BYTE EndSubChannel)=0;
	virtual void SetMediaMode(const BYTE mediaModeLen, const BYTE  * MediaMode)=0;

	virtual	BYTE GetOpcode() const;
	virtual	BYTE GetControlID() const;
	virtual	BYTE GetStartSubChannel() const;
	virtual	BYTE GetEndSubChannel() const;

	virtual  void  DeSerialize(WORD format,CSegment& seg)=0;
    virtual  void  Serialize(WORD format,CSegment& seg, BYTE isH239 = 0) = 0;

protected:
    // Attributes
	BYTE   m_opcode;
	BYTE   m_ControlID;
	BYTE   m_StartSubChannel;
	BYTE   m_EndSubChannel;
	BYTE   *m_pMediaMode;
	BYTE   m_MediaModeSize;
};

class CContentMode: public CAMSCMode
{
CLASS_TYPE_1(CContentMode, CAMSCMode)
public:
	// Constructors
	CContentMode();
	CContentMode( CContentMode& other );
	CContentMode( const BYTE  opcode, const BYTE ControlID,
					const BYTE StartSubChannel, const BYTE  EndSubChannel,
					const BYTE mediaModeLen, const BYTE* MediaMode,const WORD ContentLavel = eGraphics);
	virtual ~CContentMode();

	// Initializations
	void Create(const BYTE opcode, const BYTE ControlID, const BYTE StartSubChannel,const BYTE EndSubChannel,
					const BYTE mediaModeLen, const BYTE* pMediaMode);
	void CreateWithContentLevel(const BYTE opcode, const BYTE ControlID, const BYTE StartSubChannel,const BYTE EndSubChannel,
					const BYTE mediaModeLen, const BYTE* pMediaMode,const WORD contentLevel = eGraphics);
	void CreateMinimal(const WORD ContentLevel=eGraphics);
	virtual const char* NameOf() const;

	virtual void SetOpcode(const BYTE opcode);
	virtual void SetControlID(const BYTE ControlID);
	virtual void SetStartSubChannel(const BYTE StartSubChannel);
	virtual void SetEndSubChannel(const BYTE EndSubChannel);
	virtual void SetMediaMode(const BYTE mediaModeLen, const BYTE* pMediaMode);
	void SetContentRate (const BYTE ContentRate);
	void SetContentMode(BYTE * dump, BYTE size, BYTE * contentMsgLen);
	BYTE IsAssymetricContentMode(const CContentMode& rContentMode_2) const;

	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg,BYTE isH239 = 0);

	void SetContentModeFromH239(CSegment& seg, BYTE opcode,WORD  confChanNum);
    void CalculateSartEndDummyForH239(WORD  confChanNum);
	virtual void  SerializeToH239(CSegment& seg);

	BYTE GetContentRate() const { return m_ContentRate; }
	BYTE GetContentMediaMode() const { return (m_pMediaMode)? *m_pMediaMode : 0; }
	BYTE GetMediaModeSize() const { return m_MediaModeSize;}

	BYTE CalculateContentRate(const BYTE StartSubChannel, const BYTE EndSubChannel) const;
	BYTE CalculateContentRate() const;
	BYTE IsContentModeOn() const;

	friend WORD  operator==(const CContentMode& rContentMode1,const CContentMode& rContentMode2);
	void   operator=(const CContentMode& other);

	static BYTE IsLineRateValidForEpc(const WORD wLineRate, const WORD wIsLSD);  // line rate in opcodes of H.221
	static BYTE GetEpcContentRateByLineRate(const WORD wLineRate, const WORD isLsd, const WORD ContentLevel); // line rate in opcodes of H.221
	static BYTE GetNumberOfB0ChannelsByEpcContentRate(const BYTE byEpcContentRate);
	static BYTE GetEpcContentRateByNumberOfB0Channels(const BYTE byNumberOfB0Channels);

	static BYTE TranslateRateToH239(const BYTE byEpcContentRate);
	static BYTE TranslateRateFromH239ToEPC(const BYTE byH239ContentRate);

	void SetContentLevel(const WORD EnterpriseMode);
	WORD GetContentLevel(){return m_ContentLevel;};

    void Dump(ostringstream& str)const;

protected:
	// Attributes
	BYTE   m_ContentRate;
	BYTE   m_ContentLevel;  // Content Rate Control

	// Internal use
	void SerializeRoleLabel(CSegment& seg);
};


class CECSMode : public CPObject
{
CLASS_TYPE_1(CECSMode,CPObject )
public:

	// Constructors
  CECSMode();
  virtual ~CECSMode();
  CECSMode( CECSMode& other );
  CECSMode& operator= (const CECSMode& other);
  virtual const char*  NameOf() const;

  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg);

  void CreateLocalComModeECS();
  void SetEncrpAlg(BYTE media, BYTE alg, BYTE param);
  void SaveECSP9(CSegment* pParam);
  void SaveXmitRcvKey(CSegment* pParam);

  BYTE* GetXmitRcvKey();
  CEncrypAlg& GetXmitEncrpAlg();

protected:

  CEncrypAlg m_xmitEncrpAlg;
  BYTE *m_pXmitRcvKey;
};

class CComMode : public CPObject
{
CLASS_TYPE_1(CComMode, CPObject)
public:
	enum eMediaType { eMediaTypeAudio = 1, eMediaTypeVideo, eMediaTypeLsd,
						eMediaTypeHsd, eMediaTypeMlp, eMediaTypeHmlp, eMediaTypeContent };
  // Constructors
  CComMode();
  virtual ~CComMode();
  void  Dump(WORD  switchFlag);
  // Initializations
  CComMode( CComMode& other );

  void  SetAudMode(const CAudMode& rAudMode);
  void  SetVidMode(const CVidMode& rVidMode);
  void  SetXferMode(const CXferMode& rXferMode);
  void  SetOtherMode(const COtherMode& rOtherMode);
  void  SetLsdMlpMode(const CLsdMlpMode& rLsdMlpMode);
  void  SetHsdHmlpMode(const CHsdHmlpMode& rHsdHmlpMode);
  WORD  SetMaxMlpRate(const CCapH320& rCap);
  WORD  SetMaxHmlpRate(const CCapH320& rCap);
  void  SetNsMode(const CNonStandardMode& rNsMode);
  void  SetNsMode(WORD isContentEnabled) { m_nsMode.SetPeopleContent(isContentEnabled); }
  void  SetNsModeContent(WORD isContentEnabled)  { m_nsMode.SetPeopleContent(isContentEnabled); }
  void  SetNsModeVisualConcertPC(WORD isVisualConcertPc) { m_nsMode.SetVisualConcertPC(isVisualConcertPc); }
  void  SetNsModeVisualConcertFX(WORD isVisualConcertFx) { m_nsMode.SetVisualConcertFX(isVisualConcertFx); }
  void  SetNsModeDuoVideo(WORD isDuoVideo) { m_nsMode.SetDuoVideo(isDuoVideo); }

  void SetContentMode(const CContentMode& rContentMode);
  void SetContentFromComMode(const CComMode& rComMode);
  //void SetNsContentMode(BYTE * dump, BYTE size, BYTE * contentMsgLen){m_contentMode.SetContentModeData ( dump, size, contentMsgLen)};

  void SetContentModeOpcode  (BYTE opcode ) { m_contentMode.SetOpcode(opcode);}
  void SetContentModeControlID (BYTE ControlID) { m_contentMode.SetControlID(ControlID);}
  void SetContentModeStartSubChannel (BYTE StartSubChannel) { m_contentMode.SetStartSubChannel( StartSubChannel);}
  void SetContentModeEndSubChannel (BYTE EndSubChannel) { m_contentMode.SetEndSubChannel( EndSubChannel);}
  void SetContentModeMediaMode (const BYTE mediaModeLen,const BYTE* pMediaMode ) { m_contentMode.SetMediaMode(mediaModeLen,pMediaMode);}
  void SetContentModeContentRate (BYTE ContentRate) {m_contentMode.SetContentRate(ContentRate);}

  WORD GetAudMode() const;
  WORD GetVidMode() const;
  WORD GetXferMode() const;

  WORD GetOtherH06BcompMode() const { return m_otherMode.GetH06BCompMode();};
  WORD GetOtherEncrypMode() const { return m_otherMode.GetEncrypMode();};
  void SetOtherEncrypMode(WORD new_mode) {m_otherMode.SetEncrypMode(new_mode);};
  BYTE GetShouldDisconnectOnEncrypFailure() const  {return m_bShouldDisconnectOnEncryptionFailure;};
  void SetShouldDisconnectOnEncrypFailure(WORD bshouldDisconnect)  { m_bShouldDisconnectOnEncryptionFailure = bshouldDisconnect;};
  WORD GetOtherRestrictMode() const { return m_otherMode.GetRestrictMode();};
  WORD GetOtherLoopMode() const { return m_otherMode.GetLoopMode();};

  WORD GetNsContentMode() const { return m_nsMode.GetContentMode(); }
  WORD GetNsModeVisualConcertPC() { return m_nsMode.GetModeVisualConcertPC(); }
  WORD GetNsModeVisualConcertFX() { return m_nsMode.GetModeVisualConcertFX(); }
  WORD GetNsModeDuoVideo() const { return m_nsMode.GetModeDuoVideo(); }

  BYTE  GetContentMode() const  {return  m_contentMode.GetOpcode();}
  BYTE  GetContentModeControlID() const { return m_contentMode.GetControlID();}
  BYTE  GetContentModeStartSubChannel() const  { return m_contentMode.GetStartSubChannel();}
  BYTE  GetContentModeEndSubChannel() const { return m_contentMode.GetEndSubChannel();}
  BYTE  GetContentModeMediaMode() const {return m_contentMode.GetContentMediaMode();}
  BYTE  GetContentModeContentRate() const  {return m_contentMode.GetContentRate();}
  BYTE  IsContentOn() const  { return m_contentMode.IsContentModeOn();}

  WORD GetLsdMode() const { return m_lsdMlpMode.GetLsdMode();};
  WORD GetMlpMode() const { return m_lsdMlpMode.GetMlpMode();};

  WORD GetHsdMode() const { return m_hsdHmlpMode.GetHsdMode();};
  WORD GetHmlpMode() const { return m_hsdHmlpMode.GetHmlpMode();};

  WORD GetNumB0Chnl() const;
  WORD GetNumVideoChnl() const;
  WORD GetChnlWidth() const { return m_xferMode.GetChnlWidth();};
  WORD GetH221NetChnlWidth() const { return m_xferMode.GetH221NetChnlWidth();};
  WORD GetNumChnl()  const { return m_xferMode.GetNumChnl();};
  WORD IsFreeVideoRate() const;

  void  HandleBas(BYTE bas,CSegment& seg);
  void  HandleNsComBas(CSegment& seg);
  void	HandleH230EscComBas(CSegment& seg);

  void  GetNextNSCommand(CSegment& seg);
  void  SetFullBitRateAudioOnly();
  void  Zero(WORD restrict = 0);
  WORD  VideoOn() const;
  WORD  HsdOn() const;
  WORD  LsdOn() const;
  WORD  MlpOn() const;
  WORD  HmlpOn() const;

  void  DeSerialize(WORD format,CSegment& seg);
  void  Serialize(WORD format,CSegment& seg, BYTE isH239 = 0);

  void SetVidMode(const CCapSetH264& h264CapSet);

  // hc phase 2
  void SetVidModeOff();
  void SetH264Scm(BYTE CommonLevel,WORD CommonMaxMBPS, WORD CommonMaxFS, WORD CommonMaxDPB, WORD CommonMaxBRandCPB);
  void SetH263InterlacedScm(WORD qcif_mpi_h221_val,WORD cif_mpi_h221_val,WORD resolution_h221);
  void SetH263Scm(WORD resolution,BYTE mpi);
  void SetH261Scm(WORD resolution, WORD qcif_mpi, WORD cif_mpi);


  // Operations

  void  SetXferMode(WORD xferMode);  //new
  void  SetXferMode(WORD chnlWidth,WORD numChnl,WORD aggragation);  //old
  void  SetXferMode(WORD chnlWidth,WORD numChnl);  //old
  WORD  operator<=(const CCapH320& cap) const;
  friend WORD  operator==(const CComMode& rComMode_1,const CComMode& rComMode_2);
  friend WORD  operator<=(const CComMode& rComMode_1,const CComMode& rComMode_2);
  WORD  IsClosureOfVideoOnly(const CComMode& rComMode_2) const;
  WORD  IsClosureOfVideoAndContentOnly(const CComMode& rComMode_2) const;
  WORD  IsAsymmetricVideoProtocolOnly(const CComMode& rComMode_2) const;
  WORD  IsAsymmetricContentModeOnly(const CComMode& rComMode_2) const;

  virtual const char*  NameOf() const;

  void  UpdateH221string(BYTE isH239 = 0);
  BYTE  *GetH221str() const;
  WORD  GetH221StrLen() const;

  void  operator=(const CComMode& other);

  void GetMediaBitrate(	DWORD& audio_bitrate,DWORD& video_bitrate,
						DWORD& lsd_bitrate,DWORD& hsd_bitrate,
						DWORD& mlp_bitrate,DWORD& hmlp_bitrate,
						DWORD& content_bitrate,WORD channelWidthForCalc = 0);
  DWORD GetMediaBitrate(const eMediaType mediaType,WORD channelWidthForCalc = 0);

  BYTE GetVideoBitrateInB(WORD channelWidthForCalc = 0);

  WORD IsSameAudioBitrate(const CComMode &other) const;

  WORD IsAudioOpen() const { return m_audMode.IsAudioOpen(); }

  void CreateLocalComModeECS();
  WORD IsDifferInXferOnly(const CComMode& rComMode_2) const;
  BYTE IsDifferInContentRateOnly(const CComMode& rComMode_2) const;
  // create sip options
  void CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol,BYTE partyEncryptionMode = AUTO);

  // functions added to support UnifiedComMode
  void  SetAutoVideoScm(WORD bIsAutoVidScm) { m_vidMode.SetIsAutoVidScm(bIsAutoVidScm); }
  void  SetH264VideoMode(BYTE h264Level, DWORD maxBR, DWORD maxMBPS,
  						DWORD maxFS, DWORD maxDPB, WORD isAutoVidScm);
  void  SetIsFreeVideoRate(WORD bIsFreeVideoRate) { m_vidMode.SetFreeBitRate(bIsFreeVideoRate); }
  void  SetH264VideoParams(H264VideoModeDetails h264VidModeDetails, long sar, WORD isAutoVidScm);
  void  SetHd720Enabled(BYTE bIsHd720Enabled);
  BYTE  IsHd720Enabled() const;
  void  SetHd1080Enabled(BYTE bIsHd1080Enabled);
  BYTE  IsHd1080Enabled() const;
  void  SetHd720At60Enabled(BYTE bIsHd720At60Enabled);
  BYTE  IsHd720At60Enabled() const;
  void  SetHd1080At60Enabled(BYTE bIsHd1080At60Enabled);
  BYTE  IsHd1080At60Enabled() const;
  eVideoPartyType GetCPResourceVideoPartyType();
  //void  SetScmMpi(WORD protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi,WORD IsAutoVidScm);
  //Testing
  void TestComMode(BYTE* caps, DWORD num);
  void Dump (ostringstream& str)const;


  // Attributes

  CAudMode          m_audMode;
  CVidMode          m_vidMode;
  CXferMode         m_xferMode;
  CHsdHmlpMode      m_hsdHmlpMode;
  CLsdMlpMode       m_lsdMlpMode;
  COtherMode        m_otherMode;
  CNonStandardMode  m_nsMode;
  CContentMode      m_contentMode;

  CH221strComDrv    m_H221string;

  CECSMode			m_ECSMode;
  BYTE				m_bShouldDisconnectOnEncryptionFailure; // 7.0.3 encryption mode change
  BYTE				m_bIsHd720Enabled;
  BYTE				m_bIsHd1080Enabled;
  BYTE				m_bIsHd720At60Enabled;
  BYTE				m_bIsHd1080At60Enabled;
};


#endif /* _H320COMMODE_H  */




