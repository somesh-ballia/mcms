#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
 
from McmsConnection import *
c = McmsConnection()
c.Connect()
print "Operators Monitor ..."
c.SendXmlFile("Scripts/CardListMonitor.xml")
