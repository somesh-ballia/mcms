#ifndef _IMPORT_ISDN_PARTY_CNTL_H_
#define _IMPORT_ISDN_PARTY_CNTL_H_

#include "IsdnPartyCntl.h"

#define IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER ((WORD)405)

class CMoveIPImportParams;

////////////////////////////////////////////////////////////////////////////
//                        CImportIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
class CImportIsdnPartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CImportIsdnPartyCntl, CIsdnPartyCntl)

public:
	CImportIsdnPartyCntl();
	~CImportIsdnPartyCntl();

	CImportIsdnPartyCntl& operator=(const CImportIsdnPartyCntl& other);

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
