// Licensing.cpp: implementation of the CLicensing class.
//
//////////////////////////////////////////////////////////////////////


#include "Licensing.h"
#include "StatusesGeneral.h"
#include "Macros.h"
#include "InitCommonStrings.h"
#include "McuMngrDefines.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StructTm.h"
#include "McuMngrProcess.h"

using namespace std;


// ------------------------------------------------------------
CLicensing::CLicensing ()
{
	InitMembers();

	m_productType = CProcessBase::GetProcess()->GetProductType();
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();


}

// ------------------------------------------------------------
CLicensing::CLicensing (const CLicensing& other)
:CPObject(other)
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();

	strncpy(m_validationFlag, other.m_validationFlag, VALIDATION_STRING_LEN);
	m_validationFlag[VALIDATION_STRING_LEN] = '\0';

	for(int i=0;i<DONGLE_HEX_SERIAL_NUM_LEN;i++)
	{
		m_dongleHexSerialNumber[i]=other.m_dongleHexSerialNumber[i];
	}

	strncpy(m_upgradeFrom, other.m_upgradeFrom, CONFIGURATION_NAME_LEN);
	m_upgradeFrom[CONFIGURATION_NAME_LEN-1] = '\0';

	strncpy(m_configurationName, other.m_configurationName, CONFIGURATION_NAME_LEN);
	m_configurationName[CONFIGURATION_NAME_LEN-1] = '\0';

//    strncpy(m_mcuVerNum, other.m_mcuVerNum, VER_NUM_LEN);
//    m_mcuVerNum[VER_NUM_LEN-1] = '\0';

	strncpy(m_dongleUtilityVerNum, other.m_dongleUtilityVerNum, VER_NUM_LEN);
    m_dongleUtilityVerNum[VER_NUM_LEN-1] = '\0';
	
	strncpy(m_mplSerialNumber, other.m_mplSerialNumber, MPL_SERIAL_NUM_LEN );
	
	m_mcuVersion.ver_major    = other.m_mcuVersion.ver_major;
	m_mcuVersion.ver_minor    = other.m_mcuVersion.ver_minor;
	m_mcuVersion.ver_release  = other.m_mcuVersion.ver_release;
	m_mcuVersion.ver_internal = other.m_mcuVersion.ver_internal;
	strncpy(m_mcuDescription, other.m_mcuDescription, DESCRIPTION_LEN-1);
	m_mcuDescription[DESCRIPTION_LEN-1] = '\0';

	m_totalNumOfCopParties			= other.m_totalNumOfCopParties;
	m_totalNumOfSvcParties = other.m_totalNumOfSvcParties;
	m_totalNumOfCpParties			= other.m_totalNumOfCpParties;
	
	m_isMPMXBitEnabled      = other.m_isMPMXBitEnabled;

	m_isSvcEnabled  = other.m_isSvcEnabled;
	if ( m_productType == eProductTypeEdgeAxis && m_pProcess->IsFlexeraLicenseInSysFlag() == true)
	{
		m_isTipEnabled  = other.m_isTipEnabled;
		m_isAvcCifPlusEnabled  = other.m_isAvcCifPlusEnabled;
		m_isRppEnabled  = other.m_isRppEnabled;
	}else{
		m_isTipEnabled = FALSE;
		m_isAvcCifPlusEnabled = FALSE;
		m_isRppEnabled = FALSE;
	}
	m_isHdPortsUnit = other.m_isHdPortsUnit;

	m_isEncryptionEnabled			= other.m_isEncryptionEnabled;
	m_isPstnEnabled					= other.m_isPstnEnabled;
	m_isTelepresenceEnabled			= other.m_isTelepresenceEnabled;
//	m_isInternalSchedulerEnabled	= other.m_isInternalSchedulerEnabled;
//	m_isMsEnabled					= other.m_isMsEnabled;
	m_isMultipleServicesEnabled		= other.m_isMultipleServicesEnabled;
	m_isHDEnabled				    = other.m_isHDEnabled;

	m_isAlcatel = other.m_isAlcatel;
	m_isAvaya				= other.m_isAvaya;
	m_isEricsson			= other.m_isEricsson;
	m_isMicrosoft			= other.m_isMicrosoft;
	m_isNortel				= other.m_isNortel;
    m_isIBM                 = other.m_isIBM;
    m_CpuInfo				= other.m_CpuInfo;

    m_stBiosDetails			= other.m_stBiosDetails;

    m_flexeraLicensingInformation              = other.m_flexeraLicensingInformation;

    m_licensingConnectionStatus = eLicensingConnectionUnknown;
    m_licensingStatus = eLicensingStatusUnknown;


}

// ------------------------------------------------------------
CLicensing::~CLicensing ()
{
	POBJDELETE(m_flexeraLicensingInformation);
}

// ------------------------------------------------------------
/*
void  CLicensing::Dump(ostream& msg) const
{
	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);
	
	msg << "\n\n"
		<< "Licensing::Dump\n"
		<< "---------------\n";

	CPObject::Dump(msg);

	msg	<< setw(20) << "DHCP In Use: " << m_ipV4Struct.isDHCPv4InUse << "\n"
		<< setw(20) << "Ip Address: "  << m_ipV4Struct.iPv4Address   << "\n";

	msg<< "\n\n";
}
*/

// ------------------------------------------------------------

CLicensing& CLicensing::operator = (const CLicensing &rOther)
{
	if (this == &rOther){
		return *this;
	}

	strncpy(m_validationFlag, rOther.m_validationFlag, VALIDATION_STRING_LEN);
	m_validationFlag[VALIDATION_STRING_LEN] = '\0';

	for(int i=0;i<DONGLE_HEX_SERIAL_NUM_LEN;i++)
	{
		m_dongleHexSerialNumber[i]=rOther.m_dongleHexSerialNumber[i];
	}

	strncpy(m_upgradeFrom, rOther.m_upgradeFrom, CONFIGURATION_NAME_LEN);
	m_upgradeFrom[CONFIGURATION_NAME_LEN-1] = '\0';

	strncpy(m_configurationName, rOther.m_configurationName, CONFIGURATION_NAME_LEN);
	m_configurationName[CONFIGURATION_NAME_LEN-1] = '\0';

//    strncpy(m_mcuVerNum, rOther.m_mcuVerNum, VER_NUM_LEN);
//    m_mcuVerNum[VER_NUM_LEN-1] = '\0';


	strncpy(m_dongleUtilityVerNum, rOther.m_dongleUtilityVerNum, VER_NUM_LEN);
    m_dongleUtilityVerNum[VER_NUM_LEN-1] = '\0';
	
	strncpy(m_mplSerialNumber, rOther.m_mplSerialNumber, MPL_SERIAL_NUM_LEN );
	
	m_mcuVersion.ver_major    = rOther.m_mcuVersion.ver_major;
	m_mcuVersion.ver_minor    = rOther.m_mcuVersion.ver_minor;
	m_mcuVersion.ver_release  = rOther.m_mcuVersion.ver_release;
	m_mcuVersion.ver_internal = rOther.m_mcuVersion.ver_internal;
	strncpy(m_mcuDescription, rOther.m_mcuDescription, DESCRIPTION_LEN-1);
	m_mcuDescription[DESCRIPTION_LEN-1] = '\0';

	m_totalNumOfCopParties				= rOther.m_totalNumOfCopParties;
	m_totalNumOfSvcParties = rOther.m_totalNumOfSvcParties;
	m_totalNumOfCpParties				= rOther.m_totalNumOfCpParties;
	
	m_isMPMXBitEnabled      = rOther.m_isMPMXBitEnabled;

	m_isSvcEnabled      = rOther.m_isSvcEnabled;

	if ( m_productType == eProductTypeEdgeAxis && m_pProcess->IsFlexeraLicenseInSysFlag() == true)
	{
		m_isTipEnabled      = rOther.m_isTipEnabled;
		m_isAvcCifPlusEnabled      = rOther.m_isAvcCifPlusEnabled;
		m_isRppEnabled      = rOther.m_isRppEnabled;
	}
	m_isHdPortsUnit     = rOther.m_isHdPortsUnit;

	m_isEncryptionEnabled			= rOther.m_isEncryptionEnabled;
	m_isPstnEnabled					= rOther.m_isPstnEnabled;
	m_isTelepresenceEnabled			= rOther.m_isTelepresenceEnabled;
//	m_isInternalSchedulerEnabled	= rOther.m_isInternalSchedulerEnabled;
//	m_isMsEnabled					= rOther.m_isMsEnabled;
	m_isMultipleServicesEnabled		= rOther.m_isMultipleServicesEnabled;
	m_isHDEnabled				    = rOther.m_isHDEnabled;

	m_isAlcatel = rOther.m_isAlcatel;
	m_isAvaya				= rOther.m_isAvaya;
	m_isEricsson			= rOther.m_isEricsson;
	m_isMicrosoft			= rOther.m_isMicrosoft;
	m_isNortel				= rOther.m_isNortel;
    m_isIBM                 = rOther.m_isIBM;
    m_CpuInfo               = rOther.m_CpuInfo;
    
    m_stBiosDetails			= rOther.m_stBiosDetails;

    m_flexeraLicensingInformation              = rOther.m_flexeraLicensingInformation;

    m_licensingConnectionStatus = rOther.m_licensingConnectionStatus;
    m_licensingStatus = rOther.m_licensingStatus;
    m_productType = rOther.m_productType;
    m_rmxSystemCardsMode = rOther.m_rmxSystemCardsMode;


	return *this;
}

// ------------------------------------------------------------
void CLicensing::InitMembers()
{
	strncpy(m_validationFlag, VALIDATION_STRING, VALIDATION_STRING_LEN);
	m_validationFlag[VALIDATION_STRING_LEN] = '\0';

	for(int i=0;i<DONGLE_HEX_SERIAL_NUM_LEN;i++)
	{
		m_dongleHexSerialNumber[i]=0;
	}

	m_upgradeFrom[0] 		 = '\0';
	m_configurationName[0]	 = '\0';
//    m_mcuVerNum[0]			 = '\0';
    m_dongleUtilityVerNum[0] = '\0';

	m_mplSerialNumber[0]	 = '\0';

	m_mcuVersion.ver_major    = 0;
	m_mcuVersion.ver_minor    = 0;
	m_mcuVersion.ver_release  = 0;
	m_mcuVersion.ver_internal = 0;
	memset(m_mcuDescription, 0, DESCRIPTION_LEN);	
	
	m_totalNumOfCopParties		 = 0;
	m_totalNumOfSvcParties = 0;
	m_totalNumOfCpParties		 = 0;
	
	m_isMPMXBitEnabled  = NO;

	m_isSvcEnabled  = NO;
	m_isTipEnabled  = YES;          //BRIDGE-11854
	m_isAvcCifPlusEnabled  = YES;   //BRIDGE-11854
	m_isRppEnabled  = NO;
	m_isHdPortsUnit = NO;

	m_isEncryptionEnabled		 = NO;
	m_isPstnEnabled				 = NO;
	m_isTelepresenceEnabled		 = NO;
//	m_isInternalSchedulerEnabled = NO;
//	m_isMsEnabled				 = NO;
	m_isMultipleServicesEnabled	 = NO;
	m_isHDEnabled                = NO;

	m_isAlcatel = NO;
	m_isAvaya				 = NO;
	m_isEricsson			 = NO;
	m_isMicrosoft			 = NO;
	m_isNortel				 = NO;
    m_isIBM                  = NO;
    m_CpuInfo				 = "";

    m_stBiosDetails			 = "";

	m_flexeraLicensingInformation = new CLicensingServer;

    m_licensingConnectionStatus = eLicensingConnectionNotAttempt;
    m_licensingStatus = eLicensingStatusValid;
    m_rmxSystemCardsMode = eSystemCardsMode_illegal;


}

// ------------------------------------------------------------
void  CLicensing::Serialize(COstrStream &m_ostr)
{
	m_ostr << m_validationFlag						<< "~";
	m_ostr << m_upgradeFrom							<< "~";
	m_ostr << m_configurationName					<< "~";
//	m_ostr << m_mcuVerNum							<< "~"; 
	m_ostr << m_dongleUtilityVerNum					<< "~"; 	

	m_ostr << m_mcuVersion.ver_major				<< "~";
	m_ostr << m_mcuVersion.ver_minor				<< "~";
	m_ostr << m_mcuVersion.ver_release				<< "~";
	m_ostr << m_mcuVersion.ver_internal				<< "~";

	m_ostr << m_totalNumOfCpParties					<< "~";

	m_ostr << (WORD)m_isMPMXBitEnabled					<< "~";
	m_ostr << (WORD)m_isSvcEnabled					<< "~";


	m_ostr << (WORD)m_isTipEnabled					<< "~";
	m_ostr << (WORD)m_isAvcCifPlusEnabled					<< "~";
	m_ostr << (WORD)m_isRppEnabled					<< "~";


	m_ostr << (WORD)m_isHdPortsUnit					<< "~";

	m_ostr << (WORD)m_isEncryptionEnabled			<< "~";
	m_ostr << (WORD)m_isPstnEnabled					<< "~";
	m_ostr << (WORD)m_isTelepresenceEnabled			<< "~";
//	m_ostr << (WORD)m_isInternalSchedulerEnabled	<< "~";
//	m_ostr << (WORD)m_isMsEnabled					<< "~";
    m_ostr << (WORD)m_isMultipleServicesEnabled		<< "~";
	m_ostr << (WORD)m_isHDEnabled				    << "~";

	m_ostr << (WORD)m_isAlcatel						<< "~";
	m_ostr << (WORD)m_isAvaya						<< "~";
	m_ostr << (WORD)m_isEricsson					<< "~";
	m_ostr << (WORD)m_isMicrosoft					<< "~";
	m_ostr << (WORD)m_isNortel						<< "~";
    m_ostr << (WORD)m_isIBM  						<< "~";
    m_ostr << m_totalNumOfCopParties				<< "~";
    m_ostr << m_totalNumOfSvcParties					<< "~";
    m_ostr << m_CpuInfo 						<< "~";
    m_ostr << m_stBiosDetails						<< "~";

}

// ------------------------------------------------------------
void  CLicensing::DeSerialize(CIstrStream &m_istr)
{
	WORD tmp = 0;
	
//	if (isFromFile==NO)
//		m_istr.ignore(1);

	m_istr.getline(m_validationFlag, VALIDATION_STRING_LEN+2, '~');
	m_istr.getline(m_upgradeFrom, CONFIGURATION_NAME_LEN+1, '~');
	m_istr.getline(m_configurationName, CONFIGURATION_NAME_LEN+1, '~');	
//	m_istr.getline(m_mcuVerNum, VER_NUM_LEN+1, '~');
	m_istr.getline(m_dongleUtilityVerNum, VER_NUM_LEN+1, '~');

	m_istr >> m_mcuVersion.ver_major;
	m_istr.ignore(1);
	m_istr >> m_mcuVersion.ver_minor;
	m_istr.ignore(1);
	m_istr >> m_mcuVersion.ver_release;
	m_istr.ignore(1);
	m_istr >> m_mcuVersion.ver_internal;
	m_istr.ignore(1);

	m_istr >> m_totalNumOfCpParties;
	m_istr.ignore(1);

	m_istr >> tmp;
	m_isMPMXBitEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
	m_isSvcEnabled = (BYTE)tmp;
	m_istr.ignore(1);


	m_istr >> tmp;
	m_isTipEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
	m_isAvcCifPlusEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
	m_isRppEnabled = (BYTE)tmp;
	m_istr.ignore(1);


	m_istr >> tmp;
	m_isHdPortsUnit = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isEncryptionEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
	m_isPstnEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
	m_isTelepresenceEnabled = (BYTE)tmp;
	m_istr.ignore(1);

//	m_istr >> tmp;
//	m_isInternalSchedulerEnabled = (BYTE)tmp;
//	m_istr.ignore(1);

//	m_istr >> tmp;
//	m_isMsEnabled = (BYTE)tmp;
//	m_istr.ignore(1);
	
	m_istr >> tmp;
	m_isMultipleServicesEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isHDEnabled = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isAlcatel = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isAvaya = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isEricsson = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isMicrosoft = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> tmp;
    m_isNortel = (BYTE)tmp;
	m_istr.ignore(1);

    m_istr >> tmp;
    m_isIBM = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr >> m_totalNumOfCopParties;
	m_istr.ignore(1);
	m_istr >> m_totalNumOfSvcParties;
	m_istr.ignore(1);


	//m_CpuInfo & m_stBiosDetails - should be always last in this function
	char tmpStr[MAX_INFO_STRING_LEN];
	m_istr.getline(tmpStr, MAX_INFO_STRING_LEN+2, '~');
	m_CpuInfo = tmpStr;

	m_istr.getline(tmpStr, MAX_INFO_STRING_LEN +2 , '~');
	m_stBiosDetails = tmpStr;


}

// ------------------------------------------------------------
void CLicensing::SerializeXml(CXMLDOMElement* pParentNode)
{
	CXMLDOMElement* pDongleCfgNode = pParentNode->AddChildNode("LICENSING_CONFIGURATION");

	// -----------------  1. common params  -----------------
	CXMLDOMElement* pCommonParamsNode = pDongleCfgNode->AddChildNode("COMMON_LICENSING_PARAMS");
	pCommonParamsNode->AddChildNode("MPL_SERIAL_NUMBER",m_mplSerialNumber);

	CXMLDOMElement* pFlexeraLicensingNode = pDongleCfgNode->AddChildNode("LICENSING_SERVER_CONFIGURATION");
	pFlexeraLicensingNode->AddChildNode("PRIMARY_LICENSE_SERVER",m_flexeraLicensingInformation->m_primaryLicenseServer);
	pFlexeraLicensingNode->AddChildNode("PRIMARY_LICENSE_SERVER_PORT",m_flexeraLicensingInformation->m_primaryLicenseServerPort);
	pFlexeraLicensingNode->AddChildNode("LICENSE_EXPIRATION_DATE",m_expirationDate);



	CXMLDOMElement* pLicensingConnectionStatusNode = pDongleCfgNode->AddChildNode("LICENSING_SERVER_STATUS");
	pLicensingConnectionStatusNode->AddChildNode("LICENSING_CONNECTION_STATUS",m_licensingConnectionStatus);
	pLicensingConnectionStatusNode->AddChildNode("LICENSING_STATUS",m_licensingStatus);
	pLicensingConnectionStatusNode->AddChildNode("LAST_SUCCESSFULL_DATE",m_lastSuccessDate);
	pLicensingConnectionStatusNode->AddChildNode("LAST_ATTEMPT_DATE",m_lastAttemptDate);

	
	// ----------------  2. MCU version params  -----------------
	CXMLDOMElement* pVersionNode = pDongleCfgNode->AddChildNode("MCU_VERSION");
	if (pVersionNode)
	{
		pVersionNode->AddChildNode("MAIN",m_mcuVersion.ver_major);
		pVersionNode->AddChildNode("MAJOR",m_mcuVersion.ver_minor);
		pVersionNode->AddChildNode("MINOR",m_mcuVersion.ver_release);
		pVersionNode->AddChildNode("INTERNAL",m_mcuVersion.ver_internal);
		pVersionNode->AddChildNode("DESCRIPTION", m_mcuDescription);		
	}
	
	// ----------------  3. general params  -----------------
	CXMLDOMElement* pGeneralParamsNode = pDongleCfgNode->AddChildNode("GENERAL_LICENSING_PARAMS");
	pGeneralParamsNode->AddChildNode("TOTAL_NUMBER_OF_PARTICIPANTS",m_totalNumOfCpParties);
	pGeneralParamsNode->AddChildNode("TOTAL_NUMBER_OF_EVENT_MODE_PARTICIPANTS",m_totalNumOfCopParties);
	
	// ------------------  4. attributes  -------------------
	CXMLDOMElement* pAttributesNode = pDongleCfgNode->AddChildNode("LICENSING_ATTRIBUTES");
	pAttributesNode->AddChildNode("MPMX_BIT",m_isMPMXBitEnabled,_BOOL);
	pAttributesNode->AddChildNode("ENCRYPTION",m_isEncryptionEnabled,_BOOL);
	pAttributesNode->AddChildNode("CFS_PSTN_ENABLED",m_isPstnEnabled,_BOOL);
	pAttributesNode->AddChildNode("CFS_TELEPRESENCE_ENABLED",m_isTelepresenceEnabled,_BOOL);

	pAttributesNode->AddChildNode("MULTIPLE_SERVICES",m_isMultipleServicesEnabled, _BOOL);
	pAttributesNode->AddChildNode("SVC_ENABLED",m_isSvcEnabled,_BOOL);



	pAttributesNode->AddChildNode("TIP_ENABLED",m_isTipEnabled,_BOOL);
	pAttributesNode->AddChildNode("AVC_CIF_PLUS_ENABLED",m_isAvcCifPlusEnabled,_BOOL);
	pAttributesNode->AddChildNode("RPP_ENABLED",m_isRppEnabled,_BOOL);


	pAttributesNode->AddChildNode("HD_PORTS_UNIT",m_isHdPortsUnit,_BOOL);

	// TODO (drabkin) add: change transaction API
	// pAttributesNode->AddChildNode("HD", m_isHDEnabled, _BOOL);

	// ------------------  5. partners  -------------------
	CXMLDOMElement* pPartnersNode = pDongleCfgNode->AddChildNode("PARTNERS");

	// TODO (drabkin) remove: change transaction API
	pPartnersNode->AddChildNode("ALCATEL", m_isAlcatel, _BOOL);

	pPartnersNode->AddChildNode("AVAYA",m_isAvaya,_BOOL);
	pPartnersNode->AddChildNode("ERICSSON",m_isEricsson,_BOOL);
	pPartnersNode->AddChildNode("MICROSOFT",m_isMicrosoft,_BOOL);
	pPartnersNode->AddChildNode("NORTEL",m_isNortel,_BOOL);
    pPartnersNode->AddChildNode("IBM",m_isIBM,_BOOL);
	
    // ------------------  5. BIOS  -------------------
    CXMLDOMElement* pBiosNode = pDongleCfgNode->AddChildNode("BIOS_VERSION", m_stBiosDetails.c_str());

    // ------------------  6. Cpu Info  -------------------


    CXMLDOMElement* pCpuInfoNode = pDongleCfgNode->AddChildNode("CPU_INFO", m_CpuInfo.c_str());
	return;
}

/////////////////////////////////////////////////////////////////////////////
int	CLicensing::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError)
{
	int nStatus=STATUS_OK;
	CXMLDOMElement *pDongleCfgNode, *pTempNode,*pFlexeraLicensingNode,*pLicensingServerNode;

	GET_CHILD_NODE(pActionNode, "LICENSING_CONFIGURATION", pDongleCfgNode);
	
	if (pDongleCfgNode)
	{
		
		// -----------------  1. common params  -----------------
		GET_CHILD_NODE(pDongleCfgNode, "COMMON_LICENSING_PARAMS", pTempNode);

		if (pTempNode)
		{	
			GET_VALIDATE_CHILD(pTempNode,"MPL_SERIAL_NUMBER", m_mplSerialNumber,_0_TO_MPL_SERIAL_NUM_LENGTH);

			//GET_VALIDATE_CHILD(pTempNode,"PRIMARY_LICENSE_SERVER", m_primaryLicenseServer,IP_ADDRESS);
			//GET_VALIDATE_CHILD(pTempNode,"PRIMARY_LICENSE_SERVER_PORT", m_primaryLicenseServerPort,_0_TO_DWORD);
//			GET_VALIDATE_CHILD(pTempNode,"VALIDATION_FLAG",m_validationFlag,_0_TO_VALIDATION_STRING_LENGTH);
//			GET_VALIDATE_CHILD(pTempNode,"UPGRADE_FROM",m_upgradeFrom,_0_TO_CONFIGURATION_NAME_LENGTH);
//			GET_VALIDATE_CHILD(pTempNode,"CONFIGURATION_NAME",m_configurationName,_0_TO_CONFIGURATION_NAME_LENGTH);
//			GET_VALIDATE_CHILD(pTempNode,"MCU_VERSION_NUMBER",m_mcuVerNum,_0_TO_VER_NUM_LENGTH);
			//			GET_VALIDATE_CHILD(pTempNode,"DONGLE_UTILITY_VERSION_NUMBER",m_dongleUtilityVerNum,_0_TO_VER_NUM_LENGTH);
		}
		GET_CHILD_NODE(pDongleCfgNode, "LICENSING_SERVER_CONFIGURATION", pFlexeraLicensingNode);
		if (pFlexeraLicensingNode)
		{
			GET_VALIDATE_CHILD(pFlexeraLicensingNode,"PRIMARY_LICENSE_SERVER", m_flexeraLicensingInformation->m_primaryLicenseServer,ONE_LINE_BUFFER_LENGTH);
			GET_VALIDATE_CHILD(pFlexeraLicensingNode,"PRIMARY_LICENSE_SERVER_PORT", &m_flexeraLicensingInformation->m_primaryLicenseServerPort,_0_TO_DWORD);
			GET_VALIDATE_CHILD(pFlexeraLicensingNode,"LICENSE_EXPIRATION_DATE", &m_expirationDate,DATE_TIME);
		}

		GET_CHILD_NODE(pDongleCfgNode, "LICENSING_SERVER_STATUS", pLicensingServerNode);
			if (pLicensingServerNode)
			{
				DWORD temp = 0;
				GET_VALIDATE_CHILD(pLicensingServerNode,"LICENSING_CONNECTION_STATUS", &temp,LICENSING_CONNECTION_STATUS_ENUM);

				m_licensingConnectionStatus = (eLicensingConnectionStatus)temp;

				temp=0;
				GET_VALIDATE_CHILD(pLicensingServerNode,"LICENSING_STATUS", &temp,LICENSING_STATUS_ENUM);

				m_licensingStatus = (eLicensingStatus)temp;

				GET_VALIDATE_CHILD(pLicensingServerNode,"LAST_SUCCESSFULL_DATE", &m_lastSuccessDate,DATE_TIME);
				GET_VALIDATE_CHILD(pLicensingServerNode,"LAST_ATTEMPT_DATE", &m_lastAttemptDate,DATE_TIME);
			}

		// ----------------  3. general params  -----------------
		GET_CHILD_NODE(pDongleCfgNode, "MCU_VERSION", pTempNode);
		if (pTempNode)
		{
			GET_VALIDATE_CHILD(pTempNode,"MAIN",&(m_mcuVersion.ver_major),_0_TO_DWORD);	
			GET_VALIDATE_CHILD(pTempNode,"MAJOR",&(m_mcuVersion.ver_minor),_0_TO_DWORD);	
			GET_VALIDATE_CHILD(pTempNode,"MINOR",&(m_mcuVersion.ver_release),_0_TO_DWORD);	
			GET_VALIDATE_CHILD(pTempNode,"INTERNAL",&(m_mcuVersion.ver_internal),_0_TO_DWORD);	
			GET_VALIDATE_CHILD(pTempNode,"DESCRIPTION", m_mcuDescription, DESCRIPTION_LENGTH);			
		}

		// ----------------  4. general params  -----------------
		GET_CHILD_NODE(pDongleCfgNode, "GENERAL_LICENSING_PARAMS", pTempNode);
		
		if (pTempNode)
		{
			GET_VALIDATE_CHILD(pTempNode,"TOTAL_NUMBER_OF_PARTICIPANTS",&m_totalNumOfCpParties,_0_TO_DWORD);
			GET_VALIDATE_CHILD(pTempNode,"TOTAL_NUMBER_OF_EVENT_MODE_PARTICIPANTS",&m_totalNumOfCopParties,_0_TO_DWORD);
		} // end if (pTempNode)
		
		
		// ------------------  5. attributes  -------------------
		GET_CHILD_NODE(pDongleCfgNode, "LICENSING_ATTRIBUTES", pTempNode);

		if (pTempNode)
		{
			GET_VALIDATE_CHILD(pTempNode,"MPMX_BIT",&m_isMPMXBitEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"ENCRYPTION",&m_isEncryptionEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"CFS_PSTN_ENABLED",&m_isPstnEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"CFS_TELEPRESENCE_ENABLED",&m_isTelepresenceEnabled,_BOOL);
//			GET_VALIDATE_CHILD(pTempNode,"CFS_INTERNAL_SCHEDULER_ENABLED",&m_isInternalSchedulerEnabled,_BOOL);
//			GET_VALIDATE_CHILD(pTempNode,"CFS_MS_ENABLED",&m_isMsEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"MULTIPLE_SERVICES",&m_isMultipleServicesEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"SVC_ENABLED",&m_isSvcEnabled,_BOOL);



			GET_VALIDATE_CHILD(pTempNode,"TIP_ENABLED",&m_isTipEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"AVC_CIF_PLUS_ENABLED",&m_isAvcCifPlusEnabled,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"RPP_ENABLED",&m_isRppEnabled,_BOOL);


			GET_VALIDATE_CHILD(pTempNode,"HD_PORTS_UNIT",&m_isHdPortsUnit,_BOOL);

			// TODO (drabkin) add: change transaction API
			// GET_VALIDATE_CHILD(pTempNode,"HD", &m_isHDEnabled, _BOOL);
		} // end if (pTempNode)
		
		// ------------------  6. partners  -------------------
		GET_CHILD_NODE(pDongleCfgNode, "PARTNERS", pTempNode);

		if (pTempNode)
		{
		    // TODO (drabkin) remove: change transaction API
		    //DWORD dummy;
		    GET_VALIDATE_CHILD(pTempNode,"ALCATEL",&m_isAlcatel,_BOOL);

			GET_VALIDATE_CHILD(pTempNode,"AVAYA",&m_isAvaya,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"ERICSSON",&m_isEricsson,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"MICROSOFT",&m_isMicrosoft,_BOOL);
			GET_VALIDATE_CHILD(pTempNode,"NORTEL",&m_isNortel,_BOOL);
            GET_VALIDATE_CHILD(pTempNode,"IBM",&m_isIBM,_BOOL);
		} // end if (pTempNode)
		
		// ------------------  6. bios  -------------------
		GET_VALIDATE_CHILD(pDongleCfgNode, "BIOS_VERSION", m_stBiosDetails, MAX_INFO_STRING_LEN);

		// ------------------  6. Cpu Info  -------------------
		GET_VALIDATE_CHILD(pDongleCfgNode, "CPU_INFO", m_CpuInfo, MAX_INFO_STRING_LEN);

	} // end if (pDongleCfgNode)

	return STATUS_OK;
}

/*
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
=-=-=-=-=- Old Licensing code!!! -=-=-=-=-=
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// ------------------------------------------------------------
 STATUS CLicensing::SerializeAndEncrypt(char* DecBufAsInput, char* EncBufAsOutput, BYTE* key)
{
	STATUS retStatus = STATUS_OK;

    //char* decBuf = new char[SIZE_STREAM]; 
	//memset(decBuf , '\0', SIZE_STREAM);
	//COtrStream* pOstr1	= new COtrStream(decBuf);
	COstrStream* pOstr	= new COstrStream(DecBufAsInput);
	COstrStream&  m_ostr = *pOstr;
	Serialize(m_ostr);

	retStatus = EncryptBuffer((char*)(m_ostr.str().c_str()), EncBufAsOutput, key);
	
	POBJDELETE(pOstr);
	
	return 	retStatus;
}

// ------------------------------------------------------------
STATUS CLicensing::EncryptBuffer(char* DecBufAsInput, char* EncBufAsOutput, BYTE* key)
{
	STATUS retStatus = STATUS_OK;

	CIstrStream m_istr(DecBufAsInput);
//	ostrstream*  msg1 = new  ostrstream(m_encString,OLD_KEYCODE_LENGTH);
//	COstrStream*  msg1 = new  COstrStream(EncBufAsOutput);
	COstrStream  msg1(EncBufAsOutput);
	COstrStream&  m_ostr = msg1;
	if (key==NULL)
	{
		key = new BYTE[32];
		key [0]=56;
		key [1]=78;
		key [2]=3;
		key [3]=121;
		key [4]=89;
		key [5]=100;
		key [6]=234;
		key [7]=1;
	}

    char *buftoEncstring = new char[OLD_KEYCODE_LENGTH];
    memset(buftoEncstring,' ',OLD_KEYCODE_LENGTH);	
	
	char* buf = new char[BUFFER_SIZE]; 
	char  tmp[BUFFER_SIZE];
	m_ostr << "VERSION 2-569856215523621345 \n";
	for (int lineNum=0;lineNum<50 ;lineNum++)
	{
		memset(buf,' ',BUFFER_SIZE);
		m_istr.getline(buf,BUFFER_SIZE-1,'\n');	//-1 because the last 2 characters suppose to be for ' ' and '\n'
		if (buf[0]=='\0') 
			break;
		// /////     continue;
		if (buf[0]!='\n')
		{
			// removing left whitespaces
			for(;;)
			{
				if (buf[0]!=' ') 
					break;
				else
				{
					strncpy(tmp, buf+1,BUFFER_SIZE);
					strncpy(buf, tmp,BUFFER_SIZE);
				}
			}
		
			EncryptLine(buf,m_ostr,lineNum,key);	
		}
		else
			lineNum--;
	}

	PDELETEA(buf);
//	PDELETE(msg1);
	PDELETEA(buftoEncstring);

	sprintf(EncBufAsOutput, "%s", m_ostr.str().c_str() );
//	memcpy(EncBufAsOutput, m_ostr.str().c_str(), m_ostr.str().length());

	return retStatus;
}

// ------------------------------------------------------------
void CLicensing::EncryptLine(char* buf, COstrStream& m_ostr, int lineNum,BYTE* akey)
{
	char temp;
	char *pdest;
	int location=0,asciVal,letterLocation;
	char* EncBuf = new char[BUFFER_SIZE]; 
	memset(EncBuf,' ',BUFFER_SIZE);
	int length=strlen(buf);
	int TableRow=(((lineNum*akey[3])) % 3); //6 - changed since the length 
	int LetterShift=((((lineNum*akey[4])+(akey[(5+lineNum)%7]))) % 47);
	if (LetterShift==0)
		LetterShift=5;
	char* letters ="Q8OxjU6wITtreKZJqf0bNMA3Hi5RkPaE92zhLFGnuWvcCVpoyBlgSDYdsm7X41";
	
	for (int i =0;i<BUFFER_SIZE-2;i++)
	{
		location=PermutationTabl[TableRow][i];
		if (buf[i] != '\n')
			if (buf[i] != '\0'){					
				if (buf[i] == ' ')
				{
					unsigned char tempCh = (unsigned char)((rand()%128)+128);
					if (tempCh == 0xFF)
						tempCh = 0xBC;

					EncBuf[location]=(char)tempCh;
				}
				else{
					temp=buf[i];
					pdest=(strchr(letters,temp));
					if (pdest){
						letterLocation=(pdest-letters);
						letterLocation=((letterLocation+LetterShift) % 62);//TableRow/LetterShift
						strncpy((EncBuf+location),(letters+letterLocation),1);
					}  
					else{
						asciVal=(((int)buf[i]));
						EncBuf[location]=(char)asciVal;
					}
				}
			}
			else
				buf[i]=' ';
			else
				buf[i]=' ';
	}
	EncBuf[BUFFER_SIZE-2]='\n';
	EncBuf[BUFFER_SIZE-1]='\0';
	//	fputs(EncBuf,encfile);
	m_ostr<<EncBuf;
}

// ------------------------------------------------------------
STATUS CLicensing::DecryptAndDeserialize(char* EncBufAsInput, char* DecBufAsOutput, BYTE* key)
{	
	STATUS retStatus = STATUS_OK;
	
	retStatus = DecryptBuffer(EncBufAsInput, DecBufAsOutput, key);
	
	if (STATUS_OK == retStatus)
	{
		CIstrStream* pIstr = new CIstrStream(DecBufAsOutput); 
		CIstrStream& m_istr=*pIstr;
		DeSerialize(m_istr);
		
		POBJDELETE(pIstr);
	}
}

// ------------------------------------------------------------
STATUS CLicensing::DecryptBuffer(char* EncBufAsInput, char* DecBufAsOutput, BYTE* key)
{
	STATUS retStatus = STATUS_OK;

	CIstrStream	m_istr(EncBufAsInput);
//	COstrStream*  msg1 = new  COstrStream(m_decStringe,OLD_KEYCODE_LENGTH);
//	COstrStream*  msg1 = new  COstrStream(DecBufAsOutput);
	COstrStream  msg1(DecBufAsOutput);
	COstrStream&  m_ostr = msg1;
	if (key==NULL)
	 {key = new BYTE[32];
	  key [0]=56;
	  key [1]=78;
	  key [2]=3;
	  key [3]=121;
	  key [4]=89;
	  key [5]=100;
	  key [6]=234;
	  key [7]=1;

	 }
	
	char* encLine = new char[BUFFER_SIZE+1]; 
	int line=0;
	m_istr.getline(encLine,BUFFER_SIZE+1,'\n');

	for (int lineNum=0;lineNum<50 ;lineNum++)
	{
		m_istr.getline(encLine,BUFFER_SIZE+1,'\n');
		if (encLine[0]=='\0') 
			break;
		
		DecryptLine(encLine,m_ostr,lineNum,key);
	}

    DEALLOCBUFFER(encLine);
 //   PDELETE(msg1);
    
	sprintf(DecBufAsOutput, "%s", m_ostr.str().c_str() );
//	memcpy(DecBufAsOutput, m_ostr.str().c_str(), m_ostr.str().length());

	return retStatus;
}

// ------------------------------------------------------------
void CLicensing::DecryptLine(char* encLine, COstrStream& m_ostr, int lineNum,BYTE* akey)
{
	char* letters ="Q8OxjU6wITtreKZJqf0bNMA3Hi5RkPaE92zhLFGnuWvcCVpoyBlgSDYdsm7X41";
	char *pdest;
	int location=0,letterLocation, i=0;

	char* DecBuf = new char[BUFFER_SIZE+1]; 
	memset(DecBuf,' ',BUFFER_SIZE);

	int length=strlen(encLine);
	int TableRow=(((lineNum*akey[6])) % 3);
	int LetterShift=((((lineNum*akey[4])+(akey[(5+lineNum)%7]))) % 47);
	if (LetterShift==0)
		LetterShift=5;
	
	for (i =0;i<BUFFER_SIZE-2;i++){
		location=((PermutationTabl[TableRow][i]));
		if(encLine[location]!=' '){
			int asciVal=(((int)encLine[location]));
			if ((asciVal>=128)||(asciVal<=(0)))
				DecBuf[i]=' ';
			else{
				pdest=(strchr(letters,encLine[location]));
				if (pdest){
					letterLocation=(pdest-letters);
					letterLocation=((letterLocation-LetterShift+62) % 62);
					strncpy((DecBuf+i),(letters+letterLocation),1);
				}  
				else
					DecBuf[i]=(char)asciVal;
			}
		}
	
		
	}
	if(DecBuf[0]!=' ')
	{
		DecBuf[BUFFER_SIZE-2]=' ';
		DecBuf[BUFFER_SIZE-1]='\n';//71
		DecBuf[BUFFER_SIZE]='\0';//72
	}	
	for (i =0;i<BUFFER_SIZE-2;i++)
	{
		if(DecBuf[i]==' ')
		{
		    DecBuf[i]='\n';//71
		    DecBuf[i+1]='\0';//72
			break;
		}
		
	}
	if (i==BUFFER_SIZE-2)
		DecBuf[BUFFER_SIZE-2]='\0';
	m_ostr << DecBuf;
	DEALLOCBUFFER(DecBuf);                  
}
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
=-=-=-=-=- Old Licensing code!!! -=-=-=-=-=
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

// ------------------------------------------------------------
const char* CLicensing::GetValidationFlag() const
{
	return m_validationFlag;
}

// ------------------------------------------------------------
const char* CLicensing::GetMplSerialNum() const
{
	return m_mplSerialNumber;
}

// ------------------------------------------------------------
void CLicensing::SetMplSerialNum(const char* serialNum)
{
	strncpy( m_mplSerialNumber,
	         serialNum,
	         MPL_SERIAL_NUM_LEN - 1);
	m_mplSerialNumber[MPL_SERIAL_NUM_LEN - 1] = 0;
}

// ------------------------------------------------------------
BYTE*	CLicensing::GetDongleHexSerialNum()
{
	return m_dongleHexSerialNumber;
}

// ------------------------------------------------------------
void CLicensing::SetDongleHexSerialNum(BYTE* hexSerialNum)
{
	for(int i=0;i<8;i++)
	{
		m_dongleHexSerialNumber[i]=hexSerialNum[i];
	}
}

// ------------------------------------------------------------/
const char* CLicensing::GetUpgradeFrom() const
{
	return m_upgradeFrom;
}

// ------------------------------------------------------------/
void CLicensing::CalculateChecksum(char* hexStrChecksum)
{
/*
For ensuring that burning went all well, a checksum is calculated.
The checksum is combuned from validationString, configurationName and all other non-string variables.
All those parameters are inserted into an array of BYTEs,
  then this array is being transferred to an array of DWORDs.
Afterwards, all the DWORDs are united to one, buy a xor operation.
Then the DWORD is being shortened (using mod operation).
At the end, the final WORD is converted to a hex string, which is the output.
*/

/*----------------- 1. put all parameters in an array of BYTEs -----------------*/
 	BYTE checksumStr[DONGLE_PREPARE_CHECKSUM_STRING_LEN];
	memset(checksumStr, '\0', DONGLE_PREPARE_CHECKSUM_STRING_LEN);

	memcpy(checksumStr, m_validationFlag, VALIDATION_STRING_LEN); 

	int len = strlen(m_configurationName);
	memcpy(checksumStr+VALIDATION_STRING_LEN, m_configurationName, len);

	len += VALIDATION_STRING_LEN;

	// WORD parameters must be divided into two separate BYTEs
	checksumStr[len++] = m_totalNumOfCpParties;			// LSB
	if ( (m_totalNumOfCpParties >> 8) != 0 )				// (if MSB are zero, skip them)
		checksumStr[len++] = m_totalNumOfCpParties >> 8;	// MSB

	checksumStr[len++] = m_totalNumOfSvcParties;			// LSB
	if ( (m_totalNumOfSvcParties >> 8) != 0 )				// (if MSB are zero, skip them)
		checksumStr[len++] = m_totalNumOfSvcParties >> 8;	// MSB

	checksumStr[len++] = m_totalNumOfCopParties;			// LSB
	if ( (m_totalNumOfCopParties >> 8) != 0 )				// (if MSB are zero, skip them)
		checksumStr[len++] = m_totalNumOfCopParties >> 8;	// MSB

	checksumStr[len++] = m_isMPMXBitEnabled;
	checksumStr[len++] = m_isSvcEnabled;

	if ( m_productType == eProductTypeEdgeAxis && m_pProcess->IsFlexeraLicenseInSysFlag() == true)
	{
		checksumStr[len++] = m_isTipEnabled;
		checksumStr[len++] = m_isAvcCifPlusEnabled;
	}

	checksumStr[len++] = m_isHdPortsUnit;

    checksumStr[len++] = m_isEncryptionEnabled;
    checksumStr[len++] = m_isPstnEnabled;
    checksumStr[len++] = m_isTelepresenceEnabled;
//    checksumStr[len++] = m_isInternalSchedulerEnabled;
//    checksumStr[len++] = m_isMsEnabled;
    checksumStr[len++] = m_isMultipleServicesEnabled;
    checksumStr[len++] = m_isHDEnabled;

    checksumStr[len++] = m_isAlcatel;
    checksumStr[len++] = m_isAvaya;
    checksumStr[len++] = m_isEricsson;
    checksumStr[len++] = m_isMicrosoft;
    checksumStr[len++] = m_isNortel;


/*----------------- 2. put parameters in an array of DWORDs -----------------*/
	const int numOfDwords = DONGLE_PREPARE_CHECKSUM_STRING_LEN/4;
	DWORD checksumDwords[numOfDwords];
	memcpy( checksumDwords, checksumStr, numOfDwords*4 );


/*----------------- 3. unite to one DWORD by xoring -----------------*/
	DWORD finalChecksumDword = 0;
	for (int i=0; i<numOfDwords; i++)
		finalChecksumDword ^= checksumDwords[i];

/*----------------- 4. shorten the DWORD -----------------*/
	WORD finalChecksumWord = finalChecksumDword % 7919;


/*----------------- 5. out as hex string -----------------*/
	sprintf(hexStrChecksum, "%x", finalChecksumWord);
	hexStrChecksum[DONGLE_FINAL_CHECKSUM_STRING_LEN-1] = '\0';
}



// ------------------------------------------------------------
WORD  CLicensing::IsValid()
{
	WORD stat=STATUS_OK;
	if((strncmp(m_validationFlag,VALIDATION_STRING, VALIDATION_STRING_LEN)))
	{
//		stat = STATUS_FILE_CONFIG_NOT_VALID; 
		stat = STATUS_FAIL;
	}	
	return stat;
}

// ------------------------------------------------------------
void CLicensing::SetUpgradeFrom(const char* oldConfigurationName)
{
	strncpy(m_upgradeFrom, oldConfigurationName, sizeof(m_upgradeFrom) - 1);
	m_upgradeFrom[sizeof(m_upgradeFrom) - 1] = '\0';
}

// ------------------------------------------------------------/
const char* CLicensing::GetConfigurationName() const
{
	return m_configurationName;
}

// ------------------------------------------------------------/
void CLicensing::SetConfigurationName(const char* name)
{
	strncpy(m_configurationName, name, sizeof(m_configurationName) - 1);
	m_configurationName[sizeof(m_configurationName) - 1] = '\0';
}

// ------------------------------------------------------------
/*
void CLicensing::SetMcuVerNum(const char* num)
{
	int len = strlen(num);
	
	strncpy(m_mcuVerNum, num, VER_NUM_LEN);
	if (len >VER_NUM_LEN)
		m_mcuVerNum[VER_NUM_LEN-1] = '\0';

}

// ------------------------------------------------------------
const char*	CLicensing::GetMcuVerNum() const
{
	return m_mcuVerNum;
}
*/
// ------------------------------------------------------------
void CLicensing::SetDongleUtilityVerNum(const char* num)
{

	strncpy(m_dongleUtilityVerNum, num, sizeof(m_dongleUtilityVerNum) - 1);
	m_dongleUtilityVerNum[sizeof(m_dongleUtilityVerNum) - 1] = '\0';

}

// ------------------------------------------------------------
const char*	CLicensing::GetDongleUtilityVerNum() const
{
	return m_dongleUtilityVerNum;
}

// ------------------------------------------------------------
const VERSION_S CLicensing::GetMcuVersion() const
{
	return m_mcuVersion;
}

// ------------------------------------------------------------
void CLicensing::SetMcuVersion(const VERSION_S ver,const char *description)
{
	m_mcuVersion.ver_major    = ver.ver_major;
	m_mcuVersion.ver_minor    = ver.ver_minor;
	m_mcuVersion.ver_release  = ver.ver_release;
	m_mcuVersion.ver_internal = ver.ver_internal;
	strncpy(m_mcuDescription, description, DESCRIPTION_LEN - 1);
	m_mcuDescription[DESCRIPTION_LEN - 1] = '\0';

}

// ------------------------------------------------------------
DWORD CLicensing::GetTotalNumOfCpParties() const
{
	return m_totalNumOfCpParties;
}

// ------------------------------------------------------------
void CLicensing::SetTotalNumOfCpParties(const DWORD num)
{
	m_totalNumOfCpParties = num;
}

// ------------------------------------------------------------
DWORD CLicensing::GetTotalNumOfCopParties() const
{
	return m_totalNumOfCopParties;
}

// ------------------------------------------------------------
void CLicensing::SetTotalNumOfCopParties(const DWORD num)
{
	m_totalNumOfCopParties = num;
}

DWORD CLicensing::GetTotalNumOfSvcParties() const
{
    return m_totalNumOfSvcParties;
}

void CLicensing::SetTotalNumOfSvcParties(DWORD num)
{
    m_totalNumOfSvcParties = num;
}

// ------------------------------------------------------------///
BYTE CLicensing::GetIsEncryptionEnabled() const
{
	return m_isEncryptionEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsEncryptionEnabled(const BYTE isEnabled)
{
	m_isEncryptionEnabled = isEnabled;
}

// ------------------------------------------------------------///
BYTE CLicensing::GetIsMPMXBitEnabled() const
{
	return m_isMPMXBitEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsMPMXBitEnabled(const BYTE isEnabled)
{
	m_isMPMXBitEnabled = isEnabled;
}

BYTE CLicensing::GetIsSvcEnabled() const
{
    return m_isSvcEnabled;
}

void CLicensing::SetIsSvcEnabled(BYTE isEnabled)
{
    m_isSvcEnabled = isEnabled;
}

BYTE CLicensing::GetIsTipEnabled() const
{
    return m_isTipEnabled;
}

void CLicensing::SetIsTipEnabled(BYTE isEnabled)
{
	m_isTipEnabled = isEnabled;
}

BYTE CLicensing::GetIsAvcCifPlusEnabled() const
{
    return m_isAvcCifPlusEnabled;
}

void CLicensing::SetIsAvcCifPlusEnabled(BYTE isEnabled)
{
	m_isAvcCifPlusEnabled = isEnabled;
}


BYTE CLicensing::GetIsRppEnabled() const
{
    return m_isRppEnabled;
}

void CLicensing::SetIsRppEnabled(BYTE isEnabled)
{
	m_isRppEnabled = isEnabled;
}

// ------------------------------------------------------------///
BYTE CLicensing::GetIsHdPortsUnit() const
{
    return m_isHdPortsUnit;
}

void CLicensing::SetIsHdPortsUnit(const BYTE isHdPortsUnit)
{
    m_isHdPortsUnit = isHdPortsUnit;
}

// ------------------------------------------------------------///
BYTE CLicensing::GetIsPstnEnabled() const
{
	return m_isPstnEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsPstnEnabled(const BYTE isEnabled)
{
	m_isPstnEnabled = isEnabled;
}

// ------------------------------------------------------------///
BYTE CLicensing::GetIsTelepresenceEnabled() const
{
	return m_isTelepresenceEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsTelepresenceEnabled(const BYTE isEnabled)
{
	m_isTelepresenceEnabled = isEnabled;
}

// ------------------------------------------------------------///
/*
const BYTE CLicensing::GetIsInternalSchedulerEnabled() const
{
	return m_isInternalSchedulerEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsInternalSchedulerEnabled(const BYTE isEnabled)
{
	m_isInternalSchedulerEnabled = isEnabled;
}
*/

// ------------------------------------------------------------///
//const BYTE CLicensing::GetIsMsEnabled() const
//{
//	return m_isMsEnabled;
//}

// ------------------------------------------------------------
//void CLicensing::SetIsMsEnabled(const BYTE isEnabled)
//{
//	m_isMsEnabled = isEnabled;
//}

// ------------------------------------------------------------///
BYTE CLicensing::GetIsMultipleServicesEnabled() const
{
	return m_isMultipleServicesEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsMultipleServicesEnabled(const BYTE isMultipleServicesEnabled)
{
	m_isMultipleServicesEnabled = isMultipleServicesEnabled;
}

// ------------------------------------------------------------
BYTE CLicensing::GetIsHDEnabled(void) const
{
	return m_isHDEnabled;
}

// ------------------------------------------------------------
void CLicensing::SetIsHDEnabled(BYTE isHDEnabled)
{
	m_isHDEnabled = isHDEnabled;
	//BRIDGE-13569
	m_isAlcatel   = isHDEnabled;
}

// ------------------------------------------------------------
BYTE	CLicensing::GetIsAvaya() const
{
	return m_isAvaya;
}

// ------------------------------------------------------------
void CLicensing::SetIsAvaya(const BYTE isAvaya)
{
	m_isAvaya = isAvaya;
}
// ------------------------------------------------------------
BYTE	CLicensing::GetIsAlcatel() const
{
	return m_isAlcatel;
}

// ------------------------------------------------------------
void CLicensing::SetIsAlcatel(const BYTE isAlcatel)
{
	m_isAlcatel = isAlcatel;
}


// ------------------------------------------------------------
BYTE	CLicensing::GetIsEricsson() const
{
	return m_isEricsson;
}

// ------------------------------------------------------------
void CLicensing::SetIsEricsson(const BYTE isEricsson)
{
	m_isEricsson = isEricsson;
}

// ------------------------------------------------------------
BYTE	CLicensing::GetIsMicrosoft() const
{
	return m_isMicrosoft;
}

// ------------------------------------------------------------
void CLicensing::SetIsMicrosoft(const BYTE isMicrosoft)
{
	m_isMicrosoft = isMicrosoft;
}

// ------------------------------------------------------------
BYTE	CLicensing::GetIsNortel() const
{
	return m_isNortel;
}
// ------------------------------------------------------------
void CLicensing::SetIsNortel(const BYTE isNortel)
{
	m_isNortel = isNortel;
}
// ------------------------------------------------------------
BYTE  CLicensing::GetIsIBM() const
{
    return m_isIBM;
}
// ------------------------------------------------------------
void CLicensing::SetIsIBM(const BYTE isIBM)
{
    m_isIBM = isIBM;
}

// ------------------------------------------------------------
const std::string CLicensing::GetBiosDetails()
{
	return m_stBiosDetails;
}

// ------------------------------------------------------------
void CLicensing::SetBiosDetails(const std::string stBiosDetails)
{
	m_stBiosDetails = stBiosDetails;
	TRACESTR(eLevelInfoNormal) << "\nCLicensing::SetBiosDetails" << m_stBiosDetails;
}
// ------------------------------------------------------------
void CLicensing::SetCpuInfo(const std::string stCpuInfo)
{
	m_CpuInfo = stCpuInfo;
	TRACESTR(eLevelInfoNormal) << "\nCCLicensing::SetCpuInfo" << m_CpuInfo;
}

// ------------------------------------------------------------
void CLicensing::SetLicenseServerParams(CLicensingServer* pLicensingServer)
{
	m_flexeraLicensingInformation->m_primaryLicenseServer = pLicensingServer->m_primaryLicenseServer;
	m_flexeraLicensingInformation->m_primaryLicenseServerPort = pLicensingServer->m_primaryLicenseServerPort;
	m_flexeraLicensingInformation->m_updateCounter = pLicensingServer->m_updateCounter;
	//m_flexeraLicensingInformation->WriteXmlFile(LICENSING_CONFIGURATION.c_str());

}

CLicensingServer*  CLicensing::GetLicenseServerParams()
{
	return m_flexeraLicensingInformation;
}

void CLicensing::SetExpirationDate(const CStructTm &other)
{
	m_expirationDate = other;
}

 CStructTm  CLicensing::GetExpirationDate(void)
{
  return m_expirationDate;
}

 void CLicensing::SetLastSuccesfulDate(const CStructTm &other)
 {
	 m_lastSuccessDate = other;
 }

 CStructTm  CLicensing::GetLastSuccesfulDate(void)
 {
	 return m_lastSuccessDate;
 }


 std::string  CLicensing::GetLastSuccesfulDateAsStr(void)
  {
	 return GetTimeStr(m_lastSuccessDate);


  }


 std::string  CLicensing::GetLastAttemptDateAsStr(void)
   {
	 return GetTimeStr(m_lastAttemptDate);

   }




 void CLicensing::SetLastAttemptDate(const CStructTm &other)
 {
	 m_lastAttemptDate = other;
 }

 CStructTm  CLicensing::GetLastAttemptDate(void)
 {
	 return m_lastAttemptDate;
 }


 void CLicensing::SetConnectionStatus(eLicensingConnectionStatus connStatus)
 {
	 m_licensingConnectionStatus = connStatus;
 }

 eLicensingConnectionStatus  CLicensing::GetConnectionStatus(void)
 {
	 return m_licensingConnectionStatus;
 }


 void CLicensing::SetLicenseStatus(eLicensingStatus connStatus)
 {
	 m_licensingStatus = connStatus;
 }

 eLicensingStatus  CLicensing::GetLicenseStatus(void)
 {
	 return m_licensingStatus;
 }


 eLicenseMode  CLicensing::GetLicenseMode(void) const
{

	if (eProductTypeEdgeAxis == m_productType && m_pProcess->IsFlexeraLicenseInSysFlag() == true)
		return eLicenseMode_flexera;
	return eLicenseMode_cfs;
}

 // ------------------------------------------------------------
 void CLicensing::SetSystemCardsMode(eSystemCardsMode theMode)
 {
	 m_rmxSystemCardsMode = theMode;
 }


 // ------------------------------------------------------------
 eSystemCardsMode CLicensing::GetSystemCardsMode() const
 {
 	return m_rmxSystemCardsMode;
 }


 ConnectionStatus CLicensing::ConvertConnectionStatus()
 {
 	switch(m_licensingConnectionStatus)
 	{
 	case eLicensingConnectionNotAttempt: return eConnectionStatus_CONNECTIONNOTATTEMPTED;
 	case eLicensingConnectionConnecting: return eConnectionStatus_CONNECTING;
 	case eLicensingConnectionSuccess: return eConnectionStatus_CONNECTSUCCESS;
 	case eLicensingConnectionFail: return eConnectionStatus_CONNECTFAILURE;
 	case eLicensingConnectionUnknown: return eConnectionStatus_UNKNOWN;

 	default: return eConnectionStatus_UNKNOWN;
 	}
 }

 LicensingStatus  CLicensing::ConvertLicenseStatus()
  {
  	switch(m_licensingStatus)
  	{
  	case eLicensingStatusValid: return eLicensingStatus_VALID;
  	case eLicensingStatusInvalid: return eLicensingStatus_INVALID;
  	case eLicensingStatusRestartRequired: return eLicensingStatus_VALIDSYSTEMRESTARTREQUIRED;
  	case eLicensingStatusUnknown: return eLicensingStatus_UNKNOWN;

  	default: return eLicensingStatus_INVALID;
  	}
  }



 std::string CLicensing::GetTimeStr(CStructTm curTime)
 {

                 char yearSt[16];
                 char monthSt[16];
                 char daySt[16];
                 char hourSt[16];
                 char minSt[16];
                 char secSt[16];
                 sprintf(yearSt,"%2d",curTime.m_year);
                 sprintf(monthSt,"%02d",(curTime.m_mon));
                 sprintf(daySt,"%02d",curTime.m_day);

                 sprintf(hourSt,"%02d",curTime.m_hour);
                 sprintf(minSt,"%02d",curTime.m_min);
                 sprintf(secSt,"%02d",curTime.m_sec);

                 std::string timeRet = std::string(yearSt) + "-" + std::string(monthSt) + "-" + std::string(daySt) + "T" + std::string(hourSt) + ":" + std::string(minSt) + ":" + std::string(secSt) + ".0Z";
                 return timeRet;

 }



