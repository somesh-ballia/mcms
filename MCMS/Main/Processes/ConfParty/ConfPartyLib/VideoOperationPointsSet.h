#ifndef VIDEOOPERATIONPOINTSSET_H_
#define VIDEOOPERATIONPOINTSSET_H_

#include "PObject.h"
#include <iostream>
#include "VideoOperationPoint.h"
#include <list>
#include <sstream>
#include <string.h>
#include "ChannelParams.h"
#include "ConfPartyApiDefines.h"

class CSegment;
struct StreamDesc;

class CVideoOperationPointsSet : public CPObject
{
	CLASS_TYPE_1(VideoOperationPointsSet,CPObject )
	// function members
	  public:
		CVideoOperationPointsSet();
		CVideoOperationPointsSet(const CVideoOperationPointsSet &other);
		~CVideoOperationPointsSet();
		virtual const char* NameOf() const { return "CVideoOperationPointsSet";}
		CVideoOperationPointsSet& operator=(const CVideoOperationPointsSet &other);
		bool operator==(const CVideoOperationPointsSet &) const;
		bool operator!=(const CVideoOperationPointsSet &) const;
		//virtual ApiBaseObject* NewCopy() const {return new VideoOperationPointsSet(*this);}
		//virtual ApiBaseObject* NewEmpty() const {return new VideoOperationPointsSet();}
		//virtual int GetCurrentBinSize() const;
		virtual int WriteToBuffer(char * buffer) const;
		virtual void InitDefaults();
		//virtual bool IsTheSame(const ApiBaseObject & base) const;
		virtual int ReadFromBuffer(const char * buffer);
		void SetSetId(DWORD setId){ m_videoOperationPointsSetId = setId;}
		DWORD GetSetId(){return m_videoOperationPointsSetId;}
		const std::list <VideoOperationPoint>* GetOperationPointsList()const {return &m_videoOperationPoints;}
		bool GetOperationPointFromList(int layerId, VideoOperationPoint& videoOperationPoint)const;
		bool GetOperationPointFromListTidDid(BYTE did, BYTE tid, VideoOperationPoint& videoOperationPoint)const;

		int GetBestLayerIdByMbpsAndFs(unsigned long aMbps, unsigned long aFs, unsigned long aBitRate);
		int GetBestLayerIdByMbpsAndFsEx(long bestMbps,long bestFs,long bestBitRate);
		int GetLayerId(unsigned int width, unsigned int height, unsigned int frameRate,unsigned int bitRate, cmCapDirection direction);
		void SetDefaultParams(DWORD confRate, bool isAvcVswInSvcOnlyConf, bool bIsEnable_1080_SVC, EOperationPointPreset eOPPreset = eOPP_cif);
		void SetDefaultParamsAvcVSW (DWORD confRate, bool bEnable_1080_SVC);
		void SetParams1Set(	BYTE layerId,
							BYTE tid,
							BYTE did,
							BYTE qid,
							DWORD frameWidth,
							DWORD frameHeight,
							DWORD frameRate,
							DWORD maxBitRate,
							WORD videoProfile,
							RelayResourceLevelUsage rsrcLevel);
		void SetDefaultParamsOld(bool isRelayModeAccordingToCallRate, DWORD confRate, bool isAvcVswInSvcOnlyConf);

		void Trace(const char* title="")const;
        void Serialize(CSegment& seg) const;
		void DeSerialize(CSegment& seg);
		void SetToOperationPointsOnly(const CVideoOperationPointsSet* pOperationPoints);
		const VideoOperationPoint* GetLowestOperationPoint(DWORD partyId) const;
		const VideoOperationPoint* GetHighestOperationPoint(DWORD partyId);

        int GetNumberOfStreams() const;
        const VideoOperationPoint*  GetHighestOperationPointForStream(int aDid) const;
        RelayResourceLevelUsage GetRelayResourceLevelUsage() const; //for the relay and mixvsw in MFW
        RelayResourceLevelUsage GetRelayResourceLevelUsage(const std::list <StreamDesc>& aStreams) const; //for the relay and mixvsw in MFW

	  protected:
		void SetAvcVswMobileOptimizedParams(DWORD confRate);
		void SetAvcVswQvgaOptimizedParams(DWORD confRate);
		void SetAvcVswCifOptimizedParams(DWORD confRate);
		void SetAvcVswVgaOptimizedParams(DWORD confRate);
		void SetAvcVswSdOptimizedParams(DWORD confRate);
		void SetAvcVswHdOptimizedParams(DWORD confRate);

	  public:
		APIS32 Mbpsr,Fsr,Mbpsl,Fsl;
		APIS16 frameRater,frameRatel;

		int VSWRelayTransmitFromH264ParamsToLayerId(APIS32 Mbps,APIS32 Fs,APIS16 frameRate,APIS32 Br);

	  // data members
	  	DWORD m_videoOperationPointsSetId;
		BYTE m_numberOfOperationPoints;
		std::list <VideoOperationPoint>  m_videoOperationPoints;
		//eVideoOperationPointSetType m_videoOperationPointsSetType;
};


#endif /* VIDEOOPERATIONPOINTSSET_H_ */
