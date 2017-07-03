//+========================================================================+
//                  EpSimH323BehaviorList.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimH323BehaviorList.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                      |
//+========================================================================+

#ifndef __EPSIMH323BEHAVIORLIST_H__
#define __EPSIMH323BEHAVIORLIST_H__


#include "PObject.h"

class CSegment;


#define MAX_H323_BEHAVIORS		50
#define MAX_BEHAVIOR_NAME		250


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CH323Behavior : public CPObject//: public CStateMachine
{
CLASS_TYPE_1(CH323Behavior,CPObject)
public:
		// constructors
	CH323Behavior();
	CH323Behavior(const DWORD nId, const char* pszName);
	virtual const char* NameOf() const { return "CH323Behavior";}
	CH323Behavior(const CH323Behavior& other);
	virtual ~CH323Behavior();
		// overrides

	CH323Behavior&  operator=(const CH323Behavior& other);

	const char*	GetName() const { return m_szName; }
	void	SetName( const char* pszBehaviorName );

	DWORD	GetID() const { return m_nID; }
	void	SetID( const DWORD behaviorID );
	BOOL	IsChanged() const { return m_isChanged; }
	void	ClearChanged() { m_isChanged = FALSE; }

	WORD	GetOverwriteExisting();
	void	SetOverwriteExisting( WORD oe );

	void	OnStartElement( CSegment* pParam );
	void	Serialize(CSegment& rSegment) const;
	void	DeSerialize(CSegment& rSegParam);


protected:
	// methods

protected:
	// attributes
	char	m_szName[MAX_BEHAVIOR_NAME];
	DWORD	m_nID;	// for fast search
	BOOL	m_isChanged;

	DWORD	m_time2RingBack;
	WORD	m_ringBack;
	DWORD	m_time2CallConnected;
	WORD	m_callConnected;
	DWORD	m_time2SendCap;
	WORD	m_sendCap;
	DWORD	m_time2CapResponse;
	WORD	m_capResponse;
	DWORD	m_time2CntrlConnected;
	WORD	m_cntrlConnected;
	DWORD	m_time2IncomingChannel;
	WORD	m_incomingChannel;
	DWORD	m_time2OutgoingCannelResponse;
	WORD	m_outgoingCannelResponse;

protected:
	// behaviors for endpoint
	WORD	m_overwriteExisting;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////








//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CH323BehaviorUpdate : public CH323Behavior
{
public:
	CH323BehaviorUpdate();
	virtual ~CH323BehaviorUpdate();

	// behavior flags for endpoint
	virtual const char* NameOf() const { return "CH323BehaviorUpdate";}
	WORD	m_overwriteExistingYesNo;

protected:
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CH323BehaviorList : public CPObject
{
CLASS_TYPE_1(CH323BehaviorList, CPObject)

public:
		// constructors
	CH323BehaviorList();
	virtual const char* NameOf() const { return "CH323BehaviorList";}
	virtual ~CH323BehaviorList();
		// overrides

	const CH323Behavior* GetDefaultBehavior() const;
	const CH323Behavior* GetCurrentBehavior(const DWORD behaviorID) const;
//	const CH323Behavior* GetCurrentBehavior(const char* behaviorName) const;

	DWORD GetBehaviorListLength() const;
	void GetFullBehaviorListToGui(CSegment* pParam);
	void AddNewBehavior(CSegment* pParam );
	int  GetNextEmptyPlace() const;
	void DeleteBehavior(CSegment* pParam );

protected:
	int		AddBehavior( const CH323Behavior& rNewBehavior );
	int		DelBehavior( char* behaviorName );
	int		UpdateBehavior( char* behaviorName, CH323BehaviorUpdate* updateBehavior );

	DWORD CreateBehaviorID() const;

protected:
	DWORD			m_updateCounter;
	CH323Behavior*	m_paBehaviorArray[MAX_H323_BEHAVIORS];
	int				m_nNumElements;
	WORD			m_nDefaultBehaviorInd;

};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



#endif // __EPSIMH323BEHAVIORLIST_H__ 
