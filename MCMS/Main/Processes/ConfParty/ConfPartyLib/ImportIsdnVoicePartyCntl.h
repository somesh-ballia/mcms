#ifndef _IMPORT_ISDN_VOICE_PARTY_CNTL_H_
#define _IMPORT_ISDN_VOICE_PARTY_CNTL_H_

#include "IsdnPartyCntl.h"

#define IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER ((WORD)405)

class CMoveIPImportParams;

////////////////////////////////////////////////////////////////////////////
//                        CImportIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
class CImportIsdnVoicePartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CImportIsdnVoicePartyCntl, CIsdnPartyCntl)

public:
	CImportIsdnVoicePartyCntl();
	~CImportIsdnVoicePartyCntl();

	CImportIsdnVoicePartyCntl& operator=(const CImportIsdnVoicePartyCntl& other);

	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();

	void Create(CMoveIPImportParams* pMoveImportParams);
	void CheckBridgeConnection();
	void OnAudConnect(CSegment* pParam);
	void OnTimerConnectToAudioBrdg(CSegment* pParam);

protected:

	PDECLAR_MESSAGE_MAP
};


#endif
