/*
 * LicensingFeatures.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: racohen
 */


#include "LicensingFeatures.h"

#include <stdio.h>
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "McuMngrStructs.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "Versions.h"
#include "PrettyTable.h"

// ------------------------------------------------------------
CLicensingFeatures::CLicensingFeatures (DWORD num,LicensedItem * ptr):m_numOfFeatures(num)
{

	for (DWORD i=0;i<m_numOfFeatures;i++)
	{

		m_featureList[i].capability_id = ptr[i].capability_id;
		m_featureList[i].capability    = ptr[i].capability;
		m_featureList[i].version       = ptr[i].version;
		m_featureList[i].req_count     = ptr[i].req_count;
		m_featureList[i].count         = ptr[i].count;
		m_featureList[i].hasChanged    = ptr[i].hasChanged;
		m_featureList[i].status        = ptr[i].status;
		m_featureList[i].reason        = ptr[i].reason;

		if( m_featureList[i].exp_date == NULL )
		{
			m_featureList[i].exp_date = new struct tm;
		}


	}
}

CLicensingFeatures::CLicensingFeatures ()
{

	m_numOfFeatures =0;
	memset(m_featureList,0,sizeof(LicensedItem)*MAX_NUM_OF_FEATURES);

}

// ------------------------------------------------------------
CLicensingFeatures::CLicensingFeatures(const CLicensingFeatures &other):
CSerializeObject(other)
{
	m_numOfFeatures      = other.m_numOfFeatures;

	for (DWORD i=0;i<m_numOfFeatures;i++)
		{

			m_featureList[i].capability_id = other.m_featureList[i].capability_id;
			m_featureList[i].capability    = other.m_featureList[i].capability;
			m_featureList[i].version       = other.m_featureList[i].version;
			m_featureList[i].req_count     = other.m_featureList[i].req_count;
			m_featureList[i].count         = other.m_featureList[i].count;
			m_featureList[i].hasChanged    = other.m_featureList[i].hasChanged;
			m_featureList[i].status        = other.m_featureList[i].status;
			m_featureList[i].reason        = other.m_featureList[i].reason;

			if( m_featureList[i].exp_date == NULL )
			{
				m_featureList[i].exp_date = new struct tm;
			}

			if( m_featureList[i].exp_date != NULL && other.m_featureList[i].exp_date !=NULL)
			  memcpy((char *)(m_featureList[i].exp_date),(char *) (other.m_featureList[i].exp_date),sizeof(struct tm));

		}
}


// ------------------------------------------------------------
CLicensingFeatures::~CLicensingFeatures ()
{
}


// ------------------------------------------------------------
void  CLicensingFeatures::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
	    << "CLicensingFeatures::Dump\n"
		<< "-------------\n";
	msg << "m_numOfFeatures: " << m_numOfFeatures;

    CPrettyTable<const char*, const char*, int, const char*>
        tbl("Feature", "Version", "Count", "Failed Reason");

	for (DWORD i=0;i<m_numOfFeatures ;i++)
	{
		tbl.Add(m_featureList[i].capability.c_str(), m_featureList[i].version.c_str(), m_featureList[i].count,
				(const char *)LicAcqStatusReasonStr[m_featureList[i].reason]);
	}
	msg << tbl.Get();
}


// ------------------------------------------------------------
void CLicensingFeatures::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	TRACESTR(eLevelInfoNormal) << "inside CLicensingFeatures::SerializeXml";

	CXMLDOMElement* pLicensingFeatureNode = NULL;
	CXMLDOMElement* pFeatureNode = NULL;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("LICENSE_FEATURES_DATA");
		pLicensingFeatureNode = pFatherNode;
	}
	else
	{
		pLicensingFeatureNode = pFatherNode->AddChildNode("LICENSE_FEATURES_DATA");
	}


	for (DWORD i=0;i<m_numOfFeatures;i++)
			{
				pFeatureNode = pLicensingFeatureNode->AddChildNode("LICENSE_FEATURE");

				pFeatureNode->AddChildNode( "CAPABILITY_ID", m_featureList[i].capability_id ,LICENSE_FEATURE_ENUM);
				pFeatureNode->AddChildNode( "CAPABILITY"   , m_featureList[i].capability.c_str() );
				pFeatureNode->AddChildNode( "VERSION"      , m_featureList[i].version.c_str() );
				pFeatureNode->AddChildNode( "REQ_COUNT"    , m_featureList[i].req_count );
				pFeatureNode->AddChildNode( "COUNT"        , m_featureList[i].count );
				//pFeatureNode->AddChildNode( "HAS_CHANGED"       , m_featureList[i].hasChanged );
				pFeatureNode->AddChildNode( "STATUS"       , m_featureList[i].status ,LICENSE_FEATURE_STATUS_ENUM);
				pFeatureNode->AddChildNode( "REASON"     , m_featureList[i].reason ,LICENSE_STATUS_REASON_ENUM);

				pFeatureNode->AddChildNode( "EXPIRATION_DATE" , m_expirationDate);

			}


}

// ------------------------------------------------------------
int	 CLicensingFeatures::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char *action)
{
	TRACESTR(eLevelInfoNormal) << "inside CLicensingFeatures::DeSerializeXml";

	int nStatus = STATUS_OK;


	CXMLDOMElement *pLicensingFeatureNode=NULL ,*pTempNode=NULL;

	char* ParentNodeName;

	pActionNode->get_nodeName(&ParentNodeName);

	if( !strcmp(ParentNodeName, "LICENSE_FEATURES_DATA") )
	{
		pLicensingFeatureNode = pActionNode;
	}
	else
	{
		GET_MANDATORY_CHILD_NODE(pActionNode, "LICENSE_FEATURES_DATA", pLicensingFeatureNode);
	}

	if (pLicensingFeatureNode)
	{
		TRACESTR(eLevelInfoNormal) << "inside CLicensingFeatures::DeSerializeXml pLicensingFeatureNode";

		GET_FIRST_CHILD_NODE(pLicensingFeatureNode,"LICENSE_FEATURE",pTempNode);

		m_numOfFeatures = 0;
		while (pTempNode && m_numOfFeatures < MAX_NUM_OF_FEATURES)
		{
			TRACESTR(eLevelInfoNormal) << "inside CLicensingFeatures::DeSerializeXml pTempNode";

			DWORD tmpCapabilityId=0;
			GET_VALIDATE_CHILD(pTempNode,"CAPABILITY_ID",&tmpCapabilityId,LICENSE_FEATURE_ENUM);

			PASSERT_AND_RETURN_VALUE(tmpCapabilityId >= MAX_NUM_OF_FEATURES, STATUS_FAIL);

			m_featureList[tmpCapabilityId].capability_id = (E_FLEXERA_LICENSE_FEATURES)tmpCapabilityId;

			TRACESTR(eLevelInfoNormal) << "inside CLicensingFeatures::DeSerializeXml  CAPABILITY_ID " << tmpCapabilityId << " m_numOfFeatures " << m_numOfFeatures;

			char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

			GET_VALIDATE_CHILD(pTempNode,"CAPABILITY",tmpData , ONE_LINE_BUFFER_LENGTH);
			m_featureList[tmpCapabilityId].capability=tmpData;

			memset(tmpData        , '\0', sizeof(tmpData        ));
			GET_VALIDATE_CHILD(pTempNode,"VERSION",tmpData , ONE_LINE_BUFFER_LENGTH);
			m_featureList[tmpCapabilityId].version = tmpData;

			DWORD tmpReqCount=0;
			GET_VALIDATE_CHILD(pTempNode,"REQ_COUNT",&tmpReqCount,_0_TO_WORD);
			m_featureList[tmpCapabilityId].req_count = tmpReqCount;

			if (tmpReqCount == 0) //only on COUNTED capability update the count . on NOTCOUNTED we fill it in code .
			{
				DWORD tmpCount=0;
				GET_VALIDATE_CHILD(pTempNode,"COUNT",&tmpCount,_0_TO_WORD);
				m_featureList[tmpCapabilityId].count = tmpCount;
			}

			//WORD tmp;
			//GET_VALIDATE_CHILD(pTempNode,"HAS_CHANGED",&tmp,_0_TO_WORD);
			//m_featureList[tmpCapabilityId].hasChanged = (bool)tmp;

			DWORD status=0;
			GET_VALIDATE_CHILD(pTempNode,"STATUS",&status,LICENSE_FEATURE_STATUS_ENUM);

			TRACESTR(eLevelInfoNormal) << "inside CLicensingFeatures::DeSerializeXml  STATUS " << status << " m_numOfFeatures " << m_numOfFeatures;
			m_featureList[tmpCapabilityId].status = (LicAcqStatus)status;

			DWORD reason=0;
			GET_VALIDATE_CHILD(pTempNode,"REASON",&reason,LICENSE_STATUS_REASON_ENUM);
			m_featureList[tmpCapabilityId].reason = (LicAcqStatusReason)reason;

			GET_VALIDATE_CHILD(pTempNode,"EXPIRATION_DATE",&m_expirationDate,DATE_TIME);


			if (m_featureList[tmpCapabilityId].exp_date != NULL)
			{

				m_featureList[tmpCapabilityId].exp_date->tm_mday = m_expirationDate.m_day;
				m_featureList[tmpCapabilityId].exp_date->tm_mon  = m_expirationDate.m_mon;
				m_featureList[tmpCapabilityId].exp_date->tm_year = m_expirationDate.m_year;
				m_featureList[tmpCapabilityId].exp_date->tm_min  = m_expirationDate.m_min;
				m_featureList[tmpCapabilityId].exp_date->tm_hour = m_expirationDate.m_hour;
			}
			else
				PASSERTMSG(m_featureList[tmpCapabilityId].exp_date == NULL, "exp_date is NULL");



			m_numOfFeatures++;
			GET_NEXT_CHILD_NODE(pLicensingFeatureNode,"LICENSE_FEATURE",pTempNode);

		}
		PASSERTMSG(pTempNode && m_numOfFeatures > MAX_NUM_OF_FEATURES, "feature list includes too many features");
	}

	return nStatus;
}






// ------------------------------------------------------------
CLicensingFeatures& CLicensingFeatures::operator = (const CLicensingFeatures &rOther)
{
	m_numOfFeatures      = rOther.m_numOfFeatures;

	for (DWORD i=0;i<m_numOfFeatures;i++)
		{

			m_featureList[i].capability_id = rOther.m_featureList[i].capability_id;
			m_featureList[i].capability    = rOther.m_featureList[i].capability;
			m_featureList[i].version       = rOther.m_featureList[i].version;
			m_featureList[i].req_count     = rOther.m_featureList[i].req_count;
			m_featureList[i].count         = rOther.m_featureList[i].count;
			m_featureList[i].hasChanged    = rOther.m_featureList[i].hasChanged;
			m_featureList[i].status        = rOther.m_featureList[i].status;
			m_featureList[i].reason        = rOther.m_featureList[i].reason;

			if (m_featureList[i].exp_date != NULL && rOther.m_featureList[i].exp_date != NULL)
			memcpy((char *) m_featureList[i].exp_date,(char *) rOther.m_featureList[i].exp_date,sizeof(struct tm));

		}
    return *this;
}



