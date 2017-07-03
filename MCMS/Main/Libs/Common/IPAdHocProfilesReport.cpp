//+========================================================================+
//                IPResorceReportGenerator.cpp                             |
//            Copyright 2006 Polycom Ltd.                                  |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.                    |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Ltd.                           |
//-------------------------------------------------------------------------|
// FILE:       IPAdHocProfilesReport.cpp                                   |                                    
// PROGRAMMER: Lior Baram                                                  |
//-------------------------------------------------------------------------|
// Who | Date       | Description:                                         |
//                    This Class reperesent a Resource report per Ad hoc EQ|
//-------------------------------------------------------------------------|
//     | 24.12.07   |                                                      |
//+========================================================================+


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "IPAdHocProfilesReport.h"
#include "ObjString.h"

CIPAdHocProfilesReport::CIPAdHocProfilesReport()
{
    m_bAlmostOutOfResources = FALSE;
    m_numOfProfiles = 0;
    m_ProfilesVector = new  profilesVector;
    m_maximumAudioCapacity = 0; // Max Rai CallCapacity
  	m_maximumVideoCapacity = 0;
  	m_currentAudioCapacity = 0;	// Current Rai CallCapacity
  	m_currentVideoCapacity = 0; 
}


CIPAdHocProfilesReport::~CIPAdHocProfilesReport()
{
    for (profilesVector::iterator itr=m_ProfilesVector->begin() ; itr != m_ProfilesVector->end() ; ++itr)
    {
        AdHocProfileResourceSheet * pCurrProfile = (*itr);
        PDELETE (pCurrProfile);
    }
    m_ProfilesVector->clear();
    PDELETE(m_ProfilesVector); 
}

void CIPAdHocProfilesReport::Dump(char * headerTxt)
{
    CLargeString msg;
    msg << "*** CIPAdHocProfilesReport::Dump - " << headerTxt;
    msg << "\n m_bAlmostOutOfResources = " <<  m_bAlmostOutOfResources;
    msg << "\n m_maximumAudioCapacity  = " << m_maximumAudioCapacity;
    msg << "\n m_maximumVideoCapacity  = " << m_maximumVideoCapacity;
    msg << "\n m_currentAudioCapacity  = " << m_currentAudioCapacity;
    msg << "\n m_currentVideoCapacity  = " << m_currentVideoCapacity;
    msg << "\n m_numOfProfiles         = " << m_numOfProfiles;
    if (m_numOfProfiles)
    {
        int i = 1;
        for (profilesVector::iterator itr=m_ProfilesVector->begin() ; itr != m_ProfilesVector->end() ; ++itr)
        {
            msg << "\n ** Profile Details #" <<  i;
            AdHocProfileResourceSheet * pCurrProfile = (*itr);
            if (pCurrProfile != NULL)
            {
                msg << "\n profileID              = " << pCurrProfile->profileID;
                msg << "\n minimumPorts           = " << pCurrProfile->minimumPorts;
                msg << "\n callRate               = " << pCurrProfile->callRate;
                msg << "\n portsAvailable         = " << pCurrProfile->portsAvailable;
                msg << "\n maxPortsAvailable      = " << pCurrProfile->maxPortsAvailable;
                msg << "\n conferencesAvailable   = " << pCurrProfile->conferencesAvailable;
                msg << "\n maxConferenceAvailable = " << pCurrProfile->maxConferenceAvailable;
                msg << "\n videoRateType          = " << (BYTE)pCurrProfile->videoRateType << "\n";
                i++;
            }
        }
    }
    PTRACE (eLevelInfoNormal, msg.GetString());
}

void CIPAdHocProfilesReport::Serialize (WORD format, CSegment &seg)
{
    if (format == NATIVE)
    {
        seg << m_bAlmostOutOfResources;
        seg << m_maximumAudioCapacity;
        seg << m_maximumVideoCapacity;
        seg << m_currentAudioCapacity;
        seg << m_currentVideoCapacity;
        seg << (WORD)m_numOfProfiles;
        if (m_numOfProfiles)
        {
            for (profilesVector::iterator itr=m_ProfilesVector->begin() ; itr != m_ProfilesVector->end() ; ++itr)
            {
                AdHocProfileResourceSheet * pCurrProfile = (*itr);
                if (pCurrProfile != NULL)
                    seg.Put ((BYTE *) pCurrProfile, sizeof(AdHocProfileResourceSheet));
            }
        }
    }
    
}

void CIPAdHocProfilesReport::DeSerialize(WORD format,CSegment &seg)
{
    if (format == NATIVE)
    {
        seg >> m_bAlmostOutOfResources;
        seg >> m_maximumAudioCapacity;
        seg >> m_maximumVideoCapacity;
        seg >> m_currentAudioCapacity;
        seg >> m_currentVideoCapacity;
        seg >> m_numOfProfiles;
        for (int i=0; i<m_numOfProfiles; i++)
        {
             AdHocProfileResourceSheet * pCurrProfileRsrcReport = new AdHocProfileResourceSheet;
             if (pCurrProfileRsrcReport)
             {
                 seg.Get ((BYTE *) pCurrProfileRsrcReport, sizeof(AdHocProfileResourceSheet));
                 GetAdhocProfilesPtr()->push_back(pCurrProfileRsrcReport);
             }
        }
    }   
}

BYTE operator == (const CIPAdHocProfilesReport & left ,const CIPAdHocProfilesReport & right)
{
    BYTE retVal = TRUE;
    if (left.m_bAlmostOutOfResources == right.m_bAlmostOutOfResources)
    {
        if (left.m_numOfProfiles == right.m_numOfProfiles)
        {
            if (left.m_maximumAudioCapacity != right.m_maximumAudioCapacity)
                retVal = FALSE;
            if (left.m_maximumVideoCapacity != right.m_maximumVideoCapacity)
                retVal = FALSE;
            if (left.m_currentAudioCapacity != right.m_currentAudioCapacity)
                retVal = FALSE;
            if (left.m_currentVideoCapacity != right.m_currentVideoCapacity)
                retVal = FALSE;
            
            profilesVector::iterator rItr = right.m_ProfilesVector->begin();
            for ( profilesVector::iterator lItr = left.m_ProfilesVector->begin() ; retVal && lItr != left.m_ProfilesVector->end() ; ++lItr)
             {
                 AdHocProfileResourceSheet * lPtr = (*lItr);
                 AdHocProfileResourceSheet * rPtr = (*rItr);
                 if (strncmp(lPtr->profileID,rPtr->profileID, MaxAliasLength))
                     retVal = FALSE;
                 else if (lPtr->minimumPorts != rPtr->minimumPorts)
                     retVal = FALSE;
                 else if  (lPtr->callRate != rPtr->callRate)
                     retVal = FALSE;
                 else if  (lPtr->portsAvailable != rPtr->portsAvailable)
                     retVal = FALSE;
                 else if  (lPtr->maxPortsAvailable != rPtr->maxPortsAvailable)
                     retVal = FALSE;
                 else if  (lPtr->conferencesAvailable != rPtr->conferencesAvailable)
                     retVal = FALSE;
                 else if  (lPtr->maxConferenceAvailable != rPtr->maxConferenceAvailable)
                     retVal = FALSE;
                 else if  (lPtr->videoRateType != rPtr->videoRateType)
                     retVal = FALSE;
                
                 ++rItr;
             }
        }
        else
            retVal = FALSE;
    }
    else
        retVal = FALSE;
    
    return retVal;
}

void CIPAdHocProfilesReport::SetMaximumCapacity(APIU32 maxAudio, APIU32 maxVideo)
{
    m_maximumAudioCapacity = maxAudio;
    m_maximumVideoCapacity = maxVideo;
}
void  CIPAdHocProfilesReport::SetCurrentCapacity(APIU32 curAudio, APIU32 curVideo)
{
    m_currentAudioCapacity = curAudio;
    m_currentVideoCapacity = curVideo;
}

void CIPAdHocProfilesReport::GetMaximumCapacity(APIU32 &maxAudio, APIU32 &maxVideo)
{
    maxAudio = m_maximumAudioCapacity;
    maxVideo = m_maximumVideoCapacity;
}
void CIPAdHocProfilesReport::GetCurrentCapacity(APIU32 &curAudio, APIU32 &curVideo)
{
    curAudio = m_currentAudioCapacity;
    curVideo = m_currentVideoCapacity;
}

