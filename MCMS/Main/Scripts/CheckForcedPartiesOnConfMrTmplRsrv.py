#!/mcms/python/bin/python

# Force layout for parties in meeting room 
# Initially developed for: "layout for defined inactive meeting room (A360=46955) - KAF"

from PartyUtils.H323PartyUtils import *

from time import time


#############################################################################
# Test Script which Awakes a Meeting room by an undefined party
#  
# Date: 20/01/05
# By  : Nizar / Yael A 
#############################################################################

from McmsConnection import *


def GetMeetingRoom(connection,mrId):
    connection.LoadXmlFile('Scripts/TestMRCorruptedParams/GetMr.xml')
    connection.ModifyXml("GET_MEETING_ROOM","ID",mrId)
    connection.Send()
    #get the changed value
    #mrRsrvList = connection.xmlResponse.getElementsByTagName("RESERVATION")
    return connection.xmlResponse



def CreateMRWithParties(connection,mrName,sameLayout):
   
    profId = -1
    if sameLayout == False:
        profId = connection.AddProfile("MrProfile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")
    else:
        profId = connection.AddProfile("MrProfilesameLayout", "Scripts/AwakeMrByUndef/CreateNewProfile.xml", True)

                
    connection.CreateMR(mrName, profId,"Scripts/CheckForcedPartiesOnConfMrTmplRsrv/CreateMrWithParties.xml")

    #Get the Mr numeric id when mr was already added
    mrNumericId = ""
    mrId = ""
    mrId,mrNumericId = connection.WaitMRCreated(mrName,10)
    
    sleep(1)
    return profId,mrId,mrNumericId

def CreateRserveWithParties(connection,mrName):
   
    profId = connection.AddProfile("ResrvProfile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")
          
    resId = connection.CreateRes(mrName, profId, time(),"",0,0,"Status OK",0,"Scripts/CheckForcedPartiesOnConfMrTmplRsrv/CreateRsrvWithParties.xml")
                             
    sleep(1)
    
    return profId,resId


def CleanMrAndConf(connection, mrConfId, mrId, profId ):
    connection.DeleteConf(mrConfId)
    connection.WaitAllConfEnd()
    
    connection.DelReservation(mrId,"Scripts/AwakeMrByUndef/RemoveMr.xml")
    
    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")
    sleep(1)
    return

def CleanTemplateAndConf(connection, tempConfId, tempId, profId):
    connection.DeleteConf(tempConfId)
    connection.WaitAllConfEnd()
    
    connection.DelConfTemplate(tempId, "Scripts/AddRemoveConfTemplate/DelConfTemplate.xml")
    
    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")
    sleep(1)
    return
    
def CleanReservation(connection, resrvId, profId ):    
    connection.DeleteConf(resrvId)

    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")
    sleep(1)
    return
    
def CleanConference(connection, confId, profId):
    connection.DeleteConf(confId)
    connection.WaitAllConfEnd()
    
    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")
    sleep(1)
    return
    
    


def WaitCellIsMarkedAsAutoScan(self,confid,newConfLayout,autoScanCell,num_retries=30):
    print "Wait until cell number: " +autoScanCell +" is marked as auto scan in layout " + newConfLayout    
    
    self.LoadXmlFile('Scripts/TransConf2.xml')
    self.ModifyXml("GET","ID",confid)
    
    for retry in range(num_retries+1):        
        self.Send()
        confLayout = self.xmlResponse.getElementsByTagName("LAYOUT")
        force_list = self.xmlResponse.getElementsByTagName("FORCE")
    for force in force_list:        
        if(force.getElementsByTagName("LAYOUT")[0].firstChild.data == confLayout):
            forceToCheck=force 
            
            
            partyIdInCell =  forceToCheck.getElementsByTagName("CELL")[ForcedCell].getElementsByTagName("FORCE_ID")[0].firstChild.data
            partyIdNameCell =  forceToCheck.getElementsByTagName("CELL")[ForcedCell].getElementsByTagName("FORCE_ID")[0].firstChild.data
            



def CheckForcesForEachParticipant(c,conf_name,meetingRoomParty, sameLayout = False):
    
    conf_id = c.GetConfIdByName(conf_name)    
    listIDs,listNames = c.GetPartyIDsAndNames(conf_id)
    
    forceToCheck = c.GetForceElementFromConference(conf_id)            
        
    expectedNamesInCells = list()
    expectedIdsInCells = list()
        
    ActuallyChecked = list()        
    expectedNamesInCells = ["P1", "P2", "P3"]    
    
    num_of_cells = len(expectedNamesInCells)
    num_of_parties = len(listIDs)
    
    #actually maybe expectedIdsInCells is not needed we can check it only by names
    for cellNum in range(0,num_of_cells):
        for partyNum in range(0,num_of_parties):
            if expectedNamesInCells[cellNum] == listNames[partyNum]:
                expectedIdsInCells.append(listIDs[partyNum])
                print "party id " + listIDs[partyNum] + " is " + listNames[partyNum]
        
    for i in range(0,num_of_parties):
        partyID = listIDs[i]
        partyName = listNames[i]
        
        if (partyName in expectedNamesInCells):
            ActuallyChecked.append(partyName)  
            for CellIndexToCheck in range(0,num_of_cells):
                if (expectedNamesInCells[CellIndexToCheck] != partyName) or sameLayout:
                    print "Party " + partyName + " id " + str(partyID) + " is expected to  have party " +  str(expectedNamesInCells[CellIndexToCheck]) + " id " + str(expectedIdsInCells[CellIndexToCheck]) + " in cell " + str(CellIndexToCheck)
                                        
                    partyIdInCell =  forceToCheck.getElementsByTagName("CELL")[CellIndexToCheck].getElementsByTagName("FORCE_ID")[0].firstChild.data
                    partyIdNameCell =  forceToCheck.getElementsByTagName("CELL")[CellIndexToCheck].getElementsByTagName("FORCE_NAME")[0].firstChild.data

                    print "Actual result for Party " + partyName + " id " + str(partyID) + " see " +  str(partyIdNameCell) + " id " + str(partyIdInCell) + " in cell " + str(CellIndexToCheck)
                   
                    if (str(partyIdInCell) != str(expectedIdsInCells[CellIndexToCheck])):
                       ScriptAbort("Party " + partyName + " id " + str(partyID) + " doesnt see party " +  str(expectedNamesInCells[CellIndexToCheck]) + " id " + str(expectedIdsInCells[CellIndexToCheck]) + " in cell " + str(CellIndexToCheck))
                else:
                     print "Should become auto. Party  " + partyName + " cell " + str(CellIndexToCheck)
                                          
        else:
            print "party " + partyName + " is not in list"
            
                 
    if (len(expectedNamesInCells) != len(ActuallyChecked)):
        ScriptAbort("Test failed. not all expected parties are in list ")                 
    return conf_id 
    


    
def CreateTemplateWithParties(connection,tmplName):
    
    #add a new profile
    profId = connection.AddProfile("TempProfile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")

    #add a new template
    connection.CreateConfTemplate(tmplName,profId,"Scripts/CheckForcedPartiesOnConfMrTmplRsrv/CreateTemplateWithParties.xml")
    tmplId  = connection.WaitConfTemplateCreated(tmplName)

    return tmplId,profId

def CreateConfWithParties(connection,mrName):    
    #add a new profile
    profId = connection.AddProfile("MrProfile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")

    #Adding a new conference
    connection.CreateMR(mrName, profId, "Scripts/CheckForcedPartiesOnConfMrTmplRsrv/CreateConfWithParties.xml")
    
    sleep(1)
    
    
    return profId

def TestMrWithPartiesForces(connection, sameLayout):
    print "Testing Meeting Room with party force"
    
    profId=""
    mrId = ""
    mrConfId=""
    sentRequest=""
    
    MrName = "mrRsrv"
    profId,mrId,NumericId = CreateMRWithParties(c,MrName,sameLayout)
    PartyName = "Nizar"
  
    connection.SimulationAddH323Party(PartyName, NumericId,"FULL CAPSET")
    connection.SimulationConnectH323Party(PartyName)
    
    sleep(4)
    
    mrConfId = CheckForcesForEachParticipant(connection,MrName,True,sameLayout)    
    
    sleep(1)

    CleanMrAndConf(c, mrConfId, mrId, profId )
      
    connection.SimulationDeleteH323Party(PartyName)

    sleep(1)
    
    print "Testing Meeting Room with party force is passed successfully"
    
def TestTemplateWithPartiesForces(connection):
    profId=""
    mrId = ""
    mrConfId=""
    sentRequest=""

    print "Testing Template with party forces"
    tmplName = "Nizar"

    tmplId,profId = CreateTemplateWithParties(connection,tmplName)
    
    connection.CreateConfFromTemplate(tmplName,tmplId,profId,"Scripts/CheckForcedPartiesOnConfMrTmplRsrv/CreateConfFromTemplate.xml")
    
    sleep(4)
    
    tempConfId = CheckForcesForEachParticipant(connection,tmplName, False)
     
         
    CleanTemplateAndConf(connection, tempConfId, tmplId, profId)
    
    print "Testing Template with party force is passed successfully"
    
def TestConfWithPartiesForces(connection):
    print "Testing Conference with predefined forced parties"
    
    profId=""
    mrId = ""
    mrConfId=""
    sentRequest=""
    
    MrName = "mrRsrv"
    profId = CreateConfWithParties(c,MrName)
    
    sleep(4)
    
    ConfId = CheckForcesForEachParticipant(connection,MrName, False)
    
    sleep(1)
    
    CleanConference(connection, ConfId, profId)
    
    sleep(1)
    print "Testing Conference with predefined forced parties passed successfully"

def TestReserveConferencePartiesForces(connection):
    print "Testing Reserve conference with predefined forced parties"
    
    profId=""
    mrId = ""
    mrConfId=""
    sentRequest=""
    
    ResrvName = "myReserve"
    
    profId,resrvId = CreateRserveWithParties(c, ResrvName)

    sleep(4)
    
    resrvId = CheckForcesForEachParticipant(connection, ResrvName, False)
    
    sleep(1)
    
    CleanReservation(connection, resrvId, profId)
              
    sleep(1)
    
    print "Testing Reservation conference with predefined forced parties passed successfully"
                    
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestMrWithPartiesForces(c, False)

TestMrWithPartiesForces(c, True)

TestTemplateWithPartiesForces(c)

TestConfWithPartiesForces(c)

TestReserveConferencePartiesForces(c)

c.Disconnect()
