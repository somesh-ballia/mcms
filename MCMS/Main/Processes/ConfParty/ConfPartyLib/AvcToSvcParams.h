/*
 * AvcToSvcParams.h
 *
 *  Created on: May 30, 2012
 *      Author: Amir K.
 */

#ifndef AVCTOSVCPARAMS_H_
#define AVCTOSVCPARAMS_H_

#include "PObject.h"
#include "RsrcDesc.h"
#include <list>

class CVideoRelayMediaStream;
class CVideoOperationPointsSet;

class CAvcToSvcParams : public CPObject
{
    CLASS_TYPE_1(CAvcToSvcParams, CPObject)

	public:
		// Constructors
		CAvcToSvcParams();
		virtual ~CAvcToSvcParams();
		CAvcToSvcParams(const CAvcToSvcParams& avcToSvcParams);

		// Operations
		virtual const char* NameOf() const{ return "CAvcToSvcParams";}
		virtual void Dump() const;

		//Get&Sets
		bool IsSupportAvcSvcTranslate()const{ return m_bIsSupportAvcSvcTranslate;}
		void SetIsSupportAvcSvcTranslate(bool isSupport) { m_bIsSupportAvcSvcTranslate = isSupport;}
		void SetVideoOperationPointsSet(CVideoOperationPointsSet* pVideoOperationPointsSet){m_pVideoOperationPointsSet = pVideoOperationPointsSet;};
		CVideoOperationPointsSet* GetVideoOperationPointsSet()const {return m_pVideoOperationPointsSet;}
		const std::list<CVideoRelayMediaStream*>& GetVideoRelayMediaStream(void) const {return m_listVideoRelayMediaStreams;}
		DWORD GetChannelHandle() {return m_channelInHandle;}
		void  SetChannelHandle( DWORD channelInHandle ) { m_channelInHandle = channelInHandle;}
		void  IncEncodersConnectedCount() { m_encodersConnectedCount++; }
		int	  GetEncodersConnectedCount() { return m_encodersConnectedCount; }
		DWORD GetSacSSRC() {return m_connectToSacSSRC;}

		// others
		 virtual void                 Serialize(WORD format, CSegment& seg) const;
		 virtual void                 DeSerialize(WORD format, CSegment& seg);

		std::list < CVideoRelayMediaStream *>  m_listVideoRelayMediaStreams;	// inside, m_bIsAvailable set to false in party open channel stage

	private:
		bool 	m_bIsSupportAvcSvcTranslate;	// currently always true
		DWORD 	m_channelInHandle;			// handle from OpenSvcInChannel for SVC-SVC Mix mode
		CVideoOperationPointsSet* m_pVideoOperationPointsSet;
		int		m_encodersConnectedCount;		// number of encoders already connected
		DWORD 	m_connectToSacSSRC;			// to identify which from the streams should be connected to SAC

};


#endif /* AVCTOSVCPARAMS_H_ */
