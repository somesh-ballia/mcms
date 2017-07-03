// +========================================================================+
// BridgeInitParams.H                                                       |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       BridgeInitParams.H                                           |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Matvey                                                       |
// -------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                    |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "BridgeInitParams.h"
#include "HostCommonDefinitions.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgeInitParams
////////////////////////////////////////////////////////////////////////////
CBridgeInitParams::CBridgeInitParams() :
  m_pConf(NULL), m_pConfName(NULL), m_confRsrcId(DUMMY_CONF_ID), m_eBridgeImplementationType(eNoType)
{ }

////////////////////////////////////////////////////////////////////////////
CBridgeInitParams::CBridgeInitParams (const CBridgeInitParams& rBridgeInitParams) :
  CPObject(rBridgeInitParams),
  m_pConf(rBridgeInitParams.m_pConf),
  m_pConfName(rBridgeInitParams.m_pConfName),
  m_confRsrcId(rBridgeInitParams.m_confRsrcId),
  m_eBridgeImplementationType(rBridgeInitParams.m_eBridgeImplementationType)
{ }

////////////////////////////////////////////////////////////////////////////
CBridgeInitParams::CBridgeInitParams (const CConf* pConf, const char* pConfName, ConfRsrcID confRsrcId,
                                      const EBridgeImplementationTypes eBridgeImplementationType) :
  m_pConf(pConf), m_pConfName(pConfName), m_confRsrcId(confRsrcId), m_eBridgeImplementationType(eBridgeImplementationType)
{ }

////////////////////////////////////////////////////////////////////////////
CBridgeInitParams::~CBridgeInitParams ()
{ }

////////////////////////////////////////////////////////////////////////////
CBridgeInitParams& CBridgeInitParams::operator =(const CBridgeInitParams& rBridgeInitParams)
{
  // Operator= is not available for this class because all members are const
  return *this;
}
