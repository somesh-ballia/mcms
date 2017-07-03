#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *
from ISDNFunctions import *
          
#------------------------------------------------------------------------------
def ChangePersonalLayoutTypeTest(connection, confid, partyid, newLayout):
    print "Party ID: "+ str(partyid) + " Changing Personal Layout Type To: " + newLayout        
    connection.LoadXmlFile('Scripts/PersonalLayout/ChangePersonalLayout.xml')
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)
    connection.ModifyXml("FORCE","LAYOUT",newLayout)       
    connection.Send()
    connection.WaitPartySeesPersonalLayout(confid, partyid, newLayout)
    return
                
#------------------------------------------------------------------------------
def ChangeBackToConfLayoutTest(connection, confid, partyid, confLayout):
    print "Party ID: "+ str(partyid) + " Changing Back to Conf Layout Type: " + confLayout        
    connection.LoadXmlFile('Scripts/PersonalLayout/ChangePersonalLayout.xml')
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","LAYOUT_TYPE","conference")
    connection.ModifyXml("FORCE","LAYOUT",confLayout)       
    connection.Send()
    connection.WaitPartySeesConfLayout(confid, partyid, confLayout)
    return
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
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
	
TestDialOutISDN(c, confid, 3, 60, "FALSE",sleepBetweenParties)

#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
   c.Disconnect()                
   sys.exit("Can not monitor conf:" + status)

sleep(2)
c.WaitAllOngoingNotInIVR(confid)
print "Sleeping to IVR finishes..."
sleep(6) #need in order to fix the following assert - CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayout CONNECTED_STANDALONE : Illegal State

confLayoutType = "2x2"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

ChangePersonalLayoutTypeTest(c, confid, 2, "1and5")
ChangePersonalLayoutTypeTest(c, confid, 3, "1and5")

confLayoutType = "3x3"
c.ChangeConfLayoutType(confid, confLayoutType)
parti_ids = [0, 1, 4, 5]
for x in range(len(parti_ids)):
    c.WaitPartySeesConfLayout(confid, parti_ids[x], confLayoutType)

c.WaitPartySeesPersonalLayout(confid, 2, "1and5")
c.WaitPartySeesPersonalLayout(confid, 3, "1and5")

ChangeBackToConfLayoutTest(c, confid, 2, confLayoutType)
ChangeBackToConfLayoutTest(c, confid, 3, confLayoutType)

confLayoutType = "1x1"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

#Changing personal layout type to each layout
for layout in availableLayoutTypes:
    ChangePersonalLayoutTypeTest(c, confid, 1, layout)
    ChangePersonalLayoutTypeTest(c, confid, 6, layout)
    sleep(1)

ChangeBackToConfLayoutTest(c, confid, 1, confLayoutType)
ChangeBackToConfLayoutTest(c, confid, 6, confLayoutType)

#Delete Conf
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()


