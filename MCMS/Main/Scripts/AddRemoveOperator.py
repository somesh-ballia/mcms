#!/mcms/python/bin/python

# this is an example for list of processes that will be running under valgrind
#*PROCESSES_FOR_VALGRIND='Authentication McuMngr MplApi GideonSim QAAPI Faults Logger Configurator'
# this is an example for profile of processes that will be running under valgrind
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2

from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Adding new Operator..."

c.SendXmlFile("Scripts/AddNewOperator.xml")

print "Waiting 40 seconds to delete Operator..."
sleep (40)
c.Connect()
print "Deleting new Operator..."
c.SendXmlFile("Scripts/RemoveNewOperator.xml")
