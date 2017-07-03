// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:             PlcmDNS_TTLService.h  
// Include line recommended:
// #include "PlcmDNS_TTLService.h"         //
// 
// ==========================================

#ifndef PLCMDNS_TTLSERVICE_H_
#define PLCMDNS_TTLSERVICE_H_

#include "StateMachine.h"
#include "Segment.h"
#include "PlcmDNS_Defines.h"
#include "PlcmDNS_Tools.h" 


//====================================================================//

typedef struct _TTL_NODE
{
	char				szHostName[PLCM_DNS_HOST_NAME_SIZE]	;
	unsigned short		wServiceID							;
	unsigned int		dwTTL								;
	unsigned int		dwTimeStamp							;
	unsigned int		dwUpdateTime                        ;
	unsigned short		eDnsReqResType						;//see eDNS_TYPE_A_IPv4
}
TTL_NODE;

class cTTL_NODE : public CPObject
{
	CLASS_TYPE_1(cTTL_NODE, CPObject)
public:
	virtual const char*  NameOf() const {return "cTTL_NODE";}
			cTTL_NODE    (char * par_szHostName, unsigned short	par_wSerID, unsigned int par_dwTTL, WORD par_wReqType);
	virtual ~cTTL_NODE   ();

	void		vClear	 ();
	string *	sDump	 (string * par_pString);

public:
	//------ TTL DATA -----------------------------------// 
	TTL_NODE			m_sTTLNode;
};
//====================================================================//

//====================================================================//
typedef std::vector <cTTL_NODE*> DNS_TTL_VECTOR;  
typedef BOOL (*DnsTTLListAuditFun) (cTTL_NODE * par_pRecord, int par_nDataType, void * par_pData);

class cTTLList :  public CPObject
{
	CLASS_TYPE_1(cTTLList, CPObject)
public: 
	virtual const char*  NameOf() const {return "cTTLList";}
public:

			cTTLList ();
	virtual ~cTTLList();

	BOOL				 bInsert(cTTL_NODE * par_pNode);
	BOOL				 bDelete(cTTL_NODE * par_pNode);
	BOOL				 bDelete(char * par_szHostName, unsigned short par_wServiceID);

	cTTL_NODE          * Find(char * par_szHostName, unsigned short par_wServiceID);
	BOOL				 bClean();
	unsigned int		 dwGetSize();

	void                 vListDump(string * par_pString);

    cTTL_NODE          * pGetNearestNode();
	void				 vNearestNode_Dump(unsigned int dwCurrntTime, unsigned int dwNearestTime, cTTL_NODE * pNode, char * par_szPrefix);

	BOOL				 DnsReqListAudit(DnsTTLListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData);
	BOOL				 DnsReqListAuditBreak(DnsTTLListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData);// First return from "par_pAuditFunction" - is break from loop

protected:
	PLc_MUTEX			m_Mutex;
	DNS_TTL_VECTOR		m_TTLList;
};
//====================================================================//


//====================================================================//
class cTTLService : public CStateMachine
{
	CLASS_TYPE_1(cTTLService, CStateMachine)
public:
	virtual const char*  NameOf() const {return "cTTL_NODE";}
			cTTLService   ();
	virtual ~cTTLService  ();

	void				 vClean	();
	BOOL				 bInsert(char * par_szHostName, unsigned short par_wServiceID, unsigned int par_dwTTL, unsigned short par_wReqType);
	BOOL				 bDelete(char * par_szHostName, unsigned short par_wServiceID);
	cTTL_NODE          * Find   (char * par_szHostName, unsigned short par_wServiceID);

	void				 vNearestUpdate ();

	void				 vServiceDump   (char * par_szPrefix);

	void				 vTimerArrived	(CSegment* par_pParam);

	void				 vTimerAwake    (unsigned int par_TimeOut_Sec);//for TTL_SERVICE_TIMER
	void				 vTimerStop     ();							   //for TTL_SERVICE_TIMER  

public:
	cTTLList			m_cTTLList;
	BOOL				m_bIsTimerOrdered;

	cTTL_NODE	*		m_pNearestNode   ;


protected:
	PDECLAR_MESSAGE_MAP;
};
//====================================================================//




#endif //PLCMDNS_TTLSERVICE_H_

