#!/mcms/python/bin/python
#FUNCTIONS of Cop Scripts
#  
# Date: 03/11/09
# By  : Keren
#############################################################################
from McmsConnection import *

#------------------------------------------------------------------------------
def CreateConferenceFromProfileName(connection,conferenceName,profileName):
    print "run func CreateConferenceFromProfileName"
    connection.SendXmlFile('Scripts/GetProfileList.xml')
    profile_list = connection.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
    profId = "0"
    for index in range(len(profile_list)):
        profName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
        if (profName == profileName):
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            break
    if(profId!="0"):
        connection.CreateConfFromProfile(conferenceName, profId)
        print "Create Conference From Profile. profId = ", profId

#------------------------------------------------------------------------------
def VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, levelEncoderNum,num_retries=30):
    partyId = connection.GetPartyId(confId, partyName)
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confId)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        for index in range(len(ongoing_party_list)):
            if(partyId == ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data):
                encoderLevel = connection.GetTextUnder("ONGOING_PARTY","EVENT_MODE_LEVEL",index)
                if(levelEncoderNum== encoderLevel):
                    break
                else:
                    if (retry == num_retries):
                        connection.Disconnect()
                        ScriptAbort("party's event mode level is: " + encoderLevel + " it should be: "+levelEncoderNum)
                        break
                sys.stdout.write(".")
                sys.stdout.flush()
                sleep(2)
           
#------------------------------------------------------------------------------
def CreateAndConnectPartiestoCOP1080ConferenceFromDefaultProfile(connection):
    print "Start CreateAndConnectPartiestoCOP1080ConferenceFromDefaultProfile"
    confName = "COP1080Conference"
    profileName = "Event_Mode_1080_1728Kb"
    
    CreateConferenceFromProfileName(connection, confName,profileName)
    confId  = connection.WaitConfCreated(confName,10)  
    # disable auto layout mode        
    print "Disable auto layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioLayout.xml")
    connection.ModifyXml("SET_AUTO_LAYOUT","ID",confId)
    connection.ModifyXml("SET_AUTO_LAYOUT","AUTO_LAYOUT","false")
    connection.Send()
    sleep(4)
    
    ##Add 2 participants to the first encoder
    partyName = confName+"_H323_ENCODER_1"
    partyIp =  "1.1.1.1" 
    connection.AddVideoParty(confId, partyName, partyIp)
    print "Connecting H323 Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")


    partyName = confName+"_H323_2_ENCODER_1"
    partyIp =  "2.1.1.1" 
    connection.AddVideoParty(confId, partyName, partyIp)
    print "Connecting SECOND H323 Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")
   

    #-------Dial in SIP party
    #partyNameInSim = confName+"_SIP_IN_ENCODER_1"
    #connection.SimulationAddSipParty(partyNameInSim, confName,"H264(hd1080)+ALL")
    #connection.SimulationConnectSipParty(partyNameInSim)
    #print "Connecting SIP Dial in Party H264(hd1080)+ALL"
    #connection.WaitAllOngoingConnected(confId)
    #connection.WaitAllOngoingNotInIVR(confId)
    #partyName = "COP1080Conference_(000)"
    #VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")
    
    
    #Add 2 participants to the first encoder
    partyName = confName+"_H323_ENCODER_2"
    partyIp =  "1.1.2.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "832")
    print "Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")
   
    partyName = confName+"_SIP_ENCODER_2"
    partyIp =  "1.1.2.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "832")
    print "Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")
   

    #Add 2 participants to the third encoder
    partyName = confName+"_H323_ENCODER_3"
    partyIp =  "1.1.3.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "512")
    print "Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
   
    partyName = confName+"_SIP_ENCODER_3"
    partyIp =  "1.1.3.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "512")
    print "Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
   
    #Add 2 participants to the fourth encoder
    partyName = confName+"_H323_ENCODER_4"
    partyIp =  "1.1.4.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "384")
    print "Connecting H323 DiaSIPl out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
   
    partyName = confName+"_SIP_ENCODER_4"
    partyIp =  "1.1.4.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"h263", "384")
    print "Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
    return confId
#------------------------------------------------------------------------------
def WaitConfLayoutUpdated(connection,confId,LayoutType,num_retries=30): 
    print "Wait Conference layout updated with layout type:" + LayoutType
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confId)
    for retry in range(num_retries+1):
        connection.Send()
        conferenceLayoutType = connection.GetTextUnder("RESERVATION","LAYOUT")
        if(conferenceLayoutType == LayoutType):
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml(encoding="utf-8")
            connection.Disconnect()
            ScriptAbort("Conf layout not updated to layout: "+ LayoutType +", But in layout : " + conferenceLayoutType)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
#------------------------------------------------------------------------------
def CreateAndConnectPartiestoCOP720ConferenceFromDefaultProfile(connection):
    confName = "COP720Conference"
    profileName = "Event_Mode_720P_832Kb"
    CreateConferenceFromProfileName(connection, confName,profileName)
    confId  = connection.WaitConfCreated(confName,10)  
    print "Conference created, Conf ID = ", confId
#    listEntitiesIDs = connection.GetPartyIDs(confId)
#    for entity in listEntitiesIDs:
#        print "EntityID = ", entity
    # disable auto layout mode        
    print "Disable auto layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioLayout.xml")
    connection.ModifyXml("SET_AUTO_LAYOUT","ID",confId)
    connection.ModifyXml("SET_AUTO_LAYOUT","AUTO_LAYOUT","false")
    connection.Send()
    sleep(4)
    ##Add 2 participants to the first encoder
    partyName = confName+"_H323_ENCODER_1"
    partyIp =  "1.1.1.1" 
    connection.AddVideoParty(confId, partyName, partyIp)
    print "Connecting H323 Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")

    partyName = confName+"_SIP_ENCODER_1"
    partyIp =  "1.1.1.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True)
    print "Connecting SIP Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")

    #Add 2 participants to the second encoder
    partyName = confName+"_H323_ENCODER_2"
    partyIp =  "1.1.2.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "704")
    print "Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")

    partyName = confName+"_SIP_ENCODER_2"
    partyIp =  "1.1.2.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "704")
    print "Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")

  

    #Add 2 participants to the third encoder
    partyName = confName+"_H323_ENCODER_3"
    partyIp =  "1.1.3.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "512")
    print "Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")

    partyName = confName+"_SIP_ENCODER_3"
    partyIp =  "1.1.3.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "512")
    print "Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
 
    #Add 2 participants to the fourth encoder
    partyName = confName+"_H323_ENCODER_4"
    partyIp =  "1.1.4.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "384")
    print "Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
 
    partyName = confName+"_SIP_ENCODER_4"
    partyIp =  "1.1.4.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"h263", "384")
    print "Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
    return confId
#------------------------------------------------------------------------------
def CreateAndConnectPartiestoCOP4CIF4x3ConferenceFromDefaultProfile(connection):
    confName = "COP4Cif4x3Conference"
    profileName = "Event_Mode_4CIF_768Kb_4x3"
    CreateConferenceFromProfileName(connection, confName,profileName)
    confId  = connection.WaitConfCreated(confName,10)
    
    # disable auto layout mode        
    print "1. Disable auto layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioLayout.xml")
    connection.ModifyXml("SET_AUTO_LAYOUT","ID",confId)
    connection.ModifyXml("SET_AUTO_LAYOUT","AUTO_LAYOUT","false")
    connection.Send()
    sleep(4)  
    
    ##Add 2 participants to the first encoder
    partyName = confName+"_H323_ENCODER_1"
    partyIp =  "1.1.1.1" 
    connection.AddVideoParty(confId, partyName, partyIp)
    print "2. Connecting H323 Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")
   
    partyName = confName+"_SIP_ENCODER_1"
    partyIp =  "1.1.1.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True)
    print "3. Connecting SIP Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")
   
    #Add 2 participants to the second encoder
    partyName = confName+"_H323_ENCODER_2"
    partyIp =  "1.1.2.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "512")
    print "4. Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")
    #VerifyPartyConnectedToEncoderLevel("2")

    partyName = confName+"_SIP_ENCODER_2"
    partyIp =  "1.1.2.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "512")
    print "5. Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")
   

    #Add 2 participants to the third encoder
    partyName = confName+"_H323_ENCODER_3"
    partyIp =  "1.1.3.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "384")
    print "6. Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
    
    partyName = confName+"_SIP_ENCODER_3"
    partyIp =  "1.1.3.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "384")
    print "7. Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
    
    #Add 2 participants to the fourth encoder
    partyName = confName+"_H323_ENCODER_4"
    partyIp =  "1.1.4.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "256")
    print "8. Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
   
    partyName = confName+"_SIP_ENCODER_4"
    partyIp =  "1.1.4.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"h263", "256")
    print "9. Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
    return confId
#------------------------------------------------------------------------------
def CreateAndConnectPartiestoCOP4CIF16x9ConferenceFromDefaultProfile(connection):
    confName = "COP4Cif16x9Conference"
    profileName = "Event_Mode_4CIF_768Kb_16x9"
    CreateConferenceFromProfileName(connection, confName,profileName)
    confId  = connection.WaitConfCreated(confName,10)  
     
    # disable auto layout mode        
    print "1. Disable auto layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioLayout.xml")
    connection.ModifyXml("SET_AUTO_LAYOUT","ID",confId)
    connection.ModifyXml("SET_AUTO_LAYOUT","AUTO_LAYOUT","false")
    connection.Send()
    sleep(4)
    
    ##Add 2 participants to the first encoder
    partyName = confName+"_H323_ENCODER_1"
    partyIp =  "1.1.1.1" 
    connection.AddVideoParty(confId, partyName, partyIp)
    print "2. Connecting H323 Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")
   
    partyName = confName+"_SIP_ENCODER_1"
    partyIp =  "1.1.1.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True)
    print "3. Connecting SIP Dial out Party to encoder 1" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")
    
    #Add 2 participants to the second encoder 
    partyName = confName+"_H323_ENCODER_2"
    partyIp =  "1.1.2.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "384")
    print "4. Connecting H323 Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")
    
    partyName = confName+"_SIP_ENCODER_2"
    partyIp =  "1.1.2.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "384")
    print "5. Connecting SIP Dial out Party to encoder 2" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")
   
    
    #Add 2 participants to the third encoder - need just cif caps
    #-------Dial in H323 party
#    partyName = confName+"_H323_IN_ENCODER_3"
#    connection.SimulationAddH323Party(partyName, confName,"H264(cif)+ALL")
#    connection.SimulationConnectH323Party(partyName)
#    print "6. Connecting H323 Dial in Party - H264(cif)+ALL"
#    connection.WaitAllOngoingConnected(confId)
#    connection.WaitAllOngoingNotInIVR(confId)
##    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
   
    #-------Dial in SIP party
#    partyName = confName+"_SIP_IN_ENCODER_3"
#    connection.SimulationAddSipParty(partyName, confName,"H264(cif)+ALL")
#    connection.SimulationConnectSipParty(partyName)
#    print "7. Connecting SIP Dial in Party H264(cif)+ALL"
#    connection.WaitAllOngoingConnected(confId)
#    connection.WaitAllOngoingNotInIVR(confId)
##    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName,"3")
    
    #Add 2 participants to the fourth encoder
    partyName = confName+"_H323_ENCODER_4"
    partyIp =  "1.1.4.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "256")
    print "8. Connecting H323 Dial out Party to encoder 4" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
   
    partyName = confName+"_SIP_ENCODER_4"
    partyIp =  "1.1.4.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"h263", "256")
    print "9. Connecting SIP Dial out Party to encoder 4" 
    connection.WaitAllOngoingConnected(confId)
    connection.WaitAllOngoingNotInIVR(confId)
    VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
   
    return confId
#------------------------------------------------------------------------------
def StartLectureMode(connection,confid, partyName):
    print "Conference ID: "+ confid + " New Lecturer: " + partyName
    connection.LoadXmlFile('Scripts/PresentationMode/UpdateLectureMode.xml')
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_NAME",partyName)  
    connection.ModifyXml("SET_LECTURE_MODE","ON","true")  
    connection.ModifyXml("SET_LECTURE_MODE","TIMER","true")  
    connection.ModifyXml("SET_LECTURE_MODE","INTERVAL","15")  
    connection.ModifyXml("SET_LECTURE_MODE","AUDIO_ACTIVATED","false")
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_MODE_TYPE","lecture_mode")
    connection.Send()
    return

#------------------------------------------------------------------------------
def SetLectureMode(connection,confid,partyName="", isOn="true",isTimer="true",timerInterval="15",isAudioActivate="false",lectureModeType="lecture_mode"):
    print_name = partyName
    if print_name == "":
        print_name = "empty"
    print "SetLectureMode: lecturer:" + partyName + ", isOn:" + isOn + ", isTimer:" + isTimer + ", timerInterval:" + timerInterval + ", isAudioActivate:" + isAudioActivate + ", lectureModeType:" + lectureModeType
    connection.LoadXmlFile('Scripts/PresentationMode/UpdateLectureMode.xml')
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_NAME",partyName)  
    connection.ModifyXml("SET_LECTURE_MODE","ON",isOn)  
    connection.ModifyXml("SET_LECTURE_MODE","TIMER",isTimer)  
    connection.ModifyXml("SET_LECTURE_MODE","INTERVAL",timerInterval)  
    connection.ModifyXml("SET_LECTURE_MODE","AUDIO_ACTIVATED",isAudioActivate)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_MODE_TYPE",lectureModeType)
    connection.Send()
    return  
#------------------------------------------------------------------------------
def StartImmediatlyPartiestoCOP720ConferenceFromDefaultProfile(connection):
    confName = "COP720Conference"
    profileName = "Event_Mode_720P_832Kb"
    CreateConferenceFromProfileName(connection, confName,profileName)
    confId  = connection.WaitConfCreated(confName,10)  
    
    # disable auto layout mode        
    print "Disable auto layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioLayout.xml")
    connection.ModifyXml("SET_AUTO_LAYOUT","ID",confId)
    connection.ModifyXml("SET_AUTO_LAYOUT","AUTO_LAYOUT","false")
    connection.Send()
    sleep(4)
    ##Add 2 participants to the first encoder
    partyName = confName+"_H323_ENCODER_1"
    partyIp =  "1.1.1.1" 
    connection.AddVideoParty(confId, partyName, partyIp)
    print "Connecting H323 Dial out Party to encoder 1" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")

    partyName = confName+"_SIP_ENCODER_1"
    partyIp =  "1.1.1.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True)
    print "Connecting SIP Dial out Party to encoder 1" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "1")

    #Add 2 participants to the second encoder
    partyName = confName+"_H323_ENCODER_2"
    partyIp =  "1.1.2.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "640")
    print "Connecting H323 Dial out Party to encoder 2" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")

    partyName = confName+"_SIP_ENCODER_2"
    partyIp =  "1.1.2.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "640")
    print "Connecting SIP Dial out Party to encoder 2" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "2")

  

    #Add 2 participants to the third encoder
    partyName = confName+"_H323_ENCODER_3"
    partyIp =  "1.1.3.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "512")
    print "Connecting H323 Dial out Party to encoder 2" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")

    partyName = confName+"_SIP_ENCODER_3"
    partyIp =  "1.1.3.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"auto", "512")
    print "Connecting SIP Dial out Party to encoder 2" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "3")
 
    #Add 2 participants to the fourth encoder
    partyName = confName+"_H323_ENCODER_4"
    partyIp =  "1.1.4.1" 
    connection.AddVideoParty(confId, partyName, partyIp,False,"auto", "384")
    print "Connecting H323 Dial out Party to encoder 2" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")
 
    partyName = confName+"_SIP_ENCODER_4"
    partyIp =  "1.1.4.2" 
    connection.AddVideoParty(confId, partyName, partyIp, True,"h263", "384")
    print "Connecting SIP Dial out Party to encoder 2" 
##     connection.WaitAllOngoingConnected(confId)
##     connection.WaitAllOngoingNotInIVR(confId)
##     VerifyPartyConnectedToEncoderLevel(connection,confId,partyName, "4")


    
    return confId
#------------------------------------------------------------------------------
