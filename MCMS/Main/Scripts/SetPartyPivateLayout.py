#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

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
#    connection.WaitPartySeesPersonalLayout(confid, partyid, newLayout)
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

sleepBetweenParties = 1
if (c.IsProcessUnderValgrind("ConfParty")):
    # we need long sleep time in order to let the ip parties to change their content mode from 264 to 263 
    sleepBetweenParties = 10
    
confid = c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         3,
                         1,
                         "false",
                         "NONE",
                         sleepBetweenParties)

#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
   c.Disconnect()                
   sys.exit("Can not monitor conf:" + status)

listPartyIDs = c.GetPartyIDs(confid)
print listPartyIDs



sleep(1)
c.WaitAllOngoingNotInIVR(confid)
print "Sleeping to IVR finishes..."
sleep(6) #need in order to fix the following assert - CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayout CONNECTED_STANDALONE : Illegal State

#confLayoutType = "2x2"
#c.ChangeConfLayoutType(confid, confLayoutType)
#c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)
#sleep(20)

i = 1
for x in listPartyIDs:
    print "ChangePersonalLayoutType ID =  " + str(x)
    ChangePersonalLayoutTypeTest(c, confid, x, "1and5")
    c.WaitPartySeesPersonalLayout(confid, i, "1and5")
    i=i+1

# delete all parties
for x in listPartyIDs:            
    c.DeleteParty(confid,x)
    sleep(2)


#Delete Conf
c.DeleteConf(confid)   
c.WaitAllConfEnd()
sleep(10)

c.Disconnect()

