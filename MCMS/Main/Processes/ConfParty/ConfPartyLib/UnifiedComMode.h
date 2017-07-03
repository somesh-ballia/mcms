//+========================================================================+
//                            UnifiedComMode.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: ThIs software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and Is protected by law.        |
// It may not be copied or dIstributed in any form or medium, dIsclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UnifiedSCM.H                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Romem                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  July 2005  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _CUnifiedComMode_H_
#define _CUnifiedComMode_H_

#include "IpScm.h"
#include "CopVideoTxModes.h"
#include "ConfPartyApiDefines.h"

class CPObject;
class CComMode;
class CCopVideoTxModes;

typedef struct
{
	char  AmcRate;
	char  AmscRate;
} TH239ToEPC;

typedef map<BYTE, map<eEnterpriseMode, map<eCascadeOptimizeResolutionEnum, BYTE> > >  ContentRateMap;
typedef map<BYTE, eCascadeOptimizeResolutionEnum> Rate2ResolutionMap; // stores the MAXIMAL Resolution per CONTENT Rate

class CUnifiedComMode : public CPObject
{
	CLASS_TYPE_1(CUnifiedComMode ,CPObject)
public:
	static const TH239ToEPC g_239ToEPCTbl[MAX_H239_CAPS];
	CUnifiedComMode (BYTE amcCustomizedContentRate,BYTE lConfRate, BOOL bIsHighProfileContent = FALSE);
	virtual ~CUnifiedComMode ();
	virtual const char* NameOf() const { return "CUnifiedComMode";}
	CUnifiedComMode(const CUnifiedComMode& rOtherUnifiedSCM);
	CUnifiedComMode& operator =(const CUnifiedComMode& rOtherUnifiedSCM);

	// general flags
	void SetConfType(WORD confType);
	WORD GetConfType();
	void  SetIsAutoVidRes( BYTE isAutoVidRes);
	WORD  IsAutoVidRes();
	void  SetIsAutoVidProtocol( BYTE isAutoVidProtocol);
	WORD  IsAutoVidProtocol();
	// call rate
	void SetCallRate(BYTE reservationRate);
	DWORD GetCallRate() const;
	// video params
	// direction - 0 in and out (as today - default)
	//           -  1 out
	//            -  2 in
	void SetH264VidMode(WORD h264Profile, BYTE h264Level, long maxBR, long maxMBPS,
		long maxFS, long maxDPB, long maxSAR, long maxStaticMB, WORD isAutoVidScm /*= 0*/, WORD direction /*= cmCapReceiveAndTransmit*/);
	void SetH264VidMode(H264VideoModeDetails h264VidModeDetails, long maxSAR, WORD isAutoVidScm = 0, WORD direction = cmCapReceiveAndTransmit);
	void SetVP8VidMode(VP8VideoModeDetails vp8VideoDetails, WORD isAutoVidScm, WORD direction = cmCapReceiveAndTransmit); //N.A. DEBUG VP8
	void SetProtocolClassicVidMode(WORD protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi,WORD IsAutoVidScm = 0, WORD direction = cmCapReceiveAndTransmit);// H.263/H.261
	void SetH263ScmPlus(BYTE bAnnexP, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
		char vga, char ntsc, char svga, char xga, char qntsc, WORD direction = cmCapReceiveAndTransmit); // H.263 Annexes
	///////////////////////////////////////////////////////////////////////////
	void GetH264VidMode(WORD& h264Profile, BYTE& h264Level, long& maxBR, long& maxMBPS,
		long& maxFS, long& maxDPB, long& maxSAR, long& maxStaticMB, WORD& isAutoVidScm, WORD direction = cmCapReceiveAndTransmit) const;

	void GetProtocolClassicVidMode(WORD protocol, int& qcifMpi, int& cifMpi, int& cif4Mpi, int& cif16Mpi,WORD& IsAutoVidScm, WORD direction = cmCapReceiveAndTransmit) const;// H.263/H.261
	void GetH263ScmPlus(BYTE& bAnnexP, BYTE& bAnnexT, BYTE& bAnnexN, BYTE& bAnnexI_NS,
		char& vga, char& ntsc, char& svga, char& xga, char& qntsc) const; // H.263 Annexes
	void SetVideoOff(WORD direction);
	// audio params
	void  SetAudModeAndAudBitRate(WORD audMode, WORD direction = cmCapReceiveAndTransmit);
	WORD  GetAudMode(WORD direction = cmCapReceiveAndTransmit) const;
	WORD  GetAudBitRate() const;
	// encryption
	void SetEncrypMode(WORD mode, BOOL bShouldDisconnectOnEncryptFailure = TRUE); // if Encryp_On call CreateLocalComModeECS() of the internal object
	WORD GetEncrypMode() const;
	// restrict mode // TBD when IsDN comes in
	//
	//FECC
	void SetFECCMode(WORD mode,WORD direction = cmCapReceiveAndTransmit);
	void SetFECCcap(WORD cap,WORD direction = cmCapReceiveAndTransmit);
	WORD GetFECCMode(WORD direction = cmCapReceiveAndTransmit) const;
	WORD GetFECCcap(WORD direction = cmCapReceiveAndTransmit) const;
	// Opened channels
	WORD IsMediaOn(WORD mediaType,WORD direction = cmCapReceiveAndTransmit) const;
	DWORD GetMediaBitRate(cmCapDataType type, cmCapDirection direction,ERoleLabel eRole) const;

	CIpComMode* GetIPComMode(){return m_pIPScm;}
	CComMode*   GetIsdnComMode(){return m_pIsdnScm;}
	CCopVideoTxModes* GetCopVideoTxModes() const;
	void SetCopVideoTxModes(CCopVideoTxModes* pCopVideoTxModes);

	//Content
	CapEnum TranslateConfProtocolToH323Protocol(BYTE Protocol);
	void SetContentMode(BYTE XCallRate,eEnterpriseMode ContRatelevel,cmCapDirection eDirection, ePresentationProtocol ContProtocol, eCascadeOptimizeResolutionEnum resolutionLevel, BYTE bContentAsVideo,
	                    eConfMediaType aConfMediaType, BOOL isHD1080 = FALSE, BYTE HDMpi = 0);//Set the caps with content rate
	void SetContentTIPMode(eEnterpriseMode ContRatelevel, cmCapDirection eDirection, BYTE set264ModeAsTipContent = TRUE);
	BYTE GetContentModeAMC(BYTE lConfRate,eEnterpriseMode ContRatelevel, ePresentationProtocol contentProtocolMode, eCascadeOptimizeResolutionEnum resolutionLevel, eConfMediaType aConfMediaType);
	DWORD GetContentModeAMCInIPRate(BYTE Xfer_XlConfRate,eEnterpriseMode ContRatelevel, ePresentationProtocol contentProtocolMode, eCascadeOptimizeResolutionEnum resolutionLevel, eConfMediaType aConfMediaType);
	void SetNewContentBitRate(BYTE newAMCBitRate,cmCapDirection eDirection = cmCapReceiveAndTransmit); //Set the current content rate with a new rate
	BYTE GetCurrentContentBitRateAMC();//Get the Current content rate in AMC units
	void SetNewContentProtocol(BYTE newProtocol,cmCapDirection eDirection,BOOL isHD1080 = FALSE, BYTE HDContentMpi = 0);
	BYTE GetCurrentContentProtocolInIsdnValues();
	CapEnum GetCurrentContentProtocolInIpValues();
	//BYTE FindAMCContentRateByLevel(BYTE Xfer_XCallRate,eEnterpriseMode ContentRateLevel = eGraphics );

	BYTE FindAMCContentRateByLevel(BYTE Xfer_XCallRate,
	    eConfMediaType aConfMediaType,
		eEnterpriseMode ContentRateLevel = eGraphics,
		ePresentationProtocol contentProtocolMode = eH264Fix,
		eCascadeOptimizeResolutionEnum resolutionLevel = e_res_1080_15fps/*e_res_dummy*/);

	//HP content:
	void	SetIsHighProfileContent(const BYTE isHighProfileContent);
	BYTE	GetIsHighProfileContent() const;

	static BYTE TranslateRateToAMCRate(DWORD h323Rate);
	static DWORD TranslateAMCRateIPRate(BYTE AMCRate);
	static DWORD TranslateXferRateToIpRate(BYTE Xfer_XCallRate);
	static BYTE TranslateIPRateToXferRate(DWORD H323CallRate);
	static BYTE TranslateIPRateToXferRateForCss(DWORD H323CallRate);
	static BYTE TranslateAmcRateToAmscRate(BYTE AMCRate);
	static BYTE TranslateAmscRateToAmcRate(BYTE AMSCRate);
	static DWORD TranslateAmscRateToIPRate(BYTE AMSCRate);
	static BYTE TranslateXferRateToAmcRate(BYTE xferRate);
	static BYTE TranslateAmcRateToXferRate(BYTE AMCRate);
	static BYTE MapXferRateForDynamicContentRateTable(BYTE xferRate);


	BYTE   isHDContent1080Supported(cmCapDirection direction) const;
	BYTE   isHDContent720Supported(cmCapDirection direction) const;
	DWORD  FindIpContentRateByLevel(DWORD CallRate, eEnterpriseMode ContentRateLevel, ePresentationProtocol contentProtocolMode, eCascadeOptimizeResolutionEnum resolutionLevel, eConfMediaType aConfMediaType);

	void SetVideoBitRate(int newBitRate, cmCapDirection direction,ERoleLabel eRole = kRolePeople);

	// LPR
	void SetIsLpr(BYTE mode);
	BYTE GetIsLpr() const;

	void SetHdVswResolution(EHDResolution hdVswRes);

	BYTE FindContentRateInCascadeOPtimizedTable(BYTE Xfer_XCallRate, eEnterpriseMode ContentRateLevel, eCascadeOptimizeResolutionEnum resolutionLevel);

	BYTE FindAMCH264DynamicContentRateByLevel(BYTE Xfer_XCallRate, eEnterpriseMode ContentRateLevel, eCascadeOptimizeResolutionEnum resolutionLevel, eConfMediaType aConfMediaType);

	eCascadeOptimizeResolutionEnum getMaxContentResolutionbyAMCRate(BYTE AMCRate);

	void  SetOperationPointPreset(EOperationPointPreset eOPPreset);

	static void getAmcH264DynamicContentRateResTable(ContentRateMap& amcH264DynamicContentRateResTable, BOOL isHighProfile = FALSE);

	static void getMaxResolutionPerContentRateTable(Rate2ResolutionMap& maxResolutionPerContentRateValues, BOOL isHighProfile = FALSE);

	static eCascadeOptimizeResolutionEnum getMaxContentResolutionbyRateAndProfile(DWORD contentIPRate, BOOL isHighProfile = FALSE);  //HP content:
	void  SetH264PacketizationMode(APIU8  H264PacketizationMode);

private:

	static void initStaticContentTables();

	static void initAMCContentRateCascdeOptimizeResTable();

	static void initMaxResolutionPerContentRateTable();

	static void initAMCContentRateCascdeOptimizeResTable_HP();

	static void initMaxResolutionPerContentRateTable_HP();

protected:

	CIpComMode* m_pIPScm;
	CComMode*   m_pIsdnScm;

	// For Cop
	CCopVideoTxModes* m_pCopVideoTxModes;

	// For Content
	BOOL        m_isTIPContent;
	BYTE        m_amcCustomizedContentRate;
	BOOL        m_isHighProfileContent;

	static BOOL 	m_initContentTableFlag;

	static ContentRateMap m_amcH264DynamicContentRateResTable;
	static Rate2ResolutionMap m_maxResolutionPerContentRateTable;

	// For High Profile
	static ContentRateMap m_amcH264DynamicContentRateResTable_HP;
	static Rate2ResolutionMap m_maxResolutionPerContentRateTable_HP;
};
#endif
