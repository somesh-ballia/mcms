//GkService.cpp


#include <string>
#include "GKService.h"
#include "GKGlobals.h"
#include "GKManagerUtils.h"
#include "RvCommonDefs.h"
#include "H460_1.h"
#include "SysConfig.h"
#include "GateKeeperCommonParams.h"
#include "DataTypes.h"
#include "ProcessBase.h"
#include "OpcodesMcmsInternal.h"
#include "ServiceConfigList.h"
#include "CSKeys.h"

#define M_Min(a,b)  (((a) < (b)) ? a : b)
//Global functions
void InitAddr(ipAddressStruct *pAddr)
{
	pAddr->addr.v4.ip = 0;
	pAddr->ipVersion  = eIpVersion4;
}

/////////////////////////////////////////////////////////////////////////////
void CopyAddr(ipAddressStruct *pToAddr, ipAddressStruct *pFromAddr)
{
	pToAddr->ipVersion = pFromAddr->ipVersion;
	memcpy((char*)&pToAddr->addr, (char*)&pFromAddr->addr, sizeof(ipAddressIf));
}

/////////////////////////////////////////////////////////////////////////////
void CopyAddr(ipAddressStruct *pToAddr, mcXmlTransportAddress *pFromAddr)
{
	pToAddr->ipVersion = pFromAddr->transAddr.ipVersion;
	memcpy((char*)&pToAddr->addr, (char*)&pFromAddr->transAddr.addr, sizeof(ipAddressIf));
}

/////////////////////////////////////////////////////////////////////////////
void CopyAddr(mcXmlTransportAddress *pToAddr,ipAddressStruct *pFromAddr)
{
	pToAddr->transAddr.ipVersion = pFromAddr->ipVersion;
	memcpy((char*)&pToAddr->transAddr.addr, (char*)&pFromAddr->addr, sizeof(ipAddressIf));
}

/////////////////////////////////////////////////////////////////////////////
void CopyAddr(mcXmlTransportAddress *pToAddr,mcXmlTransportAddress *pFromAddr)
{	
	memcpy((char*)pToAddr, (char*)pFromAddr, sizeof(mcXmlTransportAddress));
}

/////////////////////////////////////////////////////////////////////////////
void SetUnionXml(xmlUnionPropertiesSt *pUnionProp, int type)
{
	pUnionProp->unionType = type;
	pUnionProp->unionSize = sizeof(union ipAddressIf);
}

//-------------------------------------------------------------------
void vGetGkIdentifierConfugured(char * par_pBuffOut, int par_nBuffLen)
{
    char szGkIdentFromSysFlag[127] = "";//"GK_ETS";//TBD from SYS_FLAG: GK_IDENTIFIER

    CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    std::string sKey;
    std::string sGkIdentigier;

    sKey = "GK_IDENTIFIER";
    sysConfig->GetDataByKey(sKey, sGkIdentigier);
    if (sGkIdentigier.size() > 0 )
    {
        sGkIdentigier.copy(szGkIdentFromSysFlag, 126);

        if((NULL != par_pBuffOut)&&(par_nBuffLen > 0))
        {
            memset(par_pBuffOut, '\0', par_nBuffLen);
            strncpy(par_pBuffOut, szGkIdentFromSysFlag, par_nBuffLen-1);
        }
    }
}

//-------------------------------------------------------------------
static int gkConvertChar2Unicode(char* pChar,int length)
{
    int         i,j;
    char        sMultyChar[MaxAddressListSize*2] = "";
    int         maxLength = MaxAddressListSize;

    memset(sMultyChar,'\0', sizeof(sMultyChar));

    if((NULL == pChar)||(length <= 0))
        return -5;

    if ((maxLength < length*2) || (((int)sizeof(sMultyChar)) < length*2)) 
    {
        return -5;
    }

    for(i=0,j=0;i<=length*2;i+=2,j++) {
        sMultyChar[i] = 0;
        sMultyChar[i+1] = pChar[j];
    }

    memcpy(pChar,sMultyChar,(length*2));

    return (length * 2);

} // gkConvertChar2Unicode

//-------------------------------------------------------------------
//  Function name:  gkConvertMultyChar2Char
//  Description:    Some BMP strings use 8 bytes to represent one
//                  letter. Some use all the 16 bytes. This function
//                  converts from 16 to 8 byte letter.
//  Return code:    0
//-------------------------------------------------------------------

static int gkConvertMultyChar2Char(char* pChar,int length)
{
    int         i,j;
    char        sChar[MaxAddressListSize] = "";
    BOOL        bIsNeedToCut    = TRUE;

    for(i=0,j=1; ((j<=length) && (bIsNeedToCut)); i++,j+=2) {
        if(pChar[j-1] == 0)
            sChar[i] = pChar[j];
        else
            bIsNeedToCut = FALSE;
    }

    if(bIsNeedToCut){
        memcpy(pChar,sChar,(length/2));
        pChar[length/2] = '\0';
    }

    return 0;

} // gkConvertMultyChar2Char




//////////////////////////////////////////////////////////////////////
// CAltGk
//////////////////////////////////////////////////////////////////////
CGkAlt::CGkAlt()
{	
	m_altGk.xmlHeader.dynamicType = tblAltGk;
	m_altGk.xmlHeader.dynamicLength = sizeof(alternateGkSt);
	m_altGk.bNeedToRegister = FALSE;
	m_altGk.priority		= -1;
	m_altGk.gkIdentLength	= 0;
	memset(&m_altGk.gkIdent[0],0,MaxIdentifierSize);
}

/////////////////////////////////////////////////////////////////////////////
bool operator<(const CGkAlt& lhs,const CGkAlt& rhs) 
{
	if (lhs.m_altGk.priority < rhs.m_altGk.priority)
    	return TRUE;
    else
    	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CGkService
//////////////////////////////////////////////////////////////////////
CGkService::CGkService(WORD ServiceId)
{
	m_ServiceId = ServiceId;
	m_ServiceName[0]		= '\0';
	memset(m_pAliasList,0,MaxAddressListSize);
	
	for( int i = 0; i< MaxNumberOfAliases; i++ )
		m_pAliasesTypes[i] = 0;
	
	memset(m_prefix,0,PHONE_NUMBER_DIGITS_LEN + 1);
	m_multcast				= TRUE;
	
	for( int j = 0; j< TOTAL_NUM_OF_IP_ADDRESSES; j++ )
		InitAddr(&m_CsIps[j]);
	InitAddr(&m_GkIp);
	InitAddr(&m_alternateGkIpConfigured);
	InitAddr(&m_gkIpConfigured);
	
	m_RegistrationTimeConfigured = MIN_RRQ_INTERVAL;
	
	m_bAreParamsReady   = FALSE;
		
	m_gkIdent[0]         = '\0';
	m_gkIdentLength		 = 0;

	m_epIdent[0]         = '\0';
	m_epIdentLength		 = 0;

	m_regStatus          = eNonRegistered;
    m_RRJCounter         = 0;
	m_discovery          = TRUE;
	m_timeToLive		 = MIN_RRQ_INTERVAL;
	m_gkURQhsRas		 = 0;
	m_eAltGkProcess		 = eNotInProcess;
	m_holdRRQAfterAltEnd = FALSE;	
	m_bIsRegAsGw		 = FALSE;
	m_audioDscp			 = 0;
	m_videoDscp			 = 0;
    m_bIsAvaya			 = FALSE;

	SetAltGkParamsToZero();
    m_pAltGkList = new std::vector<alternateGkSt>;
    
    memset(&m_dnsParams, 0, sizeof(DnsParamsSt));
    m_bOKToSendRAI = TRUE;
    m_service_ip_protocol_types = eIpType_None;
    m_SystemCfgChecked = FALSE;
 
    //----- H.235 GK Authentication -----//
    this->m_H235Params.eEncryptMethodConfiguredScale     =  0;
    this->m_H235Params.eEncryptMethodRequired            = -1;
    this->m_H235Params.nIsAuth                           =  0;
    memset(this->m_H235Params.AuthUserName,'\0', sizeof(this->m_H235Params.AuthUserName));
    memset(this->m_H235Params.AuthPassword,'\0', sizeof(this->m_H235Params.AuthPassword));

    this->m_nLastRejectReason = 0;
    //-----------------------------------//
}

/////////////////////////////////////////////////////////////////////////////
CGkService::~CGkService()
{
	PDELETE(m_pCurAltStruct);

	m_pAltGkList->clear();
	PDELETE(m_pAltGkList);	

	PDELETE(m_pUrqAltGkList); 
	
	m_audioDscp			 = 0;
	m_videoDscp			 = 0;
    m_bIsAvaya			 = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CGkService::GetServiceId() const
{
	return m_ServiceId;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetSeviceId(DWORD serviceId)
{
	m_ServiceId = serviceId;
}

/////////////////////////////////////////////////////////////////////////////
const char* CGkService::GetServiceName() const
{
	return m_ServiceName;
}

/////////////////////////////////////////////////////////////////////////////  
void CGkService::SetServiceName (const char* name)
{
	strncpy(m_ServiceName, name, sizeof(m_ServiceName)  -1);
	m_ServiceName[sizeof(m_ServiceName)  -1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////  
const char *CGkService::GetGkName() const
{
	return m_gkName;
}

/////////////////////////////////////////////////////////////////////////////  
void CGkService::SetGkName (const char* name)
{	
	strncpy(m_gkName, name, sizeof(m_gkName) - 1);
	m_gkName[sizeof(m_gkName) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////  
const char* CGkService::GetAltGkName() const
{
	return m_altGkName;
}

/////////////////////////////////////////////////////////////////////////////  
void CGkService::SetAltGkName (const char* name)
{
	strncpy(m_altGkName, name, sizeof(m_altGkName) - 1);
	m_altGkName[sizeof(m_altGkName) -1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
const char* CGkService::GetAliasList() const
{
	return m_pAliasList;
}

/////////////////////////////////////////////////////////////////////////////
/*
void CGkService::SetAliasList(char *pAliasList)
{
	int len = strlen(pAliasList);

	strncpy(m_pAliasList, pAliasList, MaxAddressListSize);

	if( len > MaxAddressListSize )
		m_pAliasList[MaxAddressListSize-1] = '\0';	
}
*/
	
/////////////////////////////////////////////////////////////////////////////
DWORD* CGkService::GetAliasType()
{
	return m_pAliasesTypes;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetAliasType(DWORD *pAliasType,int len)
{
	int length = M_Min(MaxNumberOfAliases,len);

	for(int i=0; i< length; len++)
		m_pAliasesTypes[i] = pAliasType[i];
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::GetIsGkInService()
{
	return m_bIsGkInService;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetIsGkInService(BYTE bIsGkInService)
{
	m_bIsGkInService = bIsGkInService;	
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::GetIsRegAsGw()
{
	return m_bIsRegAsGw;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetIsRegAsGw(BYTE bIsRegAsGw)
{
	m_bIsRegAsGw = bIsRegAsGw;
}
	
/////////////////////////////////////////////////////////////////////////////
char *CGkService::GetPrefix()
{
	return m_prefix;
}

/////////////////////////////////////////////////////////////////////////////
void  CGkService::SetPrefix(char* pPrefix)
{
 	memset(m_prefix,0,PHONE_NUMBER_DIGITS_LEN+1);
 	// overflow check 
 	if( strlen(pPrefix) > PHONE_NUMBER_DIGITS_LEN )
		PTRACE2INT (eLevelError, "CGkService::SetPrefix - prefix length overflow", strlen(pPrefix));
	DBGPASSERT_AND_RETURN( strlen(pPrefix) > PHONE_NUMBER_DIGITS_LEN );
	strncpy(m_prefix, pPrefix, PHONE_NUMBER_DIGITS_LEN);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CGkService::GetMultcast()
{
	return m_multcast;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetMultcast(BOOL multcast)
{
	m_multcast = m_multcast;
}

/////////////////////////////////////////////////////////////////////////////
// Cs ras address is stored at first position
CIpAddressPtr CGkService::GetCsRasIp() const
{
	return CIpAddress::CreateIpAddress(m_CsIps[0]);
}

/////////////////////////////////////////////////////////////////////////////
CIpAddressPtr CGkService::GetCsIp(int index) const
{
	CIpAddressPtr empty;
	if (index < 0 || index >= TOTAL_NUM_OF_IP_ADDRESSES)
		return empty;
	return CIpAddress::CreateIpAddress(m_CsIps[index]);
}

/////////////////////////////////////////////////////////////////////////////
// returns a pointer to cs ips list
const ipAddressStruct* CGkService::GetCsIpsSt() const
{
	return m_CsIps;
}

/////////////////////////////////////////////////////////////////////////////
// this method gets a pointer to an ipAddressStructs array and its size
// and sets m_CsIps service member array. max size might be TOTAL_NUM_OF_IP_ADDRESSES
void CGkService::SetCsIpsSt(ipAddressStruct* pAddr, int arr_size)
{
	int size = M_Min(arr_size, TOTAL_NUM_OF_IP_ADDRESSES); // avoid overflowing
	for(int i = 0, j=0; i<size ;i++)
	{
		CIpAddressPtr tmpIp;
		tmpIp = CIpAddress::CreateIpAddress(pAddr[i]);
		if(tmpIp.get()) 
		{
			CopyAddr(&m_CsIps[j++],&pAddr[i]);
			tmpIp.reset();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
const ipAddressStruct& CGkService::GetGkIpSt() const
{
	return m_GkIp;
}

/////////////////////////////////////////////////////////////////////////////
CIpAddressPtr CGkService::GetGkIp() const
{
	return CIpAddress::CreateIpAddress(m_GkIp);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::IsIpGk() const
{
	// get gk ip (could return null auto_ptr if structure is empty)
	CIpAddressPtr gkIp = CIpAddress::CreateIpAddress(m_GkIp);

	if( gkIp.get() && !gkIp->isNull() ) return TRUE;
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkIpSt(ipAddressStruct* pAddr)
{
	CopyAddr(&m_GkIp,pAddr);
	// we have a GK ip address. cs signaling addresses order might be set now
	if(IsIpGk())
		SetCsAddressesOrder(m_GkIp);
	
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkIpSt(mcXmlTransportAddress* pAddr)
{
	CopyAddr(&m_GkIp,pAddr);
	// we have a GK ip address. cs signaling addresses order might be set now
	if(IsIpGk())
		SetCsAddressesOrder(m_GkIp);
}


///////////////////////////////////////////////////////////////////////////
const ipAddressStruct& CGkService::GetGkIpConfiguredSt() const
{
	return m_gkIpConfigured;
}

///////////////////////////////////////////////////////////////////////////
CIpAddressPtr CGkService::GetGkIpConfigured() const
{
	return CIpAddress::CreateIpAddress(m_gkIpConfigured);
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkIpStConfigured(ipAddressStruct* pAddr)
{
	CopyAddr(&m_gkIpConfigured, pAddr);
}

/////////////////////////////////////////////////////////////////////////////
const char *CGkService::GetNameGkConfigured() const
{
	return m_gkNameConfigured.GetString();
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetNameGkConfigured(char *pName)
{
	m_gkNameConfigured << pName;	
}

////////////////////////////////////////////////////////////////////////////
CIpAddressPtr CGkService::GetAltGkIpConfigured() const
{
	return CIpAddress::CreateIpAddress(m_alternateGkIpConfigured);
}

/////////////////////////////////////////////////////////////////////////////
ipAddressStruct CGkService::GetAltGkIpStConfigured() const
{
	return m_alternateGkIpConfigured;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetAltGkIpStConfigured(ipAddressStruct *pAddr)
{
	CopyAddr(&m_alternateGkIpConfigured,pAddr);
}

/////////////////////////////////////////////////////////////////////////////
const char *CGkService::GetNameAltGkConfigured() const
{
	return m_altGkNameConfigured.GetString();
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetNameAltGkConfigured(char *pName)
{
	m_altGkNameConfigured << pName;	
}

/////////////////////////////////////////////////////////////////////////////
char *CGkService::GetGkIdent()
{
	if( (m_eAltGkProcess == eNotInProcess) || (m_eAltGkProcess == eStartFromOriginal) )
	{
		return m_gkIdent;
	}
	else if (m_pCurAltStruct != NULL)
	{
		return m_pCurAltStruct->gkIdent;
	}
	else 
	{
		PTRACE(eLevelInfoNormal, "CGkService::GetGkIdent m_pCurAltStruct is NULL, returning NULL");
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkIdent(char *gkIdent,int len)
{	
	int length = M_Min(len,MaxIdentifierSize);
	memcpy(m_gkIdent,gkIdent,length);
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkService::GetGkIdentLen() const
{
	WORD wGkIdentLenth = 0;
	if( (m_eAltGkProcess == eNotInProcess) || (m_eAltGkProcess == eStartFromOriginal) || (m_pCurAltStruct == NULL))
		wGkIdentLenth = m_gkIdentLength;
	else 
		wGkIdentLenth = m_pCurAltStruct->gkIdentLength;
	if (wGkIdentLenth > MaxIdentifierSize)
	{
		PTRACE2INT (eLevelError, "CGkService::GetGkIdentLen - changing to 256 wGkIdentLenth=", wGkIdentLenth);
		wGkIdentLenth = MaxIdentifierSize;
	}
	return wGkIdentLenth;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkIdentLen(WORD len)
{
	int length = M_Min(len,MaxIdentifierSize);
	m_gkIdentLength = length;
}

/////////////////////////////////////////////////////////////////////////////
char *CGkService::GetEpIdent()
{
	return m_epIdent;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetEpIdent(char *epIdent,int len)
{	
	int length = M_Min(len,MaxIdentifierSize);
	memcpy(m_epIdent,epIdent,length);
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkService::GetEpIdentLen() const
{
	return m_epIdentLength;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetEpIdentLen(WORD len)
{
	int length = M_Min(len,MaxIdentifierSize);
	m_epIdentLength = length;
}

/////////////////////////////////////////////////////////////////////////////
eRegistrationStatus CGkService::GetRegStatus() const
{
	return m_regStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetRegStatus(eRegistrationStatus status)
{
	m_regStatus = status;
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkService::GetRRJCounter() const
{
	return m_RRJCounter;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetRRJCounter(WORD counter)
{
	m_RRJCounter = counter;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CGkService::GetDiscovery() const
{
	return m_discovery;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetDiscovery(BOOL discovery)
{
	m_discovery = discovery;
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkService::GetTimeToLive() const
{
	return m_timeToLive;
}
/////////////////////////////////////////////////////////////////////////////
void CGkService::SetTimeToLive(WORD timeToLive)
{
	m_timeToLive = timeToLive;
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkService::GetConfiguredTimeToLive()
{
    return this->m_RegistrationTimeConfigured;
}
int CGkService::GetURQhsRas() const
{
	return m_gkURQhsRas;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetURQhsRas(int hsRas)
{
	m_gkURQhsRas = hsRas;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CGkService::GetIsAvaya() const
{
	return m_bIsAvaya;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetIsAvaya(BOOL bIsAvaya)
{
	m_bIsAvaya = bIsAvaya;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetQosParameters(WORD audioDscp, WORD videoDscp)
{
	m_audioDscp = (BYTE)audioDscp;
	m_videoDscp = (BYTE)videoDscp;
}

void CGkService::GetQosParameters(BYTE& audioDscp, BYTE& videoDscp)
{
	audioDscp = m_audioDscp;
	videoDscp = m_videoDscp;
}
/////////////////////////////////////////////////////////////////////////////


////AltGk/////////
//-----------------
/////////////////////////////////////////////////////////////////////////////
void CGkService::SetAltGkParamsToZero()
{
	m_bAltGkPermanent   = FALSE;
	m_eAltGkProcess     = eNotInProcess;
	m_currAltGkToSearch = 0;
	m_pCurAltStruct     = NULL;
	m_triggerOpcode		= 0;
	m_triggerConnId     = 0;
	m_pUrqAltGkList		= NULL;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::IsDiscoveryRequiredToAltGk()
{
	return TRUE; //temporary
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::IsNeedToRegisterToAltGk()
{
	alternateGkSt pCurStruct = (*m_pAltGkList)[m_currAltGkToSearch];
	if (pCurStruct.bNeedToRegister && m_bAltGkPermanent)
		return TRUE;
	
	//else: there are some exceptionals according to the standard:
	// Standard 1: "If the endpoint has not yet registered with the Gatekeeper or has reinitiated the Gatekeeper discovery process, it shall ignore the needToRegister field in the Alternate Gatekeeper list and assume that the value is TRUE".
	// Standard 2: "A Gatekeeper may send a URQ to an endpoint with a list of Alternate Gatekeepers. The endpoint shall ignore the values of the needToRegister and altGKisPermanent fields and assume that those values are TRUE". 
	if (m_regStatus != eRegister)
		return TRUE;
	
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetAltGkAsPermanent()
{
	if (m_eAltGkProcess == eStartFromOriginal)
		RestoreOriginalGkParams();
	else
	{
		alternateGkSt* altGkSt = GetCurAltGkSt();
		if (altGkSt != NULL)
			SetGkIpSt(&altGkSt->rasAddress);
	
		//update faults, gk id, gk name and gk Ip in the card properties:
		//UpdateCardPropAltGkName(1,TRUE);
	}
	
	m_regStatus	    = eRegister; //if we are here & alt is permanent => we must be registered
	m_gkURQhsRas	= 0;
	m_RRJCounter    = 0;
    m_discovery     = TRUE;
   //m_timeToLive was set in RRQInd
	m_eAltGkProcess = eNotInProcess;

	//update card properties:
	UpdateCardPropGkIp(&m_GkIp, 0);
	
	if ((m_pAltGkList->size() == 1) && GetAltGkIpConfigured().get())
	{//if there is only one alt in list and this is what was configured in service - remove this alt list and from monitoring
		if (*(GetGkIp()) == *(GetAltGkIpConfigured()))	
		{
			if ( *(GetGkIp()) == *(GetAltGkIp(0)) )
			{
				ClearAltGkList(-1);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::RestoreOriginalGkParams()
{
	//primary:
	SetGkIpToZero();	
	CopyAddr(&m_GkIp,&m_gkIpConfigured);
	
	if (!IsIpGk())
	{//check DNS
		CIpAddressPtr ip = CIpAddress::CreateIpAddress(m_dnsParams.primeGkIpFromDns);
		if (ip.get())
			SetGkIpSt(&m_dnsParams.primeGkIpFromDns);
		UpdateCardPropGkIp(&m_GkIp, 0);
		UpdateCardPropPrimaryGkConfiguredName();
	}		
	
	memset (m_gkIdent, '\0', MaxIdentifierSize);
	m_gkIdentLength = 0;

	//alternate:
	int numOfAltGks = m_pAltGkList->size();
	if ( GetAltGkIpConfigured().get() )
		UpdateCardPropGkIp(&m_alternateGkIpConfigured, numOfAltGks);
	if ( GetNameAltGkConfigured() )
		UpdateCardPropAltGkConfiguredName(numOfAltGks);	
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropAltGkIp(ipAddressStruct *pAltGkAddr, int index)
{
	UpdateCardPropGkIp(pAltGkAddr, index + 1);// first index is reserved for primary GK.
}


/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropGkIp(ipAddressStruct *pAltGkAddr, int index)
{
	SetGkIPInPropertiesReqStruct st;
	st.serviceId  = m_ServiceId;
	st.indexToSet = index;
	CopyAddr(&st.gkIp, pAltGkAddr);
		
	CSegment* pRetSeg = new CSegment;
	pRetSeg->Put((BYTE*)&st, sizeof(SetGkIPInPropertiesReqStruct));
	SendReqToCSMngr(pRetSeg, CS_GKMNGR_SET_GK_IP_IN_PROPERTIES_REQ);			
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropPrimaryGkConfiguredName()
{
	UpdateCardPropGkName(0, m_gkNameConfigured);
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropAltGkConfiguredName(int index)
{
	UpdateCardPropGkName(index+1, m_altGkNameConfigured);
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropGkName(int index, CSmallString gkName)
{
	if (gkName.GetStringLength() != 0)
	{
		int len = 0;
		SetGkNameInPropertiesReqStruct st;
		st.serviceId = m_ServiceId;
		st.indexToSet = index;
		
		len = M_Min(gkName.GetStringLength(),H243_NAME_LEN);
		memcpy(st.gkName,gkName.GetString(),len);
		if (len > H243_NAME_LEN)
			st.gkName[H243_NAME_LEN-1] = '\0';
		
		CSegment* pRetSeg = new CSegment;
		pRetSeg->Put((BYTE*)&st, sizeof(SetGkNameInPropertiesReqStruct));
		SendReqToCSMngr(pRetSeg, CS_GKMNGR_SET_GK_NAME_IN_PROPERTIES_REQ);				
	}	
}

////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropAltGkIdent(alternateGkSt *pAltGkSt, int index)
{
	UpdateCardPropGkIdent(pAltGkSt, index+1);
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateCardPropGkIdent(alternateGkSt *pAltGkSt, int index)
{
	SetGkIdInPropertiesReqStruct st;
	st.serviceId = m_ServiceId;
	
	st.indexToSet = index;
	int len = M_Min(pAltGkSt->gkIdentLength,MaxIdentifierSize);
	memset(st.gkId,'\0',sizeof(st.gkId));
	memcpy(st.gkId, pAltGkSt->gkIdent, len);
	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&st, sizeof(SetGkIdInPropertiesReqStruct));
	SendReqToCSMngr(pSeg, CS_GKMNGR_SET_GK_ID_IN_PROPERTIES_REQ);	
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SortAndInsertToAlGkList(alternateGkSt *pAltGkSt)
{
	int currPriority = pAltGkSt->priority;
	
	int size = m_pAltGkList->size();
	if (size == 0)
	{
		std::vector<alternateGkSt >::iterator beginIter = m_pAltGkList->begin();;
		m_pAltGkList->insert(beginIter, *pAltGkSt);
	}
	else
	{
		std::vector<alternateGkSt >::iterator iter;
		alternateGkSt curAlt;
		int newPriority = pAltGkSt->priority;
		BYTE bIsInsert = FALSE;
		
		for (iter = m_pAltGkList->begin(); iter != m_pAltGkList->end() && !bIsInsert; iter++)
		{
			curAlt = *iter;
			if (newPriority < curAlt.priority)
			{
				m_pAltGkList->insert(iter, *pAltGkSt);
				bIsInsert = TRUE;
			}		
		}

		if ( (iter == m_pAltGkList->end()) && !bIsInsert)
			m_pAltGkList->push_back(*pAltGkSt);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::UpdateAltGkList(altGksListSt* pListStFromGk)
{
	alternateGkSt* pCurPtrInGkList = (alternateGkSt*)&(pListStFromGk->altGkSt); //first alt GK struct
	char* pChar;
	CGkAlt* pCurStruct = NULL;
	int numOfAltGks = pListStFromGk->numOfAltGks;

	for (int i = 0; i < numOfAltGks; i++)
	{
		ipAddressStruct ipAdd;
		pCurStruct = new CGkAlt;
		alternateGkSt *pAltGkSt = &pCurStruct->m_altGk;
		
		pAltGkSt->bNeedToRegister = pCurPtrInGkList->bNeedToRegister;
		pAltGkSt->priority        = pCurPtrInGkList->priority;
		pAltGkSt->gkIdentLength   = pCurPtrInGkList->gkIdentLength;
		CopyAddr(&pAltGkSt->rasAddress,&pCurPtrInGkList->rasAddress);
		memcpy (&pAltGkSt->gkIdent[0], &pCurPtrInGkList->gkIdent[0], pCurPtrInGkList->gkIdentLength);
		
		SortAndInsertToAlGkList(pAltGkSt);

		//update card properties:
		CopyAddr(&ipAdd,&pAltGkSt->rasAddress);
		UpdateCardPropAltGkIp(&ipAdd, i);
				
		if (pAltGkSt->gkIdentLength)
			UpdateCardPropAltGkIdent(pAltGkSt,i);
			
		//go to next element:
		pChar = (char*)pCurPtrInGkList;
		pChar += sizeof(alternateGkSt);
		pCurPtrInGkList = (alternateGkSt *) pChar;
		POBJDELETE(pCurStruct);
	}	
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::ClearAltGkList(int index)
{
	if (index == -1) //clear all
		m_pAltGkList->clear();
	ClearGkParamsFromPropertiesReqStruct st;
	st.serviceId	= m_ServiceId;
	st.indexToClear = index; 
	
	CSegment* pRetSeg = new CSegment;
	pRetSeg->Put((BYTE*)&st, sizeof(ClearGkParamsFromPropertiesReqStruct));
	SendReqToCSMngr(pRetSeg, CS_GKMNGR_CLEAR_GK_PARAMS_FROM_PROPERTIES_REQ);		
}

/////////////////////////////////////////////////////////////////////////////
/*void CGkService::HandleEmptyAltGkList()
{
	ClearAltGkList(-1);//we want to clear all
	
	if ((m_pAltGkList->size() == 1) && GetAltGkIpConfigured())//we shouldn't delete from the alt list the alt Gk, which was configured in the service
	{
		if(GetCurAltGkIp() == GetAltGkIpConfigured())
			ClearAltGkList(2);
	}
	else
		ClearAltGkList(-1);//we want to clear all
}*/
/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::IsGkPathNavigator()
{
	WORD gkIdentlength = GetGkIdentLen();
	ALLOCBUFFER(gkIdentBuf, gkIdentlength + 1);
	ConvertGkIdFromBmpString(gkIdentBuf);
	BYTE result = false;

	int lenToCompareForDma = strlen(PN_DMA_IDENT);
	BYTE isDma = (!strncmp(gkIdentBuf, PN_DMA_IDENT, lenToCompareForDma));

	if (!isDma)
	{
		int lenToCompare = strlen(PN_IDENT);
		BYTE result = (!strncmp(gkIdentBuf, PN_IDENT, lenToCompare));
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CGkService::IsGkPathNavigator - DMA GK, not PN");
	}

	DEALLOCBUFFER(gkIdentBuf);

	if (result)
	{
		PTRACE(eLevelInfoNormal, "CGkService::IsGkPathNavigator - PathNavigator GK");
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::ConvertGkIdFromBmpString(char* destBuf)
{
	if( (m_eAltGkProcess == eNotInProcess) || (m_eAltGkProcess == eStartFromOriginal) )
		ConvertIdFromBmpString(&m_gkIdent[0], m_gkIdentLength, destBuf);
	else
		ConvertIdFromBmpString(&m_pCurAltStruct->gkIdent[0], m_pCurAltStruct->gkIdentLength, destBuf);
} 
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGkService::InitAltGkList(altGksListSt* pListStFromGk)
{
	//CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
	BOOL bIsEnableAltGk = 0;
	if( pServiceSysCfg != NULL )
	{
	   std::string key = "ENABLE_ALT_GK";
	   //pSysConfig->GetBOOLDataByKey(key, bIsEnableAltGk);
	   pServiceSysCfg->GetBOOLDataByKey(m_ServiceId, key, bIsEnableAltGk);
	}
	if (!bIsEnableAltGk)
		return;

	int numOfAltGks = pListStFromGk->numOfAltGks;
	PTRACE2INT (eLevelInfoNormal, "CGkService::InitAltGkList - Number of alternate Gk is:  ", numOfAltGks);
	// numOfAltGks = 0 - when the GK didn't send any list at all
	// numOfAltGks = -1 - when the GK sent an Empty list of alternates (list with no values)
	// numOfAltGks > 0 - when the GK actually sent a list of alternates
	if (numOfAltGks == 0 && !IsGkPathNavigator())
	{
		// Path navigator sends no list when it is deleting the alternate list
		PTRACE (eLevelInfoNormal, "CGkService::InitAltGkList - GK didn't change the previous list");
		return;
	}
	if ((numOfAltGks == 0 && IsGkPathNavigator()) ||  numOfAltGks == -1)
	{  //Standard: "If the Gatekeeper wishes to clear the endpoint's list of Alternate Gatekeepers,
		//it shall return an empty list of Alternate Gatekeepers to the endpoint in the RCF message."
		PTRACE2INT (eLevelInfoNormal, "CGkService::InitAltGkList - Empty alt list for service Id ", m_ServiceId);

		//DWORD dwAltGkInServiceIP = GetAltGkIpConfigured();		//we shouldn't delete from the alt list the alt Gk, which was configured in the service
		if ( (m_pAltGkList->size() == 1) && GetAltGkIpConfigured().get() )
		{
			if ( GetAltGkIpConfigured().get() == GetGkIp().get() )
				ClearAltGkList(2);
		}
		else
			ClearAltGkList(-1);
		//return;
	}
	//we always save the last alt list
	if (m_pAltGkList->size() > 0) //Not empty
		ClearAltGkList(-1);	//we want to clear all

	if (numOfAltGks > 0)
		UpdateAltGkList(pListStFromGk);

	if ( (numOfAltGks < 4) && GetAltGkIpConfigured().get() )// => we have 1 more place in card properties:so we can check if an alt gk was configured in service
	{
		//if we havn't moved to the alternate which was configured in service:
		if ( *(GetGkIp()) != *(GetAltGkIpConfigured()) )	
		{	
			UpdateCardPropAltGkIp(&m_alternateGkIpConfigured, numOfAltGks);
			UpdateCardPropAltGkConfiguredName(numOfAltGks);
			numOfAltGks++;
		
			//insert alt from service to alt gk list
			InsertAltGkInServiceToAltList();			
		}
		//here we are in altGK, and no alternates -> return to primary
		else if (numOfAltGks == 0)
		{
			UpdateCardPropAltGkIp(&m_gkIpConfigured, numOfAltGks);
			UpdateCardPropAltGkConfiguredName(numOfAltGks);
			numOfAltGks++;

			//insert primary GK from service to alt gk list
			PTRACE (eLevelInfoNormal, "CGkService::InitAltGkList - Setting configured primary GK as alternate GK");
			InsertAltGkInServiceToAltList(true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::InitAltGkListFromReject(rejectInfoSt* pRejectInfo)
{
	InitAltGkList(&pRejectInfo->altGkList);
	m_bAltGkPermanent = pRejectInfo->bAltGkPermanent;
}

/////////////////////////////////////////////////////////////////////////////////////////
//insert alt from service to alt gk list
void CGkService::InsertAltGkInServiceToAltList(bool is_prim/*=false*/)
{
	int len = sizeof(alternateGkSt);
	CGkAlt* pCurStruct   = new CGkAlt;
	alternateGkSt *pAltGk = &pCurStruct->m_altGk;
	
	pAltGk->bNeedToRegister = TRUE;
	pAltGk->priority        = 99; //highest priority

	//if is_prim is on then we should take the primary GK and make it as alternate
	CIpAddressPtr altGkConfiguredIp = is_prim ? GetGkIpConfigured() : GetAltGkIpConfigured();
	
	// fill up ip address from altGKConfiguredIp
	altGkConfiguredIp->FillIPAddress(pAltGk->rasAddress.transAddr);
	
	pAltGk->rasAddress.transAddr.port 		= GK_RAS_PORT;		
	pAltGk->gkIdentLength   				= 0;
	
	SortAndInsertToAlGkList(pAltGk); //according to priority
	POBJDELETE(pCurStruct);
}

/////////////////////////////////////////////////////////////////////////////
eAltGkProcess CGkService::GetAltGkProcess() const
{
	return m_eAltGkProcess;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetAltGkProcess(eAltGkProcess process)
{
	m_eAltGkProcess = process;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::IsAltGkPermanent() const
{
	return m_bAltGkPermanent;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetAltGkPermanent(BYTE value)
{
	m_bAltGkPermanent = value;
}

/////////////////////////////////////////////////////////////////////////////
int CGkService::GetCurrAltGkToSearch() const
{
	return m_currAltGkToSearch;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetCurrAltGkToSearch(int curAltGk)
{
	m_currAltGkToSearch = curAltGk;
}

/////////////////////////////////////////////////////////////////////////////
CIpAddressPtr CGkService::GetCurAltGkIp() const
{
	return CIpAddress::CreateIpAddress(m_pCurAltStruct->rasAddress.transAddr);	
}

/////////////////////////////////////////////////////////////////////////////
alternateGkSt *CGkService::GetCurAltGkSt()
{
	return m_pCurAltStruct;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetCurAltGkSt(alternateGkSt *pAltGkSt)
{
	PDELETE(m_pCurAltStruct);

	if (pAltGkSt)
	{
		int size = sizeof(alternateGkSt);
		m_pCurAltStruct = new alternateGkSt;
		memcpy(m_pCurAltStruct,pAltGkSt,size);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CGkService::GetSizeAltGkList()
{
	return m_pAltGkList->size();
}

/////////////////////////////////////////////////////////////////////////////
alternateGkSt* CGkService::GetAltGkMember()
{	
	return 	&((*m_pAltGkList)[m_currAltGkToSearch]);
}


/////////////////////////////////////////////////////////////////////////////
CIpAddressPtr CGkService::GetAltGkIp(int index)
{	
	CIpAddressPtr empty;
	int listSize = m_pAltGkList->size();
	if (index >= listSize)
		return empty;
	alternateGkSt* pAltSt = &((*m_pAltGkList)[index]);
	if (pAltSt)
		return CIpAddress::CreateIpAddress(pAltSt->rasAddress.transAddr);
	else
		return empty;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CGkService::GetTriggerOpcode() const
{
	return m_triggerOpcode;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetTriggerOpcode(DWORD opcode)
{
	m_triggerOpcode = opcode;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CGkService::GetTriggerConnId() const
{
	return m_triggerConnId;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetTriggerConnId(DWORD connId)
{
	m_triggerConnId = connId;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::GetHoldRRQAfterAltEnd() const
{
	return m_holdRRQAfterAltEnd;
}

/////////////////////////////////////////////////////////////////////////////
altGksListSt *CGkService::GetGkUrqAltList() const
{
	return m_pUrqAltGkList;
}
	
/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkUrqAltList(altGksListSt *pAltGkListSt)
{
	PDELETEA(m_pUrqAltGkList);
	
	BYTE *pChar;
	int size = sizeof(altGksListSt) - 1; //reduce char	altGkSt[1]; 
	size += sizeof(alternateGkSt) * pAltGkListSt->numOfAltGks;
	pChar = new BYTE[size];
	m_pUrqAltGkList = (altGksListSt *) pChar;
	memcpy((char *)m_pUrqAltGkList,(char *)pAltGkListSt,size);
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetHoldRRQAfterAltEnd(BYTE value)
{
	m_holdRRQAfterAltEnd = value;
}

/////////////////////////////////////////////////////////////////////////////
// fills mcXmlTransportAddress with details from the given cs ip (ras or second signaling ip)
void CGkService::SetTransportAddr(CIpAddressPtr csIp, mcXmlTransportAddress* pAddr, WORD port)
{
	if(!csIp.get())  
	{
		// must fill union properties even if empty 
		SetUnionXml(&pAddr->unionProps,eIpVersion6);		
		return; // null ip
	}

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	//CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
	BOOL bIsIPv6RAS = YES;
	//if( pServiceSysCfg != NULL )
	//{
	   std::string key = CS_KEY_H323_RAS_IPV6;
	   pSysConfig->GetBOOLDataByKey(key, bIsIPv6RAS);
	   //pServiceSysCfg->GetBOOLDataByKey(m_ServiceId, key, bIsIPv6RAS);
	//}
	if( csIp->GetVersion() == eIpVersion6 && !bIsIPv6RAS )
	{
		PTRACE(eLevelInfoNormal,"CGkService::SetTransportAddr - ommiting IPV6");
		// must fill union properties even if empty 
		SetUnionXml(&pAddr->unionProps,eIpVersion6);		
		return; // null ip
	}
	
	mcTransportAddress* pTranAddr = &pAddr->transAddr;

	// fill pTranAddr ip and version with cs ip
	csIp->FillIPAddress(*pTranAddr);
	
	SetUnionXml(&pAddr->unionProps,csIp->GetVersion());

	pTranAddr->port				= port;
	pTranAddr->distribution		= eDistributionUnicast;
	pTranAddr->transportType	= eTransportTypeUdp;	
}

/////////////////////////////////////////////////////////////////////////////
// ipv4 or ipv6 cs addresses
void CGkService::SetRasAddr(mcXmlTransportAddress* pAddr, WORD port)       
{
	CIpAddressPtr csRasIp = GetCsRasIp();
	SetTransportAddr(csRasIp, pAddr, port);
}

/////////////////////////////////////////////////////////////////////////////
// sets cs addresses list. 'pAddr' is a pointer to array of addresses
void CGkService::SetSignalingAddresses(mcXmlTransportAddress* pAddr, WORD port, int arr_size)
{
	int size = M_Min(arr_size,TOTAL_NUM_OF_IP_ADDRESSES);	// avoid overflow 
	for(int i = 0; i<size; i++)
	{
		CIpAddressPtr csRasIp;
		csRasIp = GetCsIp(i);
		SetTransportAddr(csRasIp, &pAddr[i], port);		
		csRasIp.reset();				
	}
}

/////////////////////////////////////////////////////////////////////////////
// set cs addresses order where first in order should match the given address type. 
// basically, this address would be the gk address. 
void CGkService::SetCsAddressesOrder(const ipAddressStruct& pAddr)
{
	PTRACE(eLevelInfoNormal,"CGkService::SetCsAddressesOrder");
	CIpAddressPtr comparedIp = CIpAddress::CreateIpAddress(pAddr);
	
	int index = GetFirstCsIpIndexAs(comparedIp);
	
	if(index == -1) // no match was found
	{
		PTRACE(eLevelError,"CGkService::SetCsAddressesOrder - no CS address matches compared ip, couldn't decide on CS addresses order");
		DBGPASSERT(YES);
		return; // nothing to be ordered
	}
	
	ipAddressStruct tmpCsIps[TOTAL_NUM_OF_IP_ADDRESSES];
	memset(tmpCsIps,0,sizeof(tmpCsIps));
	CopyAddr(&tmpCsIps[0], &m_CsIps[index]);
	// we have the first in order in hand. now, need to order rest addresses.
	// we can have (at the moment), maximum five v6 ips and one v4 ip.

	// at this point first address is ipv4 or ipv6 and only one ipv4 address may be exist.
	// therefore, next in order would be ipv6 addresses (no matter if first one is ipv4 or ipv6)
	// last one would be ipv4 (if not already inserted to list head)
	
	int nextIndex = 1; // the next index to set (index 0 alresy set)
	int ipv4Index = -1;
	for(int i = 0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		if(i==index) continue; // skip first in order ip
		CIpAddressPtr csIp;
		csIp = GetCsIp(i);
		if(csIp.get()) 
		{
			// first, concat all ipv6 addresses
			if(csIp->GetVersion() == eIpVersion6)
				CopyAddr(&tmpCsIps[nextIndex++], &m_CsIps[i]);		
			else	// keep ipv4 address index (will be inserted to list tail)
				ipv4Index = i;
				
			csIp.reset();
		}
	}
	if (ipv4Index != -1) // if found an ipv4 address add it to list tail
		CopyAddr(&tmpCsIps[nextIndex++], &m_CsIps[ipv4Index]);		
		
	SetCsIpsSt(tmpCsIps, TOTAL_NUM_OF_IP_ADDRESSES);	
}

/////////////////////////////////////////////////////////////////////////////
// Iterates over all cs addresses list and return the first ip (index) that matches comparedIp (has same version and scopeId)
int CGkService::GetFirstCsIpIndexAs(const CIpAddressPtr comparedIp) const
{
	PTRACE2(eLevelInfoNormal,"CGkService::GetFirstCsIpIndexAs - ",comparedIp->ToString(1));
	for(int i = 0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		CIpAddressPtr csIp;
		csIp = GetCsIp(i);
		if(csIp.get() && csIp->isSameTypeAs(*comparedIp)) return i;
		csIp.reset();		
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// first or second signaling address.
// index indicates whether it is the first or the second signalling address to be inserted to the given structures.
// first in order is the ras address, second is the second signaling address 
void CGkService::SetSignalingAddr(mcXmlTransportAddress* pAddr, WORD port, int index)
{
	if(index < 0 || index >= TOTAL_NUM_OF_IP_ADDRESSES) return; // avoid overflow

	CIpAddressPtr csIp;
	csIp = GetCsIp(index);
	SetTransportAddr(csIp, pAddr, port);	
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkRasAddress(mcXmlTransportAddress* pGkRasAddress)
{
	mcTransportAddress *pTranAddr = &pGkRasAddress->transAddr;

	pTranAddr->distribution		= eDistributionUnicast;
	pTranAddr->transportType	= eTransportTypeUdp;

	CIpAddressPtr ip;
	
	if( (m_eAltGkProcess == eNotInProcess) || (m_eAltGkProcess == eStartFromOriginal) ||
    	(NULL == m_pCurAltStruct ))
	{
		ip = GetGkIp();		
		pTranAddr->port			= GK_RAS_PORT; 
	}
	else
	{
		ip = GetCurAltGkIp();				
		pTranAddr->port			= m_pCurAltStruct->rasAddress.transAddr.port; 
	}
	
	if(ip.get() && false == ip->isNull())
    {
	    SetUnionXml(&pGkRasAddress->unionProps, ip->GetVersion());
	    // fill pTranAddr with chosen IpAddress parameters (version & ip)

	    ip->FillIPAddress(*pTranAddr);
    }
    else
    {
        PTRACE(eLevelError, "CGkService::SetGkRasAddress.  ip is NULL!!");
        PTRACE2INT (eLevelError, "GkService::SetGkRasAddress. m_eAltGkProcess: ", m_eAltGkProcess);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetGkIpToZero()
{
	InitAddr(&m_GkIp);
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetFsParams(h460FsSt *pFs,BOOL bIsKeepAlive)
{
	if(m_bIsAvaya)
	{
		if(!bIsKeepAlive)
		{
			pFs->desireFs.fsId = H460_K_FsId_Avaya;
			pFs->desireFs.subFs = H460_C_AvfSubFsId;	
		}
		else
			pFs->desireFs.fsId = H460_K_FsId_None;
	}
	else
		pFs->desireFs.fsId = H460_K_FsId_None;
	
	pFs->needFs.fsId	  = H460_K_FsId_None;
	pFs->supportFs.fsId  = H460_K_FsId_None;  
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::InitAliasTypes(DWORD *pAliasType)
{
	for(int i=0;i < MaxNumberOfAliases; i++)
		pAliasType[i] = m_pAliasesTypes[i];
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::InitParam(GkManagerServiceParamsIndStruct* pServiceParamsIndSt)
{
	CGkAlias gkAlias;
	SetServiceName(pServiceParamsIndSt->serviceName);
	SetGkName(pServiceParamsIndSt->gkName);
	SetAltGkName(pServiceParamsIndSt->altGkName);

	SetCsIpsSt(&(pServiceParamsIndSt->csIp[0]), TOTAL_NUM_OF_IP_ADDRESSES);
	SetGkIpSt(&pServiceParamsIndSt->gkIp);	
	
	//save originals:
	SetGkIpStConfigured(&pServiceParamsIndSt->gkIp);
	SetNameGkConfigured(pServiceParamsIndSt->gkName);				
	SetAltGkIpStConfigured(&pServiceParamsIndSt->alternateGkIp);
	SetNameAltGkConfigured(pServiceParamsIndSt->altGkName);		
	
	if (GetAltGkIpConfigured().get())
		InsertAltGkInServiceToAltList();	//insert alt from service to alt gk list

	SetPrefix(pServiceParamsIndSt->prefixName);		

	m_ServiceId 	  = pServiceParamsIndSt->serviceId;
	m_RegistrationTimeConfigured = pServiceParamsIndSt->rrqPolingInterval;
	m_timeToLive = pServiceParamsIndSt->rrqPolingInterval;
	m_bIsRegAsGw	  = pServiceParamsIndSt->bIsRegAsGw;
	//The flag of Avaya is now based on the info received in RCF and license only
	//m_bIsAvaya 		  = (pServiceParamsIndSt->bIsAvf) ? TRUE : FALSE;
	//if (AVF_DEBUG_MODE == TRUE)
	//	m_bIsAvaya = TRUE;
	m_bIsGkInService  = pServiceParamsIndSt->bIsGkInService;

	gkAlias.SetAliases(&pServiceParamsIndSt->aliases[0]);
	InsertAlias(&gkAlias);
	m_service_ip_protocol_types = pServiceParamsIndSt->service_ip_protocol_types;
 
    //----- H.235 GK Authentication -----//
    this->m_H235Params.eEncryptMethodConfiguredScale     =  eH235Method_MD5 | eH235Method_SHA1 | eH235Method_SHA256;
    this->m_H235Params.eEncryptMethodRequired            =  eH235Method_Undef;
    this->m_H235Params.nIsAuth                           =  pServiceParamsIndSt->authenticationParams.isAuthenticationEnabled;//1;
    strncpy(this->m_H235Params.AuthUserName, pServiceParamsIndSt->authenticationParams.user_name, sizeof(this->m_H235Params.AuthUserName));
    strncpy(this->m_H235Params.AuthPassword, pServiceParamsIndSt->authenticationParams.password, sizeof(this->m_H235Params.AuthPassword));
    //strncpy(this->m_H235Params.AuthUserName,"RMX", sizeof(this->m_H235Params.AuthUserName));
    //strncpy(this->m_H235Params.AuthPassword,"RMXpwd", sizeof(this->m_H235Params.AuthPassword));
    //-----------------------------------//
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkService::CompareToGkAddress(mcXmlTransportAddress *pAddr) const
{
	CIpAddressPtr ip = CIpAddress::CreateIpAddress(pAddr->transAddr);
	CIpAddressPtr gkIp = GetGkIp();	
	if(ip.get() && gkIp.get())
		return *ip == *gkIp;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
/**
 * Sets aliases list.
 *	 Aliases list is set to compose of CGkAlias (which contain max of 5 ALIAS_S)
 * 	 and prefix. prefix is inserted to the beginnig of the buffer. then, rest aliases are inserted.
 *   Each time, an alias is inserted, an overflow check is performed.
 *   (At the moment, there's a possibility to exceed MaxAddressListSize).
 * @param pAlias - the aliases list 
 * 
 */
void CGkService::InsertAlias(CGkAlias *pAlias)
{
  // initilize aliases list
  memset(m_pAliasList, 0, MaxAddressListSize);
  WORD index = 0;
  //    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  CServiceConfigList *pServiceSysCfg = CProcessBase::GetProcess()->GetServiceConfigList();
  BOOL bIsEnableCiscoGK = 0;
  if (pServiceSysCfg != NULL)
  {
    std::string key = "ENABLE_CISCO_GK";
    //	pSysConfig->GetBOOLDataByKey(key, bIsEnableCiscoGK);
    pServiceSysCfg->GetBOOLDataByKey(m_ServiceId, key, bIsEnableCiscoGK);
  }
  else
  {
    if (m_SystemCfgChecked == TRUE)
    {
      PTRACE (eLevelError, "CGkService::InsertAlias - pServiceSysCfg is NULL");
      DBGPASSERT( YES );
    }
    else
      m_SystemCfgChecked = TRUE;
  }

  // insert the prefix if exists
  if (m_prefix[0] != '\0' && !bIsEnableCiscoGK)
  {
    // overflow check
    // additional byte for ";" suffix
    if (strlen(m_prefix) + 1 >= MaxAddressListSize)
    {
      // unable to add prefix
      DBGPASSERT( (strlen(m_prefix)+1) >= MaxAddressListSize );
      PTRACE2INT (eLevelError, "CGkService::InsertAlias - prefix length overflow", strlen(m_pAliasList) + strlen(m_prefix) + 2);
      return;
    }
    // it is safe to add the prefix
    strncat(m_pAliasList, m_prefix, sizeof(m_pAliasList) - strlen(m_pAliasList) - 1);
    m_pAliasesTypes[index++] = PARTY_H323_ALIAS_E164_TYPE;
  }

  // verify we are not overflowing
  DWORD aliasesLen = pAlias->GetAliasesLength();
  DWORD aliasesNum = pAlias->GetAliasesNumber();
  // aliasesNum is the number of commas (',') which should separate all aliases. example "132,453,hgr;"
  DBGPASSERT( (strlen(m_pAliasList) + aliasesLen + aliasesNum) >= MaxAddressListSize );

  int aliasIndex = 0;
  char* pCurrentAliasContent = pAlias->GetFirstAliasContent(); // get the first alias
  while (pCurrentAliasContent && (pCurrentAliasContent[0] != '\0'))
  {
    // overflow check
    // additional 2 is for "," seperator and ";" ender
    if ((strlen(m_pAliasList) + strlen(pCurrentAliasContent) + 2) >= MaxAddressListSize)
    {
      // unable to add more aliases - finalize and return
      PTRACE2INT (eLevelError, "CGkService::InsertAlias - aliases length overflow", strlen(m_pAliasList)+strlen(pCurrentAliasContent)+2);
	  if (strlen(m_pAliasList) < (MaxAddressListSize - 1))
	  {
      	strcat(m_pAliasList, ";");
	  }
	  else
	  {
	  	PASSERT(1);
	  }
      return;
    }
    if (index) // not first one
    {
    	if(strlen(m_pAliasList) < MaxAddressListSize - 1) // B.S klocwork 1019
    		strcat(m_pAliasList, ",");
    }
    // add the alias
    if( (strlen(m_pAliasList) +  strlen(pCurrentAliasContent) + 1) <= sizeof(m_pAliasList))  // B.S klocwork 1019
    	strncat(m_pAliasList, pCurrentAliasContent, (sizeof(m_pAliasList)-1)-strlen(m_pAliasList));
    else
    {
    	PASSERT(1);
    	return;
    }

	if (index < MaxNumberOfAliases)
	{
    	m_pAliasesTypes[index++] = pAlias->GetAliasType(aliasIndex++);
	}
	else
	{
		PASSERT(1);
		aliasIndex++;
	}
	
    pCurrentAliasContent = pAlias->GetAliasContent(aliasIndex);
  }

  // finalize buffer
  if(strlen(m_pAliasList) < MaxAddressListSize - 1) // B.S klocwork 1019
	  strcat(m_pAliasList, ";");
}

//////////////////////////////////////////////////////////////////////////////
void CGkService::SetNumOfDnsWaiting(BYTE num)
{
	m_dnsParams.numOfWaiting = num;
}

//////////////////////////////////////////////////////////////////////////////
BYTE CGkService::GetNumOfDnsWaiting()
{
	return m_dnsParams.numOfWaiting;
}

//////////////////////////////////////////////////////////////////////////////
void CGkService::IncreaseNumOfDnsWaiting()
{
	m_dnsParams.numOfWaiting++;
}

//////////////////////////////////////////////////////////////////////////////
void CGkService::DecreaseNumOfDnsWaiting()
{
	m_dnsParams.numOfWaiting--;
}

void CGkService::SetPrimeGkIpFromDns(ipAddressStruct* pGkAddr)
{
	CopyAddr(&m_dnsParams.primeGkIpFromDns,pGkAddr);
	SetGkIpSt(pGkAddr);	
	UpdateCardPropGkIp(&m_GkIp);
}

void CGkService::SetAltGkIpFromDns(ipAddressStruct* pGkAddr)
{
	CopyAddr(&m_dnsParams.altGkIpFromDns,pGkAddr);
	SetAltGkIpStConfigured(pGkAddr);
	UpdateCardPropGkIp(&m_alternateGkIpConfigured, 1);
}

//////////////////////////////////////////////////////////////////////////////
void CGkService::CreateVendor(mcuVendor *pEndpVendor)
{
	pEndpVendor->info.t35CountryCode   = Israel_t35CountryCode;
	pEndpVendor->info.t35Extension     = Israel_t35Extension;
	pEndpVendor->info.manufacturerCode = Accord_manufacturerCode;
	
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
//	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
	std::string productTypeStr;
	std::string key = "MCU_H323_PRODUCT_ID"; //VNGFE-8231 - flag used to be MCU_DISPLAY_NAME
	sysConfig->GetDataByKey(key, productTypeStr);
	// VNFE-2507the display name flag default value changed to "" ,
		// if it is empty change according to product type, otherwise according to system flag
	if (productTypeStr.size() == 0)
	{
	// VNGR-10405 set the display name according to product type and not according to system flag
		productTypeStr = "Polycom RMX 2000";
		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
		if(eProductTypeRMX4000 == curProductType)
			productTypeStr = "Polycom RMX 4000";
		else if(eProductTypeCallGenerator == curProductType)
			productTypeStr = "Polycom Call Generator";
		else if (eProductTypeRMX1500 == curProductType)
			productTypeStr = "Polycom RMX 1500";
		else if(eProductTypeSoftMCU == curProductType)
			productTypeStr = "Polycom Soft MCU";
		else if(eProductTypeGesher == curProductType)
			productTypeStr = "Polycom RMX 800S";
		else if(eProductTypeNinja == curProductType)
			productTypeStr = "Polycom RMX 1800";
		else if(eProductTypeSoftMCUMfw == curProductType)
			productTypeStr = "Polycom Soft MCU MFW";
		else if(eProductTypeEdgeAxis == curProductType)
			productTypeStr = "Polycom RMX 800VE";
		else if(eProductTypeCallGeneratorSoftMCU  == curProductType)
			productTypeStr = "Polycom Call Generator Soft MCU";
	}
		
	strncpy(pEndpVendor->productID, productTypeStr.c_str(), sizeof(pEndpVendor->productID) - 1);
	pEndpVendor->productID[sizeof(pEndpVendor->productID) - 1] = '\0';
	strncpy(pEndpVendor->versionID, AccordVersionId , sizeof(pEndpVendor->versionID) - 1);
	pEndpVendor->versionID[sizeof(pEndpVendor->versionID) - 1] = '\0';
	pEndpVendor->productLen = strlen(pEndpVendor->productID)+1;
	pEndpVendor->versionLen = strlen(pEndpVendor->versionID)+1;
}

/////////////////////////////////////////////////////////////////////////////
gkReqRasGRQ *CGkService::CreateGRQ(BOOL bIsAVFLicense, int *pStructSize)
{
	int aliasLen = strlen(m_pAliasList);
	*pStructSize = sizeof(gkReqRasGRQ) - sizeof(char); //reduce aliasesList[1];
	*pStructSize += (aliasLen + 1); //for \0
	BYTE *pChar = new BYTE[*pStructSize];
	gkReqRasGRQ *pGRQreq = (gkReqRasGRQ *) pChar;
	memset(pGRQreq, '\0', *pStructSize);
	
	m_multcast = IsIpGk() ? FALSE : TRUE;
	
	SetRasAddr(&pGRQreq->rasAddress,GK_RAS_PORT);
	SetGkRasAddress(&pGRQreq->gatekeeperAddress);
	//in case of AVF license set fs Id as for Avaya because currently the service 
	//have no information about future mode
	if (GetRegStatus() != eRegister) // If register -  Avaya flag has already received its true status from gatekeeper.
		m_bIsAvaya = TRUE; // bIsAVFLicense; - always send AVF fs Id
	
	SetFsParams(&pGRQreq->fs);
	InitAliasTypes(&pGRQreq->aliasesTypes[0]);
	
	pGRQreq->endpointType  = cmEndpointTypeMCU;
	pGRQreq->bIsMulticast  = m_multcast;
	pGRQreq->bSupportAltGk = TRUE;
	pGRQreq->aliasesLength = aliasLen;
	
//	if (aliasLen)
//	{
//		strncpy(pGRQreq->aliasesList, m_pAliasList, sizeof(pGRQreq->aliasesList) -1);
//		pGRQreq->aliasesList[sizeof(pGRQreq->aliasesList) - 1] = '\0';
//	}
	if (aliasLen)
		strncpy(pGRQreq->aliasesList, m_pAliasList, aliasLen);
	pGRQreq->aliasesList[aliasLen] = '\0';
	
    //----- H.235 GK Authentication -----//
    pGRQreq->H235AuthParams = this->m_H235Params;
    //-----------------------------------//
    //----- HOMOLOGATION. Set GK Identifier (configured by system flag: GK_IDENTIFIER) -------------------//
    memset(pGRQreq->gatekeeperIdent, '\0', sizeof(pGRQreq->gatekeeperIdent));
    pGRQreq->gkIdentLength  = 0;

    char szGkIdentConfigured[127]="";

    vGetGkIdentifierConfugured(szGkIdentConfigured, sizeof(szGkIdentConfigured));

    if(0 < strlen(szGkIdentConfigured))
    {
        strncpy(pGRQreq->gatekeeperIdent, szGkIdentConfigured, sizeof(pGRQreq->gatekeeperIdent)-1);
        if(0 >= (pGRQreq->gkIdentLength = gkConvertChar2Unicode(pGRQreq->gatekeeperIdent, strlen(pGRQreq->gatekeeperIdent))) )
        {
            PTRACE2INT (eLevelError, "CGkService::CreateGRQ - gkConvertChar2Unicode failed. Len: "
                , pGRQreq->gkIdentLength);
        }
    }
    //----------------------------------------------------------------------------------------------------//
	return pGRQreq;	
}

/////////////////////////////////////////////////////////////////////////////
gkReqRasRRQ *CGkService::CreateRRQ(int bIsPolling, int *pStructSize)
{
	PTRACE(eLevelInfoNormal,"CGkService::CreateRRQ");
	int aliasLen = strlen(m_pAliasList);
	*pStructSize = sizeof(gkReqRasRRQ) - 1; //reduce aliasesList[1];	
	*pStructSize += aliasLen + 1; //for \0
	BYTE *pChar = new BYTE[*pStructSize];
	gkReqRasRRQ *pRRQreq = (gkReqRasRRQ *)pChar;
	memset(pRRQreq, '\0', *pStructSize);

	SetGkRasAddress(&pRRQreq->gatekeeperAddress);
	SetSignalingAddresses(pRRQreq->callSignalingAddress, CALL_SIGNAL_PORT, MaxNumberOfEndpointIp); 
		
	SetRasAddr(&pRRQreq->rasAddress,GK_RAS_PORT);
	CreateVendor(&pRRQreq->endpointVendor);
	InitAliasTypes(&pRRQreq->aliasesTypes[0]);
	
	pRRQreq->gwPrefix[0] = '\0';	
	
	if(m_prefix[0]!='\0')
	{
		strncpy(pRRQreq->prefix, m_prefix, PHONE_NUMBER_DIGITS_LEN); //when we have prefix
		pRRQreq->gwPrefix[PHONE_NUMBER_DIGITS_LEN] = '\0';
		
		if(m_bIsRegAsGw)
		{
			strncpy(pRRQreq->gwPrefix, m_prefix, PHONE_NUMBER_DIGITS_LEN); //this field is only for CISCO Gk
			pRRQreq->gwPrefix[PHONE_NUMBER_DIGITS_LEN] = '\0';
		}
	}

	//HOMOLOGATION.GkIdent should initiate in the GRQInd------//
	pRRQreq->gkIdentLength = GetGkIdentLen();
    char * szGkIdent = GetGkIdent();
	if (szGkIdent != NULL)
	{
        if(0 != pRRQreq->gkIdentLength)//if('\0' != szGkIdent[0])
        {
         
		    memcpy(pRRQreq->gatekeeperIdent,szGkIdent,pRRQreq->gkIdentLength);

        }
        else
        {
            //---The requirement for CS to fill the GK_INDENTIFIER from value of system flag: "GK_IDENTIFIER";---// 
            BOOL bIsRrqWithoutGrq = FALSE;
            CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
            std::string key = "RRQ_WITHOUT_GRQ";
            if( pServiceSysCfg != NULL )
                pServiceSysCfg->GetBOOLDataByKey(this->GetServiceId(), key, bIsRrqWithoutGrq);

            if(TRUE == bIsRrqWithoutGrq)
            {
                //----- HOMOLOGATION. Set GK Identifier (configured by system flag: GK_IDENTIFIER) ---------------//
                memset(pRRQreq->gatekeeperIdent, '\0', sizeof(pRRQreq->gatekeeperIdent));
                pRRQreq->gkIdentLength  = 0;

                char szGkIdentConfigured[127]="";

                vGetGkIdentifierConfugured(szGkIdentConfigured, sizeof(szGkIdentConfigured));

                if(0 < strlen(szGkIdentConfigured))
                {
                    strncpy(pRRQreq->gatekeeperIdent, szGkIdentConfigured, sizeof(pRRQreq->gatekeeperIdent)-1);
                    if(0 >= (pRRQreq->gkIdentLength = gkConvertChar2Unicode(pRRQreq->gatekeeperIdent, strlen(pRRQreq->gatekeeperIdent))) )
                    {
                        PTRACE2INT (eLevelError, "CGkService::CreateRRQ - gkConvertChar2Unicode failed. Len: "
                            , pRRQreq->gkIdentLength);
                    }
                }
                //----------------------------------------------------------------------------------------------------//
            }
            //---------------------------------------------------------------------------------------------------//
        }
	}
    //----------------------------------------------------------//

	//EpIdent using for polling RRQ
	pRRQreq->epIdentLength = m_epIdentLength;
	memcpy(pRRQreq->endpointIdent,m_epIdent,m_epIdentLength);

   if ( (m_RRJCounter == 1) || !bIsPolling )
		pRRQreq->bIsKeepAlive = FALSE; //full registration
	else
		pRRQreq->bIsKeepAlive = TRUE; //We always answer with the KeepAlive flag on at the polling 

	SetFsParams(&pRRQreq->fs,pRRQreq->bIsKeepAlive);

	pRRQreq->dicoveryComplete   = m_discovery;
	pRRQreq->bIsMulticast       = FALSE;
	pRRQreq->bSupportAltGk		= TRUE;
	pRRQreq->timeToLive         = m_RegistrationTimeConfigured;

	if(pRRQreq->timeToLive < MIN_RRQ_INTERVAL)
		pRRQreq->timeToLive = MIN_RRQ_INTERVAL;
	
	pRRQreq->aliasesLength		= aliasLen;

//	strncpy(pRRQreq->aliasesList, m_pAliasList, sizeof(pRRQreq->aliasesList) - 1);
//	pRRQreq->aliasesList[sizeof(pRRQreq->aliasesList) - 1] = '\0';
	strncpy(pRRQreq->aliasesList, m_pAliasList, aliasLen);
	pRRQreq->aliasesList[aliasLen] = '\0';
		
    //----- H.235 GK Authentication -----//
    pRRQreq->H235AuthParams = this->m_H235Params;
    //-----------------------------------//

	//pRRQreq->mcuDetails = Need to update!!!!!!!!!!!
	return pRRQreq;
}

/////////////////////////////////////////////////////////////////////////////
gkReqRasURQ *CGkService::CreateURQ(int *pStructSize)
{
	int aliasLen = strlen(m_pAliasList);
	*pStructSize = sizeof(gkReqRasURQ) - 1; //reduce sAliasesList[1];
	*pStructSize += aliasLen + 1; //for \0;
	BYTE *pChar = new BYTE[*pStructSize];
	gkReqRasURQ *pURQreq = (gkReqRasURQ *)pChar;
	memset(pURQreq, '\0', *pStructSize);
	
    SetGkRasAddress(&pURQreq->gatekeeperAddress);
	SetSignalingAddresses(pURQreq->callSignalingAddress, CALL_SIGNAL_PORT, MaxNumberOfEndpointIp); 
	InitAliasTypes(&pURQreq->aliasesTypes[0]);

	pURQreq->gkIdentLength = GetGkIdentLen();

	char *pGkIdent = GetGkIdent();
	if (pGkIdent != NULL)
	{
		memcpy(pURQreq->gatekeeperIdent,pGkIdent,pURQreq->gkIdentLength);
	}

	//EpIdent using for confirmation of identity
	pURQreq->epIdentLength = m_epIdentLength;
	memcpy(pURQreq->endpointIdent,m_epIdent,m_epIdentLength);

	pURQreq->aliasesLength		= aliasLen;
//	strncpy(pURQreq->sAliasesList,m_pAliasList,sizeof(pURQreq->sAliasesList) - 1);
//	pURQreq->sAliasesList[sizeof(pURQreq->sAliasesList) - 1] = '\0';
	strncpy(pURQreq->sAliasesList,m_pAliasList,aliasLen);
	pURQreq->sAliasesList[aliasLen] = '\0';

    //----- H.235 GK Authentication -----//
    pURQreq->H235AuthParams = this->m_H235Params;
    //-----------------------------------//

	return pURQreq;
}

/////////////////////////////////////////////////////////////////////////////
//Party initiated the srcAndDestInfo field so he must declared the right size of the message so
//I can use it and send to the GkMngr the message I received from the party.
void CGkService::CreateARQ(gkReqRasARQ *pPartyARQ)
{
	pPartyARQ->totalDynLen = pPartyARQ->destInfoLength + pPartyARQ->srcInfoLength;
	
	SetGkRasAddress(&pPartyARQ->gatekeeperAddress);
	SetUnionXml(&pPartyARQ->destCallSignalAddress.unionProps,pPartyARQ->destCallSignalAddress.transAddr.ipVersion);

	//BRIDGE-15392 - fill local signaling IP address
	//SetUnionXml(&pPartyARQ->srcCallSignalAddress.unionProps, pPartyARQ->srcCallSignalAddress.transAddr.ipVersion);
	APIU32   localPort = pPartyARQ->srcCallSignalAddress.transAddr.port;
	SetRasAddr(&pPartyARQ->srcCallSignalAddress, localPort);
	
	pPartyARQ->gkIdentLength = GetGkIdentLen();

	char *pGkIdent = GetGkIdent();
	if (pGkIdent != NULL)
	{
		memcpy(pPartyARQ->gatekeeperIdent,pGkIdent,pPartyARQ->gkIdentLength);
	}
	
	pPartyARQ->epIdentLength = m_epIdentLength;
	memcpy(pPartyARQ->endpointIdent,m_epIdent,m_epIdentLength);
	//Check is it necessary to send AVF in future versions of ACM
	if (m_bIsAvaya)
		pPartyARQ->avfFeVndIdReq.fsId = H460_K_FsId_Avaya;

    //----- H.235 GK Authentication -----//
    pPartyARQ->H235AuthParams = this->m_H235Params;
    //-----------------------------------//
}

/////////////////////////////////////////////////////////////////////////////
//This is a static struct so the message that I received from the party has the right size so I will finish
//initiate the missing field and send it to the GkMngr
void CGkService::CreateBRQ(gkReqRasBRQ *pPartyBRQ)
{
	SetGkRasAddress(&pPartyBRQ->gatekeeperAddress);

	pPartyBRQ->gkIdentLength = GetGkIdentLen();

	char *pGkIdent = GetGkIdent();
	if (pGkIdent != NULL)
	{
		memcpy(pPartyBRQ->gatekeeperIdent,pGkIdent,pPartyBRQ->gkIdentLength);
	}
	
	pPartyBRQ->epIdentLength = m_epIdentLength;
	memcpy(pPartyBRQ->endpointIdent,m_epIdent,m_epIdentLength);

    //----- H.235 GK Authentication -----//
    pPartyBRQ->H235AuthParams = this->m_H235Params;
    //-----------------------------------//
}

/////////////////////////////////////////////////////////////////////////////
//Party initiated the srcInfo field so he must declared the right size of the message so
//I can use it and send to the GkMngr the message I received from the party.
void CGkService::CreateIRR(gkReqRasIRR *pPartyIRR)
{
	SetGkRasAddress(&pPartyIRR->gatekeeperAddress);
	SetRasAddr(&pPartyIRR->rasAddress,GK_RAS_PORT);
	
	SetUnionXml(&pPartyIRR->destCallSignalAddress.unionProps,pPartyIRR->destCallSignalAddress.transAddr.ipVersion);
	SetUnionXml(&pPartyIRR->srcCallSignalAddress.unionProps, pPartyIRR->srcCallSignalAddress.transAddr.ipVersion);
	
	pPartyIRR->epIdentLength = m_epIdentLength;
	memcpy(pPartyIRR->endpointIdent,m_epIdent,m_epIdentLength);
 
    //----- H.235 GK Authentication -----//
    pPartyIRR->H235AuthParams = this->m_H235Params;
    //-----------------------------------//
}

/////////////////////////////////////////////////////////////////////////////
//This is a static struct so the message that I received from the party has the right size so I will finish
//initiate the missing field and send it to the GkMngr
void CGkService::CreateDRQ(gkReqRasDRQ *pPartyDRQ)
{
	SetGkRasAddress(&pPartyDRQ->gatekeeperAddress);

	pPartyDRQ->gkIdentLength = GetGkIdentLen();

	char *pGkIdent = GetGkIdent();
	if (pGkIdent != NULL)
	{
		memcpy(pPartyDRQ->gatekeeperIdent,pGkIdent,pPartyDRQ->gkIdentLength);
	}
	
	pPartyDRQ->epIdentLength = m_epIdentLength;
	memcpy(pPartyDRQ->endpointIdent,m_epIdent,m_epIdentLength);

    //----- H.235 GK Authentication -----//
    pPartyDRQ->H235AuthParams = this->m_H235Params;
    //-----------------------------------//
}

/////////////////////////////////////////////////////////////////////////////
gkReqURQResponse *CGkService::CreateURQResponse()
{
	gkReqURQResponse *pURQResReq = new gkReqURQResponse;

	pURQResReq->hsRas = m_gkURQhsRas;

	return pURQResReq;
}

/////////////////////////////////////////////////////////////////////////////
//This is a static struct so the message that I received from the party has the right size.
//This struct has only one field that the party already initiate it, so I just return the parameter to the GkMngr.
void CGkService::CreateDRQResponse(gkReqDRQResponse *pPartyDRQ)
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
gkReqDRQResponse  *CGkService::CreateDRQResponse(gkIndDRQFromGk *pPartyDRQFromGk)
{
	gkReqDRQResponse *pDRQResReq = new gkReqDRQResponse;
	
	pDRQResReq->hsRas = pPartyDRQFromGk->hsRas;
	
	return pDRQResReq;
}

gkReqLRQResponse *CGkService::CreateLRQResponse(int hsRas)
{
	gkReqLRQResponse *pLRQResReq = new gkReqLRQResponse;

	pLRQResReq->hsRas = hsRas;
	SetGkRasAddress(&pLRQResReq->rasAddress);
	SetSignalingAddresses(pLRQResReq->callSignalingAddress, CALL_SIGNAL_PORT, MaxNumberOfEndpointIp); 
	
	return pLRQResReq;
}

/////////////////////////////////////////////////////////////////////////////
//This is a static struct so the message that I received from the party has the right size.
//This struct has only two fields that the party already initiate them, so I just return the parameter to the GkMngr.
void CGkService::CreateBRQResponse(gkReqBRQResponse *pPartyBRQ)
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CGkService::SetServiceIpTypes(BYTE ipVerType)
{
	m_service_ip_protocol_types = (eIpType)ipVerType;
}

/////////////////////////////////////////////////////////////////////////////
eIpType CGkService::GetServiceIpTypes()
{
	return m_service_ip_protocol_types;
}
/////////////////////////////////////////////////////////////////////////////
//----- H.235 GK Authentication -----/
void   CGkService::Set_EncryptMethodRequired(int par_EncryptionMethodRequired)
{
    this->m_H235Params.eEncryptMethodRequired = par_EncryptionMethodRequired;

    return;
}

void   CGkService::Get_H235Params(GkH235AuthParam   *  par_pH235Params)
{
    memcpy(par_pH235Params, &this->m_H235Params, sizeof(GkH235AuthParam));
}

void   CGkService::Set_H235Params(GkH235AuthParam   *  par_pH235Params)    
{
    this->m_H235Params.nIsAuth = par_pH235Params->nIsAuth;
    strcpy(this->m_H235Params.AuthUserName , par_pH235Params->AuthUserName);
    strcpy(this->m_H235Params.AuthPassword , par_pH235Params->AuthPassword);
}

int    CGkService::Get_LastRejectReason()
{
    return this->m_nLastRejectReason;
}
void   CGkService::Set_LastRejectReason(int par_nLastRejectReason)
{
    this->m_nLastRejectReason = par_nLastRejectReason;
}








