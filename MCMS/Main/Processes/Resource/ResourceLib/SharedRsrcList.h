#ifndef SHAREDRSRCLIST_H_
#define SHAREDRSRCLIST_H_

#include "SerializeObject.h"
#include "AllocateStructs.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////
//                        CSharedRsrcList
////////////////////////////////////////////////////////////////////////////
class CSharedRsrcList: public CSerializeObject
{
	CLASS_TYPE_1(CSharedRsrcList, CSerializeObject)

	DISALLOW_COPY_AND_ASSIGN(CSharedRsrcList);

public:
	typedef std::vector<DWORD> SharedRsrcListArray;

	                    CSharedRsrcList();
	virtual            ~CSharedRsrcList();
	const char*         NameOf() const { return "CSharedRsrcList"; }

	void                SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                 DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

	CSerializeObject*   Clone() { return new CSharedRsrcList(); }

	DWORD               GetNumOfConfIds() const { return m_numOfConfIds; }
	DWORD               GetConfIdByIndex(DWORD indx) const;
	SharedRsrcListArray GetConfIdsArray() const { return m_confIdsArray; }

protected:
	SharedRsrcListArray m_confIdsArray;
	DWORD               m_numOfConfIds;
};


////////////////////////////////////////////////////////////////////////////
//                        CSharedRsrcList
////////////////////////////////////////////////////////////////////////////
class CSharedRsrcConfReport : public CSerializeObject
{
	CLASS_TYPE_1(CSharedRsrcConfReport, CSerializeObject)

	DISALLOW_COPY_AND_ASSIGN(CSharedRsrcConfReport);

	struct ConfPortsList
	{
		ConfPortsList() { memset(this, 0, sizeof(*this)); }

		ConfMonitorID   confId;
		WORD            parties[NUM_OF_PARTY_RESOURCE_TYPES][NUM_RPRT_TYPES];

		friend bool     operator<(const ConfPortsList& lhs, const ConfPortsList& rhs) { return (lhs.confId < rhs.confId); }

	};

	typedef std::set<ConfPortsList> Ports;

public:
	                  CSharedRsrcConfReport();
	                  CSharedRsrcConfReport(const CSharedRsrcList* pSharedRsrcList);
	virtual          ~CSharedRsrcConfReport();
	const char*       NameOf() const { return "CSharedRsrcConfReport"; }

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	CSerializeObject* Clone() {return new CSharedRsrcConfReport; }
	void              Init();
	WORD              GetNumConf() const { return m_ports.size(); }
	ConfMonitorID     GetConfIdByIndex(DWORD ind) const;
	WORD              GetNumPartiesByConfID(ConfMonitorID confId, ePartyResourceTypes type, eRPRTtypes rprt_type);
	void              SetNumPartiesByConfID(ConfMonitorID confId, ePartyResourceTypes type, eRPRTtypes rprt_type, WORD num_parties);
	void              SetAvailablePortionPPM(DWORD ppm) { m_availablePPM = ppm; }

protected:
	Ports             m_ports;
	DWORD             m_availablePPM;
};

#endif /* SHAREDRSRCLIST_H_ */
