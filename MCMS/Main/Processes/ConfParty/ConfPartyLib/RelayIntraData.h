/*
 * IntraData.h
 *
 *  Created on: May 2, 2012
 *      Author: bguelfand
 */

#ifndef INTRADATA_H_
#define INTRADATA_H_

#include "PObject.h"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include "Segment.h"
#include "SystemFunctions.h"

class RelayIntraParam : public CPObject
{
	CLASS_TYPE_1(RelayIntraParam, CPObject)

  // function members
  public:
	RelayIntraParam();
	RelayIntraParam(const RelayIntraParam &other);
	virtual ~RelayIntraParam();
	virtual const char* NameOf() const { return "RelayIntraParam"; }
	RelayIntraParam& operator=(const RelayIntraParam &other);
	bool operator==(const RelayIntraParam &) const;
	bool operator!=(const RelayIntraParam &) const;
	void InitDefaults();
	void Serialize(CSegment* pSeg) const;
	void DeSerialize(CSegment* pSeg);
	std::string ToString() const;

  public:
	DWORD m_partyRsrcId;
	bool m_bIsGdr;
	bool m_bIsSsrc;
	std::list <unsigned int>  m_listSsrc;
};

typedef std::list<RelayIntraParam> RelayIntraParams;
class IntraSuppressionParams : public CPObject
{
	CLASS_TYPE_1(IntraSuppressionParams, CPObject)

public:
	IntraSuppressionParams();
	IntraSuppressionParams(unsigned int ssrc, bool isGdr, bool isSsrc);
	virtual const char* NameOf() const { return "IntraSuppressionParams"; }
	void ActivateSuppression(DWORD durationInMilliSeconds);
	void ClearSuppression();
	void SetExpirationTime(DWORD expiration);
	void SetSuppressionFlag(bool isActive);

	unsigned int 		m_ssrc;
	bool				m_bIsGdr;
	bool				m_bIsSsrc;
	bool				m_bIsSuppressionActive;
	bool				m_bIsRequestReceivedDuringSuppression;
	DWORD				m_suppressionExpirationTime;
	//RelayIntraParam*		m_intraRequestParams;
};

typedef std::map<unsigned int, IntraSuppressionParams> SsrcSupressedIntraParams;

class AskForRelayIntra : public CPObject
{
	CLASS_TYPE_1(AskForRelayIntra, CPObject)

  // function members
  public:
	AskForRelayIntra();
	AskForRelayIntra(const AskForRelayIntra &other);
	virtual ~AskForRelayIntra();
	virtual const char* NameOf() const { return "AskForRelayIntra"; }
	AskForRelayIntra& operator=(const AskForRelayIntra &other);
	bool operator==(const AskForRelayIntra &) const;
	bool operator!=(const AskForRelayIntra &) const;
	virtual void InitDefaults();
	virtual const char * GetObjectHumanName() const {return "ask eps for intra request";}
	virtual const char * GetObjectCodeName() const {return "AskForRelayIntra";}
	virtual const char * GetObjectXmlName() const {return "ask_eps_for_intra_request";}
	static const char * GetXmlName() {return "AskForRelayIntra";}

	void Serialize(CSegment* pSeg) const;
	void DeSerialize(CSegment* pSeg);
	std::string ToString() const;

  protected:
	int WriteListToBuffer(unsigned char *buffer) const;
	int ReadListFromBuffer(const unsigned char *buffer);

  public:
	RelayIntraParams m_relayIntraParameters;
	bool m_isImmediately;
	bool m_isAllowSuppression;
};

#endif /* INTRADATA_H_ */
