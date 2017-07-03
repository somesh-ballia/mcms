#!/mcms/python/bin/python

#############################################################################
# Eror handling Script which Try to Add Conf with Min Particepent defined to various values
# 
# 1.Zero
# 3.In Limit - 20
# 2.Over Limit - 21
# 3.Parameter Limit 30 
#
# Date: 31/01/05
# By  : Yoella.

#############################################################################

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

from McmsConnection import *
from subprocess import call
import os
            
def AddRoomSwitchOnConf(connection,conf_name = "room_switch_conf",conf_file_name = "Scripts/ManageTelepresenceInternaly/add_roomswitch_on_conf.xml",profile_name = "room_switch_prof",profile_file_name = "Scripts/ManageTelepresenceInternaly/add_roomswitch_on_prof.xml" ):
    roomswitch_prof_id = connection.IsProfileNameExists(profile_name)    
    if roomswitch_prof_id == -1:
        roomswitch_prof_id = connection.AddProfile(profile_name,profile_file_name)
        
    conf_id = connection.GetConfIdByName (conf_name)
   
    if conf_id == -1:
        conf_id = connection.CreateConfFromProfile(conf_name, roomswitch_prof_id,conf_file_name)     
        conf_id = connection.WaitConfCreated(conf_name)
 
    return conf_id

def AddSIPPartyDialOut(connection,confId,party_name = "ppp_",startIP = 0):
    
    partyip =  "123.123.0." + str(startIP)
    print "Adding Party..." + party_name + " ip: " + partyip
    connection.AddVideoParty(confId, party_name, partyip,True)
            
    sleep(5)
        
    
def AddH323PartyDialIn(connection,confName,num_of_screens = 3, party_name = "ppp_",ManuName = "None"):

    for i in range(num_of_screens):
        if num_of_screens == 1:
            PartyName = party_name
        else:
            PartyName = party_name + '_' + str(i+1)
    
        connection.SimulationAddH323Party(PartyName, confName,"FULL CAPSET",0,ManuName,PartyName)
        connection.SimulationConnectH323Party(PartyName)
        
        #if (c.IsProcessUnderValgrind("ConfParty")):
        #    c.WaitPartyConnected(confId,party_id_list[x], num_retries)
    sleep(10) 

def AddH323PartyDialOut(connection,confid,num_of_screens = 3, party_name = "RPX_",startIP = 0):
    
    for i in range(num_of_screens):
        partyip =  "123.123.0." + str(startIP+i)
        
        if num_of_screens == 1:
            PartyName = party_name
        else:
            PartyName = party_name + '_' + str(i)
   
        AliasName = PartyName 
        connection.AddVideoParty(confid, PartyName, partyip,False,"auto","automatic",AliasName)
        
        #if (c.IsProcessUnderValgrind("ConfParty")):
        #    c.WaitPartyConnected(confId,party_id_list[x], num_retries)
    sleep(5)

def ChangeSpeakerForEachEP(c,conf_id,listIDs,num_of_parties):
    #Change speaker for each participants 
    counter = 0 
    print "num_of_parties: " + str(num_of_parties)
    #Change speaker on for each participant
    for x in range(num_of_parties):
        c.Disconnect()
        c.Connect()
        #Get the party id
        currPartyID = listIDs[x]
        if (currPartyID < 0):
            c.Disconnect()                
            sys.exit("Error:Can not find partry id of party: ")
        print "found party id = "+str(currPartyID)
        #speaker_ind ConfParty [conference monitor ID] [party monitor ID]
        call(["Bin/McuCmd", "speaker_ind","ConfParty",str(conf_id),str(currPartyID)])
        print "New Audio Speaker PartyId = " + str(currPartyID) + "Conf id: " + str(conf_id)
        sleep(10)
    
def TIPWithNonTIPSipDialOutTest(c):
    profile_name_str = "room_switch_prof"
    profile_file_name_str = "Scripts/ManageTelepresenceInternaly/add_roomswitch_on_prof.xml"
    conf_name_str = "rs_conf"
    conf_file_name_str = 'Scripts/CreateCPConfWith4DialInParticipants/AddCpConf.xml'
    party_file_str = "Scripts/ManageTelepresenceInternaly/AddH323DialInTelepresenceParty.xml"

    conf_id = AddRoomSwitchOnConf(c,conf_name_str)
    
    #Adding three TIP participants
    AddSIPPartyDialOut(c,conf_id,"Nizar_SIM_TIP",1)
    AddSIPPartyDialOut(c,conf_id,"Ron_SIM_TIP",5)
    AddSIPPartyDialOut(c,conf_id,"Anat_SIM_TIP",10)
    
    if (c.IsProcessUnderValgrind("ConfParty")):
    	c.WaitAllOngoingConnected(conf_id,40)
    
    #Adding three Non-TIP participants
    AddSIPPartyDialOut(c,conf_id,"Ron",15)
    AddSIPPartyDialOut(c,conf_id,"Nizar",20)
    AddSIPPartyDialOut(c,conf_id,"Anat",25)
    
    listIDs = c.GetPartyIDs(conf_id)
    num_of_parties = len(listIDs)
    num_retries = 10
    
    c.WaitAllOngoingConnected(conf_id,num_retries*num_of_parties)
      
    #Change speaker for each participants
    ChangeSpeakerForEachEP(c,conf_id,listIDs,num_of_parties)
    
    for x in range(num_of_parties):
       c.DeleteParty(conf_id,listIDs[x])
      
    c.WaitUntillPartyDeleted(conf_id,num_retries*num_of_parties)
    c.DeleteConf(conf_id)
    c.WaitAllConfEnd()
    c.DelProfileByName(profile_name_str)


def H323DialOutTest(c):
    profile_name_str = "room_switch_prof"
    profile_file_name_str = "Scripts/ManageTelepresenceInternaly/add_roomswitch_on_prof.xml"
    conf_name_str = "rs_conf"
    conf_file_name_str = 'Scripts/CreateCPConfWith4DialInParticipants/AddCpConf.xml'
    party_file_str = "Scripts/ManageTelepresenceInternaly/AddH323DialInTelepresenceParty.xml"

    conf_id = AddRoomSwitchOnConf(c,conf_name_str)
    
    #Adding three RPX EP
    AddH323PartyDialOut(c, conf_id ,3,"RPX_Nizar",0)
    AddH323PartyDialOut(c, conf_id ,3,"RPX_Anat",5)
    AddH323PartyDialOut(c, conf_id ,3,"RPX_Ron",10)
    
    #Adding three Non EP
    AddH323PartyDialOut(c, conf_id ,1,"Nizar",15)
    AddH323PartyDialOut(c, conf_id ,1,"Anat",20)
    AddH323PartyDialOut(c, conf_id ,1,"Ron",25)
    
    listIDs = c.GetPartyIDs(conf_id)
    num_of_parties = len(listIDs) 
    num_retries = 10
    
    c.WaitAllOngoingConnected(conf_id,num_retries*num_of_parties)
    
    #Change speaker for each participants       
    ChangeSpeakerForEachEP(c,conf_id,listIDs,num_of_parties)
    
    for x in range(num_of_parties):
       c.DeleteParty(conf_id,listIDs[x])

    c.WaitUntillPartyDeleted(conf_id,num_retries*num_of_parties)
    
    c.DeleteConf(conf_id)
    c.WaitAllConfEnd()
    c.DelProfileByName(profile_name_str) 

def H323DialInTest(c):
   
    profile_name_str = "room_switch_prof"
    profile_file_name_str = "Scripts/ManageTelepresenceInternaly/add_roomswitch_on_prof.xml"
    conf_name_str = "rs_conf"
    conf_file_name_str = 'Scripts/CreateCPConfWith4DialInParticipants/AddCpConf.xml'
    party_file_str = "Scripts/ManageTelepresenceInternaly/AddH323DialInTelepresenceParty.xml"

    conf_id = AddRoomSwitchOnConf(c,conf_name_str)
    
    #Adding three RPX 
    AddH323PartyDialIn(c,conf_name_str,4,"Ron_RPX", "Polycom RPX")
    AddH323PartyDialIn(c,conf_name_str,4,"Nizar_RPX","Polycom RPX")
    AddH323PartyDialIn(c,conf_name_str,4,"Anat_RPX","Polycom RPX")
    
    #Adding three regular participants
    AddH323PartyDialIn(c,conf_name_str,1,"Ron")
    AddH323PartyDialIn(c,conf_name_str,1,"Nizar")
    AddH323PartyDialIn(c,conf_name_str,1,"Anat")
    
    #Change speaker for each participants
    listIDs = c.GetPartyIDs(conf_id)
    num_of_parties = len(listIDs) 
    num_retries = 10
    
    c.WaitAllOngoingConnected(conf_id,num_retries*num_of_parties)
        
    ChangeSpeakerForEachEP(c,conf_id,listIDs,num_of_parties)
    
    for x in range(num_of_parties):
       c.DeleteParty(conf_id,listIDs[x])

    c.WaitUntillPartyDeleted(conf_id,num_retries*num_of_parties)
    
    c.DeleteConf(conf_id)
    c.WaitAllConfEnd()
    c.DelProfileByName(profile_name_str)
##--------------------------------------- TEST ---------------------------------

if __name__ == '__main__':
    os.environ["LD_LIBRARY_PATH"] = "/mcms/Bin"
    os.environ["SASL_PATH"] = "/mcms/Bin"
    
    c = McmsConnection()
    c.Connect()
    sleep(2)
    TIPWithNonTIPSipDialOutTest(c)

    sleep(2)
    H323DialInTest(c)

    sleep(2)
    H323DialOutTest(c)
    c.Disconnect()
