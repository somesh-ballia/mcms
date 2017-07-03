#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from MuteUtil import *
from McmsConnection import *
          
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVoipConf.xml',
                         'Scripts/SipAddVoipParty1.xml',
                         1,
                         60,
                         "false")

#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
   c.Disconnect()                
   sys.exit("Can not monitor conf:" + status)


sleep(10)

mutedParty = 0;
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
print
print "Mute Audio in Conf ..."
MutePartyAudioTest(c, confid, mutedParty)
#WaitNoOneSeesPartyInLayout(c, confid, mutedParty)
#print
#print "Unmute Audio in Conf ..."
#UnMutePartyAudioTest(c, confid, mutedParty)
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)

print    
print "Start Deleting Conference..."
sleep(5)
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()


