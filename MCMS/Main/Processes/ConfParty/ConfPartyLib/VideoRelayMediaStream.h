#ifndef _CVideoRelayMediaStream_H_
#define _CVideoRelayMediaStream_H_

#include "RelayMediaStream.h"

class CVideoRelayMediaStream : public CRelayMediaStream
{
  // function members
  public:
	CVideoRelayMediaStream();
	CVideoRelayMediaStream(const CVideoRelayMediaStream &other);
	~CVideoRelayMediaStream();
	CVideoRelayMediaStream& operator=(const CVideoRelayMediaStream &other);
	bool operator==(const CVideoRelayMediaStream &) const;
	bool operator!=(const CVideoRelayMediaStream &) const;
	virtual CRelayMediaStream* NewCopy() const {return new CVideoRelayMediaStream(*this);}
	virtual const char* NameOf() const { return "CVideoRelayMediaStream";}

	void	SetLayerId(int id);
	void	SetResolutionHeight(DWORD resHight) {m_resolutionHeight = resHight;}
	void	SetResolutionWidth(DWORD resWidth) {m_resolutionWidth = resWidth;}
	void	SetIsVswStream(BOOL bIsVswStream) {m_bIsVswStream = bIsVswStream;}

	int		GetLayerId() { return m_layerId; }
	DWORD	GetResolutionHeight() { return m_resolutionHeight; }
	DWORD	GetResolutionWidth() { return m_resolutionWidth; }
	virtual BOOL  IsValidParams() const;
	BOOL	IsVswStream() { return m_bIsVswStream; }

  protected:
	int	  m_layerId;
	DWORD m_resolutionHeight;
	DWORD m_resolutionWidth;
	BOOL  m_bIsVswStream;
};
#endif // _CVideoRelayMediaStream_H_
