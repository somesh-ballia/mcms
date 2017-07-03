//+========================================================================+
//                            IVRSlidesList.H                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRSlidesList.H                                             |
// SUBSYSTEM:  MCMSOPER                                                    |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 20.05.96   |                                                      |
//+========================================================================+

#ifndef __IVRSLIDESLIST_H__
#define __IVRSLIDESLIST_H__

/////////////////////////////////////////////////////////////////////////////
#define MAX_SLIDES_IN_LIST	150
#define	MAX_SLIDE_NAME_LEN	301 //AT&T: 81->301
#define	MAX_SLIDE_FULL_PATHNAME_LEN	200

#define SLIDE_PROTOCOL_263	0
#define SLIDE_PROTOCOL_264	1

#define SLIDE_PROTOCOL_261	2
#define SLIDE_PROTOCOL_RTV	3

#define SLIDE_PROTOCOL_264_TIP 	4
#define SLIDE_PROTOCOL_MSSVC 	5
#define SLIDE_PROTOCOL_VP8 		6

#define MAX_SLIDE_PROTOCOL		7	// H264, Microsoft SVC, ,RTV, H263, H261

#define SLIDE_RESOLUTION_CIF		0
#define SLIDE_RESOLUTION_QCIF		1
#define SLIDE_RESOLUTION_SD			2
#define SLIDE_RESOLUTION_HD720		3
#define SLIDE_RESOLUTION_4CIF		4
#define SLIDE_RESOLUTION_HD1080		5
#define MAX_SLIDE_RES				6	// CIF, QCIF, SD, HD720, 4CIF, HD1080

/////////////////////////////////////////////////////////////////////////////
#include "PObject.h"
#include "VideoDefines.h"

/////////////////////////////////////////////////////////////////////////////
struct SSlideParams
{
	DWORD  checksum;               // name+checksum defines the file
	DWORD  dataSize;               // media size
	time_t lastupdate;             // internal (for update)
	char   fileName[MAX_SLIDE_FULL_PATHNAME_LEN];
};

/////////////////////////////////////////////////////////////////////////////
class CIVRSlide : public CPObject
{
	CLASS_TYPE_1(CIVRSlide, CPObject)

	virtual const char* NameOf() const { return GetCompileType(); }

public:

	CIVRSlide();

	virtual ~CIVRSlide() {}

public:

	STATUS AddSlide(const char* name, bool isExternalIvr = false);
	STATUS GetSlideParams(SSlideParams* slide, time_t fileMsgLastModified);
	STATUS CheckSlideFilesExistance();

	int GetH263SlideResolution(WORD resolution);
	int GetH264SlideResolution(DWORD fs, DWORD mbps);
	int GetMSSvcSlideResolution(DWORD fs, DWORD mbps);
	int GetH261SlideResolution(WORD resolution);

	SSlideParams* GetSlide(WORD protocol, WORD resolution, DWORD fs, DWORD mbps, BYTE isTipMode);
	const char*   GetSlideBaseName() { return m_baseSlideName; }

public:

	static bool ValidateSlidename(const char* name);

public:

	char         m_baseSlideName[MAX_SLIDE_NAME_LEN];
	SSlideParams m_slideParams[MAX_SLIDE_PROTOCOL][MAX_SLIDE_RES];
};

/////////////////////////////////////////////////////////////////////////////
class CIVRSlidesList  : public CPObject
{
	CLASS_TYPE_1(CIVRSlidesList, CPObject)
	
	virtual const char* NameOf() const { return GetCompileType(); }
	
public:

	CIVRSlidesList();

	virtual ~CIVRSlidesList();

public:

	STATUS AddSlide(const char* baseName, bool isExternalIvr = false);
	STATUS DelSlide(const char* name);
	STATUS CheckSlideFilesExistance(const char* baseName);

	STATUS GetSlideParams(
		const char* baseName,
		eVideoResolution resolution,
		WORD protocol,
		DWORD fs,
		DWORD mbps,
		char* realSlideName,
		WORD maxRealSlideNameLen,
		WORD& checksum,
		DWORD& mediaSize,
		BYTE isTipMode);

	int FindSlide(const char* name);

private:

	WORD       m_numberOfSlides;
	CIVRSlide* m_pSlide[MAX_SLIDES_IN_LIST];
};

/////////////////////////////////////////////////////////////////////////////
#endif // __IVRSLIDESLIST_H__ 
