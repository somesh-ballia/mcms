// RtmIsdnMngrProcess.cpp: implementation of the CRtmIsdnMngrProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "RtmIsdnMngrProcess.h"
#include "SystemFunctions.h"
#include "StringsMaps.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "RtmIsdnServiceList.h"
#include "RtmIsdnSpanMapList.h"
#include "SlotsNumberingConversionTableWrapper.h"


extern void RtmIsdnMngrManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CRtmIsdnMngrProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CRtmIsdnMngrProcess::GetManagerEntryPoint()
{
	return RtmIsdnMngrManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CRtmIsdnMngrProcess::CRtmIsdnMngrProcess()
{
	InitMediaBoardsIds();
	InitRtmIsdnBoardsIds();

	m_pServiceListOriginal		= new CRtmIsdnServiceList;
	m_pServiceListUpdated		= new CRtmIsdnServiceList;
	m_pSpanMapList				= new CRtmIsdnSpanMapList;
	m_pSlotsNumConversionTable	= new CSlotsNumberingConversionTableWrapper;
}

//////////////////////////////////////////////////////////////////////
CRtmIsdnMngrProcess::~CRtmIsdnMngrProcess()
{
	POBJDELETE(m_pServiceListOriginal);
	POBJDELETE(m_pServiceListUpdated);
	POBJDELETE(m_pSpanMapList);
	POBJDELETE(m_pSlotsNumConversionTable);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrProcess::AddExtraStringsToMap()
{
	CProcessBase::AddExtraStringsToMap();

	CStringsMaps::AddItem(NUM_PLAN_ENUM,	eNumPlanTypeUnknown,	"unknown_plan");
	CStringsMaps::AddItem(NUM_PLAN_ENUM,	eNumPlanTypeIsdn,		"isdn_plan");
	CStringsMaps::AddItem(NUM_PLAN_ENUM,	eNumPlanTypeNational,	"national_plan");
	CStringsMaps::AddItem(NUM_PLAN_ENUM,	eNumPlanTypePrivate,	"private_plan");
//	CStringsMaps::AddItem(NUM_PLAN_ENUM,	TELEPHONY_PLAN,			"telephony_plan");
//	CStringsMaps::AddItem(NUM_PLAN_ENUM,	DATA_PLAN,				"data_plan");
//	CStringsMaps::AddItem(NUM_PLAN_ENUM,	TELEX_PLAN,				"telex_plan");
	

	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	eDfltNumTypeUnknown,		"unknown");
	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	eDfltNumTypeInternational,	"international");
	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	eDfltNumTypeNational,		"national");
	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	eDfltNumTypeSubscriber,		"subscriber");
	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	eDfltNumTypeAbbreviated,	"abbreviated");
//	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	NETWORK_SPECIFIC,			"network_specific");
//	CStringsMaps::AddItem(DFLT_NUM_TYPE_ENUM,	NUM_TYPE_DEF,				"taken_from_service");


	CStringsMaps::AddItem(SPAN_TYPE_ENUM,	eSpanTypeT1,	"t1");
	CStringsMaps::AddItem(SPAN_TYPE_ENUM,	eSpanTypeE1,	"e1");
//	CStringsMaps::AddItem(SPAN_TYPE_ENUM,	SPAN_BRI,		"bri");
//	CStringsMaps::AddItem(SPAN_TYPE_ENUM,	SPAN_T1CAS,		"t1cas");


	CStringsMaps::AddItem(ISDN_SERVICE_TYPE_ENUM,	eServiceTypePri,		"pri");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_SWITCHED_56,	"switched_56");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_LEASED_LINES,	"leased_lines");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_LEASED_LINES_31,"leased_lines_31");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_H323,			"h323");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_H323_LAN,		"h323_lan");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_H323_IPOATM,	"h323_ipoatm");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_H323_LANEMU,	"h323_lanemu");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		SERVICE_T1_CAS,			"t1_cas");
//	CStringsMaps::AddItem(SERVICE_TYPE_ENUM,		UNKNOWN_SERVICE,		"unknown_service");


	CStringsMaps::AddItem(T1_FRAMING_ENUM,	eFramingType_T1_Esf,	"t1_esf");
	CStringsMaps::AddItem(T1_FRAMING_ENUM,	eFramingType_T1_Sf,		"t1_sf");
//	CStringsMaps::AddItem(T1_FRAMING_ENUM,	frmESF_ZBTSI,			"t1_esf_zbtsi");
//	CStringsMaps::AddItem(T1_FRAMING_ENUM,	frmSF_SLC96,			"t1_sf_slc96");


	CStringsMaps::AddItem(E1_FRAMING_ENUM,	eFramingType_E1_Crc4_SiFebe,	"e1_febe");
	CStringsMaps::AddItem(E1_FRAMING_ENUM,	eFramingType_E1_Basic_NoCrc,	"e1_basic");
//	CStringsMaps::AddItem(E1_FRAMING_ENUM,	frmCRC4,						"e1_crc4");
//	CStringsMaps::AddItem(E1_FRAMING_ENUM,	frmFEBE_CAS,					"e1_febe_cas");
//	CStringsMaps::AddItem(E1_FRAMING_ENUM,	frmCRC4_CAS,					"e1_crc4_cas");
//	CStringsMaps::AddItem(E1_FRAMING_ENUM,	frmBASIC_CAS,					"e1_basic_cas");
//	CStringsMaps::AddItem(E1_FRAMING_ENUM,	frmDISABLED,					"e1_disabled");


	CStringsMaps::AddItem(T1_LINE_CODING_ENUM,	eLineCodingType_T1_B8ZS,	"t1_b8zs");
	CStringsMaps::AddItem(T1_LINE_CODING_ENUM,	eLineCodingType_T1_B7ZS,	"t1_b7zs");
	CStringsMaps::AddItem(T1_LINE_CODING_ENUM,	eLineCodingType_AMI,		"t1_ami");


	CStringsMaps::AddItem(E1_LINE_CODING_ENUM,	eLineCodingType_E1_Hdb3,	"e1_hdb3");
	CStringsMaps::AddItem(E1_LINE_CODING_ENUM,	eLineCodingType_AMI,		"e1_ami");


	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_Att4ess,		"att_4ess");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_Att5ess10,	"att_5ess10");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_Dms100,		"dms_100");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_Dms250,		"dms_250");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_Ntt,			"ntt");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_NI1,			"ni_1");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_T1_NI2,			"ni_2");
	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	eSwitchType_E1_EuroIsdn,	"euro_isdn");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_ATT4_Q931,			"att4_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_ATT5_Q931,			"att5_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_NTI_Q931,			"nti_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_NTI250_Q931,			"nti250_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_ATT_NET_Q931,		"att_net_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_MD110_T1_Q931,		"md110_t1_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_MD110_E1_Q931,		"md110_e1_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_SIEMENS_Q931,		"siemens_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_NTT_Q931,			"ntt_q931");
//	CStringsMaps::AddItem(SWITCH_TYPE_ENUM,	SWITCH_NT5_Q931,			"nt5_q931");


	CStringsMaps::AddItem(SIDE_ENUM,	eSideTypeUser,			"user_side_l2");
	CStringsMaps::AddItem(SIDE_ENUM,	eSideTypeNetwork,		"net_side_l2");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgCUST_INTRFC,		"cust_intrfc");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgNET_INTRFC,			"net_intrfc");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgSYMMETIC_SIDE_L2,	"symmetic_side_l2");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgE1_30_BCHAN,		"e1_30_bchan");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgNFAS,				"nfas");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgBASIC_RATE,			"basic_rate");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgCCITT,				"ccitt");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgB_CHAN_NEGOT,		"b_chan_negot");
//	CStringsMaps::AddItem(SIDE_ENUM,	cnfgPROC_ON_EXCLSV,		"proc_on_exclsv");



	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNull,			"none");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeAttSdn,			"att_sdn");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNtiPrivate,		"nti_private");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeAttMegacom800,	"att_Megacom_800");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNtiInwats,		"nti_inwats");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeVal,			"val");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeAttMegacom,		"att_megacom");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNtiOutwats,		"nti_outwats");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeAttAccunet,		"att_accunet");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeAtt1800,		"att_1800");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeAttMultiquest,	"att_multiquest");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNtiTro,			"nti_tro");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNtiFx,			"nti_fx");
	CStringsMaps::AddItem(NET_SPEC_FACILITY_ENUM,	eNetSpecFacilityTypeNtiTieTrunk,	"nti_tie_trunk");

	
	CStringsMaps::AddItem(SPAN_ALARM_ENUM,	eSpanAlarmTypeNone,		"no_alarm");
	CStringsMaps::AddItem(SPAN_ALARM_ENUM,	eSpanAlarmTypeYellow,	"yellow_alarm");
	CStringsMaps::AddItem(SPAN_ALARM_ENUM,	eSpanAlarmTypeRed,		"red_alarm");

	CStringsMaps::AddItem(SPAN_D_CHANNEL_STATE_ENUM,	eDChannelStateTypeEstablished,		"d_channel_established");
	CStringsMaps::AddItem(SPAN_D_CHANNEL_STATE_ENUM,	eDChannelStateTypeNotEstablished,	"d_channel_not_established");

	CStringsMaps::AddItem(SPAN_CLOCKING_ENUM,	eClockingTypeNone,		"clocking_none");
	CStringsMaps::AddItem(SPAN_CLOCKING_ENUM,	eClockingTypePrimary,	"clocking_primary");
	CStringsMaps::AddItem(SPAN_CLOCKING_ENUM,	eClockingTypeBackup,	"clocking_backup");
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceList* CRtmIsdnMngrProcess::GetServiceListOriginal() const
{
	return m_pServiceListOriginal;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceList* CRtmIsdnMngrProcess::GetServiceListUpdated() const
{
	return m_pServiceListUpdated;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapList* CRtmIsdnMngrProcess::GetSpanMapList() const
{
	return m_pSpanMapList;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrProcess::InitMediaBoardsIds()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if ( IsTarget() )
	{
		if (eProductTypeRMX4000 == curProductType)
		{
			m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1;
			m_2ndMediaBoardId = FIXED_BOARD_ID_MEDIA_2;
			m_3rdMediaBoardId = FIXED_BOARD_ID_MEDIA_3;
			m_4thMediaBoardId = FIXED_BOARD_ID_MEDIA_4;
		}
		else
		{
			if(eProductTypeRMX1500 == curProductType)
			{
				   m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1;
				   m_2ndMediaBoardId = 0; // irrelevant on RMX1500
				   m_3rdMediaBoardId = 0; // irrelevant on RMX1500
				   m_4thMediaBoardId = 0; // irrelevant on RMX1500
			}
			else // eProductTypeRMX2000, eProductTypeNPG2000 etc
			{
			   m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1;
			   m_2ndMediaBoardId = FIXED_BOARD_ID_MEDIA_2;
			   m_3rdMediaBoardId = 0; // irrelevant on RMX2000
			   m_4thMediaBoardId = 0; // irrelevant on RMX2000
			}
		}
	}
	else
	{
		m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1_SIM;
		m_2ndMediaBoardId = FIXED_BOARD_ID_MEDIA_2_SIM;
		m_3rdMediaBoardId = FIXED_BOARD_ID_MEDIA_3_SIM;
		m_4thMediaBoardId = FIXED_BOARD_ID_MEDIA_4_SIM;		
	}


}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get1stMediaBoardId()
{
	return m_1stMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get2ndMediaBoardId()
{
	return m_2ndMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get3rdMediaBoardId()
{
	return m_3rdMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get4thMediaBoardId()
{
	return m_4thMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrProcess::InitRtmIsdnBoardsIds()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if ( IsTarget() )
	{
		if (eProductTypeRMX4000 == curProductType)
		{
			m_1stRtmIsdnBoardId = FIXED_BOARD_ID_RTM_1;
			m_2ndRtmIsdnBoardId = FIXED_BOARD_ID_RTM_2;
			m_3rdRtmIsdnBoardId = FIXED_BOARD_ID_RTM_3;
			m_4thRtmIsdnBoardId = FIXED_BOARD_ID_RTM_4;
		}
		else
		{
			if (eProductTypeRMX1500 == curProductType)
			{
				m_1stRtmIsdnBoardId = FIXED_BOARD_ID_RTM_1;
				m_2ndRtmIsdnBoardId = 0; // irrelevant on RMX1500
				m_3rdRtmIsdnBoardId = 0; // irrelevant on RMX1500
				m_4thRtmIsdnBoardId = 0; // irrelevant on RMX1500
			}
			else  // eProductTypeRMX2000, eProductTypeNPG2000 etc
			{
			  // MPM and RtmIsdn boards share the same boardId (and differ in subBoardId)
			  m_1stRtmIsdnBoardId = FIXED_BOARD_ID_MEDIA_1;
			  m_2ndRtmIsdnBoardId = FIXED_BOARD_ID_MEDIA_2;
			  m_3rdRtmIsdnBoardId = 0; // irrelevant on RMX2000
			  m_4thRtmIsdnBoardId = 0; // irrelevant on RMX2000
			}
		}
	}
	else
	{
		m_1stRtmIsdnBoardId = FIXED_BOARD_ID_RTM_1;
		m_2ndRtmIsdnBoardId = FIXED_BOARD_ID_RTM_2;
		m_3rdRtmIsdnBoardId = FIXED_BOARD_ID_RTM_3;
		m_4thRtmIsdnBoardId = FIXED_BOARD_ID_RTM_4;		
	}

}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get1stRtmIsdnBoardId()
{
	return m_1stRtmIsdnBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get2ndRtmIsdnBoardId()
{
	return m_2ndRtmIsdnBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get3rdRtmIsdnBoardId()
{
	return m_3rdRtmIsdnBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnMngrProcess::Get4thRtmIsdnBoardId()
{
	return m_4thRtmIsdnBoardId;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrProcess::SetSlotsNumberingConversionTable(const SLOTS_NUMBERING_CONVERSION_TABLE_S* pTable)
{
	m_pSlotsNumConversionTable->SetStruct(pTable);
}

/////////////////////////////////////////////////////////////////////////////
CSlotsNumberingConversionTableWrapper* CRtmIsdnMngrProcess::GetSlotsNumberingConversionTable()
{
	return m_pSlotsNumConversionTable;
}
