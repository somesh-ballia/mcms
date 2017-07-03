#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from McmsConnection import *
from HDFunctions import *
import string 

          
#------------------------------------------------------------------------------
def SetPersonalMessageOverlayTest(connection, confid, partyid):
    print "Party ID: "+ str(partyid) + " text: Message Overlay Party TEST"   
    connection.LoadXmlFile('Scripts/MessageOverlay/SetPartyMessageOverlay.xml') 
    connection.ModifyXml("SET_PARTY_MESSAGE_OVERLAY","ID",confid)
    connection.ModifyXml("SET_PARTY_MESSAGE_OVERLAY","PARTY_ID",partyid)
    connection.ModifyXml("MESSAGE_OVERLAY","ON", "true")   
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_TEXT", "Message Overlay Party TEST") 
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_FONT_SIZE", "small")    
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_COLOR", "white_font_on_red_background") 
    connection.ModifyXml("MESSAGE_OVERLAY","NUM_OF_REPETITIONS", "5") 
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_DISPLAY_SPEED", "fast")
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_DISPLAY_POSITION", "bottom")
    connection.Send()


    return
                
#------------------------------------------------------------------------------
def StopPersonalMessageOverlayTest(connection, confid, partyid):
    print "Party ID: "+ str(partyid) + " text: Message Overlay Party TEST"   
    connection.LoadXmlFile('Scripts/MessageOverlay/StopPartyMessageOverlay.xml')
    connection.ModifyXml("SET_PARTY_MESSAGE_OVERLAY","ID",confid)
    connection.ModifyXml("SET_PARTY_MESSAGE_OVERLAY","PARTY_ID",partyid)
    connection.ModifyXml("MESSAGE_OVERLAY","ON", "false")
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_TEXT", "Message Overlay Party TEST") 
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_FONT_SIZE", "small")    
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_COLOR", "white_font_on_red_background") 
    connection.ModifyXml("MESSAGE_OVERLAY","NUM_OF_REPETITIONS", "5") 
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_DISPLAY_SPEED", "fast")
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_DISPLAY_POSITION", "bottom")
    connection.Send()


    return
                
#------------------------------------------------------------------------------
def ChangeToConfMessageOverlayTest(connection, confid):
    print "Conf ID: "+ str(confid)  + " text: Message Overlay Conf TEST"
    connection.LoadXmlFile('Scripts/MessageOverlay/SetMessageOverlay.xml')      
    connection.ModifyXml("SET_MESSAGE_OVERLAY","ID",confid)
    connection.ModifyXml("MESSAGE_OVERLAY","ON", "true")   
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_TEXT", "Message Overlay Conf TEST") 
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_FONT_SIZE", "small")    
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_COLOR", "white_font_on_red_background") 
    connection.ModifyXml("MESSAGE_OVERLAY","NUM_OF_REPETITIONS", "5") 
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_DISPLAY_SPEED", "slow")
    connection.ModifyXml("MESSAGE_OVERLAY","MESSAGE_DISPLAY_POSITION", "bottom")      
    connection.Send()
    return
#------------------------------------------------------------------------------

# 1. Create a conference and connect 5 video parties
c = McmsConnection()
c.Connect()
confid = c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         3,
                         60,
                         "false",
                         "NONE",
                         2)
sleepBetweenParties = 0
if (c.IsProcessUnderValgrind("ConfParty")):
    # we need long sleep time in order to let the ip parties to change their content mode from 264 to 263 
    sleepBetweenParties = 10
sleep(10)



SetPersonalMessageOverlayTest(c, confid, 1)
sleep(10)
SetPersonalMessageOverlayTest(c, confid, 2)
sleep(10)
SetPersonalMessageOverlayTest(c, confid, 3)
sleep(10)
#ChangeToConfMessageOverlayTest(c, confid)
#sleep(10)
StopPersonalMessageOverlayTest(c, confid, 2)
sleep(10)
print "Test Ended"

#Delete Conf
#c.DeleteConf(confid)   
#c.WaitAllConfEnd()

#c.Disconnect()




#------------------------------------------------------------------------------  

