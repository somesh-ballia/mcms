#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *

#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_NO_v100.0.cfs"           
#------------------------------------------------------------------------------
def WaitAllOtherOngoingSeePartyInCell(connection, confid, partyToForce, cellToForce, num_retries=30):
    print "Wait until all other ongoing parties see party: " + str(partyToForce) + " in cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        all_images_seen_in_cell = ""
        wanted_all_images_seen_in_cell = ""
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        for party in ongoing_parties:
            currentPartyId = party.getElementsByTagName("ID")[0].firstChild.data
            if currentPartyId != str(partyToForce):
                seenImageInCell = party.getElementsByTagName("CELL")[cellToForce].getElementsByTagName("SOURCE_ID")[0].firstChild.data
                all_images_seen_in_cell += (seenImageInCell + " ")
                wanted_all_images_seen_in_cell += (str(partyToForce) +" ")
                if seenImageInCell != partyToForce:
                    if (retry == num_retries):
                        print connection.xmlResponse.toprettyxml()
                        connection.Disconnect()
                        sys.exit("Party does not see Conf Level force: "+ partyToForce +", But Party : " + seenImageInCell)
        if all_images_seen_in_cell == wanted_all_images_seen_in_cell:
             break
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
                    
#------------------------------------------------------------------------------
def SetConfLevelForceTest(connection, confid, confLayoutType, partyToForce, cellToForce):
    print "Conference ID: "+ confid + " Changing Conf Level Force: PartyId: " + str(partyToForce) + " Forced to Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType) 
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "forced"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = str(partyToForce)     
    connection.Send()
    WaitAllOtherOngoingSeePartyInCell(connection, confid, partyToForce, cellToForce)
    return

#------------------------------------------------------------------------------
def RemoveConfLevelForce(connection, confid, confLayoutType, cellToForce):
    print "Conference ID: "+ confid + " Removing Conf Level Force from Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType) 
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "auto"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = "-1"     
    connection.Send()
    return 
        
#------------------------------------------------------------------------------
def SetPartyLevelForceTest(connection, confid, partyid, confLayoutType, partyToForce, cellToForce):
    print "Changing Party Level Force: PartyId: " + str(partyid) + " Force Party " + str(partyToForce) + "to Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/PersonalLayout/ChangePersonalLayout.xml')
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","LAYOUT_TYPE","conference")
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType)  
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "forced"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = str(partyToForce)     
    connection.Send()
    connection.WaitPartySeesPartyInCell(confid, partyid, partyToForce, cellToForce)
    return

#------------------------------------------------------------------------------
def RemovePartyLevelForce(connection, confid, partyid, confLayoutType, cellToForce):
    print "Remove Party Level Force: PartyId: " + str(partyid) + " From Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/PersonalLayout/ChangePersonalLayout.xml')
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","LAYOUT_TYPE","conference")
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType)  
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "auto"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = "-1"     
    connection.Send()
    return
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         3,
                         60,
                         "false",
                         "NONE",
                         1)

#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
   c.Disconnect()                
   sys.exit("Can not monitor conf:" + status)

c.WaitAllOngoingNotInIVR(confid)

print
print "Start Test Force Video in Conf Level 2x2..."
confLayoutType = "2x2"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

partyToForce = 0;
cellToForce  = 3;
c.ChangeDialOutVideoSpeaker(confid, partyToForce)
SetConfLevelForceTest(c, confid, confLayoutType, partyToForce, cellToForce)    

RemoveConfLevelForce(c, confid, confLayoutType, cellToForce) 
#the removal should not change the places in the layout since it is symmetric layout
WaitAllOtherOngoingSeePartyInCell(c, confid, partyToForce, cellToForce)

print
print "Start Test Force Video in Conf Level 1and5..."
confLayoutType = "1and5"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

partyToForce = 1;
cellToForce  = 5;
c.ChangeDialOutVideoSpeaker(confid, partyToForce)
SetConfLevelForceTest(c, confid, confLayoutType, partyToForce, cellToForce)    

RemoveConfLevelForce(c, confid, confLayoutType, cellToForce) 
#the removal should change the places in the layout since it is asymmetric layout and this is the speaker
WaitAllOtherOngoingSeePartyInCell(c, confid, partyToForce, 0)

print
print "Start Test Force Video in Party Level 3x3..."
confLayoutType = "3x3"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

partyTested = 2;
partyToForce = 0;
cellToForce  = 4;
c.ChangeDialOutVideoSpeaker(confid, partyToForce)
SetPartyLevelForceTest(c, confid, partyTested, confLayoutType, partyToForce, cellToForce)    

RemovePartyLevelForce(c, confid, partyTested, confLayoutType, cellToForce) 
#the removal should not change the places in the layout since it is symmetric layout
c.WaitPartySeesPartyInCell(confid, partyTested, partyToForce, cellToForce)

print
print "Start Test Force Video in Party Level 1and7..."
confLayoutType = "1and7"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

partyTested = 0;
partyToForce = 1;
cellToForce  = 7;
c.ChangeDialOutVideoSpeaker(confid, partyToForce)
SetPartyLevelForceTest(c, confid, partyTested, confLayoutType, partyToForce, cellToForce)    

RemovePartyLevelForce(c, confid, partyTested, confLayoutType, cellToForce)  
#the removal should change the places in the layout since it is asymmetric layout and this is the speaker
c.WaitPartySeesPartyInCell(confid, partyTested, partyToForce, 0)

print    
print "Start Deleting Conference..."
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()


