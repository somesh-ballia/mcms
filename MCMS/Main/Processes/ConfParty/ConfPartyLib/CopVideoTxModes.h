//+========================================================================+
//                         CopVideoTxModes.h                               |
//               Copyright 1995 Pictel Technologies Ltd.                   |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CopVideoTxModes.h                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: 		                                                       |
//-------------------------------------------------------------------------|
// Who  | Date    | Description			                                   |
//-------------------------------------------------------------------------|
//                                                                         |
//+========================================================================+
#ifndef COPVIDEOTXMODES_H_
#define COPVIDEOTXMODES_H_

#include "CopConfigurationList.h"
#include "H323Scm.h"

class CCopVideoTxModes : public CPObject
{
	CLASS_TYPE_1(CCopVideoTxModes ,CPObject)
public:
	CCopVideoTxModes ();
	~CCopVideoTxModes ();
	CCopVideoTxModes(const CCopVideoTxModes &other);
	CCopVideoTxModes& operator= (const CCopVideoTxModes &other);

	virtual const char*	NameOf () const;
	void Dump(std::ostream& msg) const;
    void Serialize(WORD format,CSegment& seg) const;
    void DeSerialize(WORD format,CSegment& seg);
	
    CVidModeH323* GetVideoMode(int index) const;
    void SetVideoTxMode(BYTE index,CVidModeH323* vidMode);
    void SetModesAccodingToCopParams(CCOPConfigurationList* pCOPConfigurationList);
    void CreateModeFromCopParams(CCopVideoParams* pCopVideoParams, int modeIndex);
    void Dump(const char* title, WORD level) const;
    WORD GetMatchingIndex(CVidModeH323* pVideoMode);
    BYTE IsValidForDefinedParams(int modeIndex, BYTE definedProtocol, DWORD definedRate);
    

private:
	CVidModeH323* m_pVideoModes[NUMBER_OF_COP_LEVELS];
};

#endif /*COPVIDEOTXMODES_H_*/
