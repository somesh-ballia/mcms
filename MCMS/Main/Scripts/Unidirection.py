#!/mcms/python/bin/python

# #############################################################################
# Creating CP conference and simulate Avaya activities
#
# Date: 6/5/07
# By  : Inga

#
############################################################################

from McmsConnection import *
#------------------------------------------------------------------------------
def ConnectDisconnectVideoInChannelWithoutRecap(connetion,partyname,VideoChannelOpenClose):
    
    connetion.LoadXmlFile('Scripts/SimPartyUpdateChannels.xml')
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","PARTY_NAME",partyname)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","VIDEO_CHANNEL_OPEN",VideoChannelOpenClose)
    connetion.Send()
#------------------------------------------------------------------------------
def ConnectDisconnectAudioInChannelWithoutRecap(connetion,partyname,AudioChannelOpenClose):
    
    connetion.LoadXmlFile('Scripts/SimPartyUpdateChannels.xml')
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","PARTY_NAME",partyname)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","AUDIO_CHANNEL_OPEN",AudioChannelOpenClose)
    connetion.Send()    
#------------------------------------------------------------------------------
def ConnectDisconnectVideoBeforeAfterChannels(connetion,partyname,VideoChannelOpenClose,BeforeAfterChannel):
    
    connetion.LoadXmlFile('Scripts/SimPartyUpdateChannels.xml')
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","PARTY_NAME",partyname)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","VIDEO_CHANNEL_OPEN",VideoChannelOpenClose)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","RECAP_MODE",BeforeAfterChannel)
    connetion.Send() 
#------------------------------------------------------------------------------
def ConnectDisconnectAudioBeforeAfterChannels(connetion,partyname,AudioChannelOpenClose,BeforeAfterChannel):
    
    connetion.LoadXmlFile('Scripts/SimPartyUpdateChannels.xml')
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","PARTY_NAME",partyname)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","AUDIO_CHANNEL_OPEN",AudioChannelOpenClose)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","RECAP_MODE",BeforeAfterChannel)
    connetion.Send()           
#------------------------------------------------------------------------------
def ConnectDisconnectAudioVideoBeforeAfterChannels(connetion,partyname,ChannelOpenClose,BeforeAfterChannel):
    
    connetion.LoadXmlFile('Scripts/SimPartyUpdateChannels.xml')
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","PARTY_NAME",partyname)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","AUDIO_CHANNEL_OPEN",ChannelOpenClose)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","VIDEO_CHANNEL_OPEN",ChannelOpenClose)
    connetion.ModifyXml("ENDPOINT_UPDATE_CHANNELS","RECAP_MODE",BeforeAfterChannel)
    connetion.Send()          
    
#------------------------------------------------------------------------------
def TestAvayaActivitiesInVideo(connection,num_retries):
   
    print ""
    print "Test Video"
    print"Test 1 - Disconnect and Connect Video in channel "
    print"==================================================="
    #Create conf with 1 party
    print "Adding Conf "
    confName = "ConfTest1"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    confid = connection.WaitConfCreated(confName,num_retries)
    
    # Add parties 
    print "Connecting Dial out Party "
    partyname = "Party1"
    partyIp = "1.2.3.4"
    connection.AddVideoParty(confid, partyname, partyIp)
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
     
    print "Disconnect Video In channel"
    ChannelOpenClose = "false"
    ConnectDisconnectVideoInChannelWithoutRecap(connection,"DIAL_OUT#10001",ChannelOpenClose)
    sleep(1)    
    print "Connect Video In channel"
    ChannelOpenClose = "true"
    ConnectDisconnectVideoInChannelWithoutRecap(connection,"DIAL_OUT#10001",ChannelOpenClose)
      
    sleep(3)
    
    connection.WaitAllOngoingConnected(confid)
     
    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd() 
    
         
    print""     
    print"Test 2 - Fast Disconnect and Connect Video in&out channel - Before And After"
    print"============================================================================" 
   
    #Create conf with 2 parties 
    num_of_parties = 2
    
    print "Adding Conf "
    confName = "ConfTest2"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    confid = connection.WaitConfCreated(confName,num_retries)
    
    # Add parties 
    print "Connecting Dial out Party "
    partyname1 = "Party1"
    partyIp1 = "1.2.3.1"
    connection.AddVideoParty(confid, partyname1, partyIp1)
    
    print "Connecting Dial In Party "
    partyname2 = "Party2"
    partyIp2 = "1.2.3.2"
    connection.SimulationAddH323Party(partyname2, confName)
    connection.SimulationConnectH323Party(partyname2)
    
    connection.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
   
    print "---Before----"
    BeforeAfterChannel = "before"
       
    print "Disconnect Video In&Out channel"
    ChannelOpenClose = "false"
    ConnectDisconnectVideoBeforeAfterChannels(connection,"DIAL_OUT#10001",ChannelOpenClose,BeforeAfterChannel)
    
    sleep(1)
    print "Connect Video In&Out channel"    
    ChannelOpenClose = "true"
    ConnectDisconnectVideoBeforeAfterChannels(connection,"DIAL_OUT#10001",ChannelOpenClose,BeforeAfterChannel)   
    
    sleep(3)   

    connection.WaitAllOngoingConnected(confid) 
    
    print "---After----"
    BeforeAfterChannel = "after"
    
    print "Disconnect Video In&Out channel"
    ChannelOpenClose = "false"
    ConnectDisconnectVideoBeforeAfterChannels(connection,partyname2,ChannelOpenClose,BeforeAfterChannel)
    sleep(1)
    print "Connect Video In&Out channel"    
    ChannelOpenClose = "true"
    ConnectDisconnectVideoBeforeAfterChannels(connection,partyname2,ChannelOpenClose,BeforeAfterChannel)   
    
    sleep(2)
    
    connection.WaitAllOngoingConnected(confid) 
    
    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd() 
     
#---------------------------------------------------------------------------------------------------------
def TestAvayaActivitiesInAudio(connection,num_retries):
   
    print""  
    print "Test Audio"
    print"Test 3 - Disconnect and Connect Audio in channel "
    print"==================================================="
    #Create conf with 1 party
    print "Adding Conf "
    confName = "ConfTest1"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    confid = connection.WaitConfCreated(confName,num_retries)
    
     # Add parties 
    print "Connecting Dial out Party "
    partyname = "Party1"
    partyIp = "1.2.3.4"
    connection.AddVideoParty(confid, partyname, partyIp)
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
     
    print "Disconnect Audio In channel"
    ChannelOpenClose = "false"
    ConnectDisconnectAudioInChannelWithoutRecap(connection,"DIAL_OUT#10001",ChannelOpenClose)
    sleep(1)    
    print "Connect Audio In channel"
    ChannelOpenClose = "true"
    ConnectDisconnectAudioInChannelWithoutRecap(connection,"DIAL_OUT#10001",ChannelOpenClose)
       
    sleep(2)
    
    connection.WaitAllOngoingConnected(confid)
     
    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd() 
    
         
    print""     
    print"Test 4 - Fast Disconnect and Connect Audio in&out channel - Before And After"
    print"============================================================================" 
   
    #Create conf with 2 parties 
    num_of_parties = 2
    
    print "Adding Conf "
    confName = "ConfTest2"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    confid = connection.WaitConfCreated(confName,num_retries)
    
    # Add parties 
    print "Connecting Dial out Party "
    partyname1 = "Party1"
    partyIp1 = "1.2.3.1"
    connection.AddVideoParty(confid, partyname1, partyIp1)
    
    print "Connecting Dial In Party "
    partyname2 = "Party3"
    partyIp2 = "1.2.3.3"
    connection.SimulationAddH323Party(partyname2, confName)
    connection.SimulationConnectH323Party(partyname2)
    
    connection.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    
    print "---Before----"
    BeforeAfterChannel = "before"
        
    print "Disconnect Audio In&Out channel"
    ChannelOpenClose = "false"
    ConnectDisconnectAudioBeforeAfterChannels(connection,"DIAL_OUT#10001",ChannelOpenClose,BeforeAfterChannel)
    sleep(1)
    print "Connect Audio In&Out channel"    
    ChannelOpenClose = "true"
    ConnectDisconnectAudioBeforeAfterChannels(connection,"DIAL_OUT#10001",ChannelOpenClose,BeforeAfterChannel)   
    
    sleep(3)
    
    connection.WaitAllOngoingConnected(confid) 
    
    print "---After----"
    BeforeAfterChannel = "after"
    
    print "Disconnect Audio In&Out channel"
    ChannelOpenClose = "false"
    ConnectDisconnectAudioBeforeAfterChannels(connection,partyname2,ChannelOpenClose,BeforeAfterChannel)
    sleep(1)
    print "Connect Audio In&Out channel"    
    ChannelOpenClose = "true"
    ConnectDisconnectAudioBeforeAfterChannels(connection,partyname2,ChannelOpenClose,BeforeAfterChannel)   
    
    sleep(2)
    
    connection.WaitAllOngoingConnected(confid) 
    
    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd() 
    
#---------------------------------------------------------------------------------------------------------
def  TestAvayaActivitiesInAudioAndVideo(connection,num_retries): 
     
     print ""
     print "Test Audio+Video"      
     print "Test 5 - Disconnect and Connect Audio+Video in&out channel - Before"
     print"============================================================================" 
    
     print "Adding Conf "
     confName = "ConfTest3"
     connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
     confid = connection.WaitConfCreated(confName,num_retries)
    
     # Add parties 
     print "Connecting Dial out Party "
     partyname = "Party1"
     partyIp = "1.2.3.1"
     connection.AddVideoParty(confid, partyname, partyIp)
     connection.WaitAllOngoingConnected(confid)
     connection.WaitAllOngoingNotInIVR(confid)
     
     print "---Before----"
     BeforeAfterChannel = "before"
     print "Disconnect Video & Audio channels"
     ChannelOpenClose = "false"
     ConnectDisconnectAudioVideoBeforeAfterChannels(connection,"DIAL_OUT#10001",ChannelOpenClose,BeforeAfterChannel)
     
     sleep(1)
        
     print "Connect Video & Audio channels"
     ChannelOpenClose = "true"
     ConnectDisconnectAudioVideoBeforeAfterChannels(connection,"DIAL_OUT#10001",ChannelOpenClose,BeforeAfterChannel)
       
     sleep(2)
    
     connection.WaitAllOngoingConnected(confid)
     
     print "Deleting Conf..." 
     connection.DeleteConf(confid)   
     connection.WaitAllConfEnd() 
     
   
   
#---------------------------------------------------------------------------------------------------------

c = McmsConnection()
c.Connect() 

TestAvayaActivitiesInVideo(c,5)
TestAvayaActivitiesInAudio(c,5)
TestAvayaActivitiesInAudioAndVideo(c,5)
c.Disconnect()
   