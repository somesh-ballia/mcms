#ifndef VIDEORELAYSOURCEAPI_H_
#define VIDEORELAYSOURCEAPI_H_

#include "PObject.h"
#include <iostream>
//#include "SvcParams.h"
#include <sstream>


class CVideoRelaySourceApi : public CPObject
{
    CLASS_TYPE_1(CVideoRelaySourceApi, CPObject)
  // function members
  public:
    CVideoRelaySourceApi();
    virtual ~CVideoRelaySourceApi();
	CVideoRelaySourceApi(const CVideoRelaySourceApi &other);
    virtual const char* NameOf() const { return "CVideoRelaySourceApi";}
    CVideoRelaySourceApi& operator=(const CVideoRelaySourceApi &other);
	bool operator==(const CVideoRelaySourceApi &) const;
	bool operator!=(const CVideoRelaySourceApi &) const;
	virtual CVideoRelaySourceApi* NewCopy() const {return new CVideoRelaySourceApi(*this);}
	virtual CVideoRelaySourceApi* NewEmpty() const {return new CVideoRelaySourceApi();}
	virtual void InitDefaults();
	virtual bool IsTheSame(const CVideoRelaySourceApi& other) const;
	//Get
	DWORD GetChannelHandle()const { return m_channelHandle;}
	DWORD GetSyncSource()const { return m_syncSource;}
	BYTE GetLayerId()const { return m_layerId;}
	DWORD GetPipeId()const { return m_pipeId;}
	bool GetIsSpeaker()const {return m_isSpeaker;}
	DWORD GetTid()const {return m_tid;}

	//St
	void SetChannelHandle(DWORD channelHandle){  m_channelHandle = channelHandle;}
	void SetSyncSource(DWORD ssrc){ m_syncSource =ssrc;}
	void SetLayerId(BYTE layerId){ m_layerId = layerId;}
	void SetPipeId(DWORD pipeId){  m_pipeId = pipeId;}
	void SetIsSpeaker(bool isSpeaker){m_isSpeaker = isSpeaker;}
	void SetTid(DWORD tid){m_tid = tid;}


	// data members
  private:
	DWORD m_channelHandle;
	DWORD m_syncSource;
	BYTE m_layerId;//SvcParams m_svcParams;
	DWORD m_pipeId;
	bool m_isSpeaker;
	DWORD m_tid;
};


#endif /* VIDEORELAYSOURCEAPI_H_ */
