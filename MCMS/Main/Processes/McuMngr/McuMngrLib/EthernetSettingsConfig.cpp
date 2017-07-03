// EthernetSettingsConfig.cpp: implementation of the CEthernetSettingsConfig class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "EthernetSettingsConfig.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "TraceStream.h"

#include "EncodeHelper.h"

#include "SysConfig.h"




extern const char* PortSpeedTypeToString(ePortSpeedType type);


// ------------------------------------------------------------
CEthernetSettingsConfig::CEthernetSettingsConfig ()
{
	InitMembers();
}

// ------------------------------------------------------------
CEthernetSettingsConfig::CEthernetSettingsConfig (const CEthernetSettingsConfig& other)
: CSerializeObject(other)
{
	m_slotId	= other.m_slotId;
	m_portId	= other.m_portId;
	m_portType	= other.m_portType;
	m_portSpeed	= other.m_portSpeed;

	m_mediaPortType = other.m_mediaPortType;

	m_802_1xAuthenticationProtocol = other.m_802_1xAuthenticationProtocol;
}

// ------------------------------------------------------------
CEthernetSettingsConfig::~CEthernetSettingsConfig ()
{
}

// ------------------------------------------------------------
CEthernetSettingsConfig& CEthernetSettingsConfig::operator = (const CEthernetSettingsConfig &rOther)
{
	m_slotId	= rOther.m_slotId;
	m_portId	= rOther.m_portId;
	m_portType	= rOther.m_portType;
	m_portSpeed	= rOther.m_portSpeed;

	m_802_1xAuthenticationProtocol = rOther.m_802_1xAuthenticationProtocol;
	strncpy(m_802_1xUserName,rOther.m_802_1xUserName,NAME_LEN_802_1x);
	//strncpy(m_802_1xPassword,rOther.m_802_1xPassword,NAME_LEN_802_1x);
	//strncpy(m_802_1xPassword_enc,rOther.m_802_1xPassword_enc,NAME_LEN_802_1x);
	m_802_1xPassword     = rOther.m_802_1xPassword;
	m_802_1xPassword_enc = rOther.m_802_1xPassword_enc;

	//m_IsFromEma = true;
	return *this;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SerializeXmlForEma(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pEthernetSettingsNode;

	if(!pParentNode)
	{
		pParentNode =  new CXMLDOMElement();
		pParentNode->set_nodeName("ETHERNET_SETTINGS");
		pEthernetSettingsNode = pParentNode;
	}
	else
	{
		pEthernetSettingsNode = pParentNode->AddChildNode("ETHERNET_SETTINGS");
	}


	pEthernetSettingsNode->AddChildNode("SLOT_NUMBER", m_slotId, BOARD_ENUM);
	pEthernetSettingsNode->AddChildNode("PORT",	 m_portId);
	pEthernetSettingsNode->AddChildNode("ETHERNET_PORT_TYPE", m_portType, ETHERNET_PORT_TYPE_ENUM);
	pEthernetSettingsNode->AddChildNode("SPEED", m_portSpeed, SPEED_MODE_ENUM);
	pEthernetSettingsNode->AddChildNode("AUTHENTICATION_8021x_METHOD_TYPE",m_802_1xAuthenticationProtocol,AUTHENTICATION_PROTOCOL_802_1X_ENUM);
	pEthernetSettingsNode->AddChildNode("USER_NAME",m_802_1xUserName);
	pEthernetSettingsNode->AddChildNode("PASSWORD",m_802_1xPassword);

	//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::SerializeXml SerializeXmlForEma " << m_802_1xPassword;
	return;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pEthernetSettingsNode;
 
	if(!pParentNode)
	{
		pParentNode =  new CXMLDOMElement();
		pParentNode->set_nodeName("ETHERNET_SETTINGS");
		pEthernetSettingsNode = pParentNode;
	}
	else
	{
		pEthernetSettingsNode = pParentNode->AddChildNode("ETHERNET_SETTINGS");
	}


	pEthernetSettingsNode->AddChildNode("SLOT_NUMBER", m_slotId, BOARD_ENUM);
	pEthernetSettingsNode->AddChildNode("PORT",	 m_portId);
	pEthernetSettingsNode->AddChildNode("ETHERNET_PORT_TYPE", m_portType, ETHERNET_PORT_TYPE_ENUM);
	pEthernetSettingsNode->AddChildNode("SPEED", m_portSpeed, SPEED_MODE_ENUM);
	pEthernetSettingsNode->AddChildNode("AUTHENTICATION_8021x_METHOD_TYPE",m_802_1xAuthenticationProtocol,AUTHENTICATION_PROTOCOL_802_1X_ENUM);
	pEthernetSettingsNode->AddChildNode("USER_NAME",m_802_1xUserName);


	//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::SerializeXml m_802_1xPassword " << m_802_1xPassword;


	EncodeHelper eH;
	std::string m802_1xPassword_encTmp ;

	eH.EncryptPassword((const std::string)m_802_1xPassword , m802_1xPassword_encTmp);



	//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::SerializeXml m802_1xPassword_encTmp " << m802_1xPassword_encTmp;

	pEthernetSettingsNode->AddChildNode("PASSWORD",m802_1xPassword_encTmp.c_str());



	return;
}

// ------------------------------------------------------------
int  CEthernetSettingsConfig::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus=STATUS_OK;

	CXMLDOMElement* pEthernetSettingsNode = NULL;
	char* ParentNodeName;
	

	pActionNode->get_nodeName(&ParentNodeName); 

	if( !strcmp(ParentNodeName, "ETHERNET_SETTINGS") )
	{ 
		pEthernetSettingsNode = pActionNode; 
	}  
	else
	{
		GET_MANDATORY_CHILD_NODE(pActionNode, "ETHERNET_SETTINGS", pEthernetSettingsNode);
	}

	if (pEthernetSettingsNode)
	{
		DWORD tmp = 0;
	    
		GET_VALIDATE_CHILD(pEthernetSettingsNode, "SLOT_NUMBER", &m_slotId, BOARD_ENUM);
	    
		GET_VALIDATE_CHILD(pEthernetSettingsNode, "PORT", &m_portId, _0_TO_DWORD);
	
	    GET_VALIDATE_CHILD(pEthernetSettingsNode, "ETHERNET_PORT_TYPE", &tmp, ETHERNET_PORT_TYPE_ENUM);

	    m_portType = (eEthPortType)tmp;
	    
	    tmp = 0;
		GET_VALIDATE_CHILD(pEthernetSettingsNode, "SPEED", &tmp, SPEED_MODE_ENUM);
		m_portSpeed = (ePortSpeedType)tmp;

		tmp = 0;
		GET_VALIDATE_CHILD(pEthernetSettingsNode, "AUTHENTICATION_8021x_METHOD_TYPE", &tmp, AUTHENTICATION_PROTOCOL_802_1X_ENUM);
		m_802_1xAuthenticationProtocol = (eEth802_1xAuthenticationType)tmp;

		GET_VALIDATE_CHILD(pEthernetSettingsNode, "USER_NAME", m_802_1xUserName, _0_TO_H243_NAME_LENGTH);


		//the password in the file is enc
		if ( m_IsFromEma == true)
		{


			GET_VALIDATE_CHILD(pEthernetSettingsNode, "PASSWORD", m_802_1xPassword, _0_TO_H243_NAME_LENGTH);


			//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::DeSerializeXml m_802_1xPassword " << m_802_1xPassword;


			EncodeHelper eH;
			eH.EncryptPassword(m_802_1xPassword , m_802_1xPassword_enc);



			//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::DeSerializeXml m_802_1xPassword_enc " << m_802_1xPassword_enc;

		}

		else
		{


			GET_VALIDATE_CHILD(pEthernetSettingsNode, "PASSWORD", m_802_1xPassword_enc, _0_TO_H243_NAME_LENGTH);

			EncodeHelper eH;
			eH.DecryptPassword(m_802_1xPassword_enc , m_802_1xPassword  );


			//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::DeSerializeXml m_802_1xPassword " << m_802_1xPassword;



			//FTRACESTR(eLevelInfoNormal) << "CEthernetSettingsConfig::DeSerializeXml m_802_1xPassword_enc " << m_802_1xPassword_enc;




		}

	}

	return nStatus;
}

// ------------------------------------------------------------
bool operator==(const CEthernetSettingsConfig& first,const CEthernetSettingsConfig& second)
{
	bool ret = false;       
	
	if ( (first.m_slotId	== second.m_slotId)		&&
		 (first.m_portId	== second.m_portId)		&&
		 (first.m_portType	== second.m_portType)	&&
		 (first.m_portSpeed	== second.m_portSpeed) )
	{
		ret = true;
	}


	return ret;
}

// ------------------------------------------------------------
bool operator==(const CEthernetSettingsConfig& theObject, const ETH_SETTINGS_PORT_DESC_S& theStruct)
{
	bool ret = false;       
	
	if ( (theObject.m_slotId == theStruct.slotId)	&&
		 (theObject.m_portId == theStruct.portNum) )
	{
		ret = true;
	}

	return ret;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::InitMembers()
{
	m_slotId	= 0;
	m_portId	= 0;
	m_portType	= eEthPortType_Illegal;
	m_mediaPortType	= eEthMediaPortType_Illegal;
	m_portSpeed	= ePortSpeed_Auto;
	m_802_1xAuthenticationProtocol = eEth802_1xAuthenticationType_Off;

	memset(m_802_1xUserName, 0, NAME_LEN_802_1x);
	m_802_1xPassword     = "";
	m_802_1xPassword_enc = "";

	m_IsFromEma = true;


}

// ------------------------------------------------------------
void  CEthernetSettingsConfig::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n===== CEthernetSettingsConfig::Dump =====" << "\n"
		<< std::setw(20) << "Slot id:    "	<< m_slotId << "\n"
		<< std::setw(20) << "Port id:    "	<< m_portId << "\n"
		<< std::setw(20) << "Port type:  "	<< GetEthPortTypeStr(m_portType) << "\n"
		<< std::setw(20) << "Port speed: "	<< PortSpeedTypeToString(m_portSpeed) << "\n"
	    << std::setw(20) << "802.1x authentication protocol: "	<< Get802_1xAuthenticationTypeStr(m_802_1xAuthenticationProtocol) << "\n";
}

// ------------------------------------------------------------
DWORD	CEthernetSettingsConfig::GetSlotId() const
{
	return m_slotId;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SetSlotId(const DWORD slotId)
{
	m_slotId = slotId;
}

// ------------------------------------------------------------
DWORD	CEthernetSettingsConfig::GetPortId() const
{
	return m_portId;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SetPortId(const DWORD portId)
{
	m_portId = portId;
}

// ------------------------------------------------------------
eEthPortType	CEthernetSettingsConfig::GetPortType() const
{
	return m_portType;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SetPortType(const eEthPortType portType)
{
	m_portType = portType;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SetIsFromEma(const bool isFromEma)
{
	m_IsFromEma = isFromEma;
}

const char* CEthernetSettingsConfig::Get802_1xUserName() const
{
	return m_802_1xUserName ;
}
const char* CEthernetSettingsConfig::Get802_1xUserPassword() const
{
	return m_802_1xPassword.c_str() ;
}



// ------------------------------------------------------------
eEthMediaPortType	CEthernetSettingsConfig::GetMediaPortType() const
{
	return m_mediaPortType;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SetMediaPortType(const eEthMediaPortType mediaPortType)
{
	m_mediaPortType = mediaPortType;
}

// ------------------------------------------------------------
ePortSpeedType  CEthernetSettingsConfig::GetPortSpeed() const
{
    return m_portSpeed;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::SetPortSpeed(const ePortSpeedType portSpeed)
{
	m_portSpeed = portSpeed;
}

void CEthernetSettingsConfig::Set802_1xUserName( const char * user)
 {
	if (user && strcmp(user,m_802_1xUserName))
		strncpy(m_802_1xUserName , user,sizeof(m_802_1xUserName)-1);
 }

void CEthernetSettingsConfig::Set802_1xUserPassword( const char * password)
{
	m_802_1xPassword = password;
}


// ------------------------------------------------------------
eEth802_1xAuthenticationType	CEthernetSettingsConfig::Get802_1xAuthenticationProtocol() const
{
	return m_802_1xAuthenticationProtocol;
}

// ------------------------------------------------------------
void CEthernetSettingsConfig::Set802_1xAuthenticationProtocol(const eEth802_1xAuthenticationType authenticationType)
{
	m_802_1xAuthenticationProtocol = authenticationType;
}




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//					CEthernetSettingsConfigList
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------
CEthernetSettingsConfigList::CEthernetSettingsConfigList ()
{
	InitMembers();
}

// ------------------------------------------------------------
CEthernetSettingsConfigList::CEthernetSettingsConfigList (const CEthernetSettingsConfigList& other)
: CSerializeObject(other)
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		POBJDELETE(m_pEthernetSettingsConfigList[i]);

		m_pEthernetSettingsConfigList[i] =
			new CEthernetSettingsConfig( *(other.m_pEthernetSettingsConfigList[i]) );
	}	
}

// ------------------------------------------------------------
CEthernetSettingsConfigList::~CEthernetSettingsConfigList ()
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		POBJDELETE(m_pEthernetSettingsConfigList[i]);
	}	
}

// ------------------------------------------------------------
CEthernetSettingsConfigList& CEthernetSettingsConfigList::operator = (const CEthernetSettingsConfigList &rOther)
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		POBJDELETE(m_pEthernetSettingsConfigList[i]);

		m_pEthernetSettingsConfigList[i] =
			new CEthernetSettingsConfig( *(rOther.m_pEthernetSettingsConfigList[i]) );
	}	
    
	return *this;
}

// ------------------------------------------------------------
void CEthernetSettingsConfigList::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pEthernetSettingsConfigListNode;
 
	if(!pParentNode)
	{
		pParentNode =  new CXMLDOMElement();
		pParentNode->set_nodeName("ETHERNET_SETTINGS_LIST");
		pEthernetSettingsConfigListNode = pParentNode;
	}
	else
	{
		pEthernetSettingsConfigListNode = pParentNode->AddChildNode("ETHERNET_SETTINGS_LIST");
	}


	pEthernetSettingsConfigListNode->AddChildNode("OBJ_TOKEN", 0/*m_updateCounter*/);
	
//	if (ObjToken==0xFFFFFFFF)
//		bChanged=TRUE;
//	else
//	{
//		if(m_updateCounter>ObjToken)
//			bChanged=TRUE;
//	}
	BOOL bChanged = TRUE;
	pEthernetSettingsConfigListNode->AddChildNode("CHANGED", bChanged, _BOOL);	


	if(bChanged)
	{
	  eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
	  if (eProductTypeRMX1500 == curProductType)
	  {
		  for (int i=0; i<NUM_OF_LAN_PORTS_IN_YONA; i++)
		  		{

		 			m_pEthernetSettingsConfigList[i]->SerializeXml(pEthernetSettingsConfigListNode);
		  		}
	  }
	  else
	  {
		for (int i=0; i<NUM_OF_EFFECTIVE_LAN_PORTS; i++)
		{

			    m_pEthernetSettingsConfigList[i]->SerializeXml(pEthernetSettingsConfigListNode);
		}
	  }

	}

	return;
}

// ------------------------------------------------------------
void CEthernetSettingsConfigList::SerializeXmlForEma(CXMLDOMElement*& pParentNode) const
{
    CXMLDOMElement* pEthernetSettingsConfigListNode;

    if(!pParentNode)
    {
        pParentNode =  new CXMLDOMElement();
        pParentNode->set_nodeName("ETHERNET_SETTINGS_LIST");
        pEthernetSettingsConfigListNode = pParentNode;
    }
    else
    {
        pEthernetSettingsConfigListNode = pParentNode->AddChildNode("ETHERNET_SETTINGS_LIST");
    }


    pEthernetSettingsConfigListNode->AddChildNode("OBJ_TOKEN", 0/*m_updateCounter*/);

//  if (ObjToken==0xFFFFFFFF)
//      bChanged=TRUE;
//  else
//  {
//      if(m_updateCounter>ObjToken)
//          bChanged=TRUE;
//  }
    BOOL bChanged = TRUE;
    pEthernetSettingsConfigListNode->AddChildNode("CHANGED", bChanged, _BOOL);


    if(bChanged)
    {


      BOOL isFipsMode = NO;
      BOOL enableMd5 = YES;

      CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
      if (pSysConfig)
      {
    	  pSysConfig->GetBOOLDataByKey("802_FIPS_MODE", isFipsMode);
      }

      if (isFipsMode == YES)
    	  enableMd5 = NO;

      pEthernetSettingsConfigListNode->AddChildNode("FIPS_802_ENABLE_MD5", enableMd5, _BOOL);


      eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
      if (eProductTypeRMX1500 == curProductType)
      {
          for (int i=0; i<NUM_OF_LAN_PORTS_IN_YONA; i++)
                {
                   if (m_pEthernetSettingsConfigList[i]->GetSlotId() !=0)
                    m_pEthernetSettingsConfigList[i]->SerializeXmlForEma(pEthernetSettingsConfigListNode);
                }
      }
      else
      {
        for (int i=0; i<NUM_OF_EFFECTIVE_LAN_PORTS; i++)
        {
             if (m_pEthernetSettingsConfigList[i]->GetSlotId() !=0)
                m_pEthernetSettingsConfigList[i]->SerializeXmlForEma(pEthernetSettingsConfigListNode);
        }
      }

    }

    return;
}

// ------------------------------------------------------------
int  CEthernetSettingsConfigList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus=STATUS_OK;


	CXMLDOMElement* pEthernetSettingsConfigListNode = NULL;
	char* ParentNodeName;
		
	pActionNode->get_nodeName(&ParentNodeName); 

	if( !strcmp(ParentNodeName, "ETHERNET_SETTINGS_LIST") )
	{ 
		pEthernetSettingsConfigListNode = pActionNode; 
	}  
	else
	{
		GET_MANDATORY_CHILD_NODE(pActionNode, "ETHERNET_SETTINGS_LIST", pEthernetSettingsConfigListNode);
	}


	if (pEthernetSettingsConfigListNode)
	{
		//m_bChanged=TRUE;
		DWORD	m_updateCounter	= 0;
		BOOL	m_bChanged		= TRUE;
		
		GET_VALIDATE_CHILD(pEthernetSettingsConfigListNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);	
		GET_VALIDATE_CHILD(pEthernetSettingsConfigListNode, "CHANGED", &m_bChanged, _BOOL);	

		if(m_bChanged)
		{
		    DWORD tmp = 0;

			CXMLDOMElement *pEthernetSettingsNode;
			GET_FIRST_CHILD_NODE(pEthernetSettingsConfigListNode,"ETHERNET_SETTINGS",pEthernetSettingsNode);


			eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
			if (eProductTypeRMX1500 == curProductType)

			{
				for (int i=0; i<NUM_OF_LAN_PORTS_IN_YONA; i++)
					{
					 //if (m_pEthernetSettingsConfigList[i]->GetSlotId() !=0)
					 //{
						m_pEthernetSettingsConfigList[i]->DeSerializeXml(pEthernetSettingsNode, pszError, action);
						GET_NEXT_CHILD_NODE(pEthernetSettingsConfigListNode,"ETHERNET_SETTINGS",pEthernetSettingsNode);
						 if (pEthernetSettingsNode == NULL)
											 break;
					//}
			}
			}
			else
			{
			 for (int i=0; i<NUM_OF_EFFECTIVE_LAN_PORTS; i++)
			 {
				// if (m_pEthernetSettingsConfigList[i]->GetSlotId() !=0)
				 //{
					 m_pEthernetSettingsConfigList[i]->DeSerializeXml(pEthernetSettingsNode, pszError, action);
					 GET_NEXT_CHILD_NODE(pEthernetSettingsConfigListNode,"ETHERNET_SETTINGS",pEthernetSettingsNode);

					 if (pEthernetSettingsNode == NULL)
						 break;
				 //}

			 }
			}

		}
	}

	return nStatus;
}

// ------------------------------------------------------------
bool operator==(const CEthernetSettingsConfigList& first,const CEthernetSettingsConfigList& second)
{
	bool ret = false;       
	
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		if ( first.m_pEthernetSettingsConfigList[i] == second.m_pEthernetSettingsConfigList[i] )
		{
			ret = true;
		}
	}	

	return ret;
}

// ------------------------------------------------------------
void CEthernetSettingsConfigList::InitMembers()
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		m_pEthernetSettingsConfigList[i] = new CEthernetSettingsConfig;
		m_pEthernetSettingsConfigList[i]->InitMembers();
	}


	//if ( IsTarget() ) // the initial list is hard coded
	//{
		
	  eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
	  switch (curProductType)
	  {
	    case eProductTypeRMX1500 :
	 	  {
		 	m_pEthernetSettingsConfigList[0]->SetSlotId(FIXED_BOARD_ID_RTM_1);
		 	m_pEthernetSettingsConfigList[0]->SetPortType(eEthPortType_Media);
		 	m_pEthernetSettingsConfigList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	 		m_pEthernetSettingsConfigList[1]->SetSlotId(FIXED_BOARD_ID_RTM_1);
	 		m_pEthernetSettingsConfigList[1]->SetPortType(eEthPortType_Media);
	 		m_pEthernetSettingsConfigList[1]->SetPortId(ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD);

	 		m_pEthernetSettingsConfigList[2]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	 		m_pEthernetSettingsConfigList[2]->SetPortType(eEthPortType_Modem);
	 		m_pEthernetSettingsConfigList[2]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX1500);

	 		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
		    m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500]->SetPortType(eEthPortType_Signaling1);
		 	m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500);

	 		m_pEthernetSettingsConfigList[4]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	 		m_pEthernetSettingsConfigList[4]->SetPortType(eEthPortType_Management1);
	 		m_pEthernetSettingsConfigList[4]->SetPortId(ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500);



	 		m_pEthernetSettingsConfigList[5]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	 		m_pEthernetSettingsConfigList[5]->SetPortType(eEthPortType_ManagementShelfMngr);
	 		m_pEthernetSettingsConfigList[5]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX1500);


	 		  break;
	 	  }

	    case  eProductTypeRMX4000 :
	  {


		m_pEthernetSettingsConfigList[0]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_1 dynamic
		m_pEthernetSettingsConfigList[0]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);


		m_pEthernetSettingsConfigList[1]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_1 dynamic
		m_pEthernetSettingsConfigList[1]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);
	
		m_pEthernetSettingsConfigList[2]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_2 dynamic
		m_pEthernetSettingsConfigList[2]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[2]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

		m_pEthernetSettingsConfigList[3]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_2 dynamic
		m_pEthernetSettingsConfigList[3]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[3]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);


		m_pEthernetSettingsConfigList[4]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_3 dynamic
		m_pEthernetSettingsConfigList[4]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[4]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	
		m_pEthernetSettingsConfigList[5]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_3 dynamic
		m_pEthernetSettingsConfigList[5]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[5]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);


		m_pEthernetSettingsConfigList[6]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_4 dynamic
		m_pEthernetSettingsConfigList[6]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[6]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);


		m_pEthernetSettingsConfigList[7]->SetSlotId(0);//should be set to FIXED_BOARD_ID_RTM_4 dynamic
		m_pEthernetSettingsConfigList[7]->SetPortType(eEthPortType_Media);
		m_pEthernetSettingsConfigList[7]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);


		m_pEthernetSettingsConfigList[8]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
		m_pEthernetSettingsConfigList[8]->SetPortType(eEthPortType_Modem);
		m_pEthernetSettingsConfigList[8]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD);
	
		m_pEthernetSettingsConfigList[9]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
		m_pEthernetSettingsConfigList[9]->SetPortType(eEthPortType_Management1);
		m_pEthernetSettingsConfigList[9]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD);
	
		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000]->SetPortType(eEthPortType_Signaling1);
		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000]->SetPortId(ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD);
	
		m_pEthernetSettingsConfigList[11]->SetSlotId(0); //SHOULD BE FIXED_DISPLAY_BOARD_ID_SWITCH  NOT IMPLEMENTED YET
		m_pEthernetSettingsConfigList[11]->SetPortType(eEthPortType_Management2);
		m_pEthernetSettingsConfigList[11]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_2_PORT_ON_SWITCH_BOARD);

		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000]->SetSlotId(0);
		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000]->SetPortType(eEthPortType_Signaling2);
		m_pEthernetSettingsConfigList[ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000]->SetPortId(ETH_SETTINGS_CPU_SGNLNG_2_PORT_ON_SWITCH_BOARD);
	
		m_pEthernetSettingsConfigList[13]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
		m_pEthernetSettingsConfigList[13]->SetPortType(eEthPortType_ManagementShelfMngr);
		m_pEthernetSettingsConfigList[13]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD);
		 break;
	  }

	    case eProductTypeRMX2000 :
	    {

			m_pEthernetSettingsConfigList[0]->SetSlotId(0);
			m_pEthernetSettingsConfigList[0]->SetPortType(eEthPortType_Media);
			m_pEthernetSettingsConfigList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);



			m_pEthernetSettingsConfigList[1]->SetSlotId(0);
			m_pEthernetSettingsConfigList[1]->SetPortType(eEthPortType_Media);
			m_pEthernetSettingsConfigList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

			m_pEthernetSettingsConfigList[2]->SetSlotId(0);
			m_pEthernetSettingsConfigList[2]->SetPortType(eEthPortType_Media);
			m_pEthernetSettingsConfigList[2]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);


			m_pEthernetSettingsConfigList[3]->SetSlotId(0);
			m_pEthernetSettingsConfigList[3]->SetPortType(eEthPortType_Media);
			m_pEthernetSettingsConfigList[3]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);


			m_pEthernetSettingsConfigList[ETH_SETTINGS_MODEM_PORT_IDX_RMX2000]->SetSlotId(FIXED_BOARD_ID_SWITCH);
			m_pEthernetSettingsConfigList[ETH_SETTINGS_MODEM_PORT_IDX_RMX2000]->SetPortType(eEthPortType_Modem);
			m_pEthernetSettingsConfigList[ETH_SETTINGS_MODEM_PORT_IDX_RMX2000]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX2000);

			m_pEthernetSettingsConfigList[ETH_SETTINGS_SHELF_PORT_IDX_RMX2000]->SetSlotId(FIXED_BOARD_ID_SWITCH);
			m_pEthernetSettingsConfigList[ETH_SETTINGS_SHELF_PORT_IDX_RMX2000]->SetPortType(eEthPortType_ManagementShelfMngr_Managment_Signaling_media);
			m_pEthernetSettingsConfigList[ETH_SETTINGS_SHELF_PORT_IDX_RMX2000]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX2000);

			m_pEthernetSettingsConfigList[6]->SetSlotId(FIXED_BOARD_ID_SWITCH);
			m_pEthernetSettingsConfigList[6]->SetPortType(eEthPortType_Illegal);
			m_pEthernetSettingsConfigList[6]->SetPortId(ETH_SETTINGS_NOT_USED_PORT_ON_SWITCH_BOARD_RMX2000);




			 break;
	    }

        case eProductTypeGesher:
        {
            m_pEthernetSettingsConfigList[0]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
            m_pEthernetSettingsConfigList[0]->SetPortType(eEthPortType_Management1);
            m_pEthernetSettingsConfigList[0]->SetPortId(ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU);

            m_pEthernetSettingsConfigList[1]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
            m_pEthernetSettingsConfigList[1]->SetPortType(eEthPortType_Media_Signaling);
            m_pEthernetSettingsConfigList[1]->SetPortId(ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU);

            m_pEthernetSettingsConfigList[2]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
            m_pEthernetSettingsConfigList[2]->SetPortType(eEthPortType_Media_Signaling);
            m_pEthernetSettingsConfigList[2]->SetPortId(ETH_SETTINGS_ETH2_PORT_IDX_SOFTMCU);

            m_pEthernetSettingsConfigList[3]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
            m_pEthernetSettingsConfigList[3]->SetPortType(eEthPortType_Media_Signaling);
            m_pEthernetSettingsConfigList[3]->SetPortId(ETH_SETTINGS_ETH3_PORT_IDX_SOFTMCU);
			
            break;
        }

        case eProductTypeNinja:
        {
            m_pEthernetSettingsConfigList[0]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
            m_pEthernetSettingsConfigList[0]->SetPortType(eEthPortType_Media_Signaling_Managment);
            m_pEthernetSettingsConfigList[0]->SetPortId(ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU);

            m_pEthernetSettingsConfigList[1]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
            m_pEthernetSettingsConfigList[1]->SetPortType(eEthPortType_Media_Signaling_Managment);
            m_pEthernetSettingsConfigList[1]->SetPortId(ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU);
			
            break;
        }
		
	    default :
	    	break;
	  }




/*	}
	
	else // Simulation
	{
		m_pEthernetSettingsConfigList[0]->SetSlotId(FIXED_BOARD_ID_SWITCH_SIM);
		m_pEthernetSettingsConfigList[0]->SetPortId(1);
		
		m_pEthernetSettingsConfigList[1]->SetSlotId(FIXED_BOARD_ID_MEDIA_1_SIM);
		m_pEthernetSettingsConfigList[2]->SetPortId(1);
		
		m_pEthernetSettingsConfigList[2]->SetSlotId(FIXED_BOARD_ID_MEDIA_2_SIM);
		m_pEthernetSettingsConfigList[2]->SetPortId(1);
		
		m_pEthernetSettingsConfigList[3]->SetSlotId(FIXED_BOARD_ID_MEDIA_3_SIM);
		m_pEthernetSettingsConfigList[3]->SetPortId(1);
		
		m_pEthernetSettingsConfigList[4]->SetSlotId(FIXED_BOARD_ID_MEDIA_4_SIM);
		m_pEthernetSettingsConfigList[4]->SetPortId(1);
	}*/

}



// ------------------------------------------------------------
void  CEthernetSettingsConfigList::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n===== EthernetSettingsConfigList::Dump =====";

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		msg << "\nLAN port " << i+1 << ":"
			<< *m_pEthernetSettingsConfigList[i];
	}
}

// ------------------------------------------------------------
CEthernetSettingsConfig* CEthernetSettingsConfigList::GetSpecEthernetSettingsConfig(int idx) const
{
	if ( (0 <= idx) && (MAX_NUM_OF_LAN_PORTS > idx) )
	{
		return m_pEthernetSettingsConfigList[idx];
	}
	
	return NULL;
}

// ------------------------------------------------------------
void CEthernetSettingsConfigList::SetSpecEthernetSettingsConfig(const CEthernetSettingsConfig &ethernetSettings)
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{

		if ( (ethernetSettings.GetSlotId() == m_pEthernetSettingsConfigList[i]->GetSlotId()) &&
					 (ethernetSettings.GetPortId() == m_pEthernetSettingsConfigList[i]->GetPortId()) )
				{
					//m_pEthernetSettingsConfigList[i] = (CEthernetSettingsConfig*)&ethernetSettings;
					m_pEthernetSettingsConfigList[i]->SetPortSpeed( ethernetSettings.GetPortSpeed() );
					m_pEthernetSettingsConfigList[i]->SetPortType( ethernetSettings.GetPortType() );
					m_pEthernetSettingsConfigList[i]->Set802_1xAuthenticationProtocol( ethernetSettings.Get802_1xAuthenticationProtocol());
					m_pEthernetSettingsConfigList[i]->Set802_1xUserName( ethernetSettings.Get802_1xUserName());
					m_pEthernetSettingsConfigList[i]->Set802_1xUserPassword( ethernetSettings.Get802_1xUserPassword());
					break;
				}
	}
}

// ------------------------------------------------------------
void CEthernetSettingsConfigList::SetIsFromEma(bool isForEma)
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{

		m_pEthernetSettingsConfigList[i]->SetIsFromEma(isForEma);
	}
}






