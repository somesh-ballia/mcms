#include <sstream>
#include "RelayIntraDB.h"
#include "RelayIntraData.h"
#include "VideoBridge.h"
#include "VideoBridgePartyCntl.h"
#include "VideoRelayBridgePartyCntl.h"
#include "StlUtils.h"

//////////////////////////////////////////////////////////////////////////
CRelayIntraDB::CRelayIntraDB()
{
	m_pVideoBridge = NULL;
}

//////////////////////////////////////////////////////////////////////////
CRelayIntraDB::~CRelayIntraDB()
{

}

//////////////////////////////////////////////////////////////////////////
void CRelayIntraDB::AddParty(DWORD iPartyID, const std::list<unsigned int>& listSrc)
{
	m_mapPartyIntra[iPartyID].clear();
	std::list<unsigned int>::const_iterator it = listSrc.begin();
	for ( ; it != listSrc.end(); ++it)
	{
		INTRA_SRC st(*it);
		m_mapPartyIntra[iPartyID].push_back(st);
	}
	FTRACEINTO << "\nPartyRsrcID = " << iPartyID << "\nList SSRC : " << CStlUtils::ContainerToString(listSrc).c_str();
}

//////////////////////////////////////////////////////////////////////////
void CRelayIntraDB::AddPartyNeedIntra(DWORD iPartyID, const std::list<unsigned int>& listSrc, bool bIsGDR)
{
	std::list<unsigned int>::const_iterator it = listSrc.begin();
	for ( ; it != listSrc.end(); ++it)
	{
		INTRA_SRC st(*it, true, bIsGDR);
		m_mapPartyIntra[iPartyID].push_back(st);
	}
}

//////////////////////////////////////////////////////////////////////////
void CRelayIntraDB::RemoveParty(DWORD iPartyID)
{
	m_mapPartyIntra.erase(iPartyID);
	FTRACEINTO << "\nPartyRsrcID = " << iPartyID;
}

//////////////////////////////////////////////////////////////////////////
bool CRelayIntraDB::SetNeedIntra(DWORD iPartyID)
{
	std::map<int, std::list<INTRA_SRC> >::iterator itMap = m_mapPartyIntra.find(iPartyID);
	if (itMap != m_mapPartyIntra.end())
	{
		std::list<INTRA_SRC>::iterator itList = itMap->second.begin();
		for ( ; itList != itMap->second.end(); ++itList)
			itList->m_bNeedIntra = true;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CRelayIntraDB::SetNeedIntra(DWORD iPartyID, unsigned int iSrc)
{
	std::map<int, std::list<INTRA_SRC> >::iterator itMap = m_mapPartyIntra.find(iPartyID);
	if (itMap != m_mapPartyIntra.end())
	{
		std::list<INTRA_SRC>::iterator itList = itMap->second.begin();
		for ( ; itList != itMap->second.end(); ++itList)
		{
			if (itList->m_iSrc == iSrc)
			{
				itList->m_bNeedIntra = true;
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CRelayIntraDB::SetNeedIntra(const AskForRelayIntra& req)
{
	RelayIntraParams::const_iterator itParam = req.m_relayIntraParameters.begin();
	for (; itParam != req.m_relayIntraParameters.end(); ++itParam)
	{
		bool isGdr = itParam->m_bIsGdr;
		DWORD iPartyID = itParam->m_partyRsrcId;
		std::map<int, std::list<INTRA_SRC> >::iterator itMap = m_mapPartyIntra.find(iPartyID);
		if (itMap == m_mapPartyIntra.end())
		{
			AddPartyNeedIntra(iPartyID, itParam->m_listSsrc, isGdr);
		}
		else
		{
			std::list<unsigned int>::const_iterator itSrc = itParam->m_listSsrc.begin();
			for ( ; itSrc != itParam->m_listSsrc.end(); ++itSrc)
			{
				bool foundSsrc = false;

				std::list<INTRA_SRC>::iterator itList = itMap->second.begin();
				for ( ; itList != itMap->second.end(); ++itList)
				{
					if (itList->m_iSrc == *itSrc)
					{
						if(itList->m_bNeedIntra==true && itList->m_bGDR==false && isGdr==true)
						{
							//if we prev marked it as we need IDR and it wasn't flashed yet, we will ask for IDR (it is more stricked)
							FTRACEINTO << "RelayIntraDB::SetNeedIntra altough this req is with GDR prev not flashed was with IDR partyId: " << iPartyID << " ,SSRC: "<< (*itSrc);

						}
						else
							itList->m_bGDR=isGdr;
						itList->m_bNeedIntra = true;
						foundSsrc = true;
						break;
					}
				}
				if(!foundSsrc)
				{
					FTRACEINTO << "RelayIntraDB::SetNeedIntra Can't find partyId: " << iPartyID << " ,SSRC: "<< (*itSrc);
					FPASSERT(*itSrc);

				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRelayIntraDB::Flush()
{
	std::ostringstream msg;
	std::list<unsigned int> listSsrcToSend;
	std::map<int, std::list<INTRA_SRC> >::iterator itMap = m_mapPartyIntra.begin();
	for ( ; itMap != m_mapPartyIntra.end(); ++itMap)
	{
		CVideoBridgePartyCntl* pCtrl = NULL;
		listSsrcToSend.clear();
		std::list<INTRA_SRC>::iterator itList = itMap->second.begin();
		msg << "\nPartyRsrcId=" << itMap->first;

		bool isPartyHasGDR = false;
		bool isPartyHasIDR = false;

		for ( ; itList != itMap->second.end(); ++itList)
		{

			if (itList->m_bNeedIntra)
			{
				if (NULL == pCtrl)
					pCtrl = (CVideoBridgePartyCntl*)(m_pVideoBridge->GetPartyCntl(itMap->first));

				msg << ", SSRC: "<< itList->m_iSrc;
				if(itList->m_bGDR)
				{
					msg << ", GDR = true";
					isPartyHasGDR = true;
				}
				else
				{
					isPartyHasIDR = true;
				}
				listSsrcToSend.push_back(itList->m_iSrc);

				itList->m_bNeedIntra = false;
				itList->m_bGDR = false;
			}
		}
		if ( ! listSsrcToSend.empty() )
		{
			if (pCtrl)
			{
				if(pCtrl->IsVideoRelayParty())
				{
					msg << ", listSsrcToSend size="<< listSsrcToSend.size();
					bool isReqGDR = false;
					if (isPartyHasGDR && !isPartyHasIDR) //If one of the ssrc is with IDR we will ask for IDR
						isReqGDR = true;
					((CVideoRelayBridgePartyCntl*)pCtrl)->SendIntraRequestToParty (listSsrcToSend, isReqGDR);
				}
				else
				{
					msg << ", NOT Relay party";
					if (pCtrl->IsTranslatorAvcSvcExists())
					{
						msg << " in MIX-mode => send request from AvcToSvcTranslator";
						pCtrl->SendRelayIntraRequestToAvcToSvcTranslator (listSsrcToSend, isPartyHasGDR);
					}
				}
}
			else
			{
				FTRACESTR(eLevelError) << "RelayIntraDB::Flush pCtrl is NULL\n" << msg.str().c_str();
				continue;
			}
		}
	}
	FTRACEINTO << msg.str().c_str();
}



//////////////////////////////////////////////////////////////////////////
void CRelayIntraDB::Dump(std::ostream& ost) const
{
	std::stringstream sout;
	std::map<int, std::list<INTRA_SRC> >::const_iterator itMap = m_mapPartyIntra.begin();
	for ( ; itMap != m_mapPartyIntra.end(); ++itMap)
	{
		sout << "\n\tPartyID = " << itMap->first;
		std::list<INTRA_SRC>::const_iterator itList = itMap->second.begin();
		for ( ; itList != itMap->second.end(); ++itList)
			sout << "\n\t\tSSRC = " << itList->m_iSrc << "\t\tNeedIntra = " << (itList->m_bNeedIntra ? "true" : "false");
	}

	ost << sout.str();
}
