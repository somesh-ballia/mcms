/*
 * SvcToAvcParams.h
 *
 *  Created on: May 14, 2012
 *      Author: kgratz
 */

#ifndef SVCTOAVCPARAMS_H_
#define SVCTOAVCPARAMS_H_

#include "PObject.h"
#include "RsrcDesc.h"
#include "VideoOperationPointsSet.h"
#include "VideoRelayMediaStream.h"

extern char* LogicalResourceTypeToString(APIU32 logicalResourceType);

class CImage;

class CSvcToAvcParams : public CPObject
{
    CLASS_TYPE_1(CSvcToAvcParams, CPObject)

	public:
		// Constructors
		CSvcToAvcParams();
		virtual ~CSvcToAvcParams();
		CSvcToAvcParams(const CSvcToAvcParams& svcToAvcParams);


		// Operations
		virtual const char* NameOf() const{ return "CSvcToAvcParams";}
		virtual void Dump() const;

		//Get&Sets
		bool IsSupportSvcAvcTranslate()const{ return m_bIsSupportSvcAvcTranslate;}
		void SetIsSupportSvcAvcTranslate(bool isSupport) { m_bIsSupportSvcAvcTranslate = isSupport;}

		void SetVideoOperationPointsSet(CVideoOperationPointsSet* pVideoOperationPointsSet){m_pVideoOperationPointsSet = pVideoOperationPointsSet;};
		CVideoOperationPointsSet* GetVideoOperationPointsSet()const {return m_pVideoOperationPointsSet;}

//		void SetVideoRelayInchannelHandle(DWORD videoRelayInchannelHandle){ m_videoRelayInchannelHandle =videoRelayInchannelHandle;}
//		DWORD GetVideoRelayInchannelHandle()const {return m_videoRelayInchannelHandle;}

		CVideoRelayMediaStream * GetVideoRelayMediaStream()const{return m_pTranslatedVideoRelayMediaStream;}
		bool GetIsAvcInConf() {return m_bIsAvcInConf;}
		void SetIsAvcInConf(bool isAvcInConf) { m_bIsAvcInConf = isAvcInConf;}

		void UpdateTranslatedVideoRelayMediaStream(CVideoRelayMediaStream * pVideoRelayMediaStream);
		bool IsValidParams();

		//int  GetRequiredLayerId(){return m_requiredLayerId;}
		//void SetRequiredLayerId(int layerId);
		//bool CalculateLayerId( CImage *pImage);
		void SetMaxAllowedLayerId( int maxAllowedLayerId );
		void SetMaxAllowedLayerIdHighRes( int maxAllowedLayerIdHighRes );
		void SetEnableDisableHighResFlag(BOOL bIsHighResStreamEnable) { m_bIsHighResStreamEnable = bIsHighResStreamEnable;}
		int  GetAllowedLayerId()const;	// related to HighStream Enable/Disable

	private:
		bool	m_bIsSupportSvcAvcTranslate;
		//int		m_requiredLayerId;					//the max layer-id to be translated
		DWORD	m_videoRelayInchannelHandle;
		bool	m_bIsAvcInConf;
		CVideoRelayMediaStream* 	m_pTranslatedVideoRelayMediaStream;
		CVideoOperationPointsSet* 	m_pVideoOperationPointsSet;
		int		m_maxAllowedLayerId;
		int		m_maxAllowedLayerIdHighRes;
		BOOL	m_bIsHighResStreamEnable;
};


#endif /* SVCTOAVCPARAMS_H_ */
