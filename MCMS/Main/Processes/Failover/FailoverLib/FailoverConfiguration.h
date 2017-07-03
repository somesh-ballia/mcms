/*
 * FailoverConfiguration.h
 *
 *  Created on: Aug 26, 2009
 *      Author: yael
 */

#ifndef FAILOVERCONFIGURATION_H_
#define FAILOVERCONFIGURATION_H_

#include "SerializeObject.h"
#include "FailoverProcess.h"
#include "FailoverDefines.h"

struct FAILOVER_EVENT_INFO
{
	DWORD eventType;
	WORD bEnabled;
	std::string szDescription;
};

class CFailoverConfiguration : public CSerializeObject
{
	CLASS_TYPE_1(CFailoverConfiguration, CSerializeObject)
public:
	CFailoverConfiguration();
	~CFailoverConfiguration();
	CFailoverConfiguration& operator=(const CFailoverConfiguration&);
	virtual const char*  NameOf() const {return "CFailoverConfiguration";}
	CSerializeObject* Clone() {return new CFailoverConfiguration;}
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	//void   SerializeXml(CXMLDOMElement*& pFatherNode,DWORD ObjToken);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	BOOL GetIsEnabled()const {return m_bIsEnabled;}
	BOOL GetManualTrigger()const {return m_bManualTrigger;}
	std::string GetOtherRmxIp()const {return m_strOtherRmxIp;}
	int GetOtherRmxPort()const {return m_strOtherRmxPort;}
	DWORD GetHotBackupType()const {return m_HotBackupType;}
    void SetMasterSlaveState(eFailoverStatusType eStatusType) {m_statusType=eStatusType;}
	DWORD GetMasterSlaveState(){return m_statusType;}
    void SetHotBackupType(DWORD hotBackupType) {m_HotBackupType = hotBackupType;}
	map<DWORD,FAILOVER_EVENT_INFO> GetEventList() const {return m_mapEventTrigger;}

private:

    //CFailoverConfiguration(const CFailoverConfiguration&);
	//std::string m_strOtherRmxIp;
	DWORD m_HotBackupType;
    BOOL m_bIsEnabled;
    char  m_strOtherRmxIp[IP_ADDRESS_STR_LEN];
    WORD m_strOtherRmxPort;
    DWORD m_statusType;
	BOOL m_bManualTrigger;
	
	std::map<DWORD,FAILOVER_EVENT_INFO> m_mapEventTrigger;
};

#endif /* FAILOVERCONFIGURATION_H_ */
