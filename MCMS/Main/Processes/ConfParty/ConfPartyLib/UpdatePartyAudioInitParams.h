#ifndef UPDATEPARTYAUDIOINITPARAMS_H_
#define UPDATEPARTYAUDIOINITPARAMS_H_

#include "UpdatePartyInitParams.h"
#include "BridgePartyAudioParams.h"
#include "AudioApiDefinitionsStrings.h"

class CUpdatePartyAudioInitParams : public CUpdatePartyInitParams
{
CLASS_TYPE_1(CUpdatePartyAudioInitParams,CUpdatePartyInitParams)
public:

	CUpdatePartyAudioInitParams ();
	CUpdatePartyAudioInitParams (const CBridgePartyInitParams& rOtherUpdatePartyInitParams);
	virtual const char* NameOf() const { return "CUpdatePartyAudioInitParams";}

	virtual ~CUpdatePartyAudioInitParams();
	
	virtual void AllocateInParams();
	virtual void AllocateOutParams();

	
	void InitiateMediaInParams(const CBridgePartyAudioParams* pBridgePartyMediaParams);
	void InitiateMediaOutParams(const CBridgePartyAudioParams* pBridgePartyMediaParams);
	
	void UpdateInitAudioAlgorithm(EMediaDirection eMediaDirection,DWORD newAudioAlgorithm, DWORD maxAverageBitrate);
	void UpdateMediaInType(EMediaTypeUpdate mediaTypeUpdate);
	void UpdateInitMute(EMediaDirection eMediaDirection,EOnOff eOnOff, WORD srcRequest);
	void UpdateInitVolume(EMediaDirection eMediaDirection,DWORD newAudioVolume);
	void IncreaseInitVolume(EMediaDirection eMediaDirection,BYTE increaseRate);	
	void DecreaseInitVolume(EMediaDirection eMediaDirection,BYTE increaseRate);
	void UpdateInitNoiseDetection(EOnOff eOnOff, BYTE newNoiseDetectionThreshold);
	void UpdateInitAGC(EOnOff eOnOff);
	void UpdateUseSpeakerSsrcForTx(BOOL useSpeakerSsrcForTx);
		
protected:

};


#endif /*UPDATEPARTYAUDIOINITPARAMS_H_*/
