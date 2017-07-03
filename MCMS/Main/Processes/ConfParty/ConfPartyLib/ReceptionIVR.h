// ReceptionIVR.h: API of CReceptionIVR class.


//////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date         Updated By         Description
//
//17/7/05		Yoella				Porting to Carmel
//========   ==============   =====================================================================

// Reception for IVR MOVE !!!

#if !defined(_ReceptionIVR_H__)
#define _ReceptionIVR_H__

#include "Reception.h"
//#include "CommConf.h"

//class CH323NetSetup;

																	 
class CReceptionIVR : public CReception
{
CLASS_TYPE_1(CReceptionIVR,CReception )	

public:
	
	CReceptionIVR(CTaskApp *pOwnerTask);    
	virtual const char* NameOf() const { return "CReceptionIVR";}
	virtual ~CReceptionIVR();  
	
	void  CreateConfAndSuspendIVRMove(COsQueue* pLobbyRcvMbx,CLobby* pLobby, CTaskApp* ListId,
						DWORD dwSourceConfId,char* adHocConfName, DWORD dwPartyID,ETargetConfType targetConfType,CCommRes* pAdHocRsrv);

    void  CreateSuspendIVR(COsQueue* pLobbyRcvMbx,CLobby* pLobby, CTaskApp* ListId,DWORD dwSourceConfId,
    								 char* adHocConfName,DWORD dwPartyID,ETargetConfType targetConfType);
    								
								 						  							 
   	void  OnTimerPartySuspend(CSegment* pParam);
   	void  OnConfMngrRejectPartySuspend(CSegment* pParam);
	    
	virtual void*  GetMessageMap();                             
    virtual void   AcceptParty(CNetSetup* pNetSetUp,CCommConf* pCommConf,CConfParty* pConfParty);
    
	 
		
	protected:

	// Operations
	virtual void  OnException(); 
	//void SetAdHocConfName(char* adHocConfName);
	void SetAdHocNumericId(char* adHocNumericId); 
	//char* GetAdHocConfName();
	char* GetAdHocNumericId();
    void SetRsrvPartyID(DWORD dwRsrvPartyId);
    
    DWORD GetRsrvPartyID();
	          
	// ***WORD			GetVideoSession(CComMode* pComMode, CConfDesc* pComConf);
	
	// Attributes
	//char    m_adHocConfName[H243_NAME_LEN];
	char    m_adHocNumericID[NUMERIC_CONFERENCE_ID_LEN];
	//DWORD   dwAdHocProfileID;
	DWORD  m_rsrvPartyID;
	
	
	
	PDECLAR_MESSAGE_MAP  

};


#endif // !defined(_ReceptionIVR_H__)
