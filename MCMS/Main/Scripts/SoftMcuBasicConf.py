#!/mcms/python/bin/python

# # NAME: SoftMcuBasicConf
# # OWNER: SOFT
# # GROUP: SoftConf
# # NIGHT: FALSE
# # VERSION: FALSE
# # VRESTART_AFTER: FALSE

# disabled since SMCUConference is not complete

from McmsConnection import *

c = McmsConnection()
c.Connect()
c.DeleteAllConf(1)
from SMCUConference import *
#---Create Conference
conf = SMCUConference()
conf.Create(c, "VideoConf")

sleep(10)

#---Close Conference
c.DeleteAllConf(1)
c.Disconnect()
