#!/mcms/python/bin/python

####################################################################
###				==================================				####
###				==	THIS FILE IS OBSOLETE !!!!	==				####
###				==================================				####
### From 19/9/2014 we are using the new Test script method.		####
### Please refer to cs_sipp_new_tests.py						####
###																####
import cs_sipp_new_tests										####
###																####
### We will keep the below old tests running					####
####################################################################

# currently we don't want valgrind this can change in phase3
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#*export CS_SIMULATION_TEST=YES

#*export GIDEONSIM_MNGMNT_CONFIG_FILE=Cfg/NetworkCfg_Management.xml

#*export USE_ALT_IP_SERVICE=Scripts/CsSimulationConfig/IPServiceList_default_udp.xml

#*export USE_ALT_MNGMNT_SERVICE=Scripts/CsSimulationConfig/NetworkCfg_Management_ipv4_only.xml

#*export ENDPOINTSSIM=NO

#*PRERUN_SCRIPTS =make_cs_simulation_env.py

#-- SKIP_ASSERTS
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *
#-- SKIP_ASSERTS



import os, string
import sys
import subprocess


from McmsConnection import *
from ConfUtils.ConfCreation import *
from CsSimulationConfig.SippUtill import *
from subprocess import call
from sys import stdout

connection = McmsConnection()
connection.Connect()

sleep(2)
ConfActionsClass = ConfCreation() # conf util class


sippDialInDefault = SippUtill() # create sipp EP obj 

print "Test Default Sip Scanrio rtcp-fb2.xml"
# Add conf
print "Adding Conf..."
confName = "conf_test_default"
num_of_parties = 3
ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(connection, confName)

print "confernce ID " + str(confid)
sippDialInDefault.SetCommonParams(num_of_parties,confName) # uses the default Scripts/CsSimulationConfig/rtcp-fb2.xml scanrio
#sippDialInDefault.use_remote_host=1
sippDialInDefault.Run()
connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
sleep(2)
connection.DeleteConf(confid)
sippDialInDefault.KillAllSippProcess()

print "Test scanrio dialin  no SDP Hold Resume"
sleep(2)
num_of_parties=1
sippDialInNoSDP = SippUtill() # create sipp EP obj
confName = "conf_test_No_SDP"
sippDialInNoSDP.SetCallParams('Scripts/CsSimulationConfig/dialinEX90noSDPHoldResume_5173.xml',confName)
ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(connection, confName)
print "confernce ID " + str(confid)
sippDialInNoSDP.Run()
connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
sleep(2)
connection.DeleteConf(confid)
sippDialInNoSDP.KillAllSippProcess()


def RunCommand(command):
    ps     = subprocess.Popen(command , shell=True, stdout=subprocess.PIPE)
    output = ps.stdout.read()
    ps.stdout.close()
    ps.wait()
    return output
#-----------------------


#### vngfe-7588.xml


print "DMA tags"
sleep(2)
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","YES"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","YES"])
os.system("Scripts/Flush.sh")

num_of_parties=1
sippDmaTagsAudioOnly = SippUtill() # create sipp EP obj
confName = "DMA_Tags1"
sippDmaTagsAudioOnly.SetCallParams('Scripts/CsSimulationConfig/vngfe-7588.xml',confName)
ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(connection, confName)
print "conference ID " + str(confid)
sippDmaTagsAudioOnly.Run()
connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
sleep(2)
connection.DeleteConf(confid)
os.system("Scripts/Flush.sh")
LOG=RunCommand("ls -latr LogFiles/*.txt | tail -1  |  awk '{print $9}'")
print "log = " + str(LOG)

video=RunCommand("egrep -e \"type.: 1\" " + LOG )
#video=RunCommand("grep \"type	: 1\" " + LOG )
print "video = " + str(video)

if video:
	print "Test FAILED - RMX caps shouldn't contain video = " + video
	sys.exit(1)


call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","NO"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","NO"])


sippDmaTagsAudioOnly.KillAllSippProcess()

### END vngfe-7588.xml TEST #####




#### DMA tags #2
print "######## Testing DMA tags #2             ####################"
sleep(2)
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","YES"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","YES"])
os.system("Scripts/Flush.sh")

num_of_parties=1
sippDmaTagsAudioOnly = SippUtill() # create sipp EP obj
confName = "DMA_Tags2"
sippDmaTagsAudioOnly.SetCallParams('Scripts/CsSimulationConfig/DmaTags2.xml',confName)
ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(connection, confName)
print "conference ID " + str(confid)
sippDmaTagsAudioOnly.Run()
connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
sleep(2)
connection.DeleteConf(confid)
os.system("Scripts/Flush.sh")
LOG=RunCommand("ls -latr LogFiles/*.txt | tail -1  |  awk '{print $9}'")
print "log = " + str(LOG)

fecc=RunCommand("egrep -e \"type.: 2\" " + LOG )
print "FECC = " + str(fecc)

if fecc:
       	print "Test FAILED - RMX caps shouldn't contain FECC = " + fecc
        sys.exit(1)


call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","NO"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","NO"])


sippDmaTagsAudioOnly.KillAllSippProcess()

### END DMA Tags TEST #####




#### DMA tags #3
print "######## Testing DMA tags #3             ####################"
sleep(2)
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","YES"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","YES"])
os.system("Scripts/Flush.sh")

num_of_parties=1
sippDmaTagsAudioOnly = SippUtill() # create sipp EP obj
confName = "DMA_Tags3"
sippDmaTagsAudioOnly.SetCallParams('Scripts/CsSimulationConfig/DmaTags3.xml',confName)
ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(connection, confName)
print "conference ID " + str(confid)
sippDmaTagsAudioOnly.Run()
connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
sleep(2)
connection.DeleteConf(confid)
os.system("Scripts/Flush.sh")
LOG=RunCommand("ls -latr LogFiles/*.txt | tail -1  |  awk '{print $9}'")
print "log = " + str(LOG)

dmatags4_bfcp=RunCommand("egrep -e \"type.: 2\" -A2  " + LOG )
dmatags4_bfcp2=string.find(str(dmatags4_bfcp),"UdpBfcp")
print "BFCP = " + str(dmatags4_bfcp)
print "BFCP2 = " + str(dmatags4_bfcp2)

#if bfcp:
#        print "Test FAILED - RMX caps shouldn't contain BFCP = " + str(bfcp)
#	sys.exit(1)
if dmatags4_bfcp2 != -1:
	print "RMX caps shouldn't contain BFCP --> BFCP2 = " + str(dmatags_bfcp2)
        sys.exit(1)

## verify fecc  cap appear in rmx caps.
tags4_fecc=RunCommand("egrep -e \"type.: 2\" -A17  " + LOG )
tags4_fecc2=string.find(str(tags4_fecc),"AnnexQ")
print "FECC = " + str(tags4_fecc)
print "FECC2 = " + str(tags4_fecc2)

if tags4_fecc2 == -1:
        print "Test FAILED - RMX caps should contain AnnexQ caps = " + tags4_fecc2
        sys.exit(1)


call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","NO"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","NO"])


sippDmaTagsAudioOnly.KillAllSippProcess()

### END DMA Tags TEST #####



#### DMA tags #4
#### Test is simulating VEQ-VMR scenario with DMA where DMA is sending audio/video/bfcp-udp in x-plcm-require header.
#### 1. verify fecc channel doesn't appear in RMX caps.
#### 2. verify bfcpudp channel appear in RMX caps.

print "######## Testing DMA tags #4             ####################"
sleep(2)
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","YES"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","YES"])
os.system("Scripts/Flush.sh")

num_of_parties=1
sippDmaTagsAudioOnly = SippUtill() # create sipp EP obj
confName = "DMA_Tags4"
sippDmaTagsAudioOnly.SetCallParams('Scripts/CsSimulationConfig/DmaTags4.xml',confName)
ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(connection, confName)
print "conference ID " + str(confid)
sippDmaTagsAudioOnly.Run()
connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
sleep(2)
connection.DeleteConf(confid)
os.system("Scripts/Flush.sh")
LOG=RunCommand("ls -latr LogFiles/*.txt | tail -1  |  awk '{print $9}'")
print "log = " + str(LOG)

fecc=RunCommand("egrep -e \"type.: 2\" -A17 " + LOG )
print "FECC = " + str(fecc)
fecc2=string.find(str(fecc),"AnnexQ")

if fecc2 != -1:
        print "Test FAILED - RMX caps shouldn't contain FECC = " + fecc
        sys.exit(1)


## verify bfcpudp cap appear in rmx caps.
bfcp=RunCommand("egrep -e \"type.: 2\" -A2  " + LOG )
bfcp2=string.find(str(bfcp),"UdpBfcp")
print "BFCP = " + str(bfcp)
print "BFCP2 = " + str(bfcp2)

if bfcp2 == -1:
        print "Test FAILED - RMX caps should contain BFCP UDP= " + bfcp2
        sys.exit(1)
	


call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG","NO"])
call(["Scripts/McuCmd.sh", "set","all","X_PLCM_REQUIRE_FECC_ENABLE_TAG","NO"])


sippDmaTagsAudioOnly.KillAllSippProcess()

### END DMA Tags TEST #####
# 
# print "Test scanrio dialout"
#  
# sleep(2)
# sippDialOut = SippUtill() # create sipp EP obj
# confName = "conf_test_DialOut"
# sippDialOut.SetCallParams('Scripts/CsSimulationConfig/effi_RMXdialout.xml',confName,dial_direction='DialOut')
# ConfActionsClass.CreateConf(connection, confName, 'Scripts/CsSimulationConfig/AddConfTemplate.xml', "NONE")
# confid = ConfActionsClass.WaitConfCreated(connection, confName)
# print "confernce ID " + str(confid)
# sippDialOut.Run()
# partyip =  "127.0.0.1"
# partySipAdd = ""
# connection.AddSIPParty(confid, "SipVideoParty", partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialOutSipParty.xml")
# connection.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
# connection.DeleteConf(confid)
# sippDialOut.KillAllSippProcess()


