

#include "ServiceConfigList.h"
#include "SysConfigBase.h"
#include "TraceStream.h"
#include "Segment.h"
#include "InternalProcessStatuses.h"



CServiceConfigList::CServiceConfigList()
{
	m_numb_of_serv		= 0;
	for( int i=0 ; i < MAX_SERV_PROVIDERS_IN_LIST; i++ )
    {
		m_pServiceConfig[i] = NULL;
    }
}

///////////////////////////////////////

CServiceConfigList::~CServiceConfigList()
{
	for (int i=0; i< m_numb_of_serv; i++)
	{
		PDELETE(m_pServiceConfig[i]);
	}
}

//////////////////////////////////////////////

CServiceConfigList::CServiceConfigList(const CServiceConfigList& other)
:CPObject(other)
{
	m_numb_of_serv=other.m_numb_of_serv;
	for( int i=0 ; i < MAX_SERV_PROVIDERS_IN_LIST; i++ )
	{
		if( other.m_pServiceConfig[i] == NULL)
			m_pServiceConfig[i] = NULL;
		else
			m_pServiceConfig[i]= new CServiceConfig( *other.m_pServiceConfig[i] );
	}

}

////////////////////////////////////////////////

CServiceConfigList& CServiceConfigList:: operator=( const CServiceConfigList& other)
{
	m_numb_of_serv=other.m_numb_of_serv;
	for( int i=0 ; i < MAX_SERV_PROVIDERS_IN_LIST; i++ )
	{
		if( other.m_pServiceConfig[i] == NULL)
			m_pServiceConfig[i] = NULL;
		else
			m_pServiceConfig[i]= new CServiceConfig( *other.m_pServiceConfig[i] );
	}
	return *this;

}

/////////////////////////////////////////////////////////////////////////////

void CServiceConfigList::Serialize(WORD format,CSegment *pSeg)
{
	*pSeg << m_numb_of_serv;
	for (int i=0; i< m_numb_of_serv; i++)
	{
		//TRACESTR(eLevelInfoNormal) << "CCServiceConfigList::Serialize - param name" << i;
		//std::cerr << "ServiceConfigList::Serialize: " << i << std::endl;
		m_pServiceConfig[i]->Serialize(format, pSeg);

	}

}


////////////////////////////////////////////////////////

void CServiceConfigList::DeSerialize(CSegment *pSeg)
{
	*pSeg >> m_numb_of_serv;
//	FTRACESTR(eLevelInfoNormal) << "CCServiceConfigList::m_numb_of_serv :" << m_numb_of_serv;
	for (int i=0; i< m_numb_of_serv; i++)
	{
		m_pServiceConfig[i] = new CServiceConfig();
		m_pServiceConfig[i]->DeSerialize(pSeg);
		
//		FTRACESTR(eLevelInfoNormal) << "CCServiceConfigList::DeSerialize - service id :" << m_pServiceConfig[i]->GetId();

		CSysMap *m_Map=m_pServiceConfig[i]->GetMap();
		CSysMap::iterator iTer = m_Map->begin();
		CSysMap::iterator iEnd = m_Map->end();
		while(iEnd != iTer)
		{
			const CCfgData *cfgData = iTer->second;
			//FTRACESTR(eLevelInfoNormal) << "CCServiceConfigList::DeSerialize - param name" << cfgData->GetData();
			iTer++;
		}
	}

}


///////////////////////////////////////////////////////////
//int CServiceConfigList::FindService(const CIPService& other)const
//{
////    for( int i=0; i<(int)m_numb_of_serv; i++ )
////    {
////        if ( m_pServiceConfig[i] != NULL )
////            if ( !strcmp( m_pServiceConfig[i]->GetName(), other.GetName() ) )
////                return i;
////    }
////    return NOT_FIND;
//	return 1;
//}

//////////////////////////////////////////////////////
BOOL CServiceConfigList::GetStrDataByKey(DWORD service_id, const std::string &key, std::string &data)const
{
	for (int i=0; i< m_numb_of_serv; i++)
	{
		if (m_pServiceConfig[i]->GetId()==service_id)
			return m_pServiceConfig[i]->GetStrDataByKey(key,data);
	}
	return FALSE;
}
/////////////////////////////////////////////////

//////////////////////////////////////////////////////
BOOL CServiceConfigList::GetDWORDDataByKey(DWORD service_id, const std::string &key, DWORD &data)const
{
	for (int i=0; i< m_numb_of_serv; i++)
	{
		//FTRACESTR(eLevelInfoNormal) << "CServiceConfigList::GetDWORDDataByKey" << m_pServiceConfig[i]->GetId();
		if (m_pServiceConfig[i]->GetId()==service_id)
			return m_pServiceConfig[i]->GetDWORDDataByKey(key,data);
	}
	return FALSE;
}

//////////////////////////////////////////////////////
BOOL CServiceConfigList::GetHexDataByKey(DWORD service_id, const std::string &key, DWORD &data)const
{
	for (int i=0; i< m_numb_of_serv; i++)
	{
		if (m_pServiceConfig[i]->GetId()==service_id)
			return m_pServiceConfig[i]->GetHexDataByKey(key,data);
	}
	return FALSE;
}

//////////////////////////////////////////////////////
BOOL CServiceConfigList::GetBOOLDataByKey(DWORD service_id, const std::string &key, BOOL &data)const
{
	for (int i=0; i< m_numb_of_serv; i++)
	{
		if (m_pServiceConfig[i]->GetId()==service_id)
			return m_pServiceConfig[i]->GetBOOLDataByKey(key,data);
	}
	return FALSE;
}
/////////////////////////////////////////////////////////

int CServiceConfigList::Add(CServiceConfig *pServiceConfig)
{

	if(m_numb_of_serv >= MAX_SERV_PROVIDERS_IN_LIST )
	{
		return STATUS_NUMBER_OF_H323_SERVICES_EXCEEDED;
	}

//	if(FindService( *pNewService ) != NOT_FIND )
//	{
//		return STATUS_H323_SERVICE_NAME_EXISTS;
//	}
	CServiceConfig *pNewServiceConfig = new CServiceConfig();
	pNewServiceConfig->FillMap(pServiceConfig);
	pNewServiceConfig->SetId(pServiceConfig->GetId());
	m_pServiceConfig[m_numb_of_serv] = pNewServiceConfig;
	m_numb_of_serv++;

//	FTRACESTR(eLevelInfoNormal) << "CCServiceConfigList::Add:m_numb_of_serv = " << m_numb_of_serv <<";serice id:"<< pServiceConfig->GetId();


	return 1;
}
//////////////////////////////////////////////////////
BOOL CServiceConfigList::IsServiceExists(DWORD service_id)const
{
	for (int i=0; i< m_numb_of_serv; i++)
	{
		if (m_pServiceConfig[i]->GetId()==service_id)
			return TRUE;
	}
	return FALSE;
}
//////////////////////////////////////////////////////
void CServiceConfigList::PrintServiceConfigList()
{
	FTRACESTR(eLevelInfoNormal) << "CCServiceConfigList::PrintServiceConfigList";
	for (int i=0; i< m_numb_of_serv; i++)
		m_pServiceConfig[i]->PrintServiceConfig();
}
//////////////////////////////////////////////////////
