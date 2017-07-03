#include "VideoOperationPointsSet.h"
#include "H264.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "Segment.h"
#include "ConfPartyGlobals.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "SipUtils.h"
#include "CapClass.h"
#include "PrettyTable.h"

///////////////////////////////////////////////

#define FR_7_5		1920
#define FR_15		3840
#define FR_30		7680
///////////////////////////////////////////////////
CVideoOperationPointsSet::CVideoOperationPointsSet()
:Mbpsr(0)
,Fsr(0)
,Mbpsl(0)
,Fsl(0)
,frameRater(0)
,frameRatel(0)
,m_videoOperationPointsSetId(0)
,m_numberOfOperationPoints(0)

//,m_videoOperationPointsSetType(eVideoOperationPointSetTypeUndefined)
{
	//video operation points set id
	//number of operation points
	//video operation points
	m_videoOperationPoints.clear();

	//video operation points set type
}

///////////////////////////////////////////////////
CVideoOperationPointsSet::CVideoOperationPointsSet(const CVideoOperationPointsSet &other)
:CPObject(other)
,Mbpsr(other.Mbpsr)
,Fsr(other.Fsr)
,Mbpsl(other.Mbpsl)
,Fsl(other.Fsl)
,frameRater(other.frameRater)
,frameRatel(other.frameRatel)
,m_videoOperationPointsSetId(other.m_videoOperationPointsSetId)
,m_numberOfOperationPoints(other.m_numberOfOperationPoints)
//,m_videoOperationPointsSetType(other.m_videoOperationPointsSetType)
{

	//video operation points set id

	//number of operation points

	//video operation points
	m_videoOperationPoints.assign(other.m_videoOperationPoints.begin(),
				other.m_videoOperationPoints.end());

	//video operation points set type
}

///////////////////////////////////////////////////
CVideoOperationPointsSet::~CVideoOperationPointsSet()
{

	//video operation points
	m_videoOperationPoints.clear();
}

///////////////////////////////////////////////////
void CVideoOperationPointsSet::InitDefaults()
{

	//video operation points set id
	m_videoOperationPointsSetId = 0;

	//number of operation points
	m_numberOfOperationPoints = 0;

	//video operation points
	m_videoOperationPoints.clear();

	//video operation points set type
	//m_videoOperationPointsSetType = eVideoOperationPointSetTypeUndefined;
}

///////////////////////////////////////////////////
CVideoOperationPointsSet& CVideoOperationPointsSet::operator=(const CVideoOperationPointsSet &other)
{
	if (this == &other)
		return *this;

	InitDefaults();

	//video operation points set id
	m_videoOperationPointsSetId = other.m_videoOperationPointsSetId;

	//number of operation points
	m_numberOfOperationPoints = other.m_numberOfOperationPoints;

	//video operation points
	m_videoOperationPoints.clear();
	m_videoOperationPoints.assign(other.m_videoOperationPoints.begin(), other.m_videoOperationPoints.end());

	//video operation points set type
	//m_videoOperationPointsSetType = other.m_videoOperationPointsSetType;

	return *this;
}

///////////////////////////////////////////////////

bool CVideoOperationPointsSet::operator==(const CVideoOperationPointsSet& other) const
{

	//video operation points set id
	if (m_videoOperationPointsSetId != other.m_videoOperationPointsSetId)
		return false;

	//number of operation points
	if (m_numberOfOperationPoints != other.m_numberOfOperationPoints)
		return false;

	//video operation points
	if (m_videoOperationPoints.size() != other.m_videoOperationPoints.size() )
		return false;

	std::list<VideoOperationPoint>::const_iterator itr_videoOperationPoints = m_videoOperationPoints.begin();
	std::list<VideoOperationPoint>::const_iterator itr_videoOperationPoints_other = other.m_videoOperationPoints.begin();
	while ( itr_videoOperationPoints != m_videoOperationPoints.end() && itr_videoOperationPoints_other != other.m_videoOperationPoints.end() )
	{
		if (memcmp(&*itr_videoOperationPoints,&*itr_videoOperationPoints_other,sizeof(VideoOperationPoint)) != 0)
			return false;

		itr_videoOperationPoints++;
		itr_videoOperationPoints_other++;
	}

	//video operation points set type
	//if (m_videoOperationPointsSetType != other.m_videoOperationPointsSetType)
	//	return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoOperationPointsSet::operator!=(const CVideoOperationPointsSet& other) const
{
	return !(*this == other);
}

///////////////////////////////////////////////////
int CVideoOperationPointsSet::WriteToBuffer(char *buffer) const
{
	char *original_buffer = buffer;

	//video operation points set id
	*((unsigned int*) buffer) = m_videoOperationPointsSetId;
	buffer = buffer + sizeof(m_videoOperationPointsSetId);

	//number of operation points
	*((int*) buffer) = m_numberOfOperationPoints;
	buffer = buffer + sizeof(m_numberOfOperationPoints);

	//video operation points
	int size_videoOperationPoints = m_videoOperationPoints.size();
	*((int*) buffer) = size_videoOperationPoints;
	buffer = buffer + sizeof(int);
	std::list<VideoOperationPoint>::const_iterator itr_videoOperationPoints = m_videoOperationPoints.begin();
	while (itr_videoOperationPoints != m_videoOperationPoints.end())
	{
		memcpy(buffer,(const char*)&(*itr_videoOperationPoints),sizeof(VideoOperationPoint));
		buffer = buffer + sizeof(VideoOperationPoint);
		itr_videoOperationPoints++;
	}

	//video operation points set type
	/**((eVideoOperationPointSetType*) buffer) = m_videoOperationPointsSetType;
	buffer = buffer + sizeof(m_videoOperationPointsSetType);
*/
	return buffer - original_buffer;
}

///////////////////////////////////////////////////
int CVideoOperationPointsSet::ReadFromBuffer(const char *buffer)
{
	const char *original_buffer = buffer;

	InitDefaults();

	//video operation points set id
	m_videoOperationPointsSetId = *((unsigned int*)buffer);
	buffer = buffer + sizeof(m_videoOperationPointsSetId);

	//number of operation points
	m_numberOfOperationPoints = *((int*)buffer);
	buffer = buffer + sizeof(m_numberOfOperationPoints);

	//video operation points
	m_videoOperationPoints.clear();
	int size_videoOperationPoints=*((int*)buffer);
	buffer = buffer + sizeof(int);
	for (int index_videoOperationPoints = 0;
			index_videoOperationPoints < size_videoOperationPoints;
			index_videoOperationPoints++)
	{
		m_videoOperationPoints.push_back(*((VideoOperationPoint*) buffer));
		buffer = buffer + sizeof(VideoOperationPoint);
	}

	//video operation points set type
	/*m_videoOperationPointsSetType = *((eVideoOperationPointSetType*)buffer);
	buffer = buffer + sizeof(m_videoOperationPointsSetType);
*/
	return buffer - original_buffer;
}

void CVideoOperationPointsSet::SetDefaultParams(DWORD confRate, bool isAvcVswInSvcOnlyConf, bool bIsEnable_1080_SVC,
												EOperationPointPreset eOPPreset /*= eOPP_cif*/)
{

	TRACESTRFUNC(eLevelInfoHigh) << "\nConf rate:        " << confRate
								 << "\nOperPoint Preset: " << eOPPreset;

	m_numberOfOperationPoints = 0;	// must be as every SetParams1Set() increase it

	if (isAvcVswInSvcOnlyConf)
	{
		TRACEINTOFUNC << "conf rate: " << confRate << ", isAvcVswInSvcOnlyConf==true";
//		SetDefaultParamsAvcVSW ( confRate, bIsEnable_1080_SVC );
//		SetAvcVswCifOptimizedParams(confRate);

		switch (eOPPreset)
		{
			case eOPP_mobile:
				SetAvcVswMobileOptimizedParams(confRate);
				break;
			case eOPP_qvga:
				SetAvcVswQvgaOptimizedParams(confRate);
				break;
			case eOPP_cif:
				SetAvcVswCifOptimizedParams(confRate);
				break;
			case eOPP_vga:
				SetAvcVswVgaOptimizedParams(confRate);
				break;
			case eOPP_sd:
				SetAvcVswSdOptimizedParams(confRate);
				break;
			case eOPP_hd:
				SetAvcVswHdOptimizedParams(confRate);
				break;
			default:
				PASSERT(eOPPreset);
				SetDefaultParamsAvcVSW(confRate, bIsEnable_1080_SVC);
				break;
		}
		return;
	}

	// filling the table according to Conference Rate
	//	Resolution	bitrate
	//	180p7.5		86
	//	180p15		128
	//	180p30		192
	//	360p7.5		173
	//	360p15		256
	//	360p30		384
	//	720p7.5		346
	//	720p15		512
	//	720p30		768
	//	270p7.5		115
	//	270p15		170
	//	270p30		256
	//	540p7.5		230
	//	540p15		341
	//	540p30		512
	//	1080p7.5	554
	//	1080p15		821
	//	1080p30		1232


	if(Is1080pSupportedInOperationPoint(confRate) && confRate>=2048 && confRate<=4096)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    86,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,    	384,   SVC_Profile_High,   eResourceLevel_SD);      // fr: 30
		// Stream #3
		SetParams1Set(   6,    0,   2,   0,   1920,     1080,    FR_7_5,   	554,  SVC_Profile_High,    eResourceLevel_HD1080);       // fr: 7.5
		SetParams1Set(   7,    1,   2,   0,   1920,     1080,    FR_15,    	821,  SVC_Profile_High,    eResourceLevel_HD1080);       // fr: 15
		SetParams1Set(   8,    2,   2,   0,   1920,     1080,    FR_30,    	1232, SVC_Profile_High,    eResourceLevel_HD1080);       // fr: 30
	}
	else if(confRate>=1472)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    86,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,    	384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
		SetParams1Set(   6,    0,   2,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   7,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
		SetParams1Set(   8,    2,   2,   0,   1280,     720,     FR_30,     768,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 30
	}
	else if(confRate>=1152)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    86,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,    	384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
		SetParams1Set(   6,    0,   2,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   7,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
	}
	else if(confRate>=1024)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    86,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		// Stream #3
		SetParams1Set(   5,    0,   2,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   6,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
	}

	else if(confRate>=768)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    43,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	// fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     64,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	// fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     96,    SVC_Profile_BaseLine,   eResourceLevel_CIF);    // fr: 30


		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    86,    SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     128,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,    	192,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30


		// Stream #3
		SetParams1Set(   6,    0,   2,   0,   1280,     720,     FR_7_5,    292,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   7,    1,   2,   0,   1280,     720,     FR_15,     432,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15



	}
	else if(confRate>=512)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    43,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     64,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     96,    SVC_Profile_BaseLine,   eResourceLevel_CIF);    // fr: 30

		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    86,    SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     128,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,    	192,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30


	}
	else if(confRate>=256)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    43,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     64,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15

		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   640,      360,     FR_7_5,    86,    SVC_Profile_High,   eResourceLevel_SD);	     // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   640,      360,     FR_15,     128,   SVC_Profile_High,   eResourceLevel_SD);	     // fr: 15


	}
	else if(confRate>=192)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    43,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     64,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     96,    SVC_Profile_BaseLine,   eResourceLevel_CIF);    // fr: 30

	}
	else if(confRate>=128)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,    43,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     64,    SVC_Profile_BaseLine,   eResourceLevel_CIF);	 // fr: 15

	}
	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
	{
		PASSERT((confRate*100) + m_videoOperationPoints.size());
	}

}

void CVideoOperationPointsSet::SetAvcVswMobileOptimizedParams(DWORD confRate)
{
	TRACEINTO <<"Conf rate: " << confRate;
	// filling the table according to Conference Rate
	//	Resolution	bitrate
	//	180p15		128
	//	180p30		192
	//	360p7.5		173
	//	360p15		256
	//	360p30		384
	//	720p7.5		346
	//	720p15		512
	//	720p30		768

	if (confRate >= 1472)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,   Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,     384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
		SetParams1Set(   6,    0,   2,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   7,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
		SetParams1Set(   8,    2,   2,   0,   1280,     720,     FR_30,     768,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 30
	}
	else
	if (confRate >= 1024)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,     384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
		SetParams1Set(   6,    0,   2,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   7,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
	}
	else
	if (confRate >= 768)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   5,    2,   1,   0,   640,      360,     FR_30,     384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
	}
	else
	if (confRate>=512)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 30
		// Stream #2
		SetParams1Set(   3,    0,   1,   0,   640,      360,     FR_7_5,    173,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   4,    1,   1,   0,   640,      360,     FR_15,     256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
	}
	else
	if (confRate>=256)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,     128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		// Stream #2
		SetParams1Set(   2,    2,   0,   0,   320,      180,     FR_30,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 30

	}
	else
	if (confRate>=192)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 7.5
		SetParams1Set(   1,    1,   0,   0,   320,      180,     FR_15,      128,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
	}
	else
	if (confRate>=128)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   320,      180,     FR_7_5,     64,   SVC_Profile_BaseLine,   eResourceLevel_CIF);      // fr: 7.5
	}
	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
		PASSERT((confRate*100) + m_videoOperationPoints.size());
}

void CVideoOperationPointsSet::SetAvcVswQvgaOptimizedParams(DWORD confRate)
{
	TRACEINTO <<"Conf rate: " << confRate;
	// filling the table according to Conference Rate
	//	Resolution	bitrate
	//	240p15		150
	//	240p30		225
	//	480p7.5		200
	//	480p15		300
	//	480p30		450
	//	720p7.5		346
	//	720p15		512
	//	720p30		768

	if (confRate >= 1728)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,   Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	288,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 30
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   848,     	480,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   848,     	480,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15

		// Stream #3
		SetParams1Set(   4,    0,   2,   0,   1280,     720,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   5,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15

	}
	else
	if (confRate>768)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 15
		SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	288,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   848,     	480,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   848,     	480,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
	}
	else
	if (confRate==768)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 15
		SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	288,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   848,     	480,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
	}
	else
	if (confRate>=384)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     192,   SVC_Profile_BaseLine,   eResourceLevel_CIF);      // fr: 15
		SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	288,   SVC_Profile_BaseLine,   eResourceLevel_CIF);   // fr: 30
	}
	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
		PASSERT((confRate*100) + m_videoOperationPoints.size());

}
//------------------------------------------------------------------------
void CVideoOperationPointsSet::SetAvcVswCifOptimizedParams(DWORD confRate)
{
	if (confRate >= 1728)
		{
			// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
			SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
			SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 3030

			// Stream #2
			SetParams1Set(   2,    0,   1,   0,   848,     	480,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
			SetParams1Set(   3,    1,   1,   0,   848,     	480,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5

			// Stream #3
			SetParams1Set(   4,    0,   2,   0,   1280,     720,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
			SetParams1Set(   5,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15


		}
		else
		if (confRate>1024)
		{
			// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
			SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
			SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 3030
			// Stream #2
			SetParams1Set(   2,    0,   1,   0,   848,     	480,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
			SetParams1Set(   3,    1,   1,   0,   848,     	480,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5

		}
		else
		if (confRate==1024)
		{
			// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
			SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
			SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 3030
			// Stream #2
			SetParams1Set(   2,    0,   1,   0,   848,     	480,     FR_7_5,    384,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
			}
		else
		if (confRate>512)
		{
			// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
			SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
			SetParams1Set(   1,    1,   0,   0,   424,      240,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 3030
		}
		else
		if (confRate==512)
		{
			// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
			SetParams1Set(   0,    0,   0,   0,   424,      240,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_CIF);       // fr: 15
		}
		else
		{
			PASSERT((confRate+1));
		}

		if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
			PASSERT((confRate*100) + m_videoOperationPoints.size());
}
void CVideoOperationPointsSet::SetAvcVswVgaOptimizedParams(DWORD confRate)
{
	TRACEINTO <<"Conf rate: " << confRate;
	if (confRate>=1728)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,      480,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   848,      480,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030

		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   1280,     720,     FR_30,     768,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15

	}
	else
	if (confRate>=1152)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,      480,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   848,      480,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030

		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   1280,     720,     FR_15,    512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5


	}
	else
	if (confRate>=768)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,      480,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   848,      480,     FR_30,    	512,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030

	}
	else
	if (confRate>=512)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,      480,     FR_15,     384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
	}

	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
		PASSERT((confRate*100) + m_videoOperationPoints.size());
}

void CVideoOperationPointsSet::SetAvcVswSdOptimizedParams(DWORD confRate)
{
	TRACEINTO <<"Conf rate: " << confRate;

	if (confRate >= 1536)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,   Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,     	480,     FR_15,     512,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   848,     	480,     FR_30,     720,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 30


		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
		SetParams1Set(   3,    1,   1,   0,   1280,     720,     FR_30,     768,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 30
	}
	else
	if (confRate>=1280)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,     	480,     FR_15,     512,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   848,     	480,     FR_30,     720,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 30

		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
	}
	else
	if (confRate>=768)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   848,     	480,     FR_15,     512,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   848,     	480,     FR_30,     720,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 30

	}

	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
		PASSERT((confRate*100) + m_videoOperationPoints.size());

}
void CVideoOperationPointsSet::SetAvcVswHdOptimizedParams(DWORD confRate)
{
	TRACEINTO <<"Conf rate: " << confRate;

	if (confRate >= 832)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,   Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   1280,     720,     FR_30,     768,   SVC_Profile_BaseLine,   eResourceLevel_HD720);       // fr: 30
	}
	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
		PASSERT((confRate*100) + m_videoOperationPoints.size());

}

///////////////////////////////////////////////////
void CVideoOperationPointsSet::SetDefaultParamsAvcVSW (DWORD confRate, bool bEnable_1080_SVC)
{

	// qqqAmir what about call-rate above 3M?

	// filling the table according to Conference Rate
	//	Resolution	bitrate
	//	180p7.5		86
	//	180p15		128
	//	180p30		192
	//	360p7.5		173
	//	360p15		256
	//	360p30		384
	//	720p7.5		346
	//	720p15		512
	//	720p30		768
	//	270p7.5		115
	//	270p15		170
	//	270p30		256
	//	540p7.5		230
	//	540p15		341
	//	540p30		512
	//	1080p7.5	554
	//	1080p15		821
	//	1080p30		1232
	if(bEnable_1080_SVC && confRate>=2560 && confRate<=3072)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   640,      360,     FR_15,     256,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   640,      360,     FR_30,    	384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 30
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   960,     	540,     FR_7_5,    256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   960,     	540,     FR_15,     341,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   4,    2,   1,   0,   960,     	540,     FR_30,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
		SetParams1Set(   5,    0,   2,   0,   1920,     1080,    FR_7_5,   	554,  SVC_Profile_High,   eResourceLevel_HD1080);       // fr: 7.5
		SetParams1Set(   6,    1,   2,   0,   1920,     1080,    FR_15,    	821,  SVC_Profile_High,   eResourceLevel_HD1080);       // fr: 15
		SetParams1Set(   7,    2,   2,   0,   1920,     1080,    FR_30,    	1232, SVC_Profile_High,   eResourceLevel_HD1080);       // fr: 30
	}
	else
	if(confRate>=1728)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,   Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   640,      360,     FR_15,     256,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   640,      360,     FR_30,    	384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   960,     	540,     FR_7_5,    256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   960,     	540,     FR_15,     341,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   4,    2,   1,   0,   960,     	540,     FR_30,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
		// Stream #3
		SetParams1Set(   5,    0,   2,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   6,    1,   2,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
		SetParams1Set(   7,    2,   2,   0,   1280,     720,     FR_30,     768,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 30
	}
	else
	if (confRate>=1280)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   640,      360,     FR_15,     256,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   640,      360,     FR_30,    	384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   1280,     720,     FR_7_5,    346,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   1280,     720,     FR_15,     512,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 15
		SetParams1Set(   4,    2,   1,   0,   1280,     720,     FR_30,     768,   SVC_Profile_High,   eResourceLevel_HD720);       // fr: 30
	}
	else
	if (confRate>=1024)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   640,      360,     FR_15,     256,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   640,      360,     FR_30,    	384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030
		// Stream #2
		SetParams1Set(   2,    0,   1,   0,   960,     	540,     FR_7_5,    256,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 7.5
		SetParams1Set(   3,    1,   1,   0,   960,     	540,     FR_15,     341,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   4,    2,   1,   0,   960,     	540,     FR_30,     512,   SVC_Profile_High,   eResourceLevel_SD);       // fr: 30
	}
	else
	if (confRate>=512)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   640,      360,     FR_15,     256,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
		SetParams1Set(   1,    1,   0,   0,   640,      360,     FR_30,    	384,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 3030
	}
	else
	if (confRate>=384)
	{
		// Stream #1   layer, tid, did, qid, Frame-W, Frame-H, Frame-Rate, Max-Bit-Rate,    Video-Profile,  (Aud: 48)
		SetParams1Set(   0,    0,   0,   0,   640,      360,     FR_15,     256,   SVC_Profile_BaseLine,   eResourceLevel_SD);       // fr: 15
	}
	else
	{
		PASSERT((confRate+1));
	}

	if (m_videoOperationPoints.size() != m_numberOfOperationPoints )
		PASSERT((confRate*100) + m_videoOperationPoints.size());
}



///////////////////////////////////////////////////
void CVideoOperationPointsSet::SetParams1Set( BYTE layerId,
											BYTE tid,
											BYTE did,
											BYTE qid,
											DWORD frameWidth,
											DWORD frameHeight,
											DWORD frameRate,
											DWORD maxBitRate,
											WORD videoProfile,
											RelayResourceLevelUsage rsrcLevel )
{
	VideoOperationPoint tmpVideoOperationPoint;

	tmpVideoOperationPoint.m_layerId 		= layerId;
	tmpVideoOperationPoint.m_tid 			= tid;
	tmpVideoOperationPoint.m_did 			= did;
	tmpVideoOperationPoint.m_qid 			= qid;
	tmpVideoOperationPoint.m_frameWidth 	= frameWidth;
	tmpVideoOperationPoint.m_frameHeight 	= frameHeight;
	tmpVideoOperationPoint.m_frameRate 		= frameRate;
	tmpVideoOperationPoint.m_maxBitRate 	= maxBitRate;
	tmpVideoOperationPoint.m_videoProfile 	= videoProfile;
	tmpVideoOperationPoint.m_rsrcLevel      = rsrcLevel;
	m_videoOperationPoints.push_back(tmpVideoOperationPoint);

	m_numberOfOperationPoints++;
}


void CVideoOperationPointsSet::Trace(const char* title) const
{
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    VideoOperationPoint op;

	CPrettyTable<int, int, int, int, int, int, int, int, int, const char*> tbl("Layerid", "Tid", "Did", "Qid", "Profile", "FrameWidth", "FrameHeight", "FrameRate", "BitRate", "RsrcLevel");

    for (std::list <VideoOperationPoint>::const_iterator itr = list->begin(); itr != list->end(); itr++)
    {
        op = (*itr);
		tbl.Add((int)op.m_layerId, (int)op.m_tid, (int)op.m_did, (int)op.m_qid, (int)op.m_videoProfile, (int)op.m_frameWidth,
				(int)op.m_frameHeight, (int)op.m_frameRate, (int)op.m_maxBitRate, GetResourceLevelStr(op.m_rsrcLevel));
	}
	TRACEINTO << "Number of operation points=" << list->size() << tbl.Get();
}

/////////////////////////////////////////////////////////////////////////////
void  CVideoOperationPointsSet::Serialize(CSegment& seg) const
{
    seg << m_videoOperationPointsSetId;
    seg << m_numberOfOperationPoints;

    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
    VideoOperationPoint op;
    int i =0;
    DWORD rsrcLevel;

    for(;itr != list->end(); itr++)
    {
        op = (*itr);

        rsrcLevel = op.m_rsrcLevel;
        seg << op.m_layerId << op.m_tid << op.m_did << op.m_qid
                << op.m_frameWidth << op.m_frameHeight << op.m_frameRate << op.m_maxBitRate
                << op.m_videoProfile << op.m_streamId << rsrcLevel;

    }
}

/////////////////////////////////////////////////////////////////////////////
void  CVideoOperationPointsSet::DeSerialize(CSegment& seg)
{
    InitDefaults();

    seg >> m_videoOperationPointsSetId;
    seg >> m_numberOfOperationPoints;

    VideoOperationPoint op;
    int i =0;
    DWORD rsrcLevel;

    for(int i =0; i < m_numberOfOperationPoints; i++)
    {
        seg >> op.m_layerId >> op.m_tid >> op.m_did >> op.m_qid
                >> op.m_frameWidth >> op.m_frameHeight >> op.m_frameRate >> op.m_maxBitRate
                >> op.m_videoProfile >> op.m_streamId >> rsrcLevel;
        op.m_rsrcLevel = (RelayResourceLevelUsage)rsrcLevel;

        m_videoOperationPoints.push_back(op);
    }
}

/////////////////////////////////////////////////////////////////////////////
const VideoOperationPoint*   CVideoOperationPointsSet::GetLowestOperationPoint(DWORD partyId) const
{
	int i=1;
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();

    if(itr == list->end())
    {
     	//PASSERT(1);
	TRACEINTO << "Error, empty list, return NULL !!";
    	return NULL;
    }


//    std::list <VideoOperationPoint>::const_iterator itr2=itr;
//    if(itr2!=list->end())
//    {
//    	itr2++; // should consult - lowest operation point is just too low eyaln9794
//    	if(itr2!=list->end())
//    	{
//    		itr=itr2;
//    		i++;
//    	}
//    }
//    if(itr2!=list->end())
//    {
//    	itr2++; // should consult - lowest operation point is just too low eyaln9794
//    	if(itr2!=list->end())
//    	{
//    		itr=itr2;
//    		i++;
//    	}
//    }

	if(partyId)
	{
		TRACEINTO<<"avc_vsw_relay: lowest operation point used i: "<<i<<", height: "<<itr->m_frameHeight<<", Width: "<<itr->m_frameWidth<<", frameRate: "<<itr->m_frameRate<<", bitrate: "<<itr->m_maxBitRate<<", partyId:"<<partyId;
	}
	else
	{
		TRACEINTO<<"avc_vsw_relay: lowest operation point used i: "<<i<<", height: "<<itr->m_frameHeight<<", Width: "<<itr->m_frameWidth<<", frameRate: "<<itr->m_frameRate<<", bitrate: "<<itr->m_maxBitRate<<", partyId not allocated yet";
	}

	return &(*itr);
}

/////////////////////////////////////////////////////////////////////////////
const VideoOperationPoint* CVideoOperationPointsSet::GetHighestOperationPoint(DWORD partyId)
{
	int i=1;
	const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
    std::list <VideoOperationPoint>::const_iterator prevItr=itr;
    if(itr == list->end())
    {
     	//PASSERT(1);
	TRACEINTO << "Error, empty list, return NULL !!";
    	return NULL;
    }

    for(;itr != list->end(); itr++)
    {
    	prevItr=itr;
    	i++;
    }

	if(partyId)
	{
		TRACEINTO<<"avc_vsw_relay: higher operation point used i: "<<i<<" height: "<<prevItr->m_frameHeight<<" Width: "<<prevItr->m_frameWidth<<" frameRate: "<<prevItr->m_frameRate<<" bitrate: "<<prevItr->m_maxBitRate<<" partyId:"<<partyId;
	}
	else
	{
		TRACEINTO<<"avc_vsw_relay: higher operation point used i: "<<i<<" height: "<<prevItr->m_frameHeight<<" Width: "<<prevItr->m_frameWidth<<" frameRate: "<<prevItr->m_frameRate<<" bitrate: "<<prevItr->m_maxBitRate<<" partyId not allocated yet";
	}

	return &(*prevItr);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoOperationPointsSet::SetToOperationPointsOnly(const CVideoOperationPointsSet* pOperationPoints)
{
    const std::list <VideoOperationPoint>* list = pOperationPoints->GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();

    const std::list <VideoOperationPoint>* list1 = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr1;
    int flag;

    VideoOperationPoint op;
    VideoOperationPoint op1;

    APIS32 OpResolution;
    APIS32 OpResolution1;
    m_numberOfOperationPoints=0;

    for(;itr != list->end(); itr++)
    {
        op = (*itr);
        OpResolution=op.m_frameHeight*op.m_frameWidth;

        flag=false;
        for(itr1 =  list1->begin();itr1 != list1->end() && flag==false; itr1++)
        {
        	op1=(*itr1);
            OpResolution1=op1.m_frameHeight*op1.m_frameWidth;
            if(OpResolution==OpResolution1)
            {
            	flag=true;
            }
        }


        if(flag==false)
        {
//        	TRACEINTOFUNC<<"@@@! SetToOperationPointsOnly OpResolution: "<<OpResolution;
        	m_numberOfOperationPoints++;
        	m_videoOperationPoints.push_back(op);
        }
    }
}



int CVideoOperationPointsSet::GetBestLayerIdByMbpsAndFsEx(long bestMbps,long bestFs, long bestBitRate)
{
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr;
    VideoOperationPoint op;
    long opFs = 0;
    long opMbps = 0;
    long opBitRate = 0;
    bool found = false;
    bool bypassed = false;
    int layerId = -1;
    for(itr =  list->begin();itr != list->end() && bypassed==false; itr++)
    {
        op = (*itr);
        opFs=(op.m_frameWidth*op.m_frameHeight)>>16;
        opMbps=(op.m_frameWidth*op.m_frameHeight*op.m_frameRate)>>16;
        opMbps=opMbps/500;
        if(opFs==0)
        {
        	opFs=-1;
        	opMbps=-1;

        }
        opBitRate=op.m_maxBitRate*10;

        if(bestFs>=opFs && bestBitRate>=opBitRate)
        {
        	found=true;
        	layerId=op.m_layerId;
        }
        else
        {
        	bypassed=true;
        }
    }

	TRACEINTO << "bestFs: " << bestFs << ", opFs: " << opFs << ", bestBR: " << bestBitRate << ", opBR: " << opBitRate << ", layerId: " << layerId;

    return layerId;
}

int CVideoOperationPointsSet::GetBestLayerIdByMbpsAndFs(unsigned long aMbps, unsigned long aFs, unsigned long aBitRate)
{
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
    VideoOperationPoint op;
    int layerId = -1;
    unsigned long mbps = 0;
    unsigned long fs = 0;
    unsigned long bitRate = 0;

    for(;itr != list->end(); itr++)
    {
        op = (*itr);
        mbps = ::CalcOperationPointMBPS(op);
        fs = ::CalcOperationPointFS(op);
        bitRate = op.m_maxBitRate;

        if (mbps < aMbps && fs < aFs && bitRate < aBitRate)
        {
            layerId++;
        }
        else
        {
            break;
        }
    }
    return layerId;
}
////////////////////////////////////////////////////////////////////////////////////////
bool CVideoOperationPointsSet::GetOperationPointFromList(int layerId, VideoOperationPoint& videoOperationPoint)const
{
	const std::list <VideoOperationPoint>* list = GetOperationPointsList();
	std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
	bool bFound = false;
	for(;itr != list->end() || bFound; itr++)
	{
		if ( (*itr).m_layerId == layerId)
		{
			bFound = true;
			videoOperationPoint = (*itr);
			break;
		}
	}
	return bFound;
}




// amirK
bool CVideoOperationPointsSet::GetOperationPointFromListTidDid(BYTE did, BYTE tid, VideoOperationPoint& videoOperationPoint)const
{
	const std::list <VideoOperationPoint>* list = GetOperationPointsList();
	std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
	bool bFound = false;
	for(;itr != list->end() || bFound; itr++)
	{
		if ( (*itr).m_tid == tid && (*itr).m_did == did)
		{
			bFound = true;
			videoOperationPoint = (*itr);
			break;
		}
	}
	return bFound;
}

int CVideoOperationPointsSet::GetLayerId(unsigned int width, unsigned int height, unsigned int frameRate,unsigned int bitRate, cmCapDirection direction)
{
    int retVal = ILLEGAL_LAYER_ID; //-1;
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
    VideoOperationPoint op;

    if (direction == cmCapReceive)
    {
        for(;itr != list->end(); itr++)
        {
            op = (*itr);
            // when looking for receive stream: find the lowest operation point that it's resolution >= stream resolution
            if ((op.m_frameWidth >= width)
                    && (op.m_frameHeight >= height)
                    && (op.m_frameRate >= frameRate)
                    && (op.m_maxBitRate >= bitRate))
            {
                return op.m_layerId;
            }
        }
    }
    else //direction == cmCapTransmit
    {
        for(;itr != list->end(); itr++)
        {
            op = (*itr);
            // when looking for send stream: find the highest operation point that it's resolution <= stream resolution
            if ((op.m_frameWidth <= width)
                    && (op.m_frameHeight <= height)
                    && ((op.m_frameRate <= frameRate) || (frameRate == 0))
/*                  && ((op.m_maxBitRate <= bitRate) || (bitRate == 0))*/ )
            {
                return op.m_layerId;
            }
        }

    }
    return retVal;
}

// assumes operation points are sorted from low to high
int CVideoOperationPointsSet::GetNumberOfStreams() const
{
    int numberOfStreams = 0;

    std::list <VideoOperationPoint>::const_iterator itr =  m_videoOperationPoints.begin();
    int did = -1;
    for(;itr != m_videoOperationPoints.end(); itr++)
    {
        if ((*itr).m_did != did)
        {
            did = (*itr).m_did;
            numberOfStreams++;
        }
    }

    return numberOfStreams;
}

// assumes operation points are sorted from low to high
const VideoOperationPoint*  CVideoOperationPointsSet::GetHighestOperationPointForStream(int aDid) const
{
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
    const VideoOperationPoint* vop = NULL;
    bool bFound = false;

    for (;itr != list->end(); itr++)
    {
        if ( (*itr).m_did == aDid )
        {
            bFound = true;
            vop = &(*itr);
        }
        else if (bFound == true)
        {
            break;
        }
    }
    return vop;

}

//////////////////////////////////////////////////////////////////////////////////
RelayResourceLevelUsage CVideoOperationPointsSet::GetRelayResourceLevelUsage() const
{
	RelayResourceLevelUsage rsrcLevel = eResourceLevel_CIF;
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    PASSERT_AND_RETURN_VALUE ((!list || list->empty()), rsrcLevel);

    VideoOperationPoint op = list->back();
	return op.m_rsrcLevel;
}

//////////////////////////////////////////////////////////////////////////////////
RelayResourceLevelUsage CVideoOperationPointsSet::GetRelayResourceLevelUsage(const std::list <StreamDesc>& aStreams) const
{
    RelayResourceLevelUsage rsrcLevel = eResourceLevel_CIF;

    if (!aStreams.size())
        return GetRelayResourceLevelUsage();

    // get the last stream which is the highest level
    const StreamDesc & stream = aStreams.back();

    // find the operation point that matches the stream
    const std::list <VideoOperationPoint>* list = GetOperationPointsList();
    std::list <VideoOperationPoint>::const_iterator itr =  list->begin();
    const VideoOperationPoint* vop = NULL;
    bool bFound = false;

    TRACEINTO << "Checking stream: width=" << stream.m_width << " height=" << stream.m_height << " frameRate=" << stream.m_frameRate << " maxBitRate=" << stream.m_bitRate;

    for (;itr != list->end(); itr++)
    {
        TRACEINTO << "Checking operation point: width=" << (*itr).m_frameWidth << " height=" << (*itr).m_frameHeight <<
                " frameRate=" << (*itr).m_frameRate << " maxBitRate=" << (*itr).m_maxBitRate;
        if ((*itr).m_frameWidth == stream.m_width &&
            (*itr).m_frameHeight == stream.m_height &&
            (*itr).m_frameRate == stream.m_frameRate)
        {
            bFound = true;
            TRACEINTO << "Found op";
            vop = &(*itr);
            break;
        }
    }

    if (vop)
    {
        rsrcLevel = vop->m_rsrcLevel;
        TRACEINTO << "rsrcLevel=" << GetResourceLevelStr(rsrcLevel);
    }
    return rsrcLevel;
}
