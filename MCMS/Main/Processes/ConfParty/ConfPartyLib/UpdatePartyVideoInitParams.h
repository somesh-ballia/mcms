#ifndef UPDATEPARTYVIDEOINITPARAMS_H_
#define UPDATEPARTYVIDEOINITPARAMS_H_

#include "UpdatePartyInitParams.h"
#include "BridgePartyVideoParams.h"
#include "Layout.h"


class CUpdatePartyVideoInitParams : public CUpdatePartyInitParams
{
CLASS_TYPE_1(CUpdatePartyVideoInitParams,CUpdatePartyInitParams)
public:

	CUpdatePartyVideoInitParams ();
	CUpdatePartyVideoInitParams(const CBridgePartyInitParams& rOtherUpdatePartyInitParams);
	virtual const char* NameOf() const { return "CUpdatePartyVideoInitParams";}
	virtual ~CUpdatePartyVideoInitParams();
	
	virtual void AllocateInParams();
	virtual void AllocateOutParams();

	
	void InitiateMediaOutParams(const CBridgePartyVideoOutParams* pBridgePartyMediaParams);
	void InitiateMediaInParams(const CBridgePartyVideoInParams* pBridgePartyMediaParams);
	void InitOtherVideoOutParams();

	EStat UpdateVideoInParams(const CBridgePartyVideoParams* pBridgePartyVideoParams);
	EStat UpdateVideoOutParams(const CBridgePartyVideoParams* pBridgePartyVideoParams);
	void UpdateLectureModeRole(ePartyLectureModeRole partyLectureModeRole);
	void UpdatePartyLayout(CVideoLayout newVideoLayout);
    void UpdateSiteName(const char* visualName);
	
	void SetIsIVR(BYTE IsIVR){m_isIVR = IsIVR;}
		
	ePartyLectureModeRole GetLectureModeRole();
	CLayout* GetReservationLayout(int index);
	CLayout* GetPrivateReservationLayout(int index);
		
	BYTE GetIsIVR(){return m_isIVR;}
	

protected:

	CLayout* 		m_pReservation[CP_NO_LAYOUT];//For forces in party level made on the conference layout
	CLayout*		m_pPrivateReservation[CP_NO_LAYOUT];//For forces in Private Layout
	

};


#endif /*UPDATEPARTYVIDEOINITPARAMS_H_*/
