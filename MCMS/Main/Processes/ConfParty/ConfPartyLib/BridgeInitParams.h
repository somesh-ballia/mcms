// +========================================================================+
// BridgeInitParams.CPP                                                     |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       BridgeInitParams.CPP                                         |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Matvey                                                       |
// -------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                    |
// -------------------------------------------------------------------------|
// +========================================================================+

#ifndef _CBridgeInitParams_H_
#define _CBridgeInitParams_H_

#include "Conf.h"
#include "BridgeDefs.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgeInitParams
////////////////////////////////////////////////////////////////////////////
class CBridgeInitParams : public CPObject
{
  CLASS_TYPE_1(CBridgeInitParams, CPObject)

public:
                                   CBridgeInitParams();
                                   CBridgeInitParams(const CConf* pConf, const char* pConfName, ConfRsrcID confRsrcId, const EBridgeImplementationTypes eBridgeImplementationType);
                                   CBridgeInitParams(const CBridgeInitParams&);
  virtual                         ~CBridgeInitParams();

  CBridgeInitParams&               operator =(const CBridgeInitParams& rBridgeInitParams);

  virtual const char*              NameOf() const                      { return "CBridgeInitParams";}
  const CConf*                     GetConf() const                     {return m_pConf;}
  const char*                      GetConfName() const                 {return m_pConfName;}
  ConfRsrcID                       GetConfRsrcId() const               {return m_confRsrcId;}
  void                             SetConfRsrcId(ConfRsrcID confRsrcId){ m_confRsrcId = confRsrcId;}
  EBridgeImplementationTypes 	   GetBridgeImplementationType() const {return m_eBridgeImplementationType;}

private:
  const CConf* const               m_pConf;
  const char* const                m_pConfName;
  ConfRsrcID                       m_confRsrcId;
  const EBridgeImplementationTypes m_eBridgeImplementationType;
};

#endif // ifndef _CBridgeInitParams_H_

