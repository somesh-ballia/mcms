//+========================================================================+
//                            H264VideoMode.H                                   |
//            Copyright 2006 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ResRsrcCalculator.H                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |27/10/08    |  Calculating the reservation resources type
//     |    IMPORTANT: this file must be sync with CH264VideoMode class
//+========================================================================+


#ifndef RESRSRCCALCULATOR_H_
#define RESRSRCCALCULATOR_H_

#include "AllocateStructs.h"
#include "ConfPartySharedDefines.h"
#include "CommResApi.h"
#include "CardsStructs.h"

#define MAX_NUM_SLIDER_RATE    5

// CIF30, CIF60, SD60, HD720p60, HD1080p60
#define MAX_NUM_SLIDER_RATE_MOTION    5

// CIF30, SD30, HD720p30, HD1080p30
#define MAX_NUM_SLIDER_RATE_SHARPNESS    5

#define RESOLUTION_SLIDER_CONFIG_FILE_NAME	"Cfg/ResolutionSlider.xml"
#define RMX1500Q_RESOURCE_SLIDER_CONFIG_STEPS		8

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	DWORD              thresholdBitrate;
	Eh264VideoModeType videoModeType;
} H264VideoModeThresholdStruct;

typedef struct
{
	Eh264VideoModeType videoModeType;
	EThresholdBitRate  balanced_bitrate;
	EThresholdBitRate  resource_optimized_bitrate;
	EThresholdBitRate  user_optimized_bitrate;
	EThresholdBitRate  hi_profile_optimized_bitrate;

} ResolutionThresholdStruct;
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	BYTE resTransferRate;
	eVideoPartyType sharpnessVideoPartyType ;
	eVideoPartyType motionVideoPartyType ;
} ResRateRcrsVidTypeStruct;

///////////////////////////////////////////////////////////////////////////////////////////////////

class CResolutionSliderDetails;

class CResRsrcCalculator : public CPObject
{
	CLASS_TYPE_1(CResRsrcCalculator, CPObject)
  public:

    // Constructors
	CResRsrcCalculator ();
    virtual ~CResRsrcCalculator(){}


    virtual const char* NameOf() const {return "CResRsrcCalculator";}
    eVideoPartyType GetRsrcVideoType(eSystemCardsMode systemCardsBasedMode, CCommResApi* pRsrv);
    eVideoPartyType GetRsrcVideoType(eSystemCardsMode systemCardsBasedMode, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode);

    eVideoPartyType GetMaxRsrcTypeAccordingToSysFlag(eSystemCardsMode systemCardsBasedMode);

    static DWORD GetRateAccordingToVideoModeType(eSystemCardsMode systemCardsBasedMode, eVideoQuality videoQuality,Eh264VideoModeType H264VideoMode);
    static Eh264VideoModeType GetVideoMode( eSystemCardsMode systemCardsBasedMode, DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType maxVidMode, BOOL isHighProfile = TRUE);
    static eVideoPartyType TranslateVideoTypeToResourceType ( eSystemCardsMode systemCardsBasedMode, Eh264VideoModeType vidType);
    static Eh264VideoModeType GetMaxConfResolution( eSystemCardsMode systemCardsBasedMode, CCommResApi* pRsrv);
    static Eh264VideoModeType GetMaxCPResolution( eSystemCardsMode systemCardsBasedMode );
    static EVideoResolutionType GetMaxCPResolutionType( eSystemCardsMode systemCardsBasedMode );

    static void SetHDenabled( BOOL en ) { m_isHD_enabled = en;if(m_CPLicensing_CIF_Plus){ m_CPmaxRes = en ? e_hd1080p60 : e_sd30;} }
    static BOOL IsHDenabled() { return m_isHD_enabled; }

    static void SetHD1080enabled( BOOL en ) { m_isHD1080_enabled = en; if(m_CPLicensing_CIF_Plus){ m_CPmaxRes = en ? e_hd1080p60 : e_hd720p30;} }
    static BOOL IsHD1080enabled() { return m_isHD1080_enabled; }

    static void SetRMX1500Q( BOOL bIs1500Q ) { m_isRMX1500Q = bIs1500Q; }
    static void SetRMX1500QRatios( BOOL bIs1500QRatios ) { m_isRMX1500QRatios = bIs1500QRatios; }
    static BOOL IsRMX1500Q() { return m_isRMX1500Q; }
    static BOOL IsRMX1500QRatios() { return m_isRMX1500QRatios; }
	
	static EResolutionType GetResolutionType() {return m_CPmaxRes;}
	static EResolutionConfigType GetResolutionConfigType() {return m_ConfigType;}
	static DWORD GetHDBitRateThrshld( eSystemCardsMode systemCardsBasedMode );
	//FSN-613: Dynamic Content for SVC/Mix Conf
	static DWORD GetRateThrshldBasedOnVideoModeType(eSystemCardsMode systemCardsBasedMode,  eVideoQuality videoQuality, BOOL isHighProfile, Eh264VideoModeType specificVideoModeType);

    BOOL    UpdateResolutionConfiguration( eSystemCardsMode systemCardsBasedMode, CResolutionSliderDetails* pSliderDetails);
    STATUS  ReadResolutionConfigurationFromFile(eSystemCardsMode systemCardsBasedMode, BOOL* isWasChanged = NULL);
    STATUS  SaveResolutionConfigurationToFile(eSystemCardsMode systemCardsBasedMode, BOOL useSysFlag = FALSE);

    void GetResolutionSliderDetails( eSystemCardsMode systemCardsBasedMode, CResolutionSliderDetails* pResponse)const;

	static ResolutionThresholdStruct g_defaultMotionThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION];
	static ResolutionThresholdStruct g_defaultSharpnessThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS];
	static ResolutionThresholdStruct g_defaultHighProfileMotionThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION];
	static ResolutionThresholdStruct g_defaultHighProfileSharpnessThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS];

    void DumpDynamicThresholdTbl(eSystemCardsMode systemCardsBasedMode);
    void DumpDefaultThresholdTbl(eSystemCardsMode systemCardsBasedMode);

    static WORD GetMaxNumSliderRates(bool isMotion);
	STATUS SetResolutionAccordingToLicensing(BOOL bCPLicensing_CIF_Plus,eSystemCardsMode systemCardMode);
	void SetLicensingCifPlus(BOOL bCPLicensing_CIF_Plus) {m_CPLicensing_CIF_Plus = bCPLicensing_CIF_Plus;}
	BOOL GetLicensingCifPlus(){return m_CPLicensing_CIF_Plus;}

  private:


    static H264VideoModeThresholdStruct g_dynamicMotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION];
	static H264VideoModeThresholdStruct g_dynamicSharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS];
    static H264VideoModeThresholdStruct g_dynamicHighProfileMotionThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION];
	static H264VideoModeThresholdStruct g_dynamicHighProfileSharpnessThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS];

	static H264VideoModeThresholdStruct g_CIF_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];
	static H264VideoModeThresholdStruct g_CIF_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];

	static H264VideoModeThresholdStruct g_SD_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];
	static H264VideoModeThresholdStruct g_SD_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];

	static H264VideoModeThresholdStruct g_HD720_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];
	static H264VideoModeThresholdStruct g_HD720_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];

	static H264VideoModeThresholdStruct g_HD1080_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];
	static H264VideoModeThresholdStruct g_HD1080_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];

	static H264VideoModeThresholdStruct g_HD1080_60_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];
	static H264VideoModeThresholdStruct g_HD1080_60_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES];

	static EResolutionType       m_CPmaxRes;
	static EResolutionConfigType m_ConfigType;
	static BOOL 				 m_isHD_enabled;
	static BOOL					 m_isRMX1500QRatios;
	static BOOL					 m_isRMX1500Q;
	static BOOL					 m_isHD1080_enabled;
	static EThresholdBitRate 		m_CPmaxLineRate;
	static BOOL m_CPLicensing_CIF_Plus;

	void DumpDefaultThresholdTblByInd(DWORD i, ostringstream& str);
	void DumpDynamicThresholdTblByInd(DWORD i, ostringstream& str);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CResolutionSliderDetails : public CSerializeObject
{
	CLASS_TYPE_1(CResolutionSliderDetails,CSerializeObject)

	enum ResolutionSliderMode {	eFirst = 1,	eSecond, eThird, eFourth };

	friend class CResRsrcCalculator;

public:
	CResolutionSliderDetails();
	CResolutionSliderDetails(eSystemCardsMode cardMode);

	virtual ~CResolutionSliderDetails();
    const char * NameOf() const {return "CResolutionSliderDetails";}

    DWORD GetMaxCPResolution() {return m_CPmaxRes;}

    //CSerializeObject overrides
    virtual CSerializeObject* Clone() { return new CResolutionSliderDetails(m_cardMode); }
    virtual void SerializeXml(CXMLDOMElement*& thisNode ) const;
    virtual int	 DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);

    EResolutionType GetMaxResForCardType() const;

protected:

    void Init( eSystemCardsMode systemCardsBasedMode);

  DWORD m_SD_MotionMinRate; // CIF 60
  DWORD m_HD720_MotionMinRate;//SD 60
  DWORD m_HD1080_MotionMinRate; //HD720 60
  DWORD m_HD1080_60_MotionMinRate; //HD1080 60


  DWORD m_SD_SharpnessMinRate; // SD 30
  DWORD m_HD720_SharpnessMinRate; // HD720
  DWORD m_HD1080_SharpnessMinRate; // HD1080
  DWORD m_HD1080_60_SharpnessMinRate; // HD1080 60

  DWORD m_SD_MotionMinRateHighProfile;
  DWORD m_HD720_MotionMinRateHighProfile;
  DWORD m_HD1080_MotionMinRateHighProfile;
  DWORD m_HD1080_60_MotionMinRateHighProfile;

    DWORD m_SD_SharpnessMinRateHighProfile;
    DWORD m_HD720_SharpnessMinRateHighProfile;
    DWORD m_HD1080_SharpnessMinRateHighProfile;
    DWORD m_HD1080_60_SharpnessMinRateHighProfile;

    DWORD m_CPmaxRes;
    DWORD m_ConfigType;
    DWORD m_CPmaxLineRate;	

    eSystemCardsMode m_cardMode;
};

class CSetResolutionSliderDetails : public CResolutionSliderDetails
{
	CLASS_TYPE_1(CSetResolutionSliderDetails,CResolutionSliderDetails )

	friend class CResRsrcCalculator;

public:
	CSetResolutionSliderDetails();
	CSetResolutionSliderDetails(eSystemCardsMode cardMode);

    const char * NameOf() const {return "CSetResolutionSliderDetails";}

    //CSerializeObject overrides
    virtual CSerializeObject* Clone() { return new CSetResolutionSliderDetails(m_cardMode); }
    virtual void SerializeXml(CXMLDOMElement*& thisNode ) const;
    virtual int	 DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);
};


#endif /* RESRSRCCALCULATOR_H_ */
