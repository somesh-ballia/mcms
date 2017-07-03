//+========================================================================+
//                     BridgePartyList.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ReceptionList.cpp	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | July-2005  |                                                      |
//+========================================================================+

#include "ReceptionList.h"
#include "IsdnNetSetup.h"
#include "TraceStream.h"
#include "ReceptionSip.h"

/////////////////////////////////////////////////////////////////////////////
CReceptionList::CReceptionList()
{
}

/////////////////////////////////////////////////////////////////////////////
CReceptionList::~CReceptionList()
{
}

/////////////////////////////////////////////////////////////////////////////
CReceptionList::CReceptionList (const CReceptionList& rReceptionList)
:CPObject(rReceptionList),
    m_receptionList(rReceptionList.m_receptionList)
{
}

/////////////////////////////////////////////////////////////////////////////
CReceptionList&	CReceptionList::operator= (const CReceptionList& rOtherReceptionList)
{
	if (&rOtherReceptionList == this ) 
		return *this;

	m_receptionList = rOtherReceptionList.m_receptionList;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CReception*  CReceptionList::Find(CReception*  pReception)
{

	RECEPTION_LIST::iterator itr =  FindPosition(pReception);

	if ( itr != m_receptionList.end() ) {
		return (*itr);
	}
	else {
		return NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////
CReception*  CReceptionList::Remove(CReception*  pReception)
{
	//PTRACE(eLevelInfoNormal,"CReceptionList::Remove : ");

	if ( NULL == pReception )
	{
		return pReception;
	}

//	TRACEINTO << "CReceptionList::Remove :pReception->PartyID = " << pReception->GetPartyId();
//
//	TRACEINTO << "CReceptionList::Remove :pReception = " <<  (DWORD)pReception;


	//PrintReceptionList();
	CReception*  pErasedReception = NULL;
	
	RECEPTION_LIST::iterator itr = FindPosition(pReception);
	
	if ( itr != m_receptionList.end() ) {
		pErasedReception = (*itr);
		m_receptionList.erase(itr);
	}
	
	//printing for debug only BEGIN	
//	if (pErasedReception)
//	{
//		DWORD partyId = pErasedReception->GetPartyId();
//		char* confName =(char *)pErasedReception->GetTargetConfName();
//		
//		ALLOCBUFFER(Mess1,IP_STRING_LEN);
//		sprintf(Mess1," RemovedFromList : partyId %04u , TargetConfName:  <%s>  ",partyId, confName );
//			
//		PTRACE2(eLevelInfoNormal," ---> ", Mess1);
//		DEALLOCBUFFER(Mess1);
//	}
	//END
	//PTRACE(eLevelInfoNormal," CReceptionList::Remove YOELLA");	
    return pErasedReception;
	
}

/////////////////////////////////////////////////////////////////////////////
EStat  CReceptionList::Insert(CReception* pReception)
{
//	PTRACE(eLevelInfoNormal,"CReceptionList::Insert : ");
//
//	TRACEINTO << "CReceptionList::Insert :pReception->PartyID = " << pReception->GetPartyId();
//
//	TRACEINTO << "CReceptionList::Insert :pReception = " <<  (DWORD)pReception;

	//PrintReceptionList();
	if ( NULL == pReception )
	{
		return statInconsistent;
	}

	m_receptionList.push_back(pReception);
	
	
	//printing for debug only BEGIN	
//	if (pReception)
//	{
//		DWORD partyId = pReception->GetPartyId();
//		char* confName =(char *)pReception->GetTargetConfName();
//		
//		ALLOCBUFFER(Mess1,IP_STRING_LEN);
//		sprintf(Mess1," ADDList : partyId %04u , TargetConfName:  <%s>  ",partyId, confName );
//			
//		PTRACE2(eLevelInfoNormal," ---> ", Mess1);
//		DEALLOCBUFFER(Mess1);
//	}
	//END
    //PTRACE(eLevelInfoNormal," CReceptionList::Insert YOELLA");
	return statOK;
}
/////////////////////////////////////////////////////////////////////////////
EStat  CReceptionList::Update(CReception*  pNewReception)
{
	EStat status = statOK;
	if ( NULL != pNewReception )
	{

//		TRACEINTO << "CReceptionList::Update :pReception->PartyID = " << pNewReception->GetPartyId();
//
//		TRACEINTO << "CReceptionList::Update :pReception = " <<  (DWORD)pNewReception;
//
		CReception*  pErasedReception = Remove(pNewReception); //only by number 
		status = Insert(pNewReception);
		
		//printing for debug only BEGIN	
//		DWORD partyId = pNewReception->GetPartyId();
//		char* confName =(char *)pNewReception->GetTargetConfName();
//		
//		ALLOCBUFFER(Mess1,IP_STRING_LEN);
//		sprintf(Mess1," UpdateList : partyId %04u , TargetConfName:  <%s>  ",partyId, confName );
//			
//		PTRACE2(eLevelInfoNormal," ---> ", Mess1);
//		DEALLOCBUFFER(Mess1);
		//END	

		//TRACEINTO << "CReceptionList::Update finished";
	}
	else
	  status = statIllegal;
	
	return status;

}

///////////////////////////////////////////////////////////////////////////////
DWORD CReceptionList::Size()
{
	return m_receptionList.size();
}

///////////////////////////////////////////////////////////////////////////////
CReception* CReceptionList::At(DWORD index)
{
	if( index < m_receptionList.size() )
		return (m_receptionList[index]);
	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
RECEPTION_LIST::iterator CReceptionList::FindPosition(const CReception* pReception)
{
	RECEPTION_LIST::iterator itr =  m_receptionList.begin();

	while (itr != m_receptionList.end())
	{
		if ( *(*itr) == *pReception ) 
		{
			return itr;
		}
		itr++;
	}

	return itr;
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionList::ClearAndDestroy(void)
{
	CReception*  pErasedReception = NULL;
	
	RECEPTION_LIST::iterator itr =  m_receptionList.begin();

	while (itr != m_receptionList.end())
	{
		pErasedReception = (*itr);
		m_receptionList.erase(itr);
		
		POBJDELETE(pErasedReception);
		itr =  m_receptionList.begin();
	}
}

///////////////////////////////////////////////////////////////////////////////
CReception* CReceptionList::FindSameMRReceptionConfOnAirNO(char* confTargetName)
{
	RECEPTION_LIST::iterator itr =  m_receptionList.begin();

	while (itr != m_receptionList.end())
	{
		if  ( !(strcmp(confTargetName, (*itr)->GetTargetConfName()) ) && (*itr)->GetConfOnAirFlag()==NO)
		{
			return (*itr);
		}
		itr++;
	}

	return NULL;

}

///////////////////////////////////////////////////////////////////////////////
bool CReceptionList::MarkVoiceReceptionAsDisconnected(DWORD spanId, DWORD physicalPort)
{
  RECEPTION_LIST::iterator itr =  m_receptionList.begin();
  std::string receptioVoice = "CReceptionVoice";
  CIsdnNetSetup * pNetSetup = 0;
  
  for ( ; itr != m_receptionList.end() ; ++itr )
    if (receptioVoice == (*itr)->NameOf() )
      {
	pNetSetup = dynamic_cast<CIsdnNetSetup  *>((*itr)->GetNetSetUp());
	if (!pNetSetup)
	  {
	    TRACESTR (eLevelError) << "CReceptionList::MarkVoiceReceptionAsDisconnected Dynamic cust failed!";
	    PASSERT(1);
	    return false;
	  }
	(*itr)->SetDisconnected();
	return true;
      }
  
  return false;
}

///////////////////////////////////////////////////////////////////////////////
CReception* CReceptionList::FindReceptionWithSameMsConversationIdConfOnAirNO(char* msConversationId)
{
	RECEPTION_LIST::iterator itr =  m_receptionList.begin();

	while (itr != m_receptionList.end())
	{
		if(strcmp((*itr)->NameOf(), "CReceptionSip") == 0) // Sip reception
		{
			if  ((strcmp(msConversationId, ((CReceptionSip*)(*itr))->GetClickToConfId()) == 0) && (*itr)->GetConfOnAirFlag() == NO)
			{
				return (*itr);
			}
		}
		itr++;
	}
	return NULL;
}

void CReceptionList::PrintReceptionList()
{
	PTRACE(eLevelInfoNormal,"CReceptionList::PrintReceptionList : ");

//	int ind = 0;
//	TRACEINTO << "CReceptionList::PrintReceptionList :  ListSize() = " << Size() << '\n';
//	RECEPTION_LIST::iterator itr =  m_receptionList.begin();
//	while (itr != m_receptionList.end())
//	{
//		CReception* pCurRec = *itr;
//		if (pCurRec != NULL)
//			TRACEINTO << "m_receptionList[" << ind++ << "] = " << pCurRec->GetPartyId();
//
//		itr++;
//	}
}
