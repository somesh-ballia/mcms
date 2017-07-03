//+========================================================================+
//                  EpSimCapSetsList.h                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimCapSetsList.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __EPSIMCAPSETSLIST_H__
#define __EPSIMCAPSETSLIST_H__


#include "PObject.h"
#include "IpChannelParams.h"
#include "Capabilities.h"
#include "SipStructures.h"
#include "H264.h"
#include "RvCommonDefs.h"
#include "CapClass.h"
#include "Capabilities.h"

class CSegment;
class CVideoCap;
class CVideoCapH263;
class CVideoCapH264;
class CVideoCapVP8; //N.A. DEBUG VP8
class CCapSet;
class CSdesEpCap;
class CVideoCapMSSvc;
class CVideoCapSVC;

#define MAX_H323_CAPS			50
#define MAX_CAP_NAME			250
#define MAX_VIDEO_PROTOCOL_NUM	6		// h261, h263, h264,rtv,svc, vp8
#define MAX_AUDIO_ALG_NUM		40		// g711, g722, etc.
#define MAX_SDES_CAP_NUM		3		// audio, video, data

#define 	DEFAULT_LIFE_TIME		0x80000000	// for SRTP

/// From files CapInfo.h / .cpp
#define FrameBasedFPP 1
#define NonFrameBasedFPP 20

// CapEnum - IpChannelParams.h

typedef enum
{
	eSQcif = 0,
	eQcif,
	eCif,
	e4Cif,
	e16Cif,
	eUnknownVideoFormat // should be last
} VIDEO_FORMAT;

typedef enum
{
	eVideoModeCif = 0,
	eVideoMode2Cif30,
	eVideoModeSD7_5,
	eVideoModeSD15,
	eVideoModeSD30,
	eVideoModeW4CIF30,
	eVideoModeHD720,
	eVideoModeHD1080,
	eVideoModeHD1080p60,
	eVideoModeSD60,
	eVideoModeCustom,
	eVideoModeUnknown // should be last
} enVideoModeH264;

#if 0
typedef enum
{
	eVideoProfileBaseline = 64,
	eVideoProfileHigh = 8,
	eVideoProfileMain = 32

}eVideoH264Profile;
#endif 

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CCapSetsList : public CPObject
{
CLASS_TYPE_1(CCapSetsList, CPObject)

public:
		// constructors
	CCapSetsList();
	virtual ~CCapSetsList();
		// overrides
	virtual const char* NameOf() const { return "CCapSetsList";}
	const CCapSet*	GetDefaultCap() const;
	void  GetFullCapListToGui(CSegment* pParam);
	void  AddCapFromGui(CSegment* pParam);
	void  DeleteCapFromGui(CSegment* pParam );
	void  UpdateCapFromGui(CSegment* pParam );
	void  HandleBatchEvent(const DWORD opcode, CSegment* pParam);

	CCapSet*	FindCapSet(const DWORD nId);
	CCapSet*	FindCapSet(const char* pszName);

protected:
	STATUS		AddCap( const CCapSet& rNewCap, char* pszStatusStr );
	STATUS		DeleteCapSet(const DWORD nId);
	STATUS		DeleteCapSet(const char* pszName);

	DWORD		GenerateID() const;

	DWORD		GetCapListLength() const;
	int			GetNextEmptyPlace() const;

	void 	    FillFullNonVideoCapset(CCapSet* rCap, enTransportType bfcpTransportType);

protected:
	DWORD		m_updateCounter;
	CCapSet*	m_paCapArray[MAX_H323_CAPS];
	DWORD		m_nDefaultCapIndex;

public:
	static BOOL m_isTIP;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CCapSet : public CPObject
{
CLASS_TYPE_1(CCapSet, CPObject)

public:
	enum TEpProtocolType { eProtocolH323 = 0, eProtocolSip };
public:
	CCapSet();
	CCapSet( const DWORD nId, const char* pszName );
	CCapSet( const CCapSet& other );
	CCapSet( const CCapSet& first, const CCapSet& second ,BOOL isTIP = FALSE);
	virtual ~CCapSet();
	virtual const char* NameOf() const { return "CCapSet";}

		// overrides

	CCapSet&  operator=(const CCapSet& other) ;

	const char*	GetName() const { return m_szName; }
	void	SetName( const char* pszCapName );
	DWORD	GetID() const { return m_nID; }
	void	SetID( const DWORD capID );
	BOOL	IsChanged() const { return m_isChanged; }
	void	ClearChanged() { m_isChanged = FALSE; }

//	void OnStartElement( CSegment* pParam );

	void Serialize(CSegment& rSegCap) const;
	void DeSerialize(CSegment& rSegParam);

	void CreateFromSipStruct(sipMediaLinesEntrySt* pMediaLinesEntry);
	void CreateFrom323Struct(ctCapabilitiesStruct* pCapStruct);
	WORD CreateCapStructH323(BYTE** ppIndication, const cmCapDirection eDirection, const BOOL bIsAnnexQ = FALSE) const;
	void FillCapMatrix( cap_fd_set& matrix, const BYTE numOfAudioCap,
			const BYTE numOfVideoCap, const BYTE numOfContentVideoCap, const BYTE numOfH239Cap,
			const BYTE numOfFeccCap, const BYTE numOfEncrypCap,
			const BYTE numOfLPRCap) const;
	WORD CreateCapStructSIP(BYTE** ppCapStruct, const BOOL isFirstCap, const cmCapDirection eAudioDirection, const cmCapDirection eVideoDirection, const cmCapDirection eGeneralDirection) const;
	BOOL AddSdesCapToMediaLine(cmCapDataType eMediaType, sipMediaLineSt *pMediaLine, int &capPos) const;

	void	Empty();
	void	EmptyAudio();
	void	EmptyVideo();
	void 	EmptySdes();
	void	EmptyFecc() { SetFecc(FALSE); }
	void	EmptyH239();
	BOOL	SetRate(const WORD rate);
	void	SetFecc(const BOOL fecc);
	void	SetEncryption(const BOOL encry);
	void	SetH239Rate(const WORD nRate);
	WORD	GetH239Rate() const { return m_nH239Rate; }
	BOOL	AddAudioAlg( const DWORD alg );
	BOOL	AddVideoProtocol( const CVideoCap& rVideoCap );
	BOOL	AddPresentationVideo( const CVideoCapH263& rVideoCap );
	BOOL	AddPresentationVideoH264( const CVideoCapH264& rVideoCap );

	WORD	GetCallRate() const { return m_nCallRate; }
	WORD	GetNumAudioAlg() const { return m_nAudAlgNum; }
	WORD	GetNumVideoProtocols() const { return m_nVidProtocolNum; }
	BOOL	IsFecc() const { return m_isFecc; }
	BOOL	IsEncryption() const { return m_isEncryption; }
	BOOL	IsH239() const ;
	DWORD	GetAudioAlgType( const WORD ind ) const;
	DWORD	GetVideoProtocolType( const WORD ind ) const;
	DWORD	GetPresentVideoProtocolType() const;
	const CVideoCap* GetVideoProtocol(const WORD ind) const;
	const CVideoCap* GetPresentationVideo() const;


	BOOL	IsLPR() const { return m_isLPR; }
	void	SetLPR(const BOOL lpr) { m_isLPR = lpr;}

	BOOL	IsDynamicPTReplacement() const { return m_isDPTR; }
	void	SetDynamicPTReplacement(const BOOL is_dynamicPT) { m_isDPTR = is_dynamicPT; }

	static  DWORD GetPayloadType( const DWORD algo );

	enTransportType GetBfcpTransportType(){ return m_bfcpTransType;}
	void SetBfcpTransportType(enTransportType transportType){ m_bfcpTransType = transportType;}
	BOOL IsBfcpSupported() const { return (m_bfcpTransType ==eTransportTypeUdp || m_bfcpTransType==eTransportTypeTcp || m_bfcpTransType==eTransportTypeTls);}
	BOOL IsBfcp() const;

protected:

	void	DefaultAudioSet();
	void	DefaultVideoSet();

	void 	AddSdesCaps();
	BOOL 	AddSdesCap( const CSdesEpCap& rSdesCap );
	WORD 	CreateCapStructIP(BYTE** ppIndication, const cmCapDirection eAudioDirection, const cmCapDirection eVideoDirection, const cmCapDirection eGeneralDirection,
								const TEpProtocolType protocol,	const BOOL bIsSipFirstCap, const BOOL bIsAnnexQ = FALSE) const;

protected:
	char			m_szName[MAX_CAP_NAME];
	DWORD			m_nID;	// for fast search
	BOOL			m_isChanged;

	// capabilities for endpoint
		// Call Bitrate in kB
	WORD			m_nCallRate;
		// Audio caps
	WORD			m_nAudAlgNum;
	DWORD			m_audioPayloadTypes[MAX_AUDIO_ALG_NUM];
		// Video caps
	WORD			m_nVidProtocolNum;
	CVideoCap*		m_paVideoCaps[MAX_VIDEO_PROTOCOL_NUM];
		// Fecc & encryption
	BOOL			m_isFecc;
	BOOL			m_isEncryption;
		// H.239 rate in kB
	WORD			m_nH239Rate;
	CVideoCapH264*	m_pVideoCapH264Presentation;
	CVideoCapH263*	m_pVideoCapH263Presentation;

	BOOL			m_isLPR;
	BOOL			m_isDPTR;

		// SDES caps
	WORD			m_nSdesCapNum;
	CSdesEpCap*		m_paSdesCaps[MAX_SDES_CAP_NUM];

	enTransportType m_bfcpTransType;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CVideoCap : public CPObject
{
CLASS_TYPE_1(CVideoCap, CPObject)
public:
	CVideoCap() {;}
	CVideoCap(const CVideoCap& first,const CVideoCap& second) {}
	virtual ~CVideoCap() {;}
	virtual const char* NameOf() const { return "CVideoCap";}
		// overrides

	virtual CapEnum GetPayloadType() const = 0;
	virtual CVideoCap* Clone() const = 0;

	virtual void Serialize(CSegment& rSegCap) const = 0;
	virtual void DeSerialize(CSegment& rSegParam) = 0;

		// utils
	static WORD MpiToFps(const WORD mpi);
	static WORD FpsToMpi(const WORD fps);

protected:
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CVideoCapH261 : public CVideoCap
{
CLASS_TYPE_1(CVideoCapH261, CPObject)
public:
	CVideoCapH261();
	CVideoCapH261(const h261CapStruct& tStruct);
	CVideoCapH261(const CVideoCapH261& other);
	CVideoCapH261(const CVideoCap& first,const CVideoCap& second);
	virtual ~CVideoCapH261();
	CVideoCapH261& operator =(const CVideoCapH261& other);

		// overrides
    virtual const char*  NameOf() const { return "CVideoCapH261"; }

	virtual CapEnum GetPayloadType() const { return eH261CapCode; };
	virtual CVideoCap* Clone() const;

	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);

	void FillStructH323(h261CapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;

protected:
	WORD	m_resolutions[eUnknownVideoFormat];
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CVideoCapH263 : public CVideoCap
{
CLASS_TYPE_1(CVideoCapH263, CPObject)
public:
	CVideoCapH263();
	CVideoCapH263(const h263CapStruct& tStruct);
	CVideoCapH263(const CVideoCapH263& other);
	CVideoCapH263(const CVideoCap& first,const CVideoCap& second);
	virtual ~CVideoCapH263();
	CVideoCapH263& operator =(const CVideoCapH263& other);
	virtual const char*  NameOf() const { return "CVideoCapH263"; }
		// overrides

	virtual CapEnum GetPayloadType() const { return eH263CapCode; };
	virtual CVideoCap* Clone() const;

	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);

	void FillStructH323(h263CapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;

	void CreatePresentationCap();

protected:
	WORD	m_resolutions[eUnknownVideoFormat];
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CVideoCapH264 : public CVideoCap
{
CLASS_TYPE_1(CVideoCapH264, CPObject)
public:
	CVideoCapH264();
	CVideoCapH264(const enVideoModeH264 mode,const DWORD ratio=(DWORD)-1, const DWORD staticMB=(DWORD)-1,const DWORD profile=(H264_Profile_BaseLine));
	CVideoCapH264(const CVideoCapH264& other);
	CVideoCapH264(const h264CapStruct& tStruct);
	CVideoCapH264(const CVideoCap& first,const CVideoCap& second);
	virtual ~CVideoCapH264();
	CVideoCapH264& operator =(const CVideoCapH264& other);
	virtual const char*  NameOf() const { return "CVideoCapH264"; }
		// overrides
	virtual DWORD GetProfile(){return m_profileValue;}
	virtual CapEnum GetPayloadType() const { return eH264CapCode; };
	virtual CVideoCap* Clone() const;

	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);

	void FillStructH323(h264CapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;

	void SetAspectRatio( const DWORD ratio ) { m_aspectRatio = ratio; }
	void SetStaticMB( const DWORD staticMB ) { m_staticMB = staticMB; }

protected:
	DWORD	m_levelValue;
	DWORD	m_mbps;
	DWORD	m_fs;
	DWORD	m_dpb;
	DWORD	m_brAndCpb;
	DWORD	m_staticMB;
	DWORD	m_aspectRatio;
	DWORD   m_profileValue;
	DWORD   m_maxFR;
	DWORD   m_H264mode;
};

//////////////////////////////////////////////////////////////////////////
class CVideoCapSVC : public CVideoCapH264
{
CLASS_TYPE_1(CVideoCapSVC, CPObject)
public:
CVideoCapSVC();
    CVideoCapSVC(const enVideoModeH264 mode,const DWORD ratio=(DWORD)-1, const DWORD staticMB=(DWORD)-1,const DWORD profile=(H264_Profile_BaseLine));
    CVideoCapSVC(const CVideoCapSVC& other);
    CVideoCapSVC(const svcCapStruct& tStruct);
    CVideoCapSVC(const CVideoCap& first,const CVideoCap& second);
	virtual ~CVideoCapSVC();
	CVideoCapSVC& operator =(const CVideoCapSVC& other);
	virtual const char*  NameOf() const { return "CVideoCapSVC"; }
		// overrides
	//DWORD GetProfile(){return m_profileValue;}
	virtual CapEnum GetPayloadType() const { return eSvcCapCode; };
	virtual CVideoCap* Clone() const;

	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);

	void FillStructH323(svcCapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;
	const VIDEO_OPERATION_POINT_SET_S& GetOperationSet() const { return m_operationPoints;};
	const STREAM_GROUP_S& GetRecvStreamGroup() const { return m_recvStreamsGroup;};
	const STREAM_GROUP_S& GetSendStreamGroup() const { return m_sendStreamsGroup;};
	//void SetOperationSet(const VIDEO_OPERATION_POINT_SET_S &operationPoints){m_operationPoints = operationPoints;};

protected:
    // SVC fields, copy from svcCapStruct
    VIDEO_OPERATION_POINT_SET_S m_operationPoints;
    STREAM_GROUP_S              m_recvStreamsGroup;
    STREAM_GROUP_S              m_sendStreamsGroup;
    APIU8                       m_scalableLayerId;    // needed for legacy SVC support; if MRC SVC, it will be ignored
    APIU8                       m_isLegacy;           // 0 – for MRC, 1 – for legacy

};

//////////////////////////////////////////////////////////////////////////
//N.A. DEBUG VP8
//////////////////////////////////////////////////////////////////////////
// 						Video Cap VP8									//
//////////////////////////////////////////////////////////////////////////
class CVideoCapVP8 : public CVideoCap
{
CLASS_TYPE_1(CVideoCapVP8, CPObject)
public:
	CVideoCapVP8();
	CVideoCapVP8(const CVideoCapVP8& other);
	CVideoCapVP8(const vp8CapStruct& tStruct);
	virtual ~CVideoCapVP8() {};
	CVideoCapVP8& operator =(const CVideoCapVP8& other);

	CVideoCapVP8(const CVideoCap& first,const CVideoCap& second);


	virtual CapEnum GetPayloadType() const { return eVP8CapCode; };
	virtual const char*  NameOf() const { return "CVideoCapVP8"; }
	virtual CVideoCap* Clone() const;
	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);


	void FillStructH323(vp8CapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;

	//void SetAspectRatio( const DWORD ratio ) { m_aspectRatio = ratio; }
	void SetMaxFR( const DWORD fr ) { m_maxFR = fr; }
	void SetMaxFS( const DWORD fs ) { m_maxFS = fs; }
	//void SetHeight( const DWORD height ) { m_Height = height; }
	//void SetWidth( const DWORD width ) { m_Width = width; }
	void SetMaxBitRate( const DWORD maxBR ) { m_maxBitRate = maxBR; }

protected:

	DWORD   m_maxFR;
	DWORD   m_maxFS;
	DWORD   m_maxBitRate;
	//DWORD	m_aspectRatio;
	//DWORD   m_Height;
	//DWORD   m_Width;
};

//////////////////////////////////////////////////////////////////////////
// 						End Video Cap VP8							    //
//////////////////////////////////////////////////////////////////////////







//////////////////////////////////////////////////////////////////////////
class CVideoCapRTV : public CVideoCap
{
CLASS_TYPE_1(CVideoCapRTV, CPObject)
public:
	CVideoCapRTV(ERtvVideoModeType rtvVideoMode=e_rtv_HD720Symmetric);
	CVideoCapRTV(const rtvCapStruct& tStruct);
	CVideoCapRTV(const CVideoCapRTV& other);
	CVideoCapRTV(const CVideoCap& first,const CVideoCap& second);
	virtual ~CVideoCapRTV();
	CVideoCapRTV& operator =(const CVideoCapRTV& other);
	virtual const char*  NameOf() const { return "CVideoCapRTV"; }
		// overrides

	virtual CapEnum GetPayloadType() const { return eRtvCapCode; };
	virtual CVideoCap* Clone() const;

	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);

	void FillStructH323(rtvCapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;

	void CreatePresentationCap();

private:
	DWORD m_rtvVideoMode;

};
//////////////////////////////////////////////////////////////////////////
class CVideoCapMSSvc : public CVideoCap
{
CLASS_TYPE_1(CVideoCapMSSvc, CPObject)
public:
	CVideoCapMSSvc();
  //  CVideoCapMSSvc(Eh264VideoModeType mssvcVideoMode=eHD720Symmetric);
    CVideoCapMSSvc(const msSvcCapStruct& tStruct);
    CVideoCapMSSvc(const CVideoCapMSSvc& other);
    CVideoCapMSSvc(const CVideoCap& first,const CVideoCap& second);
	virtual ~CVideoCapMSSvc();
	CVideoCapMSSvc& operator =(const CVideoCapMSSvc& other);
	virtual const char*  NameOf() const { return "CVideoCapMSSvc"; }
		// overrides

	virtual CapEnum GetPayloadType() const { return eMsSvcCapCode; };
	virtual CVideoCap* Clone() const;

	virtual void Serialize(CSegment& rSegCap) const;
	virtual void DeSerialize(CSegment& rSegParam);

	void FillStructH323(msSvcCapStruct* pStruct,const DWORD maxVideoRate,
			const BYTE direction,const BYTE role) const;

	void CreatePresentationCap();


protected:
	DWORD	m_width;
	DWORD	m_height;
	DWORD	m_maxFrameRate;
	DWORD	m_packatizationmode;
	DWORD	m_aspectRatio;

};
///// SRTP - SDES CAP
//////////////////////////////////////////////////////////////////////////
class CSdesEpCap : public CPObject
{
CLASS_TYPE_1(CSdesCap, CPObject)
public:
	CSdesEpCap() {;}
	CSdesEpCap(cmCapDataType capDataType);
	CSdesEpCap(sdesCapStruct& tStruct);
	virtual ~CSdesEpCap() {;}
	virtual const char* NameOf() const { return "CSdesCap";}

// 	void Serialize(CSegment& rSegCap) const;
//	void DeSerialize(CSegment& rSegParam);

	CSdesEpCap* Clone() const;

	BYTE GetSdesMediaType() const { return m_mediaType; }
	void FillStructSdes(sdesCapStruct* pStruct,	const BYTE direction, const BYTE role) const;

protected:

	DWORD m_tag;
	DWORD m_cryptoSuite;
	sdesSessionParamsStruct m_sessionParams;
	DWORD m_numKeyParams;
	sdesKeyParamsStruct m_sdesKeyParamsStruct;

	DWORD m_mediaType;

};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



#endif // __EPSIMCAPSETSLIST_H__
