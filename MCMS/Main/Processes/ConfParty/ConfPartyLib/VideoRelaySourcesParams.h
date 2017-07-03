#ifndef VIDEORELAYSOURCESPARAMS_H_
#define VIDEORELAYSOURCESPARAMS_H_

#include "PObject.h"
#include <iostream>
#include "VideoRelaySourceApi.h"
#include <list>
#include <sstream>


class CVideoRelaySourcesParams : public CPObject
{
CLASS_TYPE_1(CVideoRelaySourcesParams, CPObject)

  // function members
  public:
	CVideoRelaySourcesParams();
	virtual ~CVideoRelaySourcesParams();
	CVideoRelaySourcesParams(const CVideoRelaySourcesParams &other);
    virtual const char* NameOf() const { return "CVideoRelaySourcesParams";}

    CVideoRelaySourcesParams& operator=(const CVideoRelaySourcesParams &other);
	bool operator==(const CVideoRelaySourcesParams &) const;
	bool operator!=(const CVideoRelaySourcesParams &) const;

	virtual CVideoRelaySourcesParams* NewCopy() const {return new CVideoRelaySourcesParams(*this);}
	virtual CVideoRelaySourcesParams* NewEmpty() const {return new CVideoRelaySourcesParams();}
	virtual void InitDefaults();
	virtual bool IsTheSame(const CVideoRelaySourcesParams& base) const;

	std::list <CVideoRelaySourceApi>& GetVideoSourcesList() {return m_videoSources;}
	DWORD GetChannelHandle()const { return m_channelHandle;}
	DWORD GetSeqNum()const { return m_seqNum;}
	DWORD GetSourceOperationPointSetId()const {return m_sourceOperationPointSetId;}
	int   GetNumSources();

	void SetChannelHandle(DWORD channelHandle){  m_channelHandle = channelHandle;}
    void SetSeqNum(DWORD seqNum){ m_seqNum = seqNum;}
    void SetSourceOperationPointSetId(DWORD id){ m_sourceOperationPointSetId = id;}
    void Dump()const;

    bool IsVideoRelaySourcesHasSource(unsigned int ssrc);
  // data members
  protected:
	DWORD m_channelHandle;
	std::list <CVideoRelaySourceApi>  m_videoSources;
	DWORD m_seqNum;
	DWORD m_sourceOperationPointSetId;
};



#endif /* VIDEORELAYSOURCESPARAMS_H_ */
