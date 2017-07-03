#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

AutoLayoutTypes = "1x1", "1x1", "1x2", "2x2", "2x2", "1and5", "1and5", "1and7", "1and7", "2and8"

from McmsConnection import *
from ISDNFunctions import *


c = McmsConnection()
c.Connect()

status = c.SendXmlFile('Scripts/AutoLayout/AddVideoCpConfAutoLayout.xml')
print "Adding Conf... status="+str(status)
        
print "Wait untill Conf create..."
num_retries = 10
for retry in range(num_retries+1):
    sleep(1)
    status = c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
    confid = c.GetTextUnder("CONF_SUMMARY","ID")
    if confid != "":
        print "Conf created, ConfId="+str(confid)
        break
    if (retry == num_retries):
        print c.xmlResponse.toprettyxml()
        c.Disconnect()                
        c.exit("Can not monitor conf:" + status)
    c.stdout.write(".")
    c.stdout.flush()

print "Start connecting Parties..."
num_of_parties = 10   #in order to add more parties AutoLayoutTypes array must be increased
for x in range(num_of_parties):
    if x < num_of_parties/2:
    	partyname = "Party"+str(x+1)
    	partyip =  "1.2.3." + str(x+1)
    	c.AddVideoParty(confid, partyname, partyip)
    	sleep(1)
    else:
    	partyname = "IsndParty"+str(x+1)
    	phone="3333"+str(x+1)
    	print "Adding ISDN Party..."+partyname
    	AddIsdnDialoutParty(c,confid,partyname,phone)
    	sleep(1)

    c.WaitAllOngoingNotInIVR(confid)
    sleep(2)
    c.WaitAllOngoingChangedLayoutType(confid,AutoLayoutTypes[x])


print "Start disconnecting Parties..."
    
iFirstPartyId = int(c.GetPartyId(confid, "Party1"))
print "FirstPartyId = " + str(iFirstPartyId)

for x in range(num_of_parties-1):
    print "Deleting party = Party"+str(x+1)
    c.DeleteParty(confid, x+iFirstPartyId)
    sleep(1)
    c.WaitAllOngoingChangedLayoutType(confid,AutoLayoutTypes[num_of_parties-(x+2)])
   
print "Deleting Conf..." 
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()


