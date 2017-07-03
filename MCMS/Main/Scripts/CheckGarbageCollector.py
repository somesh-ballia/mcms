#!/mcms/python/bin/python

# this is an example for list of processes that will be running under valgrind
#*PROCESSES_FOR_VALGRIND='Authentication McuMngr MplApi GideonSim QAAPI Faults Logger Configurator'
# this is an example for profile of processes that will be running under valgrind
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2

from McmsConnection import *
 
c = McmsConnection()
c.Connect()
 
print "Send GET_STATE transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/GetStateWithActionTag.xml')
c.Send("Status OK")


print "Wait 60 seconds before sending GET_STATE transaction"
sleep(60)

print "Send GET_STATE transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/GetStateWithActionTag.xml')
c.Send("Status OK")

print "Wait 3 Minutes before sending GET_STATE transaction - should be failed!"
sleep(180)

print "Send GET_STATE transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/GetStateWithActionTag.xml')
c.Send("User not found")

