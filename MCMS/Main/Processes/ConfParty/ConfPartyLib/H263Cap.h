//+========================================================================+
//                            H263Cap.H                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H263Cap.H                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 10.10.07   |                                                      |
//+========================================================================+


#ifndef _H263VIDEOCAP_H
#define _H263VIDEOCAP_H

#include <set>
#include "PObject.h"
#include "AllocateStructs.h"

class CSegment;
class CObjString;


/* This is capability description for H263 video coding standard */


class CCapSetH263 : public CPObject
{
CLASS_TYPE_1(CCapSetH263,CPObject )
public:

// Constructors
	CCapSetH263();
	virtual ~CCapSetH263();
	// For quad
	CCapSetH263(BYTE vidFormaty);

// Initializations
	void  CreateDefault();
	void  Create( BYTE capBuffer[] );

// Operations
	virtual const char*  NameOf() const;
	void Dump(void) const;
	void Dump(std::ostream& ostr) const;
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	CCapSetH263& operator=(const CCapSetH263 &other);
	WORD operator<(const CCapSetH263& CapSetH263) const;
    WORD operator==(const CCapSetH263& CapSetH263) const;

	void SetBaseCapsH263(BYTE base);
	void SetOptionalCaps(BYTE optional);
	void SetHRDbAndBPPmaxKb(BYTE newHRDbAndBPPmaxKb);
	void SetVidImageFormat(BYTE base);
	void SetVidImageFormatValue(WORD newFormatValue);
	void SetMPI(BYTE mpi);
	void SetMpiFromBaseByte(BYTE base);
	void SetUMV(BYTE bitValue);
	void SetAMP(BYTE bitValue);
	void SetAC(BYTE bitValue);
	void SetPB(BYTE bitValue);
	void SetHRDb(BYTE newHRDb);
	void SetBPPmaxKb(BYTE newBPPmaxKb);
	void SetAdditionalH263Cap(BYTE AdditionalH263Cap);
	void SetIndividualOptionIndicator(BYTE IndividualOptionIndicator);
    void SetOption1Cap(BYTE SetOption1Cap);
    void SetOption2Cap(BYTE SetOption2Cap);
	void SetOption3Cap(BYTE SetOption3Cap);
	void SetRefSliceParameters(BYTE RefSliceParameters);
    void SetScalabilityDescriptor(BYTE ScalabilityDescriptor);
	void SetEnhancementLayerInfo(BYTE Index,BYTE Cap);
	void SetVideoFormatResolution(BYTE MinFrameWidth,
								  BYTE MinFrameHeight,
								  BYTE MaxFrameHeight = 0,
								  BYTE MaxFrameWidth = 0);
	void SetCustomPCF(BYTE CustomPCF_1,BYTE CustomPCF_2);
	void SetHRDBPPMaxKB(BYTE HRDBPPMaxKB);
	void SetCustomPixelWidthAndHeight(BYTE CustomPixelWidth,BYTE CustomPixelHeight);
	void SetFormatIndicator(BYTE FormatIndicator);
    void SetAnnexP(BYTE OnOff);
    void SetAnnexN(BYTE OnOff);
	void SetAnnexL(BYTE OnOff);
	void SetAnnexT(BYTE OnOff);
	void SetClockDivisor(BYTE bitValue);
    void SetClockConversionCode(BYTE bitValue);
	void SetCustomMPIIndicator(BYTE bitValue) ;
    void SetOptionsIndicator(BYTE OptionsIndicator);
	void SetVidImageToCustomFormat();
	void SetSecondAdditionalH263Cap(BYTE SecondAdditionalH263Cap);
	void SetInterlaceMode(BYTE InterlaceMode);
	void SetEnhancedReferencePicSelect(BYTE EnhancedReferencePicSelect);
	WORD friend IsSameAdditionalCap(const CCapSetH263& CapSetH263,
									const CCapSetH263& RemoteLastFormatExplicitlyDeclared);
    WORD IsComprisesCustomFormat(const CCapSetH263* CapSetH263) const;
	BYTE IsCustomTwoDistinctBoundsVidImageFormat(BYTE format) const;


	BYTE GetSetLen() const;
	BYTE GetBaseCapsH263() const;
	BYTE GetVidImageFormat() const;
	BYTE GetCustomVidImageFormat() const;
	BYTE GetCustomVidImageFormatWidthAndHight(BYTE& MinCustomPictureWidth,
											  BYTE& MaxCustomPictureWidth,
											  BYTE& MinCustomPictureHeight,
											  BYTE& MaxCustomPictureHeight) const;
	BYTE GetMPI()const;
	BYTE IsCPM()const;
	BYTE IsUMV()const;
	BYTE IsAMP()const;
	BYTE IsAC()const;
	BYTE IsPB()const;
	BYTE IsOptionalCap()const;
	BYTE IsSpecifyHRD_B()const;
	BYTE IsSpecifyBPPmaxKb()const;
    BYTE IsRefSliceParameters()const;
	BYTE GetHRDb()const;
	BYTE GetBPPmaxKb()const;
	BYTE GetOptionalCap()const;
	BYTE GetHRDbAndBPPmaxKb()const;
	BYTE GetFormatIndicator()const;
    BYTE IsCustomPCFFlag()const;
	BYTE IsHRDBPPMaxKB()const;
	BYTE IsCustomPARFlag()const;
	BYTE GetOptionsIndicator()const;
	BYTE IsOptions_1_FlagOn()const;
	BYTE IsOptions_2_FlagOn()const;
	BYTE IsOptions_3_FlagOn()const;
	BYTE IsScalabilityDescriptor_FlagOn()const;
	BYTE IsErrorCompensation_FlagOn()const;
	BYTE GetNumberOfScalableLayers()const;
	BYTE GetEnhancementLayerInfo(BYTE Index)const;
	BYTE GetMinCustomPictureHeight()const;
    BYTE GetMinCustomPictureWidth()const;
	BYTE GetClockDivisor()const;
	BYTE GetClockConversionCode()const;
	BYTE GetCustomMPIIndicator()const;
	BYTE GetSecondAdditionalH263Cap()const;
	BYTE GetEnhancedReferencePicSelect()const;
	BYTE GetInterlaceMode()const;
	WORD CalculateFramePerSecond() const;
	BYTE IsAnnex_P()const;
	BYTE IsAnnex_L()const;
	BYTE IsAnnex_N()const;
	BYTE IsAnnex_T()const;
	void HcDump(CObjString* ostr = NULL,WORD local_dump = YES) const;

	BYTE IsCapSetH263Valid()const;

private:

	// Attributes
  BYTE      m_vidImageFormat; // value of H263_QCIF_SQCIF / H263_CIF / H263_CIF_4 / H263_CIF_16 or H263_CUSTOM_FORMAT.

  //Initial BYTES - relevant only for STANDARD Formats QCIF,CIF,4CIF and 16CIF.
  BYTE      m_mpi;
  BYTE		m_optionalCap;
  BYTE		m_HRDbAndBPPmaxKb;

  //  Additional BYTES - relevant for :
  // (1) Standard Formats QCIF,CIF,4CIF and 16CIF.
  // (2) Custom Formats and NTSC.
  BYTE     m_AdditionalH263Cap; // (1)+(2)

  // Only for (2)
  BYTE     m_MinCustomPictureHeight;
  BYTE     m_MinCustomPictureWidth;
  BYTE     m_MaxCustomPictureHeight;
  BYTE     m_MaxCustomPictureWidth;
  BYTE     m_CustomPCF_1;
  BYTE     m_CustomPCF_2;
  BYTE     m_HRDBPPMaxKB;
  BYTE     m_CustomPixelWidth;
  BYTE     m_CustomPixelHeight;

  // (1)+(2)
  BYTE     m_IndividualOptionIndicator;
  BYTE     m_Option_1;
  BYTE     m_Option_2;
  BYTE     m_Option_3;
  BYTE     m_RefSliceParameters;
  BYTE     m_ScalabilityDescriptor;
  BYTE*    m_EnhancementLayerInfo;

  //Second additional byte
  BYTE     m_SecondAdditionalH263Cap;
};

//----------------------------------------------------------------------------
/* CompareBy... is comparison class: A class that takes two arguments
   of the same CCapSetH263 type (container's elements) and returns true if p1 less then p2 */
struct CompareByH263
{
  bool operator()(const CCapSetH263* p1, const CCapSetH263* p2) const
  {
    return (*p1) < (*p2);
  }
};
//----------------------------------------------------------------------------

class CCapH263 : public CPObject
{
CLASS_TYPE_1(CCapH263,CPObject )
public:

// Constructors
	CCapH263();
	CCapH263(const CCapH263&);
	virtual ~CCapH263();

// Initializations
	void  CreateDefault();
	void  Create(CSegment& seg,BYTE mbeLen);
	void  CreateAdditionalCapabilities(CSegment& seg,
									   BYTE AdditionalCapLength,
									   BYTE& IsErrorEndOfSegment);
	void  CreateSecondAdditionalCapabilities(CSegment& seg,
											 BYTE SecondAdditionalCapLength,
											 BYTE& IsErrorEndOfSegment);
// Operations
	virtual const char*  NameOf() const;
	void Dump(void) const;;
	void Dump(std::ostream& ostr) const;
	void SmartDump(std::ostream& ostr) const;
	void DumpCapH263(std::ostream& ostr) const;
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	CCapH263& operator=(const CCapH263 &other);
	WORD operator<=(const CCapH263 &other) const;
	CCapSetH263* GetStandardFomatAccordingToInitialOrder(BYTE IndexOfStandardH263Set) const;
	BYTE InsertH263CapSet(CCapSetH263* pH263CapSetBuf);
/* 	CCapSetH263* GetH263CapSet(BYTE Index) const; */
	BYTE IsAdditionalCapOrCustomFormats() const;
	BYTE IsSecondAdditionalCap() const;
	BYTE IsInterlaceCap() const;
	BYTE IsInterlaceCap(WORD format) const;
	BYTE IsCustomFormatsOrAnnexes() const;
	void SetHighestCommonVideoCapabilities(CCapH263* pCapH263,
										   WORD isAutoVidScm,
										   WORD& is_highest_common_param_changed);
	void SetCommonCapabilities(CCapSetH263* pLocalCapSetH263,CCapSetH263* pRemoteCapSetH263,
							   CCapH263* pCapH263,CCapSetH263& pNewCommonCapSetH263,
							   WORD isAutoVidScm,
							   WORD& is_highest_common_param_changed);
	void SetOneH263Cap(BYTE format, int mpi);
	void PrintH263CapSeg(CSegment& seg,BYTE Length) const;
    WORD GetNumberOfH263Sets(void) const ;
	CCapSetH263* GetCapSetH263(WORD setNumber) const;
	void GetFormatCapSet(WORD format, CCapSetH263* capSetH263) const;
	BYTE GetFormatMPI(WORD format) const;
	BYTE GetCustomFormatMPI(BYTE format) const;
	BYTE GetHighestFormat() const;
	BYTE OnVidImageFormat(WORD format) const;
	BYTE IsVidImageFormat(WORD format) const;
	BYTE IsCustomVidImageFormat(BYTE format) const;
	WORD IsSupportErrorCompensation() const;
	void RemoveDifferentAnnexes();
	void SendSecondAdditionalCap(WORD OnOff);
	BYTE IsExtensionCode(BYTE cap) const;

	void Remove();
	CCapSetH263* Find(CCapSetH263* pCapSetH263) const;
	CCapSetH263* FindEqualOrLargerResolution(const CCapSetH263* pCapSetH263) const;
	CCapSetH263* FindEquivalentResolution(const CCapSetH263* pCapSetH263) const;
	CCapSetH263* FindEqualOrComprisedCustomFormat(const CCapSetH263* pCapSetH263) const;

	BYTE GetMaxResolution()const;
	void ClearAndDestroy();
/* 	WORD GetFormatHcAnnexes(WORD format,HC_ANNEXES_ST& st_annexes) const; */
/* 	void Intersect(CCapH263& rRemoteCapH263,WORD& is_highest_common_param_changed); */
/* 	CCapSetH263* at(size_t index) const; */
/*  size_t Index(CCapSetH263* pCapSetH263) const; */
	void  SetVswAutoFullCap(BYTE is_custom_formats, BYTE is_interlaced);
	void  SetVswAutoCaps(const BYTE* formats_array);

	void RemoveAnnexesForDBC2();
	void SetCapH263_2000();
	BOOL IsH263VideoCap(WORD cap) const;
	//BYTE IsCapableOfSD15() const;
	eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities(BYTE isH2634Cif15Supported);


protected:
	void IntersectCapSet(CCapSetH263& rNewCommonCapSetH263,
						 WORD& is_highest_common_param_changed,
						 CCapSetH263& rLocalCurrentCapSet,
						 CCapSetH263& rRemoteEquivalentCapSet,
						 CCapSetH263* pLocalAdditionalCapabilitiesCapSet = NULL,
						 CCapSetH263* pLocalStandardFormatAdditionalCapabilitiesCapSet = NULL,
						 CCapSetH263* pLocalSecondAdditionalCapabilitiesCapSet = NULL,
						 CCapSetH263* pRemoteAdditionalCapabilitiesCapSet = NULL,
						 CCapSetH263* pRemoteSecondAdditionalCapabilitiesCapSet = NULL,
						 CCapSetH263* pRemoteFirstStandardFormatCapSet = NULL,
						 CCapSetH263* pRemoteStandardFormatAdditionalCapabilitiesCapSet = NULL);
	void IntersectCapSetStandardFormatParameters(CCapSetH263& rNewCommonCapSet,
												 WORD& is_highest_common_param_changed,
												 CCapSetH263& rLocalCurrentCapSet,
												 CCapSetH263* pLocalStandardFormatAdditionalCapabilitiesCapSet = NULL,
												 CCapSetH263* pRemoteFirstStandardFormatCapSet = NULL,
												 CCapSetH263* pRemoteStandardFormatAdditionalCapabilitiesCapSet = NULL);
	void IntersectCapSetCustomParameters(CCapSetH263& rNewCommonCapSet,
										 WORD& is_highest_common_param_changed,
										 CCapSetH263& rLocalCurrentCapSet,
										 CCapSetH263& rRemoteEquivalentCapSet);
	void IntersectCapSetAdditionalCapabilities(CCapSetH263& rNewCommonCapSet,
											   WORD& is_highest_common_param_changed,
											   CCapSetH263& rLocalCurrentCapSet,
											   CCapSetH263* pLocalAdditionalCapabilitiesCapSet = NULL,
											   CCapSetH263* pLocalSecondAdditionalCapabilitiesCapSet = NULL,
											   CCapSetH263* pRemoteAdditionalCapabilitiesCapSet = NULL,
											   CCapSetH263* pRemoteSecondAdditionalCapabilitiesCapSet = NULL);
private:

	// Operations
	BYTE OnOptions(BYTE cap) const;
	BYTE OnSpecifyHRDb(BYTE cap) const;
	BYTE OnSpecifyBPPmaxKb(BYTE cap) const;

	// Attributes
	WORD m_numberOfH263Sets;
	WORD m_IsSendSecondAdditionalCap;

	BYTE m_H263_2000;

    std::multiset<CCapSetH263*, CompareByH263>*   m_capH263;
};

#endif /* _H263VIDEOCAPS_H */




