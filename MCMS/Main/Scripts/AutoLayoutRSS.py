#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

AutoLayoutTypes = "1x1", "1x1", "1x2", "2x2", "2x2", "1and5", "1and5", "1and7", "1and7", "1and7"
AutoLayoutRSSTypes = "1x1", "1x2", "2x2", "2x2", "1and5", "1and5", "1and7", "1and7", "3x3", "4x4"

from McmsConnection import *
from ISDNFunctions import *

#------------------------------------------------------------------------------
def AddRecordingLink(c,num_retries):
    print "Adding Recording Link"
    c.SendXmlFile('Scripts/AutoLayout/AddRecordingLink.xml')
    print "Wait until Recording Link created"
    for retry in range(num_retries+1):
    	c.SendXmlFile('Scripts/AutoLayout/TransRecordingLinksList.xml')
    	recLinkName = c.GetTextUnder("RECORDING_LINKS_LIST","REC_LINK_DEFAULT_NAME")
    	if recLinkName != "":
            print
            break
    	if (retry == num_retries):
            print c.xmlResponse.toprettyxml()
            c.Disconnect()                
            c.exit("Can not find recording link")
    	sys.stdout.write(".")
    	sys.stdout.flush()

#------------------------------------------------------------------------------  	
def RemoveRecordingLink(c,num_retries):
    print "Deleting Recording Link"
    c.SendXmlFile('Scripts/AutoLayout/RemoveRecordingLink.xml')
    print "Wait until Recording Link deleted"
    for retry in range(num_retries+1):
    	c.SendXmlFile('Scripts/AutoLayout/TransRecordingLinksList.xml')
	#print c.xmlResponse.toprettyxml()
	if (c.GetTextUnder("RECORDING_LINKS_LIST","CHANGED") != "false"): 
	    recLinkName = c.GetTextUnder("RECORDING_LINKS_LIST","REC_LINK_DEFAULT_NAME")
    	    if recLinkName == "":
               print
               break
    	    if (retry == num_retries):
               print c.xmlResponse.toprettyxml()
               c.Disconnect()                
               c.exit("recording link still exists in system, name: "+recLinkName)
    	sys.stdout.write(".")
    	sys.stdout.flush()
#------------------------------------------------------------------------------
    	
c = McmsConnection()
c.Connect()

num_retries = 10
AddRecordingLink(c, num_retries)
print "Adding Conf..."
status = c.SendXmlFile('Scripts/AutoLayout/AddVideoCpRecordingConfAutoLayout.xml')
        
print "Wait untill Conf create...",

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

recording_link_name = "Recording"

print "Start connecting Parties..."
num_of_parties = 8   #in order to add more parties AutoLayoutTypes array must be increased
for x in range(num_of_parties):
    if x < num_of_parties/2:
    	partyname = "Party"+str(x+1)
    	partyip =  "1.2.3." + str(x+1)
    	c.AddVideoParty(confid, partyname, partyip)
    	print "current number of connected parties = " + str(x+1)
    	sleep(1)
    else:
    	partyname = "IsdnParty"+str(x+1)
    	phone="3333"+str(x+1)
    	print "Adding Party ("+partyname+")"
    	AddIsdnDialoutParty(c,confid,partyname,phone)
    	sleep(1)
	
	
    c.WaitAllOngoingNotInIVR(confid)
    sleep(2)
    if (x == 0):
	c.WaitAllPartiesWereAdded(confid,2,num_retries)
    	recording_party_id = int(c.GetPartyId(confid, recording_link_name)) - 1
    	
    c.WaitAllOngoingChangedLayoutType(confid,AutoLayoutTypes[x],num_retries*3,recording_link_name)
    c.WaitPartySeesPersonalLayout(confid,recording_party_id,AutoLayoutRSSTypes[x])

print "Start disconnecting Parties..."
recording_party_index = recording_party_id
for x in range(num_of_parties-1):
    if x < num_of_parties/2:
        partyname = "Party"+str(x+1)
    else:
        partyname = "IsdnParty"+str(x+1)	
    """
    if (x == recording_party_id):
	continue
    """
    #deleted_party_index = x
    if (x < recording_party_id):
	#deleted_party_index = x + 1
        recording_party_index = recording_party_index - 1

    print "Deleting Party - " +partyname
    deleted_party_id = c.GetPartyId(confid, partyname)
    c.DeleteParty(confid, deleted_party_id)
    num_connected_parties = num_of_parties - x - 1
    print "current number of connected parties = " + str(num_connected_parties) + " + 1 Recording Link "
    sleep(1)
    c.WaitAllOngoingChangedLayoutType(confid,AutoLayoutTypes[num_connected_parties-1],num_retries*3,recording_link_name)
    c.WaitPartySeesPersonalLayout(confid,recording_party_index,AutoLayoutRSSTypes[num_connected_parties - 1])

print "Deleting Conf..." 
c.DeleteConf(confid)   
c.WaitAllConfEnd()
RemoveRecordingLink(c, num_retries)

c.Disconnect()


