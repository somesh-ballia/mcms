//+========================================================================+
//                            CMINFO.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CMINFO.H                                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+
#ifndef _CMINFO
#define _CMINFO

#include "PObject.h"
#include "IpCommonDefinitions.h"
#include "Capabilities.h"

//#ifndef _UCOMMODE
//#include "ucommode.h"
//#endif

#include "CapInfo.h"
#include "IPUtils.h"



#define MODE_OFF      (-1)

/*
#define MLP  1
#define HMLP 2
#define LSD  3
*/

typedef struct
{
	cmCapDataType type;
	APIS16        h320ModeType;
	CapEnum       h323ModeType;
	size_t        capStructSize;
	DWORD         bitRate;
	int           framePerPacket;
} TCommunicationMode;


#define StartAudioCap 0
#define StartVideoCap 47
#define StartT120Cap  54
#define INDEX_START_OF_HMPL 64
#define INDEX_START_OF_LSD  82


class CComModeInfo : public CPObject
{
CLASS_TYPE_1(CComModeInfo, CPObject)
public:

	// Constructors
	CComModeInfo(WORD h320ModeType, WORD startCapType);
	CComModeInfo(CapEnum h323ModeType, DWORD bitRate = 0);
	virtual ~CComModeInfo() {}

	BYTE operator==(const CComModeInfo& cmInfo) const {return (m_index == cmInfo.m_index);}
	CComModeInfo& operator=(const CComModeInfo & cmInfo){m_index = cmInfo.m_index; return (*this);}

	// Operations
	virtual const char * NameOf()              const {return "CComModeInfo";}
	cmCapDataType  GetCapType()          const {return g_comModeTbl[m_index].type;}
	WORD           GetH320ModeType()     const {return g_comModeTbl[m_index].h320ModeType;}
	CapEnum        GetH323ModeType()     const {return g_comModeTbl[m_index].h323ModeType;}
	size_t         GetStructSize()       const {return g_comModeTbl[m_index].capStructSize;}
	DWORD          GetBitRate(BYTE* pData = NULL) const;
	BYTE           IsModeOff()           const;
	const char*    GetH323CapName()      const;
	BYTE           IsModeType(CapEnum)   const;
	BYTE           IsModeType(WORD)      const;
	BYTE           IsType(cmCapDataType) const;
	int            GetFramePerPacket();
	BYTE           GetCodecNumberOfChannels();

	void SetNextH320ModeWithSameH323ModeType();
	void SetPrevH320ModeWithSameH323ModeType();
	void ChangeType(CapEnum h323ModeType,int startIndex);

	// Data global array:
	static TCommunicationMode g_comModeTbl[];

private:

	int m_index;
};

#endif // _CMINFO

