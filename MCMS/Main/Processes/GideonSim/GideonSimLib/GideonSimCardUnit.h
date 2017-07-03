//+========================================================================+
//                   GideonSimCardUnit.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardUnit.h                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __GIDEONSIMCARDUNIT_
#define __GIDEONSIMCARDUNIT_


#include "StateMachine.h"


/////////////////////////////////////////////////////////////////////////////
//
//   SimBasicUnit - base class (abstract) for all simulation card's units
//
/////////////////////////////////////////////////////////////////////////////

class CSimBasicUnit  : public CStateMachine
{
CLASS_TYPE_1(CSimBasicUnit,CStateMachine )
public:
			// Constructors
	CSimBasicUnit();
	virtual ~CSimBasicUnit();
	virtual const char* NameOf() const { return "CSimBasicUnit";}

			// Initializations
	void* GetMessageMap();

			// Operations
	virtual void DoNothing() const = 0;
	void MplApiMessage(CSegment* pParam);

protected:
			// Action functions
//	void OnMngrConnectSocketIdle(CSegment* pMsg);

protected:
			// Utilities

			// Attributes
	WORD  m_wUnitId;

	PDECLAR_MESSAGE_MAP
};



/////////////////////////////////////////////////////////////////////////////
//
//   SimArtUnit - ART unit simulation
//
/////////////////////////////////////////////////////////////////////////////

class CSimArtUnit  : public CSimBasicUnit
{
CLASS_TYPE_1(CSimArtUnit,CSimBasicUnit )
public:
			// Constructors
	CSimArtUnit();
	virtual ~CSimArtUnit();
	virtual const char* NameOf() const { return "CSimArtUnit";}

			// Initializations
	void* GetMessageMap();

			// Operations
	virtual void DoNothing() const;

protected:
			// Action functions
	// Manager: connect card
//	virtual void OnMngrConnectSocketIdle(CSegment* pMsg);

protected:
			// Utilities

			// Attributes

	PDECLAR_MESSAGE_MAP
};



/////////////////////////////////////////////////////////////////////////////
//
//   SimArtLightUnit - ART-light unit simulation
//
/////////////////////////////////////////////////////////////////////////////

//class CSimArtLightUnit  : public CSimBasicUnit
//{
//public:
//			// Constructors
//	CSimArtLightUnit();
//	virtual ~CSimArtLightUnit();
//
//	virtual const char* NameOf() const { return "CSimArtLightUnit";}
//			// Initializations
//	void* GetMessageMap();
//
//			// Operations
//
//protected:
//			// Action functions
//	// Message received from Socket
//
//protected:
//			// Utilities
//
//			// Attributes
//
//	PDECLAR_MESSAGE_MAP
//};



/////////////////////////////////////////////////////////////////////////////
//
//   SimRtmUnit - RTM unit simulation
//
/////////////////////////////////////////////////////////////////////////////

class CSimRtmUnit  : public CSimBasicUnit
{
CLASS_TYPE_1(CSimRtmUnit,CSimBasicUnit )
public:
			// Constructors
	CSimRtmUnit();
	virtual ~CSimRtmUnit();
	virtual const char* NameOf() const { return "CSimRtmUnit";}

			// Initializations
	void* GetMessageMap();
	virtual void DoNothing() const;

			// Operations

protected:
			// Action functions
	// Socket: connection established
//	virtual void OnSocketConnectedSetup(CSegment* pMsg);

protected:
			// Utilities

			// Attributes

	PDECLAR_MESSAGE_MAP
};


/////////////////////////////////////////////////////////////////////////////
//
//   SimVideoUnit - video decoder unit simulation
//
/////////////////////////////////////////////////////////////////////////////

class CSimVideoUnit  : public CSimBasicUnit
{
CLASS_TYPE_1(CSimVideoUnit,CSimBasicUnit )
public:
			// Constructors
	CSimVideoUnit();
	virtual ~CSimVideoUnit();
	virtual const char* NameOf() const { return "CSimVideoUnit";}

			// Initializations
	void* GetMessageMap();
	virtual void DoNothing() const;

			// Operations

protected:
			// Action functions
	// Message received from Socket

protected:
			// Utilities

			// Attributes

	PDECLAR_MESSAGE_MAP
};


#endif /* __GIDEONSIMCARDUNIT_ */
