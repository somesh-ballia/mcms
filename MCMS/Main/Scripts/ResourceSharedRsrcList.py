#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer McuMngr Configurator BackupRestore
#*export SYSTEM_CFG_USER_FILE=Scripts/SysConfig/SystemCfgApacheKeepAlive120.xml
from McmsConnection import *
import os

import os
from ResourceUtilities import *

#------------------------------------------------------------------------------

def ConnectVideoParties(connection, conf_id, num_of_parties, num_retries):
       for x in range(num_of_parties):
          #------Dial out H323 party   
          partyname = "Party_"+str(x+1)
          partyip = "1.2.3." + str(x+1) 
          connection.AddVideoParty(conf_id, partyname, partyip)

       connection.WaitAllPartiesWereAdded(conf_id, num_of_parties, num_retries*2)
	
       connection.WaitAllOngoingConnected(conf_id)
       connection.WaitAllOngoingNotInIVR(conf_id)
#-----------------------------------------------------------------------

def TestResultsConfIdRepList(connection, numTypes, numOfConfs, testedConfIndex, testedOccupiedPorts, testedPortType):
   print "Test results:"
   print "============="
   i1 = 0
   i2 = 0
   all_conf_flg=0
   for i2 in range(numOfConfs):
     confId = connection.GetTextUnder("RSRC_REPORT_RMX_CONF_LIST","CONF_ID",i2,0)
     if confId!="":
       if ((confId!="-1") and (all_conf_flg==1)) or (confId=="-1"):
         all_conf_flg=1
         confId = connection.GetTextUnder("RSRC_REPORT_RMX_CONF_LIST","CONF_ID",i2+1,0)
     else:
       print "confId Empty !!!"
     if all_conf_flg==1:
       i2=i2+1
     if (all_conf_flg!=1 and i2==testedConfIndex) or (all_conf_flg==1 and i2==testedConfIndex+1):
       for i1 in range(numTypes):
         resourceType = connection.GetTextUnder("RSRC_REPORT_RMX_CONF","RSRC_REPORT_ITEM",i1)
         occupied = connection.GetTextUnderList("RSRC_REPORT_RMX_CONF_LIST","RSRC_REPORT_RMX_CONF","OCCUPIED",i2,i1,0)
         if resourceType=="":
            print "resourceType Empty !!!"
            return
         else:
            if resourceType==testedPortType:
               if occupied=="":
                  print "occupied Empty !!!"
                  return
               if testedOccupiedPorts!= int(occupied): 
                  print "confId = " + confId
                  print "Tested Resource Port TYPE = " + resourceType
                  print "----------------------------------------------"
                  print "occupied ports = " + str(occupied) + ", expected occupied ports = " + str(testedOccupiedPorts)
                  print connection.xmlResponse.toprettyxml(encoding="utf-8")
                  return
               else:
                  print "confId = " + confId
                  print "TestResultsConfIdRepList OK resourceType = " + resourceType
                  print "TestResultsConfIdRepList OK occupied = " + occupied
                  print "======================================================"
                  return
         	
   print "----------------------End Response function-----------------------------"
   return
#--------------------------------------------------------------------------------

num_retries=1

print "########################## Two conferences: AUTO:##############################"
print "## Two conferences - first with 1 audio party. second - with 3 CIF parties.  ##"
print "###############################################################################"

connection = McmsConnection()
connection.Connect()

c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

c.SetMode("auto")
c.CheckModes("auto","auto")
sleep(10)
confnm = "Conf1"
c.CreateConfWithDisplayName(confnm)
confid1 = c.WaitConfCreated(confnm)  
print "Adding dial-out audio only party"
partyname = "AudioParty"
partyip =  "1.7.8.0"
c.AddParty(confid1, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323AudioParty.xml")
c.WaitAllPartiesWereAdded(confid1,1,10)
partyId = c.GetPartyId(confid1, partyname)
c.WaitAllOngoingConnected(confid1)

profId=c.AddProfileWithRate("ProfRate128",128)
print "Starting one conf with 3 CIF parties"
confnm2 = "Conf2"
num_of_parties = 3
confid2 = c.CreateConfFromProfileWithVideoParties(confnm2,profId,num_of_parties)
c.WaitAllOngoingConnected(confid2)


connection.LoadXmlFile('Scripts/ResourceSharedRsrcNoConfList.xml')
connection.AddXML("GET_LS","CONF_ID",confid1)
connection.AddXML("GET_LS","CONF_ID",confid2)
connection.Send()
sleep(1)
###response
TestResultsConfIdRepList(connection, 5, 2, 0, 1, "audio")
TestResultsConfIdRepList(connection, 5, 2, 1, 3, "CIF")

c.ConnectDisconnectAllParties(confid1, "false")
c.WaitAllOngoingDisConnected(confid1)

c.ConnectDisconnectAllParties(confid2, "false")
c.WaitAllOngoingDisConnected(confid2)

connection.DeleteAllConf()
sleep(1)
c.Disconnect() 
connection.Disconnect()


print "########################## All conferences: AUTO:##############################"
print "## Creating two conferences - first with 1 party. second - with 2 parties.   ##"
print "## both uses default configuration -- 160 CIF ports available.               ##" 
print "###############################################################################"

connection = McmsConnection()
connection.Connect()

c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

c.SetMode("auto")
c.CheckModes("auto","auto")

num_of_parties1=1
confName1 = "confTst1"
connection.CreateConfWithDisplayName (confName1)
conf_id1  = connection.WaitConfCreated(confName1,num_retries)
ConnectVideoParties(connection, conf_id1, num_of_parties1, num_retries)

num_of_parties2=2
confName2 = "confTst2"
connection.CreateConfWithDisplayName (confName2)
conf_id2  = connection.WaitConfCreated(confName2,num_retries)
ConnectVideoParties(connection, conf_id2, num_of_parties2, num_retries)

connection.LoadXmlFile('Scripts/ResourceSharedRsrcNoConfList.xml')

conf_all="-1"
connection.AddXML("GET_LS","CONF_ID",conf_all)
connection.Send()
sleep(1)

###response
TestResultsConfIdRepList(connection, 2, 2, 0, 2, "video")
TestResultsConfIdRepList(connection, 2, 2, 1, 3, "video")

connection.DeleteAllConf()
sleep(1)
connection.Disconnect()
c.Disconnect()


print "##################### Three AUTO detailed conferences:#########################"
print "## Creating three conferences - first with 1 party. second - with 2 parties. ##"
print "## third with 3 parties. All confs uses default configuration. 160 CIF ports ##" 
print "###############################################################################"

connection = McmsConnection()
connection.Connect()

c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

################################################################
# 25-12-2011:
# add sleep for 10 seconds - timers were doubled - operation takes more time.
# const int SECONDS_FOR_RECONFIGURE_UNITS_TIMER             = 20; // VNGFE-4697 - Instead of 10 seconds
# const int SECONDS_FOR_RECOVERY_UNITS_TIMER                = 40; // VNGR-22871 - Instead of 20 seconds
# the problem is probably only when changing from fix to auto
# after adding sleep for 10 second problem resolved.
##############################################################
sleep(10)

c.SetMode("auto")
c.CheckModes("auto","auto")


connection.CreateConfWithDisplayName (confName1)
conf_id1  = connection.WaitConfCreated(confName1,num_retries)
ConnectVideoParties(connection, conf_id1, num_of_parties1, num_retries)

connection.CreateConfWithDisplayName (confName2)
conf_id2  = connection.WaitConfCreated(confName2,num_retries)
ConnectVideoParties(connection, conf_id2, num_of_parties2, num_retries)

num_of_parties3=3
confName3 = "confTst3"
connection.CreateConfWithDisplayName (confName3)
conf_id3  = connection.WaitConfCreated(confName3,num_retries)
ConnectVideoParties(connection, conf_id3, num_of_parties3, num_retries)

connection.LoadXmlFile('Scripts/ResourceSharedRsrcList.xml')

connection.ModifyXmlList("GET_LS","CONF_ID", 0, conf_id1)
connection.ModifyXmlList("GET_LS","CONF_ID", 1, conf_id2)
connection.ModifyXmlList("GET_LS","CONF_ID", 2, conf_id3)

connection.Send()
sleep(1)

###response
TestResultsConfIdRepList(connection, 2, 3, 0, 2, "video")
TestResultsConfIdRepList(connection, 2, 3, 1, 3, "video")
TestResultsConfIdRepList(connection, 2, 3, 2, 5, "video") #not 9

connection.DeleteAllConf()
sleep(1)
connection.Disconnect()
c.Disconnect()



print "##################### Three AUTO detailed conferences:#########################"
print "## Creating three conferences - first with 1 party. second - with 2 parties. ##"
print "##  third with 3 parties. All confs uses specificlly defined configuration.  ##" 
print "###############################################################################"

connection = McmsConnection()
connection.Connect()

c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

c.SetMode("auto")
c.CheckModes("auto","auto")
sleep(10)
#c.SetEnhancedConfiguration(350,40,15,10,0)

sleep(2)
connection.CreateConfWithDisplayName (confName1)
conf_id1  = connection.WaitConfCreated(confName1,num_retries)
ConnectVideoParties(connection, conf_id1, num_of_parties1, num_retries)

connection.CreateConfWithDisplayName (confName2)
conf_id2  = connection.WaitConfCreated(confName2,num_retries)
ConnectVideoParties(connection, conf_id2, num_of_parties2, num_retries)

num_of_parties3=3
confName3 = "confTst3"
connection.CreateConfWithDisplayName (confName3)
conf_id3  = connection.WaitConfCreated(confName3,num_retries)
ConnectVideoParties(connection, conf_id3, num_of_parties3, num_retries)

connection.LoadXmlFile('Scripts/ResourceSharedRsrcList.xml')

connection.ModifyXmlList("GET_LS","CONF_ID", 0, conf_id1)
connection.ModifyXmlList("GET_LS","CONF_ID", 1, conf_id2)
connection.ModifyXmlList("GET_LS","CONF_ID", 2, conf_id3)

connection.Send()
sleep(1)

###response
TestResultsConfIdRepList(connection, 5, 3, 0, 1, "SD")
TestResultsConfIdRepList(connection, 5, 3, 1, 2, "SD")
TestResultsConfIdRepList(connection, 5, 3, 2, 3, "SD") 


connection.DeleteAllConf()
sleep(1)
connection.Disconnect()
c.Disconnect()

####################################################################################
print "--------------- End Of Script ResourceSharedRsrcList.py ----------------" 
print "------------------------------------------------------------------------\n"

  
