// CommEndpointsSimSet.h:
//
//
//Date         Updated By         Description
//
//21/11/05	     Vasily 		Used in XML transaction.
//========   ==============   =====================================================================



#ifndef __COMMENDPOINTSSIMSET_H_
#define __COMMENDPOINTSSIMSET_H_

#include "SerializeObject.h"
#include "StringsLen.h"
#include "MrcStructs.h"
//#include "DataTypes.h"


//////////////////////////////////////////////////////////////////////
//		USE IN 'DEL H323 PARTY' TRANSACTION
//		USE IN 'CONNECT H323 PARTY' TRANSACTION
//		USE IN 'DISCONNECT H323 PARTY' TRANSACTION
//		USE IN 'DEL SIP PARTY' TRANSACTION
//		USE IN 'CONNECT SIP PARTY' TRANSACTION
//		USE IN 'DISCONNECT SIP PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetPartyCommon : public CSerializeObject
{
CLASS_TYPE_1(CCommSetPartyCommon,CSerializeObject)
public:
	CCommSetPartyCommon();
	CCommSetPartyCommon(const CCommSetPartyCommon& other);
	virtual ~CCommSetPartyCommon();
	CCommSetPartyCommon& operator =(const CCommSetPartyCommon& other);
	virtual CSerializeObject* Clone() { return new CCommSetPartyCommon; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetPartyName() { return m_szPartyName; }
	void SetPartyName(char *szPartyName) { 
            strncpy(m_szPartyName, szPartyName, H243_NAME_LEN - 1); 
            m_szPartyName[H243_NAME_LEN - 1] = '\0'; 
    }

protected:
	char  m_szPartyName[H243_NAME_LEN];
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'SIP_MUTE' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetPartyMute : public CCommSetPartyCommon
{
CLASS_TYPE_1(CCommSetPartyMute,CCommSetPartyCommon)
public:
	CCommSetPartyMute();
	CCommSetPartyMute(const CCommSetPartyMute& other);
	virtual ~CCommSetPartyMute();
	CCommSetPartyMute& operator =(const CCommSetPartyMute& other);
	virtual CSerializeObject* Clone() { return new CCommSetPartyMute; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	WORD GetMuteAudioByPort() { return m_wMuteAudioByPort; }
	WORD GetMuteVideoByPort() { return m_wMuteVideoByPort; }
	const char* GetMuteAudioByDirection() { return m_szMuteAudioByDirection; }
	const char* GetMuteVideoByDirection() { return m_szMuteVideoByDirection; }
	const char* GetMuteAudioByInactive() { return m_szMuteAudioByInactive; }
	const char* GetMuteVideoByInactive() { return m_szMuteVideoByInactive; }

protected:
	WORD  m_wMuteAudioByPort;
	WORD  m_wMuteVideoByPort;
	char  m_szMuteAudioByDirection[H243_NAME_LEN];
	char  m_szMuteVideoByDirection[H243_NAME_LEN];
	char  m_szMuteAudioByInactive[H243_NAME_LEN];
	char  m_szMuteVideoByInactive[H243_NAME_LEN];
};


//////////////////////////////////////////////////////////////////////
//		USE IN 'ADD H323 PARTY' TRANSACTION
//		USE IN 'ADD SIP PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetPartyAdd : public CCommSetPartyCommon
{
CLASS_TYPE_1(CCommSetPartyAdd,CCommSetPartyCommon)
public:
	CCommSetPartyAdd();
	CCommSetPartyAdd(const CCommSetPartyAdd& other);
	virtual ~CCommSetPartyAdd();
	CCommSetPartyAdd& operator =(const CCommSetPartyAdd& other);
	virtual CSerializeObject* Clone() { return new CCommSetPartyAdd; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetConfName() { return m_szConfName; }
	const char* GetCapsetName() { return m_szCapSetName; }

	void 	SetIpVersion(DWORD ipVer) {m_ipVersion = ipVer;}
	DWORD	GetIpVersion() {return m_ipVersion;}
	DWORD  GetCSID(void) const;
	char*  GetManufacturer() { return m_szManufacturer;}
	char*  GetAliasName() { return m_sourcePartyAlias;}

protected:
	char  m_szConfName[H243_NAME_LEN];
	char  m_szCapSetName[H243_NAME_LEN];
	DWORD m_ipVersion;
	DWORD m_csID;
	char  m_szManufacturer[H243_NAME_LEN];
	char  m_sourcePartyAlias[H243_NAME_LEN];
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'ADD ISDN PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetAddIsdnEndpoint : public CCommSetPartyCommon
{
CLASS_TYPE_1(CCommSetAddIsdnEndpoint,CCommSetPartyCommon)
public:
	CCommSetAddIsdnEndpoint();
	CCommSetAddIsdnEndpoint(const CCommSetAddIsdnEndpoint& other);
	virtual ~CCommSetAddIsdnEndpoint();
	CCommSetAddIsdnEndpoint& operator =(const CCommSetAddIsdnEndpoint& other);
	virtual CSerializeObject* Clone() { return new CCommSetAddIsdnEndpoint; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetPhoneNum() { return m_szPhoneNum; }
	const char* GetCapsetName() { return m_szCapSetName; }
	DWORD GetNumberOfChannels() { return m_numberOfChannels; }

protected:
	char  m_szCapSetName[H243_NAME_LEN];
	char  m_szPhoneNum[H243_NAME_LEN];
	DWORD m_numberOfChannels;
};


/////////////////////////////////////
class CCommSetPartyAddSipEP : public CCommSetPartyAdd
{
CLASS_TYPE_1(CCommSetPartyAddSipEP,CCommSetPartyAdd)
public:
	CCommSetPartyAddSipEP();
	CCommSetPartyAddSipEP(const CCommSetPartyAddSipEP& other);
	CCommSetPartyAddSipEP& operator= (const CCommSetPartyAddSipEP& other);
	virtual ~CCommSetPartyAddSipEP();
	virtual CSerializeObject* Clone() { return new CCommSetPartyAddSipEP; }
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const{};
	const char* GetPartyUserAgent() { return m_szUserAgent; }



protected:
	char  m_szUserAgent[IP_STRING_LEN];
};



//////////////////////////////////////////////////////////////////////
//		USE IN 'H323 PARTY SENDS DTMF' TRANSACTION
//		USE IN 'SIP PARTY SENDS DTMF' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetPartyDtmf : public CCommSetPartyCommon
{
CLASS_TYPE_1(CCommSetPartyDtmf,CCommSetPartyCommon)
public:
	CCommSetPartyDtmf();
	CCommSetPartyDtmf(const CCommSetPartyDtmf& other);
	virtual ~CCommSetPartyDtmf();
	CCommSetPartyDtmf& operator =(const CCommSetPartyDtmf& other);
	virtual CSerializeObject* Clone() { return new CCommSetPartyDtmf; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetDtmfString() { return m_szDtmfString; }
	WORD GetDtmfSource() { return m_wDtmfSource; }

protected:
	char 	m_szDtmfString[H243_NAME_LEN];
	WORD	m_wDtmfSource;
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'ENDPOINT_UPDATE_CHANNELS' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetPartyUpdateChannels : public CCommSetPartyCommon
{
CLASS_TYPE_1(CCommSetPartyUpdateChannels,CCommSetPartyCommon)
public:
	CCommSetPartyUpdateChannels();
	CCommSetPartyUpdateChannels(const CCommSetPartyUpdateChannels& other);
	virtual ~CCommSetPartyUpdateChannels();
	CCommSetPartyUpdateChannels& operator =(const CCommSetPartyUpdateChannels& other);
	virtual CSerializeObject* Clone() { return new CCommSetPartyUpdateChannels; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	BOOL GetAudioChannelOpen() { return m_isAudioChannelOpen; }
	BOOL GetVideoChannelOpen() { return m_isVideoChannelOpen; }
	BOOL GetFeccChannelOpen() { return m_isFeccChannelOpen; }
	BOOL GetH239ChannelOpen() { return m_isH239ChannelOpen; }

	BYTE GetRecapMode() const { return m_nRecapMode; }

	DWORD GetCapSetID() { return m_iCapId; }
	const char* GetManufacturer() { return m_szManufacturer; }

protected:
	BOOL  m_isAudioChannelOpen;
	BOOL  m_isVideoChannelOpen;
	BOOL  m_isFeccChannelOpen;
	BOOL  m_isH239ChannelOpen;
	BYTE  m_nRecapMode;
	DWORD m_iCapId;
	char  m_szManufacturer[H243_NAME_LEN];
};




//////////////////////////////////////////////////////////////////////
//		USE IN 'H323_PARTY_BITRATE_ERROR' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetBitRateError : public CSerializeObject
{
CLASS_TYPE_1(CCommSetBitRateError,CSerializeObject)
public:
	CCommSetBitRateError();
	CCommSetBitRateError(const CCommSetBitRateError& other);
	virtual ~CCommSetBitRateError();
	CCommSetBitRateError& operator =(const CCommSetBitRateError& other);
	virtual CSerializeObject* Clone() { return new CCommSetBitRateError; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	DWORD GetErrorBitRateVal() { return m_errorBitRate; }

protected:
	DWORD	m_errorBitRate;
};


//////////////////////////////////////////////////////////////////////
//		USE IN 'ADD_CAP_SET' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetAddCapset : public CSerializeObject
{
CLASS_TYPE_1(CCommSetAddCapset,CSerializeObject)
public:
	CCommSetAddCapset();
	CCommSetAddCapset(const CCommSetAddCapset& other);
	virtual ~CCommSetAddCapset();
	CCommSetAddCapset& operator =(const CCommSetAddCapset& other);
	virtual CSerializeObject* Clone() { return new CCommSetAddCapset; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetName() { return m_szCapsetName; }
	WORD  GetRate() { return m_nCallRate; }
	BOOL  GetFecc() { return m_fecc; }
	BOOL  GetH239() { return m_h239; }
	BOOL  GetEncrypted() { return m_encrypted; }
	BOOL  GetAudioG711() { return m_g711; }
	BOOL  GetAudioG722() { return m_g722; }
	BOOL  GetAudioG7221() { return m_g7221; }
	BOOL  GetAudioG7221AnnexC() { return m_g7221C; }
	BOOL  GetAudioG7231() { return m_g7231; }
	BOOL  GetAudioG728() { return m_g728; }
	BOOL  GetAudioG729() { return m_g729; }
	BOOL  GetAudioSiren7() { return m_siren7; }
	BOOL  GetAudioSiren14() { return m_siren14; }
	BOOL  GetAudioOpus() { return m_opus; }
	BOOL  GetAudioAACLD() {return m_AAC_LD;};
	BOOL  GetVideoH264() { return m_h264; }
	WORD  GetVideoModeH264() { return m_mode264; }
	DWORD GetH264AspectRatio() { return m_aspectRatio; }
	DWORD GetH264StaticMB() { return m_staticMB; }
	BOOL  GetVideoH263() { return m_h263; }
	BOOL  GetVideoVP8() { return m_VP8; } //N.A. DEBUG VP8
//	WORD GetDtmfSource() { return m_wDtmfSource; }

protected:
	char 	m_szCapsetName[H243_NAME_LEN];
	WORD	m_nCallRate;
	BOOL	m_fecc;
	BOOL	m_encrypted;
	BOOL	m_h239;
	BOOL	m_g711;
	BOOL	m_g722;
	BOOL	m_g7221;
	BOOL	m_g7221C;
	BOOL	m_g7231;
	BOOL	m_g728;
	BOOL	m_g729;
	BOOL	m_siren7;
	BOOL	m_siren14;
	BOOL	m_opus;
	BOOL	m_AAC_LD;
	BOOL	m_h264;
	BOOL	m_VP8; //N.A. DEBUG VP8
	WORD	m_mode264;
	DWORD	m_staticMB;
	DWORD	m_aspectRatio;
	BOOL	m_h263;
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'REQUEST_CONF_DETAILS' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetExternalDbCreate : public CSerializeObject
{
CLASS_TYPE_1(CCommSetExternalDbCreate,CSerializeObject)
public:
	CCommSetExternalDbCreate();
	CCommSetExternalDbCreate(const CCommSetExternalDbCreate& other);
	virtual ~CCommSetExternalDbCreate();
	CCommSetExternalDbCreate& operator =(const CCommSetExternalDbCreate& other);
	virtual CSerializeObject* Clone() { return new CCommSetExternalDbCreate; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetNumericId() const { return m_in_szNumericId; }

	void SetName(const char* pszNewName);
	void SetMaxParties(const WORD wMaxParties);
	void SetMinParties(const WORD wMinParties);
	void SetPassword(const char* pszNewPwd);
	void SetEntryPassword(const char* pszNewPwd);
	void SetBillingData(const char* pszNewBillData);
	void SetOwner(const char* pszNewOwner);
	void SetContactInfo(const int ind,const char* pszInfo);
	void SetDisplayName(const char* pszNewDisplayName);

protected:
		// input parameters
	char	m_in_szNumericId[H243_NAME_LEN];
	char	m_in_szPhone1[H243_NAME_LEN];
	char	m_in_szPhone2[H243_NAME_LEN];
		// output parameters
	char	m_out_szName[H243_NAME_LEN];
	WORD	m_out_wMaxParties;
	WORD	m_out_wMinParties;
	char	m_out_szPassword[H243_NAME_LEN];
	char	m_out_szEntryPassword[H243_NAME_LEN];
	char	m_out_szBillingData[H243_NAME_LEN];
	char	m_out_szOwner[H243_NAME_LEN];
	char	m_out_aszContactInfo[3][H243_NAME_LEN];
	char	m_out_szDisplayName[H243_NAME_LEN];
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'REQUEST_PARTY_DETAILS' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetExternalDbAdd : public CSerializeObject
{
CLASS_TYPE_1(CCommSetExternalDbAdd,CSerializeObject)
public:
	CCommSetExternalDbAdd();
	CCommSetExternalDbAdd(const CCommSetExternalDbAdd& other);
	virtual ~CCommSetExternalDbAdd();
	CCommSetExternalDbAdd& operator =(const CCommSetExternalDbAdd& other);
	virtual CSerializeObject* Clone() { return new CCommSetExternalDbAdd; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetNumericId() const { return m_in_szNumericId; }

	void SetName(const char* pszNewName);
	void SetLeader(const BOOL bIsLeader);
	void SetVip(const BOOL bIsVip);
	void SetUserInfo(const int ind,const char* pszInfo);

protected:
		// input parameters
	char	m_in_szNumericId[H243_NAME_LEN];
	char	m_in_szPhone1[H243_NAME_LEN];
	char	m_in_szPhone2[H243_NAME_LEN];
	char	m_in_szPassword[H243_NAME_LEN];
	BOOL	m_in_bIsGuest;
	BOOL	m_in_bIsLeader;
		// output parameters
	BOOL	m_out_bIsLeader;
	char	m_out_szName[H243_NAME_LEN];
	BOOL	m_out_bIsVip;
	char	m_out_aszUserInfo[4][H243_NAME_LEN];
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'REQUEST_USER_DETAILS' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetExternalDbUser : public CSerializeObject
{
CLASS_TYPE_1(CCommSetExternalDbUser,CSerializeObject)
public:
	CCommSetExternalDbUser();
	CCommSetExternalDbUser(const CCommSetExternalDbUser& other);
	virtual ~CCommSetExternalDbUser();
	CCommSetExternalDbUser& operator =(const CCommSetExternalDbUser& other);
	virtual CSerializeObject* Clone() { return new CCommSetExternalDbUser; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetUserName() const { return m_in_szUserName; }
	const char* GetUserPassword() const { return m_in_szPassword; }
	const char* GetStationName() const { return m_in_szStationName; }
	const char* GetStationIp() const { return m_in_szStationIp; }

	void SetGroup(const WORD group);

protected:
		// input parameters
	char	m_in_szUserName[H243_NAME_LEN];
	char	m_in_szPassword[H243_NAME_LEN];
	char	m_in_szStationName[H243_NAME_LEN];
	char	m_in_szStationIp[H243_NAME_LEN];
		// output parameters
	WORD	m_out_group;
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'SIP_ADD_SUBSCRIPTION' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetAddSubscription : public CSerializeObject
{
CLASS_TYPE_1(CCommSetAddSubscription,CSerializeObject)
public:
	CCommSetAddSubscription();
	CCommSetAddSubscription(const CCommSetAddSubscription& other);
	virtual ~CCommSetAddSubscription();
	CCommSetAddSubscription& operator =(const CCommSetAddSubscription& other);
	virtual CSerializeObject* Clone() { return new CCommSetAddSubscription(*this); }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetConfName() const { return m_szConfName; }
	const char* GetSubscriber() const { return m_szSubscriber; }
	const char* GetEvent() const { return m_szEvent; }
	WORD  GetExpires() const { return m_wExpires; }

protected:
		// input parameters
	char	m_szConfName[H243_NAME_LEN];
	char	m_szSubscriber[H243_NAME_LEN];
	char	m_szEvent[H243_NAME_LEN];
	WORD	m_wExpires;
//		// output parameters
};


//////////////////////////////////////////////////////////////////////
//		USE IN 'SIP_GET_NOTIFICATION' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetGetNotification : public CSerializeObject
{
CLASS_TYPE_1(CCommSetGetNotification,CSerializeObject)
public:
	CCommSetGetNotification();
	CCommSetGetNotification(const CCommSetGetNotification& other);
	virtual ~CCommSetGetNotification();
	CCommSetGetNotification& operator =(const CCommSetGetNotification& other);
	virtual CSerializeObject* Clone() { return new CCommSetGetNotification(*this); }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetConfName() const { return m_szConfName; }
	const char* GetSubscriber() const { return m_szSubscriber; }
	const char* GetEvent() const { return m_szEvent; }

	void SetSubscriptionNotification(const char* pszNotif);

protected:
		// input parameters
	char	m_szConfName[H243_NAME_LEN];
	char	m_szSubscriber[H243_NAME_LEN];
	char	m_szEvent[H243_NAME_LEN];
		// output parameters
	char*	m_pszSubsciptNotification;
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'LPR_MODE_CHANGE_REQUEST' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetLprModeChangeReq : public CCommSetPartyCommon
	{
	CLASS_TYPE_1(CCommSetLprModeChangeReq,CCommSetPartyCommon)
	public:
		CCommSetLprModeChangeReq();
		CCommSetLprModeChangeReq(const CCommSetLprModeChangeReq& other);
		virtual ~CCommSetLprModeChangeReq();
		CCommSetLprModeChangeReq& operator =(const CCommSetLprModeChangeReq& other);
		virtual const char*  NameOf() const { return "CCommSetLprModeChangeReq"; }
		virtual CSerializeObject* Clone() { return new CCommSetLprModeChangeReq; }
		virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
		virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		APIU32 GetLossProtection() { return m_lossProtection; }
		APIU32 GetMtbf() { return m_mtbf; }
		APIU32 GetCongestionCeiling() { return m_congestionCeiling; }
		APIU32 GetFill() { return m_fill; }
		APIU32 GetModeTimeout() { return m_modeTimeout; }

		void SetLossProtection(APIU32 lossProtection) { m_lossProtection = lossProtection; }
		void SetMtbf(APIU32 mtbf) { m_mtbf = mtbf; }
		void SetCongestionCeiling(APIU32 congestionCeiling) { m_congestionCeiling = congestionCeiling; }
		void SetFill(APIU32 fill) { m_fill = fill; }
		void SetModeTimeout(APIU32 modeTimeout) { m_modeTimeout = modeTimeout; }


	protected:
		APIU32					m_lossProtection;
		APIU32					m_mtbf;
		APIU32					m_congestionCeiling;
		APIU32					m_fill;
		APIU32					m_modeTimeout;
	};

//////////////////////////////////////////////////////////////////////
//		USE IN 'H323 PARTY SENDS FECC' TRANSACTION
//		USE IN 'SIP PARTY SENDS FECC' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetPartyFecc : public CCommSetPartyCommon
{
CLASS_TYPE_1(CCommSetPartyFecc,CCommSetPartyCommon)
public:
	CCommSetPartyFecc();
	CCommSetPartyFecc(const CCommSetPartyFecc& other);
	virtual ~CCommSetPartyFecc();
	CCommSetPartyFecc& operator =(const CCommSetPartyFecc& other);
	virtual const char*  NameOf() const { return "CCommSetPartyFecc"; }
	virtual CSerializeObject* Clone() { return new CCommSetPartyFecc; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetFeccString() { return m_szFeccString; }

protected:
	char 	m_szFeccString[H243_NAME_LEN];
};

////////////////////////////////////////////////
//		USE IN 'DEL CAP SET' TRANSACTION
////////////////////////////////////////////////

class CCommSetDelCapset : public CSerializeObject
{
CLASS_TYPE_1(CCommSetDelCapset,CSerializeObject)
public:
	CCommSetDelCapset();
	CCommSetDelCapset(const CCommSetDelCapset& other);
	virtual ~CCommSetDelCapset();
	CCommSetDelCapset& operator =(const CCommSetDelCapset& other);
	virtual CSerializeObject* Clone() { return new CCommSetDelCapset; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetName() { return m_szCapsetName; }

protected:
	char  m_szCapsetName[H243_NAME_LEN];
};

//////////////////////////////////////////////////////////////////////
//		USE IN 'SCP_STREAMS_REQUEST' TRANSACTION
//////////////////////////////////////////////////////////////////////

class CCommSetScpStreamsRequest : public CSerializeObject
{
CLASS_TYPE_1(CCommSetScpStreamsRequest,CSerializeObject)
public:
    CCommSetScpStreamsRequest();
    CCommSetScpStreamsRequest(const CCommSetScpStreamsRequest& other);
	virtual ~CCommSetScpStreamsRequest();
	CCommSetScpStreamsRequest& operator =(const CCommSetScpStreamsRequest& other);
	virtual CSerializeObject* Clone() { return new CCommSetScpStreamsRequest(*this); }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const char* GetConfName() const { return m_szConfName; }
	const char* GetPartyName() const { return m_szPartyName; }
	WORD GetNumberOfStreams() const {return m_numberOfStreams;}
	const MrmpStreamDesc* GetMrmpStreamDescArray() const {return m_streams;}

protected:
		// input parameters
	static const int MAX_STREAMS = 16;

	char	       m_szConfName[H243_NAME_LEN];
	char	       m_szPartyName[H243_NAME_LEN];
	WORD           m_numberOfStreams;
	MrmpStreamDesc m_streams[MAX_STREAMS];
};


#endif /*__COMMENDPOINTSSIMSET_H_*/
