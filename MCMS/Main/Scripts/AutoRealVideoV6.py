#!/mcms/python/bin/python

###########################################
#  By: Guy D.
#  Date: 8.10.07
#  Testing mixed dial out IpV4/IpV6 conference
###########################################
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

from McmsConnection import *

c = McmsConnection()
c.Connect()
print "Adding Conf..."
status = c.SendXmlFile('Scripts/AddVideoCpConf.xml')
print "Adding Conf..."
num_retries = 5
print "Wait untill Conf create...",
for retry in range(num_retries+1):
      status = c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
      confid = c.GetTextUnder("CONF_SUMMARY","ID")
      if confid != "":
           print
           break
      if (retry == num_retries):
         print c.xmlResponse.toprettyxml(encoding="utf-8")
         c.Disconnect()                
         ScriptAbort("Can not monitor conf:" + status)
      sys.stdout.write(".")
      sys.stdout.flush()

print "Create conf with id " + str(confid) 

partyname = "Party"+str(1)
c.AddIpV6Party(confid, partyname, "Scripts/AddVideoPartyIpV6Link.xml")                     
sleep(1)

partyname = "Party"+str(2)
c.AddIpV6Party(confid, partyname, "Scripts/AddVideoPartyIpV6Global.xml")                     
sleep(1)

partyname = "Party"+str(3)
c.AddIpV6Party(confid, partyname, "Scripts/AddVideoPartyIpV6Site.xml") 
sleep(1)

partyname = "Party"+str(4)
partyip = "1.1.2.2"
c.AddParty(confid, partyname, partyip, "Scripts/AddVideoParty1.xml")
sleep(1)

all_num_parties = 4
c.WaitAllPartiesWereAdded(confid,all_num_parties,num_retries) 

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()

some_party_id = c.GetTextUnder("PARTY","ID")
c.WaitAllOngoingConnected(confid,num_retries*all_num_parties)


# delete all parties  
for x in range(all_num_parties):
    partyname = "Party"+str(x+1)
    print "disconnecting party: " + partyname 
    c.SimulationDisconnectH323Party(partyname)   
    c.SimulationDeleteH323Party(partyname)
    sleep(1)
    
    

#print "Delete Conference..."
c.DeleteConf(confid)

c.WaitAllConfEnd()

c.Disconnect()



