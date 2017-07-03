// HwMonitoring.h: interface for the HwMonitoring classes
//
//////////////////////////////////////////////////////////////////////

#ifndef _HwMonitoring_H_
#define _HwMonitoring_H_


#include "PObject.h"
#include "psosxml.h"
#include "CardsStructs.h"
#include "CardsProcess.h"


using namespace std;

class CFaultCardDesc;



/////////////////////////////////////////////////////////////////////////////
// CCommCard 

class CCommCard : public CPObject
{
	CLASS_TYPE_1(CCommCard, CPObject)
public:
	   //Constructors
    CCommCard();                                                                            
    CCommCard( const eCardType  type, const char* serialNum,
               const WORD boardId, const WORD subBoardId, const WORD displayBoardId,
               const VERSION_S  hardVersion, const CM_SW_VERSION_S* softVersionList,
               const WORD isSimulation=SIMULATION_NO);
    CCommCard(const CCommCard &other);
    virtual ~CCommCard();
	CCommCard& operator =(const CCommCard &other);
	virtual const char* NameOf() const { return "CCommCard";}
	
	
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	
    int    TestValid() const;                 
    void   SetBoardId(const WORD slotNumber);                 
    WORD   GetBoardId () const;                 
    void   SetSubBoardId(const WORD slotNumber);                 
    WORD   GetSubBoardId () const;                 
    void   SetDisplayBoardId(const WORD slotNumber);                 
    WORD   GetDisplayBoardId () const;                 
    void   SetOnlyType(const eCardType  type);                 
    virtual void SetType(const eCardType  type);                 
    eCardType   GetType () const;                 
    void   SetHardVersion(const VERSION_S  hardVersion);                 
    const  VERSION_S  GetHardVersion () const;                 
//    void   SetSoftVersion(const VERSION_S  softVersion);                 
//    const  VERSION_S  GetSoftVersion () const;                 
    void   SetSerialNumber(const char* serialNumber);                 
    char*  GetSerialNumber ();                 
	
	/*
    void            SetH323Spec(const CCommNetCard&  other);
    CCommH323Card*  GetH323Spec();
	*/
	
protected:
    WORD              m_boardId;                   // boardId number
    WORD              m_subBoardId;                // subBoardId number
    WORD              m_displayBoardId;
    eCardType         m_cards_type;                // cards type              
    VERSION_S         m_hardVersion;               // Hardware version
	char              m_serialNumber[MPL_SERIAL_NUM_LEN]; // serial number
    WORD              m_simul;                     // simulations mode
	CM_SW_VERSION_S   m_swVersionsList[MAX_NUM_OF_SW_VERSIONS];
};


/////////////////////////////////////////////////////////////////////////////
// CCommDynCard 

class CCommDynCard : public CCommCard
{
	CLASS_TYPE_1(CCommDynCard,CCommCard )
public:
    CCommDynCard();                                                                            
    CCommDynCard( const eCardType type, const char* serialNum,
                  const WORD boardId, const WORD subBoardId, const WORD displayBoardId,
                  const VERSION_S hardVersion, const CM_SW_VERSION_S* softVersionList,
                  const WORD isSimulation=SIMULATION_NO);
    CCommDynCard(const CCommDynCard &other);
	virtual ~CCommDynCard();
	CCommDynCard& operator =(const CCommDynCard &other);
	virtual const char* NameOf() const { return "CCommDynCard";}
	
	virtual void SerializeXml(CXMLDOMElement* pFatherNode);
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	void SerializeXmlStatuses(CXMLDOMElement* pFatherNode/*, DWORD ObjToken*/);
	int  DeSerializeXmlStatuses(CXMLDOMElement *pActionNode,char *pszError);
	
    eCardState   GetState() const;                 
    void   SetState(const eCardState  state);                 
    void   SetState();                 
    WORD   GetNumStatus() const;                 
//    WORD   GetNumConf() const;                 
    CFaultCardDesc*  GetFirstStatus() ;
    CFaultCardDesc*  GetNextStatus() ;
	
	
    int    AddStatus(const CFaultCardDesc&  other);                 
    int    AddStatusInStartup(const CFaultCardDesc&  other);                 
    int    FindStatus(const CFaultCardDesc &other);  
//    int    DelStatus(const CFaultCardDesc&  other);                 
//    int    DelStatus();
    BYTE   GetGlobalStatus() const;                 
    BYTE   IsCardConf() const;                 
    char*  GetUnitAsString(const CFaultCardDesc &other);                 


/*
#ifdef __HIGHC__
	BYTE   GetIsT1casCard() const;
	BYTE   GetNetXCardConfiguration() const;
	BYTE   GetIsContainService(const char* serviceName) const;
	void   SetCardSoftwareType(BYTE t) {m_cardSoftwareType = t;}    // Unknown  - 0; ISDN - 1; T1CAS - 2
	BYTE   GetCardSoftwareType() const {return m_cardSoftwareType;} // Unknown  - 0; ISDN - 1; T1CAS - 2
#endif __HIGHC__
*/    

    WORD GetNumUnits(){return m_numb_of_units;}
    void   SetDisabledByError(WORD bl);
	void   SetDisabledManually(WORD bl);
	
	BYTE	GetChanged() const;
	DWORD	GetUpdateCounter() const;
	void	IncreaseUpdateCounter();
	void	SetUpdateCounter(DWORD dwUpdateCounter);

	//NEW_STATUS_LIST
	void AddStatus( const BYTE subject, const DWORD errorCode, const BYTE errorLevel,
	                string description, const DWORD userId=0xFFFFFFFF,
	                const DWORD unitId=0, const WORD theType=0 );
	void DelStatusByErrorCode(const DWORD errorCode);
	void DelStatusByErrorCodeUserId(const DWORD errorCode, const DWORD userId);

	
protected:
//	void InformSnmpStatusChange(WORD m_state, CFaultCardDesc* fault= NULL);
	
	eCardState m_state;
    WORD       m_numb_of_status;     
    WORD       m_numb_of_conf;       
    WORD       m_numb_of_units;       
    CFaultCardDesc*   m_pCardStatus[MAX_STATUS_IN_LIST];
	
	//NEW_STATUS_LIST
	CFaultList  *m_StatusList;

private:
    WORD  m_status_ind;
    WORD  m_conf_ind;
    WORD  m_rsrc_ind;
	
	//for differences mechanism
	DWORD m_updateCounter;
	BYTE  m_bChanged;
	
/*
#ifdef __HIGHC__
	// for Net8 (ISDN/T1cas)
	BYTE m_cardSoftwareType; // Unknown  - 0; ISDN - 1; T1CAS - 2
#endif __HIGHC__
*/
};


/////////////////////////////////////////////////////////////////////////////
// CCommCardDB 

class CCommCardDB : public CPObject
{
	CLASS_TYPE_1(CCommCardDB,CPObject )
public:                           
    CCommCardDB();                   
    CCommCardDB(const CCommCardDB &other);
    virtual ~CCommCardDB();
	virtual const char* NameOf() const { return "CCommCardDB";}
	
	void SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	CCommCardDB&  operator=( const CCommCardDB& other );

    int    Add(const CCommDynCard &other);                 
    int    Update(const CCommDynCard &other);                 
    int    Cancel(const WORD boardId, const WORD subBoardId);                                      
    int    FindSlot(const CCommDynCard &other);  
    int    FindSlot(const WORD boardId, const WORD subBoardId);
    int    GetIdx(const CCommDynCard&  other);

    const  CCommDynCard*  GetCurrentCard(const WORD boardId, const WORD subBoardId);

    int    AddStatusInStartup(const CFaultCardDesc&  other, const WORD boardId, const WORD subBoardId);                 
    int    AddStatus( const BYTE  subject, const DWORD error_code, const BYTE  faultLevel,
                      const WORD boardId, const WORD subBoardId, const WORD unitId, string description, const WORD theType);
    int    AddStatus(const CFaultCardDesc&  other, const WORD boardId, const WORD subBoardId);                 
	int    DelStatus(const BYTE  subject, const DWORD error_code, const BYTE  faultLevel, const WORD boardId, const WORD unitId=0);
    int    DelStatus(const CFaultCardDesc&  other, const WORD boardId, const WORD subBoardId);


    int    AddStatusNew( const BYTE  subject, const DWORD error_code, const BYTE  faultLevel,
                         DWORD userId, const WORD boardId, const WORD subBoardId, const WORD unitId, string description, const WORD theType );
	int    DelStatusNewByErrorCode(const DWORD error_code, const WORD boardId, const WORD subBoardId);
	int    DelStatusNewByErrorCodeUserId(const DWORD error_code, const DWORD userId, const WORD boardId, const WORD subBoardId);


	STATUS           SetCardState(WORD boardId, WORD subBoardId, const eCardState cardState);
	eCardState GetCardState(WORD boardId, WORD subBoardId);
	eCardType GetCardType(WORD boardId, WORD subBoardId);
	STATUS SetCardType(WORD boardId, WORD subBoardId, const eCardType cardType);
	
    WORD   GetState();                 

	WORD GetNumOfNotEmptySlots() const;
	WORD GetNumOfMediaBoards() const;
    bool IsAnyMediaBoardExistsInDB();
    bool IsAnyCardExistsInDB();
    bool IsSlotEmpty(WORD boardId, WORD subBoardId);
    
	
//    void InformSnmpChange();
    void  IncreaseUpdateCounter();
    BYTE  GetChanged() const;
    void  SetUnAnswerdKA(WORD boardId,DWORD unAnswerdKA);

	// add treatment for SubBoards!!
    CCommDynCard*   m_pCard[MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS];
	
	
	WORD       m_consist;  //consistent flag for file on disk
	
	BYTE	   m_bChanged;
	DWORD	   m_updateCounter;
	DWORD	   m_unAnswerdKA[MAX_NUM_OF_BOARDS];

private:	
	CCardsProcess*  m_pProcess;
};


/*
// CCardRsrc

class CCardRsrc : public CPObject
{
CLASS_TYPE_1(CCardRsrc, CPObject)
public:
	   //Constructors
	CCardRsrc();         
	CCardRsrc(const CCardRsrc &other);
	virtual const char* NameOf() const { return "CCardRsrc";}
	CCardRsrc& operator =(const CCardRsrc &other);
	virtual ~CCardRsrc();


	   // Implementation
  void    Serialize(WORD format, COstrStream  &m_ostr, DWORD apiNum = 0);     
  void    DeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum = 0);

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

  WORD    GetUnitId () const;                 
  void    SetUnitId(const WORD unitId);                 
  BYTE    GetUnitType () const;                 
  void    SetUnitType(const BYTE unitType);                 
  BYTE    GetUnitCfg () const;                 
  void    SetUnitCfg(const BYTE unitCfg);                 
  DWORD   GetUnitStatus () const;                 
  void    SetUnitStatus(const DWORD unitStatus);                 
  void    SetActive(WORD bl); 
  void    SetDisabledByError(WORD bl);
  void    SetDisabledManually(WORD bl); 
  void    SetDiagnostics(WORD bl); 
  WORD    IsActive() const;                 
  WORD    IsDisabledByError() const;                 
  WORD    IsDisabledManually() const;                 
  WORD    IsDiagnostics() const;                 
  WORD	  IsAvailable() const;
  WORD    GetPortsNumber () const;                 
  void    SetPortsNumber(const WORD portsNum);                 
  DWORD   GetActivMask1 () const;                 
  void    SetActivMask1(const DWORD activMask);                 
  DWORD   GetActivMask2 () const;                 
  void    SetActivMask2(const DWORD activMask);                 
  void    SetName(const char*  name);                 
  const char*  GetName () const;                 
  DWORD   GetUtilization () const;                 
  void    SetUtilization(const DWORD promil);    
  BYTE    GetCurrentType () const;                 
  void    SetCurrentType(const BYTE type);   
  DWORD   GetUpdateCounter () const;
  void    IncreaseUpdateCounter ()  ;


protected:
	 // Attributes
  WORD   m_unitId;
  BYTE   m_unitType;
  BYTE   m_unitCfg;
  DWORD  m_unitStatus;
  WORD   m_portsNumber;
  DWORD  m_activMask1;
  DWORD  m_activMask2;
  char   m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN];  
  DWORD  m_utilization; //The percentage of this unit which is occupied.
  BYTE   m_currentType; // current unit type
  DWORD m_UpdateCounter;
 };



/////////////////////////////////////////////////////////////////////////////
// CCardConf 

class CCardConf : public CPObject
{
CLASS_TYPE_1(CCardConf, CPObject)
public:
	   //Constructors
	CCardConf();         
	CCardConf(const CCardConf &other);
	virtual const char* NameOf() const { return "CCardConf";}
	virtual ~CCardConf();


	   // Implementation
  void    Serialize(WORD format, COstrStream  &m_ostr ,DWORD apiNum);     
  void    DeSerialize(WORD format, CIstrStream &m_istr ,DWORD apiNum);

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

  void    SetNameConf(const char*  nameConf);                 
  const char*   GetNameConf() const;                 
  DWORD  GetConferenceId () const;                 
  void   SetConferenceId(const DWORD confId);                 

protected:
	 // Attributes
  char   m_name_conf[H243_NAME_LEN];
  DWORD   m_confId;
};



/////////////////////////////////////////////////////////////////////////////
// CUnitStr

class CUnitStr : public CPObject
{
CLASS_TYPE_1(CUnitStr, CPObject)
public:
	   //Constructors
	CUnitStr();         
	virtual ~CUnitStr();
	virtual const char* NameOf() const { return "CUnitStr";}


	   // Implementation
  void    Serialize(WORD format, COstrStream  &ostr);     
  void    DeSerialize(WORD format, CIstrStream &istr);

  void    SerializeSpaces(char*);     
  void    DeSerializeSpaces(char*);
  void    SerializeSpaces(COstrStream  &ostr);     
  void    DeSerializeSpaces(CIstrStream &istr);
  void SerializeXml(CXMLDOMElement* pFatherNode);
  int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

  void    Serialize(char* userAllocatedBuff);     
  void    DeSerialize(char*);
  WORD    GetUnitId () const;                 
  void    SetUnitId(const WORD unitId);                 
  WORD    GetSlotNumb () const;                 
  void    SetSlotNumb(const WORD slot);                 


protected:
	 // Attributes
  WORD   m_unitId;
  WORD   m_slot;
};


/////////////////////////////////////////////////////////////////////////////
// CUnitList 

class CUnitList : public CPObject
{
CLASS_TYPE_1(CUnitList,CPObject )
public:                           
	   //Constructors
    CUnitList();                   
    CUnitList(const CUnitList &other);
	virtual const char* NameOf() const { return "CUnitList";}
    virtual ~CUnitList();


	   // Implementation
    void   Serialize(WORD format, COstrStream  &m_ostr);     
    void   DeSerialize(WORD format, CIstrStream &m_istr);    
	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);


    int    Add(const CUnitStr &other);                 
    int    Cancel(const CUnitStr &other);                                      
    int    FindUnit(const CUnitStr &other);  
    WORD   GetNumUnits(){return m_numb_of_units;}
    CUnitStr*  GetFirstUnit() ;
    CUnitStr*  GetNextUnit() ;
    const CUnitStr* operator[](WORD) const;
    CUnitStr* operator[](WORD);

	   // Attributes
    WORD        m_numb_of_units;   
    CUnitStr*   m_pUnit[MAX_UNITS_IN_LIST];
private:
    WORD  m_unit_ind;
};


/////////////////////////////////////////////////////////////////////////////
// CClockUnitStr

class CClockUnitStr : public CUnitStr
{
CLASS_TYPE_1(CClockUnitStr, CUnitStr)
public:
	   //Constructors
	CClockUnitStr();         
	virtual ~CClockUnitStr();
	virtual const char* NameOf() const { return "CClockUnitStr";}


	   // Implementation
  void    Serialize(WORD format, COstrStream  &ostr);     
  void    DeSerialize(WORD format, CIstrStream &istr);
  void SerializeXml(CXMLDOMElement* pFatherNode);
  int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

  void    SerializeSpaces(char*);     
  void    DeSerializeSpaces(char*);
  void    SerializeSpaces(COstrStream  &ostr);     
  void    DeSerializeSpaces(CIstrStream &istr);

  void    Serialize(char* userAllocatedBuff);     
  void    DeSerialize(char*);
  void    SetMaster();
  BYTE    IsMaster();
  void    SetBackup();
  BYTE    IsBackup();


protected:
	 // Attributes
  WORD   m_wMaster;
};


*/
/////////////////////////////////////////////////////////////////////////////
#endif /* _HwMonitoring_H_ */
