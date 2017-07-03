
#ifndef _CVideoRelayOutMediaStream_H_
#define _CVideoRelayOutMediaStream_H_

#include "VideoRelayMediaStream.h"
#include "ScpNotificationWrapper.h"


class CVideoRelayOutMediaStream : public CVideoRelayMediaStream
{
  public:
	CVideoRelayOutMediaStream();
	CVideoRelayOutMediaStream(const CVideoRelayOutMediaStream &other);
	~CVideoRelayOutMediaStream();
	CVideoRelayOutMediaStream& operator=(const CVideoRelayOutMediaStream &other);
	bool operator==(const CVideoRelayOutMediaStream &) const;
	bool operator!=(const CVideoRelayOutMediaStream &) const;
	virtual CRelayMediaStream* NewCopy() const {return new CVideoRelayOutMediaStream(*this);}
	virtual const char* NameOf() const { return "CVideoRelayOutMediaStream";}

	void     SetCsrc(DWORD csrc) {m_csrc = csrc;}
	DWORD    GetCsrc() { return m_csrc; }

	void    SetPriority(int priority){m_priority = priority;}
	int     GetPriority(){ return m_priority;}

	void    SetTemporalScalabilitySupported(bool temporalScalabilitySupported){m_bTemporalScalabilitySupported=temporalScalabilitySupported;}
	bool    GetTemporalScalabilitySupported(){return m_bTemporalScalabilitySupported;}

  // data members
  protected:
	DWORD m_csrc;
	int   m_priority;
	bool  m_bTemporalScalabilitySupported;
};




class CVideoRelayInMediaStream : public CVideoRelayMediaStream
{
public:
    CVideoRelayInMediaStream();
    CVideoRelayInMediaStream(const CVideoRelayInMediaStream &other);
    ~CVideoRelayInMediaStream();
    CVideoRelayInMediaStream& operator=(const CVideoRelayInMediaStream &other);
    bool operator==(const CVideoRelayInMediaStream &) const;
    bool operator!=(const CVideoRelayInMediaStream &) const;
    virtual CRelayMediaStream* NewCopy() const {return new CVideoRelayInMediaStream(*this);}
    virtual const char* NameOf() const { return "CVideoRelayInMediaStream";}

    // data members
public:
    CScpPipeWrapper m_scpPipe;
};


#endif // _CVideoRelayOutMediaStream_H_
