#ifndef _CRelayMediaStream_H_
#define _CRelayMediaStream_H_

#include "PObject.h"

class CRelayMediaStream : public CPObject
{
CLASS_TYPE_1(CRelayMediaStream, CPObject)

  public:
	CRelayMediaStream();
	CRelayMediaStream(const CRelayMediaStream &other);
	~CRelayMediaStream();
	CRelayMediaStream& operator=(const CRelayMediaStream &other);
	bool operator==(const CRelayMediaStream &) const;
	bool operator!=(const CRelayMediaStream &) const;
	virtual CRelayMediaStream* NewCopy() const {return new CRelayMediaStream(*this);}
	virtual const char* NameOf() const { return "CRelayMediaStream";}

	void	SetSsrc(DWORD ssrc) {m_ssrc = ssrc;}
	DWORD	GetSsrc() { return m_ssrc; }

	void	SetIsSpecificSourceCsrc(BOOL bIsSpecific) {m_bIsSpecificSourceCsrc = bIsSpecific;}
	BOOL	GetIsSpecificSourceCsrc() { return m_bIsSpecificSourceCsrc; }

	virtual BOOL  IsValidParams() const{ return true;}

// data members
  protected:
	DWORD m_ssrc;

	//for use in CVideoRelayOutMediaStream
	BOOL  m_bIsSpecificSourceCsrc;
};
#endif // _CRelayMediaStream_H_
