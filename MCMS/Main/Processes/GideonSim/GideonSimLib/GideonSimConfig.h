//+========================================================================+
//                    GideonSimConfig.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimConfig.h                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __GIDEONSIMCONFIG_
#define   __GIDEONSIMCONFIG_


#include "SerializeObject.h"
#include "McuMngrStructs.h"
#include "CardsStructs.h"

#define		MPL_SIM_CONFIG_FILE_NAME		"Cfg/MPL_SIM.XML"
#define	   ISDN_SIM_CONFIG_FILE_NAME		"VersionCfg/IsdnPartyCfg.xml"


class  CXMLDOMElement;
class  COstrStream;
class  CIstrStream;
class  CCardCfg;
class  CBehaviourSwitch;
class  CBehaviourMfa;
class  CBehaviourIsdn;

#define BarakCard_REAL_RTP_MODE 0	//BarakCard Real RTP mode, working with CS
#define BarakCard_NO_RTP_MODE 1	//BarakCard No RTP mode, working with EndpointSim

class  CGideonSimSystemCfg  : public CSerializeObject
{
CLASS_TYPE_1(CGideonSimSystemCfg,CSerializeObject)
public:

				// Constructors
	CGideonSimSystemCfg();
	virtual ~CGideonSimSystemCfg();
	CGideonSimSystemCfg(const CGideonSimSystemCfg& other);

				// Initializations

				// Operations
	CGideonSimSystemCfg& operator =(const CGideonSimSystemCfg& reference);
	virtual CSerializeObject* Clone() { return NULL; }

	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& rSegment);
	int IsEqual(const CGideonSimSystemCfg& rOther) const;
	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(MPL_SIM_CONFIG_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(MPL_SIM_CONFIG_FILE_NAME,"MPL_SIM_CONFIGURATION"); }

	char* GetBoxChassisId() { return m_szBoxChassisId; }
	WORD  GetMaxCardSlots() const			{ return m_wMaxCardSlots; }
	WORD  GetMplApiSwitchPortNumber() const	{ return m_wMplApiSwitchPortNumber; }
	WORD  GetMplApiMfaPortNumber() const	{ return m_wMplApiMfaPortNumber; }
	const char* GetMplApiIpAddress() const	{ return (const char*)m_szMplApiIp; }
	WORD  GetGuiPortNumber() const			{ return m_wGuiPortNumber; }
	//added by huiyu
	WORD  GetPcmPortNumber() const			{ return m_wPcmPortNumber; }
	WORD  GetPcmFramePortNumber() const		{ return m_wPcmFramePortNumber; }	
	BYTE GetMplApiSecureMode() const  { return m_isSecured; }
	WORD  GetPlatformType() const			{ return m_platformType; }
	WORD GetBarakCardMode() const	{return m_BarakCardMode;}

	CCardCfg*  GetCardCfg( const WORD index );
	void SetCardCfg( const WORD index, CCardCfg* cardCfg);
	void ClearCardCfg( const WORD index );

				// Cards behaviour
	WORD  GetSwitchConfigTime() const;
	WORD  GetMfaUnitConfigTime() const;
	WORD  GetMfaMediaConfigTime() const;
	WORD  GetMfaSpeakerChangeTime() const;

	BOOL  GetIsdnCapsFlag() const;
	BOOL  GetIsdnXmitModeFlag() const;
	BOOL  GetIsdnH230Flag() const;

	CBehaviourIsdn* GetBehaviourIsdn() const { return m_pBehaviourIsdn; }

protected:
	bool CheckValidCfg();
				// Operations
//	void  WriteXmlFile();
//	int   ReadXmlFile();

				// Attributes
	char		m_szBoxChassisId[MPL_SERIAL_NUM_LEN];
	char		m_szMplApiIp[IP_ADDRESS_STR_LEN];
	WORD		m_wMplApiSwitchPortNumber;
	WORD		m_wMplApiMfaPortNumber;
	WORD		m_wMaxCardSlots;
	WORD		m_wGuiPortNumber;
	WORD		m_platformType;
	//added by huiyu
	WORD		m_wPcmPortNumber;
	WORD		m_wPcmFramePortNumber;
	CCardCfg**	m_ppCardsArr;
	BYTE		m_isSecured;

	CBehaviourSwitch*	m_pBehaviourSwitch;
	CBehaviourMfa*		m_pBehaviourMfa;
	CBehaviourIsdn*		m_pBehaviourIsdn;
	WORD		m_BarakCardMode;//BarakCard_NO_RTP_MODE, BarakCard_REAL_RTP_MODE
};



class  CCardCfg : public CSerializeObject
{
CLASS_TYPE_1(CCardCfg,CSerializeObject)
public:
				// Constructors
	CCardCfg();
	virtual ~CCardCfg();
	CCardCfg(const CCardCfg& other);
	CCardCfg& operator= (const CCardCfg& other);

				// Initializations

				// Operations
	virtual CCardCfg* CreateCopy() = 0;
	virtual eCardTypes GetCardType() const = 0;

				// Utils
	WORD	GetBoardId() const      { return m_wBoardId; }
	char*   GetSerialNumber() { return m_szSerialNumber; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	virtual CSerializeObject* Clone() { return NULL; }

	BOOL IsMediaCard();
	
protected:
				// Operations

				// Attributes
	WORD	m_wBoardId;
	char	m_szSerialNumber[MPL_SERIAL_NUM_LEN];
};


class  CCardCfgSwitch : public CCardCfg
{
CLASS_TYPE_1(CCardCfgSwitch,CCardCfg )
public:
				// Constructors
	CCardCfgSwitch();
	virtual ~CCardCfgSwitch();
	CCardCfgSwitch(const CCardCfgSwitch& other);
	CCardCfgSwitch& operator= (const CCardCfgSwitch& other);

				// Initializations

				// Operations
	virtual CCardCfg* CreateCopy();
	virtual eCardTypes GetCardType() const { return eCardSwitch; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);

protected:
				// Operations

				// Attributes
	BOOL	m_isActive;
};



typedef struct
{
	WORD	   unitId;
	DWORD	   unitStatus;
} UNIT_S;


class  CBaseMediaCardCfg
{
public:
	CBaseMediaCardCfg();
	virtual ~CBaseMediaCardCfg();
	
	void SetRtmAttached(BOOL flag){m_isRtmAttached  = (flag) ? TRUE : FALSE; }
	BOOL  GetRtmAttached() const { return m_isRtmAttached; }
	eCardType  GetRtmCardType() const { return m_RtmCardType; }
	
	void SetRtmCardType(eCardType rtmCardType){m_RtmCardType  = rtmCardType ;}

	UNIT_S GetUnit(int index) const { return m_unitList[index]; }
	eCardType GetDetailedCardType() {return m_detailedCardType;}
	void SetDetailedCardType(eCardType cardType){m_detailedCardType  = cardType ;}

	CBaseMediaCardCfg& operator= (const CBaseMediaCardCfg& other);

protected:
	eCardType 	m_detailedCardType;
	BOOL        m_isRtmAttached;
	eCardType   m_RtmCardType;
	
	UNIT_S m_unitList[MAX_NUM_OF_UNITS];
};

class  CCardCfgMfa : public CCardCfg,  public CBaseMediaCardCfg
{
CLASS_TYPE_1(CCardCfgMfa,CCardCfg)
public:
				// Constructors
	CCardCfgMfa();
	CCardCfgMfa(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm);
	virtual ~CCardCfgMfa();
	CCardCfgMfa(const CCardCfgMfa& other);
	CCardCfgMfa& operator= (const CCardCfgMfa& other);

				// Initializations

				// Operations
	virtual CCardCfg* CreateCopy();
	virtual eCardTypes GetCardType() const { return eCardMfa; }


	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
};

//////BARAK///////

class  CCardCfgBarak : public CCardCfg, public CBaseMediaCardCfg
{
CLASS_TYPE_1(CCardCfgBarak,CCardCfg )
public:
				// Constructors
	CCardCfgBarak();
	CCardCfgBarak(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm);
	virtual ~CCardCfgBarak();
	CCardCfgBarak(const CCardCfgBarak& other);
	CCardCfgBarak& operator= (const CCardCfgBarak& other);

				// Initializations

				// Operations
	const char* NameOf() const {return "CCardCfgBarak";}
	virtual CCardCfg* CreateCopy();
	virtual eCardTypes GetCardType() const { return eCardBarak; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
};
//////BARAK///////

//////BREEZE///////
class  CCardCfgBreeze : public CCardCfgBarak
{
CLASS_TYPE_1(CCardCfgBreeze,CCardCfgBarak )
public:
				// Constructors
	CCardCfgBreeze();
	CCardCfgBreeze(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm);
	virtual ~CCardCfgBreeze();
	CCardCfgBreeze(const CCardCfgBreeze& other);
	CCardCfgBreeze& operator= (const CCardCfgBreeze& other);

				// Initializations

				// Operations
	const char* NameOf() const {return "CCardCfgBreeze";}
	virtual CCardCfg* CreateCopy();
	virtual eCardTypes GetCardType() const { return eCardBreeze; }
};
//////BREEZE///////

//////MPMRX///////
class  CCardCfgMpmRx : public CCardCfgBarak
{
    CLASS_TYPE_1(CCardCfgMpmRx,CCardCfgBarak )
public:
    // Constructors
    CCardCfgMpmRx();
    CCardCfgMpmRx(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm);
    virtual ~CCardCfgMpmRx();
    CCardCfgMpmRx(const CCardCfgMpmRx& other);
    CCardCfgMpmRx& operator= (const CCardCfgMpmRx& other);

    // Initializations

    // Operations
    const char* NameOf() const {return "CCardCfgMpmRx";}
    virtual CCardCfg* CreateCopy();
    virtual eCardTypes GetCardType() const { return eCardMpmRx; }
};
//////MPMRX///////

class  CCardGideonLite : public CCardCfgMfa
{
CLASS_TYPE_1(CCardGideonLite,CCardCfgMfa )
public:
				// Constructors
	CCardGideonLite();
	CCardGideonLite(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm);
	virtual ~CCardGideonLite();
	CCardGideonLite(const CCardGideonLite& other);
	CCardGideonLite& operator= (const CCardGideonLite& other);

				// Initializations

				// Operations
	virtual CCardCfg* CreateCopy();
	virtual eCardTypes GetCardType() const { return eCardGideonLite; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);

protected:
				// Operations

				// Attributes
};


class CBehaviourSwitch : public CSerializeObject
{
CLASS_TYPE_1(CBehaviourSwitch,CSerializeObject)
public:
				// Constructors
	CBehaviourSwitch(const WORD timeout=1/*seconds*/);
	CBehaviourSwitch(const CBehaviourSwitch& rOther);
	virtual ~CBehaviourSwitch();
				// Initializations

				// Operations
	virtual CSerializeObject* Clone() { return NULL; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& rSegment);
	int IsEqual(const CBehaviourSwitch& rOther) const;

	WORD  GetConfigTime() const { return m_wConfigTime; }

protected:
				// Operations

				// Attributes
	WORD		m_wConfigTime;
};


class CBehaviourMfa : public CSerializeObject
{
CLASS_TYPE_1(CBehaviourMfa,CSerializeObject)
public:
				// Constructors
	CBehaviourMfa(	const WORD unitConfigTime=1/*seconds*/,
					const WORD mediaConfigTime=1/*seconds*/,
					const WORD speakerChangeTime=1/*seconds*/);
	CBehaviourMfa(const CBehaviourMfa& rOther);
	virtual ~CBehaviourMfa();
				// Initializations

				// Operations
	virtual CSerializeObject* Clone() { return NULL; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& rSegment);
	int IsEqual(const CBehaviourMfa& rOther) const;

	WORD  GetUnitConfigReqTime() const { return m_wUnitConfigReqTime; }
	WORD  GetMediaConfigReqTime() const { return m_wMediaConfigReqTime; }
	WORD  GetSpeakerChangeTime() const { return m_wSpeakerChangeTime; }

protected:
				// Operations

				// Attributes
	WORD		m_wUnitConfigReqTime;
	WORD		m_wMediaConfigReqTime;
	WORD		m_wSpeakerChangeTime;
};


class  CUnitCfg : public CSerializeObject
{
CLASS_TYPE_1(CUnitCfg, CSerializeObject)
public:
				// Constructors
	CUnitCfg();
	CUnitCfg(WORD unitId, DWORD unitStatus);
	virtual ~CUnitCfg();
	CUnitCfg(const CUnitCfg& other);
	CUnitCfg& operator= (const CUnitCfg& other);

				// Initializations

				// Operations
	
	//virtual CCardCfg* CreateCopy() = 0;
	//virtual eCardTypes GetCardType() const = 0;

				// Utils
	WORD	GetUnitId() const      { return m_wUnitId; }
	DWORD   GetUnitStatus()	const  { return m_wUnitStatus; }

	virtual void  SerializeXml(CXMLDOMElement*& SerializeXml) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	virtual CSerializeObject* Clone() { return NULL; }

protected:
				// Operations

				// Attributes
	WORD	m_wUnitId;
	DWORD	m_wUnitStatus;
};

typedef struct
{
	DWORD timer;
	DWORD opcode;
    DWORD data[1024];
    DWORD dataLen;
} EP_SGN_S;


class CBehaviourIsdn : public CSerializeObject
{
CLASS_TYPE_1(CBehaviourIsdn,CSerializeObject)
public:
				// Constructors
	CBehaviourIsdn();
	CBehaviourIsdn(const CBehaviourIsdn& rOther);
	virtual ~CBehaviourIsdn();

				// Operations
	const char*  NameOf() const { return "CBehaviourIsdn"; }
	virtual CSerializeObject* Clone() { return NULL; }

	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;

	const EP_SGN_S* GetFirstSignal();
	const EP_SGN_S* GetCurrSignal();
	const EP_SGN_S* GetNextSignal();

	BOOL GetCapsFlag() { return m_caps; }
	BOOL GetXmitModeFlag() { return m_xmitMode; }
	BOOL GetH230Flag() { return m_h230; }

 private:
	EP_SGN_S m_sgnList[MAX_NUM_OF_UNITS];
	WORD     m_num;
	int      m_index;

	BOOL     m_caps;
	BOOL     m_xmitMode;
	BOOL     m_h230;
};


#endif /* __GIDEONSIMCONFIG_ */
