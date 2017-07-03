//+========================================================================+
//                IPAdHocProfilesReport.h                                  |
//            Copyright 2006 Polycom Ltd.                                  |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.                    |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Ltd.                           |
//-------------------------------------------------------------------------|
// FILE:       IPAdHocProfilesReport.h                                     |                                    
// PROGRAMMER: Lior Baram                                                  |
//-------------------------------------------------------------------------|
// Who | Date       | Description:                                         |
//                    This Class reperesent a Resource report per Ad hoc EQ|
//-------------------------------------------------------------------------|
//     | 24.12.07   |                                                      |
//+========================================================================+

#ifndef _CIPRSRCADHOCRPRT
#define _CIPRSRCADHOCRPRT


#include <vector>
#include "IpCsSizeDefinitions.h"
#include "AllocateStructs.h"
#include "Segment.h"
#include "PObject.h"

//When only this Percentage of resources is free, we're almost out of resources
#define ALMOST_OUT_OF_RESOURCES_FACTOR 5

typedef enum {
    eAudioOnly,
    eFixedVideoRate,
    eFreeVideoRate,
}eVideoRateType;

typedef enum {
    eRsrcTypeTotal,
    eRsrcTypeOccupied,
    eRsrcTypeFree,
}ePortsRsrcType;


typedef struct OneAdHocProfile
{
    char profileID [MaxAliasLength];
    APIU16 minimumPorts;
    APIU16 callRate;
    APIU16 portsAvailable;
    APIU16 maxPortsAvailable;
    APIU16 conferencesAvailable;
    APIU16 maxConferenceAvailable;
    eVideoRateType videoRateType;
}AdHocProfileResourceSheet;

typedef std::vector< AdHocProfileResourceSheet * > profilesVector;

class CIPAdHocProfilesReport: public CPObject
{
    CLASS_TYPE_1(IPAdHocProfilesReport, CPObject)
public:
    //Constructor
    CIPAdHocProfilesReport() ;
    
    //Destructor
    virtual ~CIPAdHocProfilesReport();
    virtual const char* NameOf() const {return "CIPAdHocProfilesReport";}
    
    APIU32 IsAlmostOutOfResources ()  {return m_bAlmostOutOfResources;}
    void SetAlmostOutOfResources (APIU32 bAlmostOutOfResources) {m_bAlmostOutOfResources = bAlmostOutOfResources;}
    void SetMaximumCapacity(APIU32 maxAudio, APIU32 maxVideo);
    void GetMaximumCapacity(APIU32 &maxAudio, APIU32 &maxVideo);
  	void SetCurrentCapacity(APIU32 curAudio, APIU32 curVideo);
    void GetCurrentCapacity(APIU32 &curAudio, APIU32 &curVideo);  	
    void SetNumOfProfiles (APIU16 numOfProfiles) {m_numOfProfiles = numOfProfiles;}
    APIU16 GetNumOfProfiles () { return m_numOfProfiles;}
    void Serialize (WORD format, CSegment &seg);
    void DeSerialize(WORD format,CSegment &seg);
    void Dump(char * headerTxt);
    friend BYTE operator == (const CIPAdHocProfilesReport & left ,const CIPAdHocProfilesReport & rigth);
    profilesVector * GetAdhocProfilesPtr () { return m_ProfilesVector;}
private:
    APIU32 m_bAlmostOutOfResources;
    APIU16  m_numOfProfiles;
    profilesVector * m_ProfilesVector;
    APIU32    		m_maximumAudioCapacity; // Max Rai CallCapacity
  	APIU32    		m_maximumVideoCapacity;
  	APIU32    		m_currentAudioCapacity;	// Current Rai CallCapacity
  	APIU32    		m_currentVideoCapacity; 
};

#endif //_CIPRSRCADHOCRPRT
