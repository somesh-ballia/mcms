#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from McmsConnection import *


c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         3,
                         60)
                                           
num_retries = 60
print "Adding Conf..."
status = c.SendXmlFile('Scripts/AddVideoCpConf.xml')
        
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
        sys.exit("Can not monitor conf:" + status)
    sys.stdout.write(".")
    sys.stdout.flush()

print "Create conf with id " + str(confid) 
c.LoadXmlFile('Scripts/AddVideoParty1.xml')
        
partyname = "Party"+str(x+1)
partyip =  "1.2.3." + str(x+1)
print "Adding Party ("+partyname+")"
c.ModifyXml("PARTY","NAME",partyname)
c.ModifyXml("PARTY","IP",partyip)
c.ModifyXml("ADD_PARTY","ID",confid)
c.Send()

c.WaitAllPartiesWereAdded(confid,1,num_retries)    
        
c.WaitAllOngoingConnected(confid,num_retries*all_num_parties)

       
c.DeleteConf(confid)
        
print "Wait until no conferences..."
c.WaitAllConfEnd()
    
c.Disconnect()


