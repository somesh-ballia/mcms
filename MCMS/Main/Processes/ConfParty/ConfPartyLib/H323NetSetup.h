//+========================================================================+
//                         H323NetS.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323NetS.h                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Mishel/Uri                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
//                                      CNetCallingParty
//                                      CNetCalledParty
//                                      CNetCause
//                                      CNetProgress 
//                                      CNetOpenDchnl
//                                      CNetInitUnitHardw 
//                                      CNetStatusAlarm 
//                                      CNetStatusDchnl
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _H323NETSETUP
#define _H323NETSETUP

#include "IpNetSetup.h"
#include "H323Alias.h"
#include "H323CsInd.h"
#include "ConfPartySharedDefines.h"

//#ifndef _H323EMB_H
//#include  <h323emb.h>
//#endif


class CH323NetSetup : public CIpNetSetup
{
CLASS_TYPE_1(CH323NetSetup,CIpNetSetup )
public:             
							// Constructors
	CH323NetSetup();
	~CH323NetSetup();  
	virtual const char* NameOf() const { return "CH323NetSetup";}
							// Initializations  

                            // Serialization
	virtual void Serialize(WORD format,CSegment& seg);
	virtual void DeSerialize(WORD format,CSegment& seg);
	virtual CIpNetSetup& operator=(const CIpNetSetup& other);
	virtual CH323NetSetup& operator=(const CH323NetSetup& other);
	virtual void copy(const CNetSetup * rhs); 

	virtual void  SetSrcPartyAddress(const char * strAddress); 
	virtual void  SetDestPartyAddress(const char * strAddress); 

	void  SetH245Address(); 
	WORD  GetH245Port()	const { return m_h245Address.port; }
	DWORD GetH245Ip() const	{ return m_h245Address.addr.v4.ip; }
	void  SetH323PartyAlias(const char*  h323PartyAlias);                 
	CH323Alias * GetH323FirstE164PartyAlias ();
	void  SetH323HighestPriorityPartyAlias ();
	const char * GetH323PartyAlias () const;         
	void  SetH323PartyAliasType(WORD h323Type); 
	DWORD GetH323PartyAliasType();
	void  SetVariabels(const mcIndCallOffering& pCallOfferingInd);

	// Set and Get function for the CH323NetSetup attributes
	char* GetH323userUser()				{ return m_sH323userUser;		}
	WORD  GetuserUserSize()				{ return m_userUserSize;		}
	DWORD GetConferenceGoal()			{ return m_conferenceGoal;		}
//	DWORD GetModel()					{ return m_callModel;			}
	WORD  GetbIsActiveMc()				{ return m_bIsActiveMc;			}
	WORD  GetbIsOrigin()				{ return m_bIsOrigin;			}
	mcTransportAddress*  Geth245Address()  { return &m_h245Address;		}
	WORD  GetbH245Establish()			{ return m_bH245Establish;		}
	DWORD GetConnectionType()			{ return m_connectionType;		}
	CH323Alias* GetSrcPartyAliasList (int *numOfSrcAlias);
	CH323Alias* GetDestPartyAliasList(int *numOfDestAlias);
	void  GetH323srcPartyTA(char* sPartyTA);
    DWORD GetCallReferenceValue();
//	PartyDataStruct* DeSerializeStringToStruct();

	//temporery for check proshare
	void  SetH323ConfIdAsGUID(const char* confId) { memcpy(m_conferenceId, confId, MaxConferenceIdSize);}
	const char* GetH323ConfIdAsGUID()			{ return m_conferenceId;		}
	void  SetLocalH225Port(WORD H225Port)	{ m_localH225Port = H225Port;	}
	WORD  GetLocalH225Port()				{ return m_localH225Port;		}
	const char* GetDialInConfId()			{ return m_conferenceId;		}
	void  SetCallId(const char* callId)     { memcpy(m_callId, callId, MaxConferenceIdSize); }
	const char* GetCallId()					{ return m_callId;				}

	DWORD GetGenerator() const {return m_generator;}
	DWORD GetHkLen() const {return m_hkLen;}
	WORD  GetHkType() const {return m_hkType;}
	BYTE *GetHalfKey() const {return m_pHalfKey;}
	const char* getBestH323PartyAlias() const {return m_bestH323PartyAlias;}

	//Multiple links for ITP in cascaded conference feature:
	const char* getH323userUser() const {return m_sH323userUser;}

	// IpV6
	void  SetSrcPartyAliases(const char* srcPartyAlias);
	virtual void  SetDestPartyAliases(const char* destPartyAlias);
	const char * GetH323SrcPartyAliases () const;
	const char * GetH323DestPartyAliases () const;
	
	protected:
	void SetDwordIp(BYTE bIsSrc);
	void InitComponnentInList(char* pRearPtr, CH323Alias* pH323Alias);
	virtual const char * GetStartOfAddress(const char * strAddress) const;

	//patch michel
	char    *m_pH323PointerToAlias;
	char    m_bestH323PartyAlias[ALIAS_NAME_LEN+1]; // to be used for SiteName/VisualName
	char	m_h323PartyAlias[IP_STRING_LEN];
	WORD    m_h323PartyAliasType;//[MaxNumberOfAliases];
	char	m_sH323userUser[MaxUserUserSize]; //Multiple links for ITP in cascaded conference feature - not patch!
	WORD	m_userUserSize;
	// IpV6
	char	m_srcH323PartyAliases[MaxAddressListSize];
	char	m_destH323PartyAliases[MaxAddressListSize];
	
	// attribute for H323 use

	DWORD	m_conferenceGoal;	// create || envite || join
	DWORD	m_connectionType;	// P2P || One2N || N2N || N2One	
//	DWORD	m_callModel;		// Direct or Gatekeeper routed
	WORD	m_bIsActiveMc;
	WORD	m_bIsOrigin;
	WORD	m_bH245Establish; 
	mcTransportAddress		m_h245Address; 				

	WORD	m_localH225Port;

	//to connect with the remote EP in dial in
	char  m_conferenceId[MaxConferenceIdSize];				
	char  m_callId[MaxConferenceIdSize];	
	DWORD m_callReferenceValue;	

	DWORD	m_generator;
	DWORD	m_hkLen;
	WORD	m_hkType;
	BYTE	*m_pHalfKey;								 
};


#endif //_H323NETSETUP
