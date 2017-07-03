#!/mcms/python/bin/python
###############################################################################################
# ResourceCause2.py
# Description: Causes for failure in resources of the system. 
#			   Auto-Add Participant when no ports left.
###############################################################################################

from McmsConnection import *
from ResourceUtilities import *
import os

#------------------------------------------------------------------------------

def ConfWithOneVideoParty(connection, confname, confFile, protocol, direction, dialInCapSetName = "FULL CAPSET"):
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf " + confname + "  ..."
    connection.Send()
    sleep(1)
    print "Wait untill Conf create...",
    num_retries = 5
    for retry in range(num_retries+1):
        status = connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if confid != "":
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            connection.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
    partyname = confname+"_"+direction+"_"+protocol
    partyip =  "1.2.3.1" 
    if(direction == "DialOut"):
        if(protocol == "H323"):
            #------Dial out H323 party
             connection.AddVideoParty(confid, partyname, partyip)
             print "Connecting H323 Dial out Party" 
             connection.WaitAllOngoingConnected(confid)
             connection.WaitAllOngoingNotInIVR(confid)
             
        if(protocol == "SIP"):   
             #------Dial out SIP party
            connection.AddVideoParty(confid, partyname, partyip, "true")
            print "Connecting SIP Dial out Party" 
            connection.WaitAllOngoingConnected(confid)
            connection.WaitAllOngoingNotInIVR(confid)
    
    if(direction == "DialIn"):
        if(protocol == "H323"):
            #-------Dial in H323 party
            connection.SimulationAddH323Party(partyname, confname,dialInCapSetName)
            connection.SimulationConnectH323Party(partyname)
            print "Connecting H323 Dial in Party"
            connection.WaitAllOngoingConnected(confid)
            connection.WaitAllOngoingNotInIVR(confid)
        if(protocol == "SIP"):  
            #-------Dial in SIP party
            connection.SimulationAddSipParty(partyname, confname,dialInCapSetName)
            connection.SimulationConnectSipParty(partyname)
            print "Connecting SIP Dial in Party" 
            connection.WaitAllOngoingConnected(confid)
            connection.WaitAllOngoingNotInIVR(confid) 
    
    sleep(1)
    
    sleep(2)
#-----------------------------------------------------------------------

def ConnectVideoParties2(connection, conf_id, start_from_indx, num_of_parties, vidMode):
       for x in range(num_of_parties):
          print "x=" + str(x)
          start_from_indx=start_from_indx+1
          #------Dial out H323 party   
          partyname = "Party_"+str(start_from_indx)
          partyip = "1.2.3." + str(start_from_indx)
          print "partyname=" + partyname
          print "partyip=" + str(partyip)
          connection.AddVideoParty(conf_id, partyname, partyip, False, vidMode)
          sleep(3)

       connection.WaitAllPartiesWereAdded(conf_id, num_of_parties, num_retries*2)
       connection.WaitAllOngoingConnected(conf_id)
       connection.WaitAllOngoingNotInIVR(conf_id)
       print "END FUNC ConnectVideoParties"
       
#--------------------------------------------------------------------------------
def GetConfId(self,confName,retires = 60):
    ##print "GetConfId"

    """Monitor conferences list until 'confName' is found.
    Returns conference ID.

    confName - lookup conf name 
    """
    bFound = False
    for retry in range(retires+1):
        status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for index in range(len(ongoing_conf_list)):  
            if(confName == ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                if confid != "":
                    bFound = True
                    break
        if(bFound):
            break
        if (retry == retires):
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            self.Disconnect()                
            ScriptAbort("GetConfId: Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
    return confid
#--------------------------------------------------------------------------------
    
##-----------
## Auto Mode:
##-----------
connection = McmsConnection()
connection.Connect()
c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

c.SetMode("auto")
c.CheckModes("auto","auto")
c.SetAutoPortConfiguration(0)

num_retries=1
num_of_parties1=1
confName1 = "ConfAutoCifSD"
#connect one SD
confFile = 'Scripts/SD/AddVideoCpConf1920kMotion.xml'
ConfWithOneVideoParty(c, confName1, confFile, "H323", "DialOut")

#connect CIFs
conf_id1 = GetConfId(c,confName1,num_retries)
ConnectVideoParties2(connection, conf_id1, 1, 161, "auto")#auto-Mix-Flex
print "after ConnectVideoParties!!!!"

#connection.DeleteAllConf()
sleep(1)
#connection.Disconnect()
