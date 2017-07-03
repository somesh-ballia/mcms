#!/mcms/python/bin/python

# # NAME: SoftEmptyCopConf
# # OWNER: SOFT
# # GROUP: SoftConf
# # NIGHT: TRUE
# # VERSION: FALSE
# # VRESTART_AFTER: FALSE

from McmsConnection import *

c = McmsConnection()
c.Connect()
c.DeleteAllConf()
c.LoadXmlFile('Scripts/AddEmptyCopConf.xml')
#print "Adding Conf: " + confname + "  ..."
c.Send()
sleep(10)         
c.DeleteAllConf()
c.WaitAllConfEnd(20)

c.Disconnect()
 
  
