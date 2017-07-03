#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *
          
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
            print "party [" + str(partyid) + "] is Video Muted" 
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Video Muted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
                    
#------------------------------------------------------------------------------
def WaitPartyAudioAppearsAsMuted(connection, confid, partyid, num_retries=30):
    print "Wait until party Appears with Audio Mute: " + str(partyid) 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if(ongoing_parties[partyid].getElementsByTagName("AUDIO_MUTE")[0].firstChild.data == "true"):
            print "party [" + str(partyid) + "] is Audio Muted" 
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Audio Muted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
                    
#------------------------------------------------------------------------------
def MutePartyVideoTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.Send()
    WaitPartyVideoAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def MutePartyAudioTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Audio: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.Send()
    WaitPartyAudioAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def SipMutePartyVideoPortTest(connection,confid, partyid, port):
    print "Conference ID: "+ confid + " Muting Party Video by Port, PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.ModifyXml("SIP_MUTE","MUTE_BY_PORT_VIDEO",port) 
    connection.Send()
#    WaitPartyVideoAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def SipMutePartyAudioPortTest(connection,confid, partyid, port):
    print "Conference ID: "+ confid + " Muting Party Audio by Port, PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.ModifyXml("SIP_MUTE","MUTE_BY_PORT_AUDIO",port) 
    connection.Send()
#    WaitPartyAudioAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def SipMutePartyVideoDirectionTest(connection,confid, partyid, direction):
    print "Conference ID: "+ confid + " Muting Party Video by Direction (" + direction + "), PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.ModifyXml("SIP_MUTE","MUTE_BY_DIRECTION_VIDEO",direction) 
    connection.Send()
#    WaitPartyVideoAppearsAsMuted(connection, confid, partyid) // currently I don't know to check that the mute action occur
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def SipMutePartyAudioDirectionTest(connection,confid, partyid, direction):
    print "Conference ID: "+ confid + " Muting Party Audio by Direction (" + direction + "), PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.ModifyXml("SIP_MUTE","MUTE_BY_DIRECTION_AUDIO",direction) 
    connection.Send()
 #   WaitPartyAudioAppearsAsMuted(connection, confid, partyid)// currently I don't know to check that the mute action occur
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def SipMutePartyVideoInactiveTest(connection,confid, partyid, state):
    print "Conference ID: "+ confid + " Muting Party Video by state (" + state + "), PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.ModifyXml("SIP_MUTE","MUTE_BY_INACTIVE_VIDEO",state) 
    connection.Send()
#    WaitPartyVideoAppearsAsMuted(connection, confid, partyid)// currently I don't know to check that the mute action occur
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def SipMutePartyAudioInactiveTest(connection,confid, partyid, state):
    print "Conference ID: "+ confid + " Muting Party Audio by state (" + state + "), PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("SIP_MUTE","PARTY_NAME",partyname) 
    connection.ModifyXml("SIP_MUTE","MUTE_BY_INACTIVE_AUDIO",state) 
    connection.Send()
#    WaitPartyAudioAppearsAsMuted(connection, confid, partyid)// currently I don't know to check that the mute action occur
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
            print "party [" + str(partyid) + "] is Video Unmuted" 
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Video UnMuted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
        
#------------------------------------------------------------------------------
def WaitPartyAudioAppearsAsUnMuted(connection, confid, partyid, num_retries=30):
    print "Wait until party Appears with Audio UnMute: " + str(partyid) 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if(ongoing_parties[partyid].getElementsByTagName("AUDIO_MUTE")[0].firstChild.data == "false"):
            print "party [" + str(partyid) + "] is Audio Unmuted" 
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Audio UnMuted: "+ str(partyid)) 
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
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("UNMUTE","PARTY_NAME",partyname) 
    connection.Send()
    WaitPartyVideoAppearsAsUnMuted(connection, confid, partyid)
    #WaitAllButSelfSeesPartyInLayout(connection, confid, partyid)
    return
#------------------------------------------------------------------------------
def UnMutePartyAudioTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " UnMuting Party Audio: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/Mute/MuteParty.xml')
    partyname = "Party"+str(partyid+1)
    connection.ModifyXml("UNMUTE","PARTY_NAME",partyname) 
    connection.Send()
    WaitPartyVideoAppearsAsUnMuted(connection, confid, partyid)
    #WaitAllButSelfSeesPartyInLayout(connection, confid, partyid)
    return
#------------------------------------------------------------------------------

