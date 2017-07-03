#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from subprocess import call
from RoomSwitch import *
import os

class EP:
    def __init__(self,name,screens):
        self.m_name = name
        self.m_screens = screens
    def GetName(self):
        return self.m_name
    def GetNumOfScreen(self):
        return self.m_screens
    
def GetNumberOfRelevantScreenByCellIndex(SpeakersScreens,ViewersScreens,cellIndex,ScreenNumber):
        NumberOfScreenViewerMustSee = -1
        if SpeakersScreens == 1 and ViewersScreens == 1: 
            if cellIndex == 0: 
                NumberOfScreenViewerMustSee = 1
        elif SpeakersScreens == 1 and ViewersScreens == 2:
            if cellIndex == 0: 
                NumberOfScreenViewerMustSee = 1
        elif SpeakersScreens == 1 and ViewersScreens == 3:
            if cellIndex == 0: 
                NumberOfScreenViewerMustSee = 1
        elif SpeakersScreens == 1 and ViewersScreens == 4: 
            if cellIndex == 0: 
                NumberOfScreenViewerMustSee = 1
        elif SpeakersScreens == 2 and ViewersScreens == 1:
            if cellIndex == 0: 
                NumberOfScreenViewerMustSee = 2
            elif cellIndex == 1:
                NumberOfScreenViewerMustSee = 1
                
        elif SpeakersScreens == 2 and ViewersScreens == 2: 
            if cellIndex == 0:
                 if ScreenNumber == 2 or ScreenNumber == 1:
                     NumberOfScreenViewerMustSee = ScreenNumber 
        elif SpeakersScreens == 2 and ViewersScreens == 3: 
            if cellIndex == 0: 
                if ScreenNumber == 2 or ScreenNumber == 1:
                     NumberOfScreenViewerMustSee = ScreenNumber 
        elif SpeakersScreens == 2 and ViewersScreens == 4: 
            if cellIndex == 0: 
                if ScreenNumber == 2 or ScreenNumber == 1:
                     NumberOfScreenViewerMustSee = ScreenNumber 
        elif SpeakersScreens == 3 and ViewersScreens == 1: 
            if cellIndex == 3: 
                NumberOfScreenViewerMustSee = 2
            elif cellIndex == 4: 
                NumberOfScreenViewerMustSee = 1
            elif cellIndex == 5: 
                NumberOfScreenViewerMustSee = 3
        elif SpeakersScreens == 3 and ViewersScreens == 2: 
            if cellIndex == 0: 
                NumberOfScreenViewerMustSee = 1
            elif ScreenNumber == 2 and cellIndex ==1:
                NumberOfScreenViewerMustSee = 2
            elif ScreenNumber == 1 and cellIndex ==1:
                NumberOfScreenViewerMustSee = 3
        elif SpeakersScreens == 3 and ViewersScreens == 3: 
             if cellIndex == 0: 
                if ScreenNumber == 1 or ScreenNumber == 2 or ScreenNumber == 3:
                    NumberOfScreenViewerMustSee = ScreenNumber
        elif SpeakersScreens == 3 and ViewersScreens == 4: 
            if cellIndex == 0: 
                if ScreenNumber == 1:
                   NumberOfScreenViewerMustSee = 3  
                elif ScreenNumber == 2:
                    NumberOfScreenViewerMustSee = 1
                elif ScreenNumber == 4:
                    NumberOfScreenViewerMustSee = 2
        elif SpeakersScreens == 4 and ViewersScreens == 1: 
            if cellIndex == 8: 
                NumberOfScreenViewerMustSee = 4
            elif cellIndex == 7: 
                NumberOfScreenViewerMustSee = 2
            elif cellIndex == 6:
                NumberOfScreenViewerMustSee = 1
            elif cellIndex == 5:
                NumberOfScreenViewerMustSee = 3       
        elif SpeakersScreens == 4 and ViewersScreens == 2: 
            if cellIndex == 0:
                if  ScreenNumber ==1:
                    NumberOfScreenViewerMustSee = 1
                elif ScreenNumber ==2:
                    NumberOfScreenViewerMustSee = 4 
            elif cellIndex == 1:
                if  ScreenNumber ==1:
                    NumberOfScreenViewerMustSee = 3
                elif ScreenNumber ==2:
                    NumberOfScreenViewerMustSee = 2
        elif SpeakersScreens == 4 and ViewersScreens == 3: 
             if cellIndex == 0: 
                 if  ScreenNumber ==1:
                    NumberOfScreenViewerMustSee = 2
                 elif ScreenNumber ==3:
                    NumberOfScreenViewerMustSee = 3
             elif cellIndex == 1:
                  if  ScreenNumber ==1:
                    NumberOfScreenViewerMustSee = 1
                  elif ScreenNumber ==2:
                    NumberOfScreenViewerMustSee = 4 
        elif SpeakersScreens == 4 and ViewersScreens == 4: 
            if cellIndex == 0: 
                if ScreenNumber ==1 or ScreenNumber ==2 or ScreenNumber ==3 or ScreenNumber ==4:
                    NumberOfScreenViewerMustSee = ScreenNumber
       
        return NumberOfScreenViewerMustSee 
          
def GetLayoutTypeAccordingToSpeakerAndViewerScreens(SpeakersScreens,ViewersScreens):
        retString = ""
        if SpeakersScreens == 1 and ViewersScreens == 1: 
                retString = "1x1"
        elif SpeakersScreens == 1 and ViewersScreens == 2:
                retString = "1x1"
        elif SpeakersScreens == 1 and ViewersScreens == 3:
                retString = "1x1"
        elif SpeakersScreens == 1 and ViewersScreens == 4: 
                retString = "1x1"
        elif SpeakersScreens == 2 and ViewersScreens == 1:
                retString = "1x2Flex"
        elif SpeakersScreens == 2 and ViewersScreens == 2: 
                retString = "1x1"
        elif SpeakersScreens == 2 and ViewersScreens == 3: 
                retString = "1x1"
        elif SpeakersScreens == 2 and ViewersScreens == 4: 
                retString = "1x1"
        elif SpeakersScreens == 3 and ViewersScreens == 1: 
                retString = "3x3"
        elif SpeakersScreens == 3 and ViewersScreens == 2: 
                retString = "1x2Flex"
        elif SpeakersScreens == 3 and ViewersScreens == 3: 
                retString = "1x1"
        elif SpeakersScreens == 3 and ViewersScreens == 4: 
                retString = "1x1"
        elif SpeakersScreens == 4 and ViewersScreens == 1: 
                retString = "1and8Upper"
        elif SpeakersScreens == 4 and ViewersScreens == 2: 
                retString ="1x2Flex"
        elif SpeakersScreens == 4 and ViewersScreens == 3: 
                retString ="1x2Flex"
        elif SpeakersScreens == 4 and ViewersScreens == 4: 
                retString = "1x1"
        return retString 
    
def GetListOfCellIndexAndValuesForEachScreen(SpeakersScreens,ViewersScreens,ScreenNumber):
        LinkedListOfCellsIndex = list()
        LinkedListOfCellsValue = list() #Blank 0 NonBlank 1
        if SpeakersScreens == 1 and ViewersScreens == 1:
            if  ScreenNumber == 1:
                LinkedListOfCellsIndex.append(0)
                LinkedListOfCellsValue.append(1)
        elif SpeakersScreens == 1 and ViewersScreens == 2: 
                LinkedListOfCellsIndex.append(0)
                if ScreenNumber == 2:
                    LinkedListOfCellsValue.append(0)
                elif ScreenNumber == 1:
                    LinkedListOfCellsValue.append(1)
        elif SpeakersScreens == 1 and ViewersScreens == 3:
                LinkedListOfCellsIndex.append(0)
                if ScreenNumber == 2:
                    LinkedListOfCellsValue.append(0)
                elif ScreenNumber == 1:
                    LinkedListOfCellsValue.append(1)
                elif ScreenNumber == 3:
                    LinkedListOfCellsValue.append(0)
        elif SpeakersScreens == 1 and ViewersScreens == 4: 
                LinkedListOfCellsIndex.append(0)
                if ScreenNumber == 4:
                    LinkedListOfCellsValue.append(0)
                elif ScreenNumber == 2:
                    LinkedListOfCellsValue.append(0)
                elif ScreenNumber == 1:
                    LinkedListOfCellsValue.append(1)
                elif ScreenNumber == 3:
                    LinkedListOfCellsValue.append(0)
        elif SpeakersScreens == 2 and ViewersScreens == 1:
                if ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
        elif SpeakersScreens == 2 and ViewersScreens == 2:
                LinkedListOfCellsIndex.append(0)
                LinkedListOfCellsValue.append(1)
        elif SpeakersScreens == 2 and ViewersScreens == 3:
                if ScreenNumber == 2 or ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)
                elif ScreenNumber == 3:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(0)
                
        elif SpeakersScreens == 2 and ViewersScreens == 4: 
                if ScreenNumber == 4 or ScreenNumber == 3:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(0)
                elif ScreenNumber == 2 or ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)                
        elif SpeakersScreens == 3 and ViewersScreens == 1: 
                if ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(3)
                    LinkedListOfCellsIndex.append(4)
                    LinkedListOfCellsIndex.append(5)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
                    
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsIndex.append(2)
                    LinkedListOfCellsIndex.append(6)
                    LinkedListOfCellsIndex.append(7)
                    LinkedListOfCellsIndex.append(8)
                    
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    
        elif SpeakersScreens == 3 and ViewersScreens == 2:
                if ScreenNumber == 2:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(1)
                elif ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
        elif SpeakersScreens == 3 and ViewersScreens == 3:
                if (ScreenNumber == 2 )or (ScreenNumber) == 1 or (ScreenNumber == 3):      
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)
                
        elif SpeakersScreens == 3 and ViewersScreens == 4: 
                if ScreenNumber == 4 or ScreenNumber == 2 or ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)
                elif ScreenNumber == 3:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(0)
                
        elif SpeakersScreens == 4 and ViewersScreens == 1: 
                if ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(5)
                    LinkedListOfCellsIndex.append(6)
                    LinkedListOfCellsIndex.append(7)
                    LinkedListOfCellsIndex.append(8)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
                    
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsIndex.append(2)
                    LinkedListOfCellsIndex.append(3)
                    LinkedListOfCellsIndex.append(4)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsValue.append(0)
                
        elif SpeakersScreens == 4 and ViewersScreens == 2: 
                if ScreenNumber == 2 or ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsValue.append(1)
                     
        elif SpeakersScreens == 4 and ViewersScreens == 3: 
                if ScreenNumber == 2:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(0)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(1)  
                elif ScreenNumber == 1:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(1)  
                elif ScreenNumber == 3:   
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)
                    LinkedListOfCellsIndex.append(1)
                    LinkedListOfCellsValue.append(0)  
                
        elif SpeakersScreens == 4 and ViewersScreens == 4: 
                if ScreenNumber == 2 or ScreenNumber == 1 or ScreenNumber == 3 or ScreenNumber == 4:
                    LinkedListOfCellsIndex.append(0)
                    LinkedListOfCellsValue.append(1)
                    
        zipped = zip(LinkedListOfCellsIndex,LinkedListOfCellsValue)      
        return zipped
    
def FindNumberOfScreensByName(listOfNamesAndScreens,partyName):
    for i in range(len(listOfNamesAndScreens)):
        if listOfNamesAndScreens[i].GetName() ==  partyName:
            return listOfNamesAndScreens[i].GetNumOfScreen()
    return -1

def GetNumberOfScreens(c,confid,partyid,listOfNamesAndScreens = 0):
    partyName = GetPartyName(c,confid,partyid)
      
    if ("SIM_TIP" in partyName ):
        return 3
    else:
        EPSuffix = ['_1','_2','_3','_4']
        for i in range(len(EPSuffix)):
            if EPSuffix[i] in partyName:
                newPartyName = partyName.replace(EPSuffix[i],"")
                return FindNumberOfScreensByName(listOfNamesAndScreens,newPartyName)
    return 1
    
def GetPartyName(c,conf_id,partyid):
    interface = c.GetPartyInterface(conf_id,partyid)
    name = "dummy"
    if interface == "h323":                   
        name = c.GetPartyAlias(conf_id,partyid)
    elif interface == "sip":
        name = c.GetPartyName(conf_id,partyid) 
    return name

def GetPartyIdByName(c,confid,partyname):
    
    partyid = -1
    if "RPX" in partyname:
        partyid = c.GetPartyIdByAliasName(confid,partyname)
    else:
        partyid = c.GetPartyId(confid,partyname)
   
    return partyid
 
def GetScreenNumber(c,confid,partyid):
    
    partyname = GetPartyName(c,confid,partyid)
    EPSuffix = ['_1','_2','_3','_4']
    for i in range(len(EPSuffix)):
         if EPSuffix[i] in partyname: 
            return int(EPSuffix[i][1])
        
    if 'SIM_TIP' in partyname:
        return 1
    return 1  

def BuildNameAndReturnId(c,confid,changedEPId,ScreenNumber):    
    changedEPName = GetPartyName(c,confid,changedEPId)
    newChangedEPName = ""
    
    TIPNames = ['_1','_2','_3','_4','_aux']
    for i in range(len(TIPNames)): 
     if TIPNames[i] in changedEPName: 
            posToReplace = changedEPName.find(TIPNames[i])
            if posToReplace != -1:
                if 'SIM_TIP' in changedEPName and ScreenNumber == 1:
                    newChangedEPName = changedEPName.replace(TIPNames[i],"") #possible bug is aux
                else:
                    newChangedEPName = changedEPName.replace(TIPNames[i], '_'+str(ScreenNumber))
                return GetPartyIdByName(c,confid,newChangedEPName)
    
    if 'SIM_TIP' in changedEPName and ScreenNumber!=1:
            newChangedEPName = changedEPName +'_'+ str(ScreenNumber)
            return GetPartyIdByName(c,confid, newChangedEPName)
        
    return GetPartyIdByName(c,confid, changedEPName)  
    
def IsSameSpeaker(c,confid,speakerid,partyid):
     speakerName = GetPartyName(c,confid,speakerid)
     partyName = GetPartyName(c,confid,partyid)
     newSpeakerName = speakerName
     newPartyName = partyName
     isRPXOrTIP = 0
     
     if ('SIM_TIP' in speakerName) and ('SIM_TIP' in partyName):
         Suffix = ["_2","_3","_aux"]
         isRPXOrTIP = 1
     elif ('RPX' in speakerName) and ('RPX' in partyName):
         Suffix = ["_1","_2","_3","_4"]
         isRPXOrTIP = 1
         
     if (isRPXOrTIP == 1):
        for x in range(len(Suffix)): 
            if speakerName.find(Suffix[x])!=-1:
                newSpeakerName = speakerName.replace(Suffix[x], "")
            
            if partyName.find(Suffix[x])!=-1:
                newPartyName = partyName.replace(Suffix[x], "")
                
        if newSpeakerName == newPartyName:
            return True
        else: 
            return False
     else:
        return speakerName == partyName 

def IsAuxParty(c,conf_id,partyid):
    partyName = GetPartyName(c,conf_id,partyid) 
    if partyName.find("_aux") == -1:
        return 0
    else:
        return  1
           
def TestLayoutOfSIPAndH323():

    os.environ["LD_LIBRARY_PATH"] = "/mcms/Bin"
    os.environ["SASL_PATH"] = "/mcms/Bin"
    
    call(["Bin/McuCmd", "set","all","ITP_CERTIFICATION","YES"])
    call(["Bin/McuCmd", "set","all","MANAGE_TELEPRESENCE_ROOM_SWITCH_LAYOUTS","YES"])
            
    c = McmsConnection()
    c.Connect()
        
    IsConfPartyUnderValgrind = False
    if (c.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True
        
    profile_name_str = "room_switch_prof"
    profile_file_name_str = "Scripts/ManageTelepresenceInternaly/add_roomswitch_on_prof.xml"
    conf_name_str = "rs_conf"
    conf_file_name_str = 'Scripts/CreateCPConfWith4DialInParticipants/AddCpConf.xml'
    party_file_str = "Scripts/ManageTelepresenceInternaly/AddH323DialInTelepresenceParty.xml"
    
    conf_id = AddRoomSwitchOnConf(c,conf_name_str)
    
    listOfNamesAndScreens = list()
    
    #TIP participants 3 screen
    AddSIPPartyDialOut(c,conf_id,"Nizar_SIM_TIP",1)
    CurrentSpeaker = c.GetPartyId(conf_id,"Nizar_SIM_TIP")
    if(IsConfPartyUnderValgrind):
    	sleep(3)
    AddSIPPartyDialOut(c,conf_id,"Ron_SIM_TIP",5)
    PreviousSpeaker = c.GetPartyId(conf_id,"Ron_SIM_TIP")
    PreviousSpeakerScreens = GetNumberOfScreens(c,conf_id,PreviousSpeaker)
    if(IsConfPartyUnderValgrind):
    	sleep(3)    
    #Non-TIP 1 screen 
    AddSIPPartyDialOut(c,conf_id,"Anat",25)
    listOfNamesAndScreens.append(EP("Anat",1))
    if(IsConfPartyUnderValgrind):
    	sleep(3)    
    #RPX EP 2 screens
    AddH323PartyDialIn(c,conf_name_str,2,"Ron_RPX", "Polycom RPX")
    listOfNamesAndScreens.append(EP("Ron_RPX",2))
    if(IsConfPartyUnderValgrind):
    	sleep(3)    
    #RPX EP 3 screens
    AddH323PartyDialIn(c,conf_name_str,3,"Nizar_RPX", "Polycom RPX")
    listOfNamesAndScreens.append(EP("Nizar_RPX",3))
    if(IsConfPartyUnderValgrind):
    	sleep(3)    
    #RPX EP 4 screens
    AddH323PartyDialIn(c,conf_name_str,4,"Jawad_RPX", "Polycom RPX")
    listOfNamesAndScreens.append(EP("Jawad_RPX",4))
    
    sleep(5)        
    
    listIDs = c.GetPartyIDs(conf_id)
    num_of_parties = len(listIDs)
    
    #Change each speaker
    for x in range(num_of_parties):      
        if IsAuxParty(c,conf_id,listIDs[x]):
            continue
        
        if IsSameSpeaker(c,conf_id, listIDs[x],CurrentSpeaker)==False:
            PreviousSpeaker = CurrentSpeaker
            PreviousSpeakerScreens = GetNumberOfScreens(c,conf_id,PreviousSpeaker,listOfNamesAndScreens)
            
        CurrentSpeaker = listIDs[x]
            
        SpeakersScreens =  GetNumberOfScreens(c,conf_id,CurrentSpeaker,listOfNamesAndScreens)
        call(["Bin/McuCmd", "speaker_ind","ConfParty",str(conf_id),str(CurrentSpeaker)])
        
        if(IsConfPartyUnderValgrind):
    		sleep(8)
        sleep(2)
            
        print "New Audio Speaker PartyId = " + str(CurrentSpeaker) + " Conf id: " + str(conf_id)
        
        #Pass on each participant layout and check if layout is okay 
        for y in range(num_of_parties): 
            if 'aux' in str(GetPartyName(c,conf_id,listIDs[y])): 
                continue
            
            ViewersScreens = GetNumberOfScreens(c,conf_id,int(listIDs[y]),listOfNamesAndScreens)
            
            ScreenNumber = GetScreenNumber(c,conf_id,int(listIDs[y]))
            if IsSameSpeaker(c,conf_id, listIDs[y],CurrentSpeaker): 
                SpeakerScreens = PreviousSpeakerScreens
            else:
                SpeakerScreens = SpeakersScreens
                    
            LayoutTypeViewerMustSee = GetLayoutTypeAccordingToSpeakerAndViewerScreens(SpeakerScreens,ViewersScreens)
            
            print "SpeakerId: " + listIDs[x] + " SpeakerScreens: " + str(SpeakerScreens) + " ViewerId: " + listIDs[y] + " ViewerScreens: " + str(ViewersScreens) + " LayoutTypeViewerMustSee: " + LayoutTypeViewerMustSee
            
            c.WaitPartySeesConfLayout(conf_id,listIDs[y],LayoutTypeViewerMustSee)
            
            zipped = GetListOfCellIndexAndValuesForEachScreen(SpeakerScreens,ViewersScreens,ScreenNumber)
                 
            x2,y2=zip(*zipped)
            listOfIndexes = list(x2)
            BlankOrNonBlankList = list(y2)
            
            for i in range(len(listOfIndexes)):
                CellIndexToCheck = listOfIndexes[i]
                isNonBlank = BlankOrNonBlankList[i] 
                if isNonBlank == 1:
                    RightScreenNumber = GetNumberOfRelevantScreenByCellIndex(SpeakerScreens,ViewersScreens,CellIndexToCheck,ScreenNumber)                       
                    if IsSameSpeaker(c,conf_id,listIDs[y],CurrentSpeaker):
                        PartyIdMustSeeInCell = BuildNameAndReturnId(c,conf_id,PreviousSpeaker,RightScreenNumber)
                        print "ViewerId: " + listIDs[y] + " ViewerCellIndexToCheck: " + str(CellIndexToCheck) + " must see speakerId: " + PartyIdMustSeeInCell  
                        c.WaitPartySeesPartyInCell(conf_id,int(listIDs[y]),PartyIdMustSeeInCell,CellIndexToCheck)
                    else:
                        PartyIdMustSeeInCell = BuildNameAndReturnId(c,conf_id,CurrentSpeaker,RightScreenNumber)
                        print "ViewerId: " + listIDs[y] + " ViewerCellIndexToCheck: " + str(CellIndexToCheck) + " must see speakerId: " + PartyIdMustSeeInCell
                        c.WaitPartySeesPartyInCell(conf_id,int(listIDs[y]),PartyIdMustSeeInCell,CellIndexToCheck)
                else:
                    print "ViewerId: " + listIDs[y] + " ViewerCellIndexToCheck: " + str(CellIndexToCheck) + " must see black screen"                 
                    c.WaitPartySeesPartyInCell(conf_id,int(listIDs[y]),-1,CellIndexToCheck)
                print ''
                       
            #raw_input("Press Enter to continue...")
   
    c.DeleteConf(conf_id)
    c.WaitAllConfEnd()
    c.DelProfileByName(profile_name_str)  
    
TestLayoutOfSIPAndH323()
