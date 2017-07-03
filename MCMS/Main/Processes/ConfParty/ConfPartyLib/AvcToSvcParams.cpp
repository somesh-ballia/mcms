#include <sstream>
#include "AvcToSvcParams.h"
#include "VideoOperationPointsSet.h"
#include "VideoRelayMediaStream.h"
#include "TraceStream.h"
#include "Trace.h"



////////////////////////////////////////////////
///             CAvcToSvcParams              ///
////////////////////////////////////////////////
CAvcToSvcParams::CAvcToSvcParams()
{
	m_bIsSupportAvcSvcTranslate = false;
	m_channelInHandle 			= 0;
	m_pVideoOperationPointsSet	= NULL;
	m_listVideoRelayMediaStreams.clear();
	m_encodersConnectedCount	= 0;
	m_connectToSacSSRC			= 0xFFFFFFFF;
}
//----------------------------------------------
CAvcToSvcParams::~CAvcToSvcParams ()
{
	std::list <CVideoRelayMediaStream*>::iterator itr =  m_listVideoRelayMediaStreams.begin();
	for(;itr!=m_listVideoRelayMediaStreams.end(); itr++)
	{
		POBJDELETE((*itr));
	}
}
//----------------------------------------------
CAvcToSvcParams::CAvcToSvcParams(const CAvcToSvcParams& avcToSvcParams): CPObject(avcToSvcParams)
{
	m_bIsSupportAvcSvcTranslate = avcToSvcParams.m_bIsSupportAvcSvcTranslate;
	m_channelInHandle 			= avcToSvcParams.m_channelInHandle;
	m_encodersConnectedCount	= avcToSvcParams.m_encodersConnectedCount;
	m_connectToSacSSRC			= avcToSvcParams.m_connectToSacSSRC;
	if(IsValidPObjectPtr(avcToSvcParams.m_pVideoOperationPointsSet))
	{
		m_pVideoOperationPointsSet = avcToSvcParams.m_pVideoOperationPointsSet;	// get the pointer, not allocate nor delete
	}
	else
		m_pVideoOperationPointsSet = NULL;

	std::list <CVideoRelayMediaStream*>::const_iterator itr =  avcToSvcParams.m_listVideoRelayMediaStreams.begin();
	for(;itr!=avcToSvcParams.m_listVideoRelayMediaStreams.end(); itr++)
	{
		CVideoRelayMediaStream* ob = new CVideoRelayMediaStream (*(*itr));
		m_listVideoRelayMediaStreams.push_back(ob);
	}


}
//----------------------------------------------
void CAvcToSvcParams::Dump() const
{
	std::ostringstream str;

	str << "CAvcToSvcParams::Dump :";
	str << "\n m_bIsSupportAvcSvcTranslate  = " << m_bIsSupportAvcSvcTranslate;
	str << "\n m_channelInHandle            = " << m_channelInHandle;
	str << "\n m_encodersConnectedCount		= " << m_encodersConnectedCount;
	str << "\n m_pVideoOperationPointsSet	= " << (DWORD)m_pVideoOperationPointsSet;
	str << "\n m_connectToSacSSRC			= " << (DWORD)m_connectToSacSSRC;
	std::list <CVideoRelayMediaStream*>::const_iterator itr =  m_listVideoRelayMediaStreams.begin();
	for(;itr!=m_listVideoRelayMediaStreams.end(); itr++)
	{
		str << "\n itr->GetLayerId()	= " << (DWORD)((*itr)->GetLayerId());
	}
	str << "\n ----------------------------- ";

	PTRACE(eLevelInfoNormal,str.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
void CAvcToSvcParams::Serialize(WORD format, CSegment& seg) const
{
   if (format == NATIVE)
  {
 	seg << (DWORD)m_bIsSupportAvcSvcTranslate;
	seg << (DWORD)m_channelInHandle;
	// std::list < CVideoRelayMediaStream *>  m_listVideoRelayMediaStreams;
  }
}

////////////////////////////////////////////////////////////////////////////
void CAvcToSvcParams::DeSerialize(WORD format, CSegment& seg)
{
   if (format == NATIVE)
  {
	DWORD tempVal;
	seg >> tempVal;
	m_bIsSupportAvcSvcTranslate = tempVal;
	seg >> tempVal;
	m_channelInHandle = tempVal;
	// std::list < CVideoRelayMediaStream *>  m_listVideoRelayMediaStreams;
  }
}




