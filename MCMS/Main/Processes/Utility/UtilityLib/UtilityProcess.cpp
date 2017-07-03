// UtilityProcess.cpp: implementation of the CUtilityProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "UtilityProcess.h"
#include "SystemFunctions.h"

extern void UtilityManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CUtilityProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CUtilityProcess::GetManagerEntryPoint()
{
	return UtilityManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CUtilityProcess::CUtilityProcess()
{
	m_pTcpDumpEntityList					= new CTcpDumpEntityList;
	m_pTcpDumpStartList					    = new CTcpDumpEntityList;
	m_pTcpDumpStatus					    = new CTcpDumpStatus;

	m_isTcpDumpRunning  = false ;
	memset(&m_time_elapsed , 0 , sizeof (CStructTm));
    m_isUiUpdateNeeded = false;
}

/////////////////////////////////////////////////////////////////////////////
bool CUtilityProcess::GetIsTcpDumpRunning ()
{
	return m_isTcpDumpRunning;
}

/////////////////////////////////////////////////////////////////////////////
void CUtilityProcess::SetIsTcpDumpRunning (bool mode)
{
	m_isTcpDumpRunning= mode;
}

void    CUtilityProcess::SetTimeElapsed(CStructTm time)
{
	m_time_elapsed = time;
}
CStructTm    CUtilityProcess::GetTimeElapsed()
{
	return m_time_elapsed;
}




//////////////////////////////////////////////////////////////////////
CUtilityProcess::~CUtilityProcess()
{
	POBJDELETE(m_pTcpDumpEntityList);
	POBJDELETE(m_pTcpDumpStartList);
	POBJDELETE(m_pTcpDumpStatus);
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityList* CUtilityProcess::GetTcpDumpEntityList()
{
	return m_pTcpDumpEntityList;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityList* CUtilityProcess::GetTcpDumpStartList()
{
	return m_pTcpDumpStartList;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpStatus* CUtilityProcess::GetTcpDumpStatus()
{
	return m_pTcpDumpStatus;
}


/////////////////////////////////////////////////////////////////////////////
void CUtilityProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddItem(MAX_CAPTURE_SIZE_ENUM,e_MaxCaptureSize_none,"none");
	CStringsMaps::AddItem(MAX_CAPTURE_SIZE_ENUM,e_MaxCaptureSize_0_5_gb,"0.5_gb");
	CStringsMaps::AddItem(MAX_CAPTURE_SIZE_ENUM,e_MaxCaptureSize_1_gb,"1_gb");
	CStringsMaps::AddItem(MAX_CAPTURE_SIZE_ENUM,e_MaxCaptureSize_1_5_gb,"1.5_gb");
	CStringsMaps::AddItem(MAX_CAPTURE_SIZE_ENUM,e_MaxCaptureSize_2_5_gb,"2.5_gb");


/*	MaxCaptureSizeType
	<xsd:enumeration value="none"/>
      <xsd:enumeration value="0.5_gb"/>
      <xsd:enumeration value="1_gb"/>
	  <xsd:enumeration value="1.5_gb"/>
	  <xsd:enumeration value="2.5_gb"/>*/

	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_none,"none");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_15_sec,"15_sec");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_30_sec,"30_sec");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_1_min,"1_min");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_2_min,"2_min");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_3_min,"3_min");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_4_min,"4_min");
	CStringsMaps::AddItem(MAX_CAPTURE_DURATION_ENUM,e_MaxCaptureDuration_5_min,"5_min");

/*	  <xsd:simpleType name="MaxCaptureDurationType">
	    <xsd:restriction base="xsd:string">
		  <xsd:enumeration value="none"/>
	      <xsd:enumeration value="15_sec"/>
	      <xsd:enumeration value="30_sec"/>
		  <xsd:enumeration value="1_min"/>
		  <xsd:enumeration value="2_min"/>
		  <xsd:enumeration value="3_min"/>
		  <xsd:enumeration value="4_min"/>
		  <xsd:enumeration value="5_min"/>
	    </xsd:restriction>
	  </xsd:simpleType> */


/*	  <xsd:simpleType name="TcpDumpStateType">
	    <xsd:restriction base="xsd:string">
	      <xsd:enumeration value="running"/>
	      <xsd:enumeration value="idle"/>
	      <xsd:enumeration value="failed"/>
	    </xsd:restriction>
	  </xsd:simpleType>*/

		CStringsMaps::AddItem(ENTITY_TYPE_ENUM, e_EntityType_Management, "management");
		CStringsMaps::AddItem(ENTITY_TYPE_ENUM, e_EntityType_Central_Signaling, "central_signaling");
		CStringsMaps::AddItem(ENTITY_TYPE_ENUM, e_EntityType_Media_Card, "media_card");

		/*
			   <xsd:simpleType name="TcpDumpEntityType">
			    <xsd:restriction base="xsd:string">
			      <xsd:enumeration value="management"/>
			      <xsd:enumeration value="central_signaling"/>
				  <xsd:enumeration value="media_card"/>
			    </xsd:restriction>
			  </xsd:simpleType>
			  */


		CStringsMaps::AddItem(TCP_DUMP_STATE_ENUM, e_TcpDumpState_Idle, "idle");
		CStringsMaps::AddItem(TCP_DUMP_STATE_ENUM, e_TcpDumpState_Success, "success");
		CStringsMaps::AddItem(TCP_DUMP_STATE_ENUM, e_TcpDumpState_Running, "running");
		CStringsMaps::AddItem(TCP_DUMP_STATE_ENUM, e_TcpDumpState_Failed, "failed");
		/* <xsd:simpleType name="TcpDumpStateType">
		    <xsd:restriction base="xsd:string">
				<xsd:enumeration value="idle"/>
				<xsd:enumeration value="success"/>
		        <xsd:enumeration value="running"/>
				<xsd:enumeration value="failed"/>
		    </xsd:restriction>*/



}

bool CUtilityProcess::GetIsUiUpdateNeeded()
{
	return m_isUiUpdateNeeded;
}

/////////////////////////////////////////////////////////////////////////////
void CUtilityProcess::SetIsUiUpdateNeeded(bool bNeeded)
{
	m_isUiUpdateNeeded = bNeeded;
}

