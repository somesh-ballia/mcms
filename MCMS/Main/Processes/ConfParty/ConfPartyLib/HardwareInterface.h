#ifndef _HARDWAREINTERFACE_H
#define _HARDWAREINTERFACE_H

#include "Interface.h"
#include "RsrcParams.h"
#include "MplMcmsProtocol.h"

// An abstract class for all HARDWARE resource objects.
class CHardwareInterface : public CInterface
{
	CLASS_TYPE_1(CHardwareInterface, CInterface)

public:
	                      CHardwareInterface();
	                    	CHardwareInterface(const CHardwareInterface&);
	virtual              ~CHardwareInterface();
	virtual const char*   NameOf() const { return "CHardwareInterface";}

	void                  Create(CRsrcParams* pRsrcParams);

	CRsrcParams*          GetRsrcParams() const { return m_pRsrcParams; }

	void                  SetPartyRsrcId(DWORD partyRsrcId);
	DWORD                 GetPartyRsrcId() const;

	void                  SetConfRsrcId(DWORD confRsrcId);
	DWORD                 GetConfRsrcId() const;

	DWORD                 GetConnectionId() const;
	eLogicalResourceTypes GetLogicalRsrcType() const { return m_pRsrcParams->GetLogicalRsrcType(); }
	void                  SetRoomId(WORD roomID);
	DWORD                 SendMsgToMPL(OPCODE opcode, CSegment* params, WORD new_card_board_id = (WORD)-1);

	void                  UpdateRsrcParams(CRsrcParams* pRsrcParams);

protected:
	CRsrcParams*          m_pRsrcParams;

	BOOL                  IsValidParams();
};

#endif /* _HARDWAREINTERFACE_H  */
