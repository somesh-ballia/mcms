#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *

#------------------------------------------------------------------------------
def WaitAllOngoingSeeSameLayout(connection,confid,num_retries=30):
    print "Wait until all ongoing parties see same layout"
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()

    for retry in range(num_retries+1):
        sleep(1)
        areAllLayoutTypesTheSame = 1
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        #first_layout_type = ongoing_parties[0].getElementsByTagName("LAYOUT")[0].firstChild.data
        first_party_force = connection.getElementsByTagNameFirstLevelOnly(ongoing_parties[0],"FORCE")[0]
        first_layout_type = first_party_force.getElementsByTagName("LAYOUT")[0].firstChild.data
	    #first_party_cells = ongoing_parties[0].getElementsByTagName("FORCE")[0].getElementsByTagName("CELL")
        first_party_cells = first_party_force.getElementsByTagName("CELL")
        for party in ongoing_parties:
            force = connection.getElementsByTagNameFirstLevelOnly(party,"FORCE")[0]
            layout = force.getElementsByTagName("LAYOUT")[0].firstChild.data
            #layout = party.getElementsByTagName("LAYOUT")[0].firstChild.data
            if layout != first_layout_type:
                print "layout != first_layout_type"
                areAllLayoutTypesTheSame = 0
                if (retry == num_retries):
                    print connection.xmlResponse.toprettyxml()
                    connection.Disconnect()
                    sys.exit("Party is not in same conf layout type: "+ first_layout_type +", But in layout : " + layout)
            else:
                #current_party_cells = party.getElementsByTagName("FORCE")[0].getElementsByTagName("CELL")
                current_party_cells = force.getElementsByTagName("CELL")
                if len(first_party_cells) != len(current_party_cells):
                    areAllLayoutTypesTheSame = 0
                    if (retry == num_retries):
                        print connection.xmlResponse.toprettyxml()
                        connection.Disconnect()
                        sys.exit("Party is not in same conf layout type: "+ first_layout_type +", But in layout : " + layout)
                else:    
                    i = 0
                    for cell in current_party_cells:
                        currentPartySourceInThisCell = cell.getElementsByTagName("SOURCE_ID")[0].firstChild.data  
                        firstPartySourceInThisCell = first_party_cells[i].getElementsByTagName("SOURCE_ID")[0].firstChild.data
                        print "Cell[" + str(i) +"]: FirstPartySource=" + firstPartySourceInThisCell + " ThisPartySource=" + currentPartySourceInThisCell
                        i = i+1
                        if(currentPartySourceInThisCell!= firstPartySourceInThisCell):
                            areAllLayoutTypesTheSame = 0
                            if (retry == num_retries):
                                print connection.xmlResponse.toprettyxml()
                                connection.Disconnect()
                                sys.exit("Party is not in conf same layout by cell sources") 
        sys.stdout.write(".")
        sys.stdout.flush()
        if areAllLayoutTypesTheSame == 1:
            print
            break
        connection.Send()

 #------------------------------------------------------------------------------     
c = McmsConnection()
c.Connect()

print "Adding Conf..."
status = c.SendXmlFile('Scripts/SameLayout/AddVideoCpConfSameLayout.xml')
        
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
    c.stdout.write(".")
    c.stdout.flush()

#connect parties
print "Start connecting Parties..."
num_of_parties = 5   
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    partyip =  "1.2.3." + str(x+1)
    c.AddVideoParty(confid, partyname, partyip)
    print "current number of connected parties = " + str(x+1)
    c.WaitAllOngoingConnected(confid)
    c.WaitAllOngoingNotInIVR(confid)
    WaitAllOngoingSeeSameLayout(c,confid)

print "Start Changing ConfLayout Type... And Speaker in Layout..."
c.ChangeConfLayoutType(confid, "2x2")
WaitAllOngoingSeeSameLayout(c,confid)

for x in range(num_of_parties): 
    c.ChangeDialOutVideoSpeaker(confid, x)
    sleep(1)
    WaitAllOngoingSeeSameLayout(c,confid)
    
c.ChangeConfLayoutType(confid, "1and5")
WaitAllOngoingSeeSameLayout(c,confid)

for x in range(num_of_parties): 
    c.ChangeDialOutVideoSpeaker(confid, x)
    sleep(1)
    WaitAllOngoingSeeSameLayout(c,confid)
    
c.ChangeConfLayoutType(confid, "1x1")
WaitAllOngoingSeeSameLayout(c,confid)

for x in range(num_of_parties): 
    c.ChangeDialOutVideoSpeaker(confid, x)
    sleep(1)
    WaitAllOngoingSeeSameLayout(c,confid)
   
print "Start disconnecting Parties..."
for x in range(num_of_parties-1):
    c.DeleteParty(confid, x)
    print "current number of connected parties = " + str(num_of_parties-(x+1))
    sleep(1)
    WaitAllOngoingSeeSameLayout(c,confid)
   
print "Deleting Conf..." 
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()
