#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=MplApi EndpointsSim GideonSim CSApi Logger Resource CDR Faults McuMngr CSMngr ConfParty EncryptionKeyServer

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

from McmsConnection import *
from ResourceUtilities import *
import os
import sys

command_line = 'Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 0'
os.system(command_line)
sleep(1)
c = McmsConnection()
c.Connect()

os.system("Bin/McuCmd set mcms IS_DOUBLE_DSP YES")

#-----------------------------------------------------------------------------
cif30resources = 1
sd15_2cif30resources = 2
sd30resources = 4


#-------Verify that all the resources are free---------
r = ResourceUtilities()
r.Connect()
r.SendXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()
freeresources = r.GetFreeCarmelParties()
print "free resource " + str(freeresources)
r.TestFreeCarmelParties(int(num_of_resource))

print "\n---------------------------------------"
print "         Full Capacity "
print "---------------------------------------"
print "Test1: Full Capacity SD 30 Sharpness"
print "---------------------------------------"
numberOfPArties = 10
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
confid = c.SimpleXmlConfPartyTest(confFile,
                         'Scripts/AddVideoParty1.xml',
                         numberOfPArties,
                         timeuot,
                                  'FALSE') #Do not delete conference
r.TestFreeCarmelParties(int(num_of_resource) - (int(numberOfPArties) * int(sd30resources)))
c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

print "Test2: Full Capacity SD 30 Motion"
print "---------------------------------------"
numberOfPArties = 10
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
confid = c.SimpleXmlConfPartyTest(confFile,
                         'Scripts/AddVideoParty1.xml',
                         numberOfPArties,
                         timeuot,
                                  'FALSE') #Do not delete conference
r.TestFreeCarmelParties(int(num_of_resource) - (int(numberOfPArties) * int(sd30resources)))
c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

print "Test3: Full Capacity SD 15 Sharpness"
print "---------------------------------------"
numberOfPArties = 20
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
confid = c.SimpleXmlConfPartyTest(confFile,
                         'Scripts/AddVideoParty1.xml',
                         numberOfPArties,
                         timeuot,
                                  'FALSE') #Do not delete conference
r.TestFreeCarmelParties(int(num_of_resource) - (int(numberOfPArties) * int(sd15_2cif30resources)))
c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

print "Test4: Full Capacity 2CIF 30 Motion"
print "---------------------------------------"
numberOfPArties = 20
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf384kMotion.xml'
confid = c.SimpleXmlConfPartyTest(confFile,
                         'Scripts/AddVideoParty1.xml',
                         numberOfPArties,
                         timeuot,
                                  'FALSE') #Do not delete conference
r.TestFreeCarmelParties(int(num_of_resource) - (int(numberOfPArties) * int(sd15_2cif30resources)))
c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

print "Test5: Full Capacity CIF 30 Sharpness"
print "---------------------------------------"
numberOfPArties = 40
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf128kSharpness.xml'
confid = c.SimpleXmlConfPartyTest(confFile,
                         'Scripts/AddVideoParty1.xml',
                         numberOfPArties,
                         timeuot,
                                  'FALSE') #Do not delete conference
r.TestFreeCarmelParties(int(num_of_resource) - int(numberOfPArties) )
c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

print "Test5: Full Capacity CIF 30 Motion"
print "---------------------------------------"
numberOfPArties = 40
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf128kMotion.xml'
confid = c.SimpleXmlConfPartyTest(confFile,
                         'Scripts/AddVideoParty1.xml',
                         numberOfPArties,
                         timeuot,
                                  'FALSE') #Do not delete conference
r.TestFreeCarmelParties(int(num_of_resource) - int(numberOfPArties) )
c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )
c.Disconnect()
