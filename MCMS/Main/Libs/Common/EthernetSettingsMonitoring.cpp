// EthernetSettingsConfig.cpp: implementation of the CEthernetSettingsStructWrapper class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "EthernetSettingsMonitoring.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "ProcessBase.h"


extern const char* PortSpeedTypeToString(ePortSpeedType type);



// ------------------------------------------------------------
CEthernetSettingsStructWrapper::CEthernetSettingsStructWrapper (const bool isStructOnly/*=false*/)
{
	m_isStructOnly			= isStructOnly;
	m_pEthSettingsStruct	= new ETH_SETTINGS_SPEC_S;

	InitMembers();
}

// ------------------------------------------------------------
CEthernetSettingsStructWrapper::CEthernetSettingsStructWrapper (const CEthernetSettingsStructWrapper& other)
: CPObject(other)
{
	m_isStructOnly	= other.m_isStructOnly;
	m_isMounted		= other.m_isMounted;

	memset(m_pEthSettingsStruct, 0, sizeof(ETH_SETTINGS_SPEC_S));

	memcpy( m_pEthSettingsStruct,
			other.m_pEthSettingsStruct,
			sizeof(ETH_SETTINGS_SPEC_S) );
}

// ------------------------------------------------------------
CEthernetSettingsStructWrapper::~CEthernetSettingsStructWrapper ()
{
	delete m_pEthSettingsStruct;
}

// ------------------------------------------------------------
CEthernetSettingsStructWrapper& CEthernetSettingsStructWrapper::operator = (const CEthernetSettingsStructWrapper &rOther)
{
	m_isStructOnly	= rOther.m_isStructOnly;
	m_isMounted		= rOther.m_isMounted;

	memcpy( m_pEthSettingsStruct,
			rOther.m_pEthSettingsStruct,
			sizeof(ETH_SETTINGS_SPEC_S) );

	return *this;
}

// ------------------------------------------------------------
bool operator==(const CEthernetSettingsStructWrapper& first,const CEthernetSettingsStructWrapper& second)
{
	bool ret = false;
	
	ETH_SETTINGS_SPEC_S	*pFirstStruct	= first.GetEthSettingsStruct(),
						*pSecondStruct	= second.GetEthSettingsStruct();
	
	if ( (pFirstStruct->portParams.slotId	== pSecondStruct->portParams.slotId)	&&
		 (pFirstStruct->portParams.portNum	== pSecondStruct->portParams.portNum) )
	{
		ret = true;
	}

	return ret;
}

// ------------------------------------------------------------
bool operator==(const CEthernetSettingsStructWrapper& theObject, const ETH_SETTINGS_PORT_DESC_S& theStruct)
{
	bool ret = false;       
	
	ETH_SETTINGS_SPEC_S	*theObjectStruct = theObject.GetEthSettingsStruct();

	if ( (theObjectStruct->portParams.slotId	== theStruct.slotId)	&&
		 (theObjectStruct->portParams.portNum	== theStruct.portNum) )
	{
		ret = true;
	}

	return ret;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::InitMembers()
{
	m_isMounted = false;
	memset(m_pEthSettingsStruct, 0, sizeof(ETH_SETTINGS_SPEC_S));
}

// ------------------------------------------------------------
void  CEthernetSettingsStructWrapper::Dump(ostream& msg) const
{
	string sIsMounted = "";
	if (m_isStructOnly) // ('m_isMounted' attribute is irrelevant)
		sIsMounted = "(irrelevant)";
	else
		sIsMounted = (true == m_isMounted) ? "yes" : "no";


	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n===== CEthernetSettingsStructWrapper::Dump =====\n"
		<< std::setw(20) << "slot id"			<< m_pEthSettingsStruct->portParams.slotId	<< "\n"
		<< std::setw(20) << "port id"			<< m_pEthSettingsStruct->portParams.portNum	<< "\n"
		<< std::setw(20) << "is mounted"		<< sIsMounted.c_str()						<< "\n"
		<< "-----------------------"														<< "\n"
		<< std::setw(20) << "RxPackets"			<< m_pEthSettingsStruct->monitoringParams.ulRxPackets		<< "\n"
		<< std::setw(20) << "RxBadPackets"		<< m_pEthSettingsStruct->monitoringParams.ulRxBadPackets	<< "\n"
		<< std::setw(20) << "RxCRC"				<< m_pEthSettingsStruct->monitoringParams.ulRxCRC			<< "\n"
		<< std::setw(20) << "RxOctets"			<< m_pEthSettingsStruct->monitoringParams.ulRxOctets		<< "\n"
		<< std::setw(20) << "MaxRxPackets"		<< m_pEthSettingsStruct->monitoringParams.ulMaxRxPackets	<< "\n"
		<< std::setw(20) << "MaxRxBadPackets"	<< m_pEthSettingsStruct->monitoringParams.ulMaxRxBadPackets	<< "\n"
		<< std::setw(20) << "MaxRxCRC"			<< m_pEthSettingsStruct->monitoringParams.ulMaxRxCRC		<< "\n"
		<< std::setw(20) << "MaxRxOctets"		<< m_pEthSettingsStruct->monitoringParams.ulMaxRxOctets		<< "\n"
		<< std::setw(20) << "TxPackets"			<< m_pEthSettingsStruct->monitoringParams.ulTxPackets		<< "\n"
		<< std::setw(20) << "TxBadPackets"		<< m_pEthSettingsStruct->monitoringParams.ulTxBadPackets	<< "\n"
		<< std::setw(20) << "TxFifoDrops"		<< m_pEthSettingsStruct->monitoringParams.ulTxFifoDrops		<< "\n"
		<< std::setw(20) << "TxOctets"			<< m_pEthSettingsStruct->monitoringParams.ulTxOctets		<< "\n"
		<< std::setw(20) << "MaxTxPackets"		<< m_pEthSettingsStruct->monitoringParams.ulMaxTxPackets	<< "\n"
		<< std::setw(20) << "MaxTxBadPackets"	<< m_pEthSettingsStruct->monitoringParams.ulMaxTxBadPackets	<< "\n"
		<< std::setw(20) << "MaxTxFifoDrops"	<< m_pEthSettingsStruct->monitoringParams.ulMaxTxFifoDrops	<< "\n"
		<< std::setw(20) << "MaxTxOctets"		<< m_pEthSettingsStruct->monitoringParams.ulMaxTxOctets		<< "\n"
		<< std::setw(20) << "ActLinkStatus"		<< m_pEthSettingsStruct->monitoringParams.ulActLinkStatus	<< "\n"
		<< std::setw(20) << "ActLinkMode"		<< m_pEthSettingsStruct->monitoringParams.ulActLinkMode		<< "\n"
		<< std::setw(20) << "ActLinkAutoNeg"	<< m_pEthSettingsStruct->monitoringParams.ulActLinkAutoNeg	<< "\n"
		<< std::setw(20) << "AdvLinkMode"		<< m_pEthSettingsStruct->monitoringParams.ulAdvLinkMode		<< "\n"
		<< std::setw(20) << "AdvLinkAutoNeg"	<< m_pEthSettingsStruct->monitoringParams.ulAdvLinkAutoNeg	<< "\n"
	    << std::setw(20) << "ActLinkAutoNeg"	<< m_pEthSettingsStruct->monitoringParams.e802_1xSuppPortStatus	<< "\n"
		<< std::setw(20) << "AdvLinkMode"		<< m_pEthSettingsStruct->monitoringParams.e802_1xMethod		<< "\n"
		<< std::setw(20) << "AdvLinkAutoNeg"	<< m_pEthSettingsStruct->monitoringParams.e802_1xFailReason	<< "\n";

}


// ------------------------------------------------------------
DWORD CEthernetSettingsStructWrapper::GetSlotId() const
{
	return m_pEthSettingsStruct->portParams.slotId;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::SetSlotId(const DWORD newSlotId)
{
	m_pEthSettingsStruct->portParams.slotId = newSlotId;
}

// ------------------------------------------------------------
DWORD CEthernetSettingsStructWrapper::GetPortId() const
{
	return m_pEthSettingsStruct->portParams.portNum;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::SetPortId(const DWORD newPortId)
{
	m_pEthSettingsStruct->portParams.portNum = newPortId;
}

// ------------------------------------------------------------
bool CEthernetSettingsStructWrapper::GetIsMounted() const
{
	return m_isMounted;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::SetIsMounted(const bool isMounted)
{
	m_isMounted = isMounted;
}

// ------------------------------------------------------------
ETH_SETTINGS_SPEC_S* CEthernetSettingsStructWrapper::GetEthSettingsStruct() const
{
	return m_pEthSettingsStruct;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::SetEthSettingsStruct(const ETH_SETTINGS_SPEC_S *pOtherStruct)
{
	memcpy( m_pEthSettingsStruct,
			pOtherStruct,
			sizeof(ETH_SETTINGS_SPEC_S) );	
}


// ------------------------------------------------------------
bool CEthernetSettingsStructWrapper::IsCpuPort()
{
	bool retVal = false;
	
	DWORD slotId = m_pEthSettingsStruct->portParams.slotId,
		  portId = m_pEthSettingsStruct->portParams.portNum;

	eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();

	if ( curProductType == eProductTypeRMX4000)
	{
	
	  if (  (FIXED_DISPLAY_BOARD_ID_SWITCH == slotId) &&
		  ( (ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD == portId) ||
			(ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD == portId) ||
			(ETH_SETTINGS_CPU_MNGMNT_2_PORT_ON_SWITCH_BOARD == portId) ||
			(ETH_SETTINGS_CPU_SGNLNG_2_PORT_ON_SWITCH_BOARD == portId) )  )
		{
			retVal = true;
		}
	}
	else if ( curProductType == eProductTypeRMX1500)
	{
		if (  (FIXED_DISPLAY_BOARD_ID_SWITCH == slotId) &&
				  ( (ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500 == portId) ||
					(ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500 == portId)   ))
				{
					retVal = true;
				}
	}
	
	return retVal;
}


// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::UpdateMaxCounters()
{
	if ( m_pEthSettingsStruct->monitoringParams.ulMaxRxPackets < m_pEthSettingsStruct->monitoringParams.ulRxPackets )
		m_pEthSettingsStruct->monitoringParams.ulMaxRxPackets = m_pEthSettingsStruct->monitoringParams.ulRxPackets;
	
	if ( m_pEthSettingsStruct->monitoringParams.ulMaxRxBadPackets < m_pEthSettingsStruct->monitoringParams.ulRxBadPackets )
		m_pEthSettingsStruct->monitoringParams.ulMaxRxBadPackets = m_pEthSettingsStruct->monitoringParams.ulRxBadPackets;
	
	if ( m_pEthSettingsStruct->monitoringParams.ulMaxRxCRC < m_pEthSettingsStruct->monitoringParams.ulRxCRC )
		m_pEthSettingsStruct->monitoringParams.ulMaxRxCRC = m_pEthSettingsStruct->monitoringParams.ulRxCRC;
	
	if ( m_pEthSettingsStruct->monitoringParams.ulMaxRxOctets < m_pEthSettingsStruct->monitoringParams.ulRxOctets )
		m_pEthSettingsStruct->monitoringParams.ulMaxRxOctets = m_pEthSettingsStruct->monitoringParams.ulRxOctets;


	if ( m_pEthSettingsStruct->monitoringParams.ulMaxTxPackets < m_pEthSettingsStruct->monitoringParams.ulTxPackets )
		m_pEthSettingsStruct->monitoringParams.ulMaxTxPackets = m_pEthSettingsStruct->monitoringParams.ulTxPackets;

	if ( m_pEthSettingsStruct->monitoringParams.ulMaxTxBadPackets < m_pEthSettingsStruct->monitoringParams.ulTxBadPackets )
		m_pEthSettingsStruct->monitoringParams.ulMaxTxBadPackets = m_pEthSettingsStruct->monitoringParams.ulTxBadPackets;

	if ( m_pEthSettingsStruct->monitoringParams.ulMaxTxFifoDrops < m_pEthSettingsStruct->monitoringParams.ulTxFifoDrops )
		m_pEthSettingsStruct->monitoringParams.ulMaxTxFifoDrops = m_pEthSettingsStruct->monitoringParams.ulTxFifoDrops;

	if ( m_pEthSettingsStruct->monitoringParams.ulMaxTxOctets < m_pEthSettingsStruct->monitoringParams.ulTxOctets )
		m_pEthSettingsStruct->monitoringParams.ulMaxTxOctets = m_pEthSettingsStruct->monitoringParams.ulTxOctets;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrapper::ClearMaxCounters()
{
	UINT32 clearVal = 0;

	m_pEthSettingsStruct->monitoringParams.ulMaxRxPackets		= clearVal;
	m_pEthSettingsStruct->monitoringParams.ulMaxRxBadPackets	= clearVal;
	m_pEthSettingsStruct->monitoringParams.ulMaxRxCRC			= clearVal;
	m_pEthSettingsStruct->monitoringParams.ulMaxRxOctets		= clearVal;

	m_pEthSettingsStruct->monitoringParams.ulMaxTxPackets		= clearVal;
	m_pEthSettingsStruct->monitoringParams.ulMaxTxBadPackets	= clearVal;
	m_pEthSettingsStruct->monitoringParams.ulMaxTxFifoDrops		= clearVal;
	m_pEthSettingsStruct->monitoringParams.ulMaxTxOctets		= clearVal;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//					CEthernetSettingsStructWrappersList
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------
CEthernetSettingsStructWrappersList::CEthernetSettingsStructWrappersList ()
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		m_pEthernetSettingsStructWrapperList[i] = new CEthernetSettingsStructWrapper;
	}

	InitMembers();
}

// ------------------------------------------------------------
CEthernetSettingsStructWrappersList::CEthernetSettingsStructWrappersList (const CEthernetSettingsStructWrappersList& other)
: CPObject(other)
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		POBJDELETE(m_pEthernetSettingsStructWrapperList[i]);

		m_pEthernetSettingsStructWrapperList[i] =
			new CEthernetSettingsStructWrapper( *(other.m_pEthernetSettingsStructWrapperList[i]) );
	}	
}

// ------------------------------------------------------------
CEthernetSettingsStructWrappersList::~CEthernetSettingsStructWrappersList ()
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		POBJDELETE(m_pEthernetSettingsStructWrapperList[i]);
	}	
}

// ------------------------------------------------------------
CEthernetSettingsStructWrappersList& CEthernetSettingsStructWrappersList::operator = (const CEthernetSettingsStructWrappersList &rOther)
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		POBJDELETE(m_pEthernetSettingsStructWrapperList[i]);

		m_pEthernetSettingsStructWrapperList[i] =
			new CEthernetSettingsStructWrapper( *(rOther.m_pEthernetSettingsStructWrapperList[i]) );
	}	
    
	return *this;
}

// ------------------------------------------------------------
bool operator==(const CEthernetSettingsStructWrappersList& first,const CEthernetSettingsStructWrappersList& second)
{
	bool ret = false;       
	
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		if ( first.m_pEthernetSettingsStructWrapperList[i] == second.m_pEthernetSettingsStructWrapperList[i] )
		{
			ret = true;
		}
	}	

	return ret;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrappersList::InitMembers()
{
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		m_pEthernetSettingsStructWrapperList[i]->InitMembers();
	}


	//if ( IsTarget() ) // the initial list is hard coded
	//{
		eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
		switch (curProductType)
		{
	    case eProductTypeRMX1500 :
		  {

			m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_RTM_1);
			m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

			m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_RTM_1);
			m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

			m_pEthernetSettingsStructWrapperList[2]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
			m_pEthernetSettingsStructWrapperList[2]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX1500);

			m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
			m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500);

			m_pEthernetSettingsStructWrapperList[4]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
			m_pEthernetSettingsStructWrapperList[4]->SetPortId(ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500);


			m_pEthernetSettingsStructWrapperList[5]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
			m_pEthernetSettingsStructWrapperList[5]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX1500);
			break;
		  }
	    case eProductTypeRMX4000 :
	    {

	    	//change default value of 13-16 to port 2. (VNGR-12831).
	    	m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_RTM_1);
	    	m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_RTM_1);
	    	m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[2]->SetSlotId(FIXED_BOARD_ID_RTM_2);
	    	m_pEthernetSettingsStructWrapperList[2]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[3]->SetSlotId(FIXED_BOARD_ID_RTM_2);
	    	m_pEthernetSettingsStructWrapperList[3]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[4]->SetSlotId(FIXED_BOARD_ID_RTM_3);
	    	m_pEthernetSettingsStructWrapperList[4]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[5]->SetSlotId(FIXED_BOARD_ID_RTM_3);
	    	m_pEthernetSettingsStructWrapperList[5]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[6]->SetSlotId(FIXED_BOARD_ID_RTM_4);
	    	m_pEthernetSettingsStructWrapperList[6]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[7]->SetSlotId(FIXED_BOARD_ID_RTM_4);
	    	m_pEthernetSettingsStructWrapperList[7]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[8]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[8]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[9]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[9]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000]->SetPortId(ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[11]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[11]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_2_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000]->SetPortId(ETH_SETTINGS_CPU_SGNLNG_2_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[13]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[13]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD);
	    	break;
	    }

	    case eProductTypeRMX2000 :

	    {

	    	//change default value of 13-16 to port 2. (VNGR-12831).

	    	m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_MEDIA_1);
	    	m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_MEDIA_1);
	    	m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[2]->SetSlotId(FIXED_BOARD_ID_MEDIA_2);
	    	m_pEthernetSettingsStructWrapperList[2]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[3]->SetSlotId(FIXED_BOARD_ID_MEDIA_2);
	    	m_pEthernetSettingsStructWrapperList[3]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_MODEM_PORT_IDX_RMX2000]->SetSlotId(FIXED_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_MODEM_PORT_IDX_RMX2000]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX2000);

	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_SHELF_PORT_IDX_RMX2000]->SetSlotId(FIXED_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[ETH_SETTINGS_SHELF_PORT_IDX_RMX2000]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX2000);

	    	m_pEthernetSettingsStructWrapperList[6]->SetSlotId(FIXED_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[6]->SetPortId(ETH_SETTINGS_NOT_USED_PORT_ON_SWITCH_BOARD_RMX2000);


	    	break;
	    }

	    case eProductTypeSoftMCU:
	    case eProductTypeSoftMCUMfw:
	    case eProductTypeEdgeAxis:
	    case eProductTypeCallGeneratorSoftMCU:
	    {

	    	//change default value of 13-16 to port 2. (VNGR-12831).

	    	m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_MEDIA_1);
	    	m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_MEDIA_1);
	    	m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[2]->SetSlotId(FIXED_BOARD_ID_MEDIA_2);
	    	m_pEthernetSettingsStructWrapperList[2]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[3]->SetSlotId(FIXED_BOARD_ID_MEDIA_2);
	    	m_pEthernetSettingsStructWrapperList[3]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);

	    	m_pEthernetSettingsStructWrapperList[4]->SetSlotId(FIXED_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[4]->SetPortId(ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[5]->SetSlotId(FIXED_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[5]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD);

	    	m_pEthernetSettingsStructWrapperList[6]->SetSlotId(FIXED_BOARD_ID_SWITCH);
	    	m_pEthernetSettingsStructWrapperList[6]->SetPortId(ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX2000);
	    	break;
	    }

	    case eProductTypeGesher:
	    {
	    	m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
	    	m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU);

	    	m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
	    	m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU);

	    	m_pEthernetSettingsStructWrapperList[2]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
	    	m_pEthernetSettingsStructWrapperList[2]->SetPortId(ETH_SETTINGS_ETH2_PORT_IDX_SOFTMCU);

	    	m_pEthernetSettingsStructWrapperList[3]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
	    	m_pEthernetSettingsStructWrapperList[3]->SetPortId(ETH_SETTINGS_ETH3_PORT_IDX_SOFTMCU);

	    	break;
	    }

	    case eProductTypeNinja:
	    {
	    	m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
	    	m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU);

	    	m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
	    	m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU);
			
	    	break;
	    }
		
	    default:
	    {
		m_pEthernetSettingsStructWrapperList[0]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);
		m_pEthernetSettingsStructWrapperList[1]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);
		m_pEthernetSettingsStructWrapperList[2]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);
		m_pEthernetSettingsStructWrapperList[3]->SetPortId(ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD);
		break;
		}


		}




	/*}
	
	else // Simulation
	{
		m_pEthernetSettingsStructWrapperList[0]->SetSlotId(FIXED_BOARD_ID_SWITCH_SIM);
		m_pEthernetSettingsStructWrapperList[0]->SetPortId(1);
		
		m_pEthernetSettingsStructWrapperList[1]->SetSlotId(FIXED_BOARD_ID_MEDIA_1_SIM);
		m_pEthernetSettingsStructWrapperList[2]->SetPortId(1);
		
		m_pEthernetSettingsStructWrapperList[2]->SetSlotId(FIXED_BOARD_ID_MEDIA_2_SIM);
		m_pEthernetSettingsStructWrapperList[2]->SetPortId(1);
		
		m_pEthernetSettingsStructWrapperList[3]->SetSlotId(FIXED_BOARD_ID_MEDIA_3_SIM);
		m_pEthernetSettingsStructWrapperList[3]->SetPortId(1);
		
		m_pEthernetSettingsStructWrapperList[4]->SetSlotId(FIXED_BOARD_ID_MEDIA_4_SIM);
		m_pEthernetSettingsStructWrapperList[4]->SetPortId(1);
	}*/

}


// ------------------------------------------------------------
void  CEthernetSettingsStructWrappersList::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n===== EthernetSettingsList::Dump =====";

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		msg << "\nLAN port " << i+1 << ":"
			<< *m_pEthernetSettingsStructWrapperList[i];
	}
}

// ------------------------------------------------------------
CEthernetSettingsStructWrapper* CEthernetSettingsStructWrappersList::GetSpecEthernetSettingsStructWrapper(int idx) const
{
	if ( (0 <= idx) && (MAX_NUM_OF_LAN_PORTS > idx) )
	{
		return m_pEthernetSettingsStructWrapperList[idx];
	}
	
	return NULL;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrappersList::SetSpecEthernetSettingsStructWrapper(ETH_SETTINGS_SPEC_S* pNewStruct) const
{
	CEthernetSettingsStructWrapper *pCurEth = NULL;

	/*TRACESTR(eLevelInfoNormal) << "\nCEthernetSettingsStructWrappersList::SetSpecEthernetSettingsStructWrapper"
						   << "\nboardId: " << pNewStruct->portParams.slotId << ", portId: " << pNewStruct->portParams.portNum
						   << ", ulActLinkStatus: " << pNewStruct->monitoringParams.ulActLinkStatus
						   <<"\ne802_1xSuppPortStatus "<<pNewStruct->monitoringParams.e802_1xSuppPortStatus
						   <<"\ne802_1xMethod "<<pNewStruct->monitoringParams.e802_1xMethod
						   <<"\ne802_1xFailReason "<<pNewStruct->monitoringParams.e802_1xFailReason;*/

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEth = GetSpecEthernetSettingsStructWrapper(i);

		// update the structure
		if (*pCurEth == pNewStruct->portParams) // similar slotId and portId
		{
			//TRACESTR(eLevelInfoNormal) << "\nCEthernetSettingsStructWrappersList::SetSpecEthernetSettingsStructWrapper - do the set";
			pCurEth->SetEthSettingsStruct(pNewStruct);
			break;
		}
	} // end loop
}

// ------------------------------------------------------------
bool CEthernetSettingsStructWrappersList::GetIsMounted(const DWORD slotId, const DWORD portId) const
{
	bool retIsMounted = false;

	ETH_SETTINGS_SPEC_S* pCurStruct = NULL;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurStruct = m_pEthernetSettingsStructWrapperList[i]->GetEthSettingsStruct();
		if ( (slotId == pCurStruct->portParams.slotId) &&
			 (portId == pCurStruct->portParams.portNum) )
		{
			retIsMounted = m_pEthernetSettingsStructWrapperList[i]->GetIsMounted();
			break;
		}
	} // end loop
	
	return retIsMounted;
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrappersList::SetIsMounted( const DWORD slotId,
													    const DWORD portId,
													    const bool isMounted )
{
	//temp - for debugging
	TRACESTR(eLevelInfoNormal) << "\nCEthernetSettingsStructWrappersList::SetIsMounted"
						   << "\nboardId: " << slotId << ", portId: " << portId
						   << ", isMounted: " << (int)isMounted << " - before setting";

	ETH_SETTINGS_SPEC_S* pCurStruct = NULL;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurStruct = m_pEthernetSettingsStructWrapperList[i]->GetEthSettingsStruct();
		if ( (slotId == pCurStruct->portParams.slotId) )
		{
			if (portId == pCurStruct->portParams.portNum)
			{
				m_pEthernetSettingsStructWrapperList[i]->SetIsMounted(isMounted);
				
				//temp - for debugging
				TRACESTR(eLevelInfoNormal) << "\nCEthernetSettingsStructWrappersList::SetIsMounted"
									   << "\nboardId: " << slotId << ", portId: " << portId
									   << ", isMounted: " << (int)isMounted << " - is set!";
			}
			/* colinzuo: comment out this old code, judith doesn't remember its purpose
			//for slot 13-16, the port might be changed from 1 to 2.  
			else if (slotId>=13 && slotId<=16)
			{
				m_pEthernetSettingsStructWrapperList[i]->SetPortId(portId);
				m_pEthernetSettingsStructWrapperList[i]->SetIsMounted(isMounted);

				//temp - for debugging
				TRACESTR(eLevelInfoNormal) << "\nCEthernetSettingsStructWrappersList::SetIsMounted"
									   << "\nboardId: " << slotId << ", portId: " << portId
									   << ", isMounted: " << (int)isMounted << " - is set!";
			}
			*/
		}
		
	} // end loop
}

// ------------------------------------------------------------
/*
void CEthernetSettingsStructWrappersList::SetIsMounted(const DWORD slotId, const DWORD portId, const bool isMounted)
{
	ETH_SETTINGS_SPEC_S* pCurStruct = NULL;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurStruct = m_pEthernetSettingsStructWrapperList[i]->GetEthSettingsStruct();
		if ( (slotId == pCurStruct->portParams.slotId) &&
			 (portId == pCurStruct->portParams.portNum) )
		{
			m_pEthernetSettingsStructWrapperList[i]->SetIsMounted(isMounted);
			break;
		}
	} // end loop
}
*/
// ------------------------------------------------------------
void CEthernetSettingsStructWrappersList::UpdateSpecMaxCounters(const DWORD slotId, const DWORD portId)
{
	ETH_SETTINGS_SPEC_S* pCurStruct = NULL;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurStruct = m_pEthernetSettingsStructWrapperList[i]->GetEthSettingsStruct();
		if ( (slotId == pCurStruct->portParams.slotId) &&
			 (portId == pCurStruct->portParams.portNum) )
		{
			m_pEthernetSettingsStructWrapperList[i]->UpdateMaxCounters();
			break;
		}
	} // end loop
}

// ------------------------------------------------------------
void CEthernetSettingsStructWrappersList::ClearSpecMaxCounters(const DWORD slotId, const DWORD portId)
{
	ETH_SETTINGS_SPEC_S* pCurStruct = NULL;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurStruct = m_pEthernetSettingsStructWrapperList[i]->GetEthSettingsStruct();
		if ( (slotId == pCurStruct->portParams.slotId) &&
			 (portId == pCurStruct->portParams.portNum) )
		{
			m_pEthernetSettingsStructWrapperList[i]->ClearMaxCounters();
			break;
		}
	} // end loop
}
