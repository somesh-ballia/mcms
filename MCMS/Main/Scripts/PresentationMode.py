#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML" 
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt" 
#*export SYSTEM_CFG_USER_FILE=Scripts/SysConfig/SystemCfgApacheKeepAlive120.xml
            
 #------------------------------------------------------------------------------     
c = McmsConnection()
c.Connect()

print "Adding Conf..."
status = c.SendXmlFile('Scripts/PresentationMode/AddVideoCpConfPresentationMode.xml')
        
print "Wait untill Conf create...",
num_retries = 5
for retry in range(num_retries+1):
    status = c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
    confid = c.GetTextUnder("CONF_SUMMARY","ID")
    if confid != "":
        print
        break
    if (retry == num_retries):
        print c.xmlResponse.toprettyxml()
        c.Disconnect()                
        c.exit("Can not monitor conf:" + status)
    sys.stdout.write(".")
    sys.stdout.flush()

sleep(10)

#connect parties
print "Start connecting Parties..."
num_of_parties = 3   
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    partyip =  "1.2.3." + str(x+1)
    c.AddVideoParty(confid, partyname, partyip)
    print "current number of connected parties = " + str(x+1)
    sleep(1)

c.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
c.WaitAllOngoingNotInIVR(confid)
print
print "Start Test in ConfLayout 2x2..."
confLayoutType = "2x2"
c.ChangeConfLayoutType(confid, confLayoutType)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

print
print "Start Changing Speaker every 2 seconds"
for x in range(num_of_parties): 
    c.ChangeDialOutAudioSpeaker(confid, x)
    sleep(2)
    c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

print
print "Start Changing Speaker every 17 seconds (over 15 seconds so the speaker will become the new lecturer)"
for x in range(num_of_parties):    
    c.ChangeDialOutAudioSpeaker(confid, x)
    sleep(2)
    c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)
    sleep(15)
    c.CheckValidityOfLectureMode(confid, x, confLayoutType)

print
print "Start Test in ConfLayout 1and5..."    
confLayoutType = "1and5"
c.ChangeConfLayoutType(confid, confLayoutType)
c.CheckValidityOfLectureMode(confid, x, confLayoutType)

print
print "Start Changing Speaker every 2 seconds"
for x in range(num_of_parties): 
    c.ChangeDialOutAudioSpeaker(confid, x)
    sleep(2)
    c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

print
print "Start Changing Speaker every 17 seconds (over 15 seconds so the speaker will become the new lecturer)"
for x in range(num_of_parties):    
    c.ChangeDialOutAudioSpeaker(confid, x)
    sleep(2)
    c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)
    sleep(15)
    c.CheckValidityOfLectureMode(confid, x, confLayoutType)

print    
print "Start disconnecting Parties..."
for x in range(num_of_parties-1):
    c.DeleteParty(confid, x+1)
   
print "Deleting Conf..." 
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()
