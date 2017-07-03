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
// FILE:       IPResorceReportGenerator.cpp                                |                                    
// PROGRAMMER: Lior Baram                                                  |
//-------------------------------------------------------------------------|
// Who | Date       | Description: Implementation of IP resources reports  |
//-------------------------------------------------------------------------|
//     | 18.12.07   |                                                      |
//+========================================================================+

#include "IPResourceReportGenerator.h"
#include "ConfPartyManagerApi.h"
#include "SingleToneApi.h"
#include "CDRUtils.h"
#include "CommConf.h"
#include "CommConfDB.h"
#define min(a,b)    (((a) < (b)) ? (a) : (b))
const int MAX_NUM_OF_PROFILES = 7;

//////////////////////////////////////////////////////////////////////
// Class CIPResourceGenerator
// This Class Generates the reports
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


void CIPResourceGenerator::GenerateAdHocProfilesReport (CIPAdHocProfilesReport * pAdhocProfilesResourceReport,
                                                        ALLOC_REPORT_PARAMS_S* pResult,
                                                        CCommResDB * pResDB,
                                                        CCommResDB * pProfilesDB)
{
    // First we Get the Resource Picture
    int nAudioOnlyPorts = pResult->m_numAudioParties[eRsrcTypeFree] +
        pResult->m_numVideoParties[eRsrcTypeFree];
    int nVideoPorts = pResult->m_numVideoParties[eRsrcTypeFree];
    int nMaxVideoPorts = pResult->m_numVideoParties[eRsrcTypeTotal];
    int nMaxAudioPorts = pResult->m_numAudioParties[eRsrcTypeTotal] +
        pResult->m_numVideoParties[eRsrcTypeTotal];

    pAdhocProfilesResourceReport->SetMaximumCapacity(nMaxAudioPorts, nMaxVideoPorts);
    pAdhocProfilesResourceReport->SetCurrentCapacity(nAudioOnlyPorts, nVideoPorts);
    // Almost out of resources factor is the lowest percentage of resources which we consider
    // OK before reporting Almost out of resources in RAI message
    if ((nMaxVideoPorts * ALMOST_OUT_OF_RESOURCES_FACTOR / 100) >= nVideoPorts ||
        (nMaxAudioPorts * ALMOST_OUT_OF_RESOURCES_FACTOR / 100)  >= nAudioOnlyPorts)
        pAdhocProfilesResourceReport->SetAlmostOutOfResources(TRUE);
    else
        pAdhocProfilesResourceReport->SetAlmostOutOfResources(FALSE);

    CMedString msg;
    msg << "\n num of Audio Only Ports = " << nAudioOnlyPorts;
    msg << "\n num of Video Ports = " << nVideoPorts;
    msg << "\n max Audio only Ports = " << nMaxAudioPorts ;
    msg << "\n max Video Ports = " << nMaxVideoPorts ;
    msg << "\n Almost Out Of Resources = " << pAdhocProfilesResourceReport->IsAlmostOutOfResources() ;
        
	PTRACE2 (eLevelInfoNormal,"CIPResourceGenerator::GenerateAdHocProfilesReport " ,msg.GetString());

    // then we go over all the ad hoc profiles which are attached to an EQ
    if (!IsValidPObjectPtr (pResDB))
    {
        PTRACE (eLevelError, "CIPResourceGenerator::GenerateAdHocProfilesReport No MR CommResDB Ptr");
        return;
    }
    if (!IsValidPObjectPtr (pProfilesDB))
    {
        PTRACE (eLevelError, "CIPResourceGenerator::GenerateAdHocProfilesReport No Profiles CommResDB Ptr");
        return;
    }
    
    const CCommResDB::ReservArray& rsrvArray = pResDB->GetReservArray();
    int i=0;
    CCommResDB::ReservArray::const_iterator iter_end= rsrvArray.end();
	for (CCommResDB::ReservArray::const_iterator iter = rsrvArray.begin(); iter != iter_end && i < MAX_NUM_OF_PROFILES ; ++iter)
	{
        CCommResShort * pCurrEQ = *iter;
        
        if (NULL != pCurrEQ && pCurrEQ->IsEntryQ())
        {
            CCommRes* pEqConf = pResDB->GetCurrentRsrv(pCurrEQ->GetConferenceId ());
            if ( IsValidPObjectPtr(pEqConf) && pEqConf->GetAdHoc())
            {
                DWORD dwAdHocProfileId = (pCurrEQ)->GetAdHocProfileId();
                // If the Ad-Hoc is checked and no profile is configured return error
                if (dwAdHocProfileId != 0xFFFFFFFF)
                {
                    // Get Profile according to Ad-Hoc Profile ID		
                    CCommRes* pConfProfile = pProfilesDB->GetCurrentRsrv(dwAdHocProfileId);	// get & allocate
                    if(NULL == pConfProfile)
                        PTRACE2INT(eLevelError, "CIPResourceGenerator::GenerateAdHocProfilesReport: NULL pointer - No profile for Ad-Hoc conf #d",dwAdHocProfileId );
                    else
                    {
                        AdHocProfileResourceSheet * pCurrProfileRsrcReport = new AdHocProfileResourceSheet;
                        strncpy(pCurrProfileRsrcReport->profileID,pCurrEQ->GetNumericConfId(), sizeof(pCurrProfileRsrcReport->profileID) - 1);
                        pCurrProfileRsrcReport->profileID[sizeof(pCurrProfileRsrcReport->profileID) - 1] = '\0';
                        pCurrProfileRsrcReport->minimumPorts = 1;

                        //Calculate Call Rate
                    
                        const char * dummy =  CCDRUtils::Get_Transfer_Rate_Command_BitRate(pConfProfile->GetConfTransferRate(),&pCurrProfileRsrcReport->callRate);
                       
                        
                        //Audio only Profile
                        if (pConfProfile->IsAudioConf())
                        {
                            pCurrProfileRsrcReport->portsAvailable = 0xFFFF;          //if 0xFFF CS will omit
                            pCurrProfileRsrcReport->maxPortsAvailable = 0;        //if 0 CS will omit
                            pCurrProfileRsrcReport->videoRateType = eAudioOnly;
                            pCurrProfileRsrcReport->maxConferenceAvailable = pResult->max_num_ongoing_conf;
                            pCurrProfileRsrcReport->conferencesAvailable = pResult->max_num_ongoing_conf - pResult->current_num_ongoing_conf;
                            pCurrProfileRsrcReport->conferencesAvailable = min ((pResult->max_num_ongoing_conf - pResult->current_num_ongoing_conf), nAudioOnlyPorts);
                        }
                        //Video Profile
                        else
                        {
                            pCurrProfileRsrcReport->portsAvailable = 0xFFFF;              //if 0xFFF cs will omit
                            pCurrProfileRsrcReport->maxPortsAvailable = 0;        //if 0 - CS will omit     
                            // Video Switch
                            if (pConfProfile->GetIsHDVSW())
                                pCurrProfileRsrcReport->videoRateType = eFixedVideoRate;
                            // CP
                            else
                                pCurrProfileRsrcReport->videoRateType = eFreeVideoRate;
                            pCurrProfileRsrcReport->conferencesAvailable = min ((pResult->max_num_ongoing_conf - pResult->current_num_ongoing_conf), nVideoPorts);
                            pCurrProfileRsrcReport->maxConferenceAvailable = pResult->max_num_ongoing_conf;
                        }   
                        pAdhocProfilesResourceReport->GetAdhocProfilesPtr()->push_back(pCurrProfileRsrcReport);
                        i++;
                        POBJDELETE(pConfProfile);
                    }
                }
            }
            POBJDELETE(pEqConf);
        }
    }
    //General Report information
    pAdhocProfilesResourceReport->SetNumOfProfiles(pAdhocProfilesResourceReport->GetAdhocProfilesPtr()->size());
}


