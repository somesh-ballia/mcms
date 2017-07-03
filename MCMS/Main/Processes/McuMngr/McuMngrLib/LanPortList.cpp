#include "LanPortList.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include "GetIFList.h"
#include "GetIFStatus.h"
#include "GetIFFeatures.h"
#include "IpmiSensorEnums.h"
#include "GetEthBondStatus.h"
//added by Richer for 802.1x project om 2013.12.26
#include "802_1xApiDefinitions.h"

/////////////////////////////////////////////////////////////////////////////
// CLanPortList

CLanPortList::CLanPortList()
{
}

CLanPortList::~CLanPortList()
{
}

char const * CLanPortList::NameOf() const
{
    return "CLanPortList";
}

////////////////////////////////////////////////////////////////////////////
void CLanPortList::Update()
{
    //added by Richer for 802.1x project om 2013.12.26
    ETH_SETTINGS_S *pCurEthSetting = new ETH_SETTINGS_S;
	
    m_lans.clear();

    vector<IFInfo> ifs;
    GetIFList(ifs);

    struct EthBondingInfo bondingInfo;
    GetEthBondingInfo(bondingInfo);

    int const count = ifs.size();
    for (int i=0; i<count; ++i)
    {
        IFInfo const & ifInfo = ifs[i];

        int const ifIndex = ifInfo.index;
        
        LanPortSummary lan;
        int isLinkUp = false;
        if ((*IsLinkUp)(ifIndex, isLinkUp))
        {
            lan.status = GetLinkStatusWithBondingInfo(ifIndex, isLinkUp, bondingInfo);
        }
        else
        {
            lan.status = LAN_STATUS_INACTIVE;
        }
        lan.portID = 0;
        lan.slotID = LAN_SLOT_ID_START+ifIndex;

	 //added by Richer for 802.1x project om 2013.12.26
        memset(pCurEthSetting, 0 , sizeof(ETH_SETTINGS_S));
	 Get802_1xStatus(ifInfo.name, pCurEthSetting);
	 Get802_1xMethodType(ifInfo.name, pCurEthSetting);
	 
	 lan.e802_1xMethod = pCurEthSetting->e802_1xMethod;
	 lan.e802_1xSuppPortStatus = pCurEthSetting->e802_1xSuppPortStatus;
	 lan.e802_1xFailReason = pCurEthSetting->e802_1xFailReason;
	 
        m_lans.push_back(lan);
    }

    //added by Richer for 802.1x project om 2013.12.26
    PDELETE(pCurEthSetting);
}

void CLanPortList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("PORT_SUMMARY_LS");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("PORT_SUMMARY_LS");
    }

    int const count = m_lans.size();
    for (int i=0; i<count; ++i)
    {
        LanPortSummary const & summary = m_lans[i];
        
        CXMLDOMElement * const pPortNode = pRootNode->AddChildNode("PORT_SUMMARY");
	 //added by Richer for 802.1x project om 2013.12.26
        pPortNode->AddChildNode("Status_802", summary.e802_1xSuppPortStatus);
	 
        pPortNode->AddChildNode("Status", summary.status);
        pPortNode->AddChildNode("PortID", summary.portID);
        pPortNode->AddChildNode("SlotID", summary.slotID);
		
	 //added by Richer for 802.1x project om 2013.12.26
        pPortNode->AddChildNode("Failure_802", summary.e802_1xFailReason);
        pPortNode->AddChildNode("Method_802", summary.e802_1xMethod);
        
    }
}

///////////////////////////////////////////////////////////////////////////////
int CLanPortList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

//added by Richer for 802.1x project om 2013.12.26
int Get802_1xStatus(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{

    #define BUFFER_LEN (256)
    UINT32 ulRc = STATUS_OK;
    std::string cLineBuffer;
    INT8 cCommandLine[BUFFER_LEN] = {0};
    INT8 cEthName[BUFFER_LEN] = {0};


    STATUS stat;
    std::string answer = "";
	
    stat = SystemPipedCommand("sudo ps x | grep 'wpa_supplicant' ",answer);
    
    if (stat != STATUS_OK)
    {

    	FPTRACE(eLevelInfoNormal,"GetEthInfo802_1xStatus: - failed to check if wpa_supplicant run " );
    	return ulRc;
    }


    snprintf(cEthName, sizeof(cEthName), "-i%s", EthX);

    //wpa_supplicant is running
    if (strstr(answer.c_str(), cEthName))
    {
    	snprintf(cCommandLine, sizeof(cCommandLine), "sudo /usr/sbin/wpa_cli -i%s status", EthX);
    	FTRACECOND(true,cCommandLine);

    	STATUS stat = SystemPipedCommand(cCommandLine ,cLineBuffer, TRUE, FALSE);


    	if(stat != STATUS_OK) 
	{

    		FPTRACE(eLevelInfoNormal,"GetEthInfo802_1xStatus:- SystemPipedCommand failed ");
    		ulRc = -1;
    		return ulRc;
    	}


    	//search for the error case first
    	if(strstr(cLineBuffer.c_str(), P802_1x_STR_FAILED_TO_CONNECT) != NULL) 
	{
    		//we got an error
    		FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus:- wpa_supplicant isnt running: ", cLineBuffer.c_str() );
    		ulRc = -1;
    		return ulRc;
    	}

    	//now search for: 'suppPortStatus='
    	if(strstr(cLineBuffer.c_str(), P802_1x_STR_SUP_PORT_STAT) != NULL)
    	{
    		//we got the line, now parse out the value into the variable sWpaCliParseRes
    		if(strstr(cLineBuffer.c_str(), P802_1x_STR_UNAUTHORIZED) != NULL) 
		{
    			pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_FAILED;
    		}
    		else if(strstr(cLineBuffer.c_str(), P802_1x_STR_AUTHORIZED) != NULL) 
		{
    			pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_AUTHENTICATED;

    		}
    		else 
		{
    			pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_INVALID;
    			FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus:- INVALID VALUE ", cLineBuffer.c_str() );

    			ulRc = -1;
    			return ulRc;
    		}
    	}

    	//now check if we need to fill in a failure reason
    	if(pST_EthProperties->e802_1xSuppPortStatus  != E_802_1x_PORT_STATUS_AUTHENTICATED) 
	{
    		pST_EthProperties->e802_1xFailReason     = E_802_1x_FR_BAD_CONFIGURATION;
    	}
    	else 
	{
    		pST_EthProperties->e802_1xFailReason     = E_802_1x_FR_OFF;
    	}




    }
    return ulRc;

}


//added by Richer for 802.1x project om 2013.12.26
int Get802_1xMethodType(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{

    int ReturnValue;
    #define WPA_CONF_BUFFER_MAX_LENGTH (4096)
    INT8 caConfStr[WPA_CONF_BUFFER_MAX_LENGTH]= {0};
    INT8 caConfFileName[P802_1x_FILES_PATH_MAX_LENGTH] = {0};
    FILE* pFile = NULL;

    /*Begin: added for BRIDGE-13961, 2014.6.23*/
    STATUS stat;
    std::string answer = "";
    INT8 cEthName[BUFFER_LEN] = {0};
	
    stat = SystemPipedCommand("sudo ps x | grep 'wpa_supplicant' ",answer);
    
    if (stat != STATUS_OK)
    {

        FPTRACE(eLevelInfoNormal,"GetEthInfo802_1xStatus: - failed to check if wpa_supplicant run " );
        pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_OFF;    
    	 return 0;
    }

    snprintf(cEthName, sizeof(cEthName), "-i%s", EthX);

    if (!strstr(answer.c_str(), cEthName))
    {
        pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_OFF;    
        return 0;
    }
    /*End: added for BRIDGE-13961, 2014.6.23*/

    if (strcmp(EthX ,"bond0:1") == 0)
    {
        strncpy(caConfFileName, (MCU_TMP_DIR+"/802_1xCtrl/wpa_eth1.conf").c_str(), sizeof(caConfFileName)-1);
    }
    else  if (strcmp(EthX ,"bond0:2") == 0)
    {
	  strncpy(caConfFileName, (MCU_TMP_DIR+"/802_1xCtrl/wpa_eth2.conf").c_str(), sizeof(caConfFileName)-1);
    }
    else
    {
        snprintf(caConfFileName, sizeof(caConfFileName)-1, (MCU_TMP_DIR+"/802_1xCtrl/wpa_%s.conf").c_str(), EthX);
    }

    // FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xMethodType: - caConfFileName ", caConfFileName);

    FTRACESTR(eLevelInfoNormal)  << "\nGetEthInfo802_1xMethodType: - caConfFileName  " << caConfFileName;


    if((pFile = fopen(caConfFileName, "r")) != NULL)
    {
        long lSize;
        size_t result;



        // obtain file size:
        fseek (pFile , 0 , SEEK_END);
        lSize = ftell (pFile);
        rewind (pFile);
        result = fread (caConfStr,1,lSize,pFile);

        //FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus: - caConfStr ", caConfStr);
        FTRACESTR(eLevelInfoNormal)  << "\nGetEthInfo802_1xStatus: - caConfStr " << caConfStr;



        if(strstr(caConfStr, P802_1x_STR_METHOD_TYPE_FROM_CONF_FILE) != NULL)
        {
                //we got the line, now parse out the value into the method types
                if(strstr(caConfStr, P802_1x_STR_METHOD_MD5_TYPE_FROM_CONF_FILE) != NULL) 
                {
                    //802.1x
                    pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_EAP_MD5;

                }
                else if(strstr(caConfStr, P802_1x_STR_METHOD_CHAP_TYPE_FROM_CONF_FILE) != NULL) 
                {
                    pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_PEAP_MSCHAPV2;
                }

                else if(strstr(caConfStr, P802_1x_STR_METHOD_TLS_TYPE_FROM_CONF_FILE) != NULL) 
                {
                    pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_EAP_TLS;
                }
                else 
                {
                    pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_OFF;

                }
            }

    }
    else
    {
        FPTRACE(eLevelInfoNormal,"GetEthInfo802_1xMethodType: - errorno ");
        return -1;
    }
    if(pFile)
        fclose(pFile);

    return 0;

}



