#ifndef CRSRCDETAILGET_
#define CRSRCDETAILGET_

#include "SerializeObject.h"
#include "ResourceProcess.h"
#include "RsrcAlloc.h"
#include "TaskApi.h"
#include "CardsStructs.h"
#include "Macros.h"
#include "SingleToneApi.h"
#include "Trace.h"
#include "InitCommonStrings.h"
#include "SystemResources.h"


#define UNKNOWN_ACTION           -1
#define NOT_FIND                 -1
#define RESET_UNIT_ACTION         0
#define ENABLE_UNIT_ACTION        1
#define DISABLE_UNIT_ACTION       2

#define CLASS_TYPE_NO_UNIT_ACTION 0
#define CLASS_TYPE_UNIT_ACTION    1

enum VideoPlusType
{
	TYPE_NONE,
	TYPE_COP,
	TYPE_COP_PRIMERY,
	TYPE_COP_SECONDARY,
	TYPE_ART,
	TYPE_VIDEO,
};

#define T1_PRI 11
#define E1_PRI 13

////////////////////////////////////////////////////////////////////////////
//                        CCardRsrc
////////////////////////////////////////////////////////////////////////////
class CCardRsrc : public CSerializeObject
{
	CLASS_TYPE_1(CCardRsrc, CSerializeObject)

public:
	                  CCardRsrc();
	                  CCardRsrc(const CCardRsrc& other);
	virtual          ~CCardRsrc();
	const char*       NameOf() const { return "CCardRsrc"; }
	CSerializeObject* Clone()        { return new CCardRsrc; }

	CCardRsrc&        operator=(const CCardRsrc& other);

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action = NULL);

	WORD              GetUnitId () const;
	void              SetUnitId(const WORD unitId);
	BYTE              GetUnitType () const;
	void              SetUnitType(const BYTE unitType);
	BYTE              GetUnitCfg () const;
	void              SetUnitCfg(const BYTE unitCfg);
	DWORD             GetUnitStatus () const;
	void              SetUnitStatus(const DWORD unitStatus);
	void              SetActive(WORD bl);
	void              SetDisabledByError(WORD bl);
	void              SetDisabledManually(WORD bl);
	void              SetDiagnostics(WORD bl);
	WORD              IsActive() const;
	WORD              IsDisabledByError() const;
	WORD              IsDisabledManually() const;
	WORD              IsDiagnostics() const;
	WORD              IsAvailable() const;
	WORD              GetPortsNumber () const;
	void              SetPortsNumber(const WORD portsNum);
	DWORD             GetActivMask1 () const;
	void              SetActivMask1(const DWORD activMask);
	DWORD             GetActivMask2 () const;
	void              SetActivMask2(const DWORD activMask);
	DWORD             GetUtilization () const;
	void              SetUtilization(const DWORD promil);
	BYTE              GetCurrentType () const;
	void              SetCurrentType(const BYTE type);
	DWORD             GetUpdateCounter () const;
	void              IncreaseUpdateCounter ();
	void              SetServiceName( const char* serviceName);

protected:
	WORD              m_unitId;
	BYTE              m_unitType;
	BYTE              m_unitCfg;
	DWORD             m_unitStatus;
	WORD              m_portsNumber;
	DWORD             m_activMask1;
	DWORD             m_activMask2;
	char              m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
	DWORD             m_utilization; //The percentage of this unit which is occupied.
	BYTE              m_currentType; // current unit type
	DWORD             m_UpdateCounter;
};


////////////////////////////////////////////////////////////////////////////
//                        CCommDynCard
////////////////////////////////////////////////////////////////////////////
class CCommDynCard : public CSerializeObject
{
	CLASS_TYPE_1(CCommDynCard, CSerializeObject )

public:
	                  CCommDynCard();
	                  CCommDynCard(const WORD slotNumber);
	                  CCommDynCard(const CCommDynCard& other);
	virtual          ~CCommDynCard();
	const char*       NameOf() const { return "CCommDynCard"; }
	CSerializeObject* Clone()        { return new CCommDynCard; }

	CCommDynCard&     operator=(const CCommDynCard& other);

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, int action);

	int               AddRsrc(const CCardRsrc& other);
	int               FindRsrc(const CCardRsrc& other);
	int               DelRsrc(const CCardRsrc& other);
	CCardRsrc*        GetFirstRsrc();
	CCardRsrc*        GetNextRsrc();
	CCardRsrc*        GetFirstRsrc(int& nPos);
	CCardRsrc*        GetNextRsrc(int& nPos);

	WORD              GetNumUnits()  { return m_numb_of_units; }
	CCardRsrc*        GetCurrentRsrc(const WORD unitId);
	void              SetDisabledByError(WORD bl);
	void              SetDisabledManually(WORD bl);

	BYTE              GetChanged() const;
	DWORD             GetUpdateCounter() const;
	void              IncreaseUpdateCounter();
	void              SetUpdateCounter(DWORD dwUpdateCounter);
	void              SetBoardId(WORD boardID);

protected:
	WORD              m_numb_of_units;
	WORD              m_BoardID;
	CCardRsrc*        m_pCardRsrc[MAX_NUM_OF_UNITS];

private:
	WORD              m_rsrc_ind;
	DWORD             m_updateCounter;
	BYTE              m_bChanged;
};


////////////////////////////////////////////////////////////////////////////
//                        CRsrcDetailGet
////////////////////////////////////////////////////////////////////////////
class CRsrcDetailGet : public CSerializeObject
{
	CLASS_TYPE_1(CRsrcDetailGet, CSerializeObject)

public:
	                  CRsrcDetailGet();
	                  CRsrcDetailGet(const CRsrcDetailGet& other);
	virtual          ~CRsrcDetailGet();
	const char*       NameOf() const { return "CRsrcDetailGet"; }
	CSerializeObject* Clone()        { return new CRsrcDetailGet; }

	CRsrcDetailGet&   operator=(const CRsrcDetailGet& other);

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int               DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);

	CCommDynCard*     GetDynCard();
	void              SetDynCard(CCommDynCard* p_dynCard);
	WORD              GetBoardId();
	DWORD             GetSubBoardId();

protected:
	WORD              m_BoardID;
	DWORD             m_SubBoardID;
	CCommDynCard*     m_DynCard;
};


////////////////////////////////////////////////////////////////////////////
//                        CDetailActivePort
////////////////////////////////////////////////////////////////////////////
class CDetailActivePort : public CSerializeObject
{
	CLASS_TYPE_1(CDetailActivePort, CSerializeObject)

public:
	                   CDetailActivePort();
	                   CDetailActivePort(const CDetailActivePort&);
	virtual           ~CDetailActivePort();
	const char*        NameOf() const { return "CDetailActivePort"; }
	CSerializeObject*  Clone()        { return new CDetailActivePort; }
	CDetailActivePort& operator=(const CDetailActivePort& other);

	void               SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);

	BYTE               IsActive() const;
	void               SetActive(BYTE activeState);

	friend WORD        operator==(const CDetailActivePort&, const CDetailActivePort&);
	friend WORD        operator< (const CDetailActivePort&, const CDetailActivePort&);

	DWORD              m_connectionId;
	BYTE               m_isActive;

protected:
	friend class       CUnitMFA;
	friend class       CSpanRTM;
	WORD               m_portId;
	DWORD              m_confId;
	DWORD              m_partyId;
	DWORD              m_promilUtilized; //The amount of Unit resources utilized by this port
	BYTE               m_porType;
};


////////////////////////////////////////////////////////////////////////////
//                        CRsrcDetailElement
////////////////////////////////////////////////////////////////////////////
class CRsrcDetailElement : public CSerializeObject
{
	CLASS_TYPE_1(CRsrcDetailElement, CSerializeObject)

public:
	                    CRsrcDetailElement();
	                    CRsrcDetailElement(const CRsrcDetailElement&);
	virtual            ~CRsrcDetailElement();
	const char*         NameOf() const { return "CRsrcDetailElement"; }
	CSerializeObject*   Clone()        { return new CRsrcDetailGet; }

	CRsrcDetailElement& operator=(const CRsrcDetailElement& other);

	void                SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                 DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                 DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);

	WORD                getNumPort();
	WORD                getNumActivePort();
	void                setNumPort(WORD num_port );

protected:
	friend class        CUnitMFA;
	friend class        CSpanRTM;

	BYTE                m_cardType; // CARD_EMPTY ,CARD_MCPU ,CARD_HDLC,CARD_PRI_48 ,CARD_PRI_64..
	WORD                m_slotNumber;
	WORD                m_numPorts; // has to be equal m_numActivePorts
	WORD                m_numActivePorts;
	CDetailActivePort** m_activePorts;
};


////////////////////////////////////////////////////////////////////////////
//                        CUnitRsrcDetailGet
////////////////////////////////////////////////////////////////////////////
class CUnitRsrcDetailGet : public CSerializeObject
{
	CLASS_TYPE_1(CUnitRsrcDetailGet, CSerializeObject)

public:
	                    CUnitRsrcDetailGet();
	                    CUnitRsrcDetailGet(const CUnitRsrcDetailGet& other);
	virtual            ~CUnitRsrcDetailGet();
	const char*         NameOf() const { return "CUnitRsrcDetailGet"; }
	CSerializeObject*   Clone()        { return new CUnitRsrcDetailGet; }

	CUnitRsrcDetailGet& operator=(const CUnitRsrcDetailGet& other);

	void                SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                 DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                 DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);

	CRsrcDetailElement* GetDetElmnt();
	WORD                GetBoardId();
	DWORD               GetSubBoardId();
	WORD                GetUnitId();
	void                SetNClassType(int iClassType );

protected:
	WORD                m_BoardID;
	WORD                m_UnitID;
	DWORD               m_SubBoardID;
	CRsrcDetailElement* m_DetElmnt;
	int                 m_iClassType;
};


////////////////////////////////////////////////////////////////////////////
//                        CUnitListRsrcAction
////////////////////////////////////////////////////////////////////////////
class CUnitListRsrcAction : public CSerializeObject
{
	CLASS_TYPE_1(CUnitListRsrcAction, CSerializeObject)

	CUnitListRsrcAction(const CUnitListRsrcAction& other);

public:
	                     CUnitListRsrcAction();
	virtual             ~CUnitListRsrcAction();
	const char*          NameOf() const { return "CUnitListRsrcAction"; }
	CSerializeObject*    Clone()        { return new CUnitListRsrcAction; }

	CUnitListRsrcAction& operator=(const CUnitListRsrcAction& other);

	void                 SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                  DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                  DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
	WORD                 GetNumbOfUnits();
	CUnitRsrcDetailGet*  GetUnitRsrcDetailGet(int nPos);
	CRsrcDetailElement*  GetDetElmnt();

protected:
	WORD                 m_numb_of_units;
	CUnitRsrcDetailGet*  m_CUnitRsrcDetailGet[MAX_UNITS_IN_LIST];
};


////////////////////////////////////////////////////////////////////////////
//                        CRsrcReport
////////////////////////////////////////////////////////////////////////////
class CRsrcReport : public CSerializeObject
{
	CLASS_TYPE_1(CRsrcReport, CSerializeObject)
public:
	                  CRsrcReport();
	                  CRsrcReport(const CRsrcReport&);
	virtual          ~CRsrcReport();
	const char*       NameOf() const  { return "CRsrcReport"; }
	CSerializeObject* Clone()         { return new CRsrcReport; }
	CRsrcReport&      operator=(const CRsrcReport& other);

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

	WORD              GetNumParties(ePartyResourceTypes type, eRPRTtypes rprt_type)                    { return m_numParties[type][rprt_type]; };
	void              SetNumParties(ePartyResourceTypes type, eRPRTtypes rprt_type, WORD num_parties ) { m_numParties[type][rprt_type] = num_parties; };

	void              SetAvailablePortionPPM(DWORD ppm)                                                { m_availablePPM = ppm; }

	friend
	std::ostream&     operator<<(std::ostream& os, const CRsrcReport& val);

protected:
	std::string       GetFloatValueAsString(const float fValue) const;

	WORD              m_numParties[NUM_OF_PARTY_RESOURCE_TYPES][NUM_RPRT_TYPES]; //the array is of size NUM_OF_PARTY_RESOURCE_TYPES, because it's bigger than audio + video (2) for mixed mode
	DWORD             m_availablePPM;
};


////////////////////////////////////////////////////////////////////////////
//                        CIpServiceRsrcReport
////////////////////////////////////////////////////////////////////////////
class CIpServiceRsrcReport : public CRsrcReport
{
	CLASS_TYPE_1(CIpServiceRsrcReport, CRsrcReport)
public:
	                  CIpServiceRsrcReport(const char* pServiceName, WORD service_id);
	                  CIpServiceRsrcReport(const CRsrcReport& other, const char* pServiceName, WORD service_id);
	                  CIpServiceRsrcReport(const CIpServiceRsrcReport& other);
	virtual          ~CIpServiceRsrcReport();
	const char*       NameOf() const         { return "CIpServiceRsrcReport"; }
	CSerializeObject* Clone()                { return new CIpServiceRsrcReport(m_serviceName, m_service_id); }

	const char*       GetServiceName() const { return m_serviceName; };
	WORD              GetServiceId() const   { return m_service_id; };

	friend bool       operator==(const CIpServiceRsrcReport&, const CIpServiceRsrcReport&);
	friend bool       operator< (const CIpServiceRsrcReport&, const CIpServiceRsrcReport&);

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

protected:
	void              SetName(const char* pName);

	char              m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN + 1];
	WORD              m_service_id;
};


////////////////////////////////////////////////////////////////////////////
//                        CServicesRsrcReport
////////////////////////////////////////////////////////////////////////////
class CServicesRsrcReport : public CSerializeObject
{
	CLASS_TYPE_1(CServicesRsrcReport, CSerializeObject)

public:
	                  CServicesRsrcReport();
	                  CServicesRsrcReport(const CServicesRsrcReport& other);
	virtual          ~CServicesRsrcReport();
	const char*       NameOf() const { return "CServicesRsrcReport"; }
	CSerializeObject* Clone()        { return new CServicesRsrcReport; }

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

	void              AddIpServiceReport(const CIpServiceRsrcReport& ipServiceRsrcReport);
	WORD              GetNumOfIpServiceReport() const;
	void              Copy(CServicesRsrcReport& target) const;

protected:
	std::set<CIpServiceRsrcReport>* m_pIpServicesReports;
};


#endif
