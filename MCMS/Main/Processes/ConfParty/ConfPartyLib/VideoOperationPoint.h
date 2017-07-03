
#ifndef VIDEOOPERATIONPOINT_H_
#define VIDEOOPERATIONPOINT_H_

#include <iostream>
#include <string.h>
#include "DataTypes.h"

enum RelayResourceLevelUsage //for the relay and mixvsw in MFW
{
	eResourceLevel_CIF,
	eResourceLevel_SD,
	eResourceLevel_HD720,
	eResourceLevel_HD1080
};

struct VideoOperationPoint
{
	VideoOperationPoint();
	VideoOperationPoint(const VideoOperationPoint &other);
	~VideoOperationPoint();
	void InitDefaults();
	BYTE m_layerId;
	BYTE m_tid;
	BYTE m_did;
	BYTE m_qid;
	DWORD m_frameWidth;
	DWORD m_frameHeight;
	DWORD m_frameRate;
	DWORD m_maxBitRate;
	WORD m_videoProfile;
	DWORD m_streamId;
	RelayResourceLevelUsage m_rsrcLevel;
};

#endif /* VIDEOOPERATIONPOINT_H_ */
