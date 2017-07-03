#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2

from McmsConnection import *

c = McmsConnection()
#c.Connect()

#c.SimpleNewConfTestWithoutTerminate("Conf1", # conference name
#                  'Scripts/AddVideoCpConfTemplate.xml', # conf template
#                  1) # timeout
                  
print "===Try to login through proxy to another RMX==="
c.ProxyLogin("172.22.185.25", "80")

#c.SimpleNewConfTestWithoutTerminate("Conf2", # conference name
#                  'Scripts/AddVideoCpConfTemplate.xml', # conf template
#                  1) # timeout

#c.Disconnect()

