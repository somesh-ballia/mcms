#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from McmsConnection import *

c = McmsConnection()
c.Connect()

c.LoadXmlFile('Scripts/AddEmptyCopConf.xml')
#print "Adding Conf: " + confname + "  ..."
c.Send()

sleep(5)
            
c.DeleteAllConf()

c.WaitAllConfEnd(20)
#c.SimpleXConfTest(1, # num of conf
#                  'Scripts/AddEmptyCopConf.xml', # conf templete
#                  20) # timeout

c.Disconnect()
 
  
