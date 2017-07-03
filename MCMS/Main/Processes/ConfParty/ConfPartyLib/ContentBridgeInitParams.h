//+========================================================================+
//                   ContentBridgeInitParams.H                             |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ContentBridgeInitParams.H                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  Nov-2008  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef CONTENT_BRIDGE_INIT_PARAMS_H_
#define CONTENT_BRIDGE_INIT_PARAMS_H_

#include "BridgeInitParams.h"

class CConf;

class CContentBridgeInitParams : public CBridgeInitParams
{
CLASS_TYPE_1(CContentBridgeInitParams,CBridgeInitParams)
public:
	CContentBridgeInitParams (const CConf* pConf, const char*	pConfName,ConfRsrcID confRsrcId,
								const EBridgeImplementationTypes eBridgeImplementationType,
								BYTE ContentProtocol, BOOL iseExcludeContentMode, BYTE ContentHighProfile);  //HP content

	CContentBridgeInitParams ();
	virtual ~CContentBridgeInitParams ();
	virtual const char*  NameOf() const;

	void SetContentProtocol(ePresentationProtocol ContentProtocol) { m_byCurrentContentProtocol = ContentProtocol; }

	BYTE GetContentProtocol()	const	{ return m_byCurrentContentProtocol; }

	void SetIsExclusiveContentMode(BOOL iseExcludeContentMode) { m_bIseExcludeContentMode = iseExcludeContentMode; }
	BOOL GetIsExclusiveContentMode()	const	{ return m_bIseExcludeContentMode; }

	//HP content
	void SetContentH264HighProfile(BYTE isHighProfile) { m_byH264HighProfile = isHighProfile; }
	BYTE GetContentH264HighProfile()	const	{ return m_byH264HighProfile; }

private:
	CContentBridgeInitParams (const CContentBridgeInitParams&);

	BYTE m_byCurrentContentProtocol;
	BOOL m_bIseExcludeContentMode;
	BYTE m_byH264HighProfile;  //HP content

};

#endif /* CONTENTBRIDGEINITPARAMS_H_ */
