// Licensing.h: interface for the CLicensing class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LICENSING_H_
#define LICENSING_H_


#include "PObject.h"
#include "psosxml.h"
#include "NStream.h"
#include "StringsLen.h"
#include "CommonStructs.h"
#include "McuMngrStructs.h"
#include "StructTm.h"
#include "ProductType.h"
#include "licensingServer.h"
#include "CardsStructs.h"
#include "LicensingServerStructs.h"
#include "PlcmLicenseStatus.h"

using namespace std;


 const int PermutationTabl[3][70]={
	
	 {29,28,17,46,44,38,7,14,55,43,37,60,24,41,3,50,19,68,23,65,11,
		58,21,33,54,63,45,16,39,6,61,15,30,5,52,25,0,4,31,20,26,22,
		66,10,48,32,57,62,18,53,47,1,27,13,64,42,59,2,12,40,36,51,49,
		67,9,69,35,8,56,34},

	 {46,69,21,66,8,59,18,42,35,58,34,3,14,52,23,57,65,40,48,9,15,
		31,39,2,61,20,27,55,50,11,28,38,0,63,45,7,43,36,56,64,22,51,
		25,5,16,24,41,33,17,60,47,4,32,53,13,44,68,1,54,26,10,49,30,
		37,29,67,12,62,6,19},
	
	{12,46,67,13,69,31,50,7,61,22,53,6,36,3,29,16,65,59,41,18,56,
		26,1,58,47,32,62,24,63,45,23,35,5,14,44,66,0,54,40,15,28,9,38
		,2,20,42,11,33,57,25,48,8,34,60,30,49,52,21,37,10,68,27,43,4
		,55,64,19,51,39,17},
 };

#define VALIDATION_STRING				"5A5AF1F1"	// as CPObject's validation flag

#define LICENSING_CONFIGURATION         ((std::string)(MCU_MCMS_DIR+"/Cfg/LicensingConfiguration.xml"))

/* 	
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
=-=-=-=-=- Old Licensing code!!! -=-=-=-=-=
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define OLD_KEYCODE_LENGTH	6500	// as in mgc
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
=-=-=-=-=- Old Licensing code!!! -=-=-=-=-=
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


class CLicensing : public CPObject
{
CLASS_TYPE_1(CLicensing, CPObject)


public:
	CLicensing ();
	CLicensing (const CLicensing& other);
	virtual const char* NameOf() const { return "CLicensing";}
	virtual ~CLicensing ();
//	virtual void Dump(ostream&) const;
	CLicensing& operator = (const CLicensing &rOther);

	void InitMembers();

 	void  Serialize(COstrStream &m_ostr);
	void  DeSerialize(CIstrStream &m_istr);
 
  	void  SerializeXml(CXMLDOMElement* pParentNode);
	int	  DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError);

/*
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
=-=-=-=-=- Old Licensing code!!! -=-=-=-=-=
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	STATUS SerializeAndEncrypt(char* DecBufAsInput, char* EncBufAsOutput, BYTE* key);
	STATUS EncryptBuffer(char* DecBufAsInput, char* EncBufAsOutput, BYTE* key = NULL);
	void   EncryptLine(char* buf, COstrStream& m_ostr, int lineNum,BYTE* akey);

	STATUS DecryptAndDeserialize(char* EncBufAsInput, char* DecBufAsOutput, BYTE* key);
	STATUS DecryptBuffer(char* EncBufAsInput, char* DecBufAsOutput, BYTE* key = NULL);
    void   DecryptLine(char* buf, COstrStream& m_ostr, int lineNum,BYTE* akey);
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
=-=-=-=-=- Old Licensing code!!! -=-=-=-=-=
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

	void   CalculateChecksum(char* hexStrChecksum);

	WORD IsValid();
	
	
	const char*	GetEncString();
	void		SetEncString(const char* encBuf);

	const char*	GetDecString();
	void		SetDecString(const char* decBuf);
	
	const char*	GetValidationFlag() const;
	
	const char*	GetMplSerialNum() const;
	void		SetMplSerialNum(const char* serialNum);

    BYTE*    	GetDongleHexSerialNum();
	void		SetDongleHexSerialNum(BYTE* hexSerialNum);
	
	const char*	GetUpgradeFrom() const;
	void		SetUpgradeFrom(const char* oldConfigurationName);
	
	const char*	GetConfigurationName() const;
	void		SetConfigurationName(const char* name);

//	const char*	GetMcuVerNum() const;
//	void		SetMcuVerNum(const char* num);

	const char*	GetDongleUtilityVerNum() const;
	void		SetDongleUtilityVerNum(const char* num);
	
	const VERSION_S	GetMcuVersion() const;
	void			SetMcuVersion(const VERSION_S ver,const char* desc);

	DWORD		GetTotalNumOfCopParties() const;
	void		SetTotalNumOfCopParties(const DWORD num);

	DWORD GetTotalNumOfSvcParties() const;
	void SetTotalNumOfSvcParties(DWORD num);

	DWORD		GetTotalNumOfCpParties() const;
	void		SetTotalNumOfCpParties(const DWORD num);

    BYTE		GetIsMPMXBitEnabled() const;
	void		SetIsMPMXBitEnabled(const BYTE isEnabled);

	BYTE GetIsSvcEnabled() const;
	void SetIsSvcEnabled(BYTE isEnabled);

	BYTE GetIsTipEnabled() const;
	void SetIsTipEnabled(BYTE isEnabled);
	BYTE GetIsAvcCifPlusEnabled() const;
	void SetIsAvcCifPlusEnabled(BYTE isEnabled);
	BYTE GetIsRppEnabled() const;
	void SetIsRppEnabled(BYTE isEnabled);



	BYTE GetIsHdPortsUnit() const;
	void SetIsHdPortsUnit(const BYTE isEnabled);

    BYTE		GetIsEncryptionEnabled() const;
	void		SetIsEncryptionEnabled(const BYTE isEnabled);

    BYTE		GetIsPstnEnabled() const;
	void		SetIsPstnEnabled(const BYTE isEnabled);
	
    BYTE		GetIsTelepresenceEnabled() const;
	void		SetIsTelepresenceEnabled(const BYTE isEnabled);
	
//    const BYTE	GetIsInternalSchedulerEnabled() const;
//	void		SetIsInternalSchedulerEnabled(const BYTE isEnabled);
	
//    const BYTE	GetIsMsEnabled() const;
//	void		SetIsMsEnabled(const BYTE isEnabled);
	
	BYTE	    GetIsMultipleServicesEnabled() const;
	void 		SetIsMultipleServicesEnabled(const BYTE isNetworkSeparationEnabled);

	BYTE	    GetIsHDEnabled(void) const;
	void		SetIsHDEnabled(BYTE isHDEnabled);

    BYTE		GetIsAlcatel() const;
	void		SetIsAlcatel(const BYTE isAlcatel);

    BYTE		GetIsAvaya() const;
	void		SetIsAvaya(const BYTE isAvaya);

    BYTE		GetIsEricsson() const;
	void		SetIsEricsson(const BYTE isEricsson);

    BYTE		GetIsMicrosoft() const;
	void		SetIsMicrosoft(const BYTE isMicrosoft);

    BYTE		GetIsNortel() const;
    void		SetIsNortel(const BYTE isNortel);

    BYTE	    GetIsIBM() const;
    void        SetIsIBM(const BYTE isIBM);

    const std::string GetBiosDetails();
    void		SetBiosDetails(const std::string stBiosDetails);
    void		SetCpuInfo(const std::string stCpuInfo);


    CLicensingServer *       GetLicenseServerParams();
    void        SetLicenseServerParams(CLicensingServer * licenseServerParams);
    void        SetExpirationDate(const CStructTm &other);
    CStructTm   GetExpirationDate(void);


    void SetLastSuccesfulDate(const CStructTm &other);
    CStructTm  GetLastSuccesfulDate(void);

    std::string  GetLastSuccesfulDateAsStr(void);
    std::string  GetLastAttemptDateAsStr(void);

    std::string  GetTimeStr(CStructTm curTime);

    void SetLastAttemptDate(const CStructTm &other);
    CStructTm  GetLastAttemptDate(void);

    void SetConnectionStatus(eLicensingConnectionStatus connStatus);
    eLicensingConnectionStatus  GetConnectionStatus(void);

    void SetLicenseStatus(eLicensingStatus connStatus);
    eLicensingStatus  GetLicenseStatus(void);

    eLicenseMode  GetLicenseMode(void) const;



    eSystemCardsMode	        GetSystemCardsMode() const;
    void                        SetSystemCardsMode(eSystemCardsMode theMode);

    ConnectionStatus            ConvertConnectionStatus();
    LicensingStatus             ConvertLicenseStatus();

protected:
	char	m_validationFlag[VALIDATION_STRING_LEN+1];
	
	char	m_mplSerialNumber[MPL_SERIAL_NUM_LEN];
	BYTE    m_dongleHexSerialNumber[DONGLE_HEX_SERIAL_NUM_LEN];//not in Serialize DeSerialize
	
	char	m_upgradeFrom[CONFIGURATION_NAME_LEN];
	
	char	m_configurationName[CONFIGURATION_NAME_LEN];
//	char    m_mcuVerNum[VER_NUM_LEN];
	char    m_dongleUtilityVerNum[VER_NUM_LEN];
	
	std::string	m_stBiosDetails;

	VERSION_S m_mcuVersion;
	char m_mcuDescription[DESCRIPTION_LEN];	

//	WORD	m_dongleApiNum;
	
	// num of participants
	DWORD	m_totalNumOfCopParties;
	DWORD m_totalNumOfSvcParties;
	DWORD	m_totalNumOfCpParties;
	
	// MPMX bit
	BYTE    m_isMPMXBitEnabled;

	BYTE	m_isSvcEnabled;

	//only for eProductTypeEdgeAxis (Flexera license)
	BYTE	m_isTipEnabled;
	BYTE	m_isAvcCifPlusEnabled;
	BYTE	m_isRppEnabled;

	BYTE	m_isHdPortsUnit;

	// other limitations
	BYTE    m_isEncryptionEnabled;
	BYTE    m_isPstnEnabled;
	BYTE    m_isTelepresenceEnabled;
//	BYTE    m_isInternalSchedulerEnabled;
//	BYTE    m_isMsEnabled;
	BYTE	m_isMultipleServicesEnabled;
	BYTE    m_isHDEnabled;

	// partners
	BYTE    m_isAlcatel;
	BYTE    m_isAvaya;
	BYTE    m_isEricsson;
	BYTE    m_isMicrosoft;
	BYTE    m_isNortel;
    BYTE    m_isIBM;
    string    m_CpuInfo;
   // std::string                 m_primaryLicenseServer;
   // DWORD                       m_primaryLicenseServerPort;
    CLicensingServer *            m_flexeraLicensingInformation;
    CStructTm                     m_expirationDate;

    //EE-560
    eLicensingConnectionStatus    m_licensingConnectionStatus;
    eLicensingStatus              m_licensingStatus;
    CStructTm                     m_lastSuccessDate;
    CStructTm                     m_lastAttemptDate;
    ///////////////////////////////////////////////////////

    eProductType                m_productType;
    CMcuMngrProcess*	m_pProcess;
    eSystemCardsMode	        m_rmxSystemCardsMode;
};





#endif /*LICENSING_H_*/
