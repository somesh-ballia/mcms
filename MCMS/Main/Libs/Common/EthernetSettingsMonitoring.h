// EthernetSettingsConfig.h: interface for the CEthernetSettingsConfig class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ETHERNETSETTINGS_H_
#define ETHERNETSETTINGS_H_


#include "PObject.h"
#include "psosxml.h"
#include "CommonStructs.h"


//////////////////////////////
class CEthernetSettingsStructWrapper : public CPObject
{
CLASS_TYPE_1(CEthernetSettingsStructWrapper, CPObject)

public:
	CEthernetSettingsStructWrapper (const bool isStructOnly=false/*contains 'm_isMounted' attribute also*/);
	CEthernetSettingsStructWrapper (const CEthernetSettingsStructWrapper& other);
	virtual ~CEthernetSettingsStructWrapper ();
	const char* NameOf() const {return "CEthernetSettingsStructWrapper";}
	CEthernetSettingsStructWrapper& operator = (const CEthernetSettingsStructWrapper &rOther);

	friend bool operator==(const CEthernetSettingsStructWrapper& first, const CEthernetSettingsStructWrapper& second);    
	friend bool operator==(const CEthernetSettingsStructWrapper& theObject, const ETH_SETTINGS_PORT_DESC_S& theStruct);    
	
	void InitMembers();
	virtual void Dump(std::ostream& msg) const;

	DWORD					GetSlotId() const;
	void					SetSlotId(const DWORD newSlotId);

	DWORD					GetPortId() const;
	void					SetPortId(const DWORD newPortId);

	bool					GetIsMounted() const;
	void					SetIsMounted(const bool isMounted);
	
	ETH_SETTINGS_SPEC_S*	GetEthSettingsStruct() const;
	void					SetEthSettingsStruct(const ETH_SETTINGS_SPEC_S *pOtherStruct);
	
	bool					IsCpuPort();

	void					UpdateMaxCounters();
	void					ClearMaxCounters();

   
protected:
	bool					m_isStructOnly; // (is 'm_isMounted' attribute relevant or not)
	bool					m_isMounted;
	ETH_SETTINGS_SPEC_S*	m_pEthSettingsStruct;
};



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//					CEthernetSettingsStructWrappersList
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class CEthernetSettingsStructWrappersList : public CPObject
{
CLASS_TYPE_1(CEthernetSettingsStructWrappersList, CPObject)

public:
	CEthernetSettingsStructWrappersList ();
	CEthernetSettingsStructWrappersList (const CEthernetSettingsStructWrappersList& other);
	virtual ~CEthernetSettingsStructWrappersList ();
	const char* NameOf() const {return "CEthernetSettingsStructWrappersList";}
	CEthernetSettingsStructWrappersList& operator = (const CEthernetSettingsStructWrappersList &rOther);

	friend bool operator==(const CEthernetSettingsStructWrappersList& first,const CEthernetSettingsStructWrappersList& second);    
	
	void InitMembers();
	virtual void Dump(std::ostream& msg) const;

	CEthernetSettingsStructWrapper* GetSpecEthernetSettingsStructWrapper(int idx) const;
	void SetSpecEthernetSettingsStructWrapper(ETH_SETTINGS_SPEC_S* pNewStruct) const;
	
	bool GetIsMounted(const DWORD slotId, const DWORD portId) const;
	void SetIsMounted(const DWORD slotId, const DWORD portId, const bool isMounted);
//	void SetIsMounted(const DWORD slotId, const DWORD portId, const bool isMounted);

	void UpdateSpecMaxCounters(const DWORD slotId, const DWORD portId);
	void ClearSpecMaxCounters(const DWORD slotId, const DWORD portId);

protected:
	CEthernetSettingsStructWrapper* m_pEthernetSettingsStructWrapperList[MAX_NUM_OF_LAN_PORTS];
};




#endif /*ETHERNETSETTINGS_H_*/
