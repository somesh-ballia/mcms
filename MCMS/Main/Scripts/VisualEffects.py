#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *
                    
#------------------------------------------------------------------------------
def SetConfVisualEffectsTest(connection, confName, background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue, imageId, num_of_parties):
    print "Creating Conf with Visual Effects"
    connection.LoadXmlFile('Scripts/AddVideoCpConf.xml')
    
    connection.ModifyXml("RESERVATION","NAME",confName)
    
    connection.loadedXml.getElementsByTagName("BACKGROUND_COLOR")[0].getElementsByTagName("RED")[0].firstChild.data = background_red 
    connection.loadedXml.getElementsByTagName("BACKGROUND_COLOR")[0].getElementsByTagName("GREEN")[0].firstChild.data = background_green 
    connection.loadedXml.getElementsByTagName("BACKGROUND_COLOR")[0].getElementsByTagName("BLUE")[0].firstChild.data = background_blue 
    
    connection.loadedXml.getElementsByTagName("LAYOUT_BORDER")[0].firstChild.data = isBorder 
    connection.loadedXml.getElementsByTagName("LAYOUT_BORDER_COLOR")[0].getElementsByTagName("RED")[0].firstChild.data = border_red 
    connection.loadedXml.getElementsByTagName("LAYOUT_BORDER_COLOR")[0].getElementsByTagName("GREEN")[0].firstChild.data = border_green 
    connection.loadedXml.getElementsByTagName("LAYOUT_BORDER_COLOR")[0].getElementsByTagName("BLUE")[0].firstChild.data = border_blue 
    
    connection.loadedXml.getElementsByTagName("SPEAKER_NOTATION")[0].firstChild.data = isSpeaker 
    connection.loadedXml.getElementsByTagName("SPEAKER_NOTATION_COLOR")[0].getElementsByTagName("RED")[0].firstChild.data = speaker_red 
    connection.loadedXml.getElementsByTagName("SPEAKER_NOTATION_COLOR")[0].getElementsByTagName("GREEN")[0].firstChild.data = speaker_green 
    connection.loadedXml.getElementsByTagName("SPEAKER_NOTATION_COLOR")[0].getElementsByTagName("BLUE")[0].firstChild.data = speaker_blue 
    
    connection.loadedXml.getElementsByTagName("IMAGE_ID")[0].firstChild.data = imageId 
    
    connection.Send()
    
    #Get ConfId
    confid=connection.WaitConfCreated(confName)
    
    connection.LoadXmlFile('Scripts/AddVideoParty1.xml')
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        partyip =  "1.2.3." + str(x+1)
        print "Adding Party ("+partyname+")"
        connection.ModifyXml("PARTY","NAME",partyname)
        connection.ModifyXml("PARTY","IP",partyip)
        connection.ModifyXml("ADD_PARTY","ID",confid)
        connection.Send()

    all_num_parties = num_of_parties
    num_retries = 5
    
    connection.WaitAllPartiesWereAdded(confid,all_num_parties,num_retries*all_num_parties)    
    connection.WaitAllOngoingConnected(confid,num_retries*all_num_parties)
    connection.WaitAllOngoingNotInIVR(confid,num_retries*all_num_parties)
        
    WaitConferenceVisualEffectsChanged(connection, confid, background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue)
    
    print    
    print "Start Deleting Conference..."
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd()
    
    return

#------------------------------------------------------------------------------
def WaitConferenceVisualEffectsChanged(connection, confid, background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue, num_retries=30):
    print "Wait until ConfId: " + str(confid) + " sees new visual effects"
    connection.LoadXmlFile('Scripts/SpeakerChange/TransConf2.xml')
    connection.ModifyXml('GET','ID',confid)
    for retry in range(num_retries+1):
        connection.Send()
        current_background_red = connection.xmlResponse.getElementsByTagName("BACKGROUND_COLOR")[0].getElementsByTagName("RED")[0].firstChild.data
        current_background_green = connection.xmlResponse.getElementsByTagName("BACKGROUND_COLOR")[0].getElementsByTagName("GREEN")[0].firstChild.data
        current_background_blue = connection.xmlResponse.getElementsByTagName("BACKGROUND_COLOR")[0].getElementsByTagName("BLUE")[0].firstChild.data
        
        current_isBorder = connection.xmlResponse.getElementsByTagName("LAYOUT_BORDER")[0].firstChild.data
        current_border_red =  connection.xmlResponse.getElementsByTagName("LAYOUT_BORDER_COLOR")[0].getElementsByTagName("RED")[0].firstChild.data 
        current_border_green =  connection.xmlResponse.getElementsByTagName("LAYOUT_BORDER_COLOR")[0].getElementsByTagName("GREEN")[0].firstChild.data 
        current_border_blue =  connection.xmlResponse.getElementsByTagName("LAYOUT_BORDER_COLOR")[0].getElementsByTagName("BLUE")[0].firstChild.data 

        current_isSpeaker = connection.xmlResponse.getElementsByTagName("SPEAKER_NOTATION")[0].firstChild.data
        current_speaker_red =  connection.xmlResponse.getElementsByTagName("SPEAKER_NOTATION_COLOR")[0].getElementsByTagName("RED")[0].firstChild.data 
        current_speaker_green =  connection.xmlResponse.getElementsByTagName("SPEAKER_NOTATION_COLOR")[0].getElementsByTagName("GREEN")[0].firstChild.data 
        current_speaker_blue =  connection.xmlResponse.getElementsByTagName("SPEAKER_NOTATION_COLOR")[0].getElementsByTagName("BLUE")[0].firstChild.data 
        
        #print str(background_red) + " " + str(background_green) + " " + str(background_blue) + " " + str(isBorder) + " " + str(border_red) + " " + str(border_green) + " " + str(border_blue) + " " + str(isSpeaker) + " " + str(speaker_red) + " " + str(speaker_green) + " " + str(speaker_blue)
        #print current_background_red + " " + str(current_background_green) + " " + str(current_background_blue) + " " + str(current_isBorder) + " " + str(current_border_red) + " " + str(current_border_green) + " " + str(current_border_blue) + " " + str(current_isSpeaker) + " " + str(current_speaker_red) + " " + str(current_speaker_green) + " " + str(current_speaker_blue)
        
        if(current_background_red == str(background_red)):
            if(current_background_green == str(background_green)):  
                if(current_background_blue == str(background_blue)):
                    if(current_isBorder == isBorder):
                        if(current_border_red == str(border_red)):
                            if(current_border_green == str(border_green)):
                                if(current_border_blue == str(border_blue)):
                                    if(current_isSpeaker == isSpeaker):
                                        if(current_border_red == str(border_red)):
                                            if(current_border_green == str(border_green)):
                                                if(current_border_blue == str(border_blue)):
                                                    break
        if (retry == num_retries):
            print
            connection.Disconnect()                
            sys.exit("Setting visual effects Conf Id: "+ str(confid)+" Failed!!!")
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
c = McmsConnection()
c.Connect()


print
print "Start Visual Effects Test..."
print "Conf with Skin 1..."
num_of_parties = 2
background_red = 0
background_green = 49
background_blue = 112
isBorder = "true"
border_red = 48
border_green = 56
border_blue = 65
isSpeaker = "true"
speaker_red = 217
speaker_green = 167
speaker_blue = 17
imageId = 0
SetConfVisualEffectsTest(c, "Conf1", background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue, imageId, num_of_parties)
print "Conf with Skin 2..."
isBorder = "false"
isSpeaker = "true"
imageId = 0
SetConfVisualEffectsTest(c, "Conf2", background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue, imageId, num_of_parties)
print "Conf with Skin 3..."
isBorder = "true"
isSpeaker = "false"
imageId = 1
SetConfVisualEffectsTest(c, "Conf3", background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue, imageId, num_of_parties)
print "Conf with Skin 4..."
background_red = 217
background_green = 167
background_blue = 17
isBorder = "true"
border_red = 48
border_green = 56
border_blue = 65
isSpeaker = "true"
speaker_red = 0
speaker_green = 49
speaker_blue = 112
imageId = 2
SetConfVisualEffectsTest(c, "Conf4", background_red, background_green, background_blue, isBorder, border_red, border_green, border_blue, isSpeaker, speaker_red, speaker_green, speaker_blue, imageId, num_of_parties)

c.Disconnect()


