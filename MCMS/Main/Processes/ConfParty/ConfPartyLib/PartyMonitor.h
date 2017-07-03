//+========================================================================+
//                            PartyMonitor.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PartyMonitor.H                                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: ANAT A                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |06/08/03     |                                                     |
//+========================================================================+
#ifndef __PARTMNT_H__
#define __PARTMNT_H__

#include "IpAddressDefinitions.h"
#include "ConfPartyDefines.h"
#include "PObject.h"
#include "psosxml.h"
#include "IpChannelParams.h"
#include "IceCmInd.h"
#include "SipConfPartyDefinitions.h"
#include "ObjString.h"



class CSegment;    
class CXMLDOMElement;  
struct annexes_fd_set;  

#define LSBmask					0xFFFF
//#define ExceedingNumber			0.01	// [> 1%]
//#define ExceedFractionLossNumber 1.275	// [>=0.5%] --> 0.005(2^8-1)
//#define ExceedLatencyNumber		19660.5 // [>300ms] --> 0.3*(2^16-1) 
//#define ExceedJitterNumber		80		// [>80ms]
#define FractionLossPercent		256
#define VideokHz				90
#define AudiokHz				8
#define FecckHz					8

#define BitRateProblem			0x1		//1
#define FractionLossProblem		0x3		//11
#define JitterProblem			0x7		//111
#define LatencyProblem			0xF		//1111
#define ProtocolProblem			0x1F	//11111
#define ResolutionProblem		0x3F	//111111
#define	FrameRateProblem		0x7F	//1111111
#define AnnexesProblem			0xFF	//11111111
#define FramePerPacketProblem	0x1FF	//111111111
#define ActualLossAccProblem	0x3FF	//1111111111
#define ActualLossInterProblem	0x7FF	//11111111111	
#define OutOfOrderAccProblem	0xFFF	//111111111111
#define OutOfOrderInterProblem	0x1FFF	//1111111111111
#define FragmentedAccProblem	0x3FFF	//11111111111111
#define FragmentedInterProblem	0x7FFF	//111111111111111


BYTE IsContainAnnex(int eAnnex,DWORD annexes);

////////////Party monitoring base parameters/////////////////
class CPrtMontrBaseParams	:  public CPObject
{
CLASS_TYPE_1(CPrtMontrBaseParams,CPObject)  
public:
	CPrtMontrBaseParams();
	CPrtMontrBaseParams(DWORD channelType);
	CPrtMontrBaseParams(const CPrtMontrBaseParams &other);
	virtual const char* NameOf() const { return "CPrtMontrBaseParams";}
	CPrtMontrBaseParams& operator=(const CPrtMontrBaseParams& other);
	virtual DWORD operator==(const CPrtMontrBaseParams& other);
	virtual DWORD operator!=(const CPrtMontrBaseParams& other);
	virtual ~CPrtMontrBaseParams(){}

	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);
	
//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void ClearMapProblem(){m_mapProblem = 0;}
	void SetProblem(DWORD problem){m_mapProblem |= problem;}
	void SetBitRate(DWORD bitRate) {m_bitRate = bitRate;}
	void SetProtocol(DWORD protocol) {m_protocol = protocol;}
	void SetChannelIndex(DWORD chanIndex) {m_channelIndex = chanIndex;}
	void SetChannelType(DWORD chanType) {m_channelType = chanType;}
	void SetPartyAddr(mcTransportAddress* partyAddr);
	void SetPartyPort(unsigned short partyPort) {m_partyAddr.port = partyPort;}
	void SetMcuAddr(mcTransportAddress* mcuAddr);
	void SetMcuPort(unsigned short mcuPort) {m_mcuAddr.port = mcuPort;}
	void SetConnectionStatus(DWORD connectionStatus) {m_connectionStatus = connectionStatus;}


	DWORD GetProblem() const {return m_mapProblem;}
	DWORD GetBitRate() const  {return m_bitRate;}
	DWORD GetProtocol() const  {return m_protocol;}
	DWORD GetChannelIndex() const  {return m_channelIndex;}
	DWORD GetChannelType() const  {return m_channelType;}
	const mcTransportAddress* GetPartyAddr() const  {return &m_partyAddr;}
	unsigned short GetPartyPort() const  {return m_partyAddr.port;}
	const mcTransportAddress* GetMcuAddr() const  {return &m_mcuAddr;}
	unsigned short GetMcuPort() const  {return m_mcuAddr.port;}
	DWORD GetConnectionStatus() const  {return m_connectionStatus;}

	//ICE
	void SetIsIce(BYTE IsIce){m_IsIce = IsIce;}
	void SetIcePartyAddr(mcTransportAddress* IcePartyAddr);
	void SetIcePartyPort(unsigned short IcePartyPort) {m_IcePartyAddr.port = IcePartyPort;}
	void SetIceMcuAddr(mcTransportAddress* IceMcuAddr);
	void SetIceMcuPort(unsigned short IceMcuPort) {m_IceMcuAddr.port = IceMcuPort;}
	void SetIceConnectionType(EIceConnectionType IceConnectionType);

	BYTE GetIsIce(){return m_IsIce;}
	const mcTransportAddress* GetIcePartyAddr() const  {return &m_IcePartyAddr;}
	unsigned short GetIcePartyPort() const  {return m_IcePartyAddr.port;}
	const mcTransportAddress* GetIceMcuAddr() const  {return &m_IceMcuAddr;}
	unsigned short GetIceMcuPort() const  {return m_IceMcuAddr.port;}
	EIceConnectionType GetIceConnectionType() const {return m_IceConnectionType;}


	virtual DWORD GetNumOfPackets() const  {return 0xFFFFFFFF;}
	virtual DWORD GetLatency() const  {return 0xFFFFFFFF;}
	virtual DWORD GetPacketLoss() const  {return 0xFFFFFFFF;}
	virtual DWORD GetJitter() const  {return 0xFFFFFFFF;}
	virtual DWORD GetJitterPeak() const  {return 0xFFFFFFFF;}
	virtual DWORD GetFranctionLost() const  {return 0xFFFFFFFF;}
	virtual DWORD GetFranctionLostPeak() const  {return 0xFFFFFFFF;}
	virtual DWORD GetFramePerPacket() {return 0xFFFFFFFF;}
	virtual int  GetResolution() const  {return 0xFFFFFFFF;}
	virtual int  GetMaxResolution() const  {return 0xFFFFFFFF;}
	virtual int  GetMinResolution() const  {return 0xFFFFFFFF;}
	virtual void SetResolution(int resolution) {}
	virtual void SetMaxResolution(int resolution) {}
	virtual void SetMinResolution(int resolution) {}
	virtual unsigned short GetFrameRate() const {return 0xFFFF;}
	virtual unsigned short GetMaxFrameRate() const {return 0xFFFF;}
	virtual unsigned short GetMinFrameRate() const {return 0xFFFF;}
	virtual void SetFrameRate(unsigned short frameRate){}
	virtual void SetMaxFrameRate(unsigned short frameRate){}
	virtual void SetMinFrameRate(unsigned short frameRate){}
	virtual DWORD GetAnnexes() const {return 0xFFFFFFFF;}


	static CPrtMontrBaseParams *AllocNewClass(EIpChannelType channelType);
	static CPrtMontrBaseParams *AllocNewClass(const CPrtMontrBaseParams &other);

	virtual void CheckExceedingFieldsRules(DWORD rate,DWORD protocol,CMedString	&strExcFieldsRules);
	
	virtual void Dump1();


protected:
	DWORD	m_mapProblem;
	DWORD	m_bitRate;
	DWORD	m_protocol;
	DWORD	m_channelIndex;
	DWORD   m_channelType;
	// IpV6 - Monitoring
	mcTransportAddress  m_partyAddr;
	mcTransportAddress	m_mcuAddr;
	mcTransportAddress  m_IcePartyAddr;
	mcTransportAddress	m_IceMcuAddr;
	DWORD	m_connectionStatus;
	EIceConnectionType m_IceConnectionType;

	BYTE	m_IsIce;

};

////////////Party monitoring global parameters/////////////////
class CPrtMontrGlobalParams : public CPrtMontrBaseParams
{
	CLASS_TYPE_1(CPrtMontrGlobalParams,CPrtMontrBaseParams)
public:
	
	CPrtMontrGlobalParams();
	CPrtMontrGlobalParams(const CPrtMontrGlobalParams &other);
	virtual const char* NameOf() const { return "CPrtMontrGlobalParams";}
	CPrtMontrGlobalParams& operator=(const CPrtMontrGlobalParams& other);
	virtual DWORD operator==(const CPrtMontrGlobalParams& other);
	virtual DWORD operator!=(const CPrtMontrGlobalParams& other);
	virtual ~CPrtMontrGlobalParams(){}
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void  SetGlobalParam(const CPrtMontrGlobalParams& globalParam);

	void SetNumOfPackets(DWORD numPackets) {m_numOfPackets = numPackets;}
	void SetLatency(DWORD latency);
	void SetPacketLoss(DWORD packetLoss) {m_packetLoss = packetLoss;}
	void SetJitter(DWORD jitter);
	void SetJitterPeak(DWORD jitterPeak);
	// the fraction loss data received from the card as UINT16 and is shifted to UINT32 and percent for operator monitoring.
	void SetFranctionLoss(unsigned short franctioLost) ;
	void SetFranctionLossPeak(unsigned short franctioLostPeak);

	virtual DWORD GetNumOfPackets() const  {return m_numOfPackets;}
	virtual DWORD GetLatency() const  {return m_latency;}
	virtual DWORD GetPacketLoss() const  {return m_packetLoss;}
	virtual DWORD GetJitter() const  {return m_jitter;}
	virtual DWORD GetJitterPeak() const  {return m_jitterPeak;}
	virtual DWORD GetFranctionLost() const  {return m_franctioLoss;}
	virtual DWORD GetFranctionLostPeak() const  {return m_franctioLossPeak;}

	virtual void CheckExceedingFieldsRules(DWORD rate,DWORD protocol,CMedString	&strExcFieldsRules);
	
	virtual void Dump1();

protected:
	DWORD   m_numOfPackets;
	DWORD	m_latency;
	DWORD	m_packetLoss;
	DWORD	m_jitter;
	DWORD	m_jitterPeak;
	DWORD	m_franctioLoss;
	DWORD   m_franctioLossPeak;
};

////////////Rtp Rtcp information/////////////////
class   CAdvanceChInfo: public CPObject
{
	CLASS_TYPE_1(CAdvanceChInfo,CPObject )
public:
	CAdvanceChInfo();
	virtual ~CAdvanceChInfo(){}
	CAdvanceChInfo(const CAdvanceChInfo &other);
	CAdvanceChInfo& operator=(const CAdvanceChInfo& other);
	virtual DWORD operator==(const CAdvanceChInfo& other);
	virtual DWORD operator!=(const CAdvanceChInfo& other);
	virtual const char* NameOf() const { return "CAdvanceChInfo";}

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void SetAccumulate(DWORD accumulate) {m_accumulate = accumulate;}
	void SetAccumulatePercent(DWORD accumulate) {m_accumulatePercent = accumulate;}
	void SetIntervalPercent(DWORD interval) {m_intervalPercent = interval;}
	void SetInterval(unsigned short interval) {m_interval = interval;}
	void SetIntervalPeak(unsigned short interval) {m_intervalPeak = interval;}
	
	DWORD GetAccumulate() {return m_accumulate;}
	DWORD GetAccumulatePercent() {return m_accumulatePercent;}
	DWORD GetIntervalPercent() {return m_intervalPercent;}
	unsigned short GetInterval() {return m_interval;}
	unsigned short GetIntervalPeak() {return m_intervalPeak;}
	
	virtual void Dump1();

protected:
	DWORD	m_accumulate;
	DWORD	m_accumulatePercent;
	DWORD	m_intervalPercent;
	unsigned short	m_interval;
	unsigned short  m_intervalPeak;
};

////////////Rtp Rtcp information/////////////////
class  CRtpInfo :  public CPObject
{
CLASS_TYPE_1(CRtpInfo,CPObject)
	friend class CAdvanceAudioIn;
	friend class CAdvanceVideoIn;
	
public:
	CRtpInfo();
	virtual ~CRtpInfo(){}
	CRtpInfo(const CRtpInfo &other);
	CRtpInfo& operator=(const CRtpInfo& other);
	virtual DWORD operator==(const CRtpInfo& other);
	virtual DWORD operator!=(const CRtpInfo& other); 
	virtual const char* NameOf() const { return "CRtpInfo";}
//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void SetRtpInfo(const CRtpInfo& rtpInfo);

	CAdvanceChInfo &GetRtpPacketLoss() {return m_rtpPacketLoss;}
	CAdvanceChInfo &GetRtpOutOfOrder() {return m_rtpOutOfOrder;}
	CAdvanceChInfo &GetRtpFragmentPackets() {return m_rtpFragmentPackets;}
	CAdvanceChInfo &GetJitterBufferSize() {return m_jitterBufferSize;}
	CAdvanceChInfo &GetJitterLatePackets() {return m_jitterLatePackets;}
	CAdvanceChInfo &GetJitterOverflows() {return m_jitterOverflows;}
	CAdvanceChInfo &GetJitterSamplePacketInterval() {return m_jitterSamplePacketInterval;}

	virtual DWORD CheckExceedingFieldsRules(CMedString &strExcFieldsRules);
	
	virtual void DumpStr();

protected:
	CAdvanceChInfo m_rtpPacketLoss;
	CAdvanceChInfo m_rtpOutOfOrder;
	CAdvanceChInfo m_rtpFragmentPackets;
	CAdvanceChInfo m_jitterBufferSize;
	CAdvanceChInfo m_jitterLatePackets;
	CAdvanceChInfo m_jitterOverflows;
	CAdvanceChInfo m_jitterSamplePacketInterval;	
};



////////////Audio advance/////////////////
class CAdvanceAudio : public CPrtMontrGlobalParams
{
CLASS_TYPE_1(CAdvanceAudio,CPrtMontrGlobalParams)
public:

	CAdvanceAudio():m_framePerPacket(0xFFFFFFFF){}
	CAdvanceAudio(const CAdvanceAudio &other);
	virtual const char* NameOf() const { return "CAdvanceAudio";}
	CAdvanceAudio& operator=(const CAdvanceAudio& other);
	virtual DWORD operator==(const CAdvanceAudio& other);
	virtual DWORD operator!=(const CAdvanceAudio& other); 
	virtual ~CAdvanceAudio(){}
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void SetFramePerPacket(DWORD framePerPacket) {m_framePerPacket = framePerPacket;}
	virtual DWORD GetFramePerPacket() {return m_framePerPacket;}

	virtual void CheckExceedingFieldsRules(DWORD rate,DWORD protocol,DWORD framePerPacket,CMedString	&strExcFieldsRules);

	void Dump1();
	
protected:
	DWORD m_framePerPacket;
};

////////////////////////////////////////////////////////////////////////
class CAdvanceAudioOut: public CAdvanceAudio
{
	CLASS_TYPE_1(CAdvanceAudioOut, CAdvanceAudio) 
public:

	CAdvanceAudioOut();
	virtual ~CAdvanceAudioOut(){}
	virtual void CopyClass(CPrtMontrBaseParams &other);
	void Dump1();
	virtual const char* NameOf() const { return "CAdvanceAudioOut";}
	
};

////////////////////////////////////////////////////////////////////////
class CAdvanceAudioIn : public CAdvanceAudio, public CRtpInfo
{
	CLASS_TYPE_2(CAdvanceAudioIn,CAdvanceAudio,CRtpInfo)  //**check macro**
public:

	CAdvanceAudioIn();
	virtual ~CAdvanceAudioIn(){}
	virtual const char* NameOf() const { return "CAdvanceAudioIn";}
	CAdvanceAudioIn(const CAdvanceAudioIn &other);
	CAdvanceAudioIn& operator=(const CAdvanceAudioIn& other);
	virtual DWORD operator==(const CAdvanceAudioIn& other);
	virtual DWORD operator!=(const CAdvanceAudioIn& other); 
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	virtual void  CheckExceedingFieldsRules(DWORD rate,DWORD protocol,long framePerPacket,CMedString &strExcFieldsRules);
	
//	void Dump1();
};

////////////Video advance/////////////////
class CAdvanceVideo	: public CPrtMontrGlobalParams
{
CLASS_TYPE_1(CAdvanceVideo,CPrtMontrGlobalParams)
public:

	CAdvanceVideo();
	virtual ~CAdvanceVideo(){}
	virtual const char* NameOf() const { return "CAdvanceVideo";}
	CAdvanceVideo(const CAdvanceVideo &other);
	CAdvanceVideo& operator=(const CAdvanceVideo& other);
	virtual DWORD operator==(const CAdvanceVideo& other);
	virtual DWORD operator!=(const CAdvanceVideo& other); 
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	virtual void SetResolution(int resolution){m_resolution = resolution;}
	virtual void SetMaxResolution(int resolution){m_maxResolution = resolution;}
	virtual void SetMinResolution(int resolution){m_minResolution = resolution;}
	virtual void SetFrameRate(unsigned short frameRate){m_frameRate = frameRate;}
	virtual void SetMaxFrameRate(unsigned short frameRate){m_maxFrameRate = frameRate;}
	virtual void SetMinFrameRate(unsigned short frameRate){m_minFrameRate = frameRate;}
	void SetAnnexes(DWORD annexes){m_annexes = annexes;}	
	void SetResolutionWidth(WORD resolutionWidth)   {m_resolutionWidth  = resolutionWidth;}
	void SetMaxResolutionWidth(WORD resolutionWidth)   {m_maxResolutionWidth  = resolutionWidth;}
	void SetMinResolutionWidth(WORD resolutionWidth)   {m_minResolutionWidth  = resolutionWidth;}
	void SetResolutionHeight(WORD resolutionHeight) {m_resolutionHeight = resolutionHeight;}
	void SetMaxResolutionHeight(WORD resolutionHeight) {m_maxResolutionHeight = resolutionHeight;}
	void SetMinResolutionHeight(WORD resolutionHeight) {m_minResolutionHeight = resolutionHeight;}

	virtual int  GetResolution() const  {return m_resolution;}
	virtual int  GetMaxResolution() const  {return m_maxResolution;}
	virtual int  GetMinResolution() const  {return m_minResolution;}
	virtual unsigned short GetFrameRate() const {return m_frameRate;}
	virtual unsigned short GetMaxFrameRate() const {return m_maxFrameRate;}
	virtual unsigned short GetMinFrameRate() const {return m_minFrameRate;}
	virtual DWORD GetAnnexes() const {return m_annexes;}
	WORD GetResolutionWidth() const {return m_resolutionWidth;}
	WORD GetMaxResolutionWidth() const {return m_maxResolutionWidth;}
	WORD GetMinResolutionWidth() const {return m_minResolutionWidth;}
	WORD GetResolutionHeight() const {return m_resolutionHeight;}
	WORD GetMaxResolutionHeight() const {return m_maxResolutionHeight;}
	WORD GetMinResolutionHeight() const {return m_minResolutionHeight;}
	BYTE bIsResolutionOk(DWORD resolution);

	virtual void  CheckExceedingFieldsRules(DWORD rate,DWORD protocol,DWORD annexes,
											int resolution,unsigned short frameRate,CMedString	&strExcFieldsRules);
	
	void Dump1();
											
protected:
	BYTE    IsAnnex(int annex) const;
	void    AddAnnex(annexes_fd_set *pTempMask,DWORD eAnnex) const;
	
	DWORD 			m_annexes;
	int 			m_resolution;
	int 			m_maxResolution;
	int 			m_minResolution;
	unsigned short  m_frameRate;
	unsigned short  m_maxFrameRate;
	unsigned short  m_minFrameRate;
	WORD 			m_resolutionWidth;
	WORD 			m_maxResolutionWidth;
	WORD 			m_minResolutionWidth;
	WORD 			m_resolutionHeight;
	WORD 			m_maxResolutionHeight;
	WORD 			m_minResolutionHeight;
	
private:
};

//////////////////////////////////////////////////////////////////////////
class CAdvanceVideoOut: public CAdvanceVideo
{
	CLASS_TYPE_1(CAdvanceVideoOut, CAdvanceVideo) 
public:

	CAdvanceVideoOut(EIpChannelType channelType);
	virtual ~CAdvanceVideoOut(){}
	virtual void CopyClass(CPrtMontrBaseParams &other);
	void Dump1();
	virtual const char* NameOf() const { return "CAdvanceVideoOut";}
};


///////////////////////////////////////////////////////////////////////////
class CAdvanceVideoIn	: public CAdvanceVideo, public CRtpInfo
{
CLASS_TYPE_2(CAdvanceVideoIn,CAdvanceVideo,CRtpInfo)  //**check macro**
public:
	CAdvanceVideoIn(EIpChannelType channelType);
	virtual ~CAdvanceVideoIn(){}
	CAdvanceVideoIn(const CAdvanceVideoIn &other);
	virtual const char* NameOf() const { return "CAdvanceVideoIn";}
	CAdvanceVideoIn& operator=(const CAdvanceVideoIn& other);
	virtual DWORD operator==(const CAdvanceVideoIn& other);
	virtual DWORD operator!=(const CAdvanceVideoIn& other); 
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);
	

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	CAdvanceChInfo &GetErrorResilience() {return m_errorResilience;}

	virtual void  CheckExceedingFieldsRules(DWORD rate,DWORD protocol,DWORD annexes,
											DWORD resolution,DWORD frameRate,CMedString	&strExcFieldsRules);
											
//	void Dump12();
	
protected:
	CAdvanceChInfo m_errorResilience;
};


////////////FECC advance/////////////////

//////////////////////////////////////////////////////////////////////////
class CAdvanceFeccOut: public CPrtMontrGlobalParams
{
	CLASS_TYPE_1(CAdvanceFeccOut, CPrtMontrGlobalParams) 
public:

	CAdvanceFeccOut();
	virtual ~CAdvanceFeccOut(){}
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual void Dump1();
	virtual const char* NameOf() const { return "CAdvanceFeccOut";}
};


///////////////////////////////////////////////////////////
class CAdvanceFeccIn : public CPrtMontrGlobalParams
{
CLASS_TYPE_1(CAdvanceFeccIn,CPrtMontrGlobalParams)
public:

	CAdvanceFeccIn();
	virtual ~CAdvanceFeccIn(){}
	virtual const char* NameOf() const { return "CAdvanceFeccIn";}
	CAdvanceFeccIn(const CAdvanceFeccIn &other);
	CAdvanceFeccIn& operator=(const CAdvanceFeccIn& other);
	virtual DWORD operator==(const CAdvanceFeccIn& other);
	virtual DWORD operator!=(const CAdvanceFeccIn& other); 
	virtual void CopyClass(CPrtMontrBaseParams &other);
	virtual DWORD IsEqual(CPrtMontrBaseParams &other);

//#ifdef __HIGHC__
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual void  Serialize(WORD format,CSegment& seg) const;
//#endif
	// for GUI only XML interface (Serialize, DeSerialize)
//	virtual void  Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum);
//	virtual void  DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	CAdvanceChInfo &GetRtpPacketLoss() {return m_rtpPacketLoss;}
	CAdvanceChInfo &GetRtpOutOfOrder() {return m_rtpOutOfOrder;}
	CAdvanceChInfo &GetRtpFragmentPackets() {return m_rtpFragmentPackets;}
	
	virtual void Dump1();

protected:
	CAdvanceChInfo m_rtpPacketLoss;
	CAdvanceChInfo m_rtpOutOfOrder;
	CAdvanceChInfo m_rtpFragmentPackets;
};

#endif
