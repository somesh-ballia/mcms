/*$Header:   M:/SCM/m3cv1d1/subsys/mcms/RSRC.H_v   1.0   08 Jul 1997 17:15:56   CARMI  $*/
// rsrc.h : interface of the CRsrc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _PARTYINTERFACE_H
#define _PARTYINTERFACE_H

#include "HardwareInterface.h"
#include "ArtDefinitions.h"

class CPartyInterface : public CHardwareInterface
{
	CLASS_TYPE_1(CPartyInterface, CHardwareInterface)
public:
	// Constructors
	CPartyInterface();
	CPartyInterface(DWORD partyRsrcId, DWORD confRsrcId);
	virtual const char* NameOf() const { return "CPartyInterface";}
	virtual ~CPartyInterface();
	CPartyInterface(const CPartyInterface&);

	// Operations
	DWORD SendCreateParty(ENetworkType networkType, DWORD maxVideoTxBitsPer10ms, BYTE bIsMrcCall = FALSE, eConfMediaType aMediaType = eAvcOnly, BOOL isEnableHighVideoResInAvcToSvcMixMode = FALSE);
	DWORD SendConnect(DWORD rsrc1ConnectionID, DWORD rsrc2ConnectionID,DWORD rsrcPartyID1, DWORD rsrcPartyID2);
	DWORD SendDeleteParty(DWORD isNeedToCollectInfoFromArt = NO);

protected:

	// Attributes

	// Operations
};

#endif /* _PARTYINTERFACE_H  */
