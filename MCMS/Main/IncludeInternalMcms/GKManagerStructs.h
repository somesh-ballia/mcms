//+========================================================================+
//                  GKManagerStructs.h									   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GKManagerStructs.h                                          |
// SUBSYSTEM:  Libs/Common		                                           |
// PROGRAMMER: Yael A.	                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |   The file includes structures interface between     |
//						GkManger process and other processes in Carmel     |
//						(ConfParty, CsMngr..)                              |
//+========================================================================+


#ifndef _GKManagerStructs_H_
#define _GKManagerStructs_H_


#include "GlobalCarmelDefinitions.h"
#include "DefinesIpService.h"
#include "IpAddressDefinitions.h"
#include "McuMngrStructs.h"
#include "GKManagerOpcodes.h"
#include "GateKeeperCommonParams.h"
#include "DefinesGeneral.h"


/******************************************************************************************/
/*							  Interface between GkManger and CsMngr						  */
/******************************************************************************************/

typedef struct
{
	BOOL          		        isAuthenticationEnabled;
	eAuthenticationProtocol     authenticationProtocol;
	char   			            user_name[USER_NAME_LEN];
	char  			            password[PASSWORD_LEN];
} authenticationStruct;



/* opcode: CS_GKMNGR_IP_SERVICE_PARAM_IND
   opcode: CS_GKMNGR_IP_SERVICE_UPDATE_PARAM_IND  */
struct GkManagerServiceParamsIndStruct
{
	DWORD						serviceId;
	char						serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
	BYTE						bIsGkInService;

	//	APIS32				prefix; //(-1 if not exist)
	char               		    prefixName[ALIAS_NAME_LEN]; // '\0' if not exist

	BYTE						bIsRegAsGw; //new in Carmel: used to be one of the GK modes in MGC
	BYTE						bIsAvf; //new in Carmel: used to be one of the GK modes in MGC
	WORD						rrqPolingInterval;
	ALIAS_S	 		   			aliases[MAX_ALIAS_NAMES_NUM];
	ipAddressStruct				csIp[TOTAL_NUM_OF_IP_ADDRESSES];
	ipAddressStruct				gkIp;
	ipAddressStruct				alternateGkIp;
	char						gkName[H243_NAME_LEN];
	char						altGkName[H243_NAME_LEN];
	eIpType             		service_ip_protocol_types;
	authenticationStruct        authenticationParams;

};

//////////////////////////////////////////////////////////////////////////
/* opcode: CS_GKMNGR_UPDATE_SERVICE_PROPERTIES_REQ */
struct GkManagerUpdateServicePropertiesReqStruct
{
	DWORD						serviceId;
	WORD						serviceStatus;
	eGkFaultsAndServiceStatus	eServiceOpcode;
	char						gkId[MaxIdentifierSize];
	WORD						rrqPolingInterval;
	eGKConnectionState			eGkConnState;
    BYTE                        ipPrecedenceAudio;   // may be [0..5] - priority for audio channel packets
    BYTE                        ipPrecedenceVideo;   // may be [0..5] - priority for video channel packets
};

//////////////////////////////////////////////////////////////////////////
/* opcode: CS_GKMNGR_CLEAR_GK_PARAMS_FROM_PROPERTIES_REQ */
struct ClearGkParamsFromPropertiesReqStruct
{
	DWORD	serviceId;
	APIS8	indexToClear; // 0-4 (-1 if need to clear all alts: 1-4)
};
//Analog functions in MGC: ClearAltGkParamsFromProperties and ClearAllAltGksParamsFromProperties


//////////////////////////////////////////////////////////////////////////
/* opcode: CS_GKMNGR_SET_GK_IP_IN_PROPERTIES_REQ */
struct SetGkIPInPropertiesReqStruct
{
	DWORD			serviceId;
	WORD			indexToSet; //0-4
	ipAddressStruct	gkIp;
};
//Analog functions in MGC: SetGkIP, SetGkAltIP, SetGkIP3, SetGkIP4, SetGkIP5


//////////////////////////////////////////////////////////////////////////
/* opcode: CS_GKMNGR_SET_GK_ID_IN_PROPERTIES_REQ */
struct SetGkIdInPropertiesReqStruct
{
	DWORD	serviceId;
	WORD	indexToSet;  //0-4
	char	gkId[MaxIdentifierSize];
};
//Analog functions in MGC: SetGkAltId, SetGatekeeperId3, SetGatekeeperId4, SetGatekeeperId5


//////////////////////////////////////////////////////////////////////////
/* opcode: CS_GKMNGR_SET_GK_NAME_IN_PROPERTIES_REQ */
struct SetGkNameInPropertiesReqStruct
{
	DWORD	serviceId;
	WORD	indexToSet;  //0-4
	char	gkName[H243_NAME_LEN];
};
//Analog functions in MGC: SetGatekeeper3Name, SetGatekeeper4Name, SetGatekeeper5Name
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

struct GKInfoStruct
{
	char			m_Name	[H243_NAME_LEN];
	char			m_GkId	[MaxIdentifierSize]; 	// use memcpy, can contain '\0'
	ipAddressStruct	m_GkIp;
	eDynamicGkRole  m_eDynamicGkRole;

public:
	GKInfoStruct()
	{
		Clear();
	}
	
	GKInfoStruct& operator=(const GKInfoStruct &rHnd)
	{
		if(&rHnd != this)
		{
			memcpy(m_Name	, rHnd.m_Name	, H243_NAME_LEN);
			memcpy(m_GkId	, rHnd.m_GkId	, MaxIdentifierSize);
			m_GkIp 				= rHnd.m_GkIp;
			m_eDynamicGkRole 	= rHnd.m_eDynamicGkRole;
		}
		return *this;
	}
	
	bool operator==(const GKInfoStruct &rHnd)const
	{
		if(this == &rHnd)
			return true;

		if (strcmp(m_Name, rHnd.m_Name) != 0)
			return false;
		if (strcmp(m_GkId, rHnd.m_GkId) != 0)
			return false;
		if (m_GkIp.ipVersion != rHnd.m_GkIp.ipVersion)
			return false;
		if (memcmp(&(m_GkIp.addr), &(rHnd.m_GkIp.addr), sizeof(ipAddressIf)) != 0)
			return false;
		if (m_eDynamicGkRole != rHnd.m_eDynamicGkRole)
			return false;
		
		return true;
	}

	bool operator!=(const GKInfoStruct& other)const
	{
		return (!(*this == other));
	}
	
	void Clear()
	{
		memset(m_Name	, '\0',H243_NAME_LEN);
		memset(m_GkId	, '\0',H243_NAME_LEN);

		memset(&m_GkIp, 0, sizeof m_GkIp);
		//m_GkIp.addr.v4.ip 	= 0;
		//memset(&(m_GkIp.addr.v6), 0, sizeof(ipAddressV6If));

		m_eDynamicGkRole 	= eGKBackup;
	}

private:
	// disabled
	GKInfoStruct(const GKInfoStruct&);
};


/* contains all parameters of monitoring GK --> CSMngr */
struct GKFullInfoStruct
{
	WORD						m_ServiceStatus;
	eGkFaultsAndServiceStatus	m_ServiceOpcode;
	WORD						m_RrqPolingInterval;
	eGKConnectionState		    m_GkConnectionState;
	GKInfoStruct 				m_GKInfoStruct[MAX_NUM_GK];

public:
	GKFullInfoStruct()
	{
		m_ServiceStatus 		= 0xFFFF;
		m_ServiceOpcode 		= eGkFaultsAndServiceStatusOk;
		m_RrqPolingInterval		= 0;
		m_GkConnectionState 	= eGKNonRegistrated;

		m_GKInfoStruct[0].m_eDynamicGkRole = eGKActive;
		for(int i = 1 ; i < MAX_NUM_GK ; i++)
		{
			m_GKInfoStruct[i].m_eDynamicGkRole = eGKBackup;
		}

	}
	GKFullInfoStruct& operator=(const GKFullInfoStruct &rHnd)
	{
		if(&rHnd != this)
		{
			m_ServiceStatus 		= rHnd.m_ServiceStatus;
			m_ServiceOpcode 		= rHnd.m_ServiceOpcode;
			m_RrqPolingInterval 	= rHnd.m_RrqPolingInterval;
			m_GkConnectionState 	= rHnd.m_GkConnectionState;
			for(int i = 0 ; i < MAX_NUM_GK ; i++)
			{
				m_GKInfoStruct[i] = rHnd.m_GKInfoStruct[i];
			}
		}
		return *this;
	}
	
	bool operator==(const GKFullInfoStruct &rHnd)const
	{
		if(this == &rHnd)
			return true;

		if (m_ServiceStatus != rHnd.m_ServiceStatus)
			return false;
		
		if (m_ServiceOpcode != rHnd.m_ServiceOpcode)
			return false;
		
		if (m_RrqPolingInterval != rHnd.m_RrqPolingInterval)
			return false;
		
		if (m_GkConnectionState != rHnd.m_GkConnectionState)
			return false;
		
		for (int i = 0 ; i < MAX_NUM_GK ; i++)
		{
			if (m_GKInfoStruct[i] != rHnd.m_GKInfoStruct[i])
				return false;
		}
		
		return true;
	}

	bool operator!=(const GKFullInfoStruct& other)const
	{
		return (!(*this == other));
	}

	void SetPrimaryGkIp(DWORD ip)
	{
		m_GKInfoStruct[0].m_GkIp.addr.v4.ip = ip;
	}
	void SetAltGkIp(DWORD ip)
	{
		m_GKInfoStruct[1].m_GkIp.addr.v4.ip = ip;
	}
	void SetGKInfo(const GkManagerUpdateServicePropertiesReqStruct &info)
	{
		m_ServiceStatus 		= info.serviceStatus;
		m_ServiceOpcode 		= info.eServiceOpcode;
		m_RrqPolingInterval 	= info.rrqPolingInterval;
		m_GkConnectionState 	= info.eGkConnState;
		memcpy(m_GKInfoStruct[0].m_GkId, info.gkId, MaxIdentifierSize);
	}
	void SetGKInfo(const SetGkIPInPropertiesReqStruct &info)
	{
		int index = GetValidateIndex(info.indexToSet, false);
		m_GKInfoStruct[index].m_GkIp = info.gkIp;
	}
	void SetGKInfo(const SetGkNameInPropertiesReqStruct &info)
	{
		int index = GetValidateIndex(info.indexToSet);
		memcpy(m_GKInfoStruct[index].m_Name, info.gkName, H243_NAME_LEN);
	}
	void SetGKInfo(const SetGkIdInPropertiesReqStruct &info)
	{
		int index = GetValidateIndex(info.indexToSet);
		memcpy(m_GKInfoStruct[index].m_GkId, info.gkId, MaxIdentifierSize);
	}
	void SetGKInfo(const ClearGkParamsFromPropertiesReqStruct &info)
	{
		if(-1 == info.indexToClear)
		{
			for(int i = 1 ; i < MAX_NUM_GK ; i++) // it clears only alts
			{
				m_GKInfoStruct[i].Clear();
			}
		}
		else
		{
			int index = GetValidateIndex(info.indexToClear);
			m_GKInfoStruct[index].Clear();
		}
	}
	WORD GetValidateIndex(WORD index, bool isAlt = true)
	{
		const WORD lowBound = (isAlt ? 1 : 0);
		WORD result = (lowBound <= index && index < MAX_NUM_GK ? index : lowBound);
		return result;
	}


private:
	// disabled
	GKFullInfoStruct(const GKFullInfoStruct&);
};


/**********************************************************************************************/
/*							    Interface between GkManger and ConfParty					  */
/**********************************************************************************************/

struct HeaderToGkManagerStruct
{
//for PORT_DESCRIPTION_HEADER_S:
	APIU32 connId;
	APIU32 partyId;
	APIU32 confId;

//for CENTRAL_SIGNALING_HEADER_S:
	APIU16 csId;
    APIU32 callIndex;
	APIU32 serviceId;
	APIS32 status;
};
//////////////////////////////////////////////////////////////////////////

struct ArqIndToPartyStruct
{
	APIS32  status;
    char    gwPrefix[PHONE_NUMBER_DIGITS_LEN+1]; // '\0' if not exist
};


/**********************************************************************************************/
/*							    Interface between GkManger and McuMngr  					  */
/**********************************************************************************************/

// MCUMNGR_GK_MNGMNT_IND
struct MngmntParamStruct
{
    DWORD ipAddress;
};


#endif /*_GKManagerStructs_H_*/

