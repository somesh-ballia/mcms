#!/mcms/python/bin/python
 
from McmsConnection import *

class McmsTargetConnection(McmsConnection):
    
#------------------------------------------------------------------------------    
  def RemoteCommand(self,cmd):
    return os.popen("./plink " + self.user + "@" + self.ip + " -pw "+ self.password +" "+cmd).readlines()

#------------------------------------------------------------------------------

