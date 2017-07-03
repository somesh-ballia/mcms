#!/mcms/python/bin/python

# # NAME: SoftMcu10Conf
# # OWNER: SOFT
# # GROUP: SoftConf
# # NIGHT: TRUE
# # VERSION: FALSE
# # VRESTART_AFTER: FALSE

from McmsConnection import *

c = McmsConnection()
c.Connect()
c.DeleteAllConf()
num_of_conf = 10
	 
c.SimpleXConfTest(num_of_conf,
                  'Scripts/AddVideoCpConfTemplate.xml', # conf templete
                  25) # timeout

c.Disconnect()
