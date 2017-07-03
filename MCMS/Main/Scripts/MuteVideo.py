#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *

#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_NO_v100.0.cfs" 
          
#------------------------------------------------------------------------------
def WaitNoOneSeesPartyInLayout(connection, confid, partyid, num_retries=30):
    print "Wait until no one sees party: " + str(partyid) + " in layout"
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        isImageSeen = 0
        for party in ongoing_parties:
            imageSources = party.getElementsByTagName("CELL")
            for image in imageSources:
                if image.getElementsByTagName("SOURCE_ID")[0].firstChild.data == str(partyid):
                    isImageSeen = 1
                    break
        if (isImageSeen == 0):
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Some One still sees image: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    

#------------------------------------------------------------------------------
def WaitPartyVideoAppearsAsMuted(connection, confid, partyid, num_retries=30):
    print "Wait until party Appears with Video Mute: " + str(partyid) 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if(ongoing_parties[partyid].getElementsByTagName("VIDEO_MUTE")[0].firstChild.data == "true"):
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Video Muted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
                    
#------------------------------------------------------------------------------
def MutePartyVideoTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","VIDEO_MUTE","true")  
    connection.Send()
    WaitPartyVideoAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def WaitPartyVideoAppearsAsUnMuted(connection, confid, partyid, num_retries=30):
    print "Wait until party Appears with Video UnMute: " + str(partyid) 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if(ongoing_parties[partyid].getElementsByTagName("VIDEO_MUTE")[0].firstChild.data == "false"):
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Video UnMuted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
        
#------------------------------------------------------------------------------
def WaitAllButSelfSeesPartyInLayout(connection, confid, partyid, num_retries=30):
    print "Wait until all see party: " + str(partyid) + " in layout"
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        isImageSeen = 0
        for party in ongoing_parties:
            if (party.getElementsByTagName("ID")[0].firstChild.data == partyid):
                break
            imageSources = party.getElementsByTagName("CELL")
            for image in imageSources:
                if image.getElementsByTagName("SOURCE_ID")[0].firstChild.data == str(partyid):
                    isImageSeen += 1
                    break
        if isImageSeen == (len(ongoing_parties)-1):
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Some One still does not see image: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
        
#------------------------------------------------------------------------------
def UnMutePartyVideoTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " UnMuting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","VIDEO_MUTE","false")  
    connection.Send()
    WaitPartyVideoAppearsAsUnMuted(connection, confid, partyid)
    #WaitAllButSelfSeesPartyInLayout(connection, confid, partyid)
    return
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         5,
                         60,
                         "false")

#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
   c.Disconnect()                
   sys.exit("Can not monitor conf:" + status)

print
print "Start Test Mute Video in Conf Level 2x2..."
confLayoutType = "2x2"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

mutedParty = 0;
WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
MutePartyVideoTest(c, confid, mutedParty)
WaitNoOneSeesPartyInLayout(c, confid, mutedParty)
UnMutePartyVideoTest(c, confid, mutedParty)
WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)

print
print "Start Test Mute Video in Conf Level 1x1..."
confLayoutType = "1x1"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

mutedParty = 1;
c.ChangeDialOutVideoSpeaker(confid, mutedParty)
WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
MutePartyVideoTest(c, confid, mutedParty)
WaitNoOneSeesPartyInLayout(c, confid, mutedParty)
c.ChangeDialOutVideoSpeaker(confid, 0)
c.ChangeDialOutVideoSpeaker(confid, 2)
WaitNoOneSeesPartyInLayout(c, confid, mutedParty)
UnMutePartyVideoTest(c, confid, mutedParty)
WaitNoOneSeesPartyInLayout(c, confid, mutedParty)
MutePartyVideoTest(c, confid, mutedParty)
c.ChangeDialOutVideoSpeaker(confid, mutedParty)
WaitNoOneSeesPartyInLayout(c, confid, mutedParty)

print
print "Start Change Conf Layout to  1and5 while party in Mute Video..."
confLayoutType = "1and5"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)
WaitNoOneSeesPartyInLayout(c, confid, mutedParty)

UnMutePartyVideoTest(c, confid, mutedParty)
WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)

print    
print "Start Deleting Conference..."
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()


